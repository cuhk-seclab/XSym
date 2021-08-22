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
   | Authors: Uwe Steinmann                                               |
   |                                                                      |
   +----------------------------------------------------------------------+
 */

/* $Id: hw.c,v 1.56 2000/06/12 13:15:41 eschmid Exp $ */

#if COMPILE_DL
#include "dl/phpdl.h"
#endif

#include "php.h"
#include "internal_functions.h"
#include "php3_list.h"

#if HYPERWAVE
#include "hw.h"

#if APACHE
#  ifndef DEBUG
#  undef palloc
#  endif
#endif

#define HW_GLOBAL(a) a
#define HW_TLS_VARS

hw_module php3_hw_module;

function_entry hw_functions[] = {
	PHP_FE(hw_connect, NULL)
	PHP_FE(hw_pconnect, NULL)
	PHP_FE(hw_close, NULL)
	PHP_FE(hw_root, NULL)
	PHP_FE(hw_info, NULL)
	PHP_FE(hw_connection_info, NULL)
	PHP_FE(hw_error, NULL)
	PHP_FE(hw_errormsg, NULL)
	PHP_FE(hw_getparentsobj, NULL)
	PHP_FE(hw_getparents, NULL)
	PHP_FE(hw_children, NULL)
	PHP_FE(hw_childrenobj, NULL)
	PHP_FE(hw_getchildcoll, NULL)
	PHP_FE(hw_getchildcollobj, NULL)
	PHP_FE(hw_getobject, NULL)
	PHP_FE(hw_getandlock, NULL)
	PHP_FE(hw_unlock, NULL)
	PHP_FE(hw_gettext, NULL)
	PHP_FE(hw_edittext, NULL)
	PHP_FE(hw_getcgi, NULL)
	PHP_FE(hw_getremote, NULL)
	PHP_FE(hw_getremotechildren, NULL)
	PHP_FE(hw_pipedocument, NULL)
	PHP_FE(hw_pipecgi, NULL)
	PHP_FE(hw_insertdocument, NULL)
	PHP_FE(hw_mv, NULL)
	PHP_FE(hw_cp, NULL)
	PHP_FE(hw_deleteobject, NULL)
	PHP_FE(hw_changeobject, NULL)
	PHP_FE(hw_modifyobject, NULL)
	PHP_FE(hw_docbyanchor, NULL)
	PHP_FE(hw_docbyanchorobj, NULL)
	PHP_FE(hw_getobjectbyquery, NULL)
	PHP_FE(hw_getobjectbyqueryobj, NULL)
	PHP_FE(hw_getobjectbyquerycoll, NULL)
	PHP_FE(hw_getobjectbyquerycollobj, NULL)
	PHP_FE(hw_getchilddoccoll, NULL)
	PHP_FE(hw_getchilddoccollobj, NULL)
	PHP_FE(hw_getanchors, NULL)
	PHP_FE(hw_getanchorsobj, NULL)
	PHP_FE(hw_getusername, NULL)
	PHP_FE(hw_setlinkroot, NULL)
	PHP_FE(hw_identify, NULL)
	PHP_FE(hw_free_document, NULL)
	PHP_FE(hw_new_document, NULL)
	PHP_FE(hw_output_document, NULL)
	{"hw_outputdocument",		php3_hw_output_document,	NULL},
	PHP_FE(hw_document_size, NULL)
	{"hw_documentsize",		php3_hw_document_size,		NULL},
	PHP_FE(hw_document_attributes, NULL)
	{"hw_documentattributes",	php3_hw_document_attributes,	NULL},
	PHP_FE(hw_document_bodytag, NULL)
	{"hw_documentbodytag",		php3_hw_document_bodytag,	NULL},
	PHP_FE(hw_document_content, NULL)
	{"hw_documentcontent",		php3_hw_document_content,	NULL},
	{"hw_documentsetcontent",	php3_hw_document_setcontent,	NULL},
	PHP_FE(hw_objrec2array, NULL)
	PHP_FE(hw_array2objrec, NULL)
	PHP_FE(hw_incollections, NULL)
	PHP_FE(hw_inscoll, NULL)
	PHP_FE(hw_insertobject, NULL)
	PHP_FE(hw_insdoc, NULL)
	PHP_FE(hw_getsrcbydestobj, NULL)
	PHP_FE(hw_getrellink, NULL)
	PHP_FE(hw_who, NULL)
	PHP_FE(hw_stat, NULL)
	PHP_FE(hw_mapid, NULL)
	PHP_FE(hw_dummy, NULL)
	{NULL, NULL, NULL}
};

php3_module_entry hw_module_entry = {
	"HyperWave", hw_functions, php3_minit_hw, NULL, NULL, NULL, php3_info_hw, 0, 0, 0, NULL
};

void print_msg(hg_msg *msg, char *str, int txt);

#if COMPILE_DL
DLEXPORT php3_module_entry *get_module(void) { return &hw_module_entry; }
#endif

void _close_hw_link(hw_connection *conn)
{
	if(conn->hostname)
		free(conn->hostname);
	if(conn->username)
		free(conn->username);
	close(conn->socket);
	free(conn);
	php3_hw_module.num_links--;
}

void _close_hw_plink(hw_connection *conn)
{
	if(conn->hostname)
		free(conn->hostname);
	if(conn->username)
		free(conn->username);
	close(conn->socket);
	free(conn);
	php3_hw_module.num_links--;
	php3_hw_module.num_persistent--;
}

void _free_hw_document(hw_document *doc)
{
	if(doc->data)
		free(doc->data);
	if(doc->attributes)
		free(doc->attributes);
	if(doc->bodytag)
		free(doc->bodytag);
	free(doc);
}

/* creates an array in return value and frees all memory
 * Also adds as an assoc. array at the end of the return array with
 * statistics.
 */
int make_return_objrec(pval **return_value, char **objrecs, int count)
{
	pval stat_arr;
	int i;
	int hidden, collhead, fullcollhead, total;
        int collheadnr, fullcollheadnr;

	if (array_init(*return_value) == FAILURE) {
		/* Ups, failed! Let's at least free the memory */
		for(i=0; i<count; i++)
			efree(objrecs[i]);
		efree(objrecs);
		return -1;
	}

	hidden = collhead = fullcollhead = total = 0;
        collheadnr = fullcollheadnr = -1;
	for(i=0; i<count; i++) {
		/* Fill the array with entries. No need to free objrecs[i], since
		 * it is not duplicated in add_next_index_string().
		 */
		if(NULL != objrecs[i]) {
			if(0 == fnAttributeCompare(objrecs[i], "PresentationHints", "Hidden"))
				hidden++;
			if(0 == fnAttributeCompare(objrecs[i], "PresentationHints", "CollectionHead")) {
				collhead++;
				collheadnr = total;
			}
			if(0 == fnAttributeCompare(objrecs[i], "PresentationHints", "FullCollectionHead")) {
				fullcollhead++;
				fullcollheadnr = total;
			}
			total++;
			add_next_index_string(*return_value, objrecs[i], 0);
		}
	}
	efree(objrecs);

	/* Array for statistics */
	if (array_init(&stat_arr) == FAILURE) {
		return -1;
	}

	add_assoc_long(&stat_arr, "Hidden", hidden);
	add_assoc_long(&stat_arr, "CollectionHead", collhead);
	add_assoc_long(&stat_arr, "FullCollectionHead", fullcollhead);
	add_assoc_long(&stat_arr, "Total", total);
	add_assoc_long(&stat_arr, "CollectionHeadNr", collheadnr);
	add_assoc_long(&stat_arr, "FullCollectionHeadNr", fullcollheadnr);

	/* Add the stat array */
	_php3_hash_next_index_insert((*return_value)->value.ht, &stat_arr, sizeof(pval), NULL);

	/* The title array can now be freed, but I don't know how */
	return 0;
}

/*
** creates an array return value from object record
*/
int make_return_array_from_objrec(pval **return_value, char *objrec) {
	char *attrname, *str, *temp, language[3], *title;
	int iTitle, iDesc, iKeyword, iGroup;
	pval title_arr;
	pval desc_arr;
	pval keyword_arr;
	pval group_arr;
	int hasTitle = 0;
	int hasDescription = 0;
	int hasKeyword = 0;
	int hasGroup = 0;

	if (array_init(*return_value) == FAILURE) {
		(*return_value)->type = IS_STRING;
		(*return_value)->value.str.val = empty_string;
		(*return_value)->value.str.len = 0;
		return -1;
	}

	/* Fill Array of titles, descriptions and keywords */
	temp = estrdup(objrec);
	attrname = strtok(temp, "\n");
	while(attrname != NULL) {
		str = attrname;
		iTitle = 0;
		iDesc = 0;
		iKeyword = 0;
		iGroup = 0;
		if(0 == strncmp(attrname, "Title=", 6)) {
			if ((hasTitle == 0) && (array_init(&title_arr) == FAILURE)) {
				return -1;
			}
			hasTitle = 1;
			str += 6;
			iTitle = 1;
		} else if(0 == strncmp(attrname, "Description=", 12)) {
			if ((hasDescription == 0) && (array_init(&desc_arr) == FAILURE)) {
				return -1;
			}
			hasDescription = 1;
			str += 12;
			iDesc = 1;
		} else if(0 == strncmp(attrname, "Keyword=", 8)) {
			if ((hasKeyword == 0) && (array_init(&keyword_arr) == FAILURE)) {
				return -1;
			}
			hasKeyword = 1;
			str += 8;
			iKeyword = 1;
		} else if(0 == strncmp(attrname, "Group=", 6)) {
			if ((hasGroup == 0) && (array_init(&group_arr) == FAILURE)) {
				return -1;
			}
			hasGroup = 1;
			str += 6;
			iGroup = 1;
		} 
		if(iTitle || iDesc || iKeyword) {	/* Poor error check if end of string */
			if(str[2] == ':') {
				str[2] = '\0';
				strcpy(language, str);
				str += 3;
			} else
				strcpy(language, "xx");

			title = str;
			if(iTitle)
				add_assoc_string(&title_arr, language, title, 1);
			else if(iDesc)
				add_assoc_string(&desc_arr, language, title, 1);
			else if(iKeyword)
				add_assoc_string(&keyword_arr, language, title, 1);
		} else if(iGroup) {
			if(iGroup)
				add_next_index_string(&group_arr, str, 1);
    }

		attrname = strtok(NULL, "\n");
	}
	efree(temp);

	/* Add the title array, if we have one */
	if(hasTitle) {
		_php3_hash_update((*return_value)->value.ht, "Title", 6, &title_arr, sizeof(pval), NULL);

		/* The title array can now be freed, but I don't know how */
	}

	if(hasDescription) {
	/* Add the description array, if we have one */
		_php3_hash_update((*return_value)->value.ht, "Description", 12, &desc_arr, sizeof(pval), NULL);

		/* The description array can now be freed, but I don't know how */
	}

	if(hasKeyword) {
	/* Add the keyword array, if we have one */
		_php3_hash_update((*return_value)->value.ht, "Keyword", 8, &keyword_arr, sizeof(pval), NULL);

		/* The keyword array can now be freed, but I don't know how */
	}

	if(hasGroup) {
	/* Add the Group array, if we have one */
		_php3_hash_update((*return_value)->value.ht, "Group", 6, &group_arr, sizeof(pval), NULL);

		/* The group array can now be freed, but I don't know how */
	}

	/* All other attributes. Make a another copy first */
	temp = estrdup(objrec);
	attrname = strtok(temp, "\n");
	while(attrname != NULL) {
		str = attrname;
		/* We don't want to insert titles, descr., keywords a second time */
		if((0 != strncmp(attrname, "Title=", 6)) &&
		   (0 != strncmp(attrname, "Description=", 12)) &&
		   (0 != strncmp(attrname, "Group=", 6)) &&
		   (0 != strncmp(attrname, "Keyword=", 8))) {
			while((*str != '=') && (*str != '\0'))
				str++;
			*str = '\0';
			str++;
			add_assoc_string(*return_value, attrname, str, 1);
		}
		attrname = strtok(NULL, "\n");
	}
	efree(temp);

	return(0);
}

#define BUFFERLEN 1024
static char * make_objrec_from_array(HashTable *lht) {
	int i, count, keytype;
	ulong length;
	char *key, str[BUFFERLEN], *objrec = NULL;
	pval *keydata;

	if(NULL == lht)
		return NULL;

	if(0 == (count = _php3_hash_num_elements(lht)))
		return NULL;
	
	_php3_hash_internal_pointer_reset(lht);
	objrec = malloc(1);
	*objrec = '\0';
	for(i=0; i<count; i++) {
		keytype = _php3_hash_get_current_key(lht, &key, &length);
//		if(HASH_KEY_IS_STRING == keytype) {
			_php3_hash_get_current_data(lht, (void **) &keydata);
			switch(keydata->type) {
				case IS_STRING:
					if(HASH_KEY_IS_STRING == keytype)
						snprintf(str, BUFFERLEN, "%s=%s\n", key, keydata->value.str.val);
					else
						snprintf(str, BUFFERLEN, "%s\n", keydata->value.str.val);
					break;
				case IS_LONG:
					if(HASH_KEY_IS_STRING == keytype)
						snprintf(str, BUFFERLEN, "%s=0x%lX\n", key, keydata->value.lval);
					else
						snprintf(str, BUFFERLEN, "0x%lX\n", keydata->value.lval);
					break;
				case IS_ARRAY: {
					int i, len, keylen, count;
					char *strarr, *ptr, *ptr1;
					count = _php3_hash_num_elements(keydata->value.ht);
					if(count > 0) {
						strarr = make_objrec_from_array(keydata->value.ht);
						len = strlen(strarr) - 1;
						keylen = strlen(key);
						if(NULL == (ptr = malloc(len + 1 + count*(keylen+1)))) {
							free(objrec);
							return(NULL);
						}
						ptr1 = ptr;
						*ptr1 = '\0';
						strcpy(ptr1, key);
						ptr1 += keylen;
						*ptr1++ = '=';
						for(i=0; i<len; i++) {
							*ptr1++ = strarr[i];
							if(strarr[i] == '\n') {
								strcpy(ptr1, key);
								ptr1 += keylen;
								*ptr1++ = '=';
							} else if(strarr[i] == '=')
								ptr1[-1] = ':';
						}
						*ptr1++ = '\n';
						*ptr1 = '\0';
						strncpy(str, ptr, 1023);
					}
					break;
				}
			}
			if(HASH_KEY_IS_STRING == keytype) efree(key);
			objrec = realloc(objrec, strlen(objrec)+strlen(str)+1);
			strcat(objrec, str);
//		}
		_php3_hash_move_forward(lht);
	}
	return objrec;
}
#undef BUFFERLEN

static int * make_ints_from_array(HashTable *lht) {
	int i, count;
	int *objrec = NULL;
	pval *keydata;

	if(NULL == lht)
		return NULL;

	if(0 == (count = _php3_hash_num_elements(lht)))
		return NULL;
	
	_php3_hash_internal_pointer_reset(lht);
	if(NULL == (objrec = emalloc(count*sizeof(int))))
		return NULL;
	for(i=0; i<count; i++) {
		_php3_hash_get_current_data(lht, (void **) &keydata);
		switch(keydata->type) {
			case IS_LONG:
				objrec[i] = keydata->value.lval;
				break;
			default:
				objrec[i] = 0;
		}
		_php3_hash_move_forward(lht);
	}
	return objrec;
}

int php3_minit_hw(INIT_FUNC_ARGS) {

	if (cfg_get_long("hw.allow_persistent",&php3_hw_module.allow_persistent)==FAILURE) {
		php3_hw_module.allow_persistent=1;
	}
	if (cfg_get_long("hw.max_persistent",&php3_hw_module.max_persistent)==FAILURE) {
		php3_hw_module.max_persistent=-1;
	}
	if (cfg_get_long("hw.max_links",&php3_hw_module.max_links)==FAILURE) {
		php3_hw_module.max_links=-1;
	}
	php3_hw_module.num_persistent=0;
	php3_hw_module.le_socketp = register_list_destructors(_close_hw_link,NULL);
	php3_hw_module.le_psocketp = register_list_destructors(NULL,_close_hw_plink);
	php3_hw_module.le_document = register_list_destructors(_free_hw_document,NULL);

	return SUCCESS;
}

#define BUFFERLEN 30
static void php3_hw_do_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	pval *argv[4];
	int argc;
	int sockfd;
	int port = 0;
	char *host = NULL;
	char *userdata = NULL;
	char *server_string = NULL;
	char *username = NULL;
	char *password = NULL;
	char *hashed_details;
	char *str = NULL;
	char buffer[BUFFERLEN];
	int hashed_details_length;
	hw_connection *ptr;
	int do_swap;
	int version = 0;
	HW_TLS_VARS;
	
	argc = ARG_COUNT(ht);
	switch(argc) {
		case 2:
		case 4:
			if (getParametersArray(ht, argc, argv) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	/* Host: */
	convert_to_string(argv[0]);
	host = (char *) estrndup(argv[0]->value.str.val,argv[0]->value.str.len);

	/* Port: */
	convert_to_long(argv[1]);
	port = argv[1]->value.lval;

	/* Username and Password */
	if(argc > 2) {
		/* Username */
		convert_to_string(argv[2]);
		username = (char *) estrndup(argv[2]->value.str.val,argv[2]->value.str.len);
		/* Password */
		convert_to_string(argv[3]);
		password = (char *) estrndup(argv[3]->value.str.val,argv[3]->value.str.len);
	}

	/* Create identifier string for connection */
	snprintf(buffer, BUFFERLEN, "%d", port);
	hashed_details_length = strlen(host)+strlen(buffer)+8;
	if(NULL == (hashed_details = (char *) emalloc(hashed_details_length+1))) {
		if(host) efree(host);
		if(password) efree(password);
		if(username) efree(username);
		php3_error(E_ERROR, "Could not get memory for connection details");
		RETURN_FALSE;
	}
	sprintf(hashed_details, "hw_%s_%d", host, port);

	if (persistent) {
		list_entry *le;

		/* try to find if we already have this link in our persistent list */
		if (_php3_hash_find(plist, hashed_details, hashed_details_length+1, (void **) &le)==FAILURE) {
			list_entry new_le;

			if (php3_hw_module.max_links!=-1 && php3_hw_module.num_links>=php3_hw_module.max_links) {
				php3_error(E_ERROR,"Hyperwave:  Too many open links (%d)",php3_hw_module.num_links);
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				efree(hashed_details);
				RETURN_FALSE;
			}
			if (php3_hw_module.max_persistent!=-1 && php3_hw_module.num_persistent>=php3_hw_module.max_persistent) {
				php3_error(E_ERROR,"Hyperwave: Too many open persistent links (%d)",php3_hw_module.num_persistent);
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				efree(hashed_details);
				RETURN_FALSE;
			}

			if ( (sockfd = open_hg_connection(host, port)) < 0 )  {
				php3_error(E_ERROR, "Could not open connection to %s, Port: %d (retval=%d)", host, port, sockfd);
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				efree(hashed_details);
				RETURN_FALSE;
				}
	
			if(NULL == (ptr = malloc(sizeof(hw_connection)))) {
				php3_error(E_ERROR, "Could not get memory for connection structure");
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				efree(hashed_details);
				RETURN_FALSE;
			}
	
			if(0 != (ptr->lasterror = initialize_hg_connection(sockfd, &do_swap, &version, &userdata, &server_string, username, password))) {
				php3_error(E_ERROR, "Could not initalize hyperwave connection");
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				if(userdata) efree(userdata);
				if(server_string) free(server_string);
				efree(hashed_details);
				RETURN_FALSE;
				}

			if(username) efree(username);
			if(password) efree(password);
	
			ptr->version = version;
			ptr->server_string = server_string;
			ptr->socket = sockfd;
			ptr->swap_on = do_swap;
			ptr->linkroot = 0;
			ptr->hostname = strdup(host);
			ptr->username = strdup("anonymous");
	
			new_le.ptr = (void *) ptr;
			new_le.type = php3_hw_module.le_psocketp;;

			if (_php3_hash_update(plist,hashed_details,hashed_details_length+1,(void *) &new_le, sizeof(list_entry), NULL)==FAILURE) {
				php3_error(E_ERROR, "Could not hash table with connection details");
				if(host) efree(host);
				if(username) efree(username);
				if(password) efree(password);
				if(server_string) free(server_string);
				efree(hashed_details);
				RETURN_FALSE;
			}

			php3_hw_module.num_links++;
			php3_hw_module.num_persistent++;
		} else {
			/*php3_printf("Found already open connection\n"); */
			if (le->type != php3_hw_module.le_psocketp) {
				RETURN_FALSE;
			}
			ptr = le->ptr;
		}

		return_value->value.lval = php3_list_insert(ptr,php3_hw_module.le_psocketp);
		return_value->type = IS_LONG;
	
	} else {
		list_entry *index_ptr,new_index_ptr;

		/* first we check the hash for the hashed_details key.  if it exists,
		 * it should point us to the right offset where the actual hyperwave link sits.
		 * if it doesn't, open a new hyperwave link, add it to the resource list,
		 * and add a pointer to it with hashed_details as the key.
		 */
		if (_php3_hash_find(list,hashed_details,hashed_details_length+1,(void **) &index_ptr)==SUCCESS) {
			int type,link;
			void *ptr;
	
			if (index_ptr->type != le_index_ptr) {
				RETURN_FALSE;
			}
			link = (int) index_ptr->ptr;
			ptr = (hw_connection *) php3_list_find(link,&type);   /* check if the link is still there */
			if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
				return_value->value.lval = php3_hw_module.default_link = link;
				return_value->type = IS_LONG;
				efree(hashed_details);
				if(username) efree(username);
				if(password) efree(password);
	  		if(host) efree(host);
				return;
			} else {
				_php3_hash_del(list,hashed_details,hashed_details_length+1);
			}
		}
	
		if ( (sockfd = open_hg_connection(host, port)) < 0 )  {
			php3_error(E_ERROR, "Could not open connection to %s, Port: %d (retval=%d", host, port, sockfd);
		  if(host) efree(host);
			if(username) efree(username);
			if(password) efree(password);
			efree(hashed_details);
			RETURN_FALSE;
			}
	
		if(NULL == (ptr = malloc(sizeof(hw_connection)))) {
			if(host) efree(host);
			if(username) efree(username);
			if(password) efree(password);
			efree(hashed_details);
			RETURN_FALSE;
		}
	
		if(0 != (ptr->lasterror = initialize_hg_connection(sockfd, &do_swap, &version, &userdata, &server_string, username, password))) {
			php3_error(E_ERROR, "Could not initalize hyperwave connection");
			if(host) efree(host);
			if(username) efree(username);
			if(password) efree(password);
			if(userdata) efree(userdata);
			if(server_string) free(server_string);
			efree(hashed_details);
			RETURN_FALSE;
			}

		if(username) efree(username);
		if(password) efree(password);
	
		ptr->version = version;
		ptr->server_string = server_string;
		ptr->socket = sockfd;
		ptr->swap_on = do_swap;
		ptr->linkroot = 0;
		ptr->hostname = strdup(host);
		ptr->username = strdup("anonymous");
	
		return_value->value.lval = php3_list_insert(ptr,php3_hw_module.le_socketp);
		return_value->type = IS_LONG;
	
		new_index_ptr.ptr = (void *) return_value->value.lval;
		new_index_ptr.type = le_index_ptr;
		if (_php3_hash_update(list,hashed_details,hashed_details_length+1,(void *) &new_index_ptr, sizeof(list_entry), NULL)==FAILURE) {
			php3_error(E_ERROR, "Could not update connection details in hash table");
			if(host) efree(host);
			efree(hashed_details);
			RETURN_FALSE;
		}
	
	}

	efree(hashed_details);
	if(host) efree(host);
	php3_hw_module.default_link=return_value->value.lval;

	/* At this point we have a working connection. If userdata was given
	   we are also indentified.
	   If there is no userdata because hw_connect was called without username
	   and password, we don't evaluate userdata.
	*/
	if(NULL == userdata)
		return;

	if(ptr->username) free(ptr->username);
	str = userdata;
	while((*str != 0) && (*str != ' '))
		str++;
	if(*str != '\0')
		ptr->username = strdup(++str);
	else
		ptr->username = NULL;
	efree(userdata);
}
#undef BUFFERLEN

/* Start of user level functions */
/* ***************************** */
/* {{{ proto int hw_connect(string host, int port [, string username [, string password]])
   Connect to the Hyperwave server */
PHP_FUNCTION(hw_connect)
{
	php3_hw_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU,0);
}
/* }}} */

/* {{{ proto int hw_pconnect(string host, int port [, string username [, string password]])
   Connect to the Hyperwave server persistent */
PHP_FUNCTION(hw_pconnect)
{
	php3_hw_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
}
/* }}} */

/* {{{ proto void hw_close(int link)
   Close connection to Hyperwave server */
PHP_FUNCTION(hw_close) {
	pval *arg1;
	int id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	php3_list_delete(id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void hw_info(int link)
   Outputs info string */
PHP_FUNCTION(hw_info)
{
	pval *arg1;
	int id, type;
	hw_connection *ptr;
	char *str;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	if(NULL != (str = get_hw_info(ptr))) {
		/*
		php3_printf("%s\n", str);
		efree(str);
		*/
		return_value->value.str.len = strlen(str);
		return_value->value.str.val = str;
		return_value->type = IS_STRING;
		return;
		}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int hw_error(int link)
   Returns last error number */
PHP_FUNCTION(hw_error)
{
	pval *arg1;
	int id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	RETURN_LONG(ptr->lasterror);
}
/* }}} */

/* {{{ proto string hw_errormsg(int link)
   Returns last error message */
PHP_FUNCTION(hw_errormsg)
{
	pval *arg1;
	int id, type;
	hw_connection *ptr;
	char errstr[100];
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	switch(ptr->lasterror) {
		case 0:
			sprintf(errstr, "No error");
			break;
		case NOACCESS:
			sprintf(errstr, "No access");
			break;
		case NODOCS:
			sprintf(errstr, "No documents");
			break;
		case NONAME:
			sprintf(errstr, "No collection name");
			break;
		case NODOC:
			sprintf(errstr, "Object is not a document");
			break;
		case NOOBJ:
			sprintf(errstr, "No object received");
			break;
		case NOCOLLS:
			sprintf(errstr, "No collections received");
			break;
		case DBSTUBNG:
			sprintf(errstr, "Connection to low-level database failed");
			break;
		case NOTFOUND:
			sprintf(errstr, "Object not found");
			break;
		case EXIST:
			sprintf(errstr, "Collection already exists");
			break;
		case FATHERDEL:
			sprintf(errstr, "parent collection disappeared");
			break;
		case FATHNOCOLL:
			sprintf(errstr, "parent collection not a collection");
			break;
		case NOTEMPTY:
			sprintf(errstr, "Collection not empty");
			break;
		case DESTNOCOLL:
			sprintf(errstr, "Destination not a collection");
			break;
		case SRCEQDEST:
			sprintf(errstr, "Source equals destination");
			break;
		case REQPEND:
			sprintf(errstr, "Request pending");
			break;
		case TIMEOUT:
			sprintf(errstr, "Timeout");
			break;
		case NAMENOTUNIQUE:
			sprintf(errstr, "Name not unique");
			break;
		case WRITESTOPPED:
			sprintf(errstr, "Database now read-only; try again later");
			break;
		case LOCKED:
			sprintf(errstr, "Object locked; try again later");
			break;
		case NOTREMOVED:
			sprintf(errstr, "Attribute not removed");
			break;
		case CHANGEBASEFLD:
			sprintf(errstr, "Change of base-attribute");
			break;
		case FLDEXISTS:
			sprintf(errstr, "Attribute exists");
			break;
		case NOLANGUAGE:
			sprintf(errstr, "No or unknown language specified");
			break;
		default:
			sprintf(errstr, "Unknown error: %d", ptr->lasterror);
		}
	RETURN_STRING(errstr, 1);
}
/* }}} */

/* {{{ proto int hw_root(void)
   Returns object id of root collection */
PHP_FUNCTION(hw_root)
{
	HW_TLS_VARS;
	return_value->value.lval = 0;
	return_value->type = IS_LONG;
}
/* }}} */

char *php3_hw_command(INTERNAL_FUNCTION_PARAMETERS, int comm) {
	pval *arg1;
	int link, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		return NULL;
	}
	convert_to_long(arg1);
	link=arg1->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		return NULL;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	if (0 != (ptr->lasterror = send_command(ptr->socket, comm, &object)))
		return NULL;

	return object;
	}
}

/* {{{ proto string hw_stat(int link)
   Returns status string */
PHP_FUNCTION(hw_stat) {
        char *object;

	object = php3_hw_command(INTERNAL_FUNCTION_PARAM_PASSTHRU, STAT_COMMAND);
	if(object == NULL)
		RETURN_FALSE;

	return_value->value.str.val = object;
	return_value->value.str.len = strlen(object);
	return_value->type = IS_STRING;
}
/* }}} */

/* {{{ proto array hw_who(int link)
   Returns names and info of users loged in */
PHP_FUNCTION(hw_who) {
	pval user_arr;
        char *object, *ptr, *temp, *attrname;
	int i;

	object = php3_hw_command(INTERNAL_FUNCTION_PARAM_PASSTHRU, WHO_COMMAND);
	if(object == NULL)
		RETURN_FALSE;

	ptr = object;

php3_printf("%s\n", ptr);
	/* Skip first two lines, they just contain:
        Users in Database

        */
        while((*ptr != '\0') && (*ptr != '\n'))
		ptr++;
        while((*ptr != '\0') && (*ptr != '\n'))
		ptr++;
	if(*ptr == '\0') {
		efree(object);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(object);
		RETURN_FALSE;
	}

	temp = estrdup(ptr);
	attrname = strtok(temp, "\n");
	i = 0;
	while(attrname != NULL) {
		char *name;

		if (array_init(&user_arr) == FAILURE) {
			efree(object);
			RETURN_FALSE;
		}

		ptr = attrname;
		if(*ptr++ == '*')
			add_assoc_long(&user_arr, "self", 1);
		else
			add_assoc_long(&user_arr, "self", 0);
			
		ptr++;
		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "id", name, 1);

		ptr++;
		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "name", name, 1);

		ptr++;
		while((*ptr != '\0') && (*ptr == ' '))
			ptr++;

		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "system", name, 1);

		ptr++;
		while((*ptr != '\0') && (*ptr == ' '))
			ptr++;

		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "onSinceDate", name, 1);

		ptr++;
		while((*ptr != '\0') && (*ptr == ' '))
			ptr++;

		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "onSinceTime", name, 1);

		ptr++;
		while((*ptr != '\0') && (*ptr == ' '))
			ptr++;

		name = ptr;
		while((*ptr != '\0') && (*ptr != ' '))
			ptr++;
		*ptr = '\0';
		add_assoc_string(&user_arr, "TotalTime", name, 1);

		/* Add the user array */
		_php3_hash_index_update(return_value->value.ht, i++, &user_arr, sizeof(pval), NULL);

		/* The user array can now be freed, but I don't know how */

		attrname = strtok(NULL, "\n");
	}
	efree(temp);
	efree(object);

}
/* }}} */

/* {{{ proto string hw_dummy(int link, int id, int msgid)
   Hyperwave dummy function */
PHP_FUNCTION(hw_dummy) {
	pval *arg1, *arg2, *arg3;
	int link, id, type, msgid;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	id=arg2->value.lval;
	msgid=arg3->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	if (0 != (ptr->lasterror = send_dummy(ptr->socket, id, msgid, &object)))
		RETURN_FALSE;

php3_printf("%s", object);
	return_value->value.str.val = object;
	return_value->value.str.len = strlen(object);
	return_value->type = IS_STRING;
	}
}
/* }}} */

/* {{{ proto string hw_getobject(int link, int objid)
   Returns object record  */
PHP_FUNCTION(hw_getobject) {
	pval *argv[3];
	int argc, link, id, type, multi;
	hw_connection *ptr;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if(argc < 2 || argc > 3)
		WRONG_PARAM_COUNT;
	if (getParametersArray(ht, argc, argv) == FAILURE)
		WRONG_PARAM_COUNT;

	convert_to_long(argv[0]);
	if(argv[1]->type == IS_ARRAY) {
		multi = 1;
		convert_to_array(argv[1]);
	} else {
		multi = 0;
		convert_to_long(argv[1]);
	}

	if(argc == 3) {
		convert_to_string(argv[2]);
	}

	link=argv[0]->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d", link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if(multi) {
		char **objects = NULL;
		int count, *ids, i;
		HashTable *lht;
		pval *keydata;

		lht = argv[1]->value.ht;
        	if(0 == (count = _php3_hash_num_elements(lht)))
			RETURN_FALSE;
		ids = emalloc(count * sizeof(hw_objectID));

		_php3_hash_internal_pointer_reset(lht);
		for(i=0; i<count; i++) {
			_php3_hash_get_current_data(lht, (void **) &keydata);
			switch(keydata->type) {
				case IS_LONG:
					ids[i] = keydata->value.lval;
					break;
				default:
					ids[i] = keydata->value.lval;
			}
			_php3_hash_move_forward(lht);
		}

		if (0 != (ptr->lasterror = send_objectbyidquery(ptr->socket, ids, &count, argv[2]->value.str.val, &objects))) {
			efree(ids);
			RETURN_FALSE;
			}
		efree(ids);
		if (array_init(return_value) == FAILURE) {
			efree(objects);
			RETURN_FALSE;
		}

		for(i=0; i<count; i++) {
			add_index_string(return_value, i, objects[i], 0);
		}
		efree(objects);
		
	} else {
		char *object = NULL;
		id=argv[1]->value.lval;
		if (0 != (ptr->lasterror = send_getobject(ptr->socket, id, &object)))
			RETURN_FALSE;

		RETURN_STRING(object, 0);
	}
}
/* }}} */

/* {{{ proto int hw_insertobject(int link, string objrec, string parms)
   Inserts an object */
PHP_FUNCTION(hw_insertobject) {
	pval *arg1, *arg2, *arg3;
	int link, type;
	char *objrec, *parms;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_string(arg3);
	link=arg1->value.lval;
	objrec=arg2->value.str.val;
	parms=arg3->value.str.val;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	int objid;
	if (0 != (ptr->lasterror = send_insertobject(ptr->socket, objrec, parms, &objid)))
		RETURN_FALSE;

	RETURN_LONG(objid);
	}
}
/* }}} */

/* {{{ proto string hw_getandlock(int link, int objid)
   Returns object record and locks object */
PHP_FUNCTION(hw_getandlock) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	if (0 != (ptr->lasterror = send_getandlock(ptr->socket, id, &object)))
		RETURN_FALSE;

	RETURN_STRING(object, 0);
	}
}
/* }}} */

/* {{{ proto void hw_unlock(int link, int objid)
   Unlocks object */
PHP_FUNCTION(hw_unlock) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_unlock(ptr->socket, id)))
		RETURN_FALSE;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void hw_deleteobject(int link, int objid)
   Deletes object */
PHP_FUNCTION(hw_deleteobject) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_deleteobject(ptr->socket, id)))
		RETURN_FALSE;
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void hw_changeobject(int link, int objid, array attributes)
   Changes attributes of an object (obsolete) */
#define BUFFERLEN 200
PHP_FUNCTION(hw_changeobject) {
	pval *arg1, *arg2, *arg3;
	int link, id, type, i;
	hw_connection *ptr;
	char *modification, *oldobjrec, buf[BUFFERLEN];
	char *tmp;
	HashTable *newobjarr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1); /* Connection */
	convert_to_long(arg2); /* object ID */
	convert_to_array(arg3); /* Array with new attributes */
	link=arg1->value.lval;
	id=arg2->value.lval;
	newobjarr=arg3->value.ht;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	/* get the old object record */
	if(0 != (ptr->lasterror = send_getandlock(ptr->socket, id, &oldobjrec)))
		RETURN_FALSE;

	_php3_hash_internal_pointer_reset(newobjarr);
	modification = strdup("");
	for(i=0; i<_php3_hash_num_elements(newobjarr); i++) {
		char *key, *str, *str1, newattribute[BUFFERLEN];
		pval *data;
		int j, noinsert=1;
		ulong ind;

		_php3_hash_get_current_key(newobjarr, &key, &ind);
		_php3_hash_get_current_data(newobjarr, (void *) &data);
		switch(data->type) {
			case IS_STRING:
				if(strlen(data->value.str.val) == 0)
					snprintf(newattribute, BUFFERLEN, "rem %s", key);
				else
					snprintf(newattribute, BUFFERLEN, "add %s=%s", key, data->value.str.val);
				noinsert = 0;
				break;
			default:
				newattribute[0] = '\0';
		}
		if(!noinsert) {
			modification = fnInsStr(modification, 0, "\\");
			modification = fnInsStr(modification, 0, newattribute);
/*			modification = fnInsStr(modification, 0, "add "); */

			/* Retrieve the old attribute from object record */
			if(NULL != (str = strstr(oldobjrec, key))) {
				str1 = str;
				j = 0;
				while((str1 != NULL) && (*str1 != '\n') && (j < BUFFERLEN-1)) {
					buf[j++] = *str1++;
				}
				buf[j] = '\0';
				modification = fnInsStr(modification, 0, "\\");
				modification = fnInsStr(modification, 0, buf);
				modification = fnInsStr(modification, 0, "rem ");
			} 
		}
		efree(key);
		_php3_hash_move_forward(newobjarr);
	}
	efree(oldobjrec);

	set_swap(ptr->swap_on);
	modification[strlen(modification)-1] = '\0';
	if (0 != (ptr->lasterror = send_changeobject(ptr->socket, id, modification))) {
		free(modification);
		send_unlock(ptr->socket, id);
		RETURN_FALSE;
	}
	free(modification);
	if (0 != (ptr->lasterror = send_unlock(ptr->socket, id))) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
#undef BUFFERLEN
/* }}} */

/* {{{ proto void hw_modifyobject(int link, int objid, array remattributes, array addattributes [, int mode])
   Modifies attributes of an object */
#define BUFFERLEN 200
PHP_FUNCTION(hw_modifyobject) {
	pval *argv[5];
	int argc;
	int link, id, type, i, mode;
	hw_connection *ptr;
	char *modification;
	HashTable *remobjarr, *addobjarr;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if((argc > 5) || (argc < 4))
		WRONG_PARAM_COUNT;

	if (getParametersArray(ht, argc, argv) == FAILURE)
	if(argc < 4) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(argv[0]); /* Connection */
	convert_to_long(argv[1]); /* object ID */
	convert_to_array(argv[2]); /* Array with attributes to remove */
	convert_to_array(argv[3]); /* Array with attributes to add */
	if(argc == 5) {
		convert_to_long(argv[4]);
		mode = argv[4]->value.lval;
	} else
		mode = 0;
	link=argv[0]->value.lval;
	id=argv[1]->value.lval;
	remobjarr=argv[2]->value.ht;
	addobjarr=argv[3]->value.ht;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	modification = strdup("");
	if(addobjarr != NULL) {
		_php3_hash_internal_pointer_reset(addobjarr);
		for(i=0; i<_php3_hash_num_elements(addobjarr); i++) {
			char *key, addattribute[BUFFERLEN];
			pval *data;
			int noinsert=1;
			ulong ind;

			_php3_hash_get_current_key(addobjarr, &key, &ind);
			_php3_hash_get_current_data(addobjarr, (void *) &data);
			switch(data->type) {
				case IS_STRING:
					if(strlen(data->value.str.val) > 0) {
						snprintf(addattribute, BUFFERLEN, "add %s=%s", key, data->value.str.val);
/* fprintf(stderr, "add: %s\n", addattribute); */
						noinsert = 0;
					}
					break;
				case IS_ARRAY: {
					int i, len, keylen, count;
					char *strarr, *ptr, *ptr1;
					count = _php3_hash_num_elements(data->value.ht);
					if(count > 0) {
						strarr = make_objrec_from_array(data->value.ht);
						len = strlen(strarr) - 1;
						keylen = strlen(key);
						if(NULL == (ptr = malloc(len + 1 + count*(keylen+1+4)))) {
							if(modification)
								free(modification);
							RETURN_FALSE;
						}
						ptr1 = ptr;
						*ptr1 = '\0';
						strcpy(ptr1, "add ");
						ptr1 += 4;
						strcpy(ptr1, key);
						ptr1 += keylen;
						*ptr1++ = '=';
						for(i=0; i<len; i++) {
							*ptr1++ = strarr[i];
							if(strarr[i] == '\n') {
								ptr1[-1] = '\\';
								strcpy(ptr1, "add ");
								ptr1 += 4;
								strcpy(ptr1, key);
								ptr1 += keylen;
								*ptr1++ = '=';
							} else if(strarr[i] == '=')
								ptr1[-1] = ':';
						}
						*ptr1 = '\0';
						strncpy(addattribute, ptr, BUFFERLEN);
						noinsert = 0;
					}
					break;
				}
			}
			if(!noinsert) {
				modification = fnInsStr(modification, 0, "\\");
				modification = fnInsStr(modification, 0, addattribute);
			}
			efree(key);
			_php3_hash_move_forward(addobjarr);
		}
	}

	if(remobjarr != NULL) {
		int nr;
		_php3_hash_internal_pointer_reset(remobjarr);
		nr = _php3_hash_num_elements(remobjarr);
		for(i=0; i<nr; i++) {
			char *key, remattribute[BUFFERLEN];
			pval *data;
			int noinsert=1;
			ulong ind;

			_php3_hash_get_current_key(remobjarr, &key, &ind);
			_php3_hash_get_current_data(remobjarr, (void *) &data);
			switch(data->type) {
				case IS_STRING:
					if(strlen(data->value.str.val) > 0) {
						snprintf(remattribute, BUFFERLEN, "rem %s=%s", key, data->value.str.val);
						noinsert = 0;
					} else {
						snprintf(remattribute, BUFFERLEN, "rem %s", key);
						noinsert = 0;
 					}
					break;
				case IS_ARRAY: {
					int i, len, keylen, count;
					char *strarr, *ptr, *ptr1;
					count = _php3_hash_num_elements(data->value.ht);
					if(count > 0) {
						strarr = make_objrec_from_array(data->value.ht);
						len = strlen(strarr) - 1;
						keylen = strlen(key);
						if(NULL == (ptr = malloc(len + 1 + count*(keylen+1+4)))) {
							if(modification)
								free(modification);
							RETURN_FALSE;
						}
						ptr1 = ptr;
						*ptr1 = '\0';
						strcpy(ptr1, "rem ");
						ptr1 += 4;
						strcpy(ptr1, key);
						ptr1 += keylen;
						*ptr1++ = '=';
						for(i=0; i<len; i++) {
							*ptr1++ = strarr[i];
							if(strarr[i] == '\n') {
								ptr1[-1] = '\\';
								strcpy(ptr1, "rem ");
								ptr1 += 4;
								strcpy(ptr1, key);
								ptr1 += keylen;
								*ptr1++ = '=';
							} else if(strarr[i] == '=')
								ptr1[-1] = ':';
						}
						*ptr1++ = '\n';
						*ptr1 = '\0';
						strncpy(remattribute, ptr, BUFFERLEN);
						noinsert = 0;
					}
					break;
				}
			}
			if(!noinsert) {
				modification = fnInsStr(modification, 0, "\\");
				modification = fnInsStr(modification, 0, remattribute);
			}
			efree(key);
			_php3_hash_move_forward(remobjarr);
		}
	}

	set_swap(ptr->swap_on);
	modification[strlen(modification)-1] = '\0';
	if(strlen(modification) == 0) {
		ptr->lasterror = 0;
		free(modification);
		RETURN_TRUE;
	}
/*	fprintf(stderr, "modifyobject: %s\n", modification);*/
	switch(mode) {
		case 0:
			if (0 == (ptr->lasterror = send_lock(ptr->socket, id))) {
				if (0 == (ptr->lasterror = send_changeobject(ptr->socket, id, modification))) {
					if (0 != (ptr->lasterror = send_unlock(ptr->socket, id))) {
						php3_error(E_WARNING,"Aiii, Changeobject failed and couldn't unlock object (id = 0x%X)", id);
						free(modification);
						RETURN_FALSE;
					}
					free(modification);
					RETURN_FALSE;
				} else {
					send_unlock(ptr->socket, id);
					free(modification);
					RETURN_FALSE;
				}
			} else {
				php3_error(E_WARNING,"Could not lock object (id = 0x%X)", id);
				free(modification);
				RETURN_FALSE;
			}
			break;
		case 1:
/* WARNING: send_groupchangobject() only works right, if each attribute
   can be modified. Doing a changeobject recursively often tries to
   modify objects which cannot be modified e.g. because an attribute cannot
   be removed. In such a case no further modification on that object is done.
   Doing a 'rem Rights\add Rights=R:a' will fail completely if the attribute
   Rights is not there already. The object locking is done in send_groupchangeobject();
*/
			if (0 != (ptr->lasterror = send_groupchangeobject(ptr->socket, id, modification))) {
				free(modification);
				RETURN_FALSE;
			}
			break;
		default:
			php3_error(E_WARNING,"hw_modifyobject: Mode must be 0 or 1 (recursive)");
	}
	free(modification);
	RETURN_TRUE;
}
#undef BUFFERLEN
/* }}} */

void php3_hw_mvcp(INTERNAL_FUNCTION_PARAMETERS, int mvcp) {
	pval *arg1, *arg2, *arg3, *arg4, **objvIDs;
	int link, type, dest=0, from=0;
	HashTable *src_arr;
	hw_connection *ptr;
	int collIDcount, docIDcount, i, *docIDs, *collIDs;
	HW_TLS_VARS;

fprintf(stderr, "Copy/Move %d\n", mvcp);
	switch(mvcp) {
		case MOVE: /* Move also has fromID */
			if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE)
				WRONG_PARAM_COUNT;
			break;
		case COPY:
			if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE)
				WRONG_PARAM_COUNT;
			break;
	}
	convert_to_long(arg1);
	convert_to_array(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	src_arr=arg2->value.ht;
	switch(mvcp) {
		case MOVE: /* Move also has fromID, which is arg3 --> arg4 becomes destID */
			convert_to_long(arg4);
			from=arg3->value.lval;
			dest=arg4->value.lval;
			break;
		case COPY: /* No fromID for Copy needed --> arg3 is destID */
			dest=arg3->value.lval;
			from = 0;
			break;
	}
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);

	if(NULL == (objvIDs = emalloc(_php3_hash_num_elements(src_arr) * sizeof(pval *)))) {
		RETURN_FALSE;
		}

	if(getParametersArray(src_arr, _php3_hash_num_elements(src_arr), objvIDs) == FAILURE) {
		efree(objvIDs);
		RETURN_FALSE;
		}

	if(NULL == (collIDs = emalloc(_php3_hash_num_elements(src_arr) * sizeof(int)))) {
		efree(objvIDs);
		RETURN_FALSE;
		}

	if(NULL == (docIDs = emalloc(_php3_hash_num_elements(src_arr) * sizeof(int)))) {
		efree(objvIDs);
		efree(collIDs);
		RETURN_FALSE;
		}

	collIDcount = docIDcount = 0;
	for(i=0; i<_php3_hash_num_elements(src_arr); i++) {
		char *objrec;
		if(objvIDs[i]->type == IS_LONG) {
			if(0 != (ptr->lasterror = send_getobject(ptr->socket, objvIDs[i]->value.lval, &objrec))) {
				efree(objvIDs);
				efree(collIDs);
				efree(docIDs);
				RETURN_FALSE;
			}
			if(0 == fnAttributeCompare(objrec, "DocumentType", "collection"))
				collIDs[collIDcount++] = objvIDs[i]->value.lval;
			else
				docIDs[docIDcount++] = objvIDs[i]->value.lval;
			efree(objrec);
		}
	}
	efree(objvIDs);
	if (0 != (ptr->lasterror = send_mvcpdocscoll(ptr->socket, docIDs, docIDcount, from, dest, mvcp))) {
		efree(collIDs);
		efree(docIDs);
		RETURN_FALSE;
	}

	if (0 != (ptr->lasterror = send_mvcpcollscoll(ptr->socket, collIDs, collIDcount, from, dest, mvcp))) {
		efree(collIDs);
		efree(docIDs);
		RETURN_FALSE;
	}

	efree(collIDs);
	efree(docIDs);

	RETURN_LONG(docIDcount + collIDcount);
}

/* {{{ proto void hw_mv(int link, array objrec, int from, int dest)
   Moves object */
PHP_FUNCTION(hw_mv) {
	php3_hw_mvcp(INTERNAL_FUNCTION_PARAM_PASSTHRU, MOVE);
}
/* }}} */

/* {{{ proto void hw_cp(int link, array objrec, int dest)
   Copies object */
PHP_FUNCTION(hw_cp) {
	php3_hw_mvcp(INTERNAL_FUNCTION_PARAM_PASSTHRU, COPY);
}
/* }}} */

/* {{{ proto hwdoc hw_gettext(int link, int objid [, int rootid])
   Returns text document. Links are relative to rootid if given. */
PHP_FUNCTION(hw_gettext) {
	pval *argv[3];
	int argc, link, id, type, mode;
	int rootid = 0;
	char *urlprefix;
	hw_document *doc;
	hw_connection *ptr;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if((argc > 3) || (argc < 2))
		WRONG_PARAM_COUNT;
		
	if (getParametersArray(ht, argc, argv) == FAILURE)
		RETURN_FALSE;

	convert_to_long(argv[0]);
	convert_to_long(argv[1]);
	mode = 0;
	urlprefix = NULL;
	if(argc == 3) {
		switch(argv[2]->type) {
			case IS_LONG:
				convert_to_long(argv[2]);
				rootid = argv[2]->value.lval;
				mode = 1;
				break;
			case IS_STRING:	
				convert_to_string(argv[2]);
				urlprefix = argv[2]->value.str.val;
				break;
		}
	}
	link=argv[0]->value.lval;
	id=argv[1]->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	char *attributes = NULL;
	char *bodytag = NULL;
	int count;
	/* !!!! memory for object and attributes is allocated with malloc !!!! */
	if (0 != (ptr->lasterror = send_gettext(ptr->socket, id, mode, rootid, &attributes, &bodytag, &object, &count, urlprefix)))
		RETURN_FALSE;
	doc = malloc(sizeof(hw_document));
	doc->data = object;
	doc->attributes = attributes;
	doc->bodytag = bodytag;
	doc->size = count;
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
	}
}
/* }}} */

/* {{{ proto void hw_edittext(int link, hwdoc doc)
   Modifies text document */
PHP_FUNCTION(hw_edittext) {
	pval *arg1, *arg2;
	int link, doc, type;
	hw_connection *ptr;
	hw_document *docptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	ptr = php3_list_find(link,&type);

	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find socket identifier %d",link);
		RETURN_FALSE;
	}

	doc=arg2->value.lval;
	docptr = php3_list_find(doc,&type);

	if(!docptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find document identifier %d", doc);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	if (0 != (ptr->lasterror =  send_edittext(ptr->socket, docptr->attributes, docptr->data))) {
		RETURN_FALSE;
		}
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto hwdoc hw_getcgi(int link, int objid)
   Returns the output of a cgi script */
#define BUFFERLEN 1000
/* FIX ME: The buffer cgi_env_str should be allocated dynamically */
PHP_FUNCTION(hw_getcgi) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_document *doc;
	hw_connection *ptr;
	char cgi_env_str[BUFFERLEN];
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	char *attributes = NULL;
	int count;

	/* Here is another undocument function of Hyperwave.
	   If you call a cgi script with getcgi-message, you will
	   have to provide the complete cgi enviroment, since it is
	   only known to the webserver (or wavemaster). This is done
	   by extending the object record with the following incomplete
	   string. It should contain any enviroment variable a cgi script
	   requires.
	*/
#if (WIN32|WINNT)
	snprintf(cgi_env_str, BUFFERLEN, "CGI_REQUEST_METHOD=%s\nCGI_PATH_INFO=%s\nCGI_QUERY_STRING=%s",
	                     getenv("REQUEST_METHOD"),
	                     getenv("PATH_INFO"),
	                     getenv("QUERY_STRING"));
#else
	snprintf(cgi_env_str, BUFFERLEN, "CGI_REQUEST_METHOD=%s\nCGI_PATH_INFO=%s\nCGI_QUERY_STRING=%s",
	                     GLOBAL(request_info).request_method,
	                     GLOBAL(request_info).path_info,
	                     GLOBAL(request_info).query_string);
#endif
	/* !!!! memory for object and attributes is allocated with malloc !!!! */
	if (0 != (ptr->lasterror = send_getcgi(ptr->socket, id, cgi_env_str, &attributes, &object, &count)))
		RETURN_FALSE;
	doc = malloc(sizeof(hw_document));
	doc->data = object;
	doc->attributes = attributes;
	doc->bodytag = NULL;
	doc->size = count;
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
	}
}
#undef BUFFERLEN
/* }}} */

/* {{{ proto int hw_getremote(int link, int objid)
   Returns the content of a remote document */
PHP_FUNCTION(hw_getremote) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_document *doc;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	char *attributes = NULL;
	int count;
	/* !!!! memory for object and attributes is allocated with malloc !!!! */
	if (0 != (ptr->lasterror = send_getremote(ptr->socket, id, &attributes, &object, &count)))
		RETURN_FALSE;
	doc = malloc(sizeof(hw_document));
	doc->data = object;
	doc->attributes = attributes;
	doc->bodytag = NULL;
	doc->size = count;
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
	}
}
/* }}} */

/* {{{ proto [array|int] hw_getremotechildren(int link, string objrec)
   Returns the remote document or an array of object records */
PHP_FUNCTION(hw_getremotechildren) {
	pval *arg1, *arg2;
	int link, type, i;
	hw_connection *ptr;
	char *objrec;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_string(arg2);
	link=arg1->value.lval;
	objrec=arg2->value.str.val;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d", link);
		RETURN_FALSE;
	}
	set_swap(ptr->swap_on);
	{
	int count, *offsets;
	char *remainder, *ptr1;
	if (0 != (ptr->lasterror = send_getremotechildren(ptr->socket, objrec, &remainder, &offsets, &count)))
		RETURN_FALSE;

/*
for(i=0;i<count;i++)
  php3_printf("offset[%d] = %d--\n", i, offsets[i]);
php3_printf("count = %d, remainder = <HR>%s---<HR>", count, remainder);
*/
	/* The remainder depends on the number of returned objects and
	   whether the MimeType of the object to retrieve is set. If
	   the MimeType is set the result will start with the
	   HTTP header 'Content-type: mimetype', otherwise it will be
	   a list of object records and therefore starts with
	   'ObjectID=0'. In the first case the offset and count are somewhat
	   strange. Quite often count had a value of 6 which appears to be
	   meaningless, but if you sum up the offsets you get the length
	   of the remainder which is the lenght of the document.
	   The document must have been chopped up into 6 pieces, each ending
	   with 'ServerId=0xYYYYYYYY'.
	   In the second case the offset contains the lenght of
	   each object record; count contains the number of object records.
	   Even if a remote object has children
	   (several sql statements) but the MimeType is set, it will
	   return a document in the format of MimeType. On the other
	   hand a remote object does not have any children but just
	   returns a docuement will not be shown unless the MimeType
	   is set. It returns the pure object record of the object without
	   the SQLStatement attribute. Quite senseless.
           Though, this behavior depends on how the hgi gateway in Hyperwave
	   is implemented.
	*/
	if(strncmp(remainder, "ObjectID=0 ", 10)) {
		hw_document *doc;
		char *ptr;
		int i, j, len;
		/* For some reason there is always the string
		   'SeverId=0xYYYYYYYY' at the end, so we cut it off.
		   The document may as well be divided into several pieces
		   and each of them has the ServerId at the end.
		   The following will put the pieces back together and
		   strip the ServerId. count contains the number of pieces.
		*/
		for(i=0, len=0; i<count; i++)
			len += offsets[i]-18;
/*fprintf(stderr,"len = %d\n", len); */
		doc = malloc(sizeof(hw_document));
		doc->data = malloc(len+1);
		ptr = doc->data;
		for(i=0, j=0; i<count; i++) {
			memcpy((char *)ptr, (char *)&remainder[j], offsets[i]-18);
/*fprintf(stderr,"rem = %s\n", &remainder[j]); */
			j += offsets[i];
			ptr += offsets[i] - 18;
		}
		*ptr = '\0';
		doc->attributes = strdup(objrec);
		doc->bodytag = NULL;
		doc->size = strlen(doc->data);
		return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
		return_value->type = IS_LONG;
	} else {
		if (array_init(return_value) == FAILURE) {
			efree(offsets);
			RETURN_FALSE;
		}

		ptr1 = remainder;
		for(i=0; i<count; i++) {
			*(ptr1+offsets[i]-1) = '\0';
			add_index_string(return_value, i, ptr1, 1);
			ptr1 += offsets[i];
		}
	}

	efree(offsets);
	efree(remainder);
	}
}
/* }}} */

/* {{{ proto void hw_setlinkroot(int link, int rootid)
   Set the id to which links are calculated */
PHP_FUNCTION(hw_setlinkroot) {
	pval *arg1, *arg2;
	int link, type, rootid;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link = arg1->value.lval;
	rootid = arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	ptr->linkroot = rootid;
	RETURN_LONG(rootid);
}
/* }}} */

/* {{{ proto hwdoc hw_pipedocument(int link, int objid)
   Returns document */
PHP_FUNCTION(hw_pipedocument) {
	pval *argv[3];
	int link, id, type, argc, mode;
	int rootid = 0;
	hw_connection *ptr;
	hw_document *doc;
#if APACHE
	server_rec *serv = GLOBAL(php3_rqst)->server;
#endif
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if((argc > 2) || (argc < 2))
		WRONG_PARAM_COUNT;
		
	if (getParametersArray(ht, argc, argv) == FAILURE)
		RETURN_FALSE;

	convert_to_long(argv[0]);
	convert_to_long(argv[1]);
/*	if(argc == 3) {
		convert_to_long(argv[2]);
		rootid = argv[2]->value.lval;
		if(rootid != 0)
			mode = 1;
	}
*/	link=argv[0]->value.lval;
	id=argv[1]->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d", link);
		RETURN_FALSE;
	}

	mode = 0;
	if(ptr->linkroot > 0)
		mode = 1;
	rootid = ptr->linkroot;

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	char *attributes = NULL;
	char *bodytag = NULL;
	int count;
	/* !!!! memory for object, bodytag and attributes is allocated with malloc !!!! */
	if (0 != (ptr->lasterror =  send_pipedocument(ptr->socket,
#if APACHE
  serv->server_hostname,
#else
  getenv("HOSTNAME"),
#endif
   id, mode, rootid, &attributes, &bodytag, &object, &count, NULL)))
		RETURN_FALSE;

	doc = malloc(sizeof(hw_document));
	doc->data = object;
	doc->attributes = attributes;
	doc->bodytag = bodytag;
	doc->size = count;
/* fprintf(stderr, "size = %d\n", count); */
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
	}
}
/* }}} */

/* {{{ proto hwdoc hw_pipecgi(int link, int objid)
   Returns output of a CGI script */
#define BUFFERLEN 1000
/* FIX ME: The buffer cgi_env_str should be allocated dynamically */
PHP_FUNCTION(hw_pipecgi) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	hw_document *doc;
	char cgi_env_str[1000];
#if APACHE
	server_rec *serv = GLOBAL(php3_rqst)->server;
#endif
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	char *attributes = NULL;
	int count;

#if (WIN32|WINNT)
	snprintf(cgi_env_str, BUFFERLEN, "CGI_REQUEST_METHOD=%s\nCGI_PATH_INFO=%s\nCGI_QUERY_STRING=%s",
	                     getenv("REQUEST_METHOD"),
	                     getenv("PATH_INFO"),
	                     getenv("QUERY_STRING"));
#else
	snprintf(cgi_env_str, BUFFERLEN, "CGI_REQUEST_METHOD=%s\nCGI_PATH_INFO=%s\nCGI_QUERY_STRING=%s",
	                     GLOBAL(request_info).request_method,
	                     GLOBAL(request_info).path_info,
	                     GLOBAL(request_info).query_string);
#endif
	/* !!!! memory for object, bodytag and attributes is allocated with malloc !!!! */
	if (0 != (ptr->lasterror =  send_pipecgi(ptr->socket,
#if APACHE
  serv->server_hostname,
#else
  getenv("HOSTNAME"),
#endif
  id, cgi_env_str, &attributes, &object, &count)))
		RETURN_FALSE;

	doc = malloc(sizeof(hw_document));
	doc->data = object;
	doc->attributes = attributes;
	doc->bodytag = NULL;
	doc->size = count;
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
	}
}
#undef BUFFERLEN
/* }}} */

/* {{{ proto void hw_insertdocument(int link, int parentid, hwdoc doc) 
   Insert new document */
PHP_FUNCTION(hw_insertdocument) {
	pval *arg1, *arg2, *arg3;
	int link, id, doc, type;
	hw_connection *ptr;
	hw_document *docptr;
	hw_objectID objid;
#if APACHE
	server_rec *serv = GLOBAL(php3_rqst)->server;
#endif
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find connection identifier %d",link);
		RETURN_FALSE;
	}

	doc=arg3->value.lval;
	docptr = php3_list_find(doc,&type);
	if(!docptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find document identifier %d",doc);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	if (0 != (ptr->lasterror =  send_putdocument(ptr->socket,
#if APACHE
  serv->server_hostname,
#else
  getenv("HOSTNAME"),
#endif
             id, docptr->attributes, docptr->data, docptr->size, &objid))) {
		RETURN_FALSE;
		}
	}
	RETURN_LONG(objid);
}
/* }}} */

/* {{{ proto hwdoc hw_new_document(string objrec, string data, int size)
   Create a new Hyperwave document */
PHP_FUNCTION(hw_new_document) {
	pval *arg1, *arg2, *arg3;
	char *ptr;
	hw_document *doc;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg1);
	convert_to_string(arg2);
	convert_to_long(arg3);

	doc = malloc(sizeof(hw_document));
	if(NULL == doc)
		RETURN_FALSE;
	doc->data = malloc(arg3->value.lval+1);
	if(NULL == doc->data) {
		free(doc);
		RETURN_FALSE;
	}
        memcpy(doc->data, arg2->value.str.val, arg3->value.lval);
	ptr = doc->data;
	ptr[arg3->value.lval] = '\0';
	doc->attributes = strdup(arg1->value.str.val);
	doc->bodytag = NULL;
	doc->size = arg3->value.lval;
	return_value->value.lval = php3_list_insert(doc,php3_hw_module.le_document);
	return_value->type = IS_LONG;
}
/* }}} */

/* {{{ proto void hw_free_document(hwdoc doc)
   Frees memory of document */
PHP_FUNCTION(hw_free_document) {
	pval *arg1;
	int id, type;
	hw_document *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	php3_list_delete(id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void hw_outputdocument(hwdoc doc)
   An alias for hw_output_document */
/* }}} */

/* {{{ proto void hw_output_document(hwdoc doc)
   Prints a Hyperwave document */
PHP_FUNCTION(hw_output_document) {
	pval *arg1;
	int id, type;
	hw_document *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	php3_header();
	php3_write(ptr->data, ptr->size);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string hw_documentbodytag(hwdoc doc [, string prefix])
   An alias for hw_document_bodytag() */
/* }}} */

/* {{{ proto string hw_document_bodytag(hwdoc doc [, string prefix])
   Return bodytag prefixed by prefix */
PHP_FUNCTION(hw_document_bodytag) {
	pval *argv[2];
	int id, type, argc;
	hw_document *ptr;
	char *temp, *str = NULL;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if((argc > 2) || (argc < 1))
		WRONG_PARAM_COUNT;
		
	if (getParametersArray(ht, argc, argv) == FAILURE)
		RETURN_FALSE;
	
	convert_to_long(argv[0]);
	id=argv[0]->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	if(argc == 2) {
		convert_to_string(argv[1]);
		str=argv[1]->value.str.val;
	}

	if(str != NULL) {
		temp = emalloc(argv[1]->value.str.len + strlen(ptr->bodytag) + 2);
		strcpy(temp, ptr->bodytag);
		strcpy(temp+strlen(ptr->bodytag)-1, str);
		strcpy(temp+strlen(ptr->bodytag)-1+argv[1]->value.str.len, ">\n");
		RETURN_STRING(temp, 0);
	} else {
		RETURN_STRING(ptr->bodytag, 1);
	}
}
/* }}} */

/* {{{ proto string hw_document_content(hwdoc doc)
   Returns content of document */
PHP_FUNCTION(hw_document_content) {
	pval *argv[1];
	int id, type, argc;
	hw_document *ptr;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if(argc != 1)
		WRONG_PARAM_COUNT;
		
	if (getParametersArray(ht, argc, argv) == FAILURE)
		RETURN_FALSE;
	
	convert_to_long(argv[0]);
	id=argv[0]->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	RETURN_STRING(ptr->data, 1);
}
/* }}} */

/* {{{ proto int hw_document_setcontent(hwdoc doc, string content)
   Sets/replaces content of document */
PHP_FUNCTION(hw_document_setcontent) {
	pval *argv[2];
	int id, type, argc;
	hw_document *ptr;
	char *str;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if(argc != 2)
		WRONG_PARAM_COUNT;
		
	if (getParametersArray(ht, argc, argv) == FAILURE)
		RETURN_FALSE;
	
	convert_to_long(argv[0]);
	convert_to_string(argv[1]);
	id=argv[0]->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	str = ptr->data;
	if(NULL != (ptr->data = strdup(argv[1]->value.str.val))) {
		ptr->size = strlen(ptr->data);
		free(str);
		RETURN_TRUE;
	} else {
		ptr->data = str;
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int hw_documentsize(hwdoc doc)
   An alias for hw_document_size() */
/* }}} */

/* {{{ proto int hw_document_size(hwdoc doc)
   Returns size of document */
PHP_FUNCTION(hw_document_size) {
	pval *arg1;
	int id, type;
	hw_document *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	RETURN_LONG(ptr->size);
}
/* }}} */

/* {{{ proto string hw_documentattributes(hwdoc doc)
   An alias for hw_document_attributes() */
/* }}} */

/* {{{ proto string hw_document_attributes(hwdoc doc)
   Returns object record of document */
PHP_FUNCTION(hw_document_attributes) {
	pval *arg1;
	int id, type;
	hw_document *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	ptr = php3_list_find(id,&type);
	if(!ptr || (type!=php3_hw_module.le_document)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	RETURN_STRING(ptr->attributes, 1);
/*	make_return_array_from_objrec(&return_value, ptr->attributes); */
}
/* }}} */

/* {{{ proto array hw_getparentsobj(int link, int objid)
   Returns array of parent object records */
PHP_FUNCTION(hw_getparentsobj) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);

	if (0 != (ptr->lasterror = send_getparentsobj(ptr->socket, id, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getparentsobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto array hw_getparents(int link, int objid)
   Returns array of parent object ids */
PHP_FUNCTION(hw_getparents) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
        {
	int  *childIDs = NULL;
	int i;

	if (0 != (ptr->lasterror = send_getparents(ptr->socket, id, &childIDs, &count))) {
		php3_error(E_WARNING, "send_command (getparents) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++) {
		add_index_long(return_value, i, childIDs[i]);
	}
	efree(childIDs);
	}

}
/* }}} */

/* {{{ proto array hw_children(int link, int objid)
   Returns an array of children object ids */
PHP_FUNCTION(hw_children) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	int  *childIDs = NULL;
	int i;

	if (0 != (ptr->lasterror = send_children(ptr->socket, id, &childIDs, &count))){
		php3_error(E_WARNING, "send_command (getchildcoll) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++) {
		add_index_long(return_value, i, childIDs[i]);
	}
	efree(childIDs);
	}
		
}
/* }}} */

/* {{{ proto array hw_childrenobj(int link, int objid)
   Returns an array of children object records */
PHP_FUNCTION(hw_childrenobj) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);

	if (0 != (ptr->lasterror = send_childrenobj(ptr->socket, id, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getchildcollobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto array hw_getchildcoll(int link, int objid)
   Returns an array of child collection object ids */
PHP_FUNCTION(hw_getchildcoll) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	int  *childIDs = NULL;
	int i;

	if (0 != (ptr->lasterror = send_getchildcoll(ptr->socket, id, &childIDs, &count))){
		php3_error(E_WARNING, "send_command (getchildcoll) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++) {
		add_index_long(return_value, i, childIDs[i]);
	}
	efree(childIDs);
	}
		
}
/* }}} */

/* {{{ proto array hw_getchildcollobj(int link, int objid)
   Returns an array of child collection object records */
PHP_FUNCTION(hw_getchildcollobj) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);

	if (0 != (ptr->lasterror = send_getchildcollobj(ptr->socket, id, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getchildcollobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto int hw_docbyanchor(int link, int anchorid)
   Returns an objid of document belonging to anchorid */
PHP_FUNCTION(hw_docbyanchor) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	int objectID;
	if (0 != (ptr->lasterror = send_docbyanchor(ptr->socket, id, &objectID)))
		RETURN_FALSE;

	RETURN_LONG(objectID);
	}
}
/* }}} */

/* {{{ proto array hw_docbyanchorobj(int link, int anchorid)
   Returns oan bject record of document belonging to anchorid */
PHP_FUNCTION(hw_docbyanchorobj) {
	pval *arg1, *arg2;
	int link, id, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *object = NULL;
	if (0 != (ptr->lasterror = send_docbyanchorobj(ptr->socket, id, &object)))
		RETURN_FALSE;

	RETURN_STRING(object, 0);
	/*
	make_return_array_from_objrec(&return_value, object);
	efree(object);
	*/
	}
}
/* }}} */

/* {{{ proto array hw_getobjectbyquery(int link, string query, int maxhits)
   Search for query and return maxhits objids */
PHP_FUNCTION(hw_getobjectbyquery) {
	pval *arg1, *arg2, *arg3;
	int link, type, maxhits;
	char *query;
	int count, i;
	int  *childIDs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	query=arg2->value.str.val;
	maxhits=arg3->value.lval;
	if (maxhits < 0) maxhits=0x7FFFFFFF;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getobjbyquery(ptr->socket, query, maxhits, &childIDs, &count))) {
		php3_error(E_WARNING, "send_command (getobjectbyquery) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++)
		add_index_long(return_value, i, childIDs[i]);
	efree(childIDs);
}
/* }}} */

/* {{{ proto array hw_getobjectbyqueryobj(int link, string query, int maxhits)
   Search for query and return maxhits object records */
PHP_FUNCTION(hw_getobjectbyqueryobj) {
	pval *arg1, *arg2, *arg3;
	int link, type, maxhits;
	char *query;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	query=arg2->value.str.val;
	maxhits=arg3->value.lval;
	if (maxhits < 0) maxhits=0x7FFFFFFF;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getobjbyqueryobj(ptr->socket, query, maxhits, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getobjectbyqueryobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto array hw_getobjectbyquerycoll(int link, int collid, string query, int maxhits)
   Search for query in collection and return maxhits objids */
PHP_FUNCTION(hw_getobjectbyquerycoll) {
	pval *arg1, *arg2, *arg3, *arg4;
	int link, id, type, maxhits;
	char *query;
	int count, i;
	hw_connection *ptr;
	int  *childIDs = NULL;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_string(arg3);
	convert_to_long(arg4);
	link=arg1->value.lval;
	id=arg2->value.lval;
	query=arg3->value.str.val;
	maxhits=arg4->value.lval;
	if (maxhits < 0) maxhits=0x7FFFFFFF;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getobjbyquerycoll(ptr->socket, id, query, maxhits, &childIDs, &count))) {
		php3_error(E_WARNING, "send_command (getobjectbyquerycoll) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++)
		add_index_long(return_value, i, childIDs[i]);
	efree(childIDs);
}
/* }}} */

/* {{{ proto array hw_getobjectbyquerycollobj(int link, int collid, string query, int maxhits)
   Search for query in collection and return maxhits object records */
PHP_FUNCTION(hw_getobjectbyquerycollobj) {
	pval *arg1, *arg2, *arg3, *arg4;
	int link, id, type, maxhits;
	char *query;
	int count;
	hw_connection *ptr;
	char  **childObjRecs = NULL;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_string(arg3);
	convert_to_long(arg4);
	link=arg1->value.lval;
	id=arg2->value.lval;
	query=arg3->value.str.val;
	maxhits=arg4->value.lval;
	if (maxhits < 0) maxhits=0x7FFFFFFF;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getobjbyquerycollobj(ptr->socket, id, query, maxhits, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getobjectbyquerycollobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto array hw_getchilddoccoll(int link, int objid)
   Returns all children ids which are documents */
PHP_FUNCTION(hw_getchilddoccoll) {
	pval *arg1, *arg2;
	int link, id, type;
	int count, i;
	int  *childIDs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getchilddoccoll(ptr->socket, id, &childIDs, &count))) {
		php3_error(E_WARNING, "send_command (getchilddoccoll) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(childIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++)
		add_index_long(return_value, i, childIDs[i]);
	efree(childIDs);
}
/* }}} */

/* {{{ proto array hw_getchilddoccollobj(int link, int objid)
   Returns all children object records which are documents */
PHP_FUNCTION(hw_getchilddoccollobj) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getchilddoccollobj(ptr->socket, id, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getchilddoccollobj) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;

}
/* }}} */

/* {{{ proto array hw_getanchors(int link, int objid)
   Return all anchors of object */
PHP_FUNCTION(hw_getanchors) {
	pval *arg1, *arg2;
	int link, id, type;
	int count, i;
	int  *anchorIDs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getanchors(ptr->socket, id, &anchorIDs, &count))) {
		php3_error(E_WARNING, "send_command (getanchors) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		efree(anchorIDs);
		RETURN_FALSE;
	}

	for(i=0; i<count; i++)
		add_index_long(return_value, i, anchorIDs[i]);
	efree(anchorIDs);
}
/* }}} */

/* {{{ proto array hw_getanchorsobj(int link, int objid)
   Return all object records of anchors of object */
PHP_FUNCTION(hw_getanchorsobj) {
	pval *arg1, *arg2;
	int link, id, type;
	int count;
	char  **anchorObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = (hw_connection *) php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getanchorsobj(ptr->socket, id, &anchorObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getanchors) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, anchorObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto string hw_getusername(int link)
   Returns the current user name */
PHP_FUNCTION(hw_getusername) {
	pval *arg1;
	int link, type;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	link = arg1->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	return_value->value.str.val = estrdup(ptr->username);
	return_value->value.str.len = strlen(ptr->username);
	return_value->type = IS_STRING;
}
/* }}} */

/* {{{ proto void hw_identify(int link, string username, string password)
   Identifies at Hyperwave server */
PHP_FUNCTION(hw_identify) {
	pval *arg1, *arg2, *arg3;
	int link, type;
	char *name, *passwd, *userdata;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_string(arg3);
	link = arg1->value.lval;
	name=arg2->value.str.val;
	passwd=arg3->value.str.val;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	{
	char *str;

	if (0 != (ptr->lasterror = send_identify(ptr->socket, name, passwd, &userdata))) {
		php3_error(E_WARNING, "send_identify returned %d\n", ptr->lasterror);
		if(ptr->username) free(ptr->username);
		ptr->username = NULL;
		RETURN_FALSE;
	}

	return_value->value.str.val = userdata;
	return_value->value.str.len = strlen(userdata);
	return_value->type = IS_STRING;
	if(ptr->username) free(ptr->username);
	str = userdata;
	while((*str != 0) && (*str != ' '))
		str++;
	if(*str != '\0')
		ptr->username = strdup(++str);
	else
		ptr->username = NULL;
	}
}
/* }}} */

/* {{{ proto array hw_objrec2array(string objrec)
   Returns object array of object record */
PHP_FUNCTION(hw_objrec2array) {
	pval *arg1;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	make_return_array_from_objrec(&return_value, arg1->value.str.val);
}
/* }}} */

/* {{{ proto string hw_array2objrec(array objarr)
   Returns an object record of object array */
PHP_FUNCTION(hw_array2objrec) {
	pval *arg1;
	char *objrec, *retobj;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_array(arg1);
	objrec = make_objrec_from_array(arg1->value.ht);
	if(objrec) {
		retobj = estrdup(objrec);
		free(objrec);
		RETURN_STRING(retobj, 0);
	} else
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto array hw_incollections(int link, array objids, array collids, int para)
   Returns object ids which are in collections */
PHP_FUNCTION(hw_incollections) {
	pval *arg1, *arg2, *arg3, *arg4;
	int type, link, i;
	hw_connection *ptr;
	int cobjids, ccollids, *objectIDs, *collIDs, cretids, *retIDs, retcoll;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_array(arg2);
	convert_to_array(arg3);
	convert_to_long(arg4);
	link = arg1->value.lval;
	retcoll=arg4->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	cobjids = _php3_hash_num_elements(arg2->value.ht);
	if(NULL == (objectIDs = make_ints_from_array(arg2->value.ht))) {
		php3_error(E_WARNING, "Could not create Int Array from Array\n");
		RETURN_FALSE;
	}

	ccollids = _php3_hash_num_elements(arg3->value.ht);
	if(NULL == (collIDs = make_ints_from_array(arg3->value.ht))) {
		php3_error(E_WARNING, "Could not create Int Array from Array\n");
		efree(objectIDs);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_incollections(ptr->socket, retcoll,
                                                      cobjids, objectIDs,
                                                      ccollids, collIDs,
                                                      &cretids, &retIDs))) {
		if(objectIDs) efree(objectIDs);
		if(collIDs) efree(collIDs);
		RETURN_FALSE;
	}

	if(objectIDs) efree(objectIDs);
	if(collIDs) efree(collIDs);

	if (array_init(return_value) == FAILURE) {
		efree(retIDs);
		RETURN_FALSE;
	}

	for(i=0; i<cretids; i++)
		add_index_long(return_value, i, retIDs[i]);
	efree(retIDs);

}
/* }}} */

/* {{{ proto void hw_inscoll(int link, int parentid, array objarr)
   Inserts a collection */
PHP_FUNCTION(hw_inscoll) {
	pval *arg1, *arg2, *arg3;
	char *objrec;
	int id, newid, type, link;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_array(arg3);
	link = arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	if(NULL == (objrec = make_objrec_from_array(arg3->value.ht))) {
		php3_error(E_WARNING, "Could not create Object Record from Array\n");
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_inscoll(ptr->socket, id, objrec, &newid))) {
		if(objrec) free(objrec);
		RETURN_FALSE;
	}

	if(objrec) free(objrec);
	RETURN_LONG(newid);
}
/* }}} */

/* {{{ proto void hw_insdoc(int link, int parentid, string objrec [, string text])
   Inserts document */
PHP_FUNCTION(hw_insdoc) {
	pval *argv[4];
	char *objrec, *text;
	int id, newid, type, link, argc;
	hw_connection *ptr;
	HW_TLS_VARS;

	argc = ARG_COUNT(ht);
	if((argc < 3) || (argc > 4))
		WRONG_PARAM_COUNT;

	if (getParametersArray(ht, argc, argv) == FAILURE)
		WRONG_PARAM_COUNT;

	convert_to_long(argv[0]);
	convert_to_long(argv[1]);
	convert_to_string(argv[2]);
	if(argc == 4) {
		convert_to_string(argv[3]);
		text = argv[3]->value.str.val;
	} else {
		text = NULL;
	}
	link = argv[0]->value.lval;
	id = argv[1]->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
        objrec = argv[2]->value.str.val;
	if (0 != (ptr->lasterror = send_insdoc(ptr->socket, id, objrec, text, &newid))) {
		RETURN_FALSE;
	}

	RETURN_LONG(newid);
}
/* }}} */

/* {{{ proto int hw_getsrcbydestobj(int link, int destid)
   Returns a object id of source document by destination anchor */
PHP_FUNCTION(hw_getsrcbydestobj) {
	pval *arg1, *arg2;
	int link, type, id;
	int count;
	char  **childObjRecs = NULL;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	link=arg1->value.lval;
	id=arg2->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_getsrcbydest(ptr->socket, id, &childObjRecs, &count))) {
		php3_error(E_WARNING, "send_command (getsrcbydest) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	/* create return value and free all memory */
	if( 0 > make_return_objrec(&return_value, childObjRecs, count))
		RETURN_FALSE;
}
/* }}} */

/* {{{ proto int hw_mapid(int link, int serverid, int destid)
   Returns a virtual object id of document on remote Hyperwave server */
PHP_FUNCTION(hw_mapid) {
	pval *arg1, *arg2, *arg3;
	int link, type, servid, id, virtid;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_long(arg3);
	link=arg1->value.lval;
	servid=arg2->value.lval;
	id=arg3->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = send_mapid(ptr->socket, servid, id, &virtid))) {
		php3_error(E_WARNING, "send_command (mapid) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}
	RETURN_LONG(virtid);
}
/* }}} */

/* {{{ proto string hw_getrellink(int link, int rootid, int sourceid, int destid)
   Get a link from a source to destination relative to rootid */
PHP_FUNCTION(hw_getrellink) {
	pval *arg1, *arg2, *arg3, *arg4;
	int link, type;
	int rootid, destid, sourceid;
	char *anchorstr;
	hw_connection *ptr;
	HW_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	convert_to_long(arg3);
	convert_to_long(arg4);
	link=arg1->value.lval;
	rootid=arg2->value.lval;
	sourceid=arg3->value.lval;
	destid=arg4->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}

	set_swap(ptr->swap_on);
	if (0 != (ptr->lasterror = getrellink(ptr->socket, rootid, sourceid, destid, &anchorstr))) {
		php3_error(E_WARNING, "command (getrellink) returned %d\n", ptr->lasterror);
		RETURN_FALSE;
	}

	RETURN_STRING(anchorstr, 0);
}
/* }}} */
	

void php3_info_hw()
{
	php3_printf("HG-CSP Version: 7.17");
}

/* {{{ proto void hw_connection_info(int link)
   Prints information about the connection to a Hyperwave server */
PHP_FUNCTION(hw_connection_info)
{
	pval *arg1;
	hw_connection *ptr;
	int link, type;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	link=arg1->value.lval;
	ptr = php3_list_find(link,&type);
	if(!ptr || (type!=php3_hw_module.le_socketp && type!=php3_hw_module.le_psocketp)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",link);
		RETURN_FALSE;
	}
	
	php3_printf("Hyperwave Info:\nhost=%s,\nserver string=%s\nversion=%d\nswap=%d\n", ptr->hostname, ptr->server_string, ptr->version, ptr->swap_on);
}
/* }}} */

void print_msg(hg_msg *msg, char *str, int txt)
{
     char *ptr;
     int  i;

     fprintf(stdout, "\nprint_msg: >>%s<<\n", str);
     fprintf(stdout, "print_msg: length  = %d\n", msg->length);
     fprintf(stdout, "print_msg: msgid = %d\n", msg->version_msgid);
     fprintf(stdout, "print_msg: msg_type  = %d\n", msg->msg_type);
     if ( msg->length > HEADER_LENGTH )  {
          ptr = msg->buf;
          for ( i = 0; i < msg->length-HEADER_LENGTH; i++ )  {
               if ( *ptr == '\n' )
                    fprintf(stdout, "%c", *ptr++);
               else if ( iscntrl(*ptr) )
                    {fprintf(stdout, "."); ptr++;}
               else
                    fprintf(stdout, "%c", *ptr++);
          }
     }
     fprintf(stdout, "\n\n");
}

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
