#ifndef DLINKED_LIST_H
#define DLINKED_LIST_H

#ifdef __STDOUT
#include <iostream>
#endif


/**
 * @brief Insert a fresh node in the list, given the head node. 
 * @details The head node is defined by the user and will be used to navigate the list
 * 
 * @param n Pointer to the node to add
 * @param head Pointer to the head node
 * @param status Status of the node to be initialized at
 */
template<typename T, typename STATUS>
void vInsertInList(T *n, T* head, STATUS s)
{
	/*
	IDLE.next.prev = new (a,d)
	new.next = IDLE.next (d)
	IDLE.next = new (c,a)
	new.prev = IDLE (d)
	*/
	head->next->prev = n;
	n->next = head->next;
	head->next = n;
	n->prev = head;

	n->_state = s;
}

/**
 * @brief Update an existing node in the list.
 * @details Removing and adding all the necessary references to reroute the list is done here.
 * 			The destination list is the one that starts at param head. The status will be updated.
 * 
 * @param n Pointer to the node to update
 * @param head Pointer to the head of the list
 * @param s Status to be updated
 */
template<typename T, typename STATUS>
void vUpdateInList(T* n, T* head, STATUS s)
{

	/*
	this.prev
		this.prev.next = this.next (a,b as b is overwritten)
	this.next
		this.next.prev = this.prev (a,c as c is overwritten)
	IDLE 
		IDLE.next.prev = this (d, f as f is overwritten)
	this
		this.next = IDLE.next (d,c as c is overwritten)
		IDLE.next = this (e,f)
		this.prev = IDLE (e,b)
	*/

	//Don't add if already in such status
	if( n->_state != s)
	{
		n->_state = s;

		n->prev->next = n->next;
		n->next->prev = n->prev;

		head->next->prev = n;

		n->next = head->next;
		head->next = n;
		n->prev = head;
	}


}

#ifdef __STDOUT
template<typename T>
void printLists(T* head, char const* msg)
{

	auto start_active = head;

	auto curr_active = head;

	std::cout << msg << std::endl;
	do{
		curr_active->print();
		curr_active = curr_active->next;

	}while( curr_active != start_active );	
	
	std::cout << std::endl;
	std::cout << std::endl;

}
#endif



#endif //DLINKED_LIST_H