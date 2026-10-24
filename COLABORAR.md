# Cómo ayudar: trabajar por hitos

Guía para quien se une al proyecto. Solo explica una cosa: **cómo hacer
hitos**. Las reglas completas están en [AGENTS.md](AGENTS.md); si usas un
asistente de IA (Claude Code, Kiro, Cursor…), dale ese archivo y este.

## Qué es el proyecto

Descompilación a mano del Halo 2 original de Xbox (`halo2ship.exe`, 11.265
funciones según Ghidra). Cada función del ejecutable se reescribe en C en
`source\`, y una prueba ejecuta la original (emulada) y la nuestra con los
mismos datos y compara el resultado. Se avanza **por hitos**: un hito es un
grupo pequeño de funciones (normalmente 4–5) con sus pruebas, su
documentación y un commit.

Dónde va el proyecto: [ESTADO.md](ESTADO.md). Qué se hizo en cada hito:
[CHANGELOG.md](CHANGELOG.md) (lo último arriba).

## Preparar el equipo (una vez)

1. Clonar: `git clone https://github.com/stevencol/halo-2-universal.git`
2. **El juego no está en el repo.** Necesitas tu propia copia de Halo 2 de
   Xbox: copia su `default.xbe` a `disc\`. `disc\` y `ghidra\` nunca se
   suben.
3. Instalar Python 3 y `pip install unicorn capstone`.
4. Compilador: clang **19.1.5** y ld.lld (el componente Clang de Visual
   Studio 2022 Build Tools). Con clang 22/23 fallan `game.c` y
   `simulation_view.c`; no es culpa de tu hito. Ponlo primero en el PATH:
   `$env:PATH = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin;$env:PATH"`
5. Ghidra 12.0.3 + [ghidra-xbe](https://github.com/XboxDev/ghidra-xbe) para
   crear `ghidra\functions.csv` (el comando está en el README, sección de
   Ghidra). Si no quieres montar Ghidra, pídele el archivo a Steven.
6. Opcional pero muy útil: clonar
   [cybersecurity/halo-ce-universal](https://github.com/cybersecurity/halo-ce-universal)
   para buscar nombres de funciones de Halo 1 (solo los nombres; no se
   copia código).
7. Comprobar que todo funciona: `python tools\test_functions.py` (tarda
   más de 10 min; al final dice cuántas pasan).

## Antes de empezar un hito: avisar

Somos dos trabajando en paralelo. Para no pisarnos:

- **Avisa a Steven qué direcciones vas a coger** (por ejemplo
  «0x175000–0x179000») y qué número de hito usarás. Él hace lo mismo.
- Trabaja en una rama: `git switch -c hito-<número>`.
- Antes de empezar: `git pull` en `main`.

## Dónde buscar funciones

- `ESTADO.md`, sección de siguientes pasos.
- `python build\line_breaking\find_easy.py 120` lista las funciones
  pequeñas que aún faltan y son fáciles. **Ese script está en `build\`,
  fuera de git**: pídeselo a Steven.
- Las saltadas a propósito (necesitan tablas o funciones desconocidas)
  están anotadas en las últimas entradas de `CHANGELOG.md`: no las cojas.

## Los pasos de un hito

1. **Leer** la función: `python tools\disassemble.py 0x<dirección>`.
   Mirar el final: si acaba en `ret N`, es `stdcall`.
2. **Escribirla** en C en `source\<sistema>\` (mira el estilo de
   `source\networking\network_life_cycle.c`).
3. **Apuntarla**: añadir su fila AL FINAL de `symbols\functions.csv` (y
   sus variables en `symbols\globals.csv`). Nunca reescribir esos archivos
   enteros. Usan saltos de línea CRLF.
4. **Probarla**: casos en `tests\test_small_<hito>.py` (copia la forma de
   `tests\test_small_741.py` … `748`), y pasar solo las nuevas:
   `python tools\test_functions.py test_small_<hito> <función> ...`
5. **Comprobar que la prueba puede fallar**: en
   `tests\mutations_<hito>.py` rompe a propósito una línea de cada función
   (ejemplo: `tests\mutations_748.py`). Todas las mutaciones tienen que ser
   detectadas.
6. **Revisión automática** (obligatoria):
   `python tools\check_milestone.py <hito> --run`
   y leer `build\scratch\check_milestone.txt`. Tiene que acabar en
   **`TODO BIEN`**. Si dice `ERROR`, corregir y repetir.
7. **Documentar**: escribir `build\scratch\spec_<hito>.py` (el formato está
   al principio de `tools\milestone_docs.py`) y ejecutar
   `python tools\milestone_docs.py build\scratch\spec_<hito>.py`. Eso
   actualiza `README.md`, `ESTADO.md` y añade la entrada en español a
   `CHANGELOG.md`.
8. **Commit** con mensaje en inglés, estilo
   `Decompile five small functions of the effects and the sound`.
9. **Subir y abrir un Pull Request** a `main`:
   `git push -u origin hito-<número>`. Steven lo revisa y lo une. Si mientras
   tanto entró otro hito con el mismo número, cambia el número y vuelve a
   ejecutar el paso 7.

Un hito por commit, y mejor un PR pequeño (1–3 hitos) que uno enorme.

## Lo que más errores ha dado

Resumen; el detalle está en AGENTS.md, «Trampas conocidas».

- Tipo de retorno en `functions.csv` que el arnés no entiende (`long`,
  vacío…): la prueba pasa sin comparar nada. Usa solo `void`, `int`,
  `bool`, `byte`, `short`, `pointer`, `pair`, `string`, `wstring`, `real`,
  `real:eax`, `real:xmm0`.
- Una tabla global dada en la prueba como `Buffer`: escribe solo un
  puntero. Las tablas van como `GlobalData(bytes)`.
- Un global de un byte se da como `bytes((v,))`, no como número.
- Orden de los argumentos: recontar la pila después de cada `push`.
- En C, `0x23E+index` es un solo número (exponente): escribe `index+0x23E`.
- No instalar nada más ni desactivar Smart App Control; las herramientas
  nuevas se escriben en Python.
