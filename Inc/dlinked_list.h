#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#ifdef __STDOUT
#include <iostream>
#endif

//The only requirement for T is that it must have next and prev variables
template<typename T>
void insertActive(T *n, T* head)
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
}

template<typename T>
void setActive(T* n, T* head)
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

	//Don't add if already active
	if( n->status != true)
	{
		n->setStatus(true);

		n->prev->next = n->next;
		n->next->prev = n->prev;

		head->next->prev = n;

		n->next = head->next;
		head->next = n;
		n->prev = head;
	}


}

template<typename T>
void setWait(T* n, T* head)
{
	if( n->status != false)
	{
		n->setStatus(false);

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



#endif //DATASTRUCTURES_H