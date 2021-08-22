/* $Id: sendmail.c,v 1.30 2000/08/11 15:08:49 fmk Exp $ */

/* 
 *    PHP Sendmail for Windows.
 *
 *  This file is rewriten specificly for PHPFI.  Some functionality
 *  has been removed (MIME and file attachments).  This code was 
 *  modified from code based on code writen by Jarle Aase.
 *
 *  This class is based on the original code by Jarle Aase, see bellow:
 *  wSendmail.cpp  It has been striped of some functionality to match
 *  the requirements of phpfi.
 *
 *  Very simple SMTP Send-mail program for sending command-line level
 *  emails and CGI-BIN form response for the Windows platform.
 *
 *  The complete wSendmail package with source code can be downloaded
 *  from http://virtual.icr.com.au:80/jgaa/cgi-bin.htm
 *
 */
#include "php.h"				/*php specific */
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "time.h"
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <winbase.h>
#include "sendmail.h"


/*
   extern int _daylight;
   extern long _timezone;
 */
/*enum
   {
   DO_CONNECT = WM_USER +1
   };
 */

static char *days[] =
{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char *months[] =
{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char Buffer[MAIL_BUFFER_SIZE];

// socket related data
SOCKET sc;
WSADATA Data;
struct hostent *adr;
SOCKADDR_IN sock_in;
int WinsockStarted;
// values set by the constructor
char *AppName;
char MailHost[HOST_NAME_LEN];
char LocalHost[HOST_NAME_LEN];
char seps[] = " ,\t\n";
char *php_mailer = "PHP 3.0 WIN32";

char *get_header(char *h, char *headers);

// Error messages
static char *ErrorMessages[] =
{
	{"Success"},
	{"Bad arguments from form"},
	{"Unable to open temporary mailfile for read"},
	{"Failed to Start Sockets"},
	{"Failed to Resolve Host"},
	{"Failed to obtain socket handle"},
	{"Failed to Connect"},
	{"Failed to Send"},
	{"Failed to Receive"},
	{"Server Error"},
	{"Failed to resolve the host IP name"},
	{"Out of memory"},
	{"Unknown error"},
	{"Bad Message Contents"},
	{"Bad Message Subject"},
	{"Bad Message destination"},
	{"Bad Message Return Path"},
	{"Bad Mail Host"},
	{"Bad Message File"},
	{"PHP Internal error: php3.ini sendmail from variable not set!"}
};



//********************************************************************
// Name:  TSendMail
// Input:   1) host:    Name of the mail host where the SMTP server resides
//                      max accepted length of name = 256
//          2) appname: Name of the application to use in the X-mailer
//                      field of the message. if NULL is given the application
//                      name is used as given by the GetCommandLine() function
//                      max accespted length of name = 100
// Output:  1) error:   Returns the error code if something went wrong or
//                      SUCCESS otherwise.
//
//  See SendText() for additional args!
//********************************************************************
int TSendMail(char *host, int *error,
			  char *headers, char *Subject, char *mailTo, char *data)
{
	int ret;
	char *RPath = NULL;
	TLS_VARS;

	GLOBAL(WinsockStarted) = FALSE;

	if (host == NULL) {
		*error = BAD_MAIL_HOST;
		return BAD_MAIL_HOST;
	} else if (strlen(host) >= HOST_NAME_LEN) {
		*error = BAD_MAIL_HOST;
		return BAD_MAIL_HOST;
	} else {
		strcpy(GLOBAL(MailHost), host);
	}

	if (php3_ini.sendmail_from){
		RPath = estrdup(php3_ini.sendmail_from);
		} else {
			return 19;
	}

	// attempt to connect with mail host
	*error = MailConnect();
	if (*error != 0) {
		if(RPath)efree(RPath);
		return *error;
	} else {
		ret = SendText(RPath, Subject, mailTo, data, headers);
		TSMClose();
		if (ret != SUCCESS) {
			*error = ret;
		}
		if(RPath)efree(RPath);
		return ret;
	}
}

//********************************************************************
// Name:  TSendMail::~TSendMail
// Input:
// Output:
// Description: DESTRUCTOR
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
void TSMClose()
{
	TLS_VARS;

	Post("QUIT\r\n");
	Ack();
	// to guarantee that the cleanup is not made twice and
	// compomise the rest of the application if sockets are used
	// elesewhere
}


//********************************************************************
// Name:  char *GetSMErrorText
// Input:   Error index returned by the menber functions
// Output:  pointer to a string containing the error description
// Description:
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
char *GetSMErrorText(int index)
{

	if ((index > MAX_ERROR_INDEX) || (index < MIN_ERROR_INDEX))
		return (ErrorMessages[UNKNOWN_ERROR]);
	else
		return (ErrorMessages[index]);
}


//********************************************************************
// Name:  TSendText
// Input:       1) RPath:   return path of the message
//                                  Is used to fill the "Return-Path" and the
//                                  "X-Sender" fields of the message.
//                  2) Subject: Subject field of the message. If NULL is given
//                                  the subject is set to "No Subject"
//                  3) mailTo:  Destination address
//                  4) data:        Null terminated string containing the data to be send.
// Output:      Error code or SUCCESS
// Description:
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
int SendText(char *RPath, char *Subject, char *mailTo, char *data, char *headers)
{

	int res, i;
	char *p;
	char *tempMailTo, *token, *pos1, *pos2;
	TLS_VARS;

	// check for NULL parameters
	if (data == NULL)
		return (BAD_MSG_CONTENTS);
	if (mailTo == NULL)
		return (BAD_MSG_DESTINATION);
	if (RPath == NULL)
		return (BAD_MSG_RPATH);

	// simple checks for the mailto address
	// have ampersand ?
	if (strchr(mailTo, '@') == NULL)
		return (BAD_MSG_DESTINATION);

	sprintf(GLOBAL(Buffer), "HELO %s\r\n", GLOBAL(LocalHost));

	// in the beggining of the dialog
	// attempt reconnect if the first Post fail
	if ((res = Post(GLOBAL(Buffer))) != SUCCESS) {
		MailConnect();
		if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
			return (res);
	}
	if ((res = Ack()) != SUCCESS)
		return (res);

	sprintf(GLOBAL(Buffer), "MAIL FROM:<%s>\r\n", RPath);
	if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
		return (res);
	if ((res = Ack()) != SUCCESS)
		return (res);

	tempMailTo = estrdup(mailTo);
	
	// Send mail to all rcpt's
	token = strtok(tempMailTo, ",");
	while(token != NULL)
	{
		sprintf(GLOBAL(Buffer), "RCPT TO:<%s>\r\n", token);
		if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
			return (res);
		if ((res = Ack()) != SUCCESS)
			return (res);
		token = strtok(NULL, ",");
	}

	// Send mail to all Cc rcpt's
	efree(tempMailTo);
	if (headers && (pos1 = strstr(headers, "Cc:"))) {
		pos2 = strstr(pos1, "\r\n");
		tempMailTo = estrndup(pos1, pos2-pos1);

		token = strtok(tempMailTo, ",");
		while(token != NULL)
		{
			sprintf(GLOBAL(Buffer), "RCPT TO:<%s>\r\n", token);
			if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
				return (res);
			if ((res = Ack()) != SUCCESS)
				return (res);
			token = strtok(NULL, ",");
		}
		efree(tempMailTo);
	}

	if ((res = Post("DATA\r\n")) != SUCCESS)
		return (res);
	if ((res = Ack()) != SUCCESS)
		return (res);


	// send message header
	if (Subject == NULL)
		res = PostHeader(RPath, "No Subject", mailTo, headers);
	else
		res = PostHeader(RPath, Subject, mailTo, headers);
	if (res != SUCCESS)
		return (res);


	// send message contents in 1024 chunks
	if (strlen(data) <= 1024) {
		if ((res = Post(data)) != SUCCESS)
			return (res);
	} else {
		p = data;
		while (1) {
			if (*p == '\0')
				break;
			if (strlen(p) >= 1024)
				i = 1024;
			else
				i = strlen(p);

			// put next chunk in buffer
			strncpy(GLOBAL(Buffer), p, i);
			GLOBAL(Buffer)[i] = '\0';
			p += i;

			// send chunk
			if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
				return (res);
		}
	}

	//send termination dot
	if ((res = Post("\r\n.\r\n")) != SUCCESS)
		return (res);
	if ((res = Ack()) != SUCCESS)
		return (res);

	return (SUCCESS);
}



//********************************************************************
// Name:  PostHeader
// Input:       1) return path
//                  2) Subject
//                  3) destination address
//                  4) DoMime flag
// Output:      Error code or Success
// Description:
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
int PostHeader(char *RPath, char *Subject, char *mailTo, char *xheaders)
{

	// Print message header according to RFC 822
	// Return-path, Received, Date, From, Subject, Sender, To, cc

	time_t tNow = time(NULL);
	struct tm *tm = localtime(&tNow);
	int zoneh = abs(_timezone);
	int zonem, res;
	char *p;
	TLS_VARS;

	p = GLOBAL(Buffer);
	zoneh /= (60 * 60);
	zonem = (abs(_timezone) / 60) - (zoneh * 60);

	if(!xheaders || !strstr(xheaders, "Date:")){
		p += sprintf(p, "Date: %s, %02d %s %04d %02d:%02d:%02d %s%02d%02d\r\n",
					 days[tm->tm_wday],
					 tm->tm_mday,
					 months[tm->tm_mon],
					 tm->tm_year + 1900,
					 tm->tm_hour,
					 tm->tm_min,
					 tm->tm_sec,
					 (_timezone > 0) ? "+" : (_timezone < 0) ? "-" : "",
					 zoneh,
					 zonem);
	}

	if(!xheaders || !strstr(xheaders, "From:")){
		p += sprintf(p, "From: %s\r\n", RPath);
	}
	p += sprintf(p, "Subject: %s\r\n", Subject);
	if(!xheaders || !strstr(xheaders, "To:")){
		p += sprintf(p, "To: %s\r\n", mailTo);
	}
	if(xheaders){
		p += sprintf(p, "%s\r\n", xheaders);
	}

	if ((res = Post(GLOBAL(Buffer))) != SUCCESS)
		return (res);

	if ((res = Post("\r\n")) != SUCCESS)
		return (res);

	return (SUCCESS);
}



//********************************************************************
// Name:  MailConnect
// Input:   None
// Output:  None
// Description: Connect to the mail host and receive the welcome message.
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
int MailConnect()
{

	int res;
	TLS_VARS;


	// Create Socket
	if ((GLOBAL(sc) = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return (FAILED_TO_OBTAIN_SOCKET_HANDLE);

	// Get our own host name
	if (gethostname(GLOBAL(LocalHost), HOST_NAME_LEN))
		return (FAILED_TO_GET_HOSTNAME);

	// Resolve the servers IP
	//if (!isdigit(GLOBAL(MailHost)[0])||!gethostbyname(GLOBAL(MailHost)))
	//{
	//	return (FAILED_TO_RESOLVE_HOST);
	//}

	// Connect to server
	GLOBAL(sock_in).sin_family = AF_INET;
	GLOBAL(sock_in).sin_port = htons(25);
	GLOBAL(sock_in).sin_addr.S_un.S_addr = GetAddr(GLOBAL(MailHost));

	if (connect(GLOBAL(sc), (LPSOCKADDR) & GLOBAL(sock_in), sizeof(GLOBAL(sock_in))))
		return (FAILED_TO_CONNECT);

	// receive Server welcome message
	res = Ack();
	return (res);
}






//********************************************************************
// Name:  Post
// Input:
// Output:
// Description:
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
int Post(LPCSTR msg)
{
	int len = strlen(msg);
	int slen;
	int index = 0;
	TLS_VARS;

	while (len > 0) {
		if ((slen = send(GLOBAL(sc), msg + index, len, 0)) < 1)
			return (FAILED_TO_SEND);
		len -= slen;
		index += slen;
	}
	return (SUCCESS);
}



//********************************************************************
// Name:  Ack
// Input:
// Output:
// Description:
// Get the response from the server. We only want to know if the
// last command was successful.
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
int Ack()
{
	static char *buf;
	int rlen;
	int Index = 0;
	int Received = 0;
	TLS_VARS;

	if (!buf)
		if ((buf = (char *) malloc(1024 * 4)) == NULL)
			return (OUT_OF_MEMORY);

  again:

	if ((rlen = recv(GLOBAL(sc), buf + Index, ((1024 * 4) - 1) - Received, 0)) < 1)
		return (FAILED_TO_RECEIVE);

	Received += rlen;
	buf[Received] = 0;
	//err_msg   fprintf(stderr,"Received: (%d bytes) %s", rlen, buf + Index);

	// Check for newline
	Index += rlen;
	if ((buf[Received - 2] != '\r') || (buf[Received - 1] != '\n'))
		// err_msg          fprintf(stderr,"Incomplete server message. Awaiting CRLF\n");
		goto again;				// Incomplete data. Line must be terminated by CRLF

	if (buf[0] > '3')
		return (SMTP_SERVER_ERROR);

	return (SUCCESS);
}


//********************************************************************
// Name:  unsigned long GetAddr (LPSTR szHost)
// Input:
// Output:
// Description: Given a string, it will return an IP address.
//   - first it tries to convert the string directly
//   - if that fails, it tries o resolve it as a hostname
//
// WARNING: gethostbyname() is a blocking function
// Author/Date:  jcar 20/9/96
// History:
//********************************************************************
unsigned long GetAddr(LPSTR szHost)
{
	LPHOSTENT lpstHost;
	u_long lAddr = INADDR_ANY;

	/* check that we have a string */
	if (*szHost) {

		/* check for a dotted-IP address string */
		lAddr = inet_addr(szHost);

		/* If not an address, then try to resolve it as a hostname */
		if ((lAddr == INADDR_NONE) && (strcmp(szHost, "255.255.255.255"))) {

			lpstHost = gethostbyname(szHost);
			if (lpstHost) {		/* success */
				lAddr = *((u_long FAR *) (lpstHost->h_addr));
			} else {
				lAddr = INADDR_ANY;		/* failure */
			}
		}
	}
	return (lAddr);
}								/* end GetAddr() */
