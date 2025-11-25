Tarea 3 — Sistemas Operativos
Implementación de mrdprotect y munrdprotect en xv6-riscv
Antonia Paz Zárate Córdova
1. Introducción

El objetivo de esta tarea fue modificar el kernel del sistema operativo educativo xv6-riscv para implementar dos nuevos llamados al sistema:

mrdprotect(void *addr, int len)

munrdprotect(void *addr, int len)

Ambos permiten controlar los permisos de lectura de páginas de memoria del espacio de usuario, manipulando directamente los Page Table Entries (PTE) asociados a un proceso.

Además, se creó un programa de usuario (rdprotect_test.c) para validar el funcionamiento correcto de las modificaciones.

2. Archivos modificados

Durante el desarrollo de la tarea se modificaron los siguientes archivos del kernel y del user space:

Kernel

kernel/syscall.h

kernel/syscall.c

kernel/sysproc.c

kernel/vm.c

kernel/defs.h

User space

user/user.h

user/usys.pl

user/rdprotect_test.c

Makefile (UPROGS)

3. Implementación de los nuevos syscalls
3.1. Cambios en syscall.h

Se agregaron los números de los nuevos syscalls de forma consecutiva al último existente:

#define SYS_mrdprotect XX
#define SYS_munrdprotect YY

3.2. Cambios en syscall.c

Se agregaron los prototipos:

extern uint64 sys_mrdprotect(void);
extern uint64 sys_munrdprotect(void);


Y se agregaron ambas funciones a la tabla de syscalls:

[SYS_mrdprotect]  sys_mrdprotect,
[SYS_munrdprotect] sys_munrdprotect,

3.3. Funciones puente en sysproc.c

Aquí ocurren dos cosas importantes:

El espacio de usuario pasa los parámetros a nivel kernel.

Las llamadas se delegan a las funciones reales implementadas en vm.c.

En este archivo ocurrió uno de los errores durante el desarrollo, porque en la versión de xv6 utilizada argaddr y argint retornan void, por lo que no pueden evaluarse en un if (...) < 0.
El error original fue:

error: void value not ignored as it ought to be


La solución fue separar la lectura de argumentos en dos pasos:

argaddr(0, &addr);
argint(1, &len);

if(len < 0)
    return -1;


El código final quedó así:

uint64
sys_mrdprotect(void)
{
    uint64 addr;
    int len;

    argaddr(0, &addr);
    argint(1, &len);

    if(len < 0)
        return -1;

    return mrdprotect((void*)addr, len);
}

uint64
sys_munrdprotect(void)
{
    uint64 addr;
    int len;

    argaddr(0, &addr);
    argint(1, &len);

    if(len < 0)
        return -1;

    return munrdprotect((void*)addr, len);
}

3.4. Implementación en vm.c

Estas funciones modifican los bits del PTE asociados a una página del espacio de usuario.

En mrdprotect, se elimina el permiso de lectura:

*pte &= ~PTE_R;


En munrdprotect, se restaura:

*pte |= PTE_R;


Se verifican todos los casos de error solicitados en la pauta:

len <= 0

addr no está alineada a página

La página no existe

No pertenece al espacio de usuario

El PTE no está marcado como válido

Finalmente se ejecuta:

sfence_vma();


Para asegurar que el TLB se actualice.

4. Programa de prueba: rdprotect_test.c

Este programa:

Reserva una página con sbrk(4096)

Escribe un valor

Llama a mrdprotect(addr, 1)

Intenta leer nuevamente la página → esto debe generar un trap

(Opcionalmente) se puede restaurar usando munrdprotect

El comportamiento esperado es que xv6 muera con un trap del tipo:

usertrap(): unexpected scause 0xf pid=X


Y vuelva al prompt del shell.

5. Problemas encontrados y soluciones
5.1. Error en sysproc.c al usar argaddr y argint

Error visto:

void value not ignored as it ought to be


Causa:

La versión de xv6 utilizada tiene argaddr y argint con retorno void.

Solución:

Separar la lectura y la verificación de valores.

5.2. Error al compilar por una línea basura en sysproc.c

En una edición previa quedó una línea corrupta:

sys_mrdprotect(void)uint64


Esto generó errores como:

expected '=', ',', ';' before '{' token


Solución:

Eliminar la línea errónea.

5.3. Error en Makefile: _rdprotect_test sin $U/

Error mostrado:

make: *** No rule to make target `_rdprotect_test', needed by `fs.img'. Stop.


Causa:

Entrada mal escrita en el bloque UPROGS.

Solución:

Agregar correctamente:

$U/_rdprotect_test\

6. Pruebas realizadas

Luego de realizar make clean && make, se ejecutó:

make qemu


Dentro de xv6:

rdprotect_test


Salida observada:

usertrap(): unexpected scause 0xf pid=3


Este resultado confirma que la página quedó sin permisos de lectura y que el trap se produjo tal como se esperaba.

7. Conclusión

La implementación de los syscalls mrdprotect y munrdprotect fue exitosa.
Se añadieron los permisos de manipulación de lectura a nivel de PTE, se crearon las rutas de syscall completas, y se validó el funcionamiento utilizando un programa de pruebas en user space.

Los errores encontrados durante el desarrollo fueron corregidos y se documentaron en este informe.

El sistema opera de acuerdo con lo solicitado en la pauta de la tarea.
