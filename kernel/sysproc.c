#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 vAddr; // the starting virtual address
  int numPages; // the number of pages to check
  unsigned int abits = 0; // store the results into a bitmask
  uint64 temp;

  if (argaddr(0, &vAddr) < 0)
    return -1;  
  if (argint(1, &numPages) < 0)
    return -1;
  if (argaddr(2, &temp) < 0)
    return -1;

  for (int i = 0; i < numPages; i++) {
    // use walk to get entries
    pte_t* pte = walk(myproc()->pagetable, vAddr, 0); 
    // check PTE_A and set abits
    if (*pte & PTE_A) {
      abits |= (1 << i);
      *pte ^= PTE_A;
    }
    // go to the next page
    vAddr += PGSIZE;
    if (vAddr >= MAXVA)
      break;
  }
  // copyout abits to address temp
  if(copyout(myproc()->pagetable, temp, (char *)&abits, sizeof(abits)) < 0)
    return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}