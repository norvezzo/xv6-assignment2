#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Invalid number of arguments\n");
    exit(1);
  }

  int n = atoi(argv[1]);

  int id = tournament_create(n);
  if (id < 0) {
    printf("Failed to create tournament lock\n");
    exit(1);
  }

  if (tournament_acquire() < 0) {
    printf("Process %d failed to acquire lock\n", id);
    exit(1);
  }

  // critical section
  printf("PID %d (tournament ID %d) in critical section\n", getpid(), id);

  if (tournament_release() < 0) {
    printf("Process %d failed to release lock\n", id);
    exit(1);
  }

  exit(0);
}