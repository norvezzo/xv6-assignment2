#include "types.h"
#include "petersonlock.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "param.h"
#include "proc.h"
#include "defs.h"

struct petersonlock peterson_locks[15];

void
init_peterson_locks(void) {
  for (int i = 0; i < 15; i++) {
    peterson_locks[i].used = 0;
    peterson_locks[i].flag[0] = 0;
    peterson_locks[i].flag[1] = 0;
    peterson_locks[i].turn = 0;
  }
}