#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "petersonlock.h"

extern struct petersonlock peterson_locks[15];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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
sys_peterson_create(void)
{
  for (int i = 0; i < 15; i++) {
    if (__sync_lock_test_and_set(&peterson_locks[i].used, 1) == 0) {
      __sync_synchronize();
      peterson_locks[i].flag[0] = 0;
      peterson_locks[i].flag[1] = 0;
      peterson_locks[i].turn = 0;
      return i;
    }
  }
  return -1;
}

uint64
sys_peterson_acquire(void)
{
  int lock_id, role;
  argint(0, &lock_id);
  argint(1, &role);

  if (lock_id < 0 || lock_id >= 15 || (role != 0 && role != 1))
    return -1;

  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->used == 0)
    return -1;

  int other = 1 - role;

  lock->flag[role] = 1;
  __sync_synchronize();
  lock->turn = role;
  __sync_synchronize();

  while (lock->flag[other] && lock->turn == role) {
    yield();
  }

  return 0;
}

uint64
sys_peterson_release(void)
{
  int lock_id, role;
  argint(0, &lock_id);
  argint(1, &role);

  if (lock_id < 0 || lock_id >= 15 || (role != 0 && role != 1))
    return -1;

  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->used == 0)
    return -1;

  __sync_synchronize(); // ensure critical section is done before release
  lock->flag[role] = 0;

  return 0;
}

uint64
sys_peterson_destroy(void)
{
  int lock_id;
  argint(0, &lock_id);

  if (lock_id < 0 || lock_id >= 15)
    return -1;

  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->used == 0)
    return -1;

  lock->flag[0] = 0;
  lock->flag[1] = 0;
  lock->turn = 0;
  __sync_synchronize();
  lock->used = 0;

  return 0;
}