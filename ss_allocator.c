#include <ss_allocator.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define SS_HEAP_PAD UINT_MAX

void (*ss_mem_die_fn)() = 0;
void (*ss_mem_log_fn)(char*, char*) = 0;
void ss_mem_die()
{
    if(ss_mem_die_fn)
	ss_mem_die_fn();
    else
	abort();
}

void ss_set_mem_die(void(*fn)())
{
    ss_mem_die_fn = fn;    
}

void ss_mem_log(char *c0, char *c1)
{
    if(ss_mem_log_fn)
	ss_mem_log_fn(c0, c1);
    else {
	puts(c0);
	puts(c1);
    }
}

void ss_set_mem_log(void(*fn)(char*,  char*))
{
    ss_mem_log_fn = fn;
}




void *ss_align_top(void *p, uint a)
{
    SS_ASSERT(a >= 1);
    SS_ASSERT(a % 2 == 0);
    uptr ptr = (uptr)p;
    uint mod = ptr % a;
    if(mod)
	ptr += a - mod;
    return (void*)ptr;
}

inline void* ss_data_ptr(ss_heap_header *h, uint al)
{
    void *p = h + 1;
    return ss_align_top(p, al);
}


char *ss_allocator_str(char *buf, uint size, ss_allocator *x, int type)
{
    if(!x) {
	snprintf(buf, size, "empty");
    }
    else if(type == SS_ALLOCATOR_STACK) {
	snprintf(buf, size, "stack\tback %p\tbuf %p\ttop %p\t size %d\tcount %d",
		x->back, x->stack.buf, x->stack.top, x->stack.size, x->stack.count);	
    }
    else if(type == SS_ALLOCATOR_POOL) {
	snprintf(buf, size, "pool\tback %p\tbuf %p\tlist %p\tblock %d\talign %d\tcount %d\tsize %d",
		 x->back, x->pool.buf, x->pool.list, x->pool.block, x->pool.align,
		 x->pool.count, x->pool.count * x->pool.block);
    }
    else if(type == SS_ALLOCATOR_LIN) {
snprintf(buf, size, "lin\tback %p\tbuf %p\tsize %d\toffset %d",
	 x->back, x->lin.buf, x->lin.size, x->lin.offset);
    }
    else if(type == SS_ALLOCATOR_HEAP) {
snprintf(buf, size, "heap\tsize %d\tcount %d",
	 x->heap.size, x->heap.count);
    }
    else {
	snprintf(buf, size, "not recognized");
    }
    return buf;
}



void ss_pool(ss_allocator *x, ss_allocator *a, uint count, uint size, uint al)
{
    memset(x, 0, sizeof(ss_allocator));
    SS_ASSERT(count > 0);
    SS_ASSERT(size > 0);
    SS_ASSERT(al > 0);    
    x->back = a;
    uint block_size = size + al;
    uint pool_size = count * block_size;
    char *m = a ? saalloc(a, pool_size, al) : ss_malloc(pool_size);
    char *c = m;
    for(uint n = 0; n < count - 1; n++) {
	uptr *next = (uptr*) c;
	*next = (uptr)c + block_size;
	c += block_size;	
    }
    uptr *end = (uptr*)c;
    *end = 0;
    x->pool.buf = m;
    x->pool.list = m;
    x->pool.align = al;
    x->pool.block = size;
    x->alloc = ss_pool_alloc;
    x->free = ss_pool_free;
    x->clear = ss_pool_clear;
    x->destroy = ss_pool_destroy;
}

void *ss_pool_alloc(ss_allocator *a, uint s, uint al)
{
    SS_ASSERT(s == a->pool.block);
    SS_ASSERT(al == a->pool.align);
    SS_ASSERT(a->pool.list != 0);
    uptr nf = *((uptr*) a->pool.list);
    void *p = a->pool.list;
    a->pool.list = (void*)nf;
    a->pool.count++;
    return p;
}

void ss_pool_free(ss_allocator *a, void *p)
{
    if(!p)
	return;
    SS_ASSERT(a->pool.count > 0);
    uptr *n = (uptr*)p;
    *n = (uptr) a->pool.list;
    a->pool.list = p;
    a->pool.count--;    
}

void ss_pool_clear(ss_allocator *a)
{
    a->pool.count = 0;
    a->pool.list = a->pool.buf;
}
void ss_pool_destroy(ss_allocator *a)
{
    if(a->back)
	sfree(a->back, a->pool.buf);
    else
	free(a->pool.buf);
    a->pool.buf = 0;    
}



void ss_stack(ss_allocator *x, ss_allocator *a, uint size)
{
    memset(x, 0, sizeof(ss_allocator));
    x->back = a;
    x->stack.buf = a ? salloc(a, size) : ss_malloc(size);
    x->stack.top = x->stack.buf;
    x->stack.size = size;
    x->stack.count = 0;
    x->alloc = ss_stack_alloc;
    x->free = ss_stack_free;
    x->clear = ss_stack_clear;
    x->destroy = ss_stack_destroy;
}

void *ss_stack_alloc(ss_allocator *a, uint s, uint al)
{
    const uint hsize = sizeof(ss_stack_header);
    uint size = hsize + s + al;
    if(a->stack.top + size > a->stack.buf + a->stack.size)
	return 0;
    uint offset = a->stack.top - a->stack.buf;
    a->stack.top = ss_align_top(a->stack.top + hsize, al) - hsize;	
    ss_stack_header *h = (ss_stack_header*)(a->stack.top);
    h->offset = offset;
    h->id = a->stack.count;
    void *ret = a->stack.top + hsize;
    a->stack.top += size;
    a->stack.count++;
    return ret;
}

void ss_stack_free(ss_allocator *a, void *p)
{
    if(!p)
	return;
    ss_stack_header *h = (ss_stack_header*)((char*)p - sizeof(ss_stack_header));
    SS_ASSERT(h->id == a->stack.count - 1);
    a->stack.top = a->stack.buf + h->offset;
    a->stack.count--;
}

void ss_stack_clear(ss_allocator *a)
{
    a->stack.top = a->stack.buf;
    a->stack.count = 0;    
}

void ss_stack_destroy(ss_allocator *a)
{
    if(a->back)
	sfree(a->back, a->stack.buf);
    else
	free(a->stack.buf);
    a->stack.buf = 0;
}








void ss_lin(ss_allocator *x, ss_allocator *a, uint size)
{
    memset(x, 0, sizeof(ss_allocator));
    x->back = a;
    x->lin.buf = a ? salloc(a, size) : ss_malloc(size);
    x->lin.size = size;
    x->lin.offset = 0;    
    x->alloc = ss_lin_alloc;
    x->free = ss_lin_free;
    x->clear = ss_lin_clear;
    x->destroy = ss_lin_destroy;
}

void *ss_lin_alloc(ss_allocator *a, uint s, uint al)
{
    uint size = s + al;
    if(a->lin.offset + size > a->lin.size)
	return 0;
    void *p = ss_align_top((char*)a->lin.buf + a->lin.offset, al);
    a->lin.offset += size;
    return p;
}

void ss_lin_free(ss_allocator *a, void*p)
{
    (void)a; (void)p;
}
void ss_lin_clear(ss_allocator *a)
{
    a->lin.offset = 0;    
}

void ss_lin_destroy(ss_allocator *a)
{
    if(a->back)
	sfree(a->back, a->lin.buf);
    else
	free(a->lin.buf);
    a->lin.buf = 0;
}


void ss_heap(ss_allocator *x)
{
    memset(x, 0, sizeof(ss_allocator));    
    mtx_init(&x->heap.mtx, mtx_timed | mtx_recursive);
    puts("mtx init");
    x->alloc = ss_heap_alloc;
    x->free = ss_heap_free;
    x->clear = ss_heap_clear;
    x->destroy = ss_heap_destroy;    
}

void *ss_heap_alloc(ss_allocator *a, uint s, uint al)
{
    mtx_t *l = &a->heap.mtx;
    mtx_lock(l);
    uint size = s + al + sizeof(ss_heap_header);
    ss_heap_header *h = ss_malloc(size);
    h->size = size;
    void *p = ss_align_top(h + 1, al);
    uint *up = (uint*)(h + 1);
    while(up != p) {
	*up = SS_HEAP_PAD;
	up++;
    }
    a->heap.size += size;
    a->heap.count++;
    mtx_unlock(l);
    return p;
}
void ss_heap_free(ss_allocator *a, void*p)
{
    if(!p)
	return;
    mtx_t *l = &a->heap.mtx;
    mtx_lock(l);
    uint *up = p;
    while(up[-1] == SS_HEAP_PAD)
	--up;
    ss_heap_header *h = (ss_heap_header*)up - 1;
    a->heap.size -= h->size;
    a->heap.count--;
    free(h);    
    mtx_unlock(l);    
}

void ss_heap_clear(ss_allocator *a)
{
    (void)a;
}
void ss_heap_destroy(ss_allocator *a)
{
    (void)a;
}

