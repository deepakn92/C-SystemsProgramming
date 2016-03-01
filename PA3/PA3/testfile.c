//
//  testfile.c
//  PA3
//
//  Created by Deepak Nalla on 10/26/15.
//  Copyright (c) 2015 Deepak Nalla. All rights reserved.
//

#include "malloc.c"

/****************************************************
 * This test will intentionally create a scenario   *
 * where I try to catch errors in malloc and free	*
 ****************************************************/

int main(void){
    
    int bigInt = 2100000;
    /* should throw error since bigInt was never allocated */
    printf("####################################\n");
    printf("Tring to free an int that was never dynamically allocated\n");
    printf("####################################\n");
    free(&bigInt);
    
    int * testInt = (int *)malloc(10);
    /* should throw errors because these address were never returned by malloc */
    printf("####################################\n");
    printf("Tring to free a dynamically allocated int pointer ");
    printf("at an address not returned by malloc\n");
    printf("####################################\n");
    free(testInt + 1);
    free(testInt + 100);
    free(testInt - 100);
    
    /* free correctly */
    free(testInt);
    
    /* should throw error since it's redundant freeing */
    printf("#################\n");
    printf("Redundant freeing\n");
    printf("#################\n");
    free(testInt);
    
    char * pointer = "why is this due at 9 a.m.???";
    /* should throw error because pointer was never allocated */
    printf("#################################################################\n");
    printf("Tring to free a char pointer that was never dynamically allocated\n");
    printf("#################################################################\n");
    free(pointer);
    

    
    /* should throw error since our "heap" is only a 5000 char array */
    printf("######################################################\n");
    printf("Attempting to allocate space greater than maximum size\n");
    printf("######################################################\n");
    char * tooBig = (char *)malloc(5001);
    free(tooBig);

    /* shouldn't work */
    printf("###################################################\n");
    printf("Tring to malloc after heap is at maximum capacity\n");
    printf("###################################################\n");
    char * stuff = (char *)malloc(1);
    
    free(stuff);
    
    return 0;
}