/*
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 2000 PHP Development Team (See Credits file)           |
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
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
 */

/* $Id: dba_db2.c,v 1.13 2000/07/10 10:15:36 sas Exp $ */

#include "php.h"

#if DBA_DB2
#include "php3_db2.h"
#include <sys/stat.h>

#include <string.h>
#if DB2_DB2_DB_H
#include <db2/db.h>
#elif DB2_DB_DB2_H
#include <db/db2.h>
#elif DB2_DB2_H
#include <db2.h>
#elif DB2_DB_H
#include <db.h>
#endif

#define DB2_DATA dba_db2_data *dba = info->dbf
#define DB2_GKEY \
	DBT gkey; \
	memset(&gkey, 0, sizeof(gkey)); \
	gkey.data = (char *) key; gkey.size = keylen

typedef struct {
	DB *dbp;
	DBC *cursor;
} dba_db2_data;

DBA_OPEN_FUNC(db2)
{
	DB *dbp;
	DBTYPE type;
	int gmode = 0;
	int filemode = 0644;
	struct stat check_stat;

	type =  info->mode == DBA_READER ? DB_UNKNOWN :
		info->mode == DBA_TRUNC ? DB_BTREE :
		stat(info->path, &check_stat) ? DB_BTREE : DB_UNKNOWN;
	  
	gmode = info->mode == DBA_READER ? DB_RDONLY :
		info->mode == DBA_CREAT  ? DB_CREATE : 
		info->mode == DBA_WRITER ? 0         : 
		info->mode == DBA_TRUNC ? DB_CREATE | DB_TRUNCATE : -1;

	if(gmode == -1)
		return FAILURE;

	if(info->argc > 0) {
		convert_to_long(info->argv[0]);
		filemode = info->argv[0]->value.lval;
	}

	if(!db_open(info->path, type, gmode, filemode, NULL, NULL, &dbp)) {
		info->dbf = malloc(sizeof(dba_db2_data));
		memset(info->dbf, 0, sizeof(dba_db2_data));
		((dba_db2_data *) info->dbf)->dbp = dbp;
		return SUCCESS;
	}
	return FAILURE;
}

DBA_CLOSE_FUNC(db2)
{
	DB2_DATA;
	
	if(dba->cursor) dba->cursor->c_close(dba->cursor);
	dba->dbp->close(dba->dbp, 0);
	free(dba);
}

DBA_FETCH_FUNC(db2)
{
	DBT gval;
	char *new = NULL;
	DB2_DATA;
	DB2_GKEY;
	
	memset(&gval, 0, sizeof(gval));
	if(!dba->dbp->get(dba->dbp, NULL, &gkey, &gval, 0)) {
		if(newlen) *newlen = gval.size;
		new = estrndup(gval.data, gval.size);
	}
	return new;
}

DBA_UPDATE_FUNC(db2)
{
	DBT gval;
	DB2_DATA;
	DB2_GKEY;
	
	memset(&gval, 0, sizeof(gval));
	gval.data = (char *) val;
	gval.size = vallen;

	if(!dba->dbp->put(dba->dbp, NULL, &gkey, &gval, 
				mode == 1 ? DB_NOOVERWRITE : 0)) {
		return SUCCESS;
	}
	return FAILURE;
}

DBA_EXISTS_FUNC(db2)
{
	DBT gval;
	DB2_DATA;
	DB2_GKEY;
	
	memset(&gval, 0, sizeof(gval));
	if(!dba->dbp->get(dba->dbp, NULL, &gkey, &gval, 0)) {
		return SUCCESS;
	}
	return FAILURE;
}

DBA_DELETE_FUNC(db2)
{
	DB2_DATA;
	DB2_GKEY;

	return dba->dbp->del(dba->dbp, NULL, &gkey, 0) ? FAILURE : SUCCESS;
}

DBA_FIRSTKEY_FUNC(db2)
{
	DB2_DATA;

	if(dba->cursor) {
		dba->cursor->c_close(dba->cursor);
	}

	dba->cursor = NULL;
#if (DB_VERSION_MAJOR > 2) || (DB_VERSION_MAJOR == 2 && DB_VERSION_MINOR > 6) || (DB_VERSION_MAJOR == 2 && DB_VERSION_MINOR == 6 && DB_VERSION_PATCH >= 4)
	if(dba->dbp->cursor(dba->dbp, NULL, &dba->cursor, 0)) {
#else
	if(dba->dbp->cursor(dba->dbp, NULL, &dba->cursor)) {
#endif
		return NULL;
	}

	/* we should introduce something like PARAM_PASSTHRU... */
	return dba_nextkey_db2(info, newlen);
}

DBA_NEXTKEY_FUNC(db2)
{
	DB2_DATA;
	DBT gkey, gval;
	char *nkey = NULL;
	
	memset(&gkey, 0, sizeof(gkey));
	memset(&gval, 0, sizeof(gval));

	if(!dba->cursor->c_get(dba->cursor, &gkey, &gval, DB_NEXT)) {
		if(gkey.data) {
			nkey = estrndup(gkey.data, gkey.size);
			if(newlen) *newlen = gkey.size;
		}
	}
	return nkey;
}

DBA_OPTIMIZE_FUNC(db2)
{
	return SUCCESS;
}

DBA_SYNC_FUNC(db2)
{
	DB2_DATA;

	return dba->dbp->sync(dba->dbp, 0) ? FAILURE : SUCCESS;
}

#endif
