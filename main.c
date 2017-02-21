#include <ss_allocator.h>
#include <stdio.h>


void stack_test()
{
    ss_allocator al = { 0 };
    ss_allocator *a = &al;
    ss_stack(a, 0, 1024);
    int *ps[1024];
    int n = 0;
    uint len = 1024;
    char buf[len];
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_STACK));
    puts("starts allocating");    
    while(ps[n++] = stalloc(a, int))
	*ps[n-1] = 10;
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_STACK));
    printf("stack allocated %d\n", n);
    while(--n >= 0)
	sfree(a, ps[n]);
    puts("freed");    
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_STACK));    
    sdestroy(a);
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_STACK));    
}

void lin_test()
{
    ss_allocator al = { 0 };
    ss_allocator *a = &al;
    ss_lin(a, 0, 1024);
    int *ps[1024];
    int n = 0;
    uint len = 1024;
    char buf[len];
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_LIN));
    puts("starts allocating");    
    while(ps[n++] = stalloc(a, int))
	*ps[n-1] = 10;
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_LIN));
    printf("lin allocated %d\n", n);
    sclear(a);
    puts("freed");    
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_LIN));    
    sdestroy(a);
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_LIN));    
}


void pool_test()
{
    ss_allocator al = { 0 };
    ss_allocator *a = &al;
    int *ps[1024];
    uint len = 1024;
    char buf[len];
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));
    ss_pool(a, 0, 10, sizeof(int), alignof(int));
    for(int n = 0; n < 10; n++) {
	ps[n] = stalloc(a, int);
	*ps[n] = 10;
    }
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));    
    for(int n = 0; n < 10; n++) {
	sfree(a, ps[n]);
    }
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));            
    for(int n = 0; n < 10; n++) {
	ps[n] = stalloc(a, int);
	*ps[n] = 10;	
    }
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));        
    sfree(a, ps[3]);
    sfree(a, ps[6]);
    sfree(a, ps[9]);
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));
    puts("3 slots");
    stalloc(a, int);
    puts("2 slots");    
    stalloc(a, int);
    puts("1 slots");    
    stalloc(a, int);    
    sdestroy(a);
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_POOL));    
}


void heap_test()
{
    ss_allocator al = { 0 };
    ss_allocator *a = &al;
    ss_heap(a);
    int *ps[1024];
    uint len = 1024;
    char buf[len];
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_HEAP));
    for(int n = 0; n < len; n++) {
	ps[n] = stalloc(a, int);
	*ps[n] = 10;
    }
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_HEAP));
    for(int n = 0; n < len; n++) {
     	sfree(a, ps[n]);
    }
    puts(ss_allocator_str(buf, len, a, SS_ALLOCATOR_HEAP));            
}

int main(void)
{
    stack_test();
    pool_test();
    lin_test();
    heap_test();
    return 0;
}
