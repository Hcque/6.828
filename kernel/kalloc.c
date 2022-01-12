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
void _kfree(void *pa, int _id); // _id is the cpuid

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  printf("kinit\n");
  for (int i = 0; i < NCPU ; i ++ )
  {
  initlock(&kmem[i].lock, "kmem" + ('A'+i));
  }
  freerange(end, (void*)PHYSTOP);
  printf("kinitDONE\n");
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE )
    kfree(p);
  printf(" freerange DONE\n");
}


// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa) // _id is the cpuid
{
  push_off();
  int _id = cpuid();
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[_id].lock);
  r->next = kmem[_id].freelist;
  kmem[_id].freelist = r;
  release(&kmem[_id].lock);
  pop_off();
}


// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  push_off();
  struct run *r;
  int _id = cpuid();

  acquire(&kmem[_id].lock);
  r = kmem[_id].freelist;
  if(r)
    kmem[_id].freelist = r->next;

  // steal from other cpu if possible =====
  // int steal_ok = 0;
  if (!r)
  {
    // printf("steal\n");
    for (int nxtid = 0; nxtid < NCPU; nxtid ++ ) if (nxtid != _id)
    {
      acquire(&kmem[nxtid].lock);
      r = kmem[nxtid].freelist;
      if(r)
      {
        // steal_ok = 1;
        kmem[nxtid].freelist = r->next;

        release(&kmem[nxtid].lock);
        break;
      }
      release(&kmem[nxtid].lock);

    }
  }
   // ==================================
  release(&kmem[_id].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  pop_off();
  return (void*)r;
}
