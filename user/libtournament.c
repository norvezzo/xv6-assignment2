#include "kernel/types.h"
#include "user/user.h"

#define MAX_PROCESSES 16

int total_levels;
int *lock_ids; 

int tournament_lock_ids[4];
int tournament_roles[4];

int tournament_create(int processes) {
  if (processes <= 0 || processes > MAX_PROCESSES)
    return -1;

  // checking if the number of processes is a power of two
  if ((processes & (processes - 1)) != 0)
    return -1;

  total_levels = 0;
  int temp = processes;
  // computing log2(processes)
  while (temp >>= 1)
    total_levels++;  

  int num_locks = processes - 1;
  lock_ids = malloc(sizeof(int) * num_locks);
  if (!lock_ids) 
    return -1;

  for (int i = 0; i < num_locks; i++) {
    int id = peterson_create();
    if (id < 0)
      return -1;
    lock_ids[i] = id;
  }

  for (int i = 0; i < processes; i++) {
    int pid = fork();
    if (pid < 0)
      return -1;
    if (pid == 0) {
      for (int l = 0; l < total_levels; l++) {
        int role_bit = (i & (1 << (total_levels - l - 1))) >> (total_levels - l - 1);
        int lockl = i >> (total_levels - l);
        int lock_array_index = lockl + ((1 << l) - 1);
        tournament_lock_ids[l] = lock_ids[lock_array_index];
        tournament_roles[l] = role_bit;
      }
      return i;
    }
  }

  // parent process waits for all children to finish
  for (int i = 0; i < processes; i++)
    wait(0);

  exit(0);
}

int tournament_acquire(void) {
  for (int l = total_levels - 1; l >= 0; l--) {
    if (peterson_acquire(tournament_lock_ids[l], tournament_roles[l]) < 0)
      return -1;
  }
  return 0;
}

int tournament_release(void) {
  for (int l = 0; l < total_levels; l++) {
    if (peterson_release(tournament_lock_ids[l], tournament_roles[l]) < 0)
      return -1;
  }
  return 0;
}