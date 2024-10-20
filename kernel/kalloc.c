// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

#define PA2IDX(pa) (((uint64)pa)-KERNBASE)/4096
#define MAXNUMENT PA2IDX(PHYSTOP)
int refCount[MAXNUMENT];

void
kinit()
{
  initlock(&kmem.lock, "kmem");
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
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  refCount[PA2IDX(pa)]--;
  if (refCount[PA2IDX(pa)] <= 0) {
    acquire(&kmem.lock);
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    refCount[PA2IDX(r)] = 1;
  }
  return (void*)r;
}

int kcowcopy(uint64 va) {  
  // safety check
  if (va >= MAXVA)
    return -1;

  struct proc *p = myproc();
  pte_t *pte;
  uint flags;
  char *mem;
  uint64 pa;

  if((pte = walk(p->pagetable, va, 0)) == 0)
    panic("cowcopy: pte should exist");
  if((*pte & PTE_V) == 0)
    panic("cowcopy: page not present");

  pa = PTE2PA(*pte);
  flags = (PTE_FLAGS(*pte) | PTE_W) & ~PTE_COW;
  if((mem = kalloc()) == 0) { // Allocate the new page
    printf("cowcopy: no free memory, the process should be killed\n");
    return -1;
  }
  memmove(mem, (char*)pa, PGSIZE);
  kfree(pa);

  uvmunmap(p->pagetable, PGROUNDDOWN(va), 1, 0);
  mappages(p->pagetable, va, 1, (uint64)mem, flags);

  return 0; 
}

void kaddRefCount(uint64 pa) {
  refCount[PA2IDX(pa)]++;
}