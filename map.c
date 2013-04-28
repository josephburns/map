/*
*
*  map.c
*  Joseph Burns
*  02-16-07
*
*  map functions implementing a hashmap of variable typess 
*  separate chaining is used for collisions
*
*/

#include <stdlib.h>
#include <stdio.h>

#include "map.h"
#include "mapconf.h"

/*
*
*  map_init - initialize a map using the default values
*
*/

int map_init(map **themap,int type) {

   return create_map(themap, type, (long)START_SIZE, (long)RESIZE_FACTOR, (double)LOAD_FACTOR);
}

/*
*
*  map_create - creates a map
*
*/

int create_map(map **inhashmap, int thetype, long start_size, int rsize_factor, double ld_factor) {
   
   bucket *curr = NULL;
   map *hashmap = NULL; 

   int i;

   hashmap = (map*)malloc(sizeof(map));

   hashmap->size = 0;
   hashmap->bucketsize = start_size;
   hashmap->resize_factor = rsize_factor;
   hashmap->load_factor = ld_factor;
   hashmap->currentload = 0.0;
   hashmap->bucketsused = 0;
   hashmap->scale = rand()% 4 + 1; /* keep coefficient small */
   hashmap->offset = rand() % hashmap->bucketsize;
   hashmap->type = thetype;   

   /* allocate space for (all of) the starting buckets */

#ifdef DEBUG
      printf("DEBUG: create_map - allocating %lu buckets with map type %d\n",hashmap->bucketsize,hashmap->type); 
#endif

   hashmap->start = (bucket*)malloc((sizeof(bucket) * hashmap->bucketsize));
   if( hashmap->start == NULL) {
      perror("malloc of buckets failed");
      return FAILURE;
   }   
   else {   
      curr = hashmap->start;
      /* initialize the buckets */
      for(i=0; i < hashmap->bucketsize; i++) {
         curr->start = NULL;                       /* set kvpair to null */
         curr->index = i;                          /* set hashcode of this bucket */
         curr->count = 0;                          /* empty now */
         curr++;                                   /* advance pointer */
      }
   }

   *inhashmap = hashmap; 
   return SUCCESS;
}

/*
*
* isEmpty - return whether the map is empty or not
*
*/

int isempty(map *themap) {

   return ((themap->size) > 0 ? FALSE : TRUE);
}

/*
*
* size - return how many key value pairs are in the map
*
*/

unsigned long map_size(map *themap) {
   return themap->size;
}

/*
*
* k_hash - wrapped hash function
*
*/

unsigned long k_hash(map *themap,void *key) {
   return mapfntab[themap->type].fn_hash(themap,key);
}

/*
*
* kv_put - wrapped put function
*
*/

int kv_put(map **ppthemap, void *key, void *value) {
   int code;
   code =  mapfntab[(*ppthemap)->type].fn_put(ppthemap, key, value);

#ifdef DEBUG
   printf("DEBUG: leaving kv_put (caller): map size %lu, bucketsize %lu, bucketsused %lu\n", (*ppthemap)->size, (*ppthemap)->bucketsize, (*ppthemap)->bucketsused);
#endif

   return code;
}

/*
*
* kv_get - wrapped get function
*
*/

int kv_get(map *themap, void *key, void **newvalue) {
   return mapfntab[themap->type].fn_get(themap, key, newvalue);
}

/*
*
* kv_remove - wrapped remove function
*
*/

int kv_remove(map *themap, void *key) {
   return mapfntab[themap->type].fn_remove(themap, key);
}

/*
*
* map_destroy - wrapped map destroy function
*
*/

int map_destroy(map *themap) {
   return mapfntab[themap->type].fn_destroy(themap);
}

/*
*
* map_rehash - wrapped rehash functio
*
*/

int map_rehash(map **themap) {
   return mapfntab[(*themap)->type].fn_rehash(themap);
}

/*
*
* map_keys - wrapped keys function
*
*/

int map_keys(map *themap, void ***keyptr) {
   return mapfntab[themap->type].fn_keys(themap,keyptr);
}
