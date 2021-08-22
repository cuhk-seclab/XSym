/*
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2000 PHP Development Team (See Credits file)      |
   +----------------------------------------------------------------------+
   | This program is free software; you can redistribute it and/or modify |
   | it under the terms of the GNU General Public License as published by |
   | the Free Software Foundation; either version 2 of the License, or    |
   | (at your option) any later version.                                  |
   |                                                                      |
   | This program is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY; without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
   | GNU General Public License for more details.                         |
   |                                                                      |
   | You should have received a copy of the GNU General Public License    |
   | along with this program; if not, write to the Free Software          |
   | Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   +----------------------------------------------------------------------+
 */


/* $Id: hg_comm.h,v 1.12 2000/01/01 04:44:09 sas Exp $ */

#ifndef _HG_COMM_H
#define _HG_COMM_H

#if HYPERWAVE

#include "hw_error.h"
#define HG_SERVER_PORT   418

#define F_DISTRIBUTED 0x80000000
#define F_COMPRESSED  0x40000000
#define F_VERSION     0x00003fff
#define VERSION       717L     /* 7.05 */

#define HEADER_LENGTH 12

#define STAT_COMMAND_STR  "stat"
#define WHO_COMMAND_STR   "who"
#define STAT_COMMAND      1
#define WHO_COMMAND       2

#define GETDOCBYANCHOR_MESSAGE        2
#define GETCHILDCOLL_MESSAGE          3
#define GETPARENT_MESSAGE             4
#define GETCHILDDOCCOLL_MESSAGE       5
#define GETOBJECT_MESSAGE             7
#define GETANCHORS_MESSAGE            8
#define GETOBJBYQUERY_MESSAGE         9
#define GETOBJBYQUERYCOLL_MESSAGE    10
#define OBJECTBYIDQUERY_MESSAGE      11
#define GETTEXT_MESSAGE              12
#define INSDOC_MESSAGE               14
#define INSCOLL_MESSAGE              17
#define GETSRCBYDEST_MESSAGE         19
#define MVCPDOCSCOLL_MESSAGE         22
#define MVCPCOLLSCOLL_MESSAGE        23
#define IDENTIFY_MESSAGE             24
#define READY_MESSAGE                25
#define COMMAND_MESSAGE              26
#define CHANGEOBJECT_MESSAGE         27
#define EDITTEXT_MESSAGE             28
#define GETANDLOCK_MESSAGE           29
#define UNLOCK_MESSAGE               30
#define INCOLLECTIONS_MESSAGE        31
#define INSERTOBJECT_MESSAGE         32
#define PIPEDOCUMENT_MESSAGE         36
#define DELETEOBJECT_MESSAGE         37
#define PUTDOCUMENT_MESSAGE          38
#define GETREMOTE_MESSAGE            39
#define GETREMOTECHILDREN_MESSAGE    40
#define HG_MAPID                     43
#define CHILDREN_MESSAGE             44
#define GETCGI_MESSAGE               45
#define PIPECGI_MESSAGE              46

#define HW_DEFAULT_LINK               0
#define HW_IMAGE_LINK                 1
#define HW_BACKGROUND_LINK            2
#define HW_INTAG_LINK                 3
#define HW_APPLET_LINK                4

#define COPY                          0
#define MOVE                          1
#define DOCUMENT                      0
#define COLLECTION                    1


#if WIN32|WINNT
# define SOCK_ERR INVALID_SOCKET
# define SOCK_CONN_ERR SOCKET_ERROR
# define HWSOCK_FCLOSE(s) closesocket(s)
#else
# define SOCK_ERR -1
# define SOCK_CONN_ERR -1
# define HWSOCK_FCLOSE(s) close(s)
#endif


/* Low error messages */
#define LE_MALLOC                    -1

typedef struct {
	int id;				/* object ID of anchor */
	int tanchor;		/* Type of anchor. Can be 1=Src, 2=Dest */
	int start;			/* start of anchor */
	int end;				/* end of anchor */
	char *nameanchor;			/* name tag attribute of destination document */
	/* if anchor is of type Src the following are used as well */
	char *destdocname;			/* name of destination document */
	char *link;			/* url for external destination */
	int linktype;		/* type of link. see above */
	char *tagattr;		/* more attributes of tag, like Border=0 */
	char *htmlattr;	/* */
	char *codebase;		/* codebase of applet */
	char *code;		/* code of applet */
	char *fragment;		/* name link of Src */

	/* if anchor is of type Dest the following are used as well */
	char *keyword;		/* name link of Dest */
	} ANCHOR;

typedef struct  {
     int  length;
     int  version_msgid;
     int  msg_type;
     char *buf;
} hg_msg;

typedef struct {
	int socket;
	int swap_on;
	int version;
	char *server_string;
        char *hostname;
        char *username;
	int lasterror;
	int linkroot;
} hw_connection;

typedef int hw_objectID;
typedef char hw_objrec;

extern void set_swap(int do_swap);
extern int  open_hg_connection(char *server_name, int port);
extern void close_hg_connection(int sockfd);
extern int initialize_hg_connection(int sockfd, int *do_swap, int *version, char **userdata, char **server_string, char *username, char *password);

extern int send_ready(int sockfd);
extern int send_command(int sockfd, int command, char **answer);

extern hg_msg *recv_hg_msg(int sockfd);
extern hg_msg *recv_ready(int sockfd);
extern hg_msg *recv_command(int sockfd);

extern char *fnInsStr(char *str, int pos, char *insstr);
extern int fnAttributeCompare(char *object, char *attrname, char *value);
extern int getrellink(int sockfd, int rootID, int thisID, int destID, char **reldesstr);

extern int send_deleteobject(int sockfd, hw_objectID objectID);
extern int send_changeobject(int sockfd, hw_objectID objectID, char *mod);
extern int send_groupchangeobject(int sockfd, hw_objectID objectID, char *mod);
extern int send_getobject(int sockfd, hw_objectID objectID, char **attributes);
extern int send_getandlock(int sockfd, hw_objectID objectID, char **attributes);
extern int send_lock(int sockfd, hw_objectID objectID);
extern int send_unlock(int sockfd, hw_objectID objectID);
extern int send_gettext(int sockfd, hw_objectID objectID, int mode, int rootid, char **objattr, char **bodytag, char **text, int *count, char *urlprefix);
extern int send_edittext(int sockfd, char *objattr, char *text);
extern int send_getcgi(int sockfd, hw_objectID objectID, char *cgi_env_str, char **objattr, char **text, int *count);
extern int send_getremote(int sockfd, hw_objectID objectID, char **objattr, char **text, int *count);
extern int send_getremotechildren(int sockfd, char *attributes, char **text, int **childIDs, int *count);
extern int send_docbyanchor(int sockfd, hw_objectID objectID, hw_objectID *anchorID);
extern int send_docbyanchorobj(int sockfd, hw_objectID objectID, char **objrec);
extern int send_mvcpdocscollscoll(int sockfd, hw_objectID *objectIDs, int count, int from, int dest, int cpmv, int docscoll);
extern int send_childrenobj(int sockfd, hw_objectID objectID, char ***childrec, int *count);
extern int send_getchildcoll(int sockfd, int objectID, hw_objectID **childIDs, int *count);
extern int send_getchildcollobj(int sockfd, hw_objectID objectID, hw_objrec ***childrec, int *count);
extern int send_getchilddoccoll(int sockfd, hw_objectID objectID, hw_objectID **childIDs, int *count);
extern int send_getchilddoccollobj(int sockfd, hw_objectID objectID, hw_objrec ***childrec, int *count);
extern int send_getanchors(int sockfd, hw_objectID objectID, hw_objectID **anchorIDs, int *count);
extern int send_getanchorsobj(int sockfd, hw_objectID objectID, char ***childrec, int *count);
extern int send_objectbyidquery(int sockfd, hw_objectID *IDs, int *count, char *query, char ***objrecs);
extern int send_getobjbyquery(int sockfd, char *query, int maxhits, hw_objectID **childIDs, int *count);
extern int send_getobjbyqueryobj(int sockfd, char *query, int maxhits, char ***childrec, int *count);
extern int send_getobjbyquerycoll(int sockfd, hw_objectID collID, char *query, int maxhits, hw_objectID **childIDs, int *count);
extern int send_getobjbyquerycollobj(int sockfd, hw_objectID collID, char *query, int maxhits, char ***childrec, int *count);
extern int send_identify(int sockfd, char *name, char *passwd, char **userdata);
extern int send_getparents(int sockfd, hw_objectID objectID, hw_objectID **childIDs, int *count);
extern int send_children(int sockfd, hw_objectID objectID, hw_objectID **childIDs, int *count);
extern int send_getparentsobj(int sockfd, hw_objectID objectID, char ***childrec, int *count);
extern int send_pipedocument(int sockfd, char *hostname, hw_objectID objectID, int mode, int rootid, char** objattr, char **bodytag, char **text, int *count, char *urlprefix);
extern int send_pipecgi(int sockfd, char *host, hw_objectID objectID, char *cgi_env_str, char **objattr, char **text, int *count);
extern int send_putdocument(int sockfd, char *hostname, hw_objectID parentID, char *objectRec, char *text, int count, hw_objectID *objectID);
extern int send_inscoll(int sockfd, hw_objectID objectID, char *objrec, hw_objectID *new_objectID);
extern int send_insertobject(int sockfd, char *objrec, char *parms, hw_objectID *objectID);
extern int send_insdoc(int sockfd, hw_objectID objectID, char *objrec, char *text, hw_objectID *new_objectID);
extern int send_incollections(int sockfd, int retcol, int cobjids, hw_objectID *objectIDs, int ccollids, hw_objectID *collIDs, int *count, hw_objectID **retIDs);
extern int send_getsrcbydest(int sockfd, hw_objectID objid, char ***childrec, int *count);
extern int send_mapid(int sockfd, int servid, hw_objectID id, int *virtid);
extern int send_dummy(int sockfd, hw_objectID objectID, int msgid, char **attributes);
extern char *get_hw_info(hw_connection *conn);

#define send_mvcpdocscoll(sockfd,objectIDs,count,from,dest,mvcp) \
		send_mvcpdocscollscoll(sockfd,objectIDs,count,from,dest,mvcp,DOCUMENT)
#define send_mvcpcollscoll(sockfd,objectIDs,count,from,dest,mvcp) \
		send_mvcpdocscollscoll(sockfd,objectIDs,count,from,dest,mvcp,COLLECTION)

#endif
#endif
