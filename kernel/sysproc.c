#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fcntl.h"

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

uint64
sys_mmap(void)
{
  uint64 addr;
  int length, fd, prot, flags, off;
  if(argaddr(0, &addr) < 0 ||
    argint(1, &length) || 
    argint(2, &prot) || 
    argint(3, &flags) || 
    argint(4, &fd) || 
    argint(5, &off)
    ){
    return -1;
  }

  pte_t *pte;
  struct proc *proc = myproc();
  uint64 va = PGROUNDUP(proc->trapframe->sp);
  int count = 0;
  uint64 startva = 0;
  while (va < MAXVA){
    // printf("%p %d\n",va, va);
    if ( (pte = walk(proc->pagetable, va, 0)) == 0)
      return -1;
    if ((*pte & PTE_V) == 0){
      if (count == 0){
        startva = va;
      }
      count++;
    }
    if (count >= 4){
      count = 0;
      break;
    }
    va += PGSIZE;
  }

  struct file *f = proc->ofile[fd];
  printf("va %p\n", va);
  struct vma vma = {
    (void*)startva, length, prot, flags, fd, off, f,
  };
  addr = startva;
  proc->vmas[proc->idx++] = &vma;
  filedup(f);
  
  return 0;
}

uint64
sys_munmap(void)
{
  uint64 addr;
  int length;
  if(argaddr(0, &addr) < 0 || argint(1, &length) ){
    return -1;
  }

struct proc *proc = myproc();
  for (int i = 0; i < 16; i++){
    uint64 ad = (uint64)proc->vmas[i]->addr;
    if (addr >= ad && addr <= ad + length){
      struct vma *vma = proc->vmas[i];
      if (vma->dirty && vma->flags == MAP_SHARED ){
        //write back
      }
      uvmunmap(proc->pagetable, ad, 1, 1);
    }
  }
  return 0;
}
