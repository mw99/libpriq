/**
 * Pcue Testsuite
 */

/* ---- System Header ------------------------------------------------------------ */
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* ---- DOT Header --------------------------------------------------------------- */
#include "priq.h"

/* ---- Help funcs & macros ------------------------------------------------------ */
#define ES_none   "\033[0m"
#define ES_bold   "\033[1m"
#define ES_red    "\033[31m"
#define ES_yellow "\033[33m"
#define ES_blue   "\033[34m"
#define ES_white  "\033[37m"
#define pinfo(format, ...) fprintf(stderr, ES_bold ES_blue "INFO " ES_none ES_white format ES_none "\n", ## __VA_ARGS__)
#define pwarn(format, ...) fprintf(stderr, ES_bold ES_yellow "WARN " ES_none ES_yellow format ES_none "\n", ## __VA_ARGS__)
#define perr(format, ...)  fprintf(stderr, ES_bold ES_red "ERROR " ES_none ES_red format ES_none "\n", ## __VA_ARGS__)

static inline void* smalloc( size_t s )
{
	void * res = malloc( s );
	if ( !res )
	{
		perr( "smalloc: Out of Memory. Requested size: %zd", s );
		abort();
	}
	return res;
}


#define TEST_FUNC_ARRAY_SIZE 20
#define TEST_ARRAY_SIZE 20000

void( *tests[TEST_FUNC_ARRAY_SIZE] )( void );
uint64_t a[TEST_ARRAY_SIZE];


/* ---- Test Functions ----------------------------------------------------------- */

int icompare( void* e1, void* e2 )
{
	uint64_t* i1 = e1;	
	uint64_t* i2 = e2;	

	return ( ( *i1 >= *i2 ) - ( *i2 >= *i1 ) );
}

void t_01(void)
{
	Priq q = priq_create( icompare );

	if( !q ) {
		perr( "T01: priq_create: failed" ); return; }


	const char* msg = priq_invariant(q);
	if(msg) {
		perr( "T01: priq_create: invariant failed on empty queue: %s", msg ); return; }
	
	priq_destroy( q, NULL );

	pinfo( "T01: priq_create & priq_destroy successful" );
}

void t_02(void)
{
	int i = 0 ;
	int* p = NULL ;

	Priq q = priq_create( icompare );

	priq_enqueue( q, a+5 );
	
	if( priq_size( q ) != 1 ) {
		perr( "T02: priq_enqueue: size should be one." ); return; }
	
	if( priq_peek( q ) != a+5 ) {
		perr( "T02: priq_peek: failed" ); return; }

	if( priq_dequeue( q ) != a+5 && priq_is_empty( q ) ) {
		perr( "T02: priq_dequeue: failed" ); return; }
	
	priq_destroy( q, NULL );

	pinfo( "T02: priq_enqueue & priq_dequeue (simpel) successful" );
}


void t_03(void)
{
	uint64_t i = 0 ;

	Priq q = priq_create( icompare );
	
	for( i = 0; i < 30; ++i)
		priq_enqueue( q, a+i );
	
	if( priq_size( q ) != 30 ) {
		perr( "T03: priq_enqueue: size should be 30." ); return; }
	
	priq_destroy( q, NULL );

	pinfo( "T03: priq_destroy (advanced) successful" );
}

void t_04(void)
{
	uint64_t i = 0 ;

	Priq q = priq_create( icompare );
	
	priq_enqueue( q, a + 15 );
	priq_enqueue( q, a + 16 );
	priq_enqueue( q, a + 14 );

	priq_enqueue( q, a + 17 );
	priq_enqueue( q, a + 10 );
	priq_enqueue( q, a + 12 );

	priq_enqueue( q, a + 11 );
	priq_enqueue( q, a + 13 );
	priq_enqueue( q, a + 18 );

	priq_enqueue( q, a + 20 );
	priq_enqueue( q, a + 19 );
	
	if( priq_size( q ) != 11 ) {
		perr( "T04: priq_enqueue: size should be 11." ); return; }
	
	for( i = 10; i < 21; ++i)
	{
		uint64_t * val = priq_dequeue( q );
		if( !val )
			perr( "T04: priq_dequeue: got NULL");
		else if( *val != i)
			perr( "T04: priq_dequeue: wrong order. Exp: %lu, got: %lu", i, *val );
	}
	
	priq_destroy( q, NULL );

	pinfo( "T04: priq_enqueue & priq_destroy (static sort) successful" );
}

struct just 
{ 
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
	uint64_t a4;
};

int just_cmp( void* e1, void* e2 )
{
	struct just * i1 = e1;	
	struct just * i2 = e2;	

	return ( ( i1->a1 >= i2->a1 ) - ( i2->a1 >= i1->a1 ) );
}

void t_05(void)
{
	uint64_t i = 0 ;
	Priq q = priq_create( just_cmp );

	for( i = 0; i < 10; ++i)
	{ 	
		struct just * add = smalloc( sizeof( *add ) );
		add->a1 = i + 40;
		priq_enqueue( q, add );
	}


	for( i = 0; i < 10; ++i)
	{ 	
		struct just * add = smalloc( sizeof( *add ) );
		add->a1 = i + 10;
		priq_enqueue( q, add );
	}

	for( i = 0; i < 10; ++i)
	{ 	
		struct just * add = smalloc( sizeof( *add ) );
		add->a1 = i + 20;
		priq_enqueue( q, add );
	}

	for( i = 0; i < 10; ++i)
	{ 	
		struct just * add = smalloc( sizeof( *add ) );
		add->a1 = i + 50;
		priq_enqueue( q, add );
	}

	if( priq_size( q) != 40 ) {
		perr( "T05: priq_enqueue: size should be 40." ); return; }
	
	if( ((struct just *)priq_peek( q ))->a1 != 10 ) {
		perr( "T05: priq_enqueue: peek unexpected top" ); return; }
	
	
	priq_destroy( q, free );

	pinfo( "T05: priq_destroy with allocated contend (valgrind test)" );
}


#define RAND_LOOPS 100000

void t_06(void)
{
	uint64_t j = 0 ;
	uint64_t k = 0 ;
	uint64_t l = 0 ;


	for( uint64_t i = 0; i < RAND_LOOPS; ++i)
	{ 	
		Priq q = priq_create( icompare );

		uint64_t inner_adds = rand() % 20;

		for( uint64_t j = 0; j < inner_adds; ++j)
		{
			priq_enqueue( q, a + (rand() % inner_adds) );
		}

		if( priq_size( q ) != inner_adds ) {
			perr( "T06: priq_enqueue: size should be %lu but was %lu.", 
			inner_adds, priq_size( q ) ); return; }

		const char* msg = priq_invariant(q);
		if(msg) {
			perr( "T01: priq_create: invariant failed: %s", msg ); return; }

		uint64_t last_time = ULLONG_MAX;;

		for( uint64_t j = 0; j < inner_adds; ++j)
		{
			uint64_t * tmp =  priq_dequeue( q );

			if( !tmp ) {
				perr( "T06: priq_dequeue: got unexpected NULL pointer" ); return; }

			if( j != 0 && icompare( &last_time, tmp ) == 1 ) {
				perr( "T06: priq_dequeue: wrong order" ); return; }

			last_time = *tmp;
		}

		if( !priq_is_empty( q ) ) {
			perr( "T06: priq_dequeue: is not empty but it should" ); return; }


		priq_destroy( q, NULL );
	}

	pinfo( "T06: priq_* massive random test successful" );
}

void t_07(void)
{
	uint64_t j = 0 ;
	uint64_t k = 0 ;
	uint64_t l = 0 ;

	Priq res = NULL;

	Priq q1 = priq_create( icompare );
	Priq q2 = priq_create( icompare );
	Priq q3 = priq_create( icompare );

	priq_enqueue( q1, a + 5);
	priq_enqueue( q1, a + 1);
	priq_enqueue( q1, a + 8);

	priq_enqueue( q2, a + 7);
	priq_enqueue( q2, a + 2);
	priq_enqueue( q2, a + 9);

	priq_enqueue( q3, a + 6);
	priq_enqueue( q3, a + 4);
	priq_enqueue( q3, a + 3);

	res = priq_merge( q1, q2 );
	res = priq_merge( res, q3 );

	if( priq_size( res ) != 9 ) {
		perr( "T07: priq_merge: size should be 9 but was %lu.", 
			priq_size( res ) ); return; }

	for( uint64_t i = 1; i < 10; ++i)
	{
		uint64_t * get = priq_dequeue( res );

		if( !get ) {
			perr( "T07: priq_merge+dequeue: got unexpected NULL pointer" ); return; }

		if( i != *get ) {
			perr( "T07: priq_merge: contend should be %lu but was %lu.", i, *get); return; }

	}

	priq_destroy(res, NULL);

	pinfo( "T07: priq_merge test successful" );
}


void t_08(void)
{
	uint64_t j = 0 ;

	Priq qmain = priq_create( icompare );

	for( uint64_t j = 1; j < 150; ++j)
	{
		priq_enqueue( qmain, a + (rand() % TEST_ARRAY_SIZE));
	}

	for( uint64_t j = 1; j < 50; ++j)
	{
		priq_dequeue( qmain );
	}

	for( uint64_t i = 1; i < 100; ++i)
	{
		Priq qtmp = priq_create( icompare );

		for( uint64_t j = 1; j < 150; ++j)
		{
			priq_enqueue( qtmp, a + (rand() % TEST_ARRAY_SIZE));
		}
		for( uint64_t j = 1; j < 50; ++j)
		{
			priq_dequeue( qtmp);
		}

		qmain = priq_merge( qmain, qtmp );
	}

	if( priq_size( qmain ) != 10000 ) {
		perr( "T08: priq_merge: size should be 10000 but was %lu.", 
			priq_size( qmain ) ); return; }

	uint64_t last = 0;

	while( !priq_is_empty(qmain))
	{
		uint64_t * get = priq_dequeue( qmain );

		if( !get ) {
			perr( "T08: priq_merge+dequeue: got unexpected NULL pointer" ); return; }

		if( last > *get ) {
			perr( "T08: priq_merge failed to preserve random order" ); return; }

	}

	priq_destroy(qmain, NULL);

	pinfo( "T08: priq_merge massive random test successful" );
}




int main( void )
{
	srand( time( NULL ) );

	// just in case
	for ( int i = 0; i < TEST_FUNC_ARRAY_SIZE ; ++i )
		tests[i] = NULL;

	for ( int i = 0; i < TEST_ARRAY_SIZE; ++i )
		a[i] = i;

	// 0 reserved
	tests[1] = t_01;
	tests[2] = t_02;
	tests[3] = t_03;
	tests[4] = t_04;
	tests[5] = t_05;
	tests[6] = t_06;
	tests[7] = t_07;
	tests[8] = t_08;

	for ( int i = 0; i < TEST_FUNC_ARRAY_SIZE ; ++i )
		if( tests[i] )
			( *tests[i] )( );
	
	return 0;
}
