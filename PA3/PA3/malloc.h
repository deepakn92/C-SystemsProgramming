//
//  Created by Deepak Nalla on 10/20/15.
//  Assignment 3: malloc() and free()
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* rewrite with custom malloc and free */
#define malloc( x )  mymalloc( x, __FILE__, __LINE__ )
#define free( x )  myfree( x, __FILE__, __LINE__ )


/* pre defined size macros */
#define HSIZE  sizeof(struct memEntry)
#define MEMSIZE  5000
#define BIGBLOCKSIZE  350
/* macros I made to convert into right size */

#define NEXT( x )  (mementryPtr) (DATA(x) + (int)x->size)
#define META( x )  ((char*)x - HSIZE)
#define DATA( x )  ((char*)x + HSIZE)

/* pattern is start date of project */
#define RECOGNITION  10212015

/*
 * malloc function, takes in size to allocate and the file and size macros
 * allocated right to left if it is a big block (as defined by the BIGBLOCKSIZE macro)
 * left to right if it is a small block
 * returns pointer to allocated memory on success, (void*) 0 on error
 */

void * mymalloc( unsigned int, char*, unsigned int );


/*
 * free function, takes in the pointer to the data to free
 * checks for redundant freeing, freeing an invalid piece of data, and freeing before allocating
 * also merges adjacent free blocks
 * returns 1 on success, 0 on error
 */

int myfree( void *, char *, unsigned int );


/*
 * Leak checking function declaration
 */

void isLeak( );

