# Instrucciones para agentes (Claude Code, Kiro, Cline, Cursor…)

Descompilación del Halo 2 original de Xbox, basada en
cybersecurity/halo-ce-universal. Antes de empezar, leer `ESTADO.md` (dónde
está el proyecto y siguientes pasos) y la última entrada de `CHANGELOG.md`
(qué funciones tienen pendiente la comprobación de romper líneas).

## Forma de trabajar (la pidió el usuario)

- **No preguntar qué hacer**: encadenar hitos uno tras otro hasta el límite
  de uso. No terminar el turno para hacer resúmenes; parar solo ante un
  bloqueo real.
- **No instalar nada.** No desactivar Smart App Control: puede bloquear
  ejecutables compilados localmente; las herramientas nuevas se escriben en
  Python.
- **Cada hito** = un grupo de funciones (varias si son pequeñas):
  1. `python tools\disassemble.py 0x<dirección>`; buscar el nombre en
     `..\OneDrive\Escritorio\halo-ce-universal\source\` (sin copiar código).
  2. Escribir la función en `source\<sistema>\`, apuntarla en
     `symbols\functions.csv` (variables en `symbols\globals.csv`).
  3. Casos de prueba en `tests\`; pasar SOLO las funciones nuevas:
     `python tools\test_functions.py test_<archivo> <función> ...`
  4. `python tools\milestone_docs.py` → actualizar `README.md`, `ESTADO.md`
     y una entrada en español en `CHANGELOG.md` (incluida la lista de
     funciones con romper líneas pendiente).
  5. Commit (mensaje en inglés, estilo "Decompile twenty small functions…").
  6. Decir en el chat **una línea** con el número de funciones y el % (de
     funciones y de bytes). Seguir con el siguiente hito.
- **Comprobaciones largas por lotes y en segundo plano**: la batería completa
  (`python tools\test_functions.py`, >10 min) y las de romper líneas
  (`tools\mutation_check.py`, reescribe el fuente mientras corre) se ejecutan
  cada ~10 funciones o en grandes avances, en el worktree aparte
  `C:\Users\steve\halo-2-check` (hacer checkout del commit allí primero).
  Nunca esperar parado: seguir descompilando; cuando llegue el resultado,
  terminar el hito en curso y luego mirar el resultado. Los fallos se
  arreglan en un hito de corrección.

## Trampas conocidas

- **Compilador**: las pruebas necesitan el clang 19.1.5 de VS Build Tools
  primero en el PATH; clang 22/23 rechazan `game.c` y `simulation_view.c`.
  PowerShell: `$env:PATH = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin;$env:PATH"`
- **Nombres de bibliotecas**: antes de llamar `unknown_XXXXXXXX` a una
  llamada por encima de ~0x320000, buscarla en `ghidra\functions.csv` (tiene
  XNet y la biblioteca de C con su nombre real).
- **Dónde buscar trabajo**: `python build\line_breaking\find_easy.py 120 > build\line_breaking\easy_120.txt`
  lista funciones pequeñas sin llamadas indirectas; elegir grupos por
  dirección.
- **Nunca reescribir enteros `symbols\functions.csv` ni `symbols\globals.csv`**
  (ni ordenarlos, ni con `csv.DictWriter`): solo añadir filas al final, con
  el mismo formato (tipo de retorno siempre escrito: `void`, `pointer`…).
  Kiro borró así tres globales en el hito 738.
- Una tabla global se da en la prueba como sus bytes (`GlobalData(bytes)`) y
  en C se declara como array de su tamaño. Un `Buffer` como valor de un
  global pone solo un puntero y la prueba no comprueba nada.
- Sin `python -c` ni `cd ...;` delante: scripts en `build\scratch\*.py`.
- Un global de UN byte en una prueba se da como `bytes((v,))`, no como int.
- Constante de 2 bytes a 4 de su vecina: declararla de 4 (`long x = 0xFFFF`).
- Calcular la dirección de cada salto cerca del final de la función antes de
  decidir qué rama devuelve TRUE; recontar offsets de pila tras cada push.
- Funciones saltadas (necesitan tablas estáticas o callees desconocidos):
  ver la lista en la última entrada de `CHANGELOG.md` / `ESTADO.md`.
