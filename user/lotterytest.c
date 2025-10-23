#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid;
  int nprocs = 3;
  int tickets[3] = {10, 20, 30};

  printf("Iniciando prueba de Lottery Scheduling...\n");

  // Crear procesos hijos
  for (int i = 0; i < nprocs; i++) {
    pid = fork();
    if (pid == 0) {
      // Proceso hijo
      settickets(tickets[i]);
      for (volatile int j = 0; j < 100000000; j++); // simula CPU work
      exit(tickets[i]); // el hijo sale retornando su número de tickets
    }
  }

  // Esperar a que terminen los hijos y mostrar resultados en orden limpio
  for (int i = 0; i < nprocs; i++) {
    int status;
    int pid_ended = wait(&status);
    printf("Proceso %d con %d tickets finaliza\n", pid_ended, status);
  }

  printf("Prueba finalizada.\n");
  exit(0);
}
  
