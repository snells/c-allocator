#ifndef SS_ALLOCATOR_H
#define SS_ALLOCATOR_H
#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>
#include <tinycthread.h>
#include <stdint.h>


typedef unsigned int uint;
typedef uintptr_t uptr;

void ss_mem_die();
void ss_set_mem_die(void(*)());
void ss_mem_log(char*, char*);
void ss_set_mem_log(void(*)(char*, char*));

#define SE_LOG_DEBUG(m, ...) do {					\
	char buf0[2048];						\
	sprintf(buf0, "[ERROR]\t%s:%i %s:", __FILE__, __LINE__,  __func__); \
	char buf1[4096];						\
	sprintf(buf1, m, ##__VA_ARGS__);				\
	ss_mem_log(buf0, buf1);						\
    } while(0)					



#if SS_DEBUG
#define

#define SE_ASSERT(c) do {			\
	if(!(c)) {				\
	    SS_LOG_ERROR("assert failed");	\
	    ss_mem_die();			\
	}} while(0)

#define SE_NNULL(x) do {			\
	if(!(x)) {				\
	    SS_LOG_ERROR("pointer null");	\
	    ss_mem_die();			\
	}} while(0)

#else

#define SS_ASSERT(c)
#define SS_NNULL(x)

#endif


#define ss_malloc(s) ({ void *p = malloc((s)); SS_NNULL(p); p; })


#if defined(SS_DEBUG)
#define salloc(a, t) ({ SS_NNULL((a)); (a)->alloc((a), (t), sizeof(max_align_t)); })
#define saalloc(a, t, al) ({ SS_NNULL((a)); (a)->alloc((a), (t), al); })
#define stalloc(a, t) ({ SS_NNULL((a)); (a)->alloc((a), sizeof(t), alignof(t)); })
#define sfree(a, p) ({ SS_NNULL((a)); (a)->free((a), (p)); })
#define sclear(a)	({ SS_NNULL((a)); (a)->clear((a));	}) 
#define sdestroy(a)	({ SS_NNULL((a)); (a)->destroy((a));	}) 


#else
#define salloc(a, t) (a)->alloc((a), (t), sizeof(max_align_t))
#define saalloc(a, t, al) (a)->alloc((a), (t), (al))
#define stalloc(a, t) (a)->alloc((a), sizeof(t), alignof(t))
#define sfree(a, p) (a)->free((a), (p))
#define sclear(a) (a)->clear((a))
#define sdestroy(a) (a)->destroy((a))

#endif

typedef struct {
    void *buf;
    void *list;
    uint block;
    uint align;
    uint count;
} ss_alloc_pool;


typedef struct {
    void *buf;
    uint size;
    uint offset;
} ss_alloc_lin;

typedef struct {
    uint offset;
    uint id;
} ss_stack_header;

typedef struct {
    ss_stack_header h;
    char *buf;
    char *top;
    uint size;
    uint count;
} ss_alloc_stack;


typedef struct {
    mtx_t mtx;
    uint size;
    uint count;        
} ss_alloc_heap;

typedef struct {
    uint size;
} ss_heap_header;

typedef struct ss_allocator_s ss_allocator;

struct ss_allocator_s {
    void*(*alloc)(ss_allocator*, uint, uint);
    void(*free)(ss_allocator*, void*);    
    void(*clear)(ss_allocator*);
    void(*destroy)(ss_allocator*);
    ss_allocator *back;
    union {
	ss_alloc_pool pool;
	ss_alloc_lin lin;
	ss_alloc_stack stack;
	ss_alloc_heap heap;
    };
};

typedef enum {
    SS_ALLOCATOR_STACK = 0,
    SS_ALLOCATOR_POOL,
    SS_ALLOCATOR_LIN,    
    SS_ALLOCATOR_HEAP,
    SS_ALLOCATOR_COUNT,    
} SS_ALLOCATOR_TYPE;



char *ss_allocator_str(char *buf, uint size, ss_allocator *x, int type);

void ss_stack(ss_allocator *x, ss_allocator *a, uint s);
void *ss_stack_alloc(ss_allocator *a, uint s, uint al);
void ss_stack_free(ss_allocator *a, void*p);
void ss_stack_clear(ss_allocator *a);
void ss_stack_destroy(ss_allocator *a);




void ss_pool(ss_allocator *x, ss_allocator *a, uint block, uint size, uint al);
void *ss_pool_alloc(ss_allocator *a, uint s, uint al);
void ss_pool_free(ss_allocator *a, void*p);
void ss_pool_clear(ss_allocator *a);
void ss_pool_destroy(ss_allocator *a);


void ss_lin(ss_allocator *x, ss_allocator *a, uint s);
void *ss_lin_alloc(ss_allocator *a, uint s, uint al);
void ss_lin_free(ss_allocator *a, void*p);
void ss_lin_clear(ss_allocator *a);
void ss_lin_destroy(ss_allocator *a);


void ss_heap(ss_allocator *x);
void *ss_heap_alloc(ss_allocator *a, uint s, uint al);
void ss_heap_free(ss_allocator *a, void*p);
void ss_heap_clear(ss_allocator *a);
void ss_heap_destroy(ss_allocator *a);

#endif 

