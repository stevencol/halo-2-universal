# Instrucciones para agentes (Claude Code, Cursor, opencode, Kiro, Cline…)

Descompilación del Halo 2 original de Xbox, basada en
cybersecurity/halo-ce-universal. Antes de empezar, leer `ESTADO.md` (dónde
está el proyecto y siguientes pasos) y la última entrada de `CHANGELOG.md`.

El método completo (prueba diferencial, mutaciones, cobertura, casos
grabados con el emulador, batería) y las mejoras pendientes están en
`docs/metodo-port/` (`README.md`, `requirements.md`, `design.md`,
`tasks.md`). Las reglas de abajo mandan sobre el spec.

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
  4. Mutaciones de sus líneas (regla 6) en `tests\mutations_<hito>.py`, y
     `python tools\check_milestone.py <hito> --run` hasta que diga
     `TODO BIEN` (ver «Lista antes de cada commit»).
  5. `python tools\milestone_docs.py build\scratch\spec_<hito>.py`
     actualiza `README.md`, `ESTADO.md` y añade la entrada en español a
     `CHANGELOG.md` (el formato del spec está al principio de
     `tools\milestone_docs.py`).
  6. Commit con mensaje en inglés, estilo
     "Decompile seventeen functions of …".
  7. Decir en el chat **una línea** con el número de funciones y el % (de
     funciones y de bytes) que imprime `milestone_docs.py`. Seguir con el
     siguiente hito. Ese número cuenta solo las funciones que están en
     `ghidra\functions.csv` (las 11.265); el total de descompiladas, que
     incluye las que Ghidra no separa, es otro número (`{total}` en el
     spec). No sumar las dos cosas.
- **La batería completa** (`python tools\test_functions.py`, más de 10
  min) solo en grandes avances, en segundo plano, en el worktree aparte
  `C:\Users\steve\halo-2-check` (hacer checkout del commit allí primero).
  No esperar parado.

## Lista antes de cada commit

**Obligatorio: `tools\check_milestone.py`.** Comprueba casi todo lo de
arriba solo: filas de los CSV solo añadidas y con su formato, variables
repetidas, tipos de retorno, `ret N` = `stdcall`, ensamblador, pruebas con
la forma del arnés, tablas dadas como `Buffer`, y las mutaciones. Con
`--run` además pasa las pruebas y las mutaciones con el clang 19.1.5.

```
python tools\check_milestone.py <hito> --run > build\scratch\salida.txt 2>&1
```

Después LEER `build\scratch\check_milestone.txt` (el script lo escribe
siempre: así da igual que la terminal no muestre la salida). Tiene que
acabar en `TODO BIEN`. Si dice `ERROR`, corregir lo que dice y volver a
pasarlo. **Nunca hacer commit con errores** ni decir que pasa sin haber
leído ese archivo.

**Plantillas:** copiar la forma de los hitos 741 a 748:
`tests\test_small_741.py` … `tests\test_small_748.py` (pruebas),
`tests\mutations_741.py` … `tests\mutations_748.py` (mutaciones), y sus
funciones en `source\networking\network_life_cycle.c`,
`network_recent_players.c` u `online_statistics.c` (estilo del C).
Ejemplos útiles: funciones de otros simuladas en la prueba
(`test_small_743.py`, `745`), variables globales que son tablas
(`744`, `746`), un flujo de bits (`748`) y un spec de documentación
(`build\scratch\spec_748.py`, si existe; si no, el formato está al
principio de `tools\milestone_docs.py`).

Además, a mano:

- [ ] `README.md`, `ESTADO.md` y `CHANGELOG.md` actualizados.
- [ ] Si te lo han pedido (Kiro: siempre, de momento), NO hacer el commit:
      dejarlo para que se revise.

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

### Aprendido en los hitos 742–748

- **Saltos de línea.** `symbols\functions.csv`, `symbols\globals.csv` y
  algunos `.c` (por ejemplo `online_tasks.c`) usan CRLF. Las filas nuevas
  tienen que acabar en `\r\n` como las demás (en Git Bash:
  `printf '%s\r\n' "fila1" "fila2" >> symbols/functions.csv`). Comprobar
  con `file <archivo>`: no debe decir «with CRLF, LF» (mezclado).
- **Simular funciones en las pruebas.** `externals={...}` simula
  cualquier función de `symbols\functions.csv`, también las ya
  descompiladas (`online_task_close`, `network_session_leave`…): el
  handler devuelve `(valor, tupla_de_argumentos_a_comparar)`. Úsalo cuando
  la función real haría demasiado (tareas, sesiones enteras). Las de la
  biblioteca (`GetTickCount`, `time`, `XNetUnregisterKey`) siempre se
  simulan así.
- **Punteros en los argumentos de una simulación.** Un puntero a un
  `Buffer` se compara con `harness.normalize(arguments[i])`. Un puntero a
  una variable global NO: se compara su posición dentro de ella,
  `arguments[i] - harness.global_addresses["nombre"]`
  (`test_small_745.py`). Una cadena de formato o un búfer en la pila
  (`csprintf256`) no se puede comparar: comparar solo los números
  (`(0, (arguments[2],))`).
- **Bucles con enlaces.** Si la función recorre una lista enlazada, la
  prueba tiene que construir listas SIN ciclos (`test_small_747.py`); si no,
  la función original no termina.
- **Mutaciones que no se pueden detectar.** Si una línea no cambia nunca el
  resultado (por ejemplo, una comprobación que el bucle ya hace de otra
  forma), quitar esa mutación de la lista y ANOTARLO en la sección «Sin
  comprobar» del `CHANGELOG.md`, explicando por qué. Nunca dejar una
  mutación que sobrevive.
- **Mutaciones que sobreviven por falta de casos.** Añadir casos dirigidos
  al límite: valores iguales al tope, el número justo en la mitad, el
  flujo que acaba exactamente en su final (`test_small_744.py`,
  `test_small_748.py`).
- **Mutación con texto repetido.** El texto de una mutación tiene que estar
  una sola vez en el archivo: si se repite, añadir la línea siguiente con
  `\n` (`mutations_748.py`).
- **Una mutación que deja un bucle infinito** tarda 300 s en darse por
  detectada: es normal, `check_milestone.py` puede tardar 6 minutos.
- **`tools\mutation_check.py` reescribe los `.c` con LF** al terminar: es
  inofensivo (Git usa `autocrlf`), no «arreglarlo».
- **Escribir archivos desde la terminal.** En un heredoc de Bash o en un
  `python -` con texto entre comillas, `\t`, `\m`, `\d` se convierten o
  dan avisos: un `tools\test_functions.py` acabó con un tabulador. Para
  textos con barras invertidas, usar la herramienta de edición de
  archivos o cadenas `r"..."`, y releer lo escrito.
- **`milestone_docs.py` y el `ESTADO` del spec:** los pares de `ESTADO`
  se aplican DESPUÉS de que la herramienta cambie el número de hito; no
  usar el número viejo en ellos.
- **Errores de `online_task_continue`:** `ONLINE_TASK_UNKNOWN80155100`
  (`online_tasks.h`) es un error que `online_task_failed` no cuenta.
