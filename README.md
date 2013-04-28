A simple hashmap implementation in c, using separate chaining for collisions.  This is a rewrite of my previous chptr-chptr hashmap implementation, but map has been generalized to be more extensible.  Currently there is a chptr-chptr key-value type implemented, adding another type is straightforward, see below.  

Joseph Burns - 3/2007

###files###

Makefile - a sample makefile for building the map functions  
map.c - the map implementation: calls both general and wrapper functions  
map.h - header file for general map functions and function wrappers  
mapconf.h - header file for type-specific function structure  
mapconf.c - function table that points to type-specific functions  
mapfn.h - header file for type-specific functions  
mapfn.c - implementation of type-specific functions  
testmap.c - a test suite that hits key functionality  

###compilation###
- make map.o, or make testmap
- uncomment /* #define DEBUG */ in map.h to see the gory details of each map operation

###usage###
- see testmap.c for examples
- be sure to map_destroy after done, and if map_keys is used be sure to free the type**

###notes###

- be sure to check for NULL on Solaris before using results, it isn't as forgiving

###adding a user type map###

1. implement type-spefic map functions in mapfn.c (header for them - mapfn.h)
```
    unsigned long (*fn_hash)(map*,void*);        /* hash function for this map's key type */  
    int (*fn_put)(map*,void*,void*);             /* put function */
    int (*fn_get)(map*,void*,void*);             /* get function */
    int (*fn_remove)(map*,void*);                /* remove function */
    int (*fn_destroy)(map*);                     /* map destroy function */
    int (*fn_rehash)(map*);                      /* rehash map function */
    int (*fn_keys)(map*,void**);                 /* given map and double ptr */
```
2. add the functions as function pointer table in mapconf.c (see mapconf.h for the structure)

3. use, as done in testmap.c 

###todo###
- thread safety

###license###
[MIT license](http://opensource.org/licenses/MIT)

