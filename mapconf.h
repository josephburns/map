/*
*
* mapconf.h - map configuration header file
* 
* Joseph Burns
* 03-19-2007
*
*/

#ifndef MAPCONF_H
#define MAPCONF_H

#include "map.h"

typedef enum {CHPTR_CHPTR, CHPTR_LONG} maptype;

typedef struct {
   maptype type;                             /* the type of the map */
   unsigned long (*fn_hash)(map*,void*);     /* hash function for this map's key type */  
   int (*fn_put)(map**,void*,void*);         /* put function */
   int (*fn_get)(map*,void*,void**);         /* get function */
   int (*fn_remove)(map*,void*);             /* remove function */
   int (*fn_destroy)(map*);                  /* map destroy function */
   int (*fn_rehash)(map**);                  /* rehash map function */
   int (*fn_keys)(map*,void***);             /* given map and triple ptr */
} mapfns;

extern mapfns mapfntab[];

#endif
