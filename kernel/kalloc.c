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

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  char name[10];  // the name of the lock
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    snprintf(kmem[i].name, 10, "kmem_%d", i);
    initlock(&kmem[i].lock, kmem[i].name);
  }
  freerange(end, (void*)PHYSTOP);
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
  // cpuid() is only safe to call it and use its result
  // when interrupts are turned off
  push_off();
  int cpu_id = cpuid();

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  
  acquire(&kmem[cpu_id].lock);
  r->next = kmem[cpu_id].freelist;
  kmem[cpu_id].freelist = r;
  release(&kmem[cpu_id].lock);

  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  // cpuid() is only safe to call it and use its result
  // when interrupts are turned off
  push_off();
  int cpu_id = cpuid();

  struct run *r;

  acquire(&kmem[cpu_id].lock);
  r = kmem[cpu_id].freelist;
  if (!r) { 
    // the one CPU must "steal" part of the other CPU's free list
    // I steal all
    int success = 0;
    for (int i = 0; i < NCPU && !success; i++) {
      if (i != cpu_id) {
        acquire(&kmem[i].lock);
        if (kmem[i].freelist) {
          kmem[cpu_id].freelist = kmem[i].freelist;
          kmem[i].freelist = 0; // freelist of core i is now empty
          r = kmem[cpu_id].freelist;
          // remember to break the loop once stealing is successful
          success = 1;
        }
        release(&kmem[i].lock);
      }
    }
  }
  if(r)
    kmem[cpu_id].freelist = r->next;
  release(&kmem[cpu_id].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  
  pop_off();

  return (void*)r;
}
