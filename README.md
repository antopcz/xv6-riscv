# Informe – Implementación de Lottery Scheduling en XV6-RISCV

**Autora:** Antonia Paz Zárate Córdova  
**Curso:** Sistemas Operativos – Universidad Adolfo Ibáñez  
**Sistema:** xv6-riscv  
**Fecha:** Octubre 2025  

---

## 1. Funcionamiento y lógica de la implementación (2 pts)

El objetivo de esta tarea fue reemplazar el algoritmo de planificación **Round Robin** del sistema operativo XV6 por un algoritmo de planificación por lotería (**Lottery Scheduling**).  
En este sistema, cada proceso posee una cierta cantidad de *tickets* que representan su probabilidad de ser elegido para ejecutarse. Cuantos más tickets tiene un proceso, mayor es la fracción de CPU que recibirá a largo plazo.

El funcionamiento general del scheduler implementado es el siguiente:

1. Se suman los tickets de todos los procesos en estado **RUNNABLE** para obtener el total de tickets del sistema.  
2. Se genera un número aleatorio dentro del rango `[0, total_tickets)`.  
3. Se recorre la lista de procesos acumulando los tickets hasta que el valor acumulado supera el número ganador.  
4. El proceso correspondiente a ese punto es seleccionado para ejecutarse.  
5. Cada vez que un proceso es elegido, se incrementa su contador `cpu_slices`, que permite llevar la cuenta de cuántas veces fue seleccionado por la CPU.  

Con este mecanismo, la probabilidad de ejecución de un proceso es proporcional a la cantidad de tickets que posee.  
El algoritmo mantiene la concurrencia y evita bloqueos, garantizando que todos los procesos con tickets positivos tengan oportunidad de ejecución.

---

## 2. Explicación de las modificaciones realizadas (1 pt)

Para implementar el sistema de **Lottery Scheduling** fue necesario modificar y agregar código en los siguientes archivos del kernel y del espacio de usuario:

### `kernel/proc.h`
Se agregaron dos nuevos campos a la estructura `proc`:
```c
// --- Lottery Scheduling ---
int tickets;           // cantidad de tickets (>=1)
uint64 cpu_slices;     // veces que este proceso fue elegido por el scheduler

Estos campos permiten asignar una cantidad de tickets a cada proceso y contabilizar cuántas veces fue elegido por el scheduler.

->kernel/syscall.h:
Se agregó la definición de un nuevo número de syscall:
#define SYS_settickets 22
Esto reserva el número 22 para la nueva syscall settickets().

->kernel/syscall.c:
Se registró la nueva syscall en la tabla del sistema:
extern uint64 sys_settickets(void);
static uint64 (*syscalls[])(void) = {
  ...
  [SYS_settickets] sys_settickets,
};
Así, el kernel asocia el número 22 con la función que implementa sys_settickets().

->kernel/sysproc.c:
Se implementó la función del sistema sys_settickets():
uint64
sys_settickets(void)
{
    int n;
    argint(0, &n);
    if (n < 1)
        n = 1;                 // asegura un mínimo de 1 ticket
    struct proc *p = myproc();
    p->tickets = n;            // asigna los tickets al proceso actual
    return 0;
}
Esta función recibe el argumento enviado desde el espacio de usuario y asigna el valor al proceso actual. Si el número es menor que 1, se reemplaza por 1 para evitar bloqueos.

->kernel/proc.c:
Se reemplazó la lógica del scheduler() para implementar la planificación por lotería:
for(;;){
  intr_on();
  struct proc *p;
  struct proc *chosen = 0;
  int total_tickets = 0;

  // Calcular total de tickets
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE && p->tickets > 0)
      total_tickets += p->tickets;
    release(&p->lock);
  }

  if(total_tickets == 0)
    continue;

  // Elegir ticket ganador
  int winner = rand() % total_tickets;
  int acc = 0;

  // Buscar proceso ganador
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == RUNNABLE){
      acc += p->tickets;
      if(acc > winner){
        chosen = p;
        break;
      }
    }
    release(&p->lock);
  }

  // Ejecutar proceso elegido
  if(chosen){
    chosen->state = RUNNING;
    chosen->cpu_slices++;
    c->proc = chosen;
    swtch(&c->context, &chosen->context);
    c->proc = 0;
    release(&chosen->lock);
  }
}
Esta versión realiza un sorteo proporcional a la cantidad de tickets de cada proceso y actualiza el contador cpu_slices cada vez que un proceso es seleccionado.

->user/user.h:
Se agregó la declaración de la función para llamar la syscall desde el espacio de usuario:
int settickets(int);

->user/usys.pl:
Se añadió la línea:
entry("settickets");
Con esto, el script genera el código necesario para invocar la syscall desde programas de usuario.

-> user/lotterytest.c
Se creó un programa de prueba para verificar el funcionamiento del scheduler:
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  int pid;
  int nprocs = 3;
  int tickets[3] = {10, 20, 30};

  printf("Iniciando prueba de Lottery Scheduling...\n");

  for (int i = 0; i < nprocs; i++) {
    pid = fork();
    if (pid == 0) {
      settickets(tickets[i]);
      for (volatile int j = 0; j < 100000000; j++); // simula CPU work
      exit(tickets[i]);
    }
  }

  for (int i = 0; i < nprocs; i++) {
    int status;
    int pid_ended = wait(&status);
    printf("Proceso %d con %d tickets finaliza\n", pid_ended, status);
  }

  printf("Prueba finalizada.\n");
  exit(0);
}
Este programa crea tres procesos, asigna distintos números de tickets a cada uno, simula carga de CPU y muestra el orden de finalización.

->Makefile:
Se añadió el nuevo programa a la lista de binarios de usuario:
UPROGS= ... \
        _lotterytest\

3. Dificultades encontradas y soluciones implementadas:
Durante la implementación se presentaron las siguientes dificultades:

Salida de texto desordenada en QEMU:
Los mensajes printf() de los procesos hijos se mezclaban en la consola.
Solución: Se eliminó la impresión desde los hijos y se delegó al proceso padre, que imprime tras wait().

Procesos con tickets inválidos (0 o negativos):
El scheduler podía quedar bloqueado si existían procesos con 0 tickets.
Solución: Se agregó la validación if (n < 1) n = 1; en sys_settickets().
Bloqueo del scheduler sin procesos RUNNABLE:
El scheduler podía entrar en bucle sin procesos listos.
Solución: Se añadió la verificación if (total_tickets == 0) continue; antes de seleccionar un ganador.
Con estas correcciones, el kernel compila y ejecuta sin errores, manteniendo estabilidad.

4. Posibles problemas de este tipo de Scheduler:
Incertidumbre temporal:
Al depender de un valor aleatorio, los tiempos de respuesta no son deterministas y pueden variar entre ejecuciones.
Equidad estadística no garantizada:
A corto plazo, la distribución de CPU puede no reflejar la proporción exacta de tickets debido al azar.
No apto para sistemas de tiempo real:
No asegura cumplimiento de deadlines ni prioridades fijas.
Sobrecarga de cálculo:
En cada iteración, el scheduler recorre la lista completa de procesos para sumar tickets y determinar el ganador.
Dependencia del generador aleatorio:
Si la función rand() no produce una distribución uniforme, la selección puede sesgarse.
Ausencia de envejecimiento (aging):
Un proceso con pocos tickets podría experimentar inanición prolongada si no se implementa una política de ajuste dinámico.

Conclusión:
La implementación del algoritmo Lottery Scheduling en XV6 fue exitosa.
La syscall settickets() funciona correctamente, el scheduler selecciona procesos de forma proporcional a sus tickets, y el sistema mantiene estabilidad y correcto funcionamiento.
El programa de prueba lotterytest.c demuestra que los procesos con más tickets tienden a recibir más tiempo de CPU, validando el comportamiento probabilístico del planificador.
