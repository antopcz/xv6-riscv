# INFORME — Tarea 0: Instalación y Ejecución de xv6 (RISC-V)

## Pasos seguidos
1. Se realizó un fork del repositorio oficial `mit-pdos/xv6-riscv` en GitHub, creando:  
   ->  https://github.com/antopcz/xv6-riscv
2. Se clonó el fork localmente:git clone https://github.com/antopcz/xv6-riscv.git
3. Se creó la rama de trabajo: git checkout -b antonia_zarate_t0
4. Se verificó la instalación de dependencias en macOS (QEMU y toolchain RISC-V).
5. Se compiló con `make` y se ejecutó con `make qemu`.
6. En xv6 se probaron los comandos:
- `ls`
- `echo "Hola xv6"`
- `cat README`

## Problemas encontrados y soluciones

- **Clonado inicial fallido**: al intentar clonar el repositorio, aparecía error porque ya existía la carpeta `xv6-riscv`.  
  -> *Solución*: se creó un nuevo fork limpio en GitHub y se clonó directamente desde ahí.

- **Error `exec git failed`**: este mensaje apareció porque se intentó ejecutar `git` dentro de la shell de xv6 (el prompt `$` de xv6), donde no existen esos comandos.  
  -> *Solución*: se cerró QEMU y se usaron los comandos de Git en la terminal de macOS.

- **Rama que no aparecía en GitHub**: después de crear `antonia_zarate_t0` localmente, no aparecía en el repositorio remoto.  
  -> *Solución*: se ejecutó `git push -u origin antonia_zarate_t0` desde la terminal de macOS, vinculando la rama al remoto.

- **Subida de imágenes desde GitHub Web fallida**: al intentar arrastrar las capturas directamente en GitHub, aparecía el error “Something went really wrong…”.  
  -> *Solución*: se usó la terminal para mover los archivos manualmente a la carpeta `docs/` y hacer commit/push desde allí.

- **Imágenes no visibles en el informe (error 404)**: al abrir el `INFORME.md`, las imágenes no se mostraban porque los archivos tenían nombres largos con espacios y paréntesis.  
  -> *Solución*: se renombraron las capturas dentro de `docs/` con nombres cortos (`captura_ls.png`, `captura_echo.png`, `captura_cat.png`), y se actualizaron los enlaces en el informe.

## Confirmación de funcionamiento
xv6 se ejecutó correctamente en QEMU y respondió a los comandos básicos solicitados.  

## Evidencia
Capturas de xv6 funcionando en QEMU:  

- `ls`: ![ls](docs/captura_ls.png)  
- `echo "Hola xv6"`: ![echo](docs/captura_echo.png)  
- `cat README`: ![cat](docs/captura_cat.png)

