#include "sorted-list.h"

/*
 * SLCreate creates a new, empty sorted list.  The caller must provide
 * a comparator function that can be used to order objects that will be
 * kept in the list, and a destruct function that gets rid of the objects
 * once they are no longer in the list or referred to in an iterator.
 * 
 * If the function succeeds, it returns a (non-NULL) SortedListT object,
 * otherwise, it returns NULL.
 *
 */
SortedListPtr SLCreate(CompareFuncT cf, DestructFuncT df){
	/*allocate memory*/
	SortedListPtr listPtr = (SortedListPtr)malloc(sizeof(struct SortedList));
	if (listPtr == NULL)
	{
		return NULL;
	}
	if (cf == NULL || df == NULL)
	{
		return NULL;
	}
	/*set head to NULL*/
	listPtr->head = NULL;
	/*set functions*/
	listPtr->compareFunc = cf;
	listPtr->destroyFunc = df;
	return listPtr;
}

/*
 * SLDestroy destroys a list, freeing all dynamically allocated memory.
 * precondtion: list was initilized
 */
void SLDestroy(SortedListPtr list){

	/*pointer to iterate the list*/
	struct SortedListNode* current;

	struct SortedListNode* temp;
	
	/*set to head of list*/
	current = list->head;

	/*iterate until end of list then null*/
	while (current != NULL)
	{
		temp = current;
		/*move to next*/
		current = current->next;

		/*deallocte node data*/
		list->destroyFunc(temp->data);

		/*deallocate node*/
		free (temp);
	}
	/*free list*/
	free (list);
}

/*
 * SLInsert inserts a given object into a sorted list, maintaining sorted
 * order of all objects in the list.  If the new object is equal to an object
 * already in the list (according to the comparator function), then the operation should fail.
 *
 * If the function succeeds, it returns 1, otherwise it returns 0.
 *
 * precondtion: list was initilized
 */
int SLInsert(SortedListPtr list, void *newObj){
	
	/*pointer to iterate the list*/
	struct SortedListNode* current;

	/*pointer to previous list of current*/
	struct SortedListNode* previous = NULL;

	/*new node*/
	struct SortedListNode* newNode;

	/*set to head of list*/
	current = list->head;

	/*move along the list*/
	while (current != NULL && list->compareFunc(current->data, newObj) < 0)
	{		
		previous = current;
		current = current->next;
	}

	/*check duplicate*/
	if (current != NULL && list->compareFunc(current->data, newObj) == 0) 
	{
		return 0;
	}

	newNode = (struct SortedListNode*)malloc(sizeof(struct SortedListNode));
	if (newNode == NULL)
	{
		return 0;
	}
	newNode->data = newObj;
	newNode->next = NULL;

	if (previous == NULL)
	{/*insert as head node*/
		newNode->next = list->head;
		list->head = newNode;
	}else{/*insert after previous and before current node*/
		previous->next = newNode;
		newNode->next = current;
	}
	/*return success*/
	return 1;
}

/*
 * SLRemove removes a given object from a sorted list.  Sorted ordering
 * should be maintained.  SLRemove may not change the object whose
 * pointer is passed as the second argument.  This allows you to pass
 * a pointer to a temp object equal to the object in the sorted list you
 * want to remove.
 *
 * If the function succeeds, it returns 1, otherwise it returns 0.
 *
 * precondtion: list was initilized
 */

int SLRemove(SortedListPtr list, void *newObj){

	/*pointer to iterate the list*/
	struct SortedListNode* current;

	/*pointer to previous list of current*/
	struct SortedListNode* previous = NULL;

	/*set to head of list*/
	current = list->head;

	/*move along the list*/
	while (current != NULL && list->compareFunc(current->data, newObj) != 0)
	{		
		previous = current;
		current = current->next;
	}

	/*check existing*/
	if (current != NULL) 
	{
		if (current == list->head)
		{/*free head node*/
			list->head = current->next;
		}else{
			previous->next = current->next;
		}

		/*deallocate node*/
		free (current);
		/*return success*/
		return 1;
	}

	/*return fail*/
	return 0;
}

/*
 * SLCreateIterator creates an iterator object that will allow the caller
 * to "walk" through the list from beginning to the end using SLNextItem.
 *
 * If the function succeeds, it returns a non-NULL pointer to a
 * SortedListIterT object, otherwise it returns NULL.  The SortedListT
 * object should point to the first item in the sorted list, if the sorted
 * list is not empty.  If the sorted list object is empty, then the iterator
 * should have a null pointer.
 *
 * precondtion: (SortedListPtr) list was initilized
 */

SortedListIteratorPtr SLCreateIterator(SortedListPtr list){
	/*allocate the memory*/
	SortedListIteratorPtr iter = (SortedListIteratorPtr)malloc(sizeof(struct SortedListIterator));
	/*check if memory is available*/
	if (iter == NULL)
	{
		return NULL;
	}
	/*set current to head of list*/
	iter->current = list->head;

	return iter;
}

/*
 * SLDestroyIterator destroys an iterator object that was created using
 * SLCreateIterator().  Note that this function should destroy the
 * iterator but should NOT affect the original list used to create
 * the iterator in any way.
 * precondtion: (SortedListIteratorPtr) list iterator was initilized
 */

void SLDestroyIterator(SortedListIteratorPtr iter){
	free (iter);
}

/*
 * SLGetItem returns the pointer to the data associated with the
 * SortedListIteratorPtr.  It should return 0 if the iterator
 * advances past the end of the sorted list.
 * precondtion: (SortedListIteratorPtr) list iterator was initilized
*/

void * SLGetItem( SortedListIteratorPtr iter ){
	if (iter->current == NULL)
	{
		return NULL;
	}
	return iter->current->data;
}

/*
 * SLNextItem returns the pointer to the data associated with the
 * next object in the list associated with the given iterator.
 * It should return a NULL when the end of the list has been reached.
 *
 * One complication you MUST consider/address is what happens if a
 * sorted list associated with an iterator is modified while that
 * iterator is active.  For example, what if an iterator is "pointing"
 * to some object in the list as the next one to be returned but that
 * object is removed from the list using SLRemove() before SLNextItem()
 * is called.
 * precondtion: (SortedListIteratorPtr) list iterator was initilized
 */

void * SLNextItem(SortedListIteratorPtr iter){
	void* data = NULL;
	
	if (iter->current != NULL)
	{
		data = iter->current->data;
		iter->current = iter->current->next;
	}
	
	return data;
}