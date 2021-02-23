// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

#define chunk ((PHYSTOP - 1 - (uint64)end) / NCPU)

const int move = 1; // #of pages move to other cpu

struct run {
  struct run *next;
};

struct {
  struct spinlock lock[NCPU];
  struct run *freelist[NCPU]; // each cpu has its free list
} kmem;

// struct {
//   struct spinlock lock;
//   struct run *freelist; // each cpu has its free list
// } kmem;

void
kinit()
{
  for (int id = 0; id < NCPU; id++){
    char name[6] = "kmem 0";
    // name[4] = id + '0';
    // printf("%s\n", 1+id);
    initlock(&kmem.lock[id], name);
    freerange(end + chunk*id, end + chunk*(id+1) - 1);
  }
    //   initlock(&kmem.lock, "kmem");
    // freerange(end , (void*)PHYSTOP);

}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{

  push_off();
  int cpuid = r_tp();
  pop_off();
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  if ((uint64)r > PHYSTOP - 10000) {
    printf("%d cpu free %p\n", cpuid, r);
  }

  //   acquire(&kmem.lock);
  // r->next = kmem.freelist;
  // kmem.freelist = r;
  // release(&kmem.lock);

// is it your cpu ?
  acquire(&kmem.lock[cpuid]);
  r->next = kmem.freelist[cpuid];
  kmem.freelist[cpuid] = r;
  release(&kmem.lock[cpuid]);
}


void 
stealfreelist(int id, int cpuid)
{
   struct run *tmp = kmem.freelist[id];
    struct run *cur = kmem.freelist[id];
    int len = 0;
    if (cur->next == 0) panic("cur->next");
    while (cur->next && len < move){
      printf("len: %d\n", len);
        cur = cur->next;
        len++;
        if (cur == 0) panic ("cur is zero");
        // if (cur->next == 0) panic ("next is zero");
    }
    
    kmem.freelist[id] = cur->next;
    cur = 0;
    kmem.freelist[cpuid] = tmp;

    release(&kmem.lock[id]);

}
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
//   push_off();
//   int cpuid = r_tp();
//   pop_off();
//  int id;

  struct run *r;

  acquire(&kmem.lock);
  // acquire(&kmem.lock[cpuid]);
  // r = kmem.freelist[cpuid];
  r = kmem.freelist;
  if(r)
    // kmem.freelist[cpuid] = r->next;
    kmem.freelist = r->next;
  // needs other cpu's freelist
  // else {
  //   // find the cpu who has freelist to steal
  //   for (int j = 1; j < NCPU; j++){
  //     id = (cpuid + j) % NCPU;
  //     acquire(&kmem.lock[id]);
  //     if (! kmem.freelist[id]){ 
  //       // goto found;
  //       stealfreelist(id, cpuid);
  //       break;
  //     }
  //     release(&kmem.lock[id]);
  //   }

  // }

  release(&kmem.lock);
  // release(&kmem.lock[cpuid]);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;






}
