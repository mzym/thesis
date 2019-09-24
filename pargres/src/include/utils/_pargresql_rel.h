/*-------------------------------------------------------------------------
 *
 * _pargresql_rel.h
 *	  PARGRESQL portions for inclusion into rel.h
 *
 * Constantin S. Pan, 2010
 *
 *-------------------------------------------------------------------------
 */
#ifndef _PARGRESQL_REL_H
#define _PARGRESQL_REL_H

/* 
 * This stuff goes into RelationData struct
 */
typedef struct _pargresql_RelationData {
	int fragment_func_divby; // 2nd argument for the hardcoded fragmentation function - mod()
} _pargresql_RelationData;

#endif   /* _PARGRESQL_REL_H */
