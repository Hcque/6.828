#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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

  backtrace();
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
sys_sigreturn(void);

uint64
sys_sigalarm(void)
{
  uint64 handler;
  int ticks;
  struct proc *p = myproc();

  if (argint(0, &ticks) < 0)
    return -1;
  if (argaddr(1, &handler) < 0)
    return -1;

  if (ticks == 0 && handler == 0){
    p->alarm = 0;
  } else {
    p->alarm = 1;
  }

  p->interval = ticks;
  p->handler = (void(*)(void))handler;
  p->inhandle = 0;
  return 0;

}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();

  p->trapframe->epc = p->pastepc;

  p->trapframe->ra = p->pastra;
       p->trapframe->sp = p->pastsp;
       p->trapframe->gp = p->pastgp;
       p->trapframe->tp = p->pasttp;
       p->trapframe->a0 = p->pasta0;
      p->trapframe->a1 = p->pasta1;
     p->trapframe->a2 = p->pasta2;
    p->trapframe->a3 = p->pasta3;
       p->trapframe->a4 = p->pasta4;
       p->trapframe->a5 = p->pasta5;
       p->trapframe->a6 = p->pasta6;
       p->trapframe->a7 = p->pasta7;
       p->trapframe->t0 = p->pastt0;
       p->trapframe->t1 = p->pastt1;
       p->trapframe->t2 = p->pastt2;
       p->trapframe->t3 = p->pastt3;
       p->trapframe->t4 = p->pastt4;
       p->trapframe->t5 = p->pastt5;
       p->trapframe->t6 = p->pastt6;

  p->trapframe->s0 = p->pasts0;
  p->trapframe->s1 = p->pasts1;
  p->trapframe->s2 = p->pasts2;
  p->trapframe->s3 = p->pasts3;
  p->trapframe->s4 = p->pasts4;
  p->trapframe->s5 = p->pasts5;
  p->trapframe->s6 = p->pasts6;
  p->trapframe->s7 = p->pasts7;
  p->trapframe->s8 = p->pasts8;
  p->trapframe->s9 = p->pasts9;
  p->trapframe->s10 = p->pasts10;
  p->trapframe->s11 = p->pasts11;

  p->inhandle = 0;
  return 0;
}
