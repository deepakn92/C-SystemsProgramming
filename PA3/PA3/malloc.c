
/*
 * malloc.c
 * PA3
 * Deepak Nalla
 * Professor Brian Russell
 * CS214, Section3
 */

#include "malloc.h"

typedef struct memEntry * mementryPtr;
struct memEntry {
    mementryPtr prev;
    int isFree;
    unsigned int recognitionPtrn;
    unsigned int size;
};

static char memblock[MEMSIZE];          //allocate 5000 char array
long unsigned int blocksCount = 0;
long unsigned int totalSpace = 0;
static mementryPtr blockFront;          //create pointers to the front and the back
static mementryPtr blockEnd;

/* leak check function - checks for memory leaks */

void isLeak() {
    if( blocksCount >0) {
        printf("Error: memory leak because you did not free all allocated blocks!");
        printf("%lu bytes in %lu block\n", totalSpace, blocksCount);
    }
    return ;}

/*
 * malloc function that intializes a list with a fixed size character array, and allocates and manage
 * memory with in it, returns a pointer to the address in memory of enough size requested  
 */

void * mymalloc(unsigned int size, char* file, unsigned int line) {
    if ((size + HSIZE) > (MEMSIZE - (totalSpace + HSIZE)) && (size + HSIZE ) != ( MEMSIZE - totalSpace)) {
        
        printf("error: There is not enough space to allocate for your needs\n");
        printf("malloc on line %d in file: \"%s\"\n", line, file);
        
        return (void *)0;
    }
    
    if ( size <= 0 ) {
        printf("error: cannot malloc space of size %d\n" , size);
        printf("malloc on line %d in file: \"%s\"\n" , line, file);
        return (void *)0;
    }
    static int initialized = 0;
    
    mementryPtr current,  newEntry;
    
    if( !initialized ) {                      // intialize the block of memory if its not already

        blockFront = (mementryPtr) memblock;
        blockEnd = blockFront;                  // At starting point only 1 block exists
        blockFront -> prev = 0;
        blockFront -> size = MEMSIZE - HSIZE;
        blockFront -> isFree = 1;
        blockFront -> recognitionPtrn = RECOGNITION;
        initialized = 1;
        atexit(isLeak);
        
    }
    
    if( size < BIGBLOCKSIZE) { /*small block - we allocate left to right */
        
        //ok, now continue from the front of array
        current = blockFront;
        mementryPtr prec = 0;
        do {
            
            mementryPtr succ = NEXT(current);
            if( current -> size < size) {
                prec = current;
                current = succ;
                
            }
            else if (!current->isFree){
                prec = current;
                current = succ;
                
            }
            /* found the exact size, we're good! */
            else if (current -> size == size) {
                current -> isFree = 0;          // currently occupied block
                current -> recognitionPtrn = RECOGNITION;
                
                /* if last block make block last */
                if( (char* )succ >= (memblock + MEMSIZE))
                {
                    blockEnd = current;
                }
                //keeping track
                blocksCount ++;
                totalSpace += current -> size + HSIZE;
                
                return( void*) (DATA(current));
            }
            /*found big enough block, so now we chop it up*/
            else {
                newEntry = (mementryPtr)(DATA(current) + size);
                newEntry -> prev = current;
                newEntry -> size = current -> size - HSIZE - size;
                newEntry -> isFree = 1;
                newEntry -> recognitionPtrn = RECOGNITION;
                
                //Ok so we set the next's previous to the newBlock
                ((mementryPtr) NEXT(current)) ->prev = newEntry;
                
                current ->size = size;
                current -> isFree = 0;
                
                /* * this is done to help the situation when users inadvertantly write past the space they allocated
                 * which effectively means they write over the next blocks' meta-data
                 */
                current -> prev = prec;
                current -> recognitionPtrn = RECOGNITION;
                
                if ( (char*) succ < (memblock + MEMSIZE ) ) {
                    succ->prev = newEntry;
                } else {
                    blockEnd = newEntry;
                }
                blocksCount++;
                totalSpace += current -> size + HSIZE;
                
                return (void*)(DATA(current))   ;
                
            }
            
        } while ((char*)current < (memblock +MEMSIZE ));
    }
    else { /*big block -we allocate right to left */
        current = blockEnd;
        mementryPtr succ = NEXT(current);
        do {
            if ( current -> size <size) {
                succ = current;
                current = current -> prev;
            }
            else if ( !current -> isFree) {
                succ = current;
                current = current -> prev;
            }
            else if (current -> size == size)
            {
                current -> isFree = 0;
                current ->recognitionPtrn = RECOGNITION; //*helps check if this block was created by you */
                
                /* if last block, make back block*/
                if( (char*) succ >= (memblock +MEMSIZE) ) {
                    blockEnd = current;
                    
                }
                blocksCount++;
                totalSpace += current -> size +HSIZE;
                return (void *) DATA(current);
                
            }
            else {
                newEntry = (mementryPtr)((char *) current +current -> size - size);
                newEntry -> prev = current;
                newEntry -> isFree= 0;
                newEntry -> size = size;
                newEntry -> recognitionPtrn = RECOGNITION;
                
                //set next's previous to the newBlock
                ((mementryPtr) NEXT(current)) -> prev = newEntry;
                current -> size = current ->size - HSIZE - size;
                current -> isFree = 1;
                current -> recognitionPtrn = RECOGNITION;
                
                if( ( char*)succ < (memblock + MEMSIZE )){
                    succ -> prev = newEntry;
                } else {
                    blockEnd = newEntry;
                }
                blocksCount++;
                totalSpace += newEntry -> size + HSIZE;
                
                return (void*) DATA(newEntry);
                
            }
        } while ((char*)current >= memblock);
        
    }
    printf("error: cannot malloc\n");
    printf("malloc is run on line %d in file: \"%s\"\n", line, file);
    
    return (void*) 0;
}


/* free function
 * takes in pointer of the data to be freed, returns a 1 if successfully freed, and 0 if not, or on error
 */

int myfree(void * fdata, char* file, unsigned int line) {
    if( fdata == 0) {
        printf("ERROR:cannot free null pointer\n");
        printf("Free on line: %d in file: %s\n", line , file);
        return 0;
    }
    if  (( char*) fdata >= (memblock + MEMSIZE) || META(fdata < memblock)) {
        printf("Free on line: %d in file: %s\n", line, file);
        return 0;
    }
         
          mementryPtr current = (mementryPtr) META(fdata);
         
         //check recognition pattern to confirm authenticity of malloced blocks by you
         
         if( current -> recognitionPtrn != RECOGNITION)
         {
             printf("ERROR: cannot free pointer that was not malloced by you! \n");
             printf("Free on line: %d in file: %s\n", line, file);
             return 0;
         }
         
         if (current -> isFree) {
             printf("ERROR: cannot free already freed block\n");
             printf("Free on line: %d in file: %s \n", line, file);
             return 0;
         }
         blocksCount --;
         totalSpace -= (current-> size + HSIZE);
         current -> isFree = 1;
         
         /* We check left/right for free blocks to merge together */
    
        mementryPtr succ = (mementryPtr) NEXT(current);
    
        if (succ <= blockEnd) {
             if ((succ-> isFree)) {
                 mementryPtr succNext = (mementryPtr) NEXT(succ);
                 if( succNext <= blockEnd) {
                     succNext -> prev = current;
                 }
                 current ->size += ((succ->size) + HSIZE);
                 if ( succ == blockEnd)
                 {
                     blockEnd = current;
                 }
             }
         }
        else { //if succ doesn't exist, then current will be last block
             blockEnd = current;
        }
    
        if ( current != blockEnd && (current->prev)-> isFree) //merge with left if free {
         {   if(current == blockEnd) {
             blockEnd = current->prev;
             
         }
        if (succ <= blockEnd) {
             succ -> prev = current -> prev;
         }
         (current->prev)->size += (current -> size +HSIZE);
         
         }
        return 1;
    }
