/****************************************************************************
*
*					Copyright (C) 1991 Kendall Bennett.
*							All rights reserved.
*
* Filename:		$RCSfile: DList.h,v $
* Version:		$Revision: 1.4 $
*
* Language:		ANSI C
* Environment:	any
*
* Description:	Header file for doubly linked list routines.
*
* $Id: DList.h,v 1.4 1999/05/28 12:14:26 sas Exp $
*
* Revision History:
* -----------------
*
* $Log: DList.h,v $
* Revision 1.4  1999/05/28 12:14:26  sas
* as reported in #1353
*
* Revision 1.3  1999/03/09 17:49:55  rasmus
* Eliminate symbol conflicts here
*
* Revision 1.2  1998/08/14 15:51:12  shane
* Some work on getting hyperwave to work on windows.  hg_comm needs a lot of work.
*
* Mainly, signals, fnctl, bzero, bcopy, etc are not portable functions.  Most of this
* will have to be rewriten for windows.
*
* Revision 1.1  1998/08/12 09:29:16  steinm
* First version of Hyperwave module.
*
* Revision 1.5  91/12/31  19:40:54  kjb
* 
* Modified include files directories.
* 
* Revision 1.4  91/09/27  03:10:41  kjb
* Added compatibility with C++.
* 
* Revision 1.3  91/09/26  10:07:16  kjb
* Took out extern references
* 
* Revision 1.2  91/09/01  19:37:20  ROOT_DOS
* Changed PHP_DLST_TAIL macro so that it returns a pointer to the REAL last
* node of the list, not the dummy last node (l->z).
* 
* Revision 1.1  91/09/01  18:38:23  ROOT_DOS
* Initial revision
* 
****************************************************************************/

#ifndef	__DLIST_H
#define	__DLIST_H

#ifndef	__DEBUG_H
/*#include "debug.h"*/
#endif

/*---------------------- Macros and type definitions ----------------------*/

typedef struct PHP_DLST_BUCKET {
	struct PHP_DLST_BUCKET	*next;
	struct PHP_DLST_BUCKET	*prev;
	} PHP_DLST_BUCKET;

/* necessary for AIX 4.2.x */

#ifdef hz
#undef hz
#endif

typedef struct {
	int			count;			/* Number of elements currently in list	*/
	PHP_DLST_BUCKET	*head;			/* Pointer to head element of list		*/
	PHP_DLST_BUCKET	*z;				/* Pointer to last node of list			*/
	PHP_DLST_BUCKET	hz[2];			/* Space for head and z nodes			*/
	} DLIST;

/* Return a pointer to the user space given the address of the header of
 * a node.
 */

#define	PHP_DLST_USERSPACE(h)	((void*)((PHP_DLST_BUCKET*)(h) + 1))

/* Return a pointer to the header of a node, given the address of the
 * user space.
 */

#define	PHP_DLST_HEADER(n)		((PHP_DLST_BUCKET*)(n) - 1)

/* Return a pointer to the user space of the list's head node. This user
 * space does not actually exist, but it is useful to be able to address
 * it to enable insertion at the start of the list.
 */

#define	PHP_DLST_HEAD(l)		PHP_DLST_USERSPACE((l)->head)

/* Return a pointer to the user space of the last node in list.	*/

#define	PHP_DLST_TAIL(l)		PHP_DLST_USERSPACE((l)->z->prev)

/* Determine if a list is empty
 */

#define	PHP_DLST_EMPTY(l)		((l)->count == 0)

/*-------------------------- Function Prototypes --------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void *dlst_newnode(int size);
void dlst_freenode(void *node);
DLIST *dlst_init(void);
void dlst_kill(DLIST *l,void (*freeNode)(void *node));
void dlst_insertafter(DLIST *l,void *node,void *after);
void *dlst_deletenext(DLIST *l,void *node);
void *dlst_first(DLIST *l);
void *dlst_last(DLIST *l);
void *dlst_next(void *prev);
void *dlst_prev(void *next);
void dlst_mergesort(DLIST *l,int (*cmp_func)(void*,void*));

#ifdef __cplusplus
}
#endif

#endif
