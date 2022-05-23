#include "deque.h"

#include <stdio.h>

t_deque *dequeInit(void)
{
	t_deque *deque;
	deque = malloc(sizeof(t_deque));
	spin_lock_init(&(deque->lock));
	deque->first = NULL;
	deque->last = NULL;
	deque->size = 0;
	return deque;
}

int isEmpty(t_deque *deque)
{	
	int ret=0;
	ret = !deque->first || !deque->last;
	return ret;
}

void pushFront(t_deque *deque,t_deque_node* node)
{	
	spin_lock(&(deque->lock));
	// t_deque_node *node = malloc(sizeof(t_deque_node));
	// node->content = content;
	node->prev = NULL;
	node->next = deque->first;
	if (isEmpty(deque))
		deque->last = node;
	else
		deque->first->prev = node;
	deque->first = node;
	deque->size++;
	spin_unlock(&(deque->lock));
}

void pushBack(t_deque *deque,t_deque_node* node)
{
	spin_lock(&(deque->lock));
	// t_deque_node *node = malloc(sizeof(t_deque_node));
	// node->content = content;
	node->prev = deque->last;
	node->next = NULL;
	if (isEmpty(deque))
		deque->first = node;
	else
		deque->last->next = node;
	deque->last = node;
	deque->size++;
	spin_unlock(&(deque->lock));
}

void *popFront(t_deque *deque)
{
	spin_lock(&(deque->lock));
	t_deque_node *node;
	// void *content;
	if (isEmpty(deque)){

		spin_unlock(&(deque->lock));
		return NULL;
	}
	node = deque->first;
	deque->first = node->next;
	if (!deque->first)
		deque->last = NULL;
	else
		deque->first->prev = NULL;
	// content = node->content;
	deque->size--;
	// free(node);
	spin_unlock(&(deque->lock));
	return node;

}

void *popBack(t_deque *deque)
{
	spin_lock(&(deque->lock));
	t_deque_node *node;
	// void *content;
	if (isEmpty(deque)){
		spin_unlock(&(deque->lock));
		return NULL;
	}
	node = deque->last;
	deque->last = node->prev;
	if (!deque->last)
		deque->first = NULL;
	else
		deque->last->next = NULL;
	// content = node->content;
	deque->size--;
	// free(node);
	spin_unlock(&(deque->lock));
	return node ;
}

void *peekFront(t_deque *deque)
{
	if(isEmpty(deque))
		return NULL;
	return deque->first;
}

void *peekBack(t_deque *deque)
{
	if(isEmpty(deque))
		return NULL;
	return deque->last;
}

int get_queue_size(t_deque *deque)
{
	return(deque->size);
}
