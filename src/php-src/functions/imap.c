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
   | Authors: Rex Logan           <veebert@dimensional.com>               |
   |          Mark Musone         <musone@chek.com>                  |
   |          Brian Wang          <brian@vividnet.com>                    |
   |          Kaj-Michael Lang    <milang@tal.org>                        |
   |          Antoni Pamies Olive <toni@readysoft.net>                    |
   |          Rasmus Lerdorf      <rasmus@lerdorf.on.ca>                  |
   |          Chuck Hagenbuch     <chagenbu@wso.williams.edu>             |
   |          Andrew Skalski      <askalski@chek.com>                  |
   +----------------------------------------------------------------------+
 */
/* $Id: imap.c,v 1.90 2000/09/30 17:32:44 sas Exp $ */

#define IMAP41

#ifdef ERROR
#undef ERROR
#endif

#if !(WIN32|WINNT)
#include "config.h"
#endif
#include "php.h"
#include "internal_functions.h"

#if COMPILE_DL
#include "dl/phpdl.h"
#endif

#if HAVE_IMAP

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include "php3_list.h"
#include "imap.h"
#include "mail.h"
#include "rfc822.h"
#include "utf8.h"
#include "modules.h"
#if (WIN32|WINNT)
#include "winsock.h"
#include "win32/imap_sendmail.h"
#endif

#ifdef IMAP41
#define LSIZE text.size
#define LTEXT text.data
#define DTYPE int
#define CONTENT_PART nested.part
#define CONTENT_MSG_BODY nested.msg->body
#define IMAPVER "Imap 4R1"
#else
#define LSIZE size
#define LTEXT text
#define DTYPE char
#define CONTENT_PART contents.part
#define CONTENT_MSG_BODY contents.msg.body
#define IMAPVER "Imap 4"
#endif

#define PHP_EXPUNGE 32768

/* type casts left out, put here to remove warnings in
   msvc
*/
extern void rfc822_date(char *date);
extern char *cpystr(const char *string);
extern unsigned long find_rightmost_bit (unsigned long *valptr);
extern void fs_give (void **block);
extern void *fs_get (size_t size);
int add_assoc_object(pval *arg, char *key, pval tmp);
int add_next_index_object(pval *arg, pval tmp);
void imap_add_body( pval *arg, BODY *body );
int _php3_imap_mail(char *to, char *subject, char *message, char *headers, char *cc, char *bcc, char *rpath);

typedef struct php3_imap_le_struct {
	MAILSTREAM *imap_stream;
	long flags;
#ifdef OP_RELOGIN
	/* AJS: busy flag for persistent connections, pointers for chaining */
	struct php3_imap_le_struct *next;
	struct php3_imap_le_struct **prev;
	char busy;
#endif
} pils;

typedef struct php3_imap_mailbox_struct {
	SIZEDTEXT text;
	DTYPE delimiter;
	long attributes;
	struct php3_imap_mailbox_struct *next;
} FOBJECTLIST;

typedef struct php3_imap_error_struct {
	SIZEDTEXT text;
	long errflg;
	struct php3_imap_error_struct *next;
} ERRORLIST;

typedef struct php3_imap_message_struct {
	unsigned long msgid;
	struct php3_imap_message_struct *next;
} MESSAGELIST;

void mail_close_it (pils *imap_le_struct);
#ifdef OP_RELOGIN
/* AJS: close persistent connection */
void mail_userlogout_it (pils *imap_le_struct);
void mail_nuke_chain (pils **headp);
#endif

/* 
 * this array should be set up as:
 * {"PHPScriptFunctionName",dllFunctionName,1} 
 */

function_entry imap_functions[] = {
	{"imap_open", php3_imap_open, NULL},
	{"imap_popen", php3_imap_popen, NULL},
	{"imap_reopen", php3_imap_reopen, NULL},
	{"imap_num_msg", php3_imap_num_msg, NULL},
	{"imap_num_recent", php3_imap_num_recent, NULL},
	{"imap_headers", php3_imap_headers, NULL},
	{"imap_header", php3_imap_headerinfo, NULL},
	{"imap_headerinfo", php3_imap_headerinfo, NULL},
	{"imap_body", php3_imap_body, NULL},
	{"imap_fetchstructure", php3_imap_fetchstructure, NULL},
	{"imap_fetchbody", php3_imap_fetchbody, NULL},
	{"imap_expunge", php3_imap_expunge, NULL},
	{"imap_delete", php3_imap_delete, NULL},
	{"imap_undelete", php3_imap_undelete, NULL},
	{"imap_check", php3_imap_check, NULL},
	{"imap_close", php3_imap_close, NULL},
	{"imap_mail_copy", php3_imap_mail_copy, NULL},
	{"imap_mail_move", php3_imap_mail_move, NULL},
	{"imap_createmailbox", php3_imap_createmailbox, NULL},
	{"imap_renamemailbox", php3_imap_renamemailbox, NULL},
	{"imap_deletemailbox", php3_imap_deletemailbox, NULL},
	{"imap_listmailbox", php3_imap_list, NULL},
	{"imap_getmailboxes", php3_imap_list_full, NULL},
	{"imap_scanmailbox", php3_imap_listscan, NULL},
	{"imap_listsubscribed", php3_imap_lsub, NULL},
	{"imap_getsubscribed", php3_imap_lsub_full, NULL},
	{"imap_subscribe", php3_imap_subscribe, NULL},
	{"imap_unsubscribe", php3_imap_unsubscribe, NULL},
	{"imap_append", php3_imap_append, NULL},
	{"imap_ping", php3_imap_ping, NULL},
	{"imap_base64", php3_imap_base64, NULL},
	{"imap_qprint", php3_imap_qprint, NULL},
	{"imap_8bit", php3_imap_8bit, NULL},
	{"imap_binary", php3_imap_binary, NULL},
	{"imap_mailboxmsginfo", php3_imap_mailboxmsginfo, NULL},
	{"imap_rfc822_write_address", php3_imap_rfc822_write_address, NULL},
	{"imap_rfc822_parse_adrlist", php3_imap_rfc822_parse_adrlist, NULL},
	{"imap_setflag_full", php3_imap_setflag_full, NULL},
	{"imap_clearflag_full", php3_imap_clearflag_full, NULL},
	{"imap_sort", php3_imap_sort, NULL},
	{"imap_fetchheader", php3_imap_fetchheader, NULL},
	{"imap_fetchtext", php3_imap_body, NULL},
	{"imap_uid", php3_imap_uid, NULL},
	{"imap_msgno",php3_imap_msgno, NULL},
	{"imap_list", php3_imap_list, NULL},
	{"imap_scan", php3_imap_listscan, NULL},
	{"imap_lsub", php3_imap_lsub, NULL},
	{"imap_create", php3_imap_createmailbox, NULL},
	{"imap_rename", php3_imap_renamemailbox, NULL},
	{"imap_status", php3_imap_status, NULL},
	{"imap_bodystruct", php3_imap_bodystruct, NULL},
	{"imap_fetch_overview", php3_imap_fetch_overview, NULL},
	{"imap_mail_compose", php3_imap_mail_compose, NULL},
	{"imap_alerts", php3_imap_alerts, NULL},
	{"imap_errors", php3_imap_errors, NULL},
	{"imap_last_error", php3_imap_last_error, NULL},
	{"imap_mail", php3_imap_mail, NULL},
	{"imap_search", php3_imap_search, NULL},
	{"imap_utf8", php3_imap_utf8, NULL},
	{"imap_utf7_decode", php3_imap_utf7_decode, NULL},
	{"imap_utf7_encode", php3_imap_utf7_encode, NULL},
	{"imap_mime_header_decode", php3_imap_mime_header_decode, NULL},
	{NULL, NULL, NULL}
};

#ifdef OP_RELOGIN
#define	IS_STREAM(ind_type)	((ind_type)==le_imap || (ind_type)==le_pimap)
#else
#define	IS_STREAM(ind_type)	((ind_type)==le_imap)
#endif

php3_module_entry php3_imap_module_entry = {
	IMAPVER, imap_functions, imap_init, NULL, imap_init_request, imap_end_request, imap_info, 0, 0, 0, NULL
};


#if COMPILE_DL
DLEXPORT php3_module_entry *get_module(void) { return &php3_imap_module_entry; }
#endif

/* Determines how mm_list() and mm_lsub() are to return their results. */
typedef enum {
	FLIST_ARRAY,
	FLIST_OBJECT
} folderlist_style_t;

/* 
   I believe since this global is used ONLY within this module,
   and nothing will link to this module, we can use the simple 
   thread local storage
*/
int le_imap;
#ifdef OP_RELOGIN
/* AJS: persistent connection type, chain pointer type */
int le_pimap;
int le_pimapchain;
#endif
char imap_user[80]="";
char imap_password[80]="";
#if HAVE_IMSP
extern char imsp_user[80];
extern char imsp_password[80];
#endif
STRINGLIST *imap_folders=NIL;
STRINGLIST *imap_sfolders=NIL;
STRINGLIST *imap_alertstack=NIL;
ERRORLIST *imap_errorstack=NIL;
MESSAGELIST *imap_messages=NIL;
FOBJECTLIST *imap_folder_objects=NIL;
FOBJECTLIST *imap_sfolder_objects=NIL;
folderlist_style_t folderlist_style = FLIST_ARRAY;
long status_flags;
unsigned long status_messages;
unsigned long status_recent;
unsigned long status_unseen;
unsigned long status_uidnext;
unsigned long status_uidvalidity;
#ifdef SA_QUOTA
unsigned long status_quota;
#endif
#ifdef SA_QUOTA_ALL
unsigned long status_quota_all;
#endif

void
mail_close_it (pils *imap_le_struct)
{
	mail_close_full (imap_le_struct->imap_stream,imap_le_struct->flags);
	efree(imap_le_struct);
}

#ifdef OP_RELOGIN
/* AJS: stream close functions for persistent connections */
void
mail_userlogout_it (pils *imap_le_struct)
{
	/* Close this user's session, putting the stream back
	 * into AUTHENTICATE state.  (Note that IMAP does not
	 * support this behavior... yet)
	 */
	imap_le_struct->busy = 0;
	mail_close_full(imap_le_struct->imap_stream,
			imap_le_struct->flags | CL_HALF);
}

void
mail_nuke_chain (pils **headp)
{
	pils		*node, *next;

	for (node = *headp; node; node = next) {
		next = node->next;
		mail_close(node->imap_stream);
		free(node);
	}

	free(headp);
}
#endif

inline int add_assoc_object(pval *arg, char *key, pval tmp)
{
	return _php3_hash_update(arg->value.ht, key, strlen(key)+1, (void *) &tmp, sizeof(pval), NULL);
}

inline int add_next_index_object(pval *arg, pval tmp)
{
	return _php3_hash_next_index_insert( arg->value.ht, (void *) &tmp, sizeof(pval), NULL); 
}


/* Mail instantiate FOBJECTLIST
 * Returns: new FOBJECTLIST list
 * Author: CJH
 */
FOBJECTLIST *mail_newfolderobjectlist (void) {
  return (FOBJECTLIST *) memset(fs_get(sizeof(FOBJECTLIST)),0,
				sizeof(FOBJECTLIST));
}


/* Mail garbage collect FOBJECTLIST
 * Accepts: pointer to FOBJECTLIST pointer
 * Author: CJH
 */
void mail_free_foblist (FOBJECTLIST **foblist)
{
  if (*foblist) {		/* only free if exists */
    if ((*foblist)->text.data) fs_give ((void **) &(*foblist)->text.data);
    mail_free_foblist (&(*foblist)->next);
    fs_give ((void **) foblist);	/* return string to free storage */
  }
}


/* Mail instantiate ERRORLIST
 * Returns: new ERRORLIST list
 * Author: CJH
 */
ERRORLIST *mail_newerrorlist (void) {
  return (ERRORLIST *) memset(fs_get(sizeof(ERRORLIST)),0,
			      sizeof(ERRORLIST));
}

/* Mail garbage collect FOBJECTLIST
 * Accepts: pointer to FOBJECTLIST pointer
 * Author: CJH
 */
void mail_free_errorlist (ERRORLIST **errlist)
{
  if (*errlist) {		/* only free if exists */
    if ((*errlist)->text.data) fs_give ((void **) &(*errlist)->text.data);
    mail_free_errorlist (&(*errlist)->next);
    fs_give ((void **) errlist);	/* return string to free storage */
  }
}

/* Mail instantiate MESSAGELIST
 * Returns: new MESSAGELIST list
 * Author: CJH
 */
MESSAGELIST *mail_newmessagelist (void)
{
	return (MESSAGELIST *) memset(fs_get(sizeof(MESSAGELIST)),0,
								  sizeof(MESSAGELIST));
}

/* Mail garbage collect MESSAGELIST
 * Accepts: pointer to MESSAGELIST pointer
 * Author: CJH
 */
void mail_free_messagelist (MESSAGELIST **msglist)
{
	if (*msglist) {		/* only free if exists */
		mail_free_messagelist (&(*msglist)->next);
		fs_give ((void **) msglist);	/* return string to free storage */
	}
}

/* Author: CJH */
int imap_init_request (INIT_FUNC_ARGS) {
  imap_errorstack = NIL;
  imap_alertstack = NIL;
  return SUCCESS;
}

/* Author: CJH */
int imap_end_request () {
  ERRORLIST *ecur = NIL;
  STRINGLIST *acur = NIL;
  
  if (imap_errorstack != NIL) {
    /* output any remaining errors at their original error level */
    ecur = imap_errorstack;
    while (ecur != NIL) {
      php3_error(E_WARNING, "errflg=%d, text=%s", ecur->errflg, ecur->LTEXT);
      ecur = ecur->next;
    }
    mail_free_errorlist(&imap_errorstack);
  }
  
  if (imap_alertstack != NIL) {
    /* output any remaining alerts at E_NOTICE level */
    acur = imap_alertstack;
    while (acur != NIL) {
      php3_error(E_NOTICE, "%s", acur->LTEXT);
      acur = acur->next;
    }
    mail_free_stringlist(&imap_alertstack);
  }
  
  return SUCCESS;
}

void imap_info(void)
{
	php3_printf("Imap Support enabled<br>");
	php3_printf("<table>");
	php3_printf("<tr><td>Imap c-client Version:</td>");
#ifdef IMAP41
	php3_printf("<td>Imap 4.1</td>");
#else
	php3_printf("<td>Imap 4.0</td>");
#endif
	php3_printf("</tr></table>");
}

int imap_init(INIT_FUNC_ARGS)
{
	unsigned long sa_all =	SA_MESSAGES | SA_RECENT | SA_UNSEEN |
				SA_UIDNEXT | SA_UIDVALIDITY;

#if !(WIN32|WINNT)
	mail_link(&unixdriver);   /* link in the unix driver */
#endif
	mail_link (&imapdriver);      /* link in the imap driver */
	mail_link (&nntpdriver);      /* link in the nntp driver */
	mail_link (&pop3driver);      /* link in the pop3 driver */
#if !(WIN32|WINNT)
	mail_link (&mhdriver);        /* link in the mh driver */
	mail_link (&mxdriver);        /* link in the mx driver */
#endif
	mail_link (&mbxdriver);       /* link in the mbx driver */
	mail_link (&tenexdriver);     /* link in the tenex driver */
	mail_link (&mtxdriver);       /* link in the mtx driver */
#if !(WIN32|WINNT)
	mail_link (&mmdfdriver);      /* link in the mmdf driver */
	mail_link (&newsdriver);      /* link in the news driver */
	mail_link (&philedriver);     /* link in the phile driver */
#endif
	mail_link (&dummydriver);     /* link in the dummy driver */
	auth_link (&auth_log);        /* link in the log authenticator */
	
	/* lets allow NIL */

        REGISTER_MAIN_LONG_CONSTANT("NIL", NIL, CONST_PERSISTENT | CONST_CS);


	/* Open Options */

        REGISTER_MAIN_LONG_CONSTANT("OP_DEBUG", OP_DEBUG, CONST_PERSISTENT | CONST_CS);
/* debug protocol negotiations */

        REGISTER_MAIN_LONG_CONSTANT("OP_READONLY", OP_READONLY, CONST_PERSISTENT | CONST_CS);
/* read-only open */

        REGISTER_MAIN_LONG_CONSTANT("OP_ANONYMOUS", OP_ANONYMOUS, CONST_PERSISTENT | CONST_CS);
	/* anonymous open of newsgroup */

        REGISTER_MAIN_LONG_CONSTANT("OP_SHORTCACHE", OP_SHORTCACHE, CONST_PERSISTENT | CONST_CS);

/* short (elt-only) caching */

        REGISTER_MAIN_LONG_CONSTANT("OP_SILENT", OP_SILENT, CONST_PERSISTENT | CONST_CS);
/* don't pass up events (internal use) */
        REGISTER_MAIN_LONG_CONSTANT("OP_PROTOTYPE", OP_PROTOTYPE, CONST_PERSISTENT | CONST_CS);
/* return driver prototype */
        REGISTER_MAIN_LONG_CONSTANT("OP_HALFOPEN", OP_HALFOPEN, CONST_PERSISTENT | CONST_CS);
/* half-open (IMAP connect but no select) */

        REGISTER_MAIN_LONG_CONSTANT("OP_EXPUNGE", OP_EXPUNGE, CONST_PERSISTENT | CONST_CS);
 /* silently expunge recycle stream */

       REGISTER_MAIN_LONG_CONSTANT("OP_SECURE", OP_SECURE, CONST_PERSISTENT | CONST_CS);
/* don't do non-secure authentication */

/* 
   PHP re-assigns CL_EXPUNGE a custom value that can be used as part of the imap_open() bitfield
   because it seems like a good idea to be able to indicate that the mailbox should be 
   automatically expunged during imap_open in case the script get interrupted and it doesn't get
   to the imap_close() where this option is normally placed.  If the c-client library adds other
   options and the value for this one conflicts, simply make PHP_EXPUNGE higher at the top of 
   this file 
*/
        REGISTER_MAIN_LONG_CONSTANT("CL_EXPUNGE", PHP_EXPUNGE, CONST_PERSISTENT | CONST_CS);
        /* expunge silently */


	/* Fetch options */

        REGISTER_MAIN_LONG_CONSTANT("FT_UID", FT_UID, CONST_PERSISTENT | CONST_CS);
         /* argument is a UID */
        REGISTER_MAIN_LONG_CONSTANT("FT_PEEK", FT_PEEK, CONST_PERSISTENT | CONST_CS);
       /* peek at data */
        REGISTER_MAIN_LONG_CONSTANT("FT_NOT", FT_NOT, CONST_PERSISTENT | CONST_CS);
        /* NOT flag for header lines fetch */
        REGISTER_MAIN_LONG_CONSTANT("FT_INTERNAL", FT_INTERNAL, CONST_PERSISTENT | CONST_CS);
    /* text can be internal strings */
        REGISTER_MAIN_LONG_CONSTANT("FT_PREFETCHTEXT", FT_PREFETCHTEXT, CONST_PERSISTENT | CONST_CS);
    /* IMAP prefetch text when fetching header */


	/* Flagging options */

        REGISTER_MAIN_LONG_CONSTANT("ST_UID", ST_UID, CONST_PERSISTENT | CONST_CS);
         /* argument is a UID sequence */
        REGISTER_MAIN_LONG_CONSTANT("ST_SILENT", ST_SILENT, CONST_PERSISTENT | CONST_CS);
     /* don't return results */
        REGISTER_MAIN_LONG_CONSTANT("ST_SET", ST_SET, CONST_PERSISTENT | CONST_CS);
     /* set vs. clear */

   /* Copy options */

        REGISTER_MAIN_LONG_CONSTANT("CP_UID", CP_UID, CONST_PERSISTENT | CONST_CS);
         /* argument is a UID sequence */
        REGISTER_MAIN_LONG_CONSTANT("CP_MOVE", CP_MOVE, CONST_PERSISTENT | CONST_CS);
        /* delete from source after copying */


   /* Search/sort options */

        REGISTER_MAIN_LONG_CONSTANT("SE_UID", SE_UID, CONST_PERSISTENT | CONST_CS);
         /* return UID */
        REGISTER_MAIN_LONG_CONSTANT("SE_FREE", SE_FREE, CONST_PERSISTENT | CONST_CS);
        /* free search program after finished */
        REGISTER_MAIN_LONG_CONSTANT("SE_NOPREFETCH", SE_NOPREFETCH, CONST_PERSISTENT | CONST_CS);
  /* no search prefetching */
        REGISTER_MAIN_LONG_CONSTANT("SO_FREE", SO_FREE, CONST_PERSISTENT | CONST_CS);
        /* free sort program after finished */
        REGISTER_MAIN_LONG_CONSTANT("SO_NOSERVER", SO_NOSERVER, CONST_PERSISTENT | CONST_CS);
   /* don't do server-based sort */

   /* Status options */

        REGISTER_MAIN_LONG_CONSTANT("SA_MESSAGES",SA_MESSAGES , CONST_PERSISTENT | CONST_CS);
    /* number of messages */
        REGISTER_MAIN_LONG_CONSTANT("SA_RECENT", SA_RECENT, CONST_PERSISTENT | CONST_CS);
      /* number of recent messages */
        REGISTER_MAIN_LONG_CONSTANT("SA_UNSEEN",SA_UNSEEN , CONST_PERSISTENT | CONST_CS);
      /* number of unseen messages */
        REGISTER_MAIN_LONG_CONSTANT("SA_UIDNEXT", SA_UIDNEXT, CONST_PERSISTENT | CONST_CS);
     /* next UID to be assigned */
        REGISTER_MAIN_LONG_CONSTANT("SA_UIDVALIDITY",SA_UIDVALIDITY , CONST_PERSISTENT | CONST_CS);
     /* UID validity value */
#ifdef SA_QUOTA
	sa_all |= SA_QUOTA;
        REGISTER_MAIN_LONG_CONSTANT("SA_QUOTA",SA_QUOTA , CONST_PERSISTENT | CONST_CS);
     /* Disk space taken up by mailbox. */
#endif
#ifdef SA_QUOTA_ALL
	sa_all |= SA_QUOTA_ALL;
        REGISTER_MAIN_LONG_CONSTANT("SA_QUOTA_ALL",SA_QUOTA_ALL , CONST_PERSISTENT | CONST_CS);
     /* Disk space taken up by all mailboxes owned by user. */
#endif
	REGISTER_MAIN_LONG_CONSTANT("SA_ALL", sa_all, CONST_PERSISTENT | CONST_CS);
     /* get all status information */
		
   /* Bits for mm_list() and mm_lsub() */

        REGISTER_MAIN_LONG_CONSTANT("LATT_NOINFERIORS",LATT_NOINFERIORS , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("LATT_NOSELECT", LATT_NOSELECT, CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("LATT_MARKED", LATT_MARKED, CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("LATT_UNMARKED",LATT_UNMARKED , CONST_PERSISTENT | CONST_CS);


   /* Sort functions */

        REGISTER_MAIN_LONG_CONSTANT("SORTDATE",SORTDATE , CONST_PERSISTENT | CONST_CS);
             /* date */
        REGISTER_MAIN_LONG_CONSTANT("SORTARRIVAL",SORTARRIVAL , CONST_PERSISTENT | CONST_CS);
          /* arrival date */
        REGISTER_MAIN_LONG_CONSTANT("SORTFROM",SORTFROM , CONST_PERSISTENT | CONST_CS);
            /* from */
        REGISTER_MAIN_LONG_CONSTANT("SORTSUBJECT",SORTSUBJECT , CONST_PERSISTENT | CONST_CS);
           /* subject */
        REGISTER_MAIN_LONG_CONSTANT("SORTTO",SORTTO , CONST_PERSISTENT | CONST_CS);
                /* to */
        REGISTER_MAIN_LONG_CONSTANT("SORTCC",SORTCC , CONST_PERSISTENT | CONST_CS);
              /* cc */
        REGISTER_MAIN_LONG_CONSTANT("SORTSIZE",SORTSIZE , CONST_PERSISTENT | CONST_CS);
              /* size */

        REGISTER_MAIN_LONG_CONSTANT("TYPETEXT",TYPETEXT , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEMULTIPART",TYPEMULTIPART , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEMESSAGE",TYPEMESSAGE , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEAPPLICATION",TYPEAPPLICATION , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEAUDIO",TYPEAUDIO , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEIMAGE",TYPEIMAGE , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEVIDEO",TYPEVIDEO , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("TYPEOTHER",TYPEOTHER , CONST_PERSISTENT | CONST_CS);
	/*      TYPETEXT                unformatted text
        TYPEMULTIPART           multiple part
        TYPEMESSAGE             encapsulated message
        TYPEAPPLICATION         application data
        TYPEAUDIO               audio
        TYPEIMAGE               static image (GIF, JPEG, etc.)
        TYPEVIDEO               video
        TYPEOTHER               unknown
	*/
        REGISTER_MAIN_LONG_CONSTANT("ENC7BIT",ENC7BIT , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("ENC8BIT",ENC8BIT , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("ENCBINARY",ENCBINARY , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("ENCBASE64",ENCBASE64, CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("ENCQUOTEDPRINTABLE",ENCQUOTEDPRINTABLE , CONST_PERSISTENT | CONST_CS);
        REGISTER_MAIN_LONG_CONSTANT("ENCOTHER",ENCOTHER , CONST_PERSISTENT | CONST_CS);
	/*
	  ENC7BIT                 7 bit SMTP semantic data
	  ENC8BIT                 8 bit SMTP semantic data
	  ENCBINARY               8 bit binary data
	  ENCBASE64               base-64 encoded data
	  ENCQUOTEDPRINTABLE      human-readable 8-as-7 bit data
	  ENCOTHER                unknown
	*/

    le_imap = register_list_destructors(mail_close_it,NULL);
#ifdef OP_RELOGIN
    /* AJS: destructors for persistent connections */
    le_pimap = register_list_destructors(mail_userlogout_it, NULL);
    le_pimapchain = register_list_destructors(NULL, mail_nuke_chain);
#endif
	return SUCCESS;
}

void php3_imap_do_open(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	pval *mailbox;
	pval *user;
	pval *passwd;
	pval *options;
	MAILSTREAM *imap_stream;
	pils *imap_le_struct;
	long flags=NIL;
	long cl_flags=NIL;
#ifdef OP_RELOGIN
	NETMBX netmbx;
	char *hashed_details = NULL;
	int hashed_details_length = 0;
#endif
	int ind;
        int myargc=ARG_COUNT(ht);

	
	if (myargc <3 || myargc >4 || getParameters(ht, myargc, &mailbox,&user,&passwd,&options) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string(mailbox);
	convert_to_string(user);
	convert_to_string(passwd);
	if(myargc ==4) {
		convert_to_long(options);
		flags = options->value.lval;
		if(flags & PHP_EXPUNGE) {
			cl_flags = CL_EXPUNGE;
			flags ^= PHP_EXPUNGE;
		}
	}
	strcpy(imap_user,user->value.str.val);
	strcpy(imap_password,passwd->value.str.val);

#ifdef OP_RELOGIN
	/* AJS: persistent connection handling */
	/* Cannot use a persistent connection if we cannot parse
	 * out the server's hostname.
	 */
	if (	persistent &&
		!mail_valid_net_parse(mailbox->value.str.val, &netmbx))
	{
		persistent = 0;
	}

	imap_stream = NIL;
	if (persistent) {
		list_entry	*le = NULL;
		list_entry	new_le;
		pils		**headp, *node;
		int		need_update = 0;

		hashed_details_length = sizeof("imap_") + strlen(netmbx.host);
		hashed_details = (char*) emalloc(hashed_details_length);
		sprintf(hashed_details, "imap_%s", netmbx.host);

		/* Check for an existing connection. */
		if (	(_php3_hash_find(plist,
					hashed_details,
					hashed_details_length,
					(void*) &le) == FAILURE) ||
			(le->type != le_pimapchain))
		{
			le = NULL;
		}

		/* Re-use existing connection if available. */
		node = NULL;
		headp = NULL;
		if (le) {
			headp = (pils**) le->ptr;

			/* find a non-busy connection */
			for (node=*headp; node; node=node->next)
				if (!node->busy)
					break;
		}

		/* If we found a node, do a relogin.  */
		if (node) {
			imap_stream = mail_open(
				node->imap_stream,
				mailbox->value.str.val,
				flags | OP_RELOGIN);
			if (imap_stream) {
				/* Ping the stream to see if it is
				 * still good. 
				 */
				if (!mail_ping(imap_stream)) {
					mail_close(imap_stream);
					imap_stream = NIL;
				}
			}
		}

		/* Get a fresh stream if we don't have one yet. */
		if (imap_stream == NIL) {
			/* Open a new connection. */
			imap_stream = mail_open(
				NIL,
				mailbox->value.str.val,
				flags | OP_RELOGIN);
		}

		/* Do we have a stream yet?  If not, bail. */
		if (imap_stream == NIL) {
			if (node) {
				/* unlink the node */
				if ((*node->prev = node->next))
					node->next->prev = node->prev;
				free(node);
				/* delete the hash entry if empty */
				if (*headp == NULL)
					_php3_hash_del(plist,
						hashed_details,
						hashed_details_length);
			}
			efree(hashed_details);
			RETURN_FALSE;
		}

		/* Allocate a new node if none. */
		if (node == NULL) {
			/* Alloc new hash entry. */
			node = malloc(sizeof(pils));
			if (node == NULL) {
				efree(hashed_details);
				RETURN_FALSE;
			}

			/* Allocate headp if it does not exist. */
			if (headp == NULL) {
				headp = calloc(1, sizeof(*headp));
				need_update = 1;
			}

			node->prev = headp;
			node->next = *headp;
			*headp = node;
		}

		/* Initialize the node. */
		node->busy = 1;
		node->imap_stream = imap_stream;
		node->flags = cl_flags;

		/* Update the hash. */
		new_le.type = le_pimapchain;
		new_le.ptr = headp;
		if (	need_update &&
			_php3_hash_update(plist, hashed_details,
				hashed_details_length, &new_le,
				sizeof(new_le), NULL) == FAILURE)
		{
			/* unlink and free the new node */
			if ((*node->prev = node->next))
				node->next->prev = node->prev;
			mail_close(node->imap_stream);
			free(node);

			free(headp);
			efree(hashed_details);
			RETURN_FALSE;
		}

		efree(hashed_details);

		imap_le_struct = node;
	}
	else {
#endif
		imap_stream = mail_open(NIL, mailbox->value.str.val, flags);
		if (imap_stream == NIL) {
			php3_error(E_WARNING,
				"Couldn't open stream %s\n",
				mailbox->value.str.val);
			RETURN_FALSE;
		}

		imap_le_struct = emalloc(sizeof(pils));
		imap_le_struct->imap_stream = imap_stream;
		imap_le_struct->flags = cl_flags;	
#ifdef OP_RELOGIN
	}

	if (persistent)
		ind = php3_list_insert(imap_le_struct, le_pimap);
	else
#endif
		ind = php3_list_insert(imap_le_struct, le_imap);

	RETURN_LONG(ind);
}

/* {{{ proto int imap_open(string mailbox, string user, string password [, int options])
   Open an IMAP stream to a mailbox */
void php3_imap_open(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_imap_do_open(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int imap_popen(string mailbox, string user, string password [, int options])
   Open an IMAP stream to a mailbox */
void php3_imap_popen(INTERNAL_FUNCTION_PARAMETERS)
{
#ifdef OP_RELOGIN
	php3_imap_do_open(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
#else
	php3_error(E_WARNING, "Persistent IMAP connections are not yet supported.\n");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto int imap_reopen(int stream_id, string mailbox [, int options])
   Reopen IMAP stream to new mailbox */
void php3_imap_reopen(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	pval *mailbox;
	pval *options;
	MAILSTREAM *imap_stream;
	pils *imap_le_struct; 
	int ind, ind_type;
	long flags=NIL;
	long cl_flags=NIL;
	int myargc=ARG_COUNT(ht);

	if (myargc<2 || myargc>3 || getParameters(ht,myargc,&streamind, &mailbox, &options) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	convert_to_long(streamind);
	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	convert_to_string(mailbox);
	if(myargc == 3) {
		convert_to_long(options);
		flags = options->value.lval;
		if(flags & PHP_EXPUNGE) {
			cl_flags = CL_EXPUNGE;
			flags ^= PHP_EXPUNGE;
		}
		imap_le_struct->flags = cl_flags;	
	}
	imap_stream = mail_open(imap_le_struct->imap_stream, mailbox->value.str.val, flags);
	if (imap_stream == NIL) {
		php3_error(E_WARNING,"Couldn't re-open stream\n");
		RETURN_FALSE;
	}
	imap_le_struct->imap_stream = imap_stream;
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imap_append(int stream_id, string folder, string message [, string flags])
   Append a string message to a specified mailbox */
void php3_imap_append(INTERNAL_FUNCTION_PARAMETERS)
{
  pval *streamind,*folder, *message,*flags;
  int ind, ind_type;
  pils *imap_le_struct; 
  STRING st;
  int myargc=ARG_COUNT(ht);
  
  if ( myargc < 3 || myargc > 4 || getParameters( ht, myargc, &streamind, &folder, &message,&flags) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  
  convert_to_long(streamind);
  convert_to_string(folder);
  convert_to_string(message);
  if (myargc == 4) convert_to_string(flags);
  ind = streamind->value.lval;
  
	imap_le_struct = (pils *)php3_list_find( ind, &ind_type );
	
	if ( !imap_le_struct || !IS_STREAM(ind_type)) {
	  php3_error(E_WARNING, "Unable to find stream pointer");
	  RETURN_FALSE;
	}
	INIT (&st,mail_string,(void *) message->value.str.val,message->value.str.len);
	if(mail_append_full(imap_le_struct->imap_stream, folder->value.str.val,myargc==4?flags->value.str.val:NIL,NIL,&st))
	  {
	  RETURN_TRUE;
	  }
	else
	  {
	  RETURN_FALSE;
	  }
}
/* }}} */

/* {{{ proto int imap_num_msg(int stream_id)
   Gives the number of messages in the current mailbox */
void php3_imap_num_msg(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);

	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	RETURN_LONG(imap_le_struct->imap_stream->nmsgs);
}
/* }}} */

/* {{{ proto int imap_ping(int stream_id)
   Check if the IMAP stream is still active */
void php3_imap_ping(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long( streamind );
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find( ind, &ind_type );
	if ( !imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error( E_WARNING, "Unable to find stream pointer" );
		RETURN_FALSE;
	}
	RETURN_LONG( mail_ping( imap_le_struct->imap_stream ) );
}
/* }}} */

/* {{{ proto int imap_num_recent(int stream_id)
   Gives the number of recent messages in current mailbox */
void php3_imap_num_recent(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	pils *imap_le_struct; 
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	RETURN_LONG(imap_le_struct->imap_stream->recent);
}
/* }}} */

/* {{{ proto int imap_expunge(int stream_id)
   Delete all messages marked for deletion */
void php3_imap_expunge(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);

	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	mail_expunge (imap_le_struct->imap_stream);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int imap_close(int stream_id [, int options])
   Close an IMAP stream */
void php3_imap_close(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *options, *streamind;
	int ind, ind_type;
	pils *imap_le_struct=NULL; 
	int myargcount=ARG_COUNT(ht);
	long flags = NIL;

	if (myargcount < 1 || myargcount > 2 || getParameters(ht, myargcount, &streamind, &options) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
    }
	if(myargcount==2) {
		convert_to_long(options);
		flags = options->value.lval;
		/* Do the translation from PHP's internal PHP_EXPUNGE define to c-client's CL_EXPUNGE */
		if(flags & PHP_EXPUNGE) {
			flags ^= PHP_EXPUNGE;
			flags |= CL_EXPUNGE;
		}	
		imap_le_struct->flags = flags;
	}
	php3_list_delete(ind);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array imap_headers(int stream_id)
   Returns headers for all messages in a mailbox */
void php3_imap_headers(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	unsigned long i;
	char *t;
	unsigned int msgno;
	pils *imap_le_struct; 
	char tmp[MAILTMPLEN];

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);

	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	/* Initialize return array */
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	for (msgno = 1; msgno <= imap_le_struct->imap_stream->nmsgs; msgno++) {
		MESSAGECACHE * cache = mail_elt (imap_le_struct->imap_stream,msgno);
		mail_fetchstructure (imap_le_struct->imap_stream,msgno,NIL);
		tmp[0] = cache->recent ? (cache->seen ? 'R': 'N') : ' ';
		tmp[1] = (cache->recent | cache->seen) ? ' ' : 'U';
		tmp[2] = cache->flagged ? 'F' : ' ';
		tmp[3] = cache->answered ? 'A' : ' ';
		tmp[4] = cache->deleted ? 'D' : ' ';
		tmp[5] = cache->draft ? 'X' : ' ';
		sprintf (tmp+5,"%4ld) ",cache->msgno);
		mail_date (tmp+11,cache);
		tmp[17] = ' ';
		tmp[18] = '\0';
		mail_fetchfrom (tmp+18,imap_le_struct->imap_stream,msgno,(long) 20);
		strcat (tmp," ");
		if ((i = cache->user_flags)) {
			strcat (tmp,"{");
			while (i) {
				strcat (tmp,imap_le_struct->imap_stream->user_flags[find_rightmost_bit (&i)]);
				if (i) strcat (tmp," ");
			}
			strcat (tmp,"} ");
		}
		mail_fetchsubject(t=tmp+strlen(tmp),imap_le_struct->imap_stream,msgno,(long)25);
		sprintf (t+=strlen(t)," (%ld chars)",cache->rfc822_size);
		add_next_index_string(return_value,tmp,1);
	}
}
/* }}} */

/* {{{ proto string imap_fetchtext(int stream_id, int msg_no [, int options])
   An alias for imap_body */
/* }}} */

/* {{{ proto string imap_body(int stream_id, int msg_no [, int options])
   Read the message body */
void php3_imap_body(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, * msgno, *flags;
	int ind, ind_type;
	pils *imap_le_struct; 
	int myargc=ARG_COUNT(ht);
	if (myargc <2 || myargc > 3 || getParameters(ht,myargc,&streamind,&msgno,&flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_long(msgno);
	if(myargc == 3) convert_to_long(flags);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	RETVAL_STRING(mail_fetchtext_full (imap_le_struct->imap_stream,msgno->value.lval,NIL,myargc == 3 ? flags->value.lval : NIL),1);
}
/* }}} */

/* {{{ proto string imap_fetchtext_full(int stream_id, int msg_no [, int options])
   Read the body of a message */
void php3_imap_fetchtext_full(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, * msgno, *flags;
	int ind, ind_type;
	pils *imap_le_struct; 
	int myargcount = ARG_COUNT(ht);
	if (myargcount >3 || myargcount <2 || getParameters(ht,myargcount,&streamind,&msgno,&flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_long(msgno);
	if (myargcount == 3) convert_to_long(flags);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	RETVAL_STRING(mail_fetchtext_full (imap_le_struct->imap_stream,msgno->value.lval,NIL,myargcount==3?flags->value.lval:NIL),1);
}
/* }}} */

/* {{{ proto int imap_mail_copy(int stream_id, int msg_no, string mailbox [, int options])
   Copy specified message to a mailbox */
void php3_imap_mail_copy(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind,*seq, *folder, *options;
	int ind, ind_type;
	pils *imap_le_struct; 
	int myargcount = ARG_COUNT(ht);
	if (myargcount > 4 || myargcount < 3 
		|| getParameters(ht,myargcount,&streamind,&seq,&folder,&options) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(seq);
	convert_to_string(folder);
	if(myargcount == 4) convert_to_long(options);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_copy_full(imap_le_struct->imap_stream,seq->value.str.val,folder->value.str.val,myargcount == 4 ? options->value.lval : NIL)==T ) {
        RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool imap_mail_move(int stream_id, int msg_no, string mailbox [, int options])
   Move specified message to a mailbox */
void php3_imap_mail_move(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind,*seq, *folder , *options;
	int ind, ind_type;
	pils *imap_le_struct; 
        int myargcount = ARG_COUNT(ht);

	if (myargcount > 4 || myargcount < 3
	    || getParameters(ht,myargcount,&streamind,&seq,&folder,&options) == FAILURE) {
	   WRONG_PARAM_COUNT;
        }

	convert_to_long(streamind);
	convert_to_string(seq);
	convert_to_string(folder);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_copy_full(imap_le_struct->imap_stream,seq->value.str.val,folder->value.str.val,myargcount == 4 ? ( options->value.lval | CP_MOVE ) : CP_MOVE )==T ) {
        RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int imap_create(int stream_id, string mailbox)
   An alias for imap_createmailbox */
/* }}} */

/* {{{ proto int imap_createmailbox(int stream_id, string mailbox)
   Create a new mailbox */
void php3_imap_createmailbox(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *folder;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht)!=2 || getParameters(ht,2,&streamind,&folder) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(folder);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_create(imap_le_struct->imap_stream,folder->value.str.val)==T ) {
        RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int imap_rename(int stream_id, string old_name, string new_name)
   An alias for imap_renamemailbox */
/* }}} */

/* {{{ proto int imap_renamemailbox(int stream_id, string old_name, string new_name)
   Rename a mailbox */
void php3_imap_renamemailbox(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *old, *new;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht)!=3 || getParameters(ht,3,&streamind,&old,&new)==FAILURE){
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(old);
	convert_to_string(new);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_rename(imap_le_struct->imap_stream,old->value.str.val,new->value.str.val)==T ) {
        RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool imap_deletemailbox(int stream_id, string mailbox)
   Delete a mailbox */
void php3_imap_deletemailbox(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *folder;
	int ind, ind_type;
	pils *imap_le_struct; 

	if (ARG_COUNT(ht)!=2 || getParameters(ht,2,&streamind,&folder) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(folder);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_delete(imap_le_struct->imap_stream,folder->value.str.val)==T ) {
        RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array imap_listmailbox(int stream_id, string ref, string pattern)
   An alias for imap_list */
/* }}} */

/* {{{ proto array imap_list(int stream_id, string ref, string pattern)
   Read the list of mailboxes */
void php3_imap_list(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *ref, *pat;
	int ind, ind_type;
	pils *imap_le_struct; 
	STRINGLIST *cur=NIL;
	
	/* set flag for normal, old mailbox list */
	folderlist_style = FLIST_ARRAY;
	
	if (ARG_COUNT(ht)!=3 
		|| getParameters(ht,3,&streamind,&ref,&pat) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(ref);
	convert_to_string(pat);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
    imap_folders = NIL;
	mail_list(imap_le_struct->imap_stream,ref->value.str.val,pat->value.str.val);
	if (imap_folders == NIL) {
		RETURN_FALSE;
	}
	array_init(return_value);
    cur=imap_folders;
    while (cur != NIL ) {
		add_next_index_string(return_value,cur->LTEXT,1);
        cur=cur->next;
    }
	mail_free_stringlist (&imap_folders);
}
/* }}} */


/* {{{ proto array imap_getmailboxes(int stream_id, string ref, string pattern)
   Reads the list of mailboxes and returns a full array of objects containing name, attributes, and delimiter. */
/* Author: CJH */
void php3_imap_list_full(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *ref, *pat, mboxob;
	int ind, ind_type;
	pils *imap_le_struct; 
	FOBJECTLIST *cur=NIL;
	char *delim=NIL;
	
	
	/* set flag for new, improved array of objects mailbox list */
	folderlist_style = FLIST_OBJECT;

	if (ARG_COUNT(ht)!=3 
		|| getParameters(ht,3,&streamind,&ref,&pat) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(streamind);
	convert_to_string(ref);
	convert_to_string(pat);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	
	imap_folder_objects = NIL;
	mail_list(imap_le_struct->imap_stream,ref->value.str.val,pat->value.str.val);
	if (imap_folder_objects == NIL) {
		RETURN_FALSE;
	}
	array_init(return_value);
	delim = emalloc(2 * sizeof(char));
	cur=imap_folder_objects;
	while (cur != NIL ) {
		object_init(&mboxob);
		add_property_string(&mboxob, "name", cur->LTEXT,1);
		add_property_long(&mboxob, "attributes", cur->attributes);
#ifdef IMAP41
		delim[0] = (char)cur->delimiter;
		delim[1] = 0;
		add_property_string(&mboxob, "delimiter", delim, 1);
#else
		add_property_string(&mboxob, "delimiter", cur->delimiter, 1);
#endif
		add_next_index_object(return_value, mboxob);
		cur=cur->next;
	}
	mail_free_foblist(&imap_folder_objects);
	efree(delim);
	folderlist_style = FLIST_ARRAY;		/* reset to default */
}
/* }}} */


/* {{{ proto array imap_scanmailbox(int stream_id, string ref, string pattern, string content)
   An alias for imap_scan */
/* }}} */

/* {{{ proto array imap_scan(int stream_id, string ref, string pattern, string content)
   Read list of mailboxes containing a certain string */
void php3_imap_listscan(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *ref, *pat, *content;
	int ind, ind_type;
	pils *imap_le_struct;
	STRINGLIST *cur=NIL;

	if (ARG_COUNT(ht)!=4 || getParameters(ht,4,&streamind,&ref,&pat,&content) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(ref);
	convert_to_string(pat);
	convert_to_string(content);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	imap_folders = NIL;
	mail_scan(imap_le_struct->imap_stream,ref->value.str.val,pat->value.str.val,content->value.str.val);
	if (imap_folders == NIL) {
		RETURN_FALSE;
	}
	array_init(return_value);
	cur=imap_folders;
	while (cur != NIL ) {
		add_next_index_string(return_value,cur->LTEXT,1);
		cur=cur->next;
	}
	mail_free_stringlist (&imap_folders);
}
/* }}} */

/* {{{ proto object imap_check(int stream_id)
   Get mailbox properties */
void php3_imap_check(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	int ind, ind_type;
	pils *imap_le_struct;
	char date[100];

	if (ARG_COUNT(ht)!=1 || getParameters(ht,1,&streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	if (mail_ping (imap_le_struct->imap_stream) == NIL ) {
		RETURN_FALSE;
    }
	if (imap_le_struct->imap_stream && imap_le_struct->imap_stream->mailbox) {
		rfc822_date (date);
		object_init(return_value);
		add_property_string(return_value,"Date",date,1);
		add_property_string(return_value,"Driver",imap_le_struct->imap_stream->dtb->name,1);
		add_property_string(return_value,"Mailbox",imap_le_struct->imap_stream->mailbox,1);
		add_property_long(return_value,"Nmsgs",imap_le_struct->imap_stream->nmsgs);
		add_property_long(return_value,"Recent",imap_le_struct->imap_stream->recent);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int imap_delete(int stream_id, int msg_no)
   Mark a message for deletion */
void php3_imap_delete(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *sequence, *flags;
	int ind, ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);

	if ( myargc < 2 || myargc > 3 || getParameters(ht,myargc,&streamind,&sequence,&flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(sequence);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	mail_setflag_full(imap_le_struct->imap_stream,sequence->value.str.val,"\\DELETED",myargc == 3 ? flags->value.lval : NIL);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int imap_undelete(int stream_id, int msg_no)
   Remove the delete flag from a message */
void php3_imap_undelete(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, * sequence, *flags;
	int ind, ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);

	if ( myargc < 2 || myargc > 3 || getParameters(ht,myargc,&streamind,&sequence,&flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	convert_to_string(sequence);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	mail_clearflag_full(imap_le_struct->imap_stream,sequence->value.str.val,"\\DELETED",myargc == 3 ? flags->value.lval : NIL);
	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto object imap_headerinfo(int stream_id, int msg_no [, int from_length [, int subject_length [, string default_host]]])
   An alias for imap_header */
/* }}} */

/* {{{ proto object imap_header(int stream_id, int msg_no [, int from_length [, int subject_length [, string default_host]]])
   Read the header of the message */
void php3_imap_headerinfo(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, * msgno,to,tovals,from,fromvals,reply_to,reply_tovals,sender;
	pval *fromlength;
	pval *subjectlength;
	pval sendervals,return_path,return_pathvals;
	pval cc,ccvals,bcc,bccvals;
	pval *defaulthost;
	int ind, ind_type;
	unsigned long length;
	pils *imap_le_struct;
	MESSAGECACHE * cache;
	char *mystring;
	char dummy[2000];
	char fulladdress[MAILTMPLEN],tempaddress[MAILTMPLEN];
	ENVELOPE *en;
	ADDRESS *addresstmp,*addresstmp2;
	int myargc;
	myargc=ARG_COUNT(ht);
	if (myargc<2 || myargc>5 || getParameters(ht,myargc,&streamind,&msgno,&fromlength,&subjectlength,&defaulthost)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	convert_to_long(msgno);
	if(myargc >= 3) convert_to_long(fromlength); else fromlength=0x00;;
	if(myargc >= 4) convert_to_long(subjectlength); else subjectlength=0x00;
	if(myargc == 5) convert_to_string(defaulthost);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	if(msgno->value.lval && msgno->value.lval <= imap_le_struct->imap_stream->nmsgs && mail_fetchstructure (imap_le_struct->imap_stream,msgno->value.lval,NIL))
		cache = mail_elt (imap_le_struct->imap_stream,msgno->value.lval);
	else
		RETURN_FALSE;
	object_init(return_value);


    mystring=mail_fetchheader_full(imap_le_struct->imap_stream,msgno->value.lval,NIL,&length,NIL);
	if(myargc ==5)
		rfc822_parse_msg (&en,NULL,mystring,length,NULL,defaulthost->value.str.val,NIL);
	else
		rfc822_parse_msg (&en,NULL,mystring,length,NULL,"UNKNOWN",NIL);

    if (en->remail) add_property_string(return_value,"remail",en->remail,1);
    if (en->date) add_property_string(return_value,"date",en->date,1);
    if (en->date) add_property_string(return_value,"Date",en->date,1);
    if (en->subject) add_property_string(return_value,"subject",en->subject,1);
    if (en->subject) add_property_string(return_value,"Subject",en->subject,1);
    if (en->in_reply_to) add_property_string(return_value,"in_reply_to",en->in_reply_to,1);
    if (en->message_id) add_property_string(return_value,"message_id",en->message_id,1);
    if (en->newsgroups) add_property_string(return_value,"newsgroups",en->newsgroups,1);
    if (en->followup_to) add_property_string(return_value,"followup_to",en->followup_to,1);
    if (en->references) add_property_string(return_value,"references",en->references,1);

	if (en->to) {
		int ok=1;
		addresstmp=en->to;
		fulladdress[0]=0x00;

		while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
			{
				addresstmp2=addresstmp->next; /* save the pointer to the next address */
				addresstmp->next=NULL; /* make this address the only one now. */
				tempaddress[0]=0x00; /* reset tempaddress buffer */
				rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
				if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
					{                /* yes */
						if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
						strcat(fulladdress,tempaddress); /* put in the new address */
					}
				else        /* no */
					{
						ok=0;  /* stop looping */
						strcat(fulladdress,", ...");
					}
				addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
				addresstmp=addresstmp->next;
			}

		if(fulladdress) add_property_string( return_value, "toaddress", fulladdress, 1);
		addresstmp=en->to;
		array_init( &to );
		do {
			object_init( &tovals);
			if(addresstmp->personal) add_property_string( &tovals, "personal", addresstmp->personal, 1);
			if(addresstmp->adl) add_property_string( &tovals, "adl", addresstmp->adl, 1);
			if(addresstmp->mailbox) add_property_string( &tovals, "mailbox", addresstmp->mailbox, 1);
			if(addresstmp->host) add_property_string( &tovals, "host", addresstmp->host, 1);
			add_next_index_object( &to, tovals );
		} while ( (addresstmp = addresstmp->next) );
		add_assoc_object( return_value, "to", to );
	}

	if(en->from)
		{
			int ok=1;
			addresstmp=en->from;
			fulladdress[0]=0x00;
  
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ...");
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}

			if(fulladdress) add_property_string( return_value, "fromaddress", fulladdress, 1);
			addresstmp=en->from;
			array_init( &from );
			do {
				object_init( &fromvals);
				if(addresstmp->personal) add_property_string( &fromvals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &fromvals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &fromvals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &fromvals, "host", addresstmp->host, 1);
				add_next_index_object( &from, fromvals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "from", from );
		}

	if(en->cc)
		{
			int ok=1;
			addresstmp=en->cc;
			fulladdress[0]=0x00;
  
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ..."); 
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}

			if(fulladdress) add_property_string( return_value, "ccaddress", fulladdress, 1);
			addresstmp=en->cc;
			array_init( &cc );
			do {
				object_init( &ccvals);
				if(addresstmp->personal) add_property_string( &ccvals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &ccvals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &ccvals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &ccvals, "host", addresstmp->host, 1);
				add_next_index_object( &cc, ccvals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "cc", cc );
		}

	if(en->bcc)
		{
			int ok=1;
			addresstmp=en->bcc;
			fulladdress[0]=0x00;
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ..."); 
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}

			if(fulladdress) add_property_string( return_value, "bccaddress", fulladdress, 1);
			addresstmp=en->bcc;
			array_init( &bcc );
			do {
				object_init( &bccvals);
				if(addresstmp->personal) add_property_string( &bccvals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &bccvals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &bccvals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &bccvals, "host", addresstmp->host, 1);
				add_next_index_object( &bcc, bccvals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "bcc", bcc );
		}

	if(en->reply_to)
		{
			int ok=1;
			addresstmp=en->reply_to;
			fulladdress[0]=0x00;
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ..."); 
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}

			if(fulladdress) add_property_string( return_value, "reply_toaddress", fulladdress, 1);
			addresstmp=en->reply_to;
			array_init( &reply_to );
			do {
				object_init( &reply_tovals);
				if(addresstmp->personal) add_property_string( &reply_tovals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &reply_tovals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &reply_tovals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &reply_tovals, "host", addresstmp->host, 1);
				add_next_index_object( &reply_to, reply_tovals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "reply_to", reply_to );
		}

	if(en->sender)
		{
			int ok=1;
			addresstmp=en->sender;
			fulladdress[0]=0x00;
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ..."); 
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}

			if(fulladdress) add_property_string( return_value, "senderaddress", fulladdress, 1);
			addresstmp=en->sender;
			array_init( &sender );
			do {
				object_init( &sendervals);
				if(addresstmp->personal) add_property_string( &sendervals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &sendervals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &sendervals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &sendervals, "host", addresstmp->host, 1);
				add_next_index_object( &sender, sendervals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "sender", sender );
		}

	if(en->return_path)
		{
			int ok=1;
			addresstmp=en->return_path;
			fulladdress[0]=0x00;
			while(ok && addresstmp) /* while length < 1000 and we are not at the end of the list */
				{
					addresstmp2=addresstmp->next; /* save the pointer to the next address */
					addresstmp->next=NULL; /* make this address the only one now. */
					tempaddress[0]=0x00; /* reset tempaddress buffer */
					rfc822_write_address(tempaddress,addresstmp); /* ok, write the address into tempaddress string */
					if((strlen(tempaddress) + strlen(fulladdress)) < 1000) /* is the new address + total address < 1000 */
						{                /* yes */
							if(strlen(fulladdress)) strcat(fulladdress,","); /* put in a comma */ 
							strcat(fulladdress,tempaddress); /* put in the new address */
						}
					else        /* no */
						{
							ok=0;  /* stop looping */
							strcat(fulladdress,", ..."); 
						}
					addresstmp->next=addresstmp2; /* reset the pointer to the next address first! */
					addresstmp=addresstmp->next;
				}
  
			if(fulladdress) add_property_string( return_value, "return_pathaddress", fulladdress, 1);
			addresstmp=en->return_path;
			array_init( &return_path );
			do {
				object_init( &return_pathvals);
				if(addresstmp->personal) add_property_string( &return_pathvals, "personal", addresstmp->personal, 1);
				if(addresstmp->adl) add_property_string( &return_pathvals, "adl", addresstmp->adl, 1);
				if(addresstmp->mailbox) add_property_string( &return_pathvals, "mailbox", addresstmp->mailbox, 1);
				if(addresstmp->host) add_property_string( &return_pathvals, "host", addresstmp->host, 1);
				add_next_index_object( &return_path, return_pathvals );
			} while ( (addresstmp = addresstmp->next) );
			add_assoc_object( return_value, "return_path", return_path );
		}
	add_property_string(return_value,"Recent",cache->recent ? (cache->seen ? "R": "N") : " ",1);
	add_property_string(return_value,"Unseen",(cache->recent | cache->seen) ? " " : "U",1);
	add_property_string(return_value,"Flagged",cache->flagged ? "F" : " ",1);
	add_property_string(return_value,"Answered",cache->answered ? "A" : " ",1);
	add_property_string(return_value,"Deleted",cache->deleted ? "D" : " ",1);
	add_property_string(return_value,"Draft",cache->draft ? "X" : " ",1);
	sprintf (dummy,"%4ld",cache->msgno);
	add_property_string(return_value,"Msgno",dummy,1);
	mail_date (dummy,cache);
	add_property_string(return_value,"MailDate",dummy,1);
	sprintf (dummy,"%ld",cache->rfc822_size); 
    add_property_string(return_value,"Size",dummy,1);
    add_property_long(return_value,"udate",mail_longdate(cache));
 
	if(en->from && fromlength)
		{
			fulladdress[0]=0x00;
			mail_fetchfrom(fulladdress,imap_le_struct->imap_stream,msgno->value.lval,fromlength->value.lval);
			add_property_string(return_value,"fetchfrom",fulladdress,1);
		}
	if(en->subject && subjectlength)
		{
			fulladdress[0]=0x00;
			mail_fetchsubject(fulladdress,imap_le_struct->imap_stream,msgno->value.lval,subjectlength->value.lval);
			add_property_string(return_value,"fetchsubject",fulladdress,1);
		}
}
/* }}} */

/* KMLANG */
/* {{{ proto array imap_listsubscribed(int stream_id, string ref, string pattern)
   An alias for imap_lsub */
/* }}} */

/* {{{ proto array imap_lsub(int stream_id, string ref, string pattern)
   Return a list of subscribed mailboxes */
void php3_imap_lsub(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *ref, *pat;
	int ind, ind_type;
	pils *imap_le_struct;
	STRINGLIST *cur=NIL;

	/* set flag for normal, old mailbox list */
	folderlist_style = FLIST_ARRAY;
	
	if (ARG_COUNT(ht)!=3 || getParameters(ht,3,&streamind,&ref,&pat) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(ref);
	convert_to_string(pat);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	imap_sfolders = NIL;
	mail_lsub(imap_le_struct->imap_stream,ref->value.str.val,pat->value.str.val);
	if (imap_sfolders == NIL) {
		RETURN_FALSE;
	}
	array_init(return_value);
	cur=imap_sfolders;
	while (cur != NIL ) {
		add_next_index_string(return_value,cur->LTEXT,1);
		cur=cur->next;
	}
	mail_free_stringlist (&imap_sfolders);
}
/* }}} */


/* {{{ proto array imap_getsubscribed(int stream_id, string ref, string pattern)
   Return a list of subscribed mailboxes */
/* Author: CJH */
void php3_imap_lsub_full(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *ref, *pat, mboxob;
	int ind, ind_type;
	pils *imap_le_struct;
	FOBJECTLIST *cur=NIL;
	char *delim=NIL;
	
	delim = emalloc(2 * sizeof(char));
	
	/* set flag for new, improved array of objects list */
	folderlist_style = FLIST_OBJECT;
	
	if (ARG_COUNT(ht)!=3 || getParameters(ht,3,&streamind,&ref,&pat) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(streamind);
	convert_to_string(ref);
	convert_to_string(pat);

	ind = streamind->value.lval;
	
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	
	imap_sfolder_objects = NIL;
	mail_lsub(imap_le_struct->imap_stream,ref->value.str.val,pat->value.str.val);
	if (imap_sfolder_objects == NIL) {
		RETURN_FALSE;
	}
	array_init(return_value);
	cur=imap_sfolder_objects;
	while (cur != NIL ) {
		object_init(&mboxob);
		add_property_string(&mboxob, "name", cur->LTEXT, 1);
		add_property_long(&mboxob, "attributes", cur->attributes);
#ifdef IMAP41
		delim[0] = (char)cur->delimiter;
		delim[1] = 0;
		add_property_string(&mboxob, "delimiter", delim, 1);
#else
		add_property_string(&mboxob, "delimiter", cur->delimiter, 1);
#endif
		add_next_index_object(return_value, mboxob);
		cur=cur->next;
	}
	mail_free_foblist (&imap_sfolder_objects);
	efree(delim);
	folderlist_style = FLIST_ARRAY; /* reset to default */
}
/* }}} */


/* {{{ proto int imap_subscribe(int stream_id, string mailbox)
   Subscribe to a mailbox */
void php3_imap_subscribe(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *folder;
	int ind, ind_type;
	pils *imap_le_struct;

	if (ARG_COUNT(ht)!=2 || getParameters(ht,2,&streamind,&folder) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(folder);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_subscribe(imap_le_struct->imap_stream,folder->value.str.val)==T ) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int imap_unsubscribe(int stream_id, string mailbox)
   Unsubscribe from a mailbox */
void php3_imap_unsubscribe(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *folder;
	int ind, ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);
	if (myargc !=2 || getParameters(ht,myargc,&streamind,&folder) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);
	convert_to_string(folder);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	if ( mail_unsubscribe(imap_le_struct->imap_stream,folder->value.str.val)==T ) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

void imap_add_body( pval *arg, BODY *body )
{
	pval parametres, param, dparametres, dparam;
	PARAMETER *par, *dpar;
	PART *part;

	if(body->type) add_property_long( arg, "type", body->type );
	if(body->encoding) add_property_long( arg, "encoding", body->encoding );

	if ( body->subtype ){
		add_property_long( arg, "ifsubtype", 1 );
		add_property_string( arg, "subtype",  body->subtype, 1 );
	} else {
		add_property_long( arg, "ifsubtype", 0 );
	}

	if ( body->description ){
		add_property_long( arg, "ifdescription", 1 );
		add_property_string( arg, "description",  body->description, 1 );
	} else {
		add_property_long( arg, "ifdescription", 0 );
	}
	if ( body->id ){
		add_property_long( arg, "ifid", 1 );
		add_property_string( arg, "id",  body->id, 1 );
	} else {
		add_property_long( arg, "ifid", 0 );
	}

	if(body->size.lines) add_property_long( arg, "lines", body->size.lines );
	if(body->size.bytes) add_property_long( arg, "bytes", body->size.bytes );
#ifdef IMAP41
	if ( body->disposition.type ){
		add_property_long( arg, "ifdisposition", 1);
		add_property_string( arg, "disposition", body->disposition.type, 1);
	} else {
		add_property_long( arg, "ifdisposition", 0);
	}

	if ( body->disposition.parameter ) {
		dpar = body->disposition.parameter;
		add_property_long( arg, "ifdparameters", 1);
		array_init( &dparametres );
		do {
			object_init( &dparam );
			add_property_string( &dparam, "attribute", dpar->attribute, 1);
			add_property_string( &dparam, "value", dpar->value, 1);
			add_next_index_object( &dparametres, dparam );
        } while ( (dpar = dpar->next) );
		add_assoc_object( arg, "dparameters", dparametres );
	} else {
		add_property_long( arg, "ifdparameters", 0);
	}
#endif
 
	if ( (par = body->parameter) ) {
		add_property_long( arg, "ifparameters", 1 );

		array_init( &parametres );
		do {
			object_init( &param );
			if(par->attribute) add_property_string( &param, "attribute", par->attribute, 1 );
			if(par->value) add_property_string( &param, "value", par->value, 1 );

			add_next_index_object( &parametres, param );
		} while ( (par = par->next) );
	} else {
		object_init(&parametres);
		add_property_long( arg, "ifparameters", 0 );
	}
	add_assoc_object( arg, "parameters", parametres );

	/* multipart message ? */

	if ( body->type == TYPEMULTIPART ) {
		array_init( &parametres );
		for ( part = body->CONTENT_PART; part; part = part->next ) {
			object_init( &param );
			imap_add_body( &param, &part->body );
			add_next_index_object( &parametres, param );
		}
		add_assoc_object( arg, "parts", parametres );
	}

	/* encapsulated message ? */

	if ((body->type == TYPEMESSAGE) && (!strcasecmp(body->subtype, "rfc822"))) {
		body = body->CONTENT_MSG_BODY;
		array_init(&parametres);
		object_init( &param );
		imap_add_body( &param, body );
		add_next_index_object(&parametres, param );
		add_assoc_object( arg, "parts", parametres );
	}
}

/* {{{ proto object imap_fetchstructure(int stream_id, int msg_no [, int options])
   Read the full structure of a message */
void php3_imap_fetchstructure(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *msgno,*flags;
	int ind, ind_type;
	pils *imap_le_struct;
	BODY *body;
	int myargc=ARG_COUNT(ht);
	if ( myargc < 2  || myargc > 3 || getParameters( ht, myargc, &streamind, &msgno ,&flags) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long( streamind );
	convert_to_long( msgno );
	if (msgno->value.lval < 1) {
		RETURN_FALSE;
	}
	if(myargc == 3) convert_to_long(flags);
	object_init(return_value);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find( ind, &ind_type );

	if ( !imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error( E_WARNING, "Unable to find stream pointer" );
		RETURN_FALSE;
	}

	mail_fetchstructure_full( imap_le_struct->imap_stream, msgno->value.lval, &body ,myargc == 3 ? flags->value.lval : NIL);

	if ( !body ) {
		php3_error( E_WARNING, "No body information available" );
		RETURN_FALSE;
	}

	imap_add_body( return_value, body );
}
/* }}} */

/* {{{ proto string imap_fetchbody(int stream_id, int msg_no, string section [, int options])
   Get a specific body section */
void php3_imap_fetchbody(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *msgno, *sec,*flags;
	int ind, ind_type;
	pils *imap_le_struct;
	char *body;
	unsigned long len;
	int myargc=ARG_COUNT(ht);
	if(myargc < 3 || myargc >4 || getParameters( ht, myargc, &streamind, &msgno, &sec,&flags ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long( streamind );
	convert_to_long( msgno );
	convert_to_string( sec );
	if(myargc == 4) convert_to_long(flags);
	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find( ind, &ind_type );

	if ( !imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error( E_WARNING, "Unable to find stream pointer" );
		RETURN_FALSE;
	}
 
	body = mail_fetchbody_full( imap_le_struct->imap_stream, msgno->value.lval, sec->value.str.val, &len,myargc == 4 ? flags->value.lval : NIL );

	if ( !body ) {
		php3_error( E_WARNING, "No body information available" );
		RETURN_FALSE;
	}
	RETVAL_STRINGL( body ,len,1);
}
/* }}} */

/* {{{ proto string imap_base64(string text)
   Decode BASE64 encoded text */
void php3_imap_base64(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *text;
	char *decode;
	unsigned long newlength;
	if ( ARG_COUNT(ht) != 1 || getParameters( ht, 1, &text) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string( text );
	object_init(return_value);

	decode = (char *) rfc822_base64((unsigned char *) text->value.str.val, text->value.str.len,&newlength);
	RETVAL_STRINGL(decode,newlength,1);
}
/* }}} */

/* {{{ proto string imap_qprint(string text)
   Convert a quoted-printable string to an 8-bit string */
void php3_imap_qprint(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *text;
	char *decode;
	unsigned long newlength;
	if ( ARG_COUNT(ht) != 1 || getParameters( ht, 1, &text) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string( text );
/*	object_init(return_value);               Why is this here?  -RL */

	decode = (char *) rfc822_qprint((unsigned char *) text->value.str.val, text->value.str.len,&newlength);
	RETVAL_STRINGL(decode,newlength,1);
}
/* }}} */

/* {{{ proto string imap_8bit(string text)
   Convert an 8-bit string to a quoted-printable string */
void php3_imap_8bit(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *text;
	char *decode;
	unsigned long newlength;
	if ( ARG_COUNT(ht) != 1 || getParameters( ht, 1, &text) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string( text );
	object_init(return_value);

	decode = (char *) rfc822_8bit((unsigned char *) text->value.str.val, text->value.str.len,&newlength);
	RETVAL_STRINGL(decode,newlength,1);
}
/* }}} */

/* {{{ proto string imap_binary(string text)
   Convert an 8bit string to a base64 string */
void php3_imap_binary(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *text;
	unsigned long len;
	if ( ARG_COUNT(ht) != 1 || getParameters( ht, 1, &text) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(text);
	RETVAL_STRINGL(rfc822_binary(text->value.str.val,text->value.str.len,&len),len,1);
}
/* }}} */

/* {{{ proto array imap_mailboxmsginfo(int stream_id)
   Returns info about the current mailbox in an associative array */
void php3_imap_mailboxmsginfo(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	char date[100];
	int ind, ind_type;
	unsigned int msgno;
	pils *imap_le_struct;
	unsigned unreadmsg,msize;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &streamind) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(streamind);

	ind = streamind->value.lval;

	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);

	if(!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}

	/* Initialize return array */
	if(object_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	unreadmsg = 0;
	msize = 0;
	for (msgno = 1; msgno <= imap_le_struct->imap_stream->nmsgs; msgno++) {
		MESSAGECACHE * cache = mail_elt (imap_le_struct->imap_stream,msgno);
		mail_fetchstructure (imap_le_struct->imap_stream,msgno,NIL);
		unreadmsg = cache->recent ? (cache->seen ? unreadmsg : unreadmsg++) : unreadmsg;
		unreadmsg = (cache->recent | cache->seen) ? unreadmsg : unreadmsg++;
		msize = msize + cache->rfc822_size;
	}
	add_property_long(return_value,"Unread",unreadmsg);
	add_property_long(return_value,"Nmsgs",imap_le_struct->imap_stream->nmsgs);
	add_property_long(return_value,"Size",msize);
	rfc822_date (date);
	add_property_string(return_value,"Date",date,1);
	add_property_string(return_value,"Driver",imap_le_struct->imap_stream->dtb->name,1);
	add_property_string(return_value,"Mailbox",imap_le_struct->imap_stream->mailbox,1);
	add_property_long(return_value,"Recent",imap_le_struct->imap_stream->recent);
}
/* }}} */

/* {{{ proto string imap_rfc822_write_address(string mailbox, string host, string personal)
   Returns a properly formatted email address given the mailbox, host, and personal info */
void php3_imap_rfc822_write_address(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *mailbox,*host,*personal;
	ADDRESS *addr;
	char string[MAILTMPLEN];
	int argc;
	argc=ARG_COUNT(ht);
	if ( argc != 3 || getParameters( ht, argc, &mailbox,&host,&personal) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(mailbox);
	convert_to_string(host);
	convert_to_string(personal);
	addr=mail_newaddr();
	if(mailbox) addr->mailbox = cpystr(mailbox->value.str.val);
	if(host) addr->host = cpystr(host->value.str.val);
	if(personal) addr->personal = cpystr(personal->value.str.val);
	addr->next=NIL;
	addr->error=NIL;
	addr->adl=NIL;
	string[0]=0x00;
  
	rfc822_write_address(string,addr);
	RETVAL_STRING(string,1);
}
/* }}} */

/* {{{ proto array imap_rfc822_parse_adrlist(string address_string, string default_host)
   Parses an address string */
void php3_imap_rfc822_parse_adrlist(INTERNAL_FUNCTION_PARAMETERS)
{
       pval *string,*defaulthost,tovals;
       ADDRESS *addresstmp;
       ENVELOPE *env;
	int argc;

	env=mail_newenvelope();
	argc=ARG_COUNT(ht);
	if ( argc != 2 || getParameters( ht, argc, &string,&defaulthost) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(string);
	convert_to_string(defaulthost);
	rfc822_parse_adrlist(&env->to,string->value.str.val,defaulthost->value.str.val);
	if(array_init(return_value) == FAILURE) {
	  RETURN_FALSE;
	}
	addresstmp=env->to;
	if(addresstmp) do {
	  object_init(&tovals);
	  if(addresstmp->mailbox) add_property_string(&tovals,"mailbox",addresstmp->mailbox,1);
	  if(addresstmp->host) add_property_string(&tovals,"host",addresstmp->host,1);
	  if(addresstmp->personal) add_property_string(&tovals,"personal",addresstmp->personal,1);
	  if(addresstmp->adl) add_property_string(&tovals,"adl",addresstmp->adl,1);
	  add_next_index_object(return_value, tovals);
	} while ((addresstmp = addresstmp->next));
}
/* }}} */


/* {{{ proto string imap_utf8(string string)
   Convert a string to UTF-8 */
void php3_imap_utf8(INTERNAL_FUNCTION_PARAMETERS)
{
       pval *string;
	int argc;
	SIZEDTEXT src,dest;
	src.data=NULL;
	src.size=0;
	dest.data=NULL;
	dest.size=0;

	argc=ARG_COUNT(ht);
	if ( argc != 1 || getParameters( ht, argc, &string) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(string);
	cpytxt(&src,string->value.str.val,string->value.str.len);
	utf8_mime2text(&src,&dest);
	RETURN_STRINGL(dest.data,strlen(dest.data),1);
}
/* }}} */

/* macros for the modified utf7 conversion functions */
/* author: Andrew Skalski <askalski@chek.com> */

/* tests `c' and returns true if it is a special character */
#define SPECIAL(c) ((c) <= 0x1f || (c) >= 0x7f)
/* validate a modified-base64 character */
#define B64CHAR(c) (isalnum(c) || (c) == '+' || (c) == ',')
/* map the low 64 bits of `n' to the modified-base64 characters */
#define B64(n)  ("ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                "abcdefghijklmnopqrstuvwxyz0123456789+,"[(n) & 0x3f])
/* map the modified-base64 character `c' to its 64 bit value */
#define UNB64(c)        ((c) == '+' ? 62 : (c) == ',' ? 63 : (c) >= 'a' ? \
                        (c) - 71 : (c) >= 'A' ? (c) - 65 : (c) + 4)

/* {{{ proto string imap_utf7_decode(string buf)
   Decode a modified UTF-7 string */
void php3_imap_utf7_decode(INTERNAL_FUNCTION_PARAMETERS)
{
	/* author: Andrew Skalski <askalski@chek.com> */
	int			argc;
	pval			*arg;
	const unsigned char	*in, *inp, *endp;
	unsigned char		*out, *outp;
	int			inlen, outlen;
	enum {	ST_NORMAL,	/* printable text */
		ST_DECODE0,	/* encoded text rotation... */
		ST_DECODE1,
		ST_DECODE2,
		ST_DECODE3
	}			state;

	/* collect arguments */
	argc = ARG_COUNT(ht);
	if (argc != 1 || getParameters(ht, argc, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);
	in = (const unsigned char*) arg->value.str.val;
	inlen = arg->value.str.len;

	/* validate and compute length of output string */
	outlen = 0;
	state = ST_NORMAL;
	for (endp = (inp = in) + inlen; inp < endp; inp++) {
		if (state == ST_NORMAL) {
			/* process printable character */
			if (SPECIAL(*inp)) {
				php3_error(E_WARNING, "imap_utf7_decode: "
					"Invalid modified UTF-7 character: "
					"`%c'", *inp);
				RETURN_FALSE;
			}
			else if (*inp != '&') {
				outlen++;
			}
			else if (inp + 1 == endp) {
				php3_error(E_WARNING, "imap_utf7_decode: "
					"Unexpected end of string");
				RETURN_FALSE;
			}
			else if (inp[1] != '-') {
				state = ST_DECODE0;
			}
			else {
				outlen++;
				inp++;
			}
		}
		else if (*inp == '-') {
			/* return to NORMAL mode */
			if (state == ST_DECODE1) {
				php3_error(E_WARNING, "imap_utf7_decode: "
					"Stray modified base64 character: "
					"`%c'", *--inp);
				RETURN_FALSE;
			}
			state = ST_NORMAL;
		}
		else if (!B64CHAR(*inp)) {
			php3_error(E_WARNING, "imap_utf7_decode: "
				"Invalid modified base64 character: "
				"`%c'", *inp);
			RETURN_FALSE;
		}
		else {
			switch (state) {
			case ST_DECODE3:
				outlen++;
				state = ST_DECODE0;
				break;
			case ST_DECODE2:
			case ST_DECODE1:
				outlen++;
			case ST_DECODE0:
				state++;
			case ST_NORMAL:
				break;
			}
		}
	}

	/* enforce end state */
	if (state != ST_NORMAL) {
		php3_error(E_WARNING, "imap_utf7_decode: "
			"Unexpected end of string");
		RETURN_FALSE;
	}

	/* allocate output buffer */
	if ((out = emalloc(outlen + 1)) == NULL) {
		php3_error(E_WARNING, "imap_utf7_decode: "
			"Unable to allocate result string");
		RETURN_FALSE;
	}

	/* decode input string */
	outp = out;
	state = ST_NORMAL;
	for (endp = (inp = in) + inlen; inp < endp; inp++) {
		if (state == ST_NORMAL) {
			if (*inp == '&' && inp[1] != '-') {
				state = ST_DECODE0;
			}
			else if ((*outp++ = *inp) == '&') {
				inp++;
			}
		}
		else if (*inp == '-') {
			state = ST_NORMAL;
		}
		else {
			/* decode input character */
			switch (state) {
			case ST_DECODE0:
				*outp = UNB64(*inp) << 2;
				state = ST_DECODE1;
				break;
			case ST_DECODE1:
				outp[1] = UNB64(*inp);
				*outp++ |= outp[1] >> 4;
				*outp <<= 4;
				state = ST_DECODE2;
				break;
			case ST_DECODE2:
				outp[1] = UNB64(*inp);
				*outp++ |= outp[1] >> 2;
				*outp <<= 6;
				state = ST_DECODE3;
				break;
			case ST_DECODE3:
				*outp++ |= UNB64(*inp);
				state = ST_DECODE0;
			case ST_NORMAL:
				break;
			}
		}
	}

	*outp = 0;

#if DEBUG
	/* warn if we computed outlen incorrectly */
	if (outp - out != outlen) {
		php3_error(E_WARNING,
			"imap_utf7_decode: outp - out [%d] != outlen [%d]",
			outp - out, outlen);
	}
#endif

	RETURN_STRINGL(out, outlen, 0);
}
/* }}} */

/* {{{ proto string imap_utf7_encode(string buf)
   Encode a string in modified UTF-7 */
void php3_imap_utf7_encode(INTERNAL_FUNCTION_PARAMETERS)
{
	/* author: Andrew Skalski <askalski@chek.com> */
	int			argc;
	pval			*arg;
	const unsigned char	*in, *inp, *endp;
	unsigned char		*out, *outp;
	int			inlen, outlen;
	enum {	ST_NORMAL,	/* printable text */
		ST_ENCODE0,	/* encoded text rotation... */
		ST_ENCODE1,
		ST_ENCODE2
	}			state;

	/* collect arguments */
	argc = ARG_COUNT(ht);
	if (argc != 1 || getParameters(ht, argc, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);
	in = (const unsigned char*) arg->value.str.val;
	inlen = arg->value.str.len;

	/* compute the length of the result string */
	outlen = 0;
	state = ST_NORMAL;
	endp = (inp = in) + inlen;
	while (inp < endp) {
		if (state == ST_NORMAL) {
			if (SPECIAL(*inp)) {
				state = ST_ENCODE0;
				outlen++;
			}
			else if (*inp++ == '&') {
				outlen++;
			}
			outlen++;
		}
		else if (!SPECIAL(*inp)) {
			state = ST_NORMAL;
		}
		else {
			/* ST_ENCODE0 -> ST_ENCODE1	- two chars
			 * ST_ENCODE1 -> ST_ENCODE2	- one char
			 * ST_ENCODE2 -> ST_ENCODE0	- one char
			 */
			if (state == ST_ENCODE2) {
				state = ST_ENCODE0;
			}
			else if (state++ == ST_ENCODE0) {
				outlen++;
			}
			outlen++;
			inp++;
		}
	}

	/* allocate output buffer */
	if ((out = emalloc(outlen + 1)) == NULL) {
		php3_error(E_WARNING, "imap_utf7_encode: "
			"Unable to allocate result string");
		RETURN_FALSE;
	}

	/* encode input string */
	outp = out;
	state = ST_NORMAL;
	endp = (inp = in) + inlen;
	while (inp < endp || state != ST_NORMAL) {
		if (state == ST_NORMAL) {
			if (SPECIAL(*inp)) {
				/* begin encoding */
				*outp++ = '&';
				state = ST_ENCODE0;
			}
			else if ((*outp++ = *inp++) == '&') {
				*outp++ = '-';
			}
		}
		else if (inp == endp || !SPECIAL(*inp)) {
			/* flush overflow and terminate region */
			if (state != ST_ENCODE0) {
				*outp++ = B64(*outp);
			}
			*outp++ = '-';
			state = ST_NORMAL;
		}
		else {
			/* encode input character */
			switch (state) {
			case ST_ENCODE0:
				*outp++ = B64(*inp >> 2);
				*outp = *inp++ << 4;
				state = ST_ENCODE1;
				break;
			case ST_ENCODE1:
				*outp++ = B64(*outp | *inp >> 4);
				*outp = *inp++ << 2;
				state = ST_ENCODE2;
				break;
			case ST_ENCODE2:
				*outp++ = B64(*outp | *inp >> 6);
				*outp++ = B64(*inp++);
				state = ST_ENCODE0;
			case ST_NORMAL:
				break;
			}
		}
	}

	*outp = 0;

#if DEBUG
	/* warn if we computed outlen incorrectly */
	if (outp - out != outlen) {
		php3_error(E_WARNING,
			"imap_utf7_encode: outp - out [%d] != outlen [%d]",
			outp - out, outlen);
	}
#endif

	RETURN_STRINGL(out, outlen, 0);
}
/* }}} */

#undef SPECIAL
#undef B64CHAR
#undef B64
#undef UNB64

/* {{{ proto object imap_mime_header_decode(string str)
   Decode mime header element in accordance with RFC 2047 and return array of objects containing 'charset' encoding and decoded 'text' */
void php3_imap_mime_header_decode(INTERNAL_FUNCTION_PARAMETERS)
{
	/* string, string-endp, charset, charset-endp,
	 * encoding, encoding-endp, text, text-endp,
	 * language separator, marked start of unencoded text */
	unsigned char	*s, *se, *cs, *ce, *e, *ee, *t, *te, *ls, *mark;
	/* decoded text */
	SIZEDTEXT	decoded;
	int		argc;
	pval		*arg, obj;

	/* collect arguments */
	argc = ARG_COUNT(ht);
	if (argc != 1 || getParameters(ht, argc, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);
	s = arg->value.str.val;
	se = s + arg->value.str.len;

	/* init return value */
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	/* conversion routine derived from Mark Crispin's c-client library
	 * function utf_mime2text()
	 */
#define MINENCWORD 9
	for (mark = s; s < se; s++) {
		/* try to tokenize as a mime2 encoded word */
		if ((se - s) > MINENCWORD && *s == '=' && s[1] == '?' &&
			(cs = mime2_token(s + 2, se, &ce)) &&
			(e = mime2_token(ce + 1, se, &ee)) &&
			(t = mime2_text(e + 2, se, &te)))
		{
			/* try to decode the text */
			if (!mime2_decode(e, t, te, &decoded)) {
				/* skip over the block */
				s = te + 1;
				continue;
			}

			/* first, flush out any non-encoded text */
			if (mark < s) {
				object_init(&obj);
				add_property_string(&obj,
					"charset", "default", 1);
				add_property_stringl(&obj,
					"text", mark, s - mark, 1);
				add_next_index_object(return_value, obj);
			}

			/* advance the string pointer and the mark */
			s = te + 1;
			mark = s + 1;

			/* tie off charset and remove language specifier */
			*ce = '\0';
			if (ls = strchr(cs, '*')) *ls = '\0';
			/* flush decoded text */
			object_init(&obj);
			add_property_string(&obj, "charset", cs, 1);
			add_property_stringl(&obj, "text",
				decoded.data, decoded.size, 1);
			add_next_index_object(return_value, obj);
			/* restore charset and free decoded data */
			if (ls) *ls = '*';
			*ce = '?';
			fs_give((void**) &decoded.data);

			/* skip trailing whitespace; handle continuations */
			for (t = s + 1; t < se && (*t == ' ' || *t == '\t');
				t++);
			if (t < se - MINENCWORD) switch (*t) {
			case '=':	/* encoded block? */
				if (t[1] == '?') s = t - 1;
				break;
			case '\r':	/* CR */
				if (t[1] == '\n') t++;
			case '\n':	/* LF */
				if (t[1] == ' ' || t[1] == '\t') {
					/* eat up leading spaces/tabs */
					do t++;
					while (t < se - MINENCWORD &&
						(t[1] == ' ' || t[1] == '\t'));
					/* found something interesting */
					if (t < se - MINENCWORD &&
						t[1] == '=' && t[2] == '?')
					{
						s = t;
					}
				}
			}
		}
	}
#undef MINENCWORD
	/* flush out remaining non-encoded text */
	if (mark < se) {
		object_init(&obj);
		add_property_string(&obj, "charset", "default", 1);
		add_property_stringl(&obj, "text", mark, se - mark, 1);
		add_next_index_object(return_value, obj);
	}
}
/* }}} */

/* {{{ proto int imap_setflag_full(int stream_id, string sequence, string flag [, int options])
   Sets flags on messages */
void php3_imap_setflag_full(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	pval *sequence;
	pval *flag;
	pval *flags;
	int ind,ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);

	if (myargc < 3 || myargc > 4 || getParameters(ht, myargc, &streamind,&sequence,&flag,&flags) ==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	convert_to_string(sequence);
	convert_to_string(flag);
	if(myargc==4) convert_to_long(flags);

	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if(!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	mail_setflag_full(imap_le_struct->imap_stream,sequence->value.str.val,flag->value.str.val,myargc == 4 ? flags->value.lval : NIL);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void imap_clearflag_full(int stream_id, string sequence, string flag [, int options])
   Clears flags on messages */
void php3_imap_clearflag_full(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	pval *sequence;
	pval *flag;
	pval *flags;
	int ind,ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);
	if (myargc < 3 || myargc > 4 || getParameters(ht, myargc, &streamind,&sequence,&flag,&flags) ==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	convert_to_string(sequence);
	convert_to_string(flag);
	if(myargc==4) convert_to_long(flags);
	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if(!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	mail_clearflag_full(imap_le_struct->imap_stream,sequence->value.str.val,flag->value.str.val,myargc == 4 ? flags->value.lval : NIL);
        RETURN_TRUE;
}
/* }}} */

/* {{{ proto array imap_sort(int stream_id, int criteria, int reverse [, int options])
   Sort an array of message headers */
void php3_imap_sort(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind;
	pval *pgm;
	pval *rev;
	pval *flags;
	int ind,ind_type;
	unsigned long *slst,*sl;
	SORTPGM *mypgm=NIL;
	SEARCHPGM *spg=NIL;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);
	if (myargc < 3 || myargc > 4 || getParameters(ht, myargc, &streamind,&pgm,&rev,&flags) ==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(streamind);
	convert_to_long(rev);
	convert_to_long(pgm);
	if(pgm->value.lval>SORTSIZE) {
		php3_error(E_WARNING, "Unrecognized sort criteria");
		RETURN_FALSE;
	}
	if(myargc==4) convert_to_long(flags);

	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if(!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	spg = mail_newsearchpgm();
	mypgm=mail_newsortpgm();
	mypgm->reverse=rev->value.lval;
	mypgm->function=pgm->value.lval;
	mypgm->next=NIL;

	array_init(return_value);
	slst=mail_sort(imap_le_struct->imap_stream,NIL,spg,mypgm,myargc == 4 ? flags->value.lval:NIL);


	for (sl = slst; *sl; sl++) { 
		add_next_index_long(return_value,*sl);
	}
		fs_give ((void **) &slst); 

}
/* }}} */

/* {{{ proto string imap_fetchheader(int stream_id, int msg_no [, int options])
   Get the full unfiltered header for a message */
void php3_imap_fetchheader(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *msgno, *flags;
	int ind, ind_type, msgindex;
	pils *imap_le_struct;
	int myargc = ARG_COUNT(ht);
	if (myargc < 2 || myargc > 3 || getParameters(ht,myargc,&streamind,&msgno,&flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(streamind);
	convert_to_long(msgno);
	if (myargc == 3) convert_to_long(flags);
	ind = streamind->value.lval;
	
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	
	if ((myargc == 3) && (flags->value.lval & FT_UID)) {
		/* This should be cached; if it causes an extra RTT to the
           IMAP server, then that's the price we pay for making sure
           we don't crash. */
		msgindex = mail_msgno(imap_le_struct->imap_stream, msgno->value.lval);
	} else {
		msgindex = msgno->value.lval;
	}
	if ((msgindex < 1) || ((unsigned) msgindex > imap_le_struct->imap_stream->nmsgs)) {
		php3_error(E_WARNING, "Bad message number");
		RETURN_FALSE;
	}
	
	RETVAL_STRING(mail_fetchheader_full (imap_le_struct->imap_stream,msgno->value.lval,NIL,NIL,myargc == 3 ? flags->value.lval : NIL),1);
}
/* }}} */

/* {{{ proto int imap_uid(int stream_id, int msg_no)
   Get the unique message id associated with a standard sequential message number */
void php3_imap_uid(INTERNAL_FUNCTION_PARAMETERS)
{
 	pval *streamind, *msgno;
 	int ind, ind_type;
	pils *imap_le_struct;
 
 	if (ARG_COUNT(ht) != 2 || getParameters(ht,2,&streamind,&msgno) == FAILURE) {
 		WRONG_PARAM_COUNT;
 	}
 	
 	convert_to_long(streamind);
 	convert_to_long(msgno);
 
 	ind = streamind->value.lval;
 
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
 
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
	php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
 
	RETURN_LONG(mail_uid(imap_le_struct->imap_stream, msgno->value.lval));
}
/* }}} */
 
/* {{{ proto int imap_msgno(int stream_id, int unique_msg_id)
   Get the sequence number associated with a UID */
void php3_imap_msgno(INTERNAL_FUNCTION_PARAMETERS)
{
 	pval *streamind, *msgno;
 	int ind, ind_type;
	pils *imap_le_struct;
 
 	if (ARG_COUNT(ht) != 2 || getParameters(ht,2,&streamind,&msgno) == FAILURE) {
 		WRONG_PARAM_COUNT;
 	}
 	
 	convert_to_long(streamind);
 	convert_to_long(msgno);
 
 	ind = streamind->value.lval;
 
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
 
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
 
 	RETURN_LONG(mail_msgno(imap_le_struct->imap_stream, msgno->value.lval));
}
/* }}} */

/* {{{ proto object imap_status(int stream_id, string mailbox, int options)
   Get status info from a mailbox */
void php3_imap_status(INTERNAL_FUNCTION_PARAMETERS)
{
 	pval *streamind, *mbx, *flags;
 	int ind, ind_type;
	pils *imap_le_struct;
	int myargc=ARG_COUNT(ht);
 	if (myargc != 3 || getParameters(ht,myargc,&streamind,&mbx,&flags) == FAILURE) {
 		WRONG_PARAM_COUNT;
 	}
 	
 	convert_to_long(streamind);
 	convert_to_string(mbx);
	convert_to_long(flags);
 	ind = streamind->value.lval;
 
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
 
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
 
	if(object_init(return_value) == FAILURE){
	  RETURN_FALSE;
	}
 	if(mail_status(imap_le_struct->imap_stream, mbx->value.str.val, flags->value.lval))
	  {
	    add_property_long(return_value,"flags",status_flags);
	    if(status_flags & SA_MESSAGES) add_property_long(return_value,"messages",status_messages);
	    if(status_flags & SA_RECENT) add_property_long(return_value,"recent",status_recent);
	    if(status_flags & SA_UNSEEN) add_property_long(return_value,"unseen",status_unseen);
	    if(status_flags & SA_UIDNEXT) add_property_long(return_value,"uidnext",status_uidnext);
	    if(status_flags & SA_UIDVALIDITY) add_property_long(return_value,"uidvalidity",status_uidvalidity);
#ifdef SA_QUOTA
	    if(status_flags & SA_QUOTA) add_property_long(return_value,"quota",status_quota);
#endif
#ifdef SA_QUOTA
	    if(status_flags & SA_QUOTA_ALL) add_property_long(return_value,"quota_all",status_quota_all);
#endif
	  }
	else
	  {
	  RETURN_FALSE;
	  }
}
/* }}} */
 
/* {{{ proto object imap_bodystruct(int stream_id, int msg_no, int section)
   Read the structure of a specified body section of a specific message */
void php3_imap_bodystruct(INTERNAL_FUNCTION_PARAMETERS)
{
 	pval *streamind, *msg, *section;
 	int ind, ind_type;
	pils *imap_le_struct;
	pval parametres, param, dparametres, dparam;
	PARAMETER *par, *dpar;
	BODY *body;
	int myargc=ARG_COUNT(ht);
 	if (myargc != 3 || getParameters(ht,myargc,&streamind,&msg,&section) == FAILURE) {
 		WRONG_PARAM_COUNT;
 	}
 	
 	convert_to_long(streamind);
 	convert_to_long(msg);
	convert_to_string(section);
 	ind = streamind->value.lval;
 
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
	  php3_error(E_WARNING, "Unable to find stream pointer");
	  RETURN_FALSE;
	}
	
	if(object_init(return_value) == FAILURE){
	  RETURN_FALSE;
	}

	body=mail_body(imap_le_struct->imap_stream, msg->value.lval, section->value.str.val);
	if(body->type) add_property_long( return_value, "type", body->type );
	if(body->encoding) add_property_long( return_value, "encoding", body->encoding );
	
	if ( body->subtype ){
	  add_property_long( return_value, "ifsubtype", 1 );
	  add_property_string( return_value, "subtype",  body->subtype, 1 );
	} else {
	  add_property_long( return_value, "ifsubtype", 0 );
	}
	
	if ( body->description ){
	  add_property_long( return_value, "ifdescription", 1 );
	  add_property_string( return_value, "description",  body->description, 1 );
	} else {
	  add_property_long( return_value, "ifdescription", 0 );
	}
	if ( body->id ){
	  add_property_long( return_value, "ifid", 1 );
	  add_property_string( return_value, "id",  body->id, 1 );
	} else {
	  add_property_long( return_value, "ifid", 0 );
	}
	
	if(body->size.lines) add_property_long( return_value, "lines", body->size.lines );
	if(body->size.bytes) add_property_long( return_value, "bytes", body->size.bytes );
#ifdef IMAP41
	if ( body->disposition.type ){
	  add_property_long( return_value, "ifdisposition", 1);
	  add_property_string( return_value, "disposition", body->disposition.type, 1);
	} else {
	  add_property_long( return_value, "ifdisposition", 0);
	}
	
	if ( body->disposition.parameter ) {
	  dpar = body->disposition.parameter;
	  add_property_long( return_value, "ifdparameters", 1);
	  array_init( &dparametres );
	  do {
	    object_init( &dparam );
	    add_property_string( &dparam, "attribute", dpar->attribute, 1);
	    add_property_string( &dparam, "value", dpar->value, 1);
	    add_next_index_object( &dparametres, dparam );
	  } while ( (dpar = dpar->next) );
	  add_assoc_object( return_value, "dparameters", dparametres );
	} else {
	  add_property_long( return_value, "ifdparameters", 0);
	}
#endif
	
	if ( (par = body->parameter) ) {
	  add_property_long( return_value, "ifparameters", 1 );
	  
	  array_init( &parametres );
	  do {
	    object_init( &param );
	    if(par->attribute) add_property_string( &param, "attribute", par->attribute, 1 );
	    if(par->value) add_property_string( &param, "value", par->value, 1 );
	    
	    add_next_index_object( &parametres, param );
	  } while ( (par = par->next) );
	} else {
	  object_init(&parametres);
	  add_property_long( return_value, "ifparameters", 0 );
	}
	add_assoc_object( return_value, "parameters", parametres );
}
/* }}} */

/* {{{ proto array imap_fetch_overview(int stream_id, int msg_no)
   Read an overview of the information in the headers of the given message */ 
void php3_imap_fetch_overview(INTERNAL_FUNCTION_PARAMETERS)
{
 	pval *streamind, *sequence;
 	int ind, ind_type;
	pils *imap_le_struct;
	pval myoverview;
	char address[MAILTMPLEN];
	int myargc=ARG_COUNT(ht);
 	if (myargc != 2 || getParameters(ht,myargc,&streamind,&sequence) == FAILURE) {
 		WRONG_PARAM_COUNT;
 	}
 	
 	convert_to_long(streamind);
 	convert_to_string(sequence);

 	ind = streamind->value.lval;
 	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
	  php3_error(E_WARNING, "Unable to find stream pointer");
	  RETURN_FALSE;
	}
	array_init(return_value);
	if (mail_uid_sequence (imap_le_struct->imap_stream,(char *)sequence)) {
	  MESSAGECACHE *elt;
	  ENVELOPE *env;
	  unsigned long i;
	  for (i = 1; i <= imap_le_struct->imap_stream->nmsgs; i++)
	    if (((elt = mail_elt (imap_le_struct->imap_stream,i))->sequence) &&
		(env = mail_fetch_structure (imap_le_struct->imap_stream,i,NIL,NIL))) {
	      object_init(&myoverview);
	      add_property_string(&myoverview,"subject",env->subject,1);
	      env->from->next=NULL;
	      rfc822_write_address(address,env->from);
	      add_property_string(&myoverview,"from",address,1);
	      add_property_string(&myoverview,"date",env->date,1);
	      add_property_string(&myoverview,"message_id",env->message_id,1);
	      add_property_string(&myoverview,"references",env->references,1);
	      add_property_long(&myoverview,"size",elt->rfc822_size);
	      add_property_long(&myoverview,"uid",mail_uid(imap_le_struct->imap_stream,i));
	      add_property_long(&myoverview,"msgno",i);
	      add_property_long(&myoverview,"recent",elt->recent);
	      add_property_long(&myoverview,"flagged",elt->flagged);
	      add_property_long(&myoverview,"answered",elt->answered);
	      add_property_long(&myoverview,"deleted",elt->deleted);
	      add_property_long(&myoverview,"seen",elt->seen);
	      add_property_long(&myoverview,"draft",elt->draft);
	      add_next_index_object(return_value,myoverview);
	    }
	}
}
/* }}} */

/* {{{ proto string imap_mail_compose(array envelope, array body)
   Create a MIME message based on given envelope and body sections */
void php3_imap_mail_compose(INTERNAL_FUNCTION_PARAMETERS)
{
  pval *envelope, *body;
  char *key;
  pval *data,*pvalue;
  ulong ind;
  char *cookie = NIL;
  ENVELOPE *env;
  BODY *bod=NULL,*topbod=NULL;
  PART *mypart=NULL,*toppart=NULL,*part;
  PARAMETER *param;
  char tmp[8*MAILTMPLEN],*mystring=NULL,*t,*tempstring;
  int myargc=ARG_COUNT(ht);
  if (myargc != 2 || getParameters(ht,myargc,&envelope,&body) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  convert_to_array(envelope);
  convert_to_array(body);
  env=mail_newenvelope();
  if(_php3_hash_find(envelope->value.ht,"remail",sizeof("remail"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    env->remail=cpystr(pvalue->value.str.val);
  }
  
  if(_php3_hash_find(envelope->value.ht,"return_path",sizeof("return_path"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->return_path,pvalue->value.str.val,"NO HOST");
  }
  if(_php3_hash_find(envelope->value.ht,"date",sizeof("date"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    env->date=cpystr(pvalue->value.str.val);
  }
  if(_php3_hash_find(envelope->value.ht,"from",sizeof("from"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->from,pvalue->value.str.val,"NO HOST");
  }
  if(_php3_hash_find(envelope->value.ht,"reply_to",sizeof("reply_to"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->reply_to,pvalue->value.str.val,"NO HOST");
  }
  if(_php3_hash_find(envelope->value.ht,"to",sizeof("to"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->to,pvalue->value.str.val,"NO HOST");
  }
  if(_php3_hash_find(envelope->value.ht,"cc",sizeof("cc"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->cc,pvalue->value.str.val,"NO HOST");
  }
  if(_php3_hash_find(envelope->value.ht,"bcc",sizeof("bcc"),(void **) &pvalue)== SUCCESS){
    convert_to_string(pvalue);
    rfc822_parse_adrlist (&env->bcc,pvalue->value.str.val,"NO HOST");
  }
 if(_php3_hash_find(envelope->value.ht,"message_id",sizeof("message_id"),(void **) &pvalue)== SUCCESS){
   convert_to_string(pvalue);
   env->message_id=cpystr(pvalue->value.str.val);
 }
 
 _php3_hash_internal_pointer_reset(body->value.ht);
 _php3_hash_get_current_key(body->value.ht, &key, &ind);
 _php3_hash_get_current_data(body->value.ht, (void **) &data);
 if(data->type == IS_ARRAY)
   {
     bod=mail_newbody();
     topbod=bod;
     if(_php3_hash_find(data->value.ht,"type",sizeof("type"),(void **) &pvalue)== SUCCESS){
       convert_to_long(pvalue);
       bod->type=pvalue->value.lval;
     }
     if(_php3_hash_find(data->value.ht,"encoding",sizeof("encoding"),(void **) &pvalue)== SUCCESS){
       convert_to_long(pvalue);
       bod->encoding=pvalue->value.lval;
     }
     if(_php3_hash_find(data->value.ht,"subtype",sizeof("subtype"),(void **) &pvalue)== SUCCESS){
       convert_to_string(pvalue);
       bod->subtype=cpystr(pvalue->value.str.val);
     }
     if(_php3_hash_find(data->value.ht,"id",sizeof("id"),(void **) &pvalue)== SUCCESS){
       convert_to_string(pvalue);
       bod->id=cpystr(pvalue->value.str.val);
     }
     if(_php3_hash_find(data->value.ht,"contents.data",sizeof("contents.data"),(void **) &pvalue)== SUCCESS){
       convert_to_string(pvalue);     
       bod->contents.text.data=(char *)fs_get(pvalue->value.str.len);
       memcpy(bod->contents.text.data,pvalue->value.str.val,pvalue->value.str.len+1);
       bod->contents.text.size=pvalue->value.str.len;
     }
     if(_php3_hash_find(data->value.ht,"lines",sizeof("lines"),(void **) &pvalue)== SUCCESS){
       convert_to_long(pvalue);
       bod->size.lines=pvalue->value.lval;
     }
     if(_php3_hash_find(data->value.ht,"bytes",sizeof("bytes"),(void **) &pvalue)== SUCCESS){
       convert_to_long(pvalue);
       bod->size.bytes=pvalue->value.lval;
     }
     if(_php3_hash_find(data->value.ht,"md5",sizeof("md5"),(void **) &pvalue)== SUCCESS){
       convert_to_string(pvalue);
       bod->md5=cpystr(pvalue->value.str.val);
     }
   }
 _php3_hash_move_forward(body->value.ht);
 while(_php3_hash_get_current_data(body->value.ht, (void **) &data)==SUCCESS)
   {
     _php3_hash_get_current_key(body->value.ht, &key, &ind);
     if(data->type == IS_ARRAY)
       {
	 if(!toppart)
	   {		     
	     bod->nested.part=mail_newbody_part();
	     mypart=bod->nested.part;
	     toppart=mypart;
	     bod=&mypart->body;
	   }
	 else
	   {
	     mypart->next=mail_newbody_part();
	     mypart=mypart->next;
	     bod=&mypart->body;
	   }
	 if(_php3_hash_find(data->value.ht,"type",sizeof("type"),(void **) &pvalue)== SUCCESS){
	   convert_to_long(pvalue);
	   bod->type=pvalue->value.lval;
	 }
	 if(_php3_hash_find(data->value.ht,"encoding",sizeof("encoding"),(void **) &pvalue)== SUCCESS){
	   convert_to_long(pvalue);
	   bod->encoding=pvalue->value.lval;
	 }
	 if(_php3_hash_find(data->value.ht,"subtype",sizeof("subtype"),(void **) &pvalue)== SUCCESS){
	   convert_to_string(pvalue);
	   bod->subtype=cpystr(pvalue->value.str.val);
	 }
	 if(_php3_hash_find(data->value.ht,"id",sizeof("id"),(void **) &pvalue)== SUCCESS){
	   convert_to_string(pvalue);
	   bod->id=cpystr(pvalue->value.str.val);
	 }
	 if(_php3_hash_find(data->value.ht,"contents.data",sizeof("contents.data"),(void **) &pvalue)== SUCCESS){
	   convert_to_string(pvalue);     
	   bod->contents.text.data=(char *)fs_get(pvalue->value.str.len);
	   memcpy(bod->contents.text.data,pvalue->value.str.val,pvalue->value.str.len+1);
	   bod->contents.text.size=pvalue->value.str.len;
	 }
	 if(_php3_hash_find(data->value.ht,"lines",sizeof("lines"),(void **) &pvalue)== SUCCESS){
	   convert_to_long(pvalue);
	   bod->size.lines=pvalue->value.lval;
	 }
	 if(_php3_hash_find(data->value.ht,"bytes",sizeof("bytes"),(void **) &pvalue)== SUCCESS){
	   convert_to_long(pvalue);
	   bod->size.bytes=pvalue->value.lval;
	 }
	 if(_php3_hash_find(data->value.ht,"md5",sizeof("md5"),(void **) &pvalue)== SUCCESS){
	   convert_to_string(pvalue);
	   bod->md5=cpystr(pvalue->value.str.val);
	 }
	 _php3_hash_move_forward(body->value.ht);
       }
   }

  rfc822_encode_body_7bit (env,topbod); 
  rfc822_header (tmp,env,topbod);  
  mystring=emalloc(strlen(tmp)+1);
  strcpy(mystring,tmp);

  bod=topbod;
  switch (bod->type) {
  case TYPEMULTIPART:           /* multipart gets special handling */
    part = bod->nested.part;   /* first body part */
                                /* find cookie */
    for (param = bod->parameter; param && !cookie; param = param->next)
      if (!strcmp (param->attribute,"BOUNDARY")) cookie = param->value;
    if (!cookie) cookie = "-";  /* yucky default */
    do {                        /* for each part */
                                /* build cookie */
      sprintf (t=tmp,"--%s\015\012",cookie);
                                /* append mini-header */
      rfc822_write_body_header (&t,&part->body);
      strcat (t,"\015\012");    /* write terminating blank line */
                                /* output cookie, mini-header, and contents */
      tempstring=emalloc(strlen(mystring)+strlen(tmp)+1);
      strcpy(tempstring,mystring);
      efree(mystring);
      mystring=tempstring;
      strcat(mystring,tmp);

      bod=&part->body;

      tempstring=emalloc(strlen(bod->contents.text.data)+strlen(mystring)+1);
      strcpy(tempstring,mystring);
      efree(mystring);
      mystring=tempstring;
      strcat(mystring,bod->contents.text.data);

    } while ((part = part->next));/* until done */
                                /* output trailing cookie */

    sprintf(tmp,"--%s--",cookie);
    tempstring=emalloc(strlen(tmp)+strlen(mystring)+1);
    strcpy(tempstring,mystring);
    efree(mystring);
    mystring=tempstring;
    strcat(mystring,tmp);
    break;
  default:                      /* all else is text now */
    tempstring=emalloc(strlen(bod->contents.text.data)+strlen(mystring)+1);
    strcpy(tempstring,mystring);
    efree(mystring);
    mystring=tempstring;
    strcat(mystring,bod->contents.text.data);
    break;
  }
  
  RETURN_STRINGL(mystring,strlen(mystring),1);  
}
/* }}} */


/* {{{ proto string imap_alerts(void)
   Returns an array of all IMAP alerts that have been generated */
/* Returns an array of all IMAP alerts that have been generated either
   since the last page load, or since the last imap_alerts() call,
   whichever came last. The alert stack is cleared after imap_alerts()
   is called. */
/* Author: CJH */
void php3_imap_alerts(INTERNAL_FUNCTION_PARAMETERS)
{
  STRINGLIST *cur=NIL;
  
  int arg_count = ARG_COUNT(ht);
  
  if (arg_count > 0) {
    WRONG_PARAM_COUNT;
  }
  
  if (imap_alertstack == NIL) {
    RETURN_FALSE;
  }
  
  array_init(return_value);
  cur = imap_alertstack;
  while (cur != NIL) {
    add_next_index_string(return_value,cur->LTEXT,1);
    cur = cur->next;
  }
  mail_free_stringlist(&imap_alertstack);
  imap_alertstack = NIL;
}
/* }}} */


/* {{{ proto array imap_errors(void)
   Returns an array of all IMAP errors generated */
/* Returns an array of all IMAP errors generated either since the last
   page load, or since the last imap_errors() call, whichever came
   last. The error stack is cleared after imap_errors() is called. */
/* Author: CJH */
void php3_imap_errors(INTERNAL_FUNCTION_PARAMETERS)
{
  ERRORLIST *cur=NIL;
  
  int arg_count = ARG_COUNT(ht);
  
  if (arg_count > 0) {
    WRONG_PARAM_COUNT;
  }
  
  if (imap_errorstack == NIL) {
    RETURN_FALSE;
  }
  
  array_init(return_value);
  cur = imap_errorstack;
  while (cur != NIL) {
    add_next_index_string(return_value,cur->LTEXT,1);
    cur = cur->next;
  }
  mail_free_errorlist(&imap_errorstack);
  imap_errorstack = NIL;
}
/* }}} */


/* {{{ proto string imap_last_error(void) 
   Returns the last error that was generated by an IMAP function. The error stack is NOT cleared after this call. */
/* Author: CJH */
void php3_imap_last_error(INTERNAL_FUNCTION_PARAMETERS)
{
  ERRORLIST *cur=NIL;
  
  int arg_count = ARG_COUNT(ht);
  
  if (arg_count > 0) {
    WRONG_PARAM_COUNT;
  }
  
  if (imap_errorstack == NIL) {
    RETURN_FALSE;
  }
  
  cur = imap_errorstack;
  while (cur != NIL) {
    if (cur->next == NIL) {
      RETURN_STRING(cur->LTEXT, 1);
    }
    cur = cur->next;
  }
}
/* }}} */


/* {{{ proto int imap_mail(string to, string subject, string message [, string additional_headers [, string cc [, string bcc [, string rpath]]]])
   Send an email message */
void php3_imap_mail(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *argv[6];
	char *to=NULL, *message=NULL, *headers=NULL, *subject=NULL, *cc=NULL, *bcc=NULL, *rpath=NULL;
	int argc;
	TLS_VARS;
	
	argc = ARG_COUNT(ht);
	if (argc < 3 || argc > 7 || getParametersArray(ht, argc, argv) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	/* To: */
	convert_to_string(argv[0]);
	if (argv[0]->value.str.val) {
		to = argv[0]->value.str.val;
	} else {
		php3_error(E_WARNING, "No to field in mail command");
		RETURN_FALSE;
	}

	/* Subject: */
	convert_to_string(argv[1]);
	if (argv[1]->value.str.val) {
		subject = argv[1]->value.str.val;
	} else {
		php3_error(E_WARNING, "No subject field in mail command");
		RETURN_FALSE;
	}

	/* message body */
	convert_to_string(argv[2]);
	if (argv[2]->value.str.val) {
		message = argv[2]->value.str.val;
	} else {
		/* this is not really an error, so it is allowed. */
		php3_error(E_WARNING, "No message string in mail command");
		message = NULL;
	}

	/* other headers */
	if (argc > 3) {
		convert_to_string(argv[3]);
		headers = argv[3]->value.str.val;
	}
	
	/* cc */
	if (argc > 4) {
		convert_to_string(argv[4]);
		cc = argv[4]->value.str.val;
	}

	/* bcc */
	if (argc > 5) {
		convert_to_string(argv[5]);
		bcc = argv[5]->value.str.val;
	}

	/* rpath */
	if (argc > 6) {
		convert_to_string(argv[6]);
		rpath = argv[6]->value.str.val;
	}

	if (_php3_imap_mail(to, subject, message, headers, cc, bcc, rpath)){
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

int _php3_imap_mail(char *to, char *subject, char *message, char *headers, char *cc, char *bcc, char* rpath)
{
#if MSVC5
	int tsm_err;
#else
	FILE *sendmail;
	int ret;
#endif
	TLS_VARS;

#if MSVC5
	if (imap_TSendMail(php3_ini.smtp,&tsm_err,headers,subject,to,message,cc,bcc,rpath) != SUCCESS){
		php3_error(E_WARNING, "%s", GetSMErrorText(tsm_err));
		return 0;
	}
#else
	if (!php3_ini.sendmail_path) {
		return 0;
	}
	sendmail = popen(php3_ini.sendmail_path, "w");
	if (sendmail) {
		fprintf(sendmail, "To: %s\n", to);
		if (cc && cc[0]) fprintf(sendmail, "Cc: %s\n", cc);
		if (bcc && bcc[0]) fprintf(sendmail, "Bcc: %s\n", bcc);
		fprintf(sendmail, "Subject: %s\n", subject);
		if (headers != NULL) {
			fprintf(sendmail, "%s\n", headers);
		}
		fprintf(sendmail, "\n%s\n", message);
		ret = pclose(sendmail);
		if (ret == -1) {
			return 0;
		} else {
			return 1;
		}
	} else {
		php3_error(E_WARNING, "Could not execute mail delivery program");
		return 0;
	}
#endif
	return 1;
}

/* {{{ proto array imap_search(int stream_id, string criteria [, long flags])
   Return a list of messages matching the criteria */
void php3_imap_search(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *streamind, *criteria, *search_flags;
	int ind, ind_type, args;
	pils *imap_le_struct;
	long flags;
	MESSAGELIST *cur;
    
	args = ARG_COUNT(ht);
	if (args < 2 || args > 3 || getParameters(ht, args, &streamind, &criteria, &search_flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(streamind);
	convert_to_string(criteria);
	
	if (args == 2) {
		flags = SE_FREE;
	} else {
		convert_to_long(search_flags);
		flags = search_flags->value.lval;
	}
	
	ind = streamind->value.lval;
	imap_le_struct = (pils *)php3_list_find(ind, &ind_type);
	if (!imap_le_struct || !IS_STREAM(ind_type)) {
		php3_error(E_WARNING, "Unable to find stream pointer");
		RETURN_FALSE;
	}
	
	imap_messages = NIL;
	mail_search_full(imap_le_struct->imap_stream, NIL, mail_criteria(criteria->value.str.val), flags);
	if (imap_messages == NIL) {
		RETURN_FALSE;
	}
	
	array_init(return_value);
	cur = imap_messages;
	while (cur != NIL) {
		add_next_index_long(return_value, cur->msgid);
		cur = cur->next;
	}
	mail_free_messagelist(&imap_messages);
}
/* }}} */

/* Interfaces to C-client */


void mm_searched (MAILSTREAM *stream,unsigned long number)
{
	MESSAGELIST *cur = NIL;
	
	if (imap_messages == NIL) {
		imap_messages = mail_newmessagelist();
		imap_messages->msgid = number;
		imap_messages->next = NIL;
	} else {
		cur = imap_messages;
		while (cur->next != NIL) {
			cur = cur->next;
		}
		cur->next = mail_newmessagelist();
		cur = cur->next;
		cur->msgid = number;
		cur->next = NIL;
	}
}


void mm_exists (MAILSTREAM *stream,unsigned long number)
{
}


void mm_expunged (MAILSTREAM *stream,unsigned long number)
{
}


void mm_flags (MAILSTREAM *stream,unsigned long number)
{
}


/* Author: CJH */
void mm_notify (MAILSTREAM *stream,char *string, long errflg)
{
  STRINGLIST *cur = NIL;
  
  if (strncmp(string, "[ALERT] ", 8) == 0) {
    if (imap_alertstack == NIL) {
      imap_alertstack = mail_newstringlist();
      imap_alertstack->LSIZE = strlen(imap_alertstack->LTEXT = cpystr(string));
      imap_alertstack->next = NIL; 
    } else {
      cur = imap_alertstack;
      while (cur->next != NIL ) {
	cur = cur->next;
      }
      cur->next = mail_newstringlist ();
      cur = cur->next;
      cur->LSIZE = strlen(cur->LTEXT = cpystr(string));
      cur->next = NIL;
    }
  }
}

void mm_list (MAILSTREAM *stream,DTYPE delimiter,char *mailbox,long attributes)
{
	STRINGLIST *cur=NIL;
	FOBJECTLIST *ocur=NIL;
	
	if (folderlist_style == FLIST_OBJECT) {
		/* build up a the new array of objects */
		/* Author: CJH */
		if (imap_folder_objects == NIL) {
			imap_folder_objects = mail_newfolderobjectlist();
			imap_folder_objects->LSIZE=strlen(imap_folder_objects->LTEXT=cpystr(mailbox));
			imap_folder_objects->delimiter = delimiter;
			imap_folder_objects->attributes = attributes;
			imap_folder_objects->next = NIL;
		} else {
			ocur=imap_folder_objects;
			while (ocur->next != NIL) {
				ocur=ocur->next;
			}
			ocur->next=mail_newfolderobjectlist();
			ocur=ocur->next;
			ocur->LSIZE = strlen(ocur->LTEXT = cpystr(mailbox));
			ocur->delimiter = delimiter;
			ocur->attributes = attributes;
			ocur->next = NIL;
		}
		
	} else {
		/* build the old imap_folders variable to allow old imap_listmailbox() to work */
		if (!(attributes & LATT_NOSELECT)) {
			if (imap_folders == NIL) {
				imap_folders=mail_newstringlist();
				imap_folders->LSIZE=strlen(imap_folders->LTEXT=cpystr(mailbox));
				imap_folders->next=NIL; 
			} else {
				cur=imap_folders;
				while (cur->next != NIL ) {
					cur=cur->next;
				}
				cur->next=mail_newstringlist ();
				cur=cur->next;
				cur->LSIZE = strlen (cur->LTEXT = cpystr (mailbox));
				cur->next = NIL;
			}
		}
	}
}

void mm_lsub (MAILSTREAM *stream,DTYPE delimiter,char *mailbox,long attributes)
{
	STRINGLIST *cur=NIL;
	FOBJECTLIST *ocur=NIL;
	
	if (folderlist_style == FLIST_OBJECT) {
		/* build the array of objects */
		/* Author: CJH */
		if (imap_sfolder_objects == NIL) {
			imap_sfolder_objects = mail_newfolderobjectlist();
			imap_sfolder_objects->LSIZE=strlen(imap_sfolder_objects->LTEXT=cpystr(mailbox));
			imap_sfolder_objects->delimiter = delimiter;
			imap_sfolder_objects->attributes = attributes;
			imap_sfolder_objects->next = NIL;
		} else {
			ocur=imap_sfolder_objects;
			while (ocur->next != NIL) {
				ocur=ocur->next;
			}
			ocur->next=mail_newfolderobjectlist();
			ocur=ocur->next;
			ocur->LSIZE=strlen(ocur->LTEXT = cpystr(mailbox));
			ocur->delimiter = delimiter;
			ocur->attributes = attributes;
			ocur->next = NIL;
		}
		
	} else {
		/* build the old simple array for imap_listsubscribed() */
		if (imap_sfolders == NIL) {
			imap_sfolders=mail_newstringlist();
			imap_sfolders->LSIZE=strlen(imap_sfolders->LTEXT=cpystr(mailbox));
			imap_sfolders->next=NIL; 
		} else {
			cur=imap_sfolders;
			while (cur->next != NIL ) {
				cur=cur->next;
			}
			cur->next=mail_newstringlist ();
			cur=cur->next;
			cur->LSIZE = strlen (cur->LTEXT = cpystr (mailbox));
			cur->next = NIL;
		}
	}
}

void mm_status (MAILSTREAM *stream,char *mailbox,MAILSTATUS *status)
{
  status_flags=status->flags;
  if(status_flags & SA_MESSAGES) status_messages=status->messages;
  if(status_flags & SA_RECENT) status_recent=status->recent;
  if(status_flags & SA_UNSEEN) status_unseen=status->unseen;
  if(status_flags & SA_UIDNEXT) status_uidnext=status->uidnext;
  if(status_flags & SA_UIDVALIDITY) status_uidvalidity=status->uidvalidity;
#ifdef SA_QUOTA
  if(status_flags & SA_QUOTA) status_quota=status->quota;
#endif
#ifdef SA_QUOTA_ALL
  if(status_flags & SA_QUOTA_ALL) status_quota_all=status->quota_all;
#endif
}

void mm_log (char *string,long errflg)
{
  ERRORLIST *cur = NIL;
  
  /* Author: CJH */
  if (errflg != NIL) { /* CJH: maybe put these into a more comprehensive log for debugging purposes? */
    if (imap_errorstack == NIL) {
      imap_errorstack = mail_newerrorlist();
      imap_errorstack->LSIZE = strlen(imap_errorstack->LTEXT = cpystr(string));
      imap_errorstack->errflg = errflg;
      imap_errorstack->next = NIL; 
    } else {
      cur = imap_errorstack;
      while (cur->next != NIL ) {
	cur = cur->next;
      }
      cur->next = mail_newerrorlist ();
      cur = cur->next;
      cur->LSIZE = strlen(cur->LTEXT = cpystr(string));
      cur->errflg = errflg;
      cur->next = NIL;
    }
  }
}

void mm_dlog (char *string)
{
  /* CJH: this is for debugging; it might be useful to allow setting
     the stream to debug mode and capturing this somewhere - syslog?
     php debugger? */
}

/* CJH: check the service name in order to allow different IMSP
   usernames & passwords */
void mm_login (NETMBX *mb,char *user,char *pwd,long trial)
{
#if HAVE_IMSP
	if (*mb->service && strcmp(mb->service, "imsp") == 0) {
		if (*mb->user) {
			strcpy(user, mb->user);
		} else {
			strcpy(user, imsp_user);
		}
		strcpy (pwd,imsp_password);
	} else {
#endif
		if (*mb->user) {
			strcpy(user, mb->user);
		} else {
			strcpy(user, imap_user);
		}
		strcpy(pwd, imap_password);
#if HAVE_IMSP
	}
#endif
}


void mm_critical (MAILSTREAM *stream)
{
}


void mm_nocritical (MAILSTREAM *stream)
{
}


long mm_diskerror (MAILSTREAM *stream,long errcode,long serious)
{
	return 1;
}


void mm_fatal (char *string)
{
}

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
