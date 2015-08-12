/**
 * Universal priority queue data structure.
 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// HEADER

#include "priq.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// DEBUGGING
#if defined(INVARIANT_CHECKS)
	#include <stdio.h>
	bool priq_check_invariant(Priq r);
#endif

#ifdef INVARIANT_CHECKS
	#define perr(format, ...)  fprintf(stderr, "ERROR " format "\n", ## __VA_ARGS__)
	#define EXIT_FAILURE_ASSERT 110
	#define ASSERT(x, ...) \
		if (!(x)) \
		{\
			perr("ASSERT FAILED: " #__VA_ARGS__ " (" #x ")   File: " \
			__FILE__ "   Line: %d", __LINE__); \
			exit(EXIT_FAILURE_ASSERT); \
		}
#else
	#define ASSERT(x, ...)
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS INTERN

static inline Heap* _priq_create_heap(cp c);
static Heap* _priq_heap_merge(Heap* h1, Heap* h2, Pricmp cmp);
static bool _priq_heap_inv(Heap* h, Pricmp cmp);
static uint64_t _priq_count_contend(Heap* h);
static void _priq_heap_destroy(Heap* h, Freefunc ff);

// -----------------------------------------------------------------------------

#define _priq_is_empty_heap(h) ((h)==NULL)
#define _priq_heap_contend(h) (h->contend)


// -----------------------------------------------------------------------------
/**
 * Safe malloc. TODO implement user defind allocation function. 
 */
static inline void* _smalloc(uint64_t s)
{
	void* res = malloc(s);
	if (!res)
		abort();
	return res;
}
// -----------------------------------------------------------------------------
/**
 * Creates a new heap. 
 */
static inline Heap* _priq_create_heap(cp c)
{
	Heap* res = _smalloc(sizeof(*res));
	res->right = NULL;
	res->left = NULL;
	res->contend = c;
	return res;
}

// -----------------------------------------------------------------------------
/**
 * Merges to heaps together. Will preserve the correct priority order.
 * Complexity O(log n)
 */
static Heap* _priq_heap_merge(Heap* h1, Heap* h2, Pricmp cmp)
{
	if(_priq_is_empty_heap(h1))
		return h2;

	if(_priq_is_empty_heap(h2))
		return h1;

	if(cmp(h1->contend, h2->contend) > 0) // h1 > h2
	{ 
		// h1 now tmp var
		h1 = _priq_heap_merge(h2->left, h1, cmp);

		// care for balance
		h2->left = h2->right;
		h2->right = h1;

		ASSERT(_priq_heap_inv(h2, cmp), "_priq_heap_merge: inv failed");
		return h2;
	}
	else   // h1 <= h2	
	{
		// h2 now tmp var
		h2 = _priq_heap_merge(h1->left, h2, cmp);

		// care for balance
		h1->left = h1->right;
		h1->right = h2;

		ASSERT(_priq_heap_inv(h1, cmp), "_priq_heap_merge: inv failed");
		return h1;
	}
}

// -----------------------------------------------------------------------------
/**
 * Invariant helper function.
 */
static inline bool _priq_ge_or_eq(Heap* h, cp c, Pricmp cmp)
{
	return _priq_is_empty_heap(h)
		||
		(cmp(h->contend, c) >= 0);
}

// -----------------------------------------------------------------------------
/**
 * Invariant helper function.
 */
static uint64_t _priq_count_contend(Heap* h)
{
	if(_priq_is_empty_heap(h))
		return 0;
	else
		return 1 + _priq_count_contend(h->left)
		         + _priq_count_contend(h->right);

}

// -----------------------------------------------------------------------------
/**
 * Heap Invariant.
 * Complexity always O(n)
 */
static bool _priq_heap_inv(Heap* h, Pricmp cmp)
{
	return _priq_is_empty_heap(h) 
		||
		(
			_priq_ge_or_eq(h->left, h->contend, cmp)
		 	&&
		  	_priq_ge_or_eq(h->right, h->contend, cmp)
		 	&&
		  	_priq_heap_inv(h->left, cmp)
		 	&&
		  	_priq_heap_inv(h->right, cmp)
		);
}


// -----------------------------------------------------------------------------
/**
 * Destroys a heap, frees all memory and relives the contend with
 * a user defined free function. The contend will not be freed
 * if = NULL.
 * Complexity O(n)
 */
static void _priq_heap_destroy(Heap* h, Freefunc ff)
{
	if(! _priq_is_empty_heap(h))
	{
		_priq_heap_destroy(h->right, ff);
		_priq_heap_destroy(h->left, ff);
		
		if(ff != NULL)
			ff(h->contend);

		free(h);
	}
}


// -----------------------------------------------------------------------------
/**
 * Calculates the distance to the deepest leaf in the tree. Used for balance debug.
 * Complexity O(n)
 */
#ifdef BALANCE_TESTS
static uint64_t _priq_heap_deepness(Heap* h)
{
	if(_priq_is_empty_heap(h))
		return 0;
	
	uint64_t l = _priq_heap_deepness(h->left);
	uint64_t r = _priq_heap_deepness(h->right);

	return ( l > r ) ? 1 + l : 1 + r;
}
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS EXTERN

// -----------------------------------------------------------------------------
/**
 * Priority queue invariant check.
 * Complexity always O(2n)
 * @return If NULL -> Ok. Else an error msg.
 */
const char* priq_invariant(Priq q)
{
	if(!q)
		return "NULL POINTER EXCEP: Pcue base struct undefinded";

	if(!q->cmp)
		return "NULL POINTER EXCEP: Pcue compare function undefinded";

	if(!q->top && priq_size(q) != 0)
		return "WRONG STRUCTURE: top = NULL but size > 0";

	if(q->top && priq_size(q) == 0)
		return "WRONG STRUCTURE: top != NULL but size = 0";

	if(!_priq_heap_inv(q->top, q->cmp))
		return "WRONG STRUCTURE: heap invariant failed";
	
	if(q->size != _priq_count_contend(q->top))
		return "WRONG STRUCTURE: size != real #contend";

	return NULL;
}


#ifdef INVARIANT_CHECKS
// -----------------------------------------------------------------------------
/**
 * Invariant check suited for assert macros. 
 * Complexity always O(2n)
 */
bool priq_check_invariant(Priq q)
{
	const char* msg = priq_invariant(q);
	if(msg)
	{
		perr("%s", msg);
		return false;
	}
	return true;
}
#endif

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
Priq priq_create(Pricmp cmp)
{
	Priq res = _smalloc(sizeof(*res));
	res->cmp = cmp;
	res->size = 0;
	res->top = NULL;

	ASSERT(priq_check_invariant(res));
	return res;
}


// -----------------------------------------------------------------------------
/**
 * Destroys a queue. All memory is released.
 * Freefunc will be used on every contend unless it is NULL;
 * Complexity always O(n)
 */
void priq_destroy(Priq q, Freefunc ff)
{
	ASSERT(priq_check_invariant(q), "priq_destroy: inv failed before");

	_priq_heap_destroy(q->top, ff);

	free(q);
}


// -----------------------------------------------------------------------------
/**
 * Enqueues an element into the queue.
 * Complexity always O(log n)
 */
void priq_enqueue(Priq q, cp c)
{
	ASSERT(priq_check_invariant(q), "priq_enqueue: inv failed before");

	Heap* tmp = _priq_create_heap(c);

	q->top = _priq_heap_merge(q->top, tmp, q->cmp);
	q->size++;

	ASSERT(priq_check_invariant(q), "priq_enqueue: inv failed after");
}

// -----------------------------------------------------------------------------
/**
 * Dequeues an element from the queue. Return NULL if the queue is empty.
 * Complexity always O(log n)
 */
cp priq_dequeue(Priq q)
{
	ASSERT(priq_check_invariant(q), "priq_dequeue: inv failed before");
	
	if(priq_is_empty(q))
		return NULL;

	cp* res = priq_peek(q);
	Heap* delme = q->top;

	q->top = _priq_heap_merge(q->top->right, q->top->left, q->cmp);
	free(delme);
	q->size--;

	ASSERT(priq_check_invariant(q), "priq_dequeue: inv failed after");

	return res;
}


// -----------------------------------------------------------------------------
/**
 * Merges two queues intp one. Don't use q1 or q2 after the call of 
 * this function. Only the returned queue is supposed to be touched again.
 * Returns NULL if the queues have different comparison functions.
 * Complexity O(log n)
 */
Priq priq_merge(Priq q1, Priq q2)
{
	ASSERT(priq_check_invariant(q1), "priq_merge: inv q1 failed before");
	ASSERT(priq_check_invariant(q2), "priq_merge: inv q2 failed before");

	if(q1 == q2)
		return q1;

	if(q1->cmp != q2->cmp)
		return NULL;

	q1->top = _priq_heap_merge(q1->top, q2->top, q1->cmp);
	q1->size += q2->size;

	free(q2);

	ASSERT(priq_check_invariant(q1), "priq_merge: inv failed after");

	return q1;
}

