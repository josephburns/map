/*
*
* mapfn.h - function prototypes for type-specific map functions
*
* Joseph Burns
* 03-19-2007 create
*
*/

#ifndef MAPFN_H
#define MAPFN_H

#include "map.h"

/*
*
* key: 
* sp_<function> - map functions for char ptr - char ptr map type
* si_<function> - map functions for char ptr - long map type 
*
*/

/* chptr-chptr functions */

/* given a map ptr and key ptr, return a hash of that key specific to key type */
unsigned long sp_k_hash(map*,void*);

/* given map, key, and value ptrs allocate memory for a new kvpair and insert the data: return a status code */
int sp_kv_put(map**,void*,void*); 

/* given a map, key, and value dbl ptr, find value in the map and set ptr to it: return a status code */
int sp_kv_get(map*,void*,void**);

/* given map and key ptrs, remove and deallocate the memory for the kvpair match and return a status code */
int sp_kv_remove(map*,void*);

/* given a ptr to a map, remove and deallocate memory for all kvpairs and buckets in the map */
int sp_map_destroy(map*);

/* given a ptr to a map, perform a rehashing operation and return a status code */
int sp_map_rehash(map**); 

/* given a map and key type double ptr, alloc and create keys array and point double ptr to it, return status */
int sp_map_keys(map*,void***);

/* chptr-long specific functions, for others sp functions reused */ 

/* given a map ptr and key ptr, return a hash of that key specific to key type */
int si_kv_put(map**,void*,void*);

/* given map, key, and value ptrs allocate memory for a new kvpair and insert the data: return a status code */
int si_kv_get(map*,void*,void**);

#endif
