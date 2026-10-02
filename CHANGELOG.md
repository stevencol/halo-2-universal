# Registro de cambios

Los cambios más recientes van arriba.

## 2026-10-01 — Las opciones de una partida

Las dos funciones que deciden si unas opciones de partida se pueden jugar y
si dos opciones son de la misma partida. Eran lo que faltaba para volver a
un punto de control sin funciones simuladas. De 242 funciones se pasa a 244,
el 2,17 % de las del juego (ya el 1,000 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_options_verify` | `0x00138180` | `game/game.c` |
| `game_options_match` | `0x001384a0` | `game/game.c` |

- `tests/test_game_options.py`: opciones buenas que se estropean al azar de
  una o varias maneras (2.500 casos por función).

### Cambiado

- `struct game_options` tiene ya casi todos sus campos: el mapa, la
  dificultad, si es cooperativo, las reglas del multijugador, la tabla de
  16 consolas, esta consola y los 16 jugadores.
- `game_is_campaign_unknown12C` pasa a llamarse `game_is_coop`: el campo
  que no se entendía es «campaña de dos jugadores».

### Lo que se sabe

- Unas opciones valen si: el modo, la simulación, el ritmo de ticks (1 a
  300) y la dificultad están en rango; hay nombre de mapa; no hay dos
  consolas iguales ni dos jugadores iguales; cada jugador tiene mando de 0
  a 3 e identificador distinto de cero, y está en una consola de la partida
  (o en ninguna, con la marca correspondiente); y el número de jugadores
  cuadra: 1 en campaña, 2 en cooperativo, de 1 a 16 en multijugador.
- En los modos 4 y 5 no se mira cuántos jugadores hay ni si esta consola
  está en la partida.
- Dos opciones son de la misma partida si coinciden el mapa, el modo, el
  ritmo de ticks y los mandos de esta consola, y además la dificultad y el
  cooperativo (campaña) o las reglas enteras (multijugador).

### Sin comprobar

- Rechazar un modo de juego negativo al principio de la validación: quitar
  esa comprobación no cambia nada, porque más abajo se rechaza igualmente.

## 2026-10-01 — Once funciones pequeñas del juego

Funciones cortas de `game.c` que el resto del juego consulta a todas horas:
si hay partida, cómo se simula, si se perdió o terminó, las opciones por
defecto y los ticks de un fotograma. De 231 funciones se pasa a 242, el
2,15 % de las del juego (el 0,963 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_options_new_default` | `0x00138110` | `game/game.c` |
| `game_run_ticks` | `0x00137f70` | `game/game.c` |
| `game_in_progress` | `0x00138800` | `game/game.c` |
| `game_is_campaign_unknown12C` | `0x00138820` | `game/game.c` |
| `game_is_distributed` | `0x00138840` | `game/game.c` |
| `game_is_authoritative` | `0x00138860` | `game/game.c` |
| `game_is_predicted` | `0x00138880` | `game/game.c` |
| `game_unknown_001388a0` | `0x001388a0` | `game/game.c` |
| `game_lost` | `0x00138960` | `game/game.c` |
| `game_finish` | `0x001389c0` | `game/game.c` |
| `game_finished_wait_over` | `0x00138a10` | `game/game.c` |

### Corregido

- El campo de las opciones que llamé `reverted` no es una marca: es el tipo
  de simulación de la partida (`simulation`). Volver a un punto de control
  lo pone a 1, que es «en una sola consola»; por eso parecía una marca.

### Lo que se sabe

- Las opciones por defecto de una partida: campaña, una consola, dificultad
  normal, semilla 0x78a8 y un ritmo de ticks igual a la mitad de la
  frecuencia de la imagen (30 por segundo a 60 Hz, 25 a 50 Hz).
- Cinco segundos después de perder, el juego vuelve al punto de control;
  siete segundos después de terminar, acaba.
- Un fotograma ejecuta varios ticks seguidos. Si entre dos ticks el bucle
  tiene algo que hacer (guardar, cambiar de bsp...), los que faltan no se
  ejecutan y su tiempo se descuenta.
- Tipo de simulación: de 2 a 5 la partida se reparte entre consolas; 4 es
  la consola que predice (cliente). Los nombres `distributed`,
  `authoritative` y `predicted` son suposiciones.

### Sin comprobar

- Dos cambios que no alteran ningún resultado: dividir la frecuencia entre
  dos con división o con desplazamiento (las negativas se sustituyen antes
  por 60), y leer la simulación con signo o sin él.

## 2026-10-01 — El cambio de bsp y las acciones del bucle

Cómo el juego cambia la parte del nivel que tiene cargada (el bsp), y tres
de las acciones que el bucle principal atiende entre fotogramas. De 226
funciones se pasa a 231, el 2,05 % de las del juego (el 0,944 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `scenario_switch_structure_bsp` | `0x0011c1b0` | `scenario/scenario.c` |
| `cache_file_structure_bsp_unload` | `0x00122bc0` | `cache/cache_files.c` |
| `main_save_map_private` | `0x0012bb20` | `main/main.c` |
| `main_switch_structure_bsp_private` | `0x0012bd40` | `main/main.c` |
| `main_unknown4E_private` | `0x0012bdf0` | `main/main.c` |

- `tests/test_scenario_switch.py`.
- `ESTADO.md` (lista de lo hecho y lo que falta) y
  `tools/mutation_check.py`, del paso anterior, se mantienen al día desde
  aquí.

### Lo que se sabe

- Cambiar de bsp, por orden: avisar a cada parte del juego; desmontar lo
  que las partes tienen del bsp viejo; olvidar el bsp viejo; leer el nuevo
  del disco; apuntar las variables globales (bsp, mapa de luces, bsp de
  colisión); y preparar las partes para el nuevo. Si la lectura falla, el
  juego se queda sin bsp y las partes no se preparan.
- El tiempo que tarda la carga no cuenta en el fotograma siguiente.
- Las tres acciones muestran un mensaje al jugador en la campaña. Sus
  textos son `hud_autosave_end` (punto de control hecho),
  `hud_loading_end` (carga terminada) y `hud_saving_end` (guardado
  terminado); los nombres salen de la tabla de cadenas de los mapas.
- Al guardar, el mensaje va a cada jugador de la consola; en las otras
  dos, solo al primero.

### Sin comprobar

- En `scenario_switch_structure_bsp`, poner a cero el mapa de luces cuando
  el bsp no tiene: el juego lo hace, pero ya estaba a cero.
- `main_save_map_private` y `main_switch_structure_bsp_private` con el
  juego sin preparar (`main_game_globals` nulo): el juego original lee de
  la dirección cero, así que no se puede probar.

### Pendiente

- La acción `0x0012bbd0`, que abre pantallas de la interfaz.

## 2026-10-01 — Un fotograma del juego y su reloj

Lo que el bucle principal hace en cada fotograma, y la función que calcula
cuántos segundos cuenta ese fotograma. De 224 funciones se pasa a 226, el
2,01 % de las del juego (el 0,910 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_loop_body` | `0x0012b450` | `main/main.c` |
| `main_time_update` | `0x0012b0e0` | `main/main.c` |

- `tests/test_main.py`.

### Cambiado

- Las variables sueltas del tiempo del bucle son ahora una estructura,
  `main_time_globals` (`0x004e6388`). Los ocho bytes que `game_state_save`
  copiaba sin saber qué eran son el momento del último fotograma: después
  de guardar se igualan al contador actual para que el tiempo que tardó el
  guardado no cuente en el fotograma siguiente.
- `0x00509340` es `application_paused`, como en Halo 1.

### Lo que se sabe

- Un fotograma hace, por orden: lo que el juego le pidió al bucle (volver
  al punto de control, reiniciar, cambiar de bsp, guardar, cargar un
  núcleo); varias partes que corren haya o no partida; guardar las
  preferencias si cambiaron; un paso de la copia de mapas al disco duro; y,
  si el juego no está en pausa, el reloj y las partes de la partida.
- El tiempo se cuenta en refrescos verticales de la imagen, no en
  milisegundos: los segundos de un fotograma son los refrescos pasados
  divididos por la frecuencia (60 si no se conoce), entre 0 y 10.
- Si al juego se le fija un número de refrescos por fotograma, cada
  fotograma dura como mínimo dos.
- El contador de milisegundos de la consola se guarda en 64 bits, sumando
  uno a la parte alta cada vez que da la vuelta (cada 49 días).
- El campo de las tareas que llamé `tasks_added` se iguala en cada
  fotograma al número de tareas pendientes: es una cuenta aproximada de
  las que quedan, no de las creadas.

### Sin comprobar

- Qué son las 33 funciones que el fotograma llama y que no están
  descompiladas: solo se comprueba que se llaman en el mismo orden y con
  los mismos argumentos que en el juego.
- En `main_time_update`, la comparación del último fotograma con el momento
  previsto: cambiar `>` por `>=` da el mismo resultado.

### Pendiente

- Lo que el juego le pide al bucle y aún no está: guardar
  (`0x0012bb20`), cambiar de bsp (`0x0012bd40`), `0x0012bbd0` y
  `0x0012bdf0`.

## 2026-10-01 — El bucle principal y la tabla de sistemas

El bucle principal del juego y, con él, la tabla de las 68 partes del juego
(objetos, sonido, IA...) que se recorre para prepararlas y desmontarlas. De
210 funciones se pasa a 224, el 1,99 % de las del juego (el 0,876 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_loop` | `0x0012b690` | `main/main.c` |
| `main_time_initialize` | `0x0012b070` | `main/main.c` |
| `main_revert_map_private` | `0x0012ba30` | `main/main.c` |
| `main_reset_map_private` | `0x0012ba90` | `main/main.c` |
| `main_revert_map_if_possible_private` | `0x0012bad0` | `main/main.c` |
| `main_action_pending` | `0x0012be90` | `main/main.c` |
| `main_cancel_map_copy_and_wait` | `0x0012bf40` | `main/main.c` |
| `game_initialize` | `0x00137c20` | `game/game.c` |
| `game_initialize_for_new_map` | `0x00137ca0` | `game/game.c` |
| `game_options_new` | `0x00137dd0` | `game/game.c` |
| `game_dispose_from_old_map` | `0x00137d00` | `game/game.c` |
| `game_initialize_for_new_structure_bsp` | `0x00137d40` | `game/game.c` |
| `game_dispose_from_old_structure_bsp` | `0x00137da0` | `game/game.c` |
| `no_operation` | `0x00175f40` | `cseries/cseries.c` |

- La tabla `game_systems` (`0x00440dd8`): 68 partes con hasta 9 funciones
  cada una. La prueba comprueba que cada casilla tiene la misma función que
  la del juego.
- `game_dispose`, que el juego no tiene como función aparte: su código está
  dentro de `main_loop`.
- Archivos nuevos: `main/main.{c,h}`; `tests/test_game.py`.

### Cambiado

- `main_reset_map_requested` pasa a ser un campo de `main_globals`
  (`0x00547f28`), que reúne lo que el juego le pide al bucle.
- `tools/difftest.py`: los punteros a funciones del juego que `source/` aún
  no tiene también se comparan por nombre.

### Lo que se sabe

- El bucle (`main_loop`): empieza a contar el tiempo, prepara el juego,
  repite fotogramas hasta que se pide salir, termina la partida y desmonta
  el juego.
- Cada parte del juego puede tener hasta nueve funciones: preparar y
  desmontar; mapa nuevo y mapa viejo; bsp nuevo y bsp viejo; y tres más
  para cuando cambia el bsp o lo que está activo. Se preparan en el orden
  de la tabla y se desmontan en el contrario. En Halo 1 cada función
  llamaba a las partes por su nombre; aquí es una tabla.
- `game_state` es la parte 16 de la tabla. De las 166 funciones distintas
  que hay en ella, solo cinco están descompiladas; el resto están apuntadas
  por su dirección (`unknown_...`).
- Lo que el juego le pide al bucle (reiniciar el mapa, volver al punto de
  control, guardar, cambiar de bsp...) son marcas en `main_globals` que se
  atienden al principio de cada fotograma.

### Sin comprobar

- Qué hace cada una de las tres últimas funciones de una parte: sus nombres
  son suposiciones, por cómo las llama `0x0011c1b0`.

### Pendiente

- El cuerpo de un fotograma (`0x0012b450`), que llama a unas 35 funciones.
- `scenario_switch_structure_bsp` (`0x0011c1b0`), que carga un bsp y usa
  las funciones de bsp de la tabla.

## 2026-10-01 — El arranque del juego

Las primeras funciones del ejecutable: por dónde empieza el juego, qué
prepara antes de su bucle principal y qué desmonta al salir. Con esto hay
un hilo desde el punto de entrada hasta casi todo lo descompilado. De 203
funciones se pasa a 210, el 1,86 % de las del juego (el 0,840 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `shell_initialize` | `0x00012000` | `shell/shell.c` |
| `shell_dispose` | `0x00012080` | `shell/shell.c` |
| `shell_main` | `0x00012190` | `shell/shell.c` |
| `shell_platform_initialize` | `0x00012200` | `shell/shell.c` |
| `shell_get_video_settings` | `0x000123b0` | `shell/shell.c` |
| `shell_test_device` | `0x00012420` | `shell/shell.c` |
| `shell_create_device` | `0x00012490` | `shell/shell.c` |

- Archivos nuevos: `shell/shell.{c,h}`; `tests/test_shell.py`.

### Cambiado

- `tools/difftest.py`: al sustituir una función del juego que termina
  saltando a otra, se mira en esa otra cuántos argumentos quita de la pila.
  Antes se tomaba el primer retorno que hubiera después, que podía ser de
  otra función (pasaba con `cache_files_initialize`).

### Lo que se sabe

- El orden del arranque (`shell_main`): crear y soltar el dispositivo de
  Direct3D como prueba, reservar la memoria física, `shell_initialize`, el
  bucle principal y `shell_dispose`. La función no retorna: acaba en un
  punto de ruptura.
- `shell_initialize` prepara, en este orden: una biblioteca de la consola,
  0x23000 bytes de memoria, la plataforma, el modo de coma flotante
  (precisión simple), el hilo de tareas, dos sistemas aún sin identificar,
  los archivos de caché, otro sin identificar, el estado del juego y el
  rasterizador. Si no hay dispositivo de Direct3D se rinde; si lo hay,
  prepara dos sistemas más (en Halo 1 son la entrada y el sonido).
- Al salir no se detiene el hilo de tareas: solo se cierran sus cuatro
  identificadores.
- El dispositivo se pide a 640×480 con Z de 24 bits. El modo panorámico y
  el progresivo salen de los ajustes de la consola; la frecuencia (50 o
  60 Hz) solo se ajusta en consolas PAL.
- Si el juego se lanza desde el disco de demos (`XDEMOS`), se anota la
  palabra `xdemo` en una variable que leerá otra parte del juego.

### Sin comprobar

- Que `shell_initialize` se rinda cuando `shell_platform_initialize`
  devuelve falso: esa función siempre devuelve verdadero, el compilador de
  `source/` lo sabe y no mira el resultado. En el juego la comprobación
  está, pero nunca se cumple.
- En `shell_create_device`, poner a cero el puntero del dispositivo cuando
  ya es cero: el juego lo hace y no cambia nada.

### Pendiente

- Los doce sistemas que `shell_initialize` y `shell_dispose` preparan y
  desmontan y que aún no están descompilados (`unknown_...`), y el bucle
  principal (`0x0012b690`).

## 2026-10-01 — El estado del juego y los puntos de control

`game_state.c`: la memoria donde el juego guarda todo lo que hay que
recordar de una partida, y cómo hace un punto de control y vuelve a él. Con
ello, el CRC de 32 bits. De 183 funciones se pasa a 203, el 1,80 % de las
del juego (el 0,809 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_initialize` | `0x00123b30` | `saved games/game_state.c` |
| `game_state_dispose` | `0x00123bf0` | `saved games/game_state.c` |
| `game_state_initialize_for_new_map` | `0x00123c20` | `saved games/game_state.c` |
| `game_state_buffer_protect` | `0x00123b00` | `saved games/game_state.c` |
| `game_state_set_structure_bsp_index` | `0x00123d20` | `saved games/game_state.c` |
| `game_state_malloc` | `0x00123d40` | `saved games/game_state.c` |
| `game_state_malloc_aligned` | `0x00123d80` | `saved games/game_state.c` |
| `game_state_save` | `0x00123e20` | `saved games/game_state.c` |
| `game_state_note_death` | `0x00123ed0` | `saved games/game_state.c` |
| `game_state_revert` | `0x00123f60` | `saved games/game_state.c` |
| `game_state_header_valid` | `0x00124520` | `saved games/game_state.c` |
| `game_state_headers_match` | `0x00124590` | `saved games/game_state.c` |
| `game_state_set_revert_time` | `0x00124620` | `saved games/game_state.c` |
| `game_state_allocation_allocate` | `0x00124700` | `saved games/game_state.c` |
| `game_state_allocation_restore` | `0x00124750` | `saved games/game_state.c` |
| `game_state_call_before_load_procs` | `0x00199520` | `saved games/game_state.c` |
| `game_state_call_after_load_procs` | `0x00199540` | `saved games/game_state.c` |
| `crc_build_table` | `0x00163c00` | `memory/crc.c` |
| `crc_checksum_buffer` | `0x00163ba0` | `memory/crc.c` |
| `no_operation1` | `0x00072c70` | `cseries/cseries.c` |

- Archivos nuevos: `saved games/game_state.c`, `memory/crc.{c,h}`,
  `game/game.{c,h}` (solo las variables y estructuras que hacen falta);
  `tests/test_game_state.py`.

### Cambiado

- `tools/difftest.py`:
  - `memory={dirección: Memory(...)}` pone datos en una dirección fija
    antes de la llamada y los compara después: para lo que el juego alcanza
    por un puntero de una variable global.
  - `GlobalRange(tamaño)`: una variable que la prueba no toca, solo para
    que los punteros a ella se comparen bien.
  - Columna `convention` en `symbols/functions.csv`: `thiscall` para una
    función de `source/` que recibe su primer argumento en `ecx`.
  - La memoria física del emulador pasa de 1 MB a 8 MB, y el límite de
    instrucciones de 20 a 200 millones (poner a cero el estado son 4 MB).
- La biblioteca C de las pruebas gana `strchr`.
- Los tamaños de la memoria del estado estaban al revés: la parte normal es
  de 0x3be000 bytes y la del final, de 0x40000.

### Lo que se sabe

- La memoria del estado se reparte al arrancar, sin liberar nunca: cada
  sistema pide su parte con `game_state_malloc`. Los tamaños pedidos van a
  un CRC; un estado guardado con otro reparto tiene otro CRC y se rechaza.
- La cabecera del estado (0x1288 bytes) guarda ese CRC, la dirección de la
  memoria, el mapa, el build, un checksum del mapa, las opciones de la
  partida y el bsp cargado.
- Los dos archivos se alternan: cada punto de control se escribe en el que
  no se escribió la última vez, así siempre queda el anterior.
- Si el jugador muere cinco veces seguidas, cada una a menos de 8 segundos
  del punto de control, el juego pasa al punto de control anterior.
- Para volver a un punto de control se comprueba que sea de la misma
  partida y del mismo mapa. Si no lo es, o no hay ninguno, se pide
  reiniciar el mapa. La lectura se repite hasta que lo leído sea el punto
  de control.
- Antes de guardar se llama a 4 funciones y después a 1; antes de cargar a
  5 y después a 20. Solo se conocen cuatro de ellas; las demás están
  apuntadas por su dirección (`unknown_...`).
- `game_state_allocation` es un objeto que vive dentro del propio estado y
  da memoria del estado a quien pide un asignador (los `data_array`). Como
  al cargar un estado vuelve con la dirección de funciones de cuando se
  guardó, una de las funciones de después de cargar se la repone.

### Sin comprobar

- En `game_state_header_valid`, que el nombre del mapa no esté vacío: el
  juego lo comprueba, pero la comprobación siguiente (que tenga una barra)
  ya lo implica, así que quitarla no cambia ningún resultado.
- Si la búsqueda de la barra es `strchr` o `strrchr`: la función del juego
  (`0x003219f0`) no tiene nombre y aquí solo importa si la encuentra.

### Pendiente

- El resto de `game_state.c`: guardar y cargar el estado en un archivo de
  núcleo y en una partida guardada (`0x001240b0` a `0x00124490`,
  `0x00124640`, `0x00124770` en adelante).
- Las 21 funciones de antes y después de guardar y cargar.

## 2026-10-01 — Los archivos del estado del juego

Los archivos donde el juego guarda su estado para los puntos de control
(`n:\savegame0.bin` y `n:\savegame1.bin`): abrirlos, cerrarlos, escribir y
leer. Es el `game_state_xbox.c` de Halo 1, con los nombres de allí. De 177
funciones se pasa a 183, el 1,62 % de las del juego (el 0,759 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_create_or_open_file` | `0x00214f80` | `saved games/game_state_xbox.c` |
| `game_state_close_file` | `0x002150f0` | `saved games/game_state_xbox.c` |
| `game_state_write_finish` | `0x00215140` | `saved games/game_state_xbox.c` |
| `game_state_write_reclaim` | `0x00215180` | `saved games/game_state_xbox.c` |
| `game_state_write_to_file` | `0x002151f0` | `saved games/game_state_xbox.c` |
| `game_state_read_from_file` | `0x002152e0` | `saved games/game_state_xbox.c` |

- Archivos nuevos: `saved games/game_state.h` y
  `saved games/game_state_xbox.c`; `tests/test_game_state_xbox.py`.

### Corregido

- `cache_file_buffer_allocate` (`0x00214f10`) no era de los archivos de
  caché: es `game_state_allocate_buffer`, la que reserva la memoria del
  estado del juego. Se ha movido a `game_state_xbox.c`, y
  `cache_file_buffer_globals` pasa a ser `xbox_game_state_globals`, con
  todos sus campos.
- `physical_memory_buffer_allocate` recibe dos funciones, no una: la
  segunda es la que el juego llama para recuperar la memoria.

### Cambiado

- `tools/difftest.py`: al sustituir una función de `source/` que quita sus
  argumentos de la pila (`stdcall`), la pila queda como la dejaría la
  función.

### Lo que se sabe

- Hay dos archivos de estado, de 0x40b000 bytes cada uno; el estado ocupa
  0x3fe000 (0x40000 más 0x3be000).
- Un punto de control no para el juego: se copia el estado a memoria
  prestada y una tarea escribe la copia con prioridad 2. Si no hay memoria
  para la copia, se escribe el estado mismo con prioridad 3 y el juego
  espera, dibujando la pantalla de carga.
- Un archivo solo se puede leer si su última escritura llegó entera. Antes
  de leer o de volver a escribir se espera a la escritura que esté en curso.
- En Halo 1 había un solo archivo y se escribía y leía sin tareas.

### Sin comprobar

- Los dos argumentos de `game_state_write_reclaim` cuando la llaman
  `game_state_write_to_file` y `game_state_read_from_file`: la función no
  los usa, y el compilador de `source/` no los pone. No se comparan.

### Pendiente

- `game_state.c` (`0x00123b30` en adelante): `game_state_initialize`,
  guardar y volver a un punto de control. Necesita el CRC (`0x00163ba0`).

## 2026-10-01 — La copia de mapas al disco duro

El juego copia cada mapa del DVD al disco duro en segundo plano, para
cargarlo más rápido. Queda descompilado todo ese mecanismo: la cola de
peticiones, la tabla de copias fallidas, la elección del sitio y la máquina
de veinte pasos que hace la copia. De 150 funciones se pasa a 177, el
1,57 % de las del juego (el 0,729 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cached_map_file_copy_update` | `0x002141f0` | `cache/cache_files.c` |
| `cached_map_file_request` | `0x00213d20` | `cache/cache_files.c` |
| `cached_map_file_status` | `0x00213f10` | `cache/cache_files.c` |
| `cached_map_file_copy_current_priority` | `0x00213e30` | `cache/cache_files.c` |
| `cached_map_file_copy_pending_priority` | `0x00213e90` | `cache/cache_files.c` |
| `cached_map_file_copy_priority` | `0x00213ef0` | `cache/cache_files.c` |
| `cached_map_file_copy_cancel` | `0x00214990` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_reclaim` | `0x00213fa0` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_acquire` | `0x00213fe0` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_release` | `0x00214030` | `cache/cache_files.c` |
| `cached_map_file_copy_get_progress` | `0x002148b0` | `cache/cache_files.c` |
| `cached_map_file_get_progress` | `0x00214940` | `cache/cache_files.c` |
| `cached_map_file_failure_find` | `0x00214060` | `cache/cache_files.c` |
| `cached_map_file_failure_add` | `0x00214100` | `cache/cache_files.c` |
| `cached_map_file_failure_remove` | `0x002141b0` | `cache/cache_files.c` |
| `cached_map_file_failure_is_final` | `0x002141d0` | `cache/cache_files.c` |
| `cached_map_file_is_better_place` | `0x00214ac0` | `cache/cache_files.c` |
| `cached_map_file_choose_place` | `0x00214b80` | `cache/cache_files.c` |
| `cached_map_file_shared_index` | `0x00214c70` | `cache/cache_files.c` |
| `cache_file_map_paths_equal` | `0x00214e40` | `cache/cache_files.c` |
| `cache_file_xor_checksum` | `0x00123110` | `cache/cache_files.c` |
| `async_create_file` | `0x001a0b40` | `cseries/async.c` |
| `async_copy_work` | `0x001a1100` | `cseries/async.c` |
| `async_copy_position` | `0x001a1210` | `cseries/async.c` |
| `async_flush_file_work` | `0x001a1680` | `cseries/async.c` |
| `async_flush_file` | `0x001a16a0` | `cseries/async.c` |
| `file_time_get_current` | `0x00136e70` | `cseries/cseries.c` |

- `tests/test_map_copy.py`.

### Corregido

- `cache_file_header_verify` (`0x00122870`) pide la hora a la consola antes
  de comprobar la cabecera, sin usarla. La versión descompilada no lo hacía;
  salió a la luz al probar la máquina de la copia, que compara las llamadas.

### Cambiado

- `tools/difftest.py`:
  - Las funciones de la consola de `DEFAULT_EXTERNALS` (por ahora, la hora)
    se simulan solas si la prueba no las da.
  - Una función que es solo un salto a una función del kernel se puede
    sustituir.
  - `GlobalData` admite `clean`, para dejar fuera de la comparación lo que
    el juego copia de memoria que nunca escribió.
  - Un puntero al final exacto de una variable ya no se confunde con el
    principio de la siguiente.
- En la cabecera de un mapa, los 8 bytes en `0x14c` son la fecha en que se
  hizo la copia (`copy_time`).
- Los 0x840 bytes sin nombre de `cached_map_file_globals` son la tabla de
  copias fallidas, y `cached_map_file_copy_globals` tiene ya todos sus
  campos con nombre.
- `struct cache_file_time` pasa a ser `struct file_time`, en `cseries.h`.

### Lo que se sabe

- Solo se copia un mapa a la vez y solo uno puede esperar: el pedido con
  más prioridad, o el último. Si el que espera tiene tanta o más prioridad
  que el que se copia, la copia en curso se cancela.
- Los pasos: abrir el mapa del disco, leer su cabecera, comprobar que los
  mapas compartidos que usa están en el disco duro y son los mismos con
  los que se hizo, medir el archivo, elegir sitio, borrar la cabecera de
  la copia, copiar los datos por trozos, y escribir la cabecera al final.
  Así una copia a medias nunca tiene cabecera válida.
- Lo copiado se comprueba con un checksum: el o-exclusivo de todas las
  palabras de 32 bits, comparado con el campo `checksum` de la cabecera.
- Los trozos son de 256 KB con prioridad 0 y de 1 MB con las demás. El
  buffer es memoria prestada: si el juego la reclama, la copia la devuelve
  al terminar el trozo y la vuelve a pedir después.
- Un mapa cuya copia falla tres veces no se vuelve a intentar. La tabla
  guarda ocho mapas; si se llena, se pierde el fallo más antiguo.
- El sitio de la copia depende del tipo de escenario: uno fijo para cada
  mapa compartido, el menú y el multijugador, y dos para la campaña, de
  los que se elige el de la copia más antigua. Nunca se pisa el mapa que
  está cargado.
- La última escritura de la cabecera se pide con prioridad 0, no con la de
  la copia como las demás.

### Sin comprobar

- Los modos de `async_create_file` por encima de 5 y los tipos de
  escenario por encima de 4 en `cached_map_file_choose_place`: en el juego
  saltan a una tabla que no tiene esas entradas, así que no hay nada con
  qué comparar.

### Pendiente

- Quién llama a `cached_map_file_copy_update` y a `cached_map_file_request`
  desde fuera (`0x0012b450`, `0x0012bf40`, `0x00163890`).
- `physical_memory_buffer_allocate` y `physical_memory_buffer_free`
  (`0x0012d400`, `0x0012d520`), la memoria prestada.

## 2026-10-01 — Arranque de los archivos de caché

`cache_files_initialize`, lo primero que hace el juego con los mapas al
arrancar, y todo lo que necesita: abrir las seis copias de mapas del disco
duro, comprobarlas y rehacer las que no sirven. De 136 funciones se pasa a
150, el 1,33 % de las del juego (el 0,653 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cache_files_initialize` | `0x002136c0` | `cache/cache_files.c` |
| `cached_map_files_open` | `0x00213970` | `cache/cache_files.c` |
| `cached_map_file_read_header` | `0x002149c0` | `cache/cache_files.c` |
| `cached_map_file_shared_get` | `0x00214c30` | `cache/cache_files.c` |
| `async_create_file_work` | `0x001a08f0` | `cseries/async.c` |
| `async_write_work` | `0x001a0fb0` | `cseries/async.c` |
| `async_write_position` | `0x001a1050` | `cseries/async.c` |
| `async_set_file_size_work` | `0x001a12c0` | `cseries/async.c` |
| `async_set_file_size` | `0x001a1310` | `cseries/async.c` |
| `async_close_file_work` | `0x001a1530` | `cseries/async.c` |
| `async_close_file` | `0x001a1550` | `cseries/async.c` |
| `async_get_file_size_work` | `0x001a15d0` | `cseries/async.c` |
| `async_get_file_size` | `0x001a15f0` | `cseries/async.c` |
| `csstrcat256` | `0x00122810` | `cseries/cseries.c` |

- `tests/test_cached_map_files.py`, y más casos en `tests/test_async.py`.

### Cambiado

- `tools/difftest.py` llena la pila de un valor que no es cero antes de cada
  llamada, para que se note una variable que solo uno de los dos lados
  inicializa.
- En la cabecera de un mapa, los tres bytes en `0x174` dicen de cuáles de
  los mapas compartidos toma recursos (`uses_shared_map`), en el orden de
  las tres últimas fechas: menú principal, `shared` y
  `single_player_shared`.

### Lo que se sabe

- Al arrancar, el juego borra las copias `cache006.map` a `cache019.map`
  (de otras versiones) y abre las seis que usa. Si el idioma de la consola
  o el número de build cambiaron desde la última vez, las borra todas.
- Una copia cuyo archivo no tiene el tamaño de su sitio se redimensiona y
  se le escribe una cabecera vacía; las copias que van detrás se borran,
  una sola vez.
- La cabecera vacía que se escribe es memoria de la pila sin inicializar con
  solo las cuatro cadenas vaciadas. Es un descuido del juego original y
  está reproducido tal cual.
- De una copia con el tamaño correcto se lee la cabecera. Solo la de
  `single_player_shared` se compara además con el mapa del disco (el campo
  en `0x144`). El código para comparar las fechas de los mapas compartidos
  está ahí, pero con una condición que nunca se cumple.
- Las tareas de archivo son cinco: abrir (puede crear las carpetas de la
  ruta), leer, escribir, cambiar el tamaño y cerrar, más la que mide.

### Sin comprobar

- Que `cached_map_files_open` vacíe la cadena del nombre en las dos
  cabeceras de la pila: quitar esa línea no cambia ningún resultado que las
  pruebas vean. El resto de la función sí se comprobó rompiendo líneas una
  a una.

### Pendiente

- Los que usan `cached_map_file_copy_globals` (`0x0055b4f0`), que copian un
  mapa del disco al disco duro: `0x002150f0` a `0x002152e0`, `0x00214ac0`.
- Las preferencias guardadas (`0x00120d80` a `0x00121280`).

## 2026-10-01 — El hilo que lee el disco

El juego no lee el disco desde el hilo principal: deja tareas en una lista y
otro hilo las va haciendo. Ese módulo queda descompilado entero, con la
lectura de archivos que usa la carga de mapas. De 117 funciones se pasa a
136, el 1,21 % de las del juego (el 0,584 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_initialize` | `0x00120a90` | `cseries/async.c` |
| `async_task_allocate` | `0x001208b0` | `cseries/async.c` |
| `async_task_list_insert` | `0x00120900` | `cseries/async.c` |
| `async_task_queue` | `0x001209c0` | `cseries/async.c` |
| `async_task_list_remove` | `0x00120a30` | `cseries/async.c` |
| `async_task_add` | `0x00120ba0` | `cseries/async.c` |
| `async_task_count` | `0x00120bf0` | `cseries/async.c` |
| `async_thread_procedure` | `0x00120c30` | `cseries/async.c` |
| `async_thread_loop` | `0x00120c40` | `cseries/async.c` |
| `async_task_set_priority` | `0x00120ce0` | `cseries/async.c` |
| `async_wait` | `0x00120d50` | `cseries/async.c` |
| `async_read_work` | `0x001a0e70` | `cseries/async.c` |
| `async_read_position` | `0x001a0f10` | `cseries/async.c` |
| `async_read_goes_before` | `0x001a1720` | `cseries/async.c` |
| `csnzprintf` | `0x0011c9c0` | `cseries/cseries.c` |
| `language_from_xbox_language` | `0x0011ca80` | `cseries/cseries.c` |
| `cached_map_file_maximum_size` | `0x00214c90` | `cache/cache_files.c` |
| `cached_map_file_get_path` | `0x00214cc0` | `cache/cache_files.c` |
| `cached_map_files_delete` | `0x002138f0` | `cache/cache_files.c` |

- `tests/test_async.py`, y más casos en `tests/test_map_files.py`.

### Cambiado

- `tools/difftest.py`:
  - `GlobalData`: el valor de una variable global que tiene punteros a sí
    misma. El juego y `source/` no la tienen en la misma dirección, así que
    los punteros se comparan por el sitio al que apuntan.
  - `GlobalPointer`: un argumento que es la dirección de una variable global.
  - `StopCall`: una función simulada puede cortar la llamada, para probar
    funciones que nunca retornan (el bucle del hilo).
  - El tipo de retorno `pointer` en `symbols/functions.csv`.
  - Los punteros a funciones de `source/` se comparan por nombre.
- `vsnprintf` de la biblioteca C de las pruebas corta el texto como la del
  juego: sin cero al final cuando no cabe.
- Las variables sueltas `async_mutex` y `async_task_list` pasan a ser campos
  de `async_globals`.
- Los dos argumentos sin nombre de `cache_file_read_async` y
  `async_read_position` son la categoría y la prioridad de la tarea.

### Lo que se sabe

- Hay 150 tareas como mucho. Cada una tiene prioridad, categoría, diez
  parámetros, una función y un aviso que se pone a 1 al terminar.
- La lista se ordena por prioridad. Entre lecturas de la misma prioridad se
  ordena por archivo y posición a partir de donde acaba la lectura
  anterior, para que el lector del disco no vaya y venga.
- La primera tarea de la lista solo cede su sitio a una de más prioridad:
  es la que el hilo puede tener a medias.
- Una lectura se hace en trozos de 256 KB como mucho; la función de la
  tarea devuelve 0 mientras le quede y el hilo la vuelve a llamar.
- Las copias de los mapas en el disco duro se llaman `cache000.map` a
  `cache005.map`: las dos compartidas en `z:` y las demás en `n:`. Cada
  sitio tiene un tamaño máximo.
- `0x00510834` es un bloque de 0x1e8 bytes que el juego guarda firmado
  (las preferencias): el idioma está en `+4` y el número de build de las
  copias de los mapas en `+0x1d4`. `0x00510819` avisa de que cambió.

### Pendiente

- `cache_files_initialize` (`0x002136c0`) y la función que copia los mapas
  al disco duro (`0x00213970`), que Ghidra no descompila bien.
- `main_loading_idle` (`0x00125d60`), que `async_wait` llama mientras espera.

## 2026-10-01 — Carga de los tags de un mapa

La cadena completa con la que el juego carga un mapa: construir la ruta del
archivo, buscarlo entre los mapas abiertos, reservar memoria, leer el índice
y los datos de los tags y dejar apuntados el escenario y los globals. De 104
funciones se pasa a 117, el 1,04 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `scenario_load` | `0x001228f0` | `cache/cache_files.c` |
| `scenario_tags_unload` | `0x00122af0` | `cache/cache_files.c` |
| `cache_file_read_async` | `0x00213760` | `cache/cache_files.c` |
| `cache_file_read_header` | `0x00213800` | `cache/cache_files.c` |
| `cache_file_block_until_not_busy` | `0x00213890` | `cache/cache_files.c` |
| `cached_map_files_find_map` | `0x00214a60` | `cache/cache_files.c` |
| `cache_file_get_map_name` | `0x00214d40` | `cache/cache_files.c` |
| `cache_file_get_map_path` | `0x00214db0` | `cache/cache_files.c` |
| `cache_file_buffer_allocate` | `0x00214f10` | `cache/cache_files.c` |
| `physical_memory_initialize` | `0x0012b400` | `memory/physical_memory.c` |
| `async_category_busy` | `0x00120b50` | `cseries/async.c` |
| `csstrcasecmp` | `0x0011c920` | `cseries/cseries.c` |
| `csprintf256` | `0x000b66f0` | `cseries/cseries.c` |

- Archivos nuevos: `memory/physical_memory.{c,h}`, `cseries/async.{c,h}`,
  `scenario/scenario.{c,h}` (las variables `global_scenario_index`,
  `global_scenario` y `global_game_globals`).
- `tests/test_scenario_load.py` y `tests/test_map_files.py`.

### Cambiado

- `tools/difftest.py`:
  - Una prueba puede sustituir cualquier función de
    `symbols/functions.csv` por nombre, esté descompilada o no. Así cada
    función se prueba sola, sin depender de las que llama.
  - `source/` se compila con `-fno-inline`, para que una llamada siga
    siendo una llamada y se pueda sustituir.
  - Una variable global puede recibir bytes (la estructura entera), que se
    comparan al terminar.
  - El emulador tiene memoria en `0x80000000`, donde la consola pone su
    memoria física.
- La biblioteca C de las pruebas (`tools/difftest_libc`) gana `strrchr`,
  `strspn`, `strpbrk`, `towlower` y `vsnprintf`.
- En la cabecera de un mapa, el campo de 256 bytes en `0x20` es la ruta
  del archivo. En el disco está vacío: lo rellena el juego al abrirlo.

### Lo que se sabe

- El juego pide al arrancar un bloque de memoria física de `0x00061000` a
  `0x03145000` (unos 49 MB) y lo reparte por los dos extremos, en páginas
  de 4 KB. Hay cinco etapas; cada una parte de lo que dejó la anterior.
- Los tags se leen en dos partes dentro del mismo bloque: el índice al
  principio y los datos al final. En medio queda el hueco del bsp.
- El juego tiene una tabla de seis mapas abiertos, cada uno con su cabecera.
  Los tres primeros son `shared`, `single_player_shared` y `mainmenu`. Los
  dos bits altos de un desplazamiento eligen de cuál se lee.
- La ruta de un mapa es `d:\maps\<nombre>.map`, con el nombre en
  minúsculas y sin extensión. Si el nombre empieza por `t:\` o `w:\` se
  usa tal cual.
- Si una lectura de los tags falla, el juego pone a cero el número de
  compilación guardado (`0x00510a08`, normalmente 9809). Eso le hace
  copiar otra vez los mapas al disco duro.
- `cache_file_block_until_not_busy` espera a las tareas de las categorías
  2 a 6.

### Nombres supuestos

- Todos menos `csstrcasecmp`, `cache_file_get_map_path`,
  `cache_file_block_until_not_busy` y `scenario_tags_unload`, que hacen lo
  mismo que las de Halo 1 con ese nombre. `scenario_load` reúne lo que en
  Halo 1 son `scenario_load` y `scenario_tags_load`.
- La estructura de las tareas (`async_task`) y sus campos.

## 2026-10-01 — Lectura del mapa y fin de las tablas de datos

Cuatro funciones más: de 100 se pasa a 104, el 0,92 % de las del juego.
Son las primeras que llaman a funciones del juego aún sin descompilar.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `data_new` | `0x0016b570` | `memory/data.c` |
| `data_dispose` | `0x0016b5d0` | `memory/data.c` |
| `cache_file_blocking_read` | `0x00122d60` | `cache/cache_files.c` |
| `cache_file_structure_bsp_load` | `0x00122b40` | `cache/cache_files.c` |

- Con `data_new` y `data_dispose` el módulo de tablas de datos
  (`memory/data.c`, 20 funciones) queda completo.
- `tools/mapinfo.py`: lista el contenido de un mapa. Sustituye al programa
  en C.

### Cambiado

- `tools/difftest.py` puede probar funciones que llaman a otras:
  - `Mock`: una función simulada para un puntero a función (por ejemplo,
    las de un asignador de memoria).
  - Funciones externas: una fila de `symbols/functions.csv` sin archivo de
    origen es una función del juego sin descompilar. La prueba pone la
    misma función simulada en el original y en el nuestro.
  - Las llamadas que reciben las funciones simuladas, con sus argumentos,
    forman parte de lo que se compara.
- `build.ninja` solo comprueba los tamaños de las estructuras
  (`tools/verify_definitions.c`). Ya no genera ningún programa.

### Quitado

- `tools/mapinfo/` y `tools/common/` (el `mapinfo` en C). Smart App Control
  bloqueó `mapinfo.exe` al recompilarlo. No queda ningún ejecutable propio.

### Lo que se sabe

- Una tabla de datos guarda el asignador con el que se creó: un objeto con
  dos funciones, reservar y liberar. `data_dispose` libera con él.
- `cache_file_blocking_read` pide la lectura a `0x00213760` (sin
  descompilar) y cede el procesador hasta que termina.
- `cache_file_structure_bsp_load` redondea el tamaño del bloque a sectores
  de 512 bytes, lo lee en la dirección que dice el escenario, comprueba la
  firma `sbsp` y da su dirección a los tags del bsp y de su lightmap.
- `cache_file_globals` tiene en `+4` un puntero a la memoria de los tags.

### Nombres supuestos

- `cache_file_blocking_read`, `cache_file_structure_bsp_load` y
  `cache_file_read_async` (`0x00213760`).

## 2026-10-01 — Cien funciones

Cinco funciones sueltas más: de 95 se pasa a 100, el 0,89 % de las del
juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bit_vector_test_flag` | `0x000bfe60` | `cseries/cseries.c` |
| `fast_vector_sphere_intersection` | `0x0011e6f0` | `math/real_math.c` |
| `normalize3d_with_default` | `0x001201a0` | `math/real_math.c` |
| `bitmap_group_get_bitmap` | `0x00137550` | `bitmaps/bitmap_group.c` |
| `path_remove_last_element` | `0x001373c0` | `cseries/cseries.c` |

### Lo que se sabe

- `bitmap_group_get_bitmap` no pasa por `tag_get`: lee la dirección del tag
  en la tabla de instancias sin comprobar su tipo ni su identificador.
- `path_remove_last_element` deja la ruta vacía si no tiene ninguna barra
  invertida.

### Nombres supuestos

- Los cinco. `bitmap_group_get_bitmap` es el nombre de Halo 1 para una
  función que allí recibe el grupo y no su índice.

## 2026-10-01 — Acceso a los tags

Las funciones con las que casi todo el juego llega a sus datos. De 90 se
pasa a 95, el 0,84 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `tag_get` | `0x00122c10` | `cache/cache_files.c` |
| `tag_iterator_next` | `0x00122c70` | `cache/cache_files.c` |
| `cache_file_tag_group_compare` | `0x00122cf0` | `cache/cache_files.c` |
| `cache_file_tag_group_get` | `0x00122d00` | `cache/cache_files.c` |
| `binary_search` | `0x0013ddd0` | `memory/binary_search.c` |

- Las variables globales `cache_file_globals` (`0x005476f8`) y
  `global_tag_instances` (`0x004e3b44`).
- `tests/test_tags.py`: unos 3.700 casos, con índices de tags generados al
  azar.

### Cambiado

- `tools/difftest.py`: un argumento puede ser un puntero a una función
  (cada lado recibe la suya), una dirección dentro de un bloque de la misma
  llamada, y una variable global puede ser un campo de una estructura
  (`nombre+desplazamiento`).

### Lo que se sabe

- `tag_get` comprueba el tipo del tag pedido: lo acepta si es del grupo
  pedido o de un grupo que lo tiene como padre o abuelo. Para eso busca el
  grupo del tag, por búsqueda binaria, en la tabla de grupos.
- La comparación de grupos es una resta. Solo ordena bien valores cercanos,
  como son los cuatro caracteres de un grupo; con valores cualesquiera de 32
  bits la resta se desborda y la búsqueda falla.
- `cache_file_globals` es la de Halo 1 con un campo más: la dirección de la
  memoria de los tags, antes de la cabecera.
- Las funciones que el juego pasa a otras funciones retiran sus argumentos
  de la pila ellas mismas.

### Nombres supuestos

- `binary_search`, `cache_file_tag_group_get` y
  `cache_file_tag_group_compare`. `tag_get` y `tag_iterator_next` son los de
  Halo 1.

## 2026-10-01 — Cuaterniones y restos de módulos

De 83 funciones se pasa a 90, el 0,80 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quaternion_from_vectors3d` | `0x0011d6a0` | `math/real_math.c` |
| `quaternion_multiply` | `0x0011d820` | `math/real_math.c` |
| `fast_quaternion_interpolate_and_normalize` | `0x0011d950` | `math/real_math.c` |
| `rectangle3d_build_vertices` | `0x0011f9a0` | `math/real_math.c` |
| `matrix4x3_from_plane` | `0x00142390` | `math/matrix_math.c` |
| `matrix3x3_multiply` | `0x00142eb0` | `math/matrix_math.c` |
| `data_connect` | `0x0016b6b0` | `memory/data.c` |

### Cambiado

- `tools/difftest.py`: un argumento real puede ir en un registro SSE
  (`xmm0` a `xmm3`), que es donde lo recibe
  `fast_quaternion_interpolate_and_normalize`.

### Lo que se sabe

- El módulo de matemáticas reales termina hacia `0x00120890`; lo que sigue
  (`0x001208b0` en adelante) es otro módulo, con semáforos y colas, que
  parece la carga de datos en segundo plano.
- `data_connect` reconstruye el vector de bits de una tabla a partir de los
  identificadores de sus elementos. Sirve para tablas cuyos elementos vienen
  ya hechos, como las de un mapa.

### Nombres supuestos

- `quaternion_from_vectors3d`, `quaternion_multiply`,
  `rectangle3d_build_vertices` y `data_connect`.

## 2026-10-01 — Cadenas anchas y más matemáticas

De 66 funciones se pasa a 83, el 0,74 % de las del juego.

### Añadido

- `source/text/unicode.c` y `unicode.h`: cadenas de caracteres de 16 bits.

  | Función | Dirección |
  | --- | --- |
  | `ustrnzcat` | `0x0013fa70` |
  | `ustrnzcpy` | `0x0013fac0` |
  | `ustrnupr` | `0x0013fae0` |
  | `wchar_string_to_ascii_string` | `0x0013fb90` |
  | `ascii_string_to_wchar_string` | `0x0013fc90` |
  | `character_in_ranges` | `0x0013fcd0` |
  | `character_is_east_asian` | `0x0013fd90` |

- En `source/math/real_math.c`:

  | Función | Dirección |
  | --- | --- |
  | `limit3d` | `0x000a75d0` |
  | `quaternion_normalize` | `0x0011d610` |
  | `euler_angles2d_from_vector3d` | `0x0011df30` |
  | `vectors3d_from_euler_angles3d` | `0x0011df60` |
  | `vectors3d_from_euler_angles2d` | `0x0011dfb0` |
  | `project_point2d` | `0x00120790` |
  | `project_point3d` | `0x00120810` |

- `bsp3d_get_plane_from_designator` (`0x0014a300`) en `physics/bsp3d.c`.
- `real_alpha_to_pixel32` (`0x00131fc0`) y
  `real_alpha_and_rgb_color_to_pixel32` (`0x00131f40`) en
  `bitmaps/bitmap_color_conversion.c`.
- `tests/test_unicode.py` y más casos en los demás: unos 12.000 casos nuevos.

### Lo que se sabe

- El módulo de matemáticas reales del juego está entre `0x0011cf50` y
  `0x00120850`: unas 50 funciones, de las que hay 12 hechas. Las que quedan
  son más grandes (hasta 1.385 bytes).
- El código propio del juego está en direcciones múltiplo de 16. Las 2.581
  funciones que no lo están (394.000 bytes) se concentran en zonas que
  parecen librerías enlazadas: `0x220000`–`0x260000` y `0x2b0000` en
  adelante. Havok estará entre ellas.
- `ustrnupr` pasa a mayúsculas también la é y la ó, las únicas letras
  acentuadas que trata.
- Las funciones de proyección usan una tabla de ejes (`0x00440b94`): para
  cada eje dominante de un plano y cada sentido, qué dos ejes forman sus
  puntos 2D.

### Nombres supuestos

- `ustrnzcat`, `ustrnzcpy`, las dos conversiones entre cadenas anchas y
  ASCII, `character_in_ranges`, `character_is_east_asian` y
  `real_alpha_and_rgb_color_to_pixel32`. Los demás son los de Halo 1.

## 2026-10-01 — Primeras funciones de objetos

Las primeras funciones que usan el estado global del juego. De 59 se pasa a
66, el 0,59 % de las del juego.

### Añadido

- `source/objects/objects.c` y `objects.h`: la cabecera de un objeto, lo que
  se conoce de sus datos y siete funciones.

  | Función | Dirección |
  | --- | --- |
  | `object_wake` | `0x000b7360` |
  | `object_get_origin` | `0x000b9dd0` |
  | `object_get_world_matrix` | `0x000ba160` |
  | `object_get_location` | `0x000ba300` |
  | `object_try_and_get_and_verify_type` | `0x000badc0` |
  | `object_iterator_next` | `0x000baeb0` |
  | `object_get_ultimate_parent` | `0x000baf80` |

- `symbols/globals.csv`: el registro de variables globales, con su dirección
  en el ejecutable. La primera es `object_header_data` (`0x004e0300`), la
  tabla de cabeceras de objetos.
- `tests/test_objects.py`: unos 4.000 casos, con mundos de objetos generados
  al azar, algunos enganchados a otros.

### Cambiado

- `tools/difftest.py` y `tools/test_functions.py`: una prueba puede dar
  valor a variables globales para una llamada (un entero, o un bloque de
  memoria al que la variable apunta). El valor se escribe en la variable del
  ejecutable y en la de `source/`.

### Lo que se sabe de los objetos

- La cabecera de un objeto es la de Halo 1: identificador, flags, tipo y la
  dirección de sus datos.
- De los datos de un objeto: el objeto al que está enganchado (`0x14`) y el
  nodo de este (`0x18`), su localización en el nivel (`0x28`), su posición
  (`0x64`), sus vectores adelante (`0x70`) y arriba (`0x7C`), y dónde están
  las matrices de sus nodos (`0x116`).
- Un objeto enganchado a otro tiene su posición relativa al nodo del otro.

### Nombres supuestos

- `object_wake` y el bit de cabecera que activa; `object_get_location` y
  `object_get_world_matrix`.

## 2026-10-01 — Flujo de bits y más vectores

De 51 funciones se pasa a 59, el 0,52 % de las del juego.

### Añadido

- `source/networking/bitstream.c` y `bitstream.h`: el flujo de bits con el
  que el código de red empaqueta sus mensajes.

  | Función | Dirección |
  | --- | --- |
  | `bitstream_write_bits` | `0x001955d0` |
  | `bitstream_write_integer` | `0x00195720` |
  | `bitstream_read_bool` | `0x001957d0` |
  | `bitstream_read_bits` | `0x00195820` |
  | `bitstream_read_integer` | `0x001959c0` |

- En `source/math/real_math.c`: `projection_from_vector3d` (`0x00120850`),
  `fast_vector_intersects_sphere` (`0x0011e5e0`) y
  `fast_quaternion_normalize` (`0x001d8d60`).
- `tests/test_bitstream.py` y más casos en `tests/test_real_math.py`: unos
  3.900 casos nuevos.

### Lo que se sabe

- Halo 1 no tiene flujo de bits: su red es otra. Los nombres de estas
  funciones son supuestos.
- Las funciones que escriben en el flujo no borran los bits que pisan: dan
  por hecho que desde la posición en adelante hay ceros.
- `bitstream_read_bool` lee también en la posición justo después del final
  de los datos.
- `projection_from_vector3d` y `fast_vector_intersects_sphere` son las de
  Halo 1.

## 2026-10-01 — Vectores, azar, colores y árbol de colisión

Diez funciones sueltas de las más llamadas: de 41 se pasa a 51, el 0,45 %
de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `magnitude3d` | `0x00024590` | `math/real_math.c` |
| `normalize2d` | `0x0002b400` | `math/real_math.c` |
| `distance3d` | `0x0003ea30` | `math/real_math.c` |
| `perpendicular3d` | `0x0011d000` | `math/real_math.c` |
| `real_seed_random` | `0x000259a0` | `math/random_math.c` |
| `real_seed_random_range` | `0x000259d0` | `math/random_math.c` |
| `pixel32_to_real_rgb_color` | `0x00131bb0` | `bitmaps/bitmap_utilities.c` |
| `real_argb_color_to_pixel32` | `0x00131e50` | `bitmaps/bitmap_color_conversion.c` |
| `real_rgb_color_to_pixel32` | `0x00131ed0` | `bitmaps/bitmap_color_conversion.c` |
| `bsp3d_test_point` | `0x0014a280` | `physics/bsp3d.c` |

- `tests/test_bitmaps.py`, `tests/test_bsp3d.py` y más casos en
  `tests/test_real_math.py`: unos 6.000 casos nuevos.

### Lo que se sabe

- Los números aleatorios son los de Halo 1: mismas constantes y mismo
  método.
- `bsp3d_test_point` es la primera función de colisión: dice en qué hoja
  del árbol está un punto. El nodo ocupa 8 bytes, con dos hijos de 24 bits,
  donde el de Halo 1 ocupa 12.
- Los colores se redondean al entero más cercano, como redondea el
  procesador, no truncando.

### Límite conocido

- `magnitude3d` y `distance3d` usan en el juego instrucciones x87, que
  conservan un cuadrado demasiado pequeño o demasiado grande para un real.
  Para vectores de menos de 1e-19 o más de 1e19 de longitud, las de
  `source/` dan otro resultado. La prueba excluye esos casos.
- `real_seed_random` es un nombre supuesto.

## 2026-10-01 — Matrices

El módulo de matrices: de 21 funciones se pasa a 41, el 0,36 % de las del
juego.

### Añadido

- `source/math/matrix_math.c`: 20 funciones. Son las de `matrix_math.c` de
  Halo 1, salvo `matrix3x3_from_angles`, que ese juego no tiene.

  | Función | Dirección |
  | --- | --- |
  | `matrix4x3_inverse` | `0x00141590` |
  | `matrix4x3_translation` | `0x001416c0` |
  | `matrix4x3_rotation_from_vectors` | `0x00141710` |
  | `matrix4x3_rotation_from_angles` | `0x00141ce0` |
  | `matrix3x3_rotation_from_quaternion` | `0x00141e10` |
  | `matrix3x3_rotation_to_quaternion` | `0x00141f60` |
  | `matrix4x3_from_point_and_vectors` | `0x001420f0` |
  | `matrix4x3_from_orientation` | `0x001421f0` |
  | `matrix4x3_transform_point` | `0x00142570` |
  | `matrix4x3_transform_vector` | `0x00142640` |
  | `matrix4x3_inverse_transform_point` | `0x00142700` |
  | `matrix4x3_inverse_transform_vector` | `0x001427f0` |
  | `matrix4x3_inverse_transform_plane` | `0x001428b0` |
  | `matrix4x3_transform_points` | `0x001429d0` |
  | `matrix4x3_multiply` | `0x00142a60` |
  | `matrix3x3_transpose` | `0x00142b80` |
  | `matrix3x3_inverse` | `0x00142bf0` |
  | `matrix3x3_from_forward_and_up` | `0x00142d10` |
  | `matrix3x3_from_angles` | `0x00142da0` |
  | `matrix3x3_transform_vector` | `0x00143070` |

- `source/math/real_math.h`: los tipos de cuaternión, orientación y
  matrices, y `reciprocal_estimate`.
- `tests/test_matrix_math.py`: unos 3.000 casos.

### Cambiado

- `tools/difftest.py` compila `source/` con los reales en precisión simple
  (SSE) y pone el procesador emulado en precisión simple, como lo deja
  Direct3D en la consola. Con eso, **los reales se comparan exactos**: las
  41 funciones dan los mismos bits que el juego. `DIFFTEST_TOLERANT=1`
  admite diferencias en los últimos bits, para distinguir un orden de
  operaciones distinto de un cálculo distinto.

### Lo que se sabe

- `matrix4x3_rotation_from_angles` es la de Halo 1. Confirma la fórmula con
  la que `tools/scenario_objects.py` orienta los objetos de los niveles.
- Varias funciones (`matrix4x3_multiply`, `matrix4x3_transform_points`,
  `matrix4x3_from_orientation`) están escritas a mano con instrucciones
  SSE. El orden de sus sumas no es el del pseudocódigo: hay que leerlo en
  las instrucciones.
- `matrix4x3_from_orientation` usa la estimación del inverso del procesador
  (`rcpss`) con un paso de refinado. `reciprocal_estimate` da esa
  instrucción donde existe; en otro procesador el resultado diferiría en
  los últimos bits.

### Pendiente

- Del mismo módulo: `0x001417b0` (1.317 bytes), `0x00142390`, `0x00142eb0`,
  `0x00143120`, `0x00143250` (producto de matriz por vector de cualquier
  tamaño) y `0x00143360`.

## 2026-10-01 — Tablas de datos y primer vector

Primeras funciones del motor descompiladas con el banco de pruebas. De 3
funciones se pasa a 21, todas comprobadas contra el ejecutable.

### Añadido

- `source/memory/data.c` y `data.h`: las tablas de datos del motor, donde
  casi todos los sistemas guardan su estado (objetos, jugadores, efectos).
  17 funciones:

  | Función | Dirección |
  | --- | --- |
  | `data_initialize` | `0x0016b5f0` |
  | `data_initialize_disconnected` | `0x0016b650` |
  | `data_make_valid` | `0x0016b790` |
  | `data_delete_all` | `0x0016b7a0` |
  | `datum_new` | `0x0016b840` |
  | `datum_new_at_index` | `0x0016b910` |
  | `datum_new_at_absolute_index` | `0x0016b990` |
  | `datum_initialize` | `0x0016ba00` |
  | `datum_delete` | `0x0016ba40` |
  | `datum_try_and_get` | `0x0016bae0` |
  | `datum_try_and_get_absolute` | `0x0016bb20` |
  | `datum_absolute_index_to_index` | `0x0016bb50` |
  | `data_iterator_next` | `0x0016bb70` |
  | `data_next_index` | `0x0016bbc0` |
  | `data_next_absolute_index` | `0x0016bc00` |
  | `data_last_index` | `0x0016bc40` |
  | `data_prev_index` | `0x0016bcc0` |

- `source/math/real_math.c`: `normalize3d` (`0x00030bf0`), la función sin
  llamadas internas más usada del juego (188 sitios la llaman).
- `source/cseries/cseries.h`: los tipos base con los nombres de Halo 1.
- `tests/test_data.py`, `tests/test_real_math.py`: unos 9.600 casos, con
  tablas generadas al azar.
- `tools/function_index.py`: consulta el índice de funciones (las más
  llamadas, las que usan un texto, el pseudocódigo de una).
- `tools/disassemble.py`: muestra las instrucciones de una función y los
  valores de los datos que lee.

### Cambiado

- `tools/difftest.py`: los argumentos pueden ser bloques de memoria con
  punteros a sí mismos y con formato, para comparar reales con tolerancia;
  lee un real devuelto en un registro SSE.

### Lo que se sabe

- Las tablas de datos siguen el diseño de Halo 1 con cambios: contadores de
  32 bits, un vector de bits que marca los elementos en uso, alineación de
  los elementos y un puntero a quien reservó la tabla. La estructura ocupa
  `0x4C` bytes.
- El juego mezcla instrucciones x87 y SSE para los reales, y a veces
  devuelve un real en `xmm0`. Nuestro código usa solo x87, así que los
  resultados pueden diferir en los últimos bits; la prueba lo tolera.
- Los nombres `data_initialize_disconnected` y los de índices absolutos no
  existen en Halo 1: son los más probables, no confirmados.

### Pendiente

- `data_new` (`0x0016b570`) y `data_dispose` (`0x0016b5d0`), que usan el
  reservador de memoria, y `0x0016b6b0`, que conecta una tabla con
  elementos que están en otro sitio.

## 2026-10-01 — Banco de pruebas de funciones

Primera pieza para descompilar el ejecutable a escala: comprobar cada
función reescrita contra la original.

### Añadido

- `tools/difftest.py`: carga en un emulador de procesador (Unicorn) el
  ejecutable del juego y `source/` compilado para x86 de 32 bits, y ejecuta
  la misma función en los dos con los mismos argumentos. Nada del juego ni
  de `source/` se ejecuta en el propio ordenador, así que Smart App Control
  no interviene.
- `tools/test_functions.py`: ejecuta los casos de `tests/` y compara lo que
  devuelve cada función y lo que deja en memoria. Informa del avance sobre
  el total del juego.
- `symbols/functions.csv`: el registro de funciones descompiladas, con su
  dirección en el ejecutable y dónde recibe la original cada argumento (un
  registro o la pila).
- `tests/test_cache_files.py`: 162 casos para las tres funciones de
  `cache_files.c`. Coinciden todos.
- `tools/ghidra_scripts/ExportAllDecompiled.java`: exporta el pseudocódigo
  de las 11.265 funciones a `ghidra/decompiled/all/`, con un índice de
  llamadas y cadenas de texto, para buscar en ellas.
- `tools/difftest_libc/`: las funciones de la librería de C que usa el
  código del juego, para la imagen de pruebas.

### Lo que se sabe

- Las funciones del juego reciben a menudo sus argumentos en registros que
  elige el compilador, no según una convención fija. Por eso el registro
  indica el lugar de cada argumento.
- El emulador no tiene el kernel de la consola. Cada función del kernel que
  una prueba necesite hay que darla aparte; de momento solo la que da la
  hora.

## 2026-10-01 — Objetos de los niveles

Sexta meta: poner en cada nivel los objetos que coloca su escenario.

### Añadido

- `source/scenario/scenario_definitions.h`: los bloques de objetos del
  escenario (escenografía, bípedos, vehículos, equipo, armas, máquinas,
  controles y cajas), su paleta y el principio común de cada colocación.
- `source/objects/object_definitions.h`: el camino de un objeto a su modelo
  (`hlmt`) y de este a su render model (`mode`).
- `tools/scenario_objects.py`: lee las colocaciones y calcula la posición de
  cada punto del modelo en el nivel.

### Cambiado

- `tools/bsp2obj.py` escribe también los objetos del nivel, cada uno con su
  modelo en su sitio (`--no-objects` lo evita).
- `tools/bitmap.py` lee además los formatos A8, AY8 y A8Y8. A8, A8Y8 y
  A8R8G8B8 están sin probar: ninguna textura exportada los usa.
- `tools/test_maps.py` cuenta los objetos. Pasan los 30 mapas: 11.177
  objetos colocados, 218 más sin modelo, y 949 texturas.

### Corregido

- Los buffers de vértices de tipo `0x0A` (16 bytes) tienen posiciones
  comprimidas, no reales: el Jackal y otros modelos salían con posiciones
  inválidas. `tools/geometry.py` rechaza ahora una sección cuyas posiciones
  no sean números o estén demasiado lejos, para que la prueba detecte un
  tipo de vértice mal leído.

### Lo que se sabe de los objetos

- El escenario tiene, por cada clase de objeto, una lista de colocaciones y
  una paleta con los tags que usa el nivel. Cada colocación empieza igual:
  índice en la paleta, nombre, posición, rotación (tres ángulos) y escala.
- De un objeto se llega a su modelo (`hlmt`) y de este al render model.
- La rotación se convierte a vectores con la misma fórmula que Halo 1
  (`matrix4x3_rotation_from_angles` en halo-ce-universal).

### Pendiente

- La rotación está comprobada a simple vista en un nivel, no objeto por
  objeto. Si Halo 2 cambió el orden de los ángulos, los objetos inclinados
  saldrían mal orientados.
- Las armas y los vehículos de los mapas multijugador no salen: no son
  colocaciones del escenario, sino puntos de aparición de otro bloque.
- Cada objeto sale con la primera variante de su modelo y en su pose
  original; las variantes y las poses las decide el juego.

## 2026-10-01 — Modelos

Quinta meta: leer los modelos de los objetos (personajes, vehículos, armas).

### Añadido

- `source/models/render_model_definitions.h`: el tag `mode` (regiones,
  permutaciones, secciones, nodos, materiales, información de compresión).
- `tools/render_model.py`: lee un modelo.
- `tools/model2obj.py`: exporta un modelo a `.obj` con sus texturas.
- `tools/obj_writer.py`: la escritura de `.obj`, materiales y texturas, que
  comparten `bsp2obj.py` y `model2obj.py`.

### Cambiado

- `tools/geometry.py` lee vértices comprimidos y tiras de triángulos.
- `tools/cache_file.py` lee los string ids y busca tags por nombre.
- `tools/test_maps.py` lee todas las secciones de todos los modelos. Pasan
  los 30 mapas: 927 modelos, 2.978 secciones y 855.353 triángulos.

### Lo que se sabe de los modelos

- Un modelo tiene regiones; cada región, permutaciones (sus variantes); y
  cada permutación, una sección por nivel de detalle, hasta seis.
- Las secciones usan el mismo bloque `blkh` que los niveles. Las hay
  en el propio mapa y en `shared.map` (las del Warthog, por ejemplo).
- Las posiciones y las coordenadas de textura suelen ir comprimidas en
  enteros de 16 bits, que cubren el rango que da el modelo.
- Cada parte indica con un bit si sus índices son una lista de triángulos o
  una tira. Con esa regla, el número de triángulos coincide con el declarado
  en todas las secciones del juego, de niveles y de modelos.
- Los vértices están en el espacio del modelo, en su pose original: no hace
  falta el esqueleto para exportarlo.

### Pendiente

- El esqueleto (nodos), los pesos de cada vértice y las animaciones (`jmad`).
- Colocar los objetos en el nivel: el escenario dice qué objetos hay y
  dónde, y de cada objeto hay que llegar a su modelo.
- El Jefe Maestro sale blanco: el color de la armadura lo pone el juego.

## 2026-10-01 — Texturas

Cuarta meta: ver los niveles con sus texturas.

### Añadido

- `source/bitmaps/bitmap_definitions.h`: el tag `bitm` y cada bitmap que
  contiene (tamaño, formato, bloques de píxeles).
- `source/shaders/shader_definitions.h`: el principio del tag `shad`, hasta
  el mapa difuso.
- `tools/bitmap.py`: llega del shader a su mapa difuso y decodifica el
  bitmap a RGBA. Lee DXT1, DXT3, DXT5, Y8, R5G6B5, A4R4G4B4, X8R8G8B8 y
  A8R8G8B8, y deshace el orden "swizzled" de la consola. A8R8G8B8 está sin
  probar: ningún mapa difuso de nivel lo usa.
- `tools/bitmap2png.py`: escribe un bitmap de un mapa como PNG.
- `tools/png.py`: escribe y lee PNG sin librerías externas.

### Cambiado

- `tools/bsp2obj.py` escribe el mapa difuso de cada shader como PNG en una
  carpeta junto al `.obj` y lo enlaza en el `.mtl` (`--no-textures` lo
  evita).
- `tools/obj_preview.py` dibuja las texturas.
- `tools/test_maps.py` exporta también las texturas. Pasan los 30 mapas: 614
  texturas distintas, ninguna sin leer.

### Lo que se sabe de las texturas

- Un shader tiene un bloque con su mapa difuso; el 88 % de los shaders de
  nivel lo tiene. El resto son cristales, niebla, agua y similares, que
  usan otros bitmaps.
- Un bitmap guarda sus píxeles en hasta seis bloques, de más a menos
  detalle, cada uno seguido de sus mipmaps. Muchos están en `mainmenu.map`,
  `shared.map` o `single_player_shared.map`.
- Los bitmaps de los niveles son todos 2D. Los comprimidos son DXT; los no
  comprimidos están en orden "swizzled".

### Pendiente

- Solo se usa el mapa difuso. Un shader tiene además mapas de detalle, de
  relieve y otros, y una plantilla (`stem`) que dice cómo se combinan.
- Los bitmaps 3D y los mapas cúbicos no se leen, ni los formatos que no usan
  los mapas difusos de los niveles.
- El canal alfa se descarta al exportar un nivel: en la mayoría de los mapas
  difusos no es transparencia.

## 2026-10-01 — Geometría que dibuja el juego

Tercera meta: leer la geometría de los niveles que se ve en pantalla.

### Añadido

- `source/geometry/geometry_definitions.h`: las secciones de geometría (el
  bloque `blkh` / `rsrc` / `blkf`, sus recursos, las partes y los buffers de
  vértices).
- `source/structures/structure_bsp_definitions.h`: los clusters, los
  materiales, las definiciones de geometría instanciada y sus instancias.
- `tools/geometry.py`: lee una sección de geometría.
- `tools/structure_bsp.py`: lee los structure bsps de un mapa.

### Cambiado

- `tools/bsp2obj.py` exporta ahora la geometría que dibuja el juego, con
  coordenadas de textura: un objeto por cluster y por instancia. La
  geometría de colisión se exporta con `--collision`.
- `tools/cache_file.py` lee bloques de recursos que están en otro mapa
  (`mainmenu`, `shared`, `single_player_shared`).
- `tools/test_maps.py` comprueba las dos geometrías. Pasan los 30 mapas:
  4.191 clusters, 12.961 instancias y 3.551.172 triángulos.

### Lo que se sabe de la geometría

- Cada pieza de geometría tiene sus vértices y triángulos en un bloque del
  mapa aparte del tag. Los dos bits altos de su posición indican en qué
  archivo está.
- Un bloque contiene una "sección" con campos sin dirección y una lista de
  recursos; cada recurso es el contenido de un campo de la sección.
- En los structure bsps los triángulos son listas (tres índices cada uno),
  las posiciones son tres reales y las coordenadas de textura dos reales.
  Todos los mapas usan esos mismos formatos.
- Un nivel se compone de clusters, en coordenadas del mundo, y de geometría
  instanciada: definiciones que se colocan una o más veces con una
  posición, una rotación y una escala.

### Pendiente

- Las texturas: de cada shader hay que llegar a sus bitmaps y decodificarlos.
- Los buffers de vértices de tipo `0x1B`, `0x1F` y `0x30` (por tamaño:
  normales, coordenadas del mapa de luz) no se leen.
- Las secciones de los modelos (`mode`) usan el mismo formato de bloque pero
  no se han probado; pueden tener vértices comprimidos y tiras de triángulos.

## 2026-10-01 — Geometría de colisión de los niveles

Segunda meta: leer la geometría del nivel y verla fuera del juego.

### Añadido

- `source/scenario/scenario_definitions.h`: la referencia del escenario a
  cada structure bsp (bloque `0x210` del tag `scnr`).
- `source/structures/structure_bsp_definitions.h`: la cabecera del bloque de
  un structure bsp, el principio del tag `sbsp` y sus materiales de
  colisión.
- `source/physics/collision_bsp_definitions.h`: el collision bsp
  (superficies, aristas, vértices).
- `source/tag_files/tag_groups.h`, `source/math/real_math.h`: tipos base.
- `tools/cache_file.py`: lector de mapas para las herramientas en Python.
- `tools/bsp2obj.py`: exporta la geometría de colisión de todos los
  structure bsps de un mapa a un `.obj`, con un material por shader.
- `tools/obj_preview.py`: dibuja un `.obj` como imagen PNG, sin programas
  externos.
- `tools/common/`: el lector de mapas en C, separado de `mapinfo`.

### Cambiado

- `tools/test_maps.py` exporta además la geometría de cada mapa y comprueba
  que ninguna superficie esté rota ni del revés. Pasan los 30 mapas: 80
  structure bsps y 799.323 superficies en total, 0 rotas, 0 del revés.

### Lo que se sabe de los niveles

- Un structure bsp es un bloque del mapa aparte de los datos de los tags,
  con una cabecera de 16 bytes y la firma `sbsp`. El juego lo lee
  (`0x00122b40`) a la dirección que indica el escenario, entre el índice de
  tags y los datos de los tags.
- Los mapas de campaña tienen de 1 a 8 structure bsps, todos en la misma
  dirección: se carga uno a la vez.
- El collision bsp tiene la organización de Halo 1, con índices de 16 bits
  en lugar de 32.
- `shared`, `single_player_shared` y `mainmenu` tienen un structure bsp de
  relleno: una caja de 6 caras.

### Pendiente

- `0x00122b40`, la función que carga un structure bsp, está identificada
  pero no reescrita.
- La geometría que se dibuja (clusters, vértices con texturas) y los mapas
  de luz (`ltmp`) aún no se leen. Lo exportado es lo que usa la física, sin
  texturas, e incluye las paredes invisibles que cierran el nivel.
- Windows (Smart App Control) bloqueó un exportador compilado en C; se
  reescribió en Python.

## 2026-10-01 — Cargador de mapas

Primera meta: abrir los mapas del juego como lo hace el ejecutable.

### Añadido

- `source/cache/cache_files.h`: la cabecera de los mapas (caché versión 8),
  la cabecera del índice de tags, los grupos de tags y las instancias. Los
  campos sin identificar se llaman `unknown`.
- `source/cache/cache_files.c`: las tres primeras funciones descompiladas.

  | Función | Dirección | Qué hace |
  | --- | --- | --- |
  | `cache_file_header_verify` | `0x00122870` | Valida la cabecera de un mapa |
  | `cache_file_maximum_length` | `0x00214ed0` | Tamaño máximo según el tipo de mapa |
  | `cache_file_build_verify` | `0x00012130` | Comprueba la build del mapa |

- `tools/mapinfo`: programa que abre un mapa y lista la cabecera, los grupos
  de tags, los tags con su nombre y los string ids.
- `tools/test_maps.py`: prueba que abre todos los mapas. Pasan los 30.
- `tools/inspect_disc.py`: lee la cabecera del `default.xbe` (secciones,
  librerías del SDK, fecha de compilación) y la de cada mapa.
- `tools/ghidra_scripts/ExportFunctions.java`: exporta la lista de funciones
  que encuentra Ghidra.
- `tools/ghidra_scripts/DecompileFunctions.java`: descompila funciones por
  dirección o por una constante que usen.
- `tools/source_paths.py`, `tools/function_stats.py`, `tools/dump_header.py`:
  scripts de exploración.

### Lo que se sabe del ejecutable

- `halo2ship.exe`, compilado el 2004-09-28 con el XDK 5849.
- El código del juego (`.text`) ocupa 3,57 MB. Ghidra encuentra 11.265
  funciones; solo 310 salen con nombre, y son del SDK de Xbox y de la
  librería de C.
- Las librerías de la consola están en secciones aparte (`DSOUND`, `WMADEC`,
  `XONLINE`, `XNET`, `BINK*`, `D3D`, `XPP`): unas 2.500 funciones que no hay
  que descompilar.
- No quedan rutas de archivos fuente ni mensajes de aserción. Havok está
  dentro de `.text`.

### Lo que se sabe de los mapas

- El índice de tags se carga en la dirección `0x80061000`.
- El bloque de memoria de un mapa tiene el índice al principio, los datos de
  los tags al final y, entre ambos, espacio para la geometría del nivel
  (`sbsp`), que se carga aparte.
- Todos los mapas tienen los mismos 118 grupos de tags.

### Pendiente

- `0x001228f0`, la función que carga los tags, está identificada pero no
  reescrita.
- Los tags `sbsp` y `ltmp` (geometría y mapas de luz) aún no se leen.
