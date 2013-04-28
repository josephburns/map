/*
*
* map.h - header file for general map data structures and function prototypes
*
* Joseph Burns
* 03-19-2007 create
*
*/

#ifndef MAP_H 
#define MAP_H

typedef enum {FAILURE,SUCCESS,NOTFOUND} status;
typedef enum {EQUAL,NOTEQUAL} comparison;
typedef enum {FALSE,TRUE} boolean;

/* #define DEBUG  */

#ifndef LOAD_FACTOR
#define LOAD_FACTOR 0.5    /* load factor contant */
#endif

#ifndef START_SIZE
#define START_SIZE 5       /* starting size of the map */
#endif

#ifndef RESIZE_FACTOR
#define RESIZE_FACTOR 2    /* factor by which the bucket size is increased */
#endif

/* the key-value pair structure.  void *'s which are cast in map-specific functions */
typedef struct kvpair {
   void *key;                    /* the key */
   void *value;                  /* the value associated with this key */
   struct kvpair *next;          /* given separate chaining, the next node with the same hash value */
} kvpair;

/* bucket structure which holds kvpairs */ 
typedef struct bucket {
   kvpair *start;
   unsigned long index;          /* the hashcode of the bucket */
   long count;                   /* number of kvpairs in the bucket */
} bucket;

/* the outer map structure */
typedef struct map {
   bucket* start;                         /* ptr to the first bucket */
   unsigned long bucketsize;              /* number of buckets in the map */
   unsigned long size;                    /* number of kvpairs in the map */
   unsigned long bucketsused;             /* number of buckets that are being used */
   unsigned int resize_factor;            /* factor by which the bucket size is increased */
   double currentload;                    /* proportion of buckets used */
   double load_factor;                    /* proportion at which the map is rehashed */
   unsigned int scale;                    /* coefficient used after hash */
   unsigned int offset;                   /* hash offset */
   /* pthread_mutex_t mapmutex; */        /* TODO: mutex for map operations */
   status type;                           /* the type of this map */
} map;


/* general map function prototypes - these are not specific to the map type */

/* return the size of the map */
unsigned long map_size(map *);

/* return a boolean: whether or not the map is empty */
int map_isempty(map *);

/* allocate the memory for a map, set a ptr to it, and return a status code */
int map_create(map**, int, long, int, double);

/* call map_create with default values */
int map_init(map**, int);


/* wrapped functions, the map type field is used to call the appropriate function */

/* given a map ptr and key ptr, return a hash of that key specific to key type */ 
unsigned long k_hash(map*,void*);

/* given map, key, and value ptrs allocate memory for a new kvpair and insert the data: return a status code */
int kv_put(map**,void*,void*);

/* given a map, key, and value type ptrs, find value in the map and set ptr to it: return a status code */ 
int kv_get(map*,void*,void**);

/* given map and key ptrs, remove and deallocate the memory for the kvpair match and return a status code */
int kv_remove(map*,void*);

/* given a ptr to a map, remove and deallocate memory for all kvpairs and buckets in the map */
int map_destroy(map*);  

/* given a ptr to a map, perform a rehashing operation and return a status code */
int map_rehash(map**);

/* given a map and key type double ptr, alloc and create keys array and point double ptr to it, return status */
int map_keys(map*,void***);


#endif
