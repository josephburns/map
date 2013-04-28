/*
*
* mapconf.c - map configuration
*
* Joseph Burns
* 03-19-2007
*
*/

#include "mapconf.h"
#include "mapfn.h"

/* table for map generalization */

mapfns mapfntab[] = {
   {	
      CHPTR_CHPTR,      /* a string-string key/value pair map */
      sp_k_hash, 
      sp_kv_put, 
      sp_kv_get, 
      sp_kv_remove, 
      sp_map_destroy, 
      sp_map_rehash, 
      sp_map_keys 
   },
   {
      CHPTR_LONG,       /* string-long key/value pair map */
      sp_k_hash,        /* reuse above string hashing function */
      si_kv_put,        /* this function specific to chptr-long map type */
      si_kv_get,        /* get also specific to chptr-long */
      sp_kv_remove,     /* sp function works */
      sp_map_destroy,   /* can reuse this also, nothing type specific  */
      sp_map_rehash,    /* this function generalized so it works for both */
      sp_map_keys       /* reuse above keys function */
   }
};


