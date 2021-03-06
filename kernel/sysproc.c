#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "stat.h"

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


// for lab soft link
uint64
sys_symlink(void)
{
  //TODO
  int maxstr = 128;
  char target[128], path[128];
  // struct inode *dp;
    struct inode *ip;
  if ( argstr(0, target, maxstr) < 0 || argstr(1, path, maxstr) < 0 ) 
    return -1;
  
  begin_op();
  int creat = 0;
  // printf("%s, %s\n", target, path);
  if((ip = namei(path)) == 0){
    // printf("not found path create\n");
    if ((ip = create(path, T_SYMLINK, 0, 0)) == 0)
      panic("create symlink");
    creat = 1;
  }

  // assume target exists, no need to create target
  if (!creat){
    ilock(ip);
  }
  if ( writei(ip, 0, (uint64)target, 0, sizeof(target)) < 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  // printf("link done\n");
  
  end_op();
  return 0;
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
