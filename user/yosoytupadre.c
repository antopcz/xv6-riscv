#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int me  = getpid();
  int dad = getppid();

  printf("yo (%d), mi padre es (%d)\n", me, dad);

  for (int k = 0; k <= 3; k++) {
    int anc = getancestor(k);
    printf("getancestor(%d) = %d\n", k, anc);
  }

  int pid = fork();
  if (pid == 0) {
    // Proceso hijo
    printf("[hijo] pid=%d, ppid=%d (getppid)\n", getpid(), getppid());
    exit(0);
  }
  wait(0);
  exit(0);
}

