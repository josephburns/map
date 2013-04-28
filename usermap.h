#ifndef USERMAP_H 
#define USERMAP_H

typedef enum maptypes {CHARCHAR};

typedef enum status {SUCCESS,FAILURE};
typedef enum comparison {EQUAL,NOTEQUAL};
typedef enum boolean {TRUE,FALSE};

typedef struct kvop {
	unsigned long (*hash)(void*);		/* given a key, hash it and return an unsigned long */
	int (*equals)(void*,void*);		/* given a key and equivalent type, compare the two and return a comparison */ 
	void* (*create)(void*,void*);		/* given a key and value, allocate the memory for the kvpair struct and return a pointer to it */
	int (*destroy)(void*);				/* given a key, deallocate the memory for the kvpair and return a status */
}

#endif
