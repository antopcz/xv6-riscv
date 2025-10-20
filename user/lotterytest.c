#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid;
  int nprocs = 3;

  printf("Iniciando prueba de Lottery Scheduling...\n");

  for(int i = 0; i < nprocs; i++){
    pid = fork();
    if(pid == 0){ // proceso hijo
      int tickets = (i + 1) * 10;
      settickets(tickets);
      for(int j = 0; j < 100000000; j++); // simula trabajo
      printf("Proceso %d con %d tickets finaliza\n", getpid(), tickets);
      exit(0);
    }
  }

  for(int i = 0; i < nprocs; i++){
    wait(0);
  }

  printf("Prueba finalizada.\n");
  exit(0);
}

