#include <stdlib.h>
#include <stdio.h>
#include "map.h"
#include "mapconf.h"

void testsimple(void);
void testsimplelong(void);

void testnotsosimple(void);
void testnotsosimplelong(void);

int main(void) {
	
   testsimple();
   testnotsosimple();

   testsimplelong();
   testnotsosimplelong();

   return 0;

}

/* test some simple operations */

void testsimple(void) {

	map *testmap = NULL; 
	char *getresult = NULL;
	status code1, code2, code3, code4;

	printf("\n\nTesting map_init, kv_put, kv_get, kv_remove...\n\n");
	
	code1 = map_init(&testmap, CHPTR_CHPTR); 
   
   /* map is potentially mutable in kv_put, so must pass a reference */
   code2 = kv_put(&testmap,"fred","flinstone");
   
   /* map is not mutable in kv_put so pass ptr, cast ptr on new value void to avoid compiler warnings */
   /* or can declare getresult as void* and cast when dereferenced after this call */ 
   code3 = kv_get(testmap,"fred", (void**)&getresult); 
   if(getresult != NULL) {	 /* test and free when done, mem is allocated for this in kv_get */
      printf("get fred = %s\n", getresult);
      free(getresult);
   }
   getresult = NULL;

   code2 = kv_put(&testmap,"fred","flanders");
   code3 = kv_get(testmap,"fred", (void**)&getresult);
   if(getresult != NULL) {
	   printf("get fred = %s\n", getresult); 
      free(getresult);
   }
   getresult = NULL;

   /* map not mutable in kv_remove, pass ordinary ptr */   
	kv_remove(testmap,"fred");
	
   code3= kv_get(testmap,"fred", (void**)&getresult);
   if(getresult == NULL) {	
      printf("after remove: get fred = %s\n", ((getresult!=NULL)?getresult:"(null)")); 
   }
   else free(getresult);
   getresult = NULL;

   /* pass ptr in map_destroy */
	code4 = map_destroy(testmap);

}

void testsimplelong(void) {
   
   map *testmap = NULL;
   long *getresult, putarg = 35;
   status code1,code2,code3,code4;
   
   printf("\n\nTesting chptr-long map simple map functions...\n\n");

   code1 = map_init(&testmap, CHPTR_LONG);
   
   code2 = kv_put(&testmap, "fred", &putarg); 
   code3 = kv_get(testmap, "fred", (void**)&getresult); 
   if(getresult!=NULL)  {
      printf("get fred = %ld\n",*getresult);
      free(getresult);     /* important, memory is allocated for what the pointer points to */
   }
   getresult = NULL; 

   putarg = 53;

   code2 = kv_put(&testmap, "fred", &putarg); 
   code3 = kv_get(testmap, "fred", (void**)&getresult); 
   if(getresult!=NULL) {
      printf("get fred = %ld\n",*getresult);
      free(getresult);     /* important, memory is allocated for what the pointer points to */
   }
   getresult = NULL; 

   kv_remove(testmap,"fred");
   code3 = kv_get(testmap, "fred", (void**)&getresult); 
   if(getresult==NULL) { 
      printf("get fred after remove = (null)\n");
   }
   else {
      printf("remove failed!\n");
      free(getresult);
   }
   
   getresult = NULL;

   code4 = map_destroy(testmap);
   
}

void testnotsosimple(void) {

	map *testmap;
	int i,j,k,t;
   int code1,code2,code3;
	char *key=NULL,*value=NULL,**thekeys=NULL; 

   printf("\n\ntesting keys...\n\n");

	code1 = map_init(&testmap, CHPTR_CHPTR);

   for(i=0;i<25;i++) {
	   t = (random() % 25) + 1; 
		key = (char*)malloc(sizeof(char) * t + 1); 
		value = (char*)malloc(sizeof(char) * t + 1); 
      for(k=0;k<t;k++) {
	      j = random() % 26;
         *(key+k) = *(value+k) = 97 + j; 
      }
		*(key+t) = *(value+t) = 0x00; 
		
      printf("%d - inserting key: %s, value: %s\n",i,key,value);
		code2 = kv_put(&testmap,key,value);
      
      if(key!=NULL)   
         free((void*)key);
      if(value!=NULL) 
         free((void*)value);
      key = value = NULL;
	}

	map_keys(testmap,(void***)&thekeys);
	
	for(i=0;i<testmap->size;i++) {
		printf("key %d: %s\n",i,*(thekeys+i));	
	}

	if(thekeys!=NULL) /* must not forget to free keys, must be done by the caller of map_keys */
		free(thekeys);
   	
	code3 = map_destroy(testmap);

}

void testnotsosimplelong(void) {

	map *testmap;
	int i,j,k,t;
   int code1,code2,code3;
	char *key=NULL,**thekeys=NULL; 
   long value;

   printf("\n\ntesting keys...\n\n");

	code1 = map_init(&testmap, CHPTR_LONG);

   for(i=0;i<25;i++) {
	   t = (random() % 25) + 1; 
		key = (char*)malloc(sizeof(char) * t + 1); 
      for(k=0;k<t;k++) {
	      j = random() % 26;
         *(key+k) =  97 + j; 
      }
      value = (long)j;                 /* use the last random value */
		*(key+t) = 0x00; 
		
      printf("%d - inserting key: %s, value: %ld\n",i,key,value);
		code2 = kv_put(&testmap,key,&value);
      if(key!=NULL) 
         free((void*)key);
      key = NULL;
	}

	map_keys(testmap,(void***)&thekeys);
	
	for(i=0;i<testmap->size;i++) {
		printf("key %d: %s\n",i,*(thekeys+i));	
	}

	if(thekeys!=NULL) /* must not forget to free keys, must be done by the caller of map_keys */
		free(thekeys);
   	
	code3 = map_destroy(testmap);

}
