/*
*
* mapfn.c - type-specific map functions
*
* Joseph Burns
* 03-19-2007 create
*
*/

#include <stdlib.h>
#include <stdio.h>
#include "mapfn.h"
#include "mapconf.h"

unsigned long chptr_hash(unsigned char *);
kvpair* sp_createkvpair(char *, char*);
kvpair* si_createkvpair(char *, long);
unsigned long nextprobableprime(unsigned long);

/*
*
*  hash - returns a hash of the given character*
*
*  uses the djb2 algorithm for now: described at
*  http://www.cse.yorku.ca/~oz/hash.html
*  todo: test performance of some other string
*  hashing functions
*
*/

unsigned long chptr_hash(unsigned char *str) {
   unsigned long hashvalue = 5381;
   int c;
   while ((c = *str++))
      hashvalue = ((hashvalue << 5) + hashvalue) + c;
   return hashvalue;
}

/*
*
*  gethash - wrap hash with map specifics
*
*  ((mx+b) % size) used, where:
*  m = scale
*  b = offset
*  size = number of current buckets in the map
*  these are all properties of the map structure
*
*/

unsigned long sp_k_hash(map* themap, void* string) {
   return ((themap->scale) * chptr_hash((char*)string) + themap->offset) % themap->bucketsize;
}

/*
*
* createkvpair - create a kvpair
*
* given key and value character pointers, allocate the
* memory for a key value pair and return a pointer to it
* after a deep copy of the values
*
* return FAILURE if the memory allocation fails
*
*/

kvpair* sp_createkvpair(char *k, char *v) {

   kvpair* newpair = NULL;
   newpair = (kvpair*)malloc(sizeof(kvpair));
   if(newpair == NULL) { /* create a new kvpair */
      perror("malloc of kvpair failed");
      return FAILURE; 
   }
   else { /* malloc of kvpair ok */
      newpair->key = (void*)malloc(sizeof(char)*strlen(k) + 1);
      if(newpair->key == NULL) {
         perror("malloc of new key failed\n"); 
         return FAILURE;
      }
      newpair->value = (void*)malloc(sizeof(char)*strlen(v) +1);
      if(newpair->value == NULL) {
         perror("malloc of new value failed\n"); 
         return FAILURE;
      }
      strcpy((char*)newpair->key,k); 
      strcpy((char*)newpair->value,v); 
   }
   newpair->next = NULL; /* next always null when kvpair created */
   return newpair;
}


/*
*
* sp_kv_put - put a chptr,chptr kv pair into the map
*
* if the key already exists, overwrite the existing value
* if the load is exceeded, resize the buckets and rehash
* return the value inserted. if a value is replaced,
* that memory is freed and the new value is returned 
* return NULL in the case of a failure (mem alloc, etc.)
*
*/

int sp_kv_put(map **inmap, void *inkey, void *invalue) {
	
   unsigned long code;
	bucket *bucketptr;
	char *prev = NULL;
   
   map *themap = *inmap; 
   char* key = (char*)inkey;
   char* value = (char*)invalue;

#ifdef DEBUG
				printf("DEBUG: enter sp_kv_put with arguments key: %s value: %s - hashcode: %d\n",key,value,(int)sp_k_hash(themap,key));
				printf("DEBUG: enter sp_kv_put map size: %d, bucketsused: %d, currentload: %f, loadfactor: %f\n",(int)themap->size,(int)themap->bucketsused,themap->currentload,themap->load_factor);
#endif

	bucketptr = themap->start; /* point to first bucket in map */
	code = sp_k_hash(themap, key);		/* use wrapped hash function */
	bucketptr += code; 		/* point to bucket corresponding to value */
	if(bucketptr->start == NULL) { /* no value at this hash */
		bucketptr->start = sp_createkvpair(key,value);
		if(bucketptr->start == NULL) {
			perror("new kvpair allocation failed");
			return FAILURE;
		}
		else { /* we have a kvpair, update map state */
			themap->size++; /* increase the kvpair count */
			themap->bucketsused++; /* an empty bucket now used */
			themap->currentload = (double)(themap->bucketsused) / (double)(themap->bucketsize);
			if(themap->currentload > themap->load_factor) {
				sp_map_rehash(inmap);  /* buckets too full, rehash */

#ifdef DEBUG
	printf("DEBUG: sp_kv_put - just rehashed: map size: %lu, bucketsize: %lu bucketsused: %lu, currentload: %f, loadfactor: %f\n",(*inmap)->size,(*inmap)->bucketsize,(*inmap)->bucketsused,(*inmap)->currentload,(*inmap)->load_factor);
#endif

			}
		}
	}
	else { /* we have a collision to deal with at this hash */
		kvpair *entry,*preventry;
		entry = bucketptr->start;
		while(entry != NULL) { /* cycle through kvpairs looking for match */ 
			if(strcmp(entry->key, key) == 0) { /* we have a match */
				prev = entry->value; /* set pointer to old value */

#ifdef DEBUG
				printf("DEBUG: sp_kv_put - collision - freeing old and inserting new value\n");
#endif

				if(prev != NULL)
					free((void*)prev); /* free up old value */
				prev = NULL;
				entry->value = (char*)malloc(sizeof(value) + 1); /* alloc new */
				strcpy(entry->value,value); /* put new one in */
				return SUCCESS; /* return map */
			}
			preventry = entry; /* set the previous entry */
			entry = entry->next; /* advance to next kvpair in list */
		}
		entry = sp_createkvpair(key,value); /* at the end of kvpairs, no match, create one */
		if(entry == NULL) {
			perror("allocate of kvpair at end of list failed");
			return FAILURE;
		}
		else { /* new kvpair added at end of list, update map state */
			themap->size++; /* increase the kvpair count */
			preventry->next = entry; /* update the pointer to extend the chain */
		}	
	}

#ifdef DEBUG
	printf("DEBUG: leaving sp_kv_put - map size: %lu, bucketsused: %lu, currentload: %f, loadfactor: %f bucketsize: %lu\n",(*inmap)->size,(*inmap)->bucketsused,(*inmap)->currentload,(*inmap)->load_factor,(*inmap)->bucketsize);
#endif

	return SUCCESS;
}


/*
*
*	get - given a key, get the (char*) value
*
*/

int sp_kv_get(map *themap, void *key, void **value) {

	unsigned long code = 0;
	bucket* buck = NULL;
	kvpair *kvcurr = NULL;

	code = sp_k_hash(themap, key); /* hash the key */

#ifdef DEBUG
	      printf("DEBUG: enter sp_kv_get - key %s hashes to %lu\n",(char*)key,code);
#endif

   buck = themap->start + code; /* go to the appropriate bucket */	
	kvcurr = buck->start;	

	while(kvcurr != NULL) {

#ifdef DEBUG
	      printf("DEBUG: sp_kv_get - kvcurr key %s\n",(char*)kvcurr->key);
#endif

      if(strcmp(((char*)kvcurr->key),(char*)key) == 0) { /* we have a match */
         /* value returned is not the data in the map, deep copied */
		   (*value) = malloc(sizeof(char) * strlen(kvcurr->value) + 1);
         strcpy((*value),kvcurr->value);

#ifdef DEBUG
	      printf("DEBUG: sp_kv_get - get match for %s returns %s\n",(char*)key,(char*)(*value));
#endif

         return SUCCESS; 


		}
		kvcurr = kvcurr->next;
	}

   *value = NULL;    /* if not found, be sure to set value pointer to NULL */     
	return NOTFOUND;  /* not found */

}


/*
*
* removekv - given a map and search char *,
* see if there is a key that matches the search string
* and remove the corresponding key/value pair
*
*/

int sp_kv_remove(map *themap, void *searchstring) {

	bucket *bucketptr=NULL;
	kvpair *prev=NULL,*curr=NULL;
	int i=0,code;

#ifdef DEBUG
	printf("DEBUG: enter sp_kv_remove, search string %s\n",(char*)searchstring);
#endif

	code = sp_k_hash(themap, (char*)searchstring);
	bucketptr = themap->start + code; /* advance pointer to proper bucket */
	
	if(bucketptr == NULL)
		return FALSE; /* it isn't here */
	else {
		prev = curr = bucketptr->start; /* set the kvpair pointer */
		while(curr != NULL) {
		 	if(strcmp(curr->key, (char*)searchstring)==0) { /* we have a match */

#ifdef DEBUG
				   printf("DEBUG: sp_kv_remove - match found for %s, is the %d'th kvpair in a bucket\n",(char*)searchstring,i);
#endif

				if(i==0) /* this is the first kvpair in the bucket, must be treated differently */ 
					bucketptr->start = curr->next;
				else
					prev->next = curr->next; /* take the current one out of the chain */
				if(curr->key != NULL) 
					free((void*)curr->key);
            curr->key = NULL;
				if(curr->value != NULL)
					free((void*)curr->value);
            curr->value = NULL;
            if(curr != NULL) 
               free((void*)curr);
            curr = prev = NULL;
            
				return TRUE;
			}
			prev = curr;
			curr = curr->next;
			i++; /* advance i, now we know it isn't the first one in the bucket */
		}

#ifdef DEBUG
		   printf("DEBUG: sp_kv_remove - no match found for search string %s, exiting removekv\n",(char*)searchstring);
#endif
		return NOTFOUND;
	}
}


/*
*
*	removemap - delete all memory associated with a map
*
*/

int sp_map_destroy(map *themap) {

	bucket *bucketptr = NULL;
	kvpair *kvcurr = NULL, *kvprev = NULL;
	int i=0;

#ifdef DEBUG
				printf("DEBUG: enter sp_map_destroy\n"); 
#endif

	bucketptr = themap->start;

	for(i=0; i < themap->bucketsize; i++) {
		if(bucketptr->start != NULL) { /* entries exist */
			kvcurr = bucketptr->start;
			while(kvcurr != NULL) { /* free up each kvpair */

#ifdef DEBUG
            if(themap->type == CHPTR_CHPTR)
				   printf("DEBUG: freeing curr key \"%s\" and value \"%s\"\n",(char*)kvcurr->key,(char*)kvcurr->value); 
			   else if (themap->type == CHPTR_LONG)	
               printf("DEBUG: freeing curr key \"%s\" and value \"%ld\"\n",(char*)kvcurr->key,*((long*)kvcurr->value));  
#endif

				if(kvcurr->key != NULL) 	
					free(kvcurr->key);
				kvcurr->key = NULL;
				if(kvcurr->value != NULL)	
					free(kvcurr->value);
				kvcurr->value = NULL;
				kvprev = kvcurr;
				kvcurr = kvcurr->next;
				kvprev->next = NULL;

#ifdef DEBUG
				printf("DEBUG: sp_kv_remove - removing prev keyvalue pair\n"); 
#endif

				if(kvprev != NULL)	
					free((void*)kvprev);
				kvprev = NULL;
			}
		}
		bucketptr++;
	}
	
	if(themap->start != NULL) /* free buckets */
		free((void*)themap->start);
   themap->start = NULL;

#ifdef DEBUG
	printf("DEBUG: sp_kv_remove - exit - set map to NULL\n");
#endif
	
	themap = NULL;
   return SUCCESS;
}


/*
*
*	rehash - resize and rehash all values once the load factor is reached
*
*/

int sp_map_rehash(map **incmap) {
   
   map *themap = *incmap;     /* another level of indirection */
	map *pnewmap = NULL;		      /* the bigger map */ 
	kvpair *curr=NULL,*prev=NULL;
	bucket *bucketptr=NULL;
   status code;
	long i=0;

#ifdef DEBUG
   printf("*********************************************************************************************\n");
	printf("DEBUG: enter sp_map_rehash - map size %lu, bucketsize %lu, bucketsused %lu\n",themap->size, themap->bucketsize,themap->bucketsused); 
#endif
   pnewmap = (map*)malloc(sizeof(map)); /* allocate memory for the new map */
	
   code = create_map(&pnewmap, themap->type, nextprobableprime((themap->bucketsize) * (themap->resize_factor)), themap->resize_factor, themap->load_factor);
	
	bucketptr = themap->start;

	for(i=0;i<themap->bucketsize;i++) {
		if((bucketptr->start) == NULL) {
			bucketptr++;
			continue; 	/* no kvpairs at this hashcode */

#ifdef DEBUG
			printf("DEBUG: sp_map_rehash - ** no kvpairs at this hashcode, continue **\n");
#endif

		}
		else {
			curr = bucketptr->start; /* set curr to the first kvpair */
			while(curr!=NULL) { /* now iterate through all kvpairs at this hashcode */

#ifdef DEBUG
	printf("DEBUG: putting key: %s value: %s into new map\n",(char*)curr->key,(char*)curr->value);
#endif

				kv_put(&pnewmap, (void*)curr->key, (void*)curr->value); /* put the key value pair into the new map */
				prev = curr;
				curr = curr->next;
				if((prev->key)!=NULL) 
			   	free((void*)prev->key);
				prev->key = NULL;
				
				if((prev->value)!=NULL)	
					free((void*)prev->value);
				prev->value = NULL;
				
            if(prev!=NULL)
               free((void*)prev);
            prev = NULL; /* old kvpair is free */

			}
		}
		bucketptr++;
	}
	
	if(themap->start!=NULL)
		free((void*)themap->start); /* free the buckets in the old map */
	themap->start = NULL;
 
   if(themap!=NULL) 
      free((void*)themap);    /* old map is now free */ 
   themap = NULL; 

   *incmap = pnewmap;         /* set the dereferenced map pointer passed in to the new map */

#ifdef DEBUG
   printf("DEBUG: leave sp_map_rehash - new map size: %lu, bucketsused: %lu, currentload: %f, loadfactor: %f bucketsize: %lu\n",(*incmap)->size,(*incmap)->bucketsused,(*incmap)->currentload,(*incmap)->load_factor,(*incmap)->bucketsize);
   printf("*******************************************************************************************************************\n");
#endif

   return SUCCESS;
}

/*
*
* sp_map_keys - construct an array of keys, and return a status code
*
*/

int sp_map_keys(map *themap, void ***inkeys) { 

	bucket *bucketptr=NULL;
   kvpair *curr=NULL;
   void **pkeys = NULL;
   int i=0;

#ifdef DEBUG
   printf("DEBUG: entering keys\n");
	printf("DEBUG: memory allocated: %lu\n",sizeof(char*) * ((themap->size) + 1));
#endif

	pkeys = (void**)malloc(sizeof(char*) * ((themap->size)+1));
   if(pkeys == NULL) { /* malloc failed */
	   perror("key list malloc failed\n");
      return FAILURE;
   }

   bucketptr = themap->start; /* advance pointer to proper bucket */

	for(i=0;i<themap->bucketsize;i++) {
		if((bucketptr->start) == NULL) {
			bucketptr++;
			continue; 	/* no kvpairs at this hashcode */

#ifdef DEBUG
			printf("DEBUG: -keys- ** no kvpairs at this hashcode, continue **\n");
#endif

		}
		else {
			curr = bucketptr->start; /* set curr to the first kvpair */
			while(curr!=NULL) { /* now iterate through all kvpairs at this hashcode */
				*pkeys = (char*)malloc(sizeof(char) * (strlen(curr->key)+1));
				if((*pkeys)!=NULL) {
					strcpy((*pkeys),curr->key);	
					pkeys++;	
				}
				curr = curr->next;
			}
		}
		bucketptr++;
	}


	*pkeys = NULL; /* set last to NULL */
  
   *inkeys = pkeys -= themap->size; 

   return SUCCESS;

}


/*
*
* nextprobableprime - returns the next prime 
* todo: implement this 
* fast prime lookup for rehash.
*
*/

unsigned long nextprobableprime(unsigned long num) {
	return num + 1;
}


kvpair* si_createkvpair(char *k, long v) {

   kvpair* newpair = NULL;
   long *plong;
   newpair = (kvpair*)malloc(sizeof(kvpair));

   if(newpair == NULL) { /* create a new kvpair */
      perror("malloc of kvpair failed");
      return FAILURE; 
   }
   else { /* malloc of kvpair ok */
      newpair->key = (void*)malloc(sizeof(char)*strlen(k) + 1);
      if(newpair->key == NULL) {
         perror("malloc of new key failed\n"); 
         return FAILURE;
      }
      plong = (long*)malloc(1 * sizeof(long));
      if(plong == NULL) {
         perror("malloc of new value failed\n"); 
         return FAILURE;
      }
      strcpy((char*)newpair->key,k); 
      *plong = v;
      newpair->value = (void*)plong; 
   }
   newpair->next = NULL; /* next always null when kvpair created */
   return newpair;
}

/*
*
* si_kv_put - put a chptr,long kv pair into the map
*
* if the key already exists, overwrite the existing value
* if the load is exceeded, resize the buckets and rehash
* return the value inserted.
*
* return NULL in the case of a failure (mem alloc, etc.)
*
*/

int si_kv_put(map **inmap, void *inkey, void *invalue) {

   unsigned long code;
	bucket *bucketptr;
   
   map *themap = *inmap; 
   char* key = (char*)inkey;
   long value = *((long*)invalue);

#ifdef DEBUG
				printf("DEBUG: enter si_kv_put with arguments key: %s value: %ld - hashcode: %d\n",key,value,(int)sp_k_hash(themap,key));
				printf("DEBUG: enter si_kv_put map size: %d, bucketsused: %d, currentload: %f, loadfactor: %f\n",(int)themap->size,(int)themap->bucketsused,themap->currentload,themap->load_factor);
#endif

	bucketptr = themap->start; /* point to first bucket in map */
	code = sp_k_hash(themap, key);		/* use wrapped hash function */
	bucketptr += code; 		/* point to bucket corresponding to value */
	if(bucketptr->start == NULL) { /* no value at this hash */
		bucketptr->start = si_createkvpair(key,value);
		if(bucketptr->start == NULL) {
			perror("new kvpair allocation failed");
			return FAILURE;
		}
		else { /* we have a kvpair, update map state */
			themap->size++; /* increase the kvpair count */
			themap->bucketsused++; /* an empty bucket now used */
			themap->currentload = (double)(themap->bucketsused) / (double)(themap->bucketsize);
			if(themap->currentload > themap->load_factor) {
				map_rehash(inmap);  /* buckets too full, rehash */

#ifdef DEBUG
	printf("DEBUG: si_kv_put - just rehashed: map size: %lu, bucketsize: %lu bucketsused: %lu, currentload: %f, loadfactor: %f\n",(*inmap)->size,(*inmap)->bucketsize,(*inmap)->bucketsused,(*inmap)->currentload,(*inmap)->load_factor);
#endif

			}
		}
	}
	else { /* we have a collision to deal with at this hash */
		kvpair *entry,*preventry;
		entry = bucketptr->start;
		while(entry != NULL) { /* cycle through kvpairs looking for match */ 
			if(strcmp(entry->key, key) == 0) { /* we have a match */

#ifdef DEBUG
				printf("DEBUG: sp_kv_put - collision - inserting new value\n");
#endif
				
            (*((long*)entry->value)) = value;
				return SUCCESS; /* return map */
			}
			preventry = entry; /* set the previous entry */
			entry = entry->next; /* advance to next kvpair in list */
		}
		entry = si_createkvpair(key,value); /* at the end of kvpairs, no match, create one */
		if(entry == NULL) {
			perror("allocate of kvpair at end of list failed");
			return FAILURE;
		}
		else { /* new kvpair added at end of list, update map state */
			themap->size++; /* increase the kvpair count */
			preventry->next = entry; /* update the pointer to extend the chain */
		}	
	}

#ifdef DEBUG
	printf("DEBUG: leaving sp_kv_put - map size: %lu, bucketsused: %lu, currentload: %f, loadfactor: %f bucketsize: %lu\n",(*inmap)->size,(*inmap)->bucketsused,(*inmap)->currentload,(*inmap)->load_factor,(*inmap)->bucketsize);
#endif

	return SUCCESS;
}


/*
*
*	si_kv_get - given a key, get the (long) value
*
*/

int si_kv_get(map *themap,void *key,void **value) {

	unsigned long code = 0;
	bucket* buck = NULL;
	kvpair *kvcurr = NULL;
   long *plong = NULL;

	code = sp_k_hash(themap, key); /* hash the key */

#ifdef DEBUG
	      printf("DEBUG: enter si_kv_get - key %s hashes to %lu\n",(char*)key,code);
#endif

   buck = themap->start + code; /* go to the appropriate bucket */	
	kvcurr = buck->start;	

	while(kvcurr != NULL) {

#ifdef DEBUG
	   printf("DEBUG: si_kv_get - kvcurr key %s\n",(char*)kvcurr->key);
#endif

      if(strcmp(((char*)kvcurr->key),(char*)key) == 0) { /* we have a match */
          
#ifdef DEBUG
	      printf("DEBUG: si_kv_get - matched key %s value is %ld\n",(char*)key,*((long*)kvcurr->value));
#endif

         /* value returned is not the data in the map, deep copied */
		   plong = (void*)malloc(sizeof(long));
         if(plong != NULL) {
            *plong = *((long*)kvcurr->value);
            *value = (void*)plong;  
         }
          
#ifdef DEBUG
	      printf("DEBUG: si_kv_get - get match for %s returns %s\n",(char*)key,(char*)(*value));
#endif

         return SUCCESS; 


		}
		kvcurr = kvcurr->next;
	}

   *value = NULL;    /* if not found, be sure to set value pointer to NULL */     
	return NOTFOUND;  /* not found */

}
