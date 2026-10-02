# Estado del proyecto

Lista de lo hecho y lo que falta, para retomar otro día. La historia con
detalle está en [CHANGELOG.md](CHANGELOG.md); aquí solo el resumen.

**Última actualización:** 2026-10-01, al terminar el hito 33.

## Dónde estamos

- **301 funciones descompiladas de 11.265: el 2,67 % del juego** (el
  1,254 % de los bytes de código).
- Las 301 están comprobadas contra el ejecutable original
  (`python tools\test_functions.py`).
- Último hito: *Decompile the first half of the texture cache* (ver `git log`).
  El árbol de trabajo está limpio.
- Además hay 280 funciones apuntadas pero sin descompilar (las que el código
  llama); 237 de ellas solo se conocen por su dirección (`unknown_...`).

## Cómo retomar

1. Abrir una terminal en `C:\Users\steve\halo-2-universal`.
2. Comprobar que todo sigue pasando:
   `python tools\test_functions.py` (tarda un rato; al final dice cuántas
   funciones hay y cuántas pasan).
3. Elegir un punto de «Siguiente paso» de más abajo.
4. Para cada función, el ciclo es siempre el mismo:
   - leer su desensamblado: `python tools\disassemble.py 0x<dirección>`;
   - buscar su equivalente en Halo 1 (`halo-ce-universal\source\`) para el
     nombre, sin copiar código;
   - escribirla en `source\<sistema>\`;
   - apuntarla en `symbols\functions.csv` (y sus variables en
     `symbols\globals.csv`);
   - escribir sus casos en `tests\`;
   - pasar las pruebas;
   - romper líneas a propósito para ver que las pruebas lo notan
     (`python tools\mutation_check.py tests\mutations_example.py`; la lista
     de líneas a romper se escribe a mano, como en ese ejemplo);
   - anotar en `CHANGELOG.md`, actualizar el número del `README.md` y este
     archivo, y hacer commit.

## Hecho

### Datos del disco (opción 1, terminada)

- [x] Leer los mapas: cabecera, índice de tags (`tools\mapinfo.py`).
- [x] Geometría de colisión de los niveles (`tools\bsp2obj.py --collision`).
- [x] Geometría que se dibuja, con coordenadas de textura.
- [x] Texturas (DXT1/3/5 y formatos sin comprimir) a PNG.
- [x] Modelos (`tools\model2obj.py`).
- [x] Colocación de los objetos en los niveles.
- [x] Todo comprobado en los 30 mapas (`python tools\test_maps.py <carpeta>`).

### Código del ejecutable (opción 2, en curso)

Herramientas:

- [x] Arnés de pruebas: ejecuta la función del juego y la nuestra en un
      emulador y compara resultados, memoria y llamadas
      (`tools\difftest.py`, `tools\test_functions.py`).
- [x] Pseudocódigo de Ghidra de las 11.265 funciones (`ghidra\decompiled\`,
      no está en git) e índice para buscar (`tools\function_index.py`).
- [x] Desensamblador (`tools\disassemble.py`).
- [x] Comprobación por mutación (`tools\mutation_check.py`).

Módulos (funciones hechas / archivo):

- [x] Tablas de datos — `memory\data.c` (20, módulo completo)
- [x] CRC de 32 bits — `memory\crc.c` (2, completo)
- [x] Búsqueda binaria — `memory\binary_search.c` (1)
- [x] Caché de bloques de páginas — `memory\lruv_cache.c` (8, completo)
- [x] Caché de animaciones — `cache\xbox_animation_cache.c` (18, completo)
- [ ] Caché de texturas — `cache\xbox_texture_cache.c` (17; falta la carga
      de un bitmap)
- [x] Cadenas anchas — `text\unicode.c` (7; quedan 3)
- [x] Flujo de bits de red — `networking\bitstream.c` (5; quedan 2)
- [ ] Matemáticas reales — `math\real_math.c` (21 de unas 50)
- [ ] Matrices — `math\matrix_math.c` (22; quedan 4)
- [ ] Números aleatorios — `math\random_math.c` (2)
- [ ] Colores y bitmaps — `bitmaps\` (6)
- [ ] BSP 3D — `physics\bsp3d.c` (2)
- [ ] Objetos — `objects\objects.c` (7 de cientos)
- [x] Hilo de tareas y tareas de archivo — `cseries\async.c` (29; quedan
      algunas tareas de archivo)
- [x] Archivos de caché — `cache\cache_files.c` (46):
  - [x] acceso a los tags y carga de un mapa
  - [x] arranque (`cache_files_initialize`) y apertura de las copias
  - [x] copia de mapas al disco duro (máquina de 20 pasos)
- [x] Archivos del estado del juego — `saved games\game_state_xbox.c` (7,
      completo)
- [ ] Estado del juego y puntos de control — `saved games\game_state.c`
      (17; falta guardar/cargar núcleo y partida guardada)
- [x] Arranque del juego — `shell\shell.c` (7)
- [x] Preferencias guardadas — `main\global_preferences.c` (13; quedan 2
      que leen nombres)
- [ ] Juego y tabla de sistemas — `game\game.c` (19): la tabla, lo que la
      recorre, las opciones (por defecto, validar, comparar), los ticks de
      un fotograma, partida perdida y terminada, y el tipo de simulación
- [ ] Bucle principal — `main\main.c` (12):
  - [x] el bucle, un fotograma y el reloj
  - [x] guardar, cambiar de bsp y avisar de que se guardó
  - [ ] la acción `0x0012bbd0` (pantallas de la interfaz)
- [ ] Escenario — `scenario\scenario.c` (1): cambiar de bsp
- [ ] Memoria física — `memory\physical_memory.c` (1)
- [ ] Varios — `cseries\cseries.c` (10)

## Siguiente paso (por orden de recomendación)

- [x] ~~Acciones del bucle: guardar, cambiar de bsp y `0x0012bdf0`~~ (hito
      26; queda `0x0012bbd0`).
- [x] ~~`scenario_switch_structure_bsp`~~ (hito 26).
- [ ] **Rellenar filas de la tabla de sistemas** (`game_systems`,
      `0x00440dd8`, en `game\game.c`): son 68 sistemas y 166 funciones, de
      las que hay 5. Cada módulo que se descompile rellena su fila. Es el
      mapa de carreteras del resto del proyecto.
- [ ] **Los sistemas que prepara `shell_initialize`** y que no están:
      `0x00121fc0`, `0x00012560` (rasterizador),
      `0x001248b0`, `0x001253d0`. (Las preferencias, `0x00121350`, ya
      están: hito 29.)
- [x] ~~Opciones de partida: validar y comparar~~ (hito 28).
- [ ] **Resto de `game_state.c`:** núcleo y partida guardada (`0x001240b0` a
      `0x00124490`, `0x00124640`, `0x00124770` en adelante).
- [x] ~~`lruv_block_new`~~ (hito 31).
- [x] ~~Caché de animaciones~~ (hito 32).
- [x] ~~Caché de texturas, primera parte~~ (hito 33): crear, abrir,
      cerrar, cada fotograma, lo que le da a `lruv_cache` y la memoria
      prestada.
- [ ] **Caché de texturas, segunda parte:** la carga de un bitmap. Están
      leídas y con borrador: `0x0012c450`, `0x0012c640`, `0x0012c820`,
      `0x0012c960`, `0x0012c990`, `0x0012cb10`, `0x0012cb80`, `0x0012ccb0`,
      `0x0012ccf0`, `0x0012ce00` (en Halo 1,
      `_texture_cache_bitmap_get_hardware_format`), `0x0012d5a0`,
      `0x0012d6d0`, y `texture_cache_steal_memory` (`0x0012d400`, que mide
      el tiempo con `rdtsc` y `0x00223bb0`). Hacen falta `0x000359e0`
      (cuántos mipmaps) y `0x001365a0` (bytes por fila), y las tablas de
      formatos de `0x00450768` y `0x004507c8`.
- [ ] **Caché de geometría** (`0x0012d8b0` en adelante): el mismo esquema
      que las otras dos cachés.
- [ ] **La tabla de `0x00453c00`**: ocho pares de funciones (soltar y
      tomar memoria) de los que usan memoria al cargar un mapa; el par de
      la caché de animaciones ya está. La recorren `0x0018ec60` y
      `0x0018ed00`, que abren y cierran un mapa.
- [ ] **Resto de las tareas de archivo:** `0x001a0a70`, `0x001a0da0`,
      `0x001a1480`, `0x001a17d0`.
- [ ] **Matemáticas que quedan:** `real_math.c` (`0x0011cf50`, `0x0011d090`,
      `0x0011d790`, `0x0011e000` y las grandes) y las 4 de matrices.

## Lo que falta en grande

- [ ] El 98 % del código: unas 11.000 funciones. Los sistemas grandes ni
      están empezados: objetos, unidades, IA, física (Havok va dentro del
      ejecutable), renderizado, sonido, red, interfaz, scripts.
- [ ] Decidir qué hacer con Havok y las bibliotecas de la consola (unas
      2.600 funciones que no son código del juego).
- [ ] Un ejecutable propio que arranque. Aún no hay ninguno: el control de
      aplicaciones de Windows bloqueó los programas compilados aquí, por eso
      todas las herramientas están en Python.

## Avisos para no perder tiempo

- No hace falta esperar a comprobaciones largas, pero una mutación que deja
  un bucle sin fin se corta sola a las 10.000 llamadas simuladas.
- El compilador de `source\` (clang) quita cosas que el del juego no quitó:
  argumentos que la función llamada no usa, y comprobaciones de funciones
  que siempre devuelven lo mismo. Eso no se puede comparar; se anota en el
  CHANGELOG bajo «Sin comprobar».
- Una función del juego que aún no está descompilada se declara en
  `source\` sin `stdcall`, aunque en el juego quite sus argumentos de la
  pila: el arnés lo hace por ella.
- Los mapas del juego están en
  `E:\Datos\Downloads\juegos\roms\xbox\Halo 2`; no se copian ni se tocan.
