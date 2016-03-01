#include <stdio.h>
#include "sorted-list.h"

int compare( void* x, void *y){
	if (*(int*)x < *(int*)y){
		return -1;
	}else if (*(int*)x == *(int*)y){
		return 0;
	}else{
		return 1;
	}
}

void destroy( void* a){
	free(a);
}

/*
test 1:
1. create list, check it is empty
2. use 1, create list iterate, check empty
3. use 2, check SLGetItem
4. use 3, check SLNextItem
5. destroy list iterator
6. destroy list
*/
void test1(){
	SortedListPtr list = SLCreate(compare, destroy);
	SortedListIteratorPtr iter;
	int* value;	

	if (list == NULL)
	{
		printf("Cannot created list\n");
		exit(0);
	}
	iter = SLCreateIterator(list);
	value = (int*)SLGetItem(iter);
	if (value != NULL)
	{
		printf("Value must be null\n");
		exit(0);
	}
	value = (int*)SLNextItem(iter);
	if (value != NULL)
	{
		printf("Value must be null\n");
		exit(0);
	}
	SLDestroy(list);
	SLDestroyIterator(iter);

	printf("Test 1 completed\n");
}

/*
test 2:
1. create list, null compare
2. use 1, check null list
*/
void test2(){
	SortedListPtr list = SLCreate(NULL, destroy);
	if (list != NULL)
	{
		printf("List must be NULL due to invalid argument\n");
		exit(0);
	}

	printf("Test 2 completed\n");
}

/*
test 3:
1. create list, null destroy
2. use 1, check null list
*/
void test3(){
	SortedListPtr list = SLCreate(compare, NULL);
	if (list != NULL)
	{
		printf("List must be NULL due to invalid argument\n");
		exit(0);
	}

	printf("Test 3 completed\n");
}


/*
test 4:
1. create list with compare and destroy functions
2. check SLInsert, add 1, 2, 3, 4, 5
3. create list iterate
4. check SLGetItem
5. destroy list iterator
6. destroy list
*/
void test4(){
	SortedListPtr list = SLCreate(compare, destroy);
	SortedListIteratorPtr iter;
	int i;
	int* value;

	for (i = 1; i <= 5; i++)
	{
		value = (int*)malloc(sizeof(int));	
		*value = i;
		SLInsert(list, (void*)value);
	}

	iter = SLCreateIterator(list);

	value = (int*)SLGetItem(iter);
	printf("Value = %d\n", *value);

	SLDestroy(list);
	SLDestroyIterator(iter);

	printf("Test 4 completed\n");
}


/*
test 5:
1. create list with compare and destroy functions
2. check SLInsert, add 1, 2, 3, 4, 5
3. create list iterate
4. check SLGetItem, SLNextItem
5. destroy list iterator
6. destroy list
*/
void test5(){
	SortedListPtr list = SLCreate(compare, destroy);
	SortedListIteratorPtr iter;
	int i;
	int* value;

	for (i = 1; i <= 5; i++)
	{
		value = (int*)malloc(sizeof(int));	
		*value = i;
		SLInsert(list, (void*)value);
	}

	iter = SLCreateIterator(list);
	
	value = (int*)SLGetItem(iter);
	while (value != NULL)
	{
		printf("Value = %d\n", *value);		
		value = (int*)SLNextItem(iter);		
	}	


	SLDestroy(list);
	SLDestroyIterator(iter);

	printf("Test 5 completed\n");
}

/*
test 6:
1. Do the test 5 with SLNextItem over list
*/
void test6(){
	SortedListPtr list = SLCreate(compare, destroy);
	SortedListIteratorPtr iter;
	int i;
	int* value;

	for (i = 1; i <= 5; i++)
	{
		value = (int*)malloc(sizeof(int));	
		*value = i;
		SLInsert(list, (void*)value);
	}

	iter = SLCreateIterator(list);
	
	value = (int*)SLGetItem(iter);
	while (value != NULL)
	{
		printf("Value = %d\n", *value);		
		value = (int*)SLNextItem(iter);		
	}	

	value = (int*)SLNextItem(iter);		
	if (value != NULL)
	{
		printf("Over end of list, must be null\n");
		exit(0);
	}

	SLDestroy(list);
	SLDestroyIterator(iter);

	printf("Test 6 completed\n");
}


/*
test 7:
1. create list with compare and destroy functions
2. check SLInsert, add 1, 2, 3, 4, 5, SLRemove
3. create list iterate
4. check SLGetItem, SLNextItem
5. destroy list iterator
6. destroy list
*/
void test7(){
	SortedListPtr list = SLCreate(compare, destroy);
	SortedListIteratorPtr iter;
	int i;
	int* value;

	for (i = 1; i <= 5; i++)
	{
		value = (int*)malloc(sizeof(int));	
		*value = i;
		SLInsert(list, (void*)value);
	}

	iter = SLCreateIterator(list);
	i = SLRemove(list, value);
	printf("SLRemove 5 returned %d\n", i);

	i = SLRemove(list, value);
	printf("SLRemove 5 returned %d\n", i);

	value = (int*)SLGetItem(iter);
	while (value != NULL)
	{
		printf("Value = %d\n", *value);		
		value = (int*)SLNextItem(iter);		
	}	


	SLDestroy(list);
	SLDestroyIterator(iter);

	printf("Test 7 completed\n");
}

int main(){

	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();

	return 0;
}