/*
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2000 PHP Development Team (See Credits file)      |
   +----------------------------------------------------------------------+
   | This program is free software; you can redistribute it and/or modify |
   | it under the terms of one of the following licenses:                 |
   |                                                                      |
   |  A) the GNU General Public License as published by the Free Software |
   |     Foundation; either version 2 of the License, or (at your option) |
   |     any later version.                                               |
   |                                                                      |
   |  B) the PHP License as published by the PHP Development Team and     |
   |     included in the distribution in the file: LICENSE                |
   |                                                                      |
   | This program is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY; without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
   | GNU General Public License for more details.                         |
   |                                                                      |
   | You should have received a copy of both licenses referred to here.   |
   | If you did not, or have any questions about PHP licensing, please    |
   | contact core@php.net.                                                |
   +----------------------------------------------------------------------+
   | Authors:                                                             |
   |          Andrew Skalski      <askalski@chek.com>                     |
   +----------------------------------------------------------------------+
 */

/* $Id: ftp.c,v 1.16 2000/02/22 20:51:05 askalski Exp $ */

#include "php.h"

#if HAVE_FTP

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef WIN32
#include "win32/unistd.h"
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <errno.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "ftp.h"


/* sends an ftp command, returns true on success, false on error.
 * it sends the string "cmd args\r\n" if args is non-null, or
 * "cmd\r\n" if args is null
 */
static int		ftp_putcmd(	ftpbuf_t *ftp,
					const char *cmd,
					const char *args);

/* wrapper around send/recv to handle timeouts */
static int		my_send(int s, void *buf, size_t len);
static int		my_recv(int s, void *buf, size_t len);
static int		my_connect(int s, const struct sockaddr *addr,
				int addrlen);
static int		my_accept(int s, struct sockaddr *addr, int *addrlen);

/* reads a line the socket , returns true on success, false on error */
static int		ftp_readline(ftpbuf_t *ftp);

/* reads an ftp response, returns true on success, false on error */
static int		ftp_getresp(ftpbuf_t *ftp);

/* sets the ftp transfer type */
static int		ftp_type(ftpbuf_t *ftp, ftptype_t type);

/* opens up a data stream */
static databuf_t*	ftp_getdata(ftpbuf_t *ftp);

/* accepts the data connection, returns updated data buffer */
static databuf_t*	data_accept(databuf_t *data);

/* closes the data connection, returns NULL */
static databuf_t*	data_close(databuf_t *data);

/* generic file lister */
static char**		ftp_genlist(ftpbuf_t *ftp,
				const char *cmd, const char *path);

/* IP and port conversion box */
union ipbox {
	unsigned long	l[2];
	unsigned short	s[4];
	unsigned char	c[8];
};


ftpbuf_t*
ftp_open(const char *host, short port)
{
	int			fd = -1;
	ftpbuf_t		*ftp;
	struct sockaddr_in	addr;
	struct hostent		*he;
	int			size;

	/* set up the address */
	if ((he = gethostbyname(host)) == NULL) {
#if 0
		herror("gethostbyname");
#endif
		return NULL;
	}

	memset(&addr, 0, sizeof(addr));
	memcpy(&addr.sin_addr, he->h_addr, he->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = port ? port : htons(21);


	/* alloc the ftp structure */
	ftp = calloc(1, sizeof(*ftp));
	if (ftp == NULL) {
		perror("calloc");
		return NULL;
	}

	/* connect */
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		goto bail;
	}

	if (my_connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("connect");
		goto bail;
	}

	size = sizeof(addr);
	if (getsockname(fd, (struct sockaddr*) &addr, &size) == -1) {
		perror("getsockname");
		goto bail;
	}

	ftp->localaddr = addr.sin_addr;
	ftp->fd = fd;

	if (!ftp_getresp(ftp) || ftp->resp != 220) {
		goto bail;
	}

	return ftp;

bail:
	if (fd != -1)
		close(fd);
	free(ftp);
	return NULL;
}


ftpbuf_t*
ftp_close(ftpbuf_t *ftp)
{
	if (ftp == NULL)
		return NULL;
	if (ftp->fd != -1)
		close(ftp->fd);
	ftp_gc(ftp);
	free(ftp);
	return NULL;
}


void
ftp_gc(ftpbuf_t *ftp)
{
	if (ftp == NULL)
		return;

	free(ftp->pwd);
	ftp->pwd = NULL;
	free(ftp->syst);
	ftp->syst = NULL;
}


int
ftp_quit(ftpbuf_t *ftp)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "QUIT", NULL))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 221)
		return 0;

	free(ftp->pwd);
	ftp->pwd = NULL;

	return 1;
}


int
ftp_login(ftpbuf_t *ftp, const char *user, const char *pass)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "USER", user))
		return 0;
	if (!ftp_getresp(ftp))
		return 0;
	if (ftp->resp == 230)
		return 1;
	if (ftp->resp != 331)
		return 0;
	if (!ftp_putcmd(ftp, "PASS", pass))
		return 0;
	if (!ftp_getresp(ftp))
		return 0;
	return (ftp->resp == 230);
}


int
ftp_reinit(ftpbuf_t *ftp)
{
	if (ftp == NULL)
		return 0;

	ftp_gc(ftp);

	if (!ftp_putcmd(ftp, "REIN", NULL))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 220)
		return 0;

	return 1;
}


const char*
ftp_syst(ftpbuf_t *ftp)
{
	char		*syst, *end;

	if (ftp == NULL)
		return NULL;

	/* default to cached value */
	if (ftp->syst)
		return ftp->syst;

	if (!ftp_putcmd(ftp, "SYST", NULL))
		return NULL;
	if (!ftp_getresp(ftp) || ftp->resp != 215)
		return NULL;

	syst = ftp->inbuf;
	if ((end = strchr(syst, ' ')))
		*end = 0;
	ftp->syst = strdup(syst);
	if (end)
		*end = ' ';

	return ftp->syst;
}


const char*
ftp_pwd(ftpbuf_t *ftp)
{
	char		*pwd, *end;

	if (ftp == NULL)
		return NULL;

	/* default to cached value */
	if (ftp->pwd)
		return ftp->pwd;

	if (!ftp_putcmd(ftp, "PWD", NULL))
		return NULL;
	if (!ftp_getresp(ftp) || ftp->resp != 257)
		return NULL;

	/* copy out the pwd from response */
	if ((pwd = strchr(ftp->inbuf, '"')) == NULL)
		return NULL;
	end = strrchr(++pwd, '"');
	*end = 0;
	ftp->pwd = strdup(pwd);
	*end = '"';

	return ftp->pwd;
}


int
ftp_chdir(ftpbuf_t *ftp, const char *dir)
{
	if (ftp == NULL)
		return 0;

	free(ftp->pwd);
	ftp->pwd = NULL;

	if (!ftp_putcmd(ftp, "CWD", dir))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 250)
		return 0;

	return 1;
}


int
ftp_cdup(ftpbuf_t *ftp)
{
	if (ftp == NULL)
		return 0;

	free(ftp->pwd);
	ftp->pwd = NULL;

	if (!ftp_putcmd(ftp, "CDUP", NULL))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 250)
		return 0;

	return 1;
}


char*
ftp_mkdir(ftpbuf_t *ftp, const char *dir)
{
	char		*mkd, *end;

	if (ftp == NULL)
		return NULL;

	if (!ftp_putcmd(ftp, "MKD", dir))
		return NULL;
	if (!ftp_getresp(ftp) || ftp->resp != 257)
		return NULL;

	/* copy out the dir from response */
	if ((mkd = strchr(ftp->inbuf, '"')) == NULL)
		return NULL;
	end = strrchr(++mkd, '"');
	*end = 0;
	mkd = strdup(mkd);
	*end = '"';

	return mkd;
}


int
ftp_rmdir(ftpbuf_t *ftp, const char *dir)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "RMD", dir))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 250)
		return 0;

	return 1;
}


char**
ftp_nlist(ftpbuf_t *ftp, const char *path)
{
	return ftp_genlist(ftp, "NLST", path);
}


char**
ftp_list(ftpbuf_t *ftp, const char *path)
{
	return ftp_genlist(ftp, "LIST", path);
}


int
ftp_type(ftpbuf_t *ftp, ftptype_t type)
{
	char		typechar[2] = "?";

	if (ftp == NULL)
		return 0;

	if (type == ftp->type)
		return 1;

	if (type == FTPTYPE_ASCII)
		typechar[0] = 'A';
	else if (type == FTPTYPE_IMAGE)
		typechar[0] = 'I';
	else
		return 0;

	if (!ftp_putcmd(ftp, "TYPE", typechar))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 200)
		return 0;

	ftp->type = type;

	return 1;
}


int
ftp_pasv(ftpbuf_t *ftp, int pasv)
{
	char			*ptr;
	union ipbox		ipbox;
	unsigned long		b[6];
	int			n;

	if (ftp == NULL)
		return 0;

	if (pasv && ftp->pasv == 2)
		return 1;

	ftp->pasv = 0;
	if (!pasv)
		return 1;

	if (!ftp_putcmd(ftp, "PASV", NULL))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 227)
		return 0;

	/* parse out the IP and port */
	for (ptr = ftp->inbuf; *ptr && !isdigit(*ptr); ptr++);
	n = sscanf(ptr, "%lu,%lu,%lu,%lu,%lu,%lu",
		&b[0], &b[1], &b[2], &b[3], &b[4], &b[5]);
	if (n != 6)
		return 0;

	for (n=0; n<6; n++)
		ipbox.c[n] = b[n];

	memset(&ftp->pasvaddr, 0, sizeof(ftp->pasvaddr));
	ftp->pasvaddr.sin_family = AF_INET;
	ftp->pasvaddr.sin_addr.s_addr = ipbox.l[0];
	ftp->pasvaddr.sin_port = ipbox.s[2];

	ftp->pasv = 2;

	return 1;
}


int
ftp_get(ftpbuf_t *ftp, FILE *outfp, const char *path, ftptype_t type)
{
	databuf_t		*data = NULL;
	char			*ptr;
	int			lastch;
	int			rcvd;

	if (ftp == NULL)
		return 0;

	if (!ftp_type(ftp, type))
		goto bail;

	if ((data = ftp_getdata(ftp)) == NULL)
		goto bail;

	if (!ftp_putcmd(ftp, "RETR", path))
		goto bail;
	if (!ftp_getresp(ftp) || (ftp->resp != 150 && ftp->resp != 125))
		goto bail;

	if ((data = data_accept(data)) == NULL)
		goto bail;

	lastch = 0;
	while ((rcvd = my_recv(data->fd, data->buf, FTP_BUFSIZE))) {
		if (rcvd == -1)
			goto bail;

		if (type == FTPTYPE_ASCII) {
			for (ptr = data->buf; rcvd; rcvd--, ptr++) {
				if (lastch == '\r' && *ptr != '\n')
					putc('\r', outfp);
				if (*ptr != '\r')
					putc(*ptr, outfp);
				lastch = *ptr;
			}
		}
		else {
			fwrite(data->buf, rcvd, 1, outfp);
		}
	}

	if (type == FTPTYPE_ASCII && lastch == '\r')
		putc('\r', outfp);

	data = data_close(data);

	if (ferror(outfp))
		goto bail;

	if (!ftp_getresp(ftp) || ftp->resp != 226)
		goto bail;

	return 1;
bail:
	data_close(data);
	return 0;
}


int
ftp_put(ftpbuf_t *ftp, const char *path, FILE *infp, ftptype_t type)
{
	databuf_t		*data = NULL;
	int			size;
	char			*ptr;
	int			ch;

	if (ftp == NULL)
		return 0;

	if (!ftp_type(ftp, type))
		goto bail;

	if ((data = ftp_getdata(ftp)) == NULL)
		goto bail;

	if (!ftp_putcmd(ftp, "STOR", path))
		goto bail;
	if (!ftp_getresp(ftp) || (ftp->resp != 150 && ftp->resp != 125))
		goto bail;

	if ((data = data_accept(data)) == NULL)
		goto bail;

	size = 0;
	ptr = data->buf;
	while ((ch = getc(infp)) != EOF) {
		/* flush if necessary */
		if (FTP_BUFSIZE - size < 2) {
			if (my_send(data->fd, data->buf, size) != size)
				goto bail;
			ptr = data->buf;
			size = 0;
		}

		if (ch == '\n' && type == FTPTYPE_ASCII) {
			*ptr++ = '\r';
			size++;
		}

		*ptr++ = ch;
		size++;
	}

	if (size && my_send(data->fd, data->buf, size) != size)
		goto bail;

	if (ferror(infp))
		goto bail;

	data = data_close(data);

	if (!ftp_getresp(ftp) || ftp->resp != 226)
		goto bail;

	return 1;
bail:
	data_close(data);
	return 0;
}


int
ftp_size(ftpbuf_t *ftp, const char *path)
{
	if (ftp == NULL)
		return -1;

	if (!ftp_putcmd(ftp, "SIZE", path))
		return -1;
	if (!ftp_getresp(ftp) || ftp->resp != 213)
		return -1;

	return atoi(ftp->inbuf);
}


time_t
ftp_mdtm(ftpbuf_t *ftp, const char *path)
{
	time_t		stamp;
	struct tm	*gmt;
	struct tm	tm;
	char		*ptr;
	int		n;

	if (ftp == NULL)
		return -1;

	if (!ftp_putcmd(ftp, "MDTM", path))
		return -1;
	if (!ftp_getresp(ftp) || ftp->resp != 213)
		return -1;

	/* parse out the timestamp */
	for (ptr = ftp->inbuf; *ptr && !isdigit(*ptr); ptr++);
	n = sscanf(ptr, "%4u%2u%2u%2u%2u%2u",
		&tm.tm_year, &tm.tm_mon, &tm.tm_mday,
		&tm.tm_hour, &tm.tm_min, &tm.tm_sec);
	if (n != 6)
		return -1;
	tm.tm_year -= 1900;
	tm.tm_mon--;
	tm.tm_isdst = -1;

	/* figure out the GMT offset */
	stamp = time(NULL);
	gmt = gmtime(&stamp);
	gmt->tm_isdst = -1;

	/* apply the GMT offset */
	tm.tm_sec += stamp - mktime(gmt);
	tm.tm_isdst = gmt->tm_isdst;

	stamp = mktime(&tm);

	return stamp;
}


int
ftp_delete(ftpbuf_t *ftp, const char *path)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "DELE", path))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 250)
		return 0;

	return 1;
}


int
ftp_rename(ftpbuf_t *ftp, const char *src, const char *dest)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "RNFR", src))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 350)
		return 0;

	if (!ftp_putcmd(ftp, "RNTO", dest))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp != 250)
		return 0;

	return 1;
}


int
ftp_site(ftpbuf_t *ftp, const char *cmd)
{
	if (ftp == NULL)
		return 0;

	if (!ftp_putcmd(ftp, "SITE", cmd))
		return 0;
	if (!ftp_getresp(ftp) || ftp->resp < 200 || ftp->resp >= 300)
		return 0;

	return 1;
}

/* static functions */

int
ftp_putcmd(ftpbuf_t *ftp, const char *cmd, const char *args)
{
	int		size;
	char		*data;

	/* build the output buffer */
	if (args) {
		/* "cmd args\r\n\0" */
		if (strlen(cmd) + strlen(args) + 4 > FTP_BUFSIZE)
			return 0;
		size = sprintf(ftp->outbuf, "%s %s\r\n", cmd, args);
	}
	else {
		/* "cmd\r\n\0" */
		if (strlen(cmd) + 3 > FTP_BUFSIZE)
			return 0;
		size = sprintf(ftp->outbuf, "%s\r\n", cmd);
	}

	data = ftp->outbuf;
	if (my_send(ftp->fd, data, size) != size)
		return 0;

	return 1;
}


int
ftp_readline(ftpbuf_t *ftp)
{
	int		size, rcvd;
	char		*data, *eol;

	/* shift the extra to the front */
	size = FTP_BUFSIZE;
	rcvd = 0;
	if (ftp->extra) {
		memmove(ftp->inbuf, ftp->extra, ftp->extralen);
		rcvd = ftp->extralen;
	}

	data = ftp->inbuf;

	do {
		size -= rcvd;
		for (eol = data; rcvd; rcvd--, eol++) {
			if (*eol == '\r') {
				*eol = 0;
				ftp->extra = eol + 1;
				if (rcvd > 1 && *(eol + 1) == '\n') {
					ftp->extra++;
					rcvd--;
				}
				if ((ftp->extralen = --rcvd) == 0)
					ftp->extra = NULL;
				return 1;
			}
			else if (*eol == '\n') {
				*eol = 0;
				ftp->extra = eol + 1;
				if ((ftp->extralen = --rcvd) == 0)
					ftp->extra = NULL;
				return 1;
			}
		}

		data = eol;
		if ((rcvd = my_recv(ftp->fd, data, size)) < 1)
			return 0;
	} while (size);

	return 0;
}


int
ftp_getresp(ftpbuf_t *ftp)
{
	char		*buf;

	if (ftp == NULL)
		return 0;
	buf = ftp->inbuf;
	ftp->resp = 0;


	while (1) {
		if (!ftp_readline(ftp))
			return 0;
		if (ftp->inbuf[3] == '-')
			continue;
		else if (ftp->inbuf[3] != ' ')
			return 0;
		break;
	}

	/* translate the tag */
	if (	!isdigit(ftp->inbuf[0]) ||
		!isdigit(ftp->inbuf[1]) ||
		!isdigit(ftp->inbuf[2]))
	{
		return 0;
	}

	ftp->resp =	100 * (ftp->inbuf[0] - '0') +
			10 * (ftp->inbuf[1] - '0') +
			(ftp->inbuf[2] - '0');

	memmove(ftp->inbuf, ftp->inbuf + 4, FTP_BUFSIZE - 4);

	return 1;
}


int
my_send(int s, void *buf, size_t len)
{
	fd_set		write_set;
	struct timeval	tv;
	int		n, size, sent;

	size = len;
	while (size) {
		tv.tv_sec = FTP_TIMEOUT;
		tv.tv_usec = 0;

		FD_ZERO(&write_set);
		FD_SET(s, &write_set);
		n = select(s + 1, NULL, &write_set, NULL, &tv);
		if (n < 1) {
			if (n == 0)
#if !(WIN32|WINNT)
				errno = ETIMEDOUT;
#endif
			return -1;
		}

		sent = send(s, buf, size, 0);
		if (sent == -1)
			return -1;

		buf = (char*) buf + sent;
		size -= sent;
	}

	return len;
}


int
my_recv(int s, void *buf, size_t len)
{
	fd_set		read_set;
	struct timeval	tv;
	int		n;

	tv.tv_sec = FTP_TIMEOUT;
	tv.tv_usec = 0;

	FD_ZERO(&read_set);
	FD_SET(s, &read_set);
	n = select(s + 1, &read_set, NULL, NULL, &tv);
	if (n < 1) {
		if (n == 0)
#if !(WIN32|WINNT)
			errno = ETIMEDOUT;
#endif
		return -1;
	}

	return recv(s, buf, len, 0);
}


int
my_connect(int s, const struct sockaddr *addr, int addrlen)
#if !defined(WIN32)
{
	fd_set		conn_set;
	int		flags;
	int		n;
	int		error = 0;
	struct timeval	tv;

	flags = fcntl(s, F_GETFL, 0);
	fcntl(s, F_SETFL, flags | O_NONBLOCK);

	n = connect(s, addr, addrlen);
	if (n == -1 && errno != EINPROGRESS)
		return -1;

	if (n) {
		FD_ZERO(&conn_set);
		FD_SET(s, &conn_set);

		tv.tv_sec = FTP_TIMEOUT;
		tv.tv_usec = 0;

		n = select(s + 1, &conn_set, &conn_set, NULL, &tv);
		if (n < 1) {
			if (n == 0)
				errno = ETIMEDOUT;
			return -1;
		}

		if (FD_ISSET(s, &conn_set)) {
			n = sizeof(error);
			n = getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &n);
			if (n == -1 || error) {
				if (error)
					errno = error;
				return -1;
			}
		}
	}

	fcntl(s, F_SETFL, flags);

	return 0;
}
#else
{
	return connect(s, addr, addrlen);
}
#endif


int
my_accept(int s, struct sockaddr *addr, int *addrlen)
{
	fd_set		accept_set;
	struct timeval	tv;
	int		n;

	tv.tv_sec = FTP_TIMEOUT;
	tv.tv_usec = 0;
	FD_ZERO(&accept_set);
	FD_SET(s, &accept_set);

	n = select(s + 1, &accept_set, NULL, NULL, &tv);
	if (n < 1) {
		if (n == 0)
#if !(WIN32|WINNT)
			errno = ETIMEDOUT;
#endif
		return -1;
	}

	return accept(s, addr, addrlen);
}


databuf_t*
ftp_getdata(ftpbuf_t *ftp)
{
	int			fd = -1;
	databuf_t		*data;
	struct sockaddr_in	addr;
	int			size;
	union ipbox		ipbox;
	char			arg[sizeof("255,255,255,255,255,255")];


	/* ask for a passive connection if we need one */
	if (ftp->pasv && !ftp_pasv(ftp, 1))
		return NULL;

	/* alloc the data structure */
	data = calloc(1, sizeof(*data));
	if (data == NULL) {
		perror("calloc");
		return NULL;
	}
	data->listener = -1;
	data->fd = -1;
	data->type = ftp->type;

	/* bind/listen */
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		goto bail;
	}

	/* passive connection handler */
	if (ftp->pasv) {
		/* clear the ready status */
		ftp->pasv = 1;

		/* connect */
		if (my_connect(fd, (struct sockaddr*) &ftp->pasvaddr,
			sizeof(ftp->pasvaddr)) == -1)
		{
			perror("connect");
			close(fd);
			free(data);
			return NULL;
		}

		data->fd = fd;

		return data;
	}


	/* active (normal) connection */

	/* bind to a local address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;

	if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("bind");
		goto bail;
	}

	size = sizeof(addr);
	if (getsockname(fd, (struct sockaddr*) &addr, &size) == -1) {
		perror("getsockname");
		goto bail;
	}

	if (listen(fd, 5) == -1) {
		perror("listen");
		goto bail;
	}

	data->listener = fd;

	/* send the PORT */
	ipbox.l[0] = ftp->localaddr.s_addr;
	ipbox.s[2] = addr.sin_port;
	sprintf(arg, "%u,%u,%u,%u,%u,%u",
		ipbox.c[0], ipbox.c[1], ipbox.c[2], ipbox.c[3],
		ipbox.c[4], ipbox.c[5]);

	if (!ftp_putcmd(ftp, "PORT", arg))
		goto bail;
	if (!ftp_getresp(ftp) || ftp->resp != 200)
		goto bail;

	return data;

bail:
	if (fd != -1)
		close(fd);
	free(data);
	return NULL;
}


databuf_t*
data_accept(databuf_t *data)
{
	struct sockaddr_in	addr;
	int			size;

	if (data->fd != -1)
		return data;

	size = sizeof(addr);
	data->fd = my_accept(data->listener, (struct sockaddr*) &addr, &size);
	close(data->listener);
	data->listener = -1;

	if (data->fd == -1) {
		free(data);
		return NULL;
	}

	return data;
}


databuf_t*
data_close(databuf_t *data)
{
	if (data == NULL)
		return NULL;
	if (data->listener != -1)
		close(data->listener);
	if (data->fd != -1)
		close(data->fd);
	free(data);
	return NULL;
}


char**
ftp_genlist(ftpbuf_t *ftp, const char *cmd, const char *path)
{
	FILE		*tmpfp = NULL;
	databuf_t	*data = NULL;
	char		*ptr;
	int		ch, lastch;
	int		size, rcvd;
	int		lines;
	char		**ret = NULL;
	char		**entry;
	char		*text;


	if ((tmpfp = tmpfile()) == NULL)
		return NULL;

	if (!ftp_type(ftp, FTPTYPE_ASCII))
		goto bail;

	if ((data = ftp_getdata(ftp)) == NULL)
		goto bail;

	if (!ftp_putcmd(ftp, cmd, path))
		goto bail;
	if (!ftp_getresp(ftp) || (ftp->resp != 150 && ftp->resp != 125))
		goto bail;

	/* pull data buffer into tmpfile */
	if ((data = data_accept(data)) == NULL)
		goto bail;

	size = 0;
	lines = 0;
	lastch = 0;
	while ((rcvd = my_recv(data->fd, data->buf, FTP_BUFSIZE))) {
		if (rcvd == -1)
			goto bail;

		fwrite(data->buf, rcvd, 1, tmpfp);

		size += rcvd;
		for (ptr = data->buf; rcvd; rcvd--, ptr++) {
			if (*ptr == '\n' && lastch == '\r')
				lines++;
			else
				size++;
			lastch = *ptr;
		}
	}

	data = data_close(data);

	if (ferror(tmpfp))
		goto bail;



	rewind(tmpfp);

	ret = malloc((lines + 1) * sizeof(char**) + size * sizeof(char*));
	if (ret == NULL) {
		perror("malloc");
		goto bail;
	}

	entry = ret;
	text = (char*) (ret + lines + 1);
	*entry = text;
	lastch = 0;
	while ((ch = getc(tmpfp)) != EOF) {
		if (ch == '\n' && lastch == '\r') {
			*(text - 1) = 0;
			*++entry = text;
		}
		else {
			*text++ = ch;
		}
		lastch = ch;
	}
	*entry = NULL;

	if (ferror(tmpfp))
		goto bail;

	fclose(tmpfp);

	if (!ftp_getresp(ftp) || ftp->resp != 226) {
		free(ret);
		return NULL;
	}

	return ret;
bail:
	data_close(data);
	fclose(tmpfp);
	free(ret);
	return NULL;
}

#endif /* HAVE_FTP */
