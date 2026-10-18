# Instrucciones para agentes (Claude Code, Kiro, Cline, Cursor…)

Descompilación del Halo 2 original de Xbox, basada en
cybersecurity/halo-ce-universal. Antes de empezar, leer `ESTADO.md` (dónde
está el proyecto y siguientes pasos) y la última entrada de `CHANGELOG.md`.

## REGLAS OBLIGATORIAS

Romper una de estas reglas ya costó un hito entero de correcciones (el 738,
que empezó Kiro: 9 de sus 17 funciones estaban mal aunque sus pruebas decían
«ok»).

1. **No tocar lo que ya existe en `symbols\functions.csv` ni en
   `symbols\globals.csv`.** Solo se AÑADEN filas al final, a mano o con un
   script que abra el archivo en modo añadir (`"a"`). Prohibido leerlos y
   volver a escribirlos enteros (`csv.DictWriter`, ordenar, filtrar por
   dirección): así se borraron `main_vblank_count`, `xbox_video_settings` y
   `main_game_globals`.
2. **Formato de las filas de funciones:**
   `dirección,nombre,source/<archivo>.c,argumentos,retorno[,stdcall]`.
   El retorno se escribe siempre y SOLO con un tipo que el arnés entiende:
   `void`, `int`, `bool`, `byte`, `short`, `pointer`, `pair`, `string`,
   `wstring`, `real`, `real:eax`, `real:xmm0`. **Con cualquier otro (`long`,
   `ptr`, vacío…) el valor devuelto NO se compara** y la prueba pasa
   aunque la función devuelva basura: le pasó a cuatro funciones de Kiro.
   Lo que devuelve un puntero a una tabla global es `pointer`, y esa tabla
   va en la prueba como `GlobalData`.
3. **Leer el final de la función antes de escribirla.** Si acaba en
   `ret N`, saca N bytes de la pila: la fila lleva `stdcall` y el C
   `__attribute__((stdcall))`. Los argumentos van en el orden en que los
   USA el código (`[esp+4]` es el primero de la pila después de contar cada
   `push`), no en el que se supone.
4. **Una variable global que es una tabla se declara en C como array de su
   tamaño** (el hueco hasta la siguiente variable conocida) y en la prueba
   se da con sus bytes: `GlobalData(bytes)` (o `bytes` si nada apunta a
   ella). **Nunca un `Buffer` como valor de un global**: eso escribe solo un
   puntero de 4 bytes y la prueba no compara la tabla.
5. **Si una variable ya existe con otro nombre, se usa esa** (buscar la
   dirección en `symbols\globals.csv` antes de crear una).
6. **Las pruebas tienen que poder fallar.** Antes del commit, romper a
   propósito una línea de cada función y ver que la prueba falla
   (`tests\mutations_738.py` es un ejemplo:
   `python tools\mutation_check.py tests\mutations_<hito>.py`). Si una
   mutación sobrevive, la prueba no sirve: arreglarla.
7. **No instalar nada. No desactivar Smart App Control.** Las herramientas
   nuevas se escriben en Python.
8. **Comandos:** sin `python -c "..."` y sin `cd ...;` delante (la terminal
   ya está en la carpeta del repo). Scripts auxiliares en
   `build\scratch\<nombre>.py`.
9. **Compilador:** antes de las pruebas, el clang 19.1.5 de Build Tools
   primero en el PATH. Con clang 22/23 fallan `game.c` y
   `simulation_view.c`: NO es un error del hito, no tocar esos archivos.
   PowerShell:
   `$env:PATH = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin;$env:PATH"`
10. **Si te atascas** (la misma prueba falla 3 veces, o un comando no da
    resultado), para y explica qué pasa. No reintentes lo mismo en bucle ni
    cambies archivos que no son del hito para que pase.

## Forma de trabajar (la pidió el usuario)

- **No preguntar qué hacer**: encadenar hitos uno tras otro hasta el límite
  de uso. No terminar el turno para hacer resúmenes; parar solo ante un
  bloqueo real.
- **Cada hito** = un grupo de funciones (varias si son pequeñas):
  1. `python tools\disassemble.py 0x<dirección>`; buscar el nombre en
     `C:\Users\steve\OneDrive\Escritorio\halo-ce-universal\source\` (sin
     copiar código).
  2. Escribir la función en `source\<sistema>\`; añadir su fila a
     `symbols\functions.csv` (y sus variables a `symbols\globals.csv`).
  3. Casos de prueba en `tests\`; pasar SOLO las funciones nuevas:
     `python tools\test_functions.py test_<archivo> <función> ...`
  4. Mutaciones de sus líneas (regla 6).
  5. `python tools\milestone_docs.py build\scratch\spec_<hito>.py`
     actualiza `README.md`, `ESTADO.md` y añade la entrada en español a
     `CHANGELOG.md` (el formato del spec está al principio de
     `tools\milestone_docs.py`).
  6. Commit con mensaje en inglés, estilo
     "Decompile seventeen functions of …".
  7. Decir en el chat **una línea** con el número de funciones y el % (de
     funciones y de bytes) que imprime `milestone_docs.py`. Seguir con el
     siguiente hito.
- **La batería completa** (`python tools\test_functions.py`, más de 10
  min) solo en grandes avances, en segundo plano, en el worktree aparte
  `C:\Users\steve\halo-2-check` (hacer checkout del commit allí primero).
  No esperar parado.

## Lista antes de cada commit

- [ ] `git diff --stat symbols\` solo muestra líneas AÑADIDAS (`+`), del
      orden del número de funciones y variables nuevas. Si hay miles de
      cambios o líneas borradas, algo reescribió el archivo: deshacerlo.
- [ ] Las pruebas de las funciones nuevas pasan con el clang 19.1.5.
- [ ] Ninguna mutación sobrevive.
- [ ] Ninguna tabla global se da como `Buffer` en las pruebas.
- [ ] Ningún tipo de retorno fuera de la lista de la regla 2
      (`grep ",long" symbols\functions.csv` no da nada nuevo).
- [ ] `README.md`, `ESTADO.md` y `CHANGELOG.md` actualizados.

## Trampas conocidas

- **Nombres de bibliotecas**: antes de llamar `unknown_XXXXXXXX` a una
  llamada por encima de ~0x320000, buscarla en `ghidra\functions.csv` (tiene
  XNet y la biblioteca de C con su nombre real).
- **Dónde buscar trabajo**:
  `python build\line_breaking\find_easy.py 120 > build\line_breaking\easy_120.txt`
  lista funciones pequeñas sin llamadas indirectas; elegir grupos por
  dirección.
- `or reg, 0x80000000` seguido de `je`: el salto nunca se toma (el resultado
  nunca es 0); no inventar una comprobación de nulo.
- `neg dl; sbb edx, edx; and eax, edx` = deja `eax` si `dl` NO es cero y lo
  pone a 0 si `dl` es cero.
- Copiar una constante `real` a una variable `long` con `mov` es copiar sus
  bits, no convertirla: declarar las dos como `long`.
- En C, `&variable_long + n` avanza `n * 4` bytes: indexar el array
  (`&tabla[n]`), no sumar bytes a mano sobre un `long`.
- Un global de UN byte en una prueba se da como `bytes((v,))`, no como int.
- Constante de 2 bytes a 4 de su vecina: declararla de 4 (`long x = 0xFFFF`).
- Calcular la dirección de cada salto cerca del final de la función antes de
  decidir qué rama devuelve TRUE; recontar offsets de pila tras cada push.
- Funciones saltadas (necesitan tablas estáticas o callees desconocidos):
  ver la lista en las últimas entradas de `CHANGELOG.md` / `ESTADO.md`.
