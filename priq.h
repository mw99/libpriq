/**
 * Universal priority queue data structure.
 */

#ifndef _PRIQ_H_
#define _PRIQ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// TYPES

// contend pointer
typedef void* cp;

// Node structure (Can't be opaque because of macro based interface)
struct _Heap
{
	/** Node Contend */
	cp contend;
	/** The right heap  */
	struct _Heap* right;
	/** The left heap  */
	struct _Heap* left;
};

typedef struct _Heap Heap;

// Used for contend comparison, see priq_create
typedef int(*Pricmp)(cp c1, cp c2);

// Used for contend deletion, see priq_destroy
typedef void(*Freefunc)(cp c);

// Base structure (Can't be opaque because of macro based interface)
struct _Priq
{
	uint64_t size;
	Heap* top;
	Pricmp cmp;
};

// Just 'Priq' for the main data structure
typedef struct _Priq* Priq; 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// INTERFACE 

// -----------------------------------------------------------------------------
/**
 * Returns the queue size. 
 * Complexity always O(1)
 */
#define priq_size(q) (q->size)


// -----------------------------------------------------------------------------
/**
 * True if the queue is empty.
 * Complexity always O(1)
 */
#define priq_is_empty(q) (!priq_size(q))


// -----------------------------------------------------------------------------
/**
 * Creates a new priority queue. Needs a priority comparison function.
 *
 * The compare function should return an integer less than, equal to, 
 * or greater than zero if the priority of c1 is found, to be less than, 
 * to match, or be greater than c2.
 * (c1 > c2) =  1
 * (c1 < c2) = -1
 * (c1 = c2) =  0
 * Complexity always O(1)
 */
Priq priq_create(Pricmp cmp);


// -----------------------------------------------------------------------------
/**
 * Destroys a queue. All memory is released.
 * Freefunc will be used on every contend unless it is NULL;
 * Complexity always O(n)
 */
void priq_destroy(Priq q, Freefunc f);


// -----------------------------------------------------------------------------
/**
 * Returns the element with the lowest priority. But does not remove it.
 * NULL if the queue is empty.
 * Complexity always O(1)
 */
#define priq_peek(q) ((priq_is_empty(q)) ? NULL : q->top->contend)


// -----------------------------------------------------------------------------
/**
 * Enqueues an element into the queue.
 * Complexity O(log n)
 */
void priq_enqueue(Priq q, cp c);
 

// -----------------------------------------------------------------------------
/**
 * Dequeues an element from the queue. Return NULL if the queue is empty.
 * Complexity O(log n)
 */
cp priq_dequeue(Priq q);


// -----------------------------------------------------------------------------
/**
 * Merges two queues intp one. Don't use q1 or q2 after the call of 
 * this function. Only the returned queue is supposed to be touched again.
 * Returns NULL if the queues have different comparison functions.
 * Complexity O(log n)
 */
Priq priq_merge(Priq q1, Priq q2);


// -----------------------------------------------------------------------------
/**
 * Priority queue invariant check.
 * Complexity always O(2n)
 * @return If NULL -> Ok. Else an error msg.
 */
const char* priq_invariant(Priq q);


#ifdef __cplusplus
}
#endif

#endif
