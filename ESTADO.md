# Estado del proyecto

Lista de lo hecho y lo que falta, para retomar otro día. La historia con
detalle está en [CHANGELOG.md](CHANGELOG.md); aquí solo el resumen.

**Última actualización:** 2026-10-08, al terminar el hito 569.

## Dónde estamos

- **1935 funciones descompiladas de 11.265: el 17,18 % del juego** (el
  10,829 % de los bytes de código).
- Las 1935 están comprobadas contra el ejecutable original: las 1390 del
  hito 400 con la batería completa, y las de después con sus propias pruebas
  (`python tools\test_functions.py`).
- Último hito: *Decompile which players are still there and how a player that another console says left leaves* (ver `git log`).
  El árbol de trabajo está limpio.
- Además hay 610 funciones apuntadas pero sin descompilar (las que el código
  llama); 461 de ellas solo se conocen por su dirección (`unknown_...`).

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
- [x] Búsqueda binaria — `memory\binary_search.c` (2)
- [x] Ordenar números — `cseries\sort.c` (4)
- [x] Tabla hash — `memory\hash_table.c` (5)
- [x] Vectores de bits — `memory\bit_vector.c` (5)
- [x] Anillo de bytes — `memory\ring_buffer.c` (3: añadir, poner y sacar;
      de `0x001a1900` a `0x001a19a0`)
- [x] Caché de bloques de páginas — `memory\lruv_cache.c` (8, completo)
- [x] Fondo de memoria — `memory\memory_pool.c` (11, completo)
- [x] Caché de animaciones — `cache\xbox_animation_cache.c` (18, completo)
- [x] Caché de texturas — `cache\xbox_texture_cache.c` (30, completo)
- [ ] Caché de geometría — `cache\xbox_geometry_cache.c` (17; falta
      `0x0012e480`, el aviso de que falta geometría)
- [x] Cargar y descargar un mapa — `main\main_game.c` (23, completo)
- [x] Memoria de la interfaz — `interface\user_interface_memory.c` (4)
- [x] Mandos — `input\input_xbox.c` (10, completo)
- [ ] Textos por idioma — `text\unicode_string_list.c` (6, de
      `0x001a0120` a `0x001a0350`; el archivo sigue hasta `0x001a08d0`)
- [x] Nombres por defecto de los archivos —
      `saved games\saved_game_file_names.c` (14, de `0x00217620` a
      `0x00217b70`)
- [x] Archivos de los jugadores — `saved games\saved_game_files.c` (48 de
      48; va de `0x00215790` a `0x00217590`: completo)
- [x] Tareas de los archivos de los jugadores —
      `saved games\saved_game_file_tasks.c` (20 de 20; el archivo va de
      `0x002accd0` a `0x002ae090`: completo)
- [x] Perfiles de los jugadores — `saved games\player_profile.c` (11:
      completo)
- [ ] Sonido — `sound\sound_manager.c` (237; faltan muchas de
      `0x00125f70` a `0x0012b070`), `sound\sound_classes.c` (1)
- [x] Archivos — `tag_files\files.c`, `files_xbox.c` (22, completo: de
      `0x00136710` a `0x001374c0`. Lo que hay justo antes, de `0x00136140`
      a `0x001366d0`, es de los bitmaps)
- [x] Cadenas anchas y UTF-8 — `text\unicode.c` (17)
- [x] Fuentes — `text\fonts.c` (7: completo, con `fonts_initialize`)
- [x] Caché de los caracteres de las fuentes — `text\font_cache.c` (28:
      completo; va de `0x001406b0` a `0x00141480`)
- [x] Dibujar texto — `text\draw_string.c` (19: completo, de `0x0013e8a0`
      a `0x0013f700`. Quien pinta de verdad los caracteres está en otro
      archivo, hacia `0x0001f070`)
- [x] Pintar los caracteres — `rasterizer\rasterizer_text.c` (6: completo)
- [ ] Dibujo de depuración — `rasterizer\rasterizer_debug.c` (3: empezar y
      terminar líneas o triángulos; faltan `0x0001e930` y `0x0001eb00`, que
      preparan la consola para dibujar)
- [ ] Flujo de bits de red — `networking\bitstream.c` (11; lo que queda,
      de `0x00194870` a `0x00195560`, comprime reales y direcciones con
      logaritmos y ángulos del procesador antiguo)
- [ ] Sesión de red — `networking\network_session.c` (181: el estado de
      una sesión, prepararla, arrancarla, su identificador y su clave,
      las consolas que entran y salen, cómo buscarlas, y los jugadores
      que lo piden;
      falta `0x000619b0`, y alrededor `0x00058ee0`, `0x000590b0` y de
      `0x00063220` a `0x00063980`)
- [ ] Dirección y claves de red — `networking\transport_security.c` (16:
      lo que la consola es en la red, la clave de cada sesión y las
      direcciones; de `0x0007a8a0` a `0x0007af80`, y falta lo que sigue
      desde `0x0007afd0`)
- [x] Mensajes que no van por un canal —
  `networking\network_message_gateway.c` (6: meter un mensaje en el
  paquete, mandarlo, leer el que llega, y con qué empieza un mensaje).
  Entero.
- [x] Intentos de medir la conexión — `networking\transport_qos.c` (8: la
  tabla de intentos, empezar uno, soltarlo, cómo va y qué encontró con
  cada consola). Entero.
- [x] Canales del juego — `networking\network_channel.c` (28: tomar un
      canal, pedir conexión y contestar, cerrarlo, soltarlo, recorrer
      sus piezas, sus horas y lo que el observador les pregunta;
      leer y escribir un paquete y lo que se hace cada vez; completo;
      romper líneas pendiente en `0x000883c0` y `0x00088980`)
- [ ] Cosas que las consolas comparten —
      `networking\replication_entity_manager.c` (15: la tabla de 1024,
      crear y borrar cosas y sus cadenas, las vistas, marcar cambios y
      quedarse con lo que hizo otra consola; lo que sigue, de
      `0x00089dc0` a `0x0008a110`, es de otros módulos)
- [ ] Vistas de las cosas — `networking\replication_entity_view.c`
      (17: preparar y vaciar una vista, poner una cosa en un estado,
      marcarla, avisar de que se borra, recorrer las que hay que
      anunciar, escribir creaciones y borrados (de una cosa o de
      una cadena) en un paquete, guardar lo mandado, atender lo que
      llega y saber si un paquete llegó o se perdió; quedan 3 de la
      lista, que parecen de otro módulo: `0x00097f00`, `0x00097f30` y
      `0x000998e0`; romper líneas pendiente en las 14 últimas)
- [ ] Lo que pasa en la partida —
      `networking\replication_event_manager.c` (6: la lista de
      sucesos, pedirlos al montón de la red, soltarlos y mandarlos
      contar; de `0x00089dc0` a `0x00089f20`, y `0x0008b0a0`)
- [ ] Vistas de los sucesos — `networking\replication_event_view.c`
      (2: vaciar una vista y saber si una cosa se está borrando;
      quedan 6 de la lista, de `0x000970b0` a `0x00097d80`; romper líneas
      pendiente)
- [x] La segunda pieza de un canal — `networking\network_message_queue.c`
      (11: lo que el canal le pregunta, vaciarla, guardar los
      mensajes que llegan, escribirlos y leerlos en un paquete, qué
      fue de cada paquete, partir un mensaje en trozos y juntarlos;
      tabla `0x00450db8`; completo; romper líneas pendiente en las
      cinco últimas)
- [ ] La partida en red, desde fuera — `networking\simulation.c`
      (16: hacer una vista, lo que `game_tick` le pide (hacer la actualización de cada
      tick y aplicarla), si se está jugando, pasar una actualización a
      bytes y rehacerla, una consola que ya está dentro, un canal que
      se va y la partida que no puede seguir; quedan `0x000680f0`,
      `0x000683a0` y de `0x00068a90` a
      `0x00069260`)
- [ ] La partida en red y sus vistas — `networking\simulation_world.c`
      (56: hacerla, soltar todas las vistas, hacer la actualización de un
      tick, lo que hacen los
      jugadores de esta consola, llevar una vista hasta
      el final, la espera a las demás
      consolas, la cola de actualizaciones
      entera, el estado del juego que
      llega, sus jugadores y lo que hacen los de otra consola, sus otras
      dieciséis cosas, recorrer las vistas, buscar una de tres maneras, un byte de
      cada jugador, devolver memoria prestada, ponerse en marcha, su
      segundo número, darse por vencida, contar
      lo que espera, ponerse al día, recibir una actualización, sacar la
      primera de la cola y decir qué vista frena; es lo de
      `unknown_004cf770`, y queda casi todo, de `0x00069300` a
      `0x0006ad40`)
- [ ] Las vistas de la partida en red — `networking\simulation_view.c`
      (27: tomar sitio en las tablas, nacer, soltar su canal e irse de la partida, lo que le queda por
      contar, poner una vista en un estado, seguir el del otro extremo,
      ver si está lista, darla por mala, marcar la que frena la partida, lo que hacen los
      jugadores de la otra consola y cuáles conoce, los trozos del
      estado del juego, desde qué número sigue la partida y pasar una
      actualización, y mandar lo que hacen sus jugadores; quedan
      `0x00085e70` y lo que sigue desde `0x00086bc0`)
- [ ] Los jugadores de la partida en red —
      `networking\simulation_player.c` (2: apuntar uno nuevo y dar al
      juego lo que hace; queda el resto, de `0x000847d0` a
      `0x00085540`)
- [ ] Lo que hace un jugador en un tick — `game\player_action.c` (2:
      dejarlo en blanco y comprobar que vale; `0x0014d040` y
      `0x0014d0a0`; sigue `0x0014d2a0`)
- [ ] A quien se dan los mensajes de las vistas —
      `networking\network_message_handler.c` (7: de `0x00094910` a
      `0x00094aa0`, completo; falta lo que las vistas hacen con cada
      mensaje, de `0x00085b20` a `0x00085f50`, y `0x000698e0`; romper
      líneas pendiente)
- [x] Los enchufes del juego — `networking\transport_endpoint.c` (15:
      hacer uno, sus opciones, abrirlo, atarlo, conectarlo, cerrarlo, leer
      y escribir, con dirección y sin ella)
- [x] La lista de canales por dirección — `networking\network_link.c`
      (19: abrirla con sus enchufes, apuntar, quitar, buscar, parar todos,
      hacerlos avanzar, cerrar los enchufes, los bytes de un paquete,
      mandarlos y leerlos)
- [x] Un bloque con suma, versión y tamaño — `networking\network_blob.c`
      (8: la suma, ver si vale, tomar y devolver su memoria, empezar a
      leerlo y a escribirlo, lo que pasa cuando la tarea acaba y su
      avance)
- [x] El almacén del servicio de partidas — `networking\online_storage.c`
      (7: el nombre, empezar a escribir y a leer, de memoria o de archivo,
      el resultado de leer y el avance)
- [ ] **Las funciones pequeñas que ya se llaman**: de las que el código
      llama sin tenerlas, las más cortas son las más fáciles (ya tienen
      nombre de registros y pruebas que las sustituyen). Quedan, por
      tamaño: `0x00147f4f` (30 bytes), `0x00063d80`, `0x001565a0`… (hitos 538 y 539; la lista
      la da `build\line_breaking\small_externals.py`)
- [ ] Los jugadores y las consolas de la simulación —
      `networking\simulation_players.c` (11, 7 de las 19 de `0x00082fa0` a
      `0x000846c0`; faltan las grandes: `0x00083000`, `0x00083220`,
      `0x00083370`, `0x00083610`, `0x00083a40`, `0x00083c90`,
      `0x00083db0`, `0x00083f00`, `0x00084270`, `0x000843f0`,
      `0x00084560`, `0x00084630`)
- [ ] El mensaje del día y los amigos del servicio —
      `networking\network_motd.c` (4, más dos que están en
      `online_friends.c`), `networking\online_friends.c` (18) y
      `networking\online_presence.c` (8), de la zona de `0x0008bf10` a
      `0x0008d550`, que tiene 35 funciones: quedan 2 (`0x0008c790`, que
      espera memoria con el reloj de la consola, y `0x0008d0b0`, que da un
      puntero a una función que no está en la lista)
- [ ] El mapa que se copia por adelantado — `main\main_map_precache.c`
      (3 de 7, de `0x00163610` a `0x00163b84`; faltan `0x00163610`,
      `0x00163680`, `0x00163890` y `0x00163b20`)
- [ ] Los altavoces — `sound\sound_speakers.c` (4, de `0x00192950` a
      `0x00192d70`; faltan `0x00192970`, `0x00192a30`, `0x00192b20` y
      `0x00192b90`, que necesita `_CIpow`)
- [ ] La lista de partidas del servicio — `networking\network_hoppers.c`
      (24 de unas 30, de `0x00192db0` a `0x001939f0`: la tabla de
      `0x00551ae8`, cuántos jugadores y equipos admite cada una, si es
      buena y qué partida juega; faltan `0x00193010` y `0x001936e0`)
- [ ] Lo que la interfaz pregunta a la partida en red —
      `interface\user_interface_networking.c` (5, de la zona de
      `0x0019989d` a `0x0019a02c`; falta `0x0019989d`)
- [ ] Los equipos del servicio de partidas — `networking\online_teams.c`
      (10: buscar los equipos de un jugador y los miembros de un equipo,
      lo que se sabe de cada uno, y cuatro tareas más; falta `0x000ac360`)
- [ ] El jugador de cada mando — `interface\user_interface_controller.c`
      (59 de unas 75, de `0x0018f372` a `0x001910b8`: cuál es, el
      siguiente, si es invitado, contarlos y buscar el que tiene perfil o
      está conectado, quitarle el perfil, dar su perfil, su nombre, quién
      es en el servicio y sus invitados, desconectarlo, dejar el mando sin
      jugador, escribir los perfiles que cambiaron, cambiar un campo
      del perfil, darle perfil y nombre, lo que se dice de él a los
      demás y a sus amigos del servicio, y ajustar su mando; falta
      `0x0018f58c` porque borra `0x48` bytes después de la tabla)
- [x] Las tareas del servicio de partidas — `networking\online_tasks.c`
      (28: la tabla, una nueva, por dónde va, acabar con una o con todas,
      recorrerlas, buscar una, volver a empezarla, hacerlas avanzar, en
      qué está la conexión, quién está conectado y lo que cada una hace
      cada vuelta, lo que se hace al conectarse y en qué está cada
      jugador, desconectarse, el nombre de un juego, la invitación
      aceptada, quitar o añadir a un jugador de los conectados y los
      dos textos de cada clase de tarea (46): completo)
- [x] La red de la consola — `networking\transport.c` (8: iniciarla y
      deshacerla, mirar el cable, arrancarla y pararla, apuntar a quién se
      avisa y avisar, y si está)
- [ ] Lo que la red hace cada vuelta — `networking\network_globals.c`
      (4: lo de cada vuelta, lo que se manda al final, y abrir y cerrar
      los enchufes; faltan hacerla y deshacerla, `0x0008d840` a
      `0x0008df50`, y diez de las cosas que hace cada vuelta)
- [x] Las tareas de los bloques — `networking\network_blob_task.c` (7:
      reiniciarlas, la primera libre, empezar a leer y a escribir, acabar
      una, ponerlas en marcha y recibir el resultado)
- [ ] La memoria de la red — `networking\network_memory.c` (6: tomarla
      al abrir un mapa, soltarla al cerrarlo, tres constructores, el de
      todo entre ellos, y deshacerlo todo)
- [x] La pieza principal de un canal — `networking\network_connection.c`
      (18: empezar de cero, el número de cada paquete, escribir y
      leer la cabecera de un paquete, lo que la otra consola
      contesta, qué fue de cada uno, dar por perdido el más viejo, el
      tiempo de ida y vuelta, las ventanas de paquetes llegados y
      mandados, y lo que el canal le pregunta; de `0x00095cf0` a
      `0x00096d80`, completo, con su tabla `0x00450dd8`)
- [ ] Lo que cada cosa es — `networking\simulation_entity_database.c`
      (22, y una en `networking\network_heap.c`: crear una o varias,
      apuntar, activar y quitar cosas, vaciar, pedir su memoria,
      pasar los cambios a las vistas, escribirlas y leerlas en un
      paquete, y catorce de las dieciséis funciones de su tabla
      `0x00450d60`; quedan `0x0008a460` y `0x0008a9c0`, que hacen
      cuentas con reales; romper líneas pendiente en las 23)
- [x] Observador de la red — `networking\network_observer.c` (74: lo que
      tiene de cada sesión, sus canales con las otras consolas y sus
      estados, el caudal de cada uno y cuándo puede mandar; de
      `0x00075970` a `0x0007a740`, completo, con dos que la lista de
      Ghidra no tenía)
- [x] Partida en red — `networking\network_life_cycle.c` (42: qué está
      haciendo, sus dos sesiones, lo que se le pide y su fotograma; de
      `0x000591e0` a `0x00059ab0`, completo)
- [x] Voz de los jugadores — `networking\voice.c` (22: completo, de
      `0x00055080` a `0x00055920`)
- [x] Lista de silenciados — `networking\voice_mute_list.c` (7: completo,
      de `0x00058a50` a `0x00058cb0`)
- [x] La voz entre consolas — `networking\voice_routing.c` (40: a quién
      habla cada jugador, las colas, lo que una consola manda a otra y
      quién oye a quién y los sonidos; va de `0x00055960` en adelante y
      está completo)
- [ ] Matemáticas reales — `math\real_math.c` (40 de unas 50, con dos que están lejos de las demás)
- [x] Matrices — `math\matrix_math.c` (26: completo)
- [ ] Números aleatorios — `math\random_math.c` (2)
- [x] Colores y bitmaps — `bitmaps\` (21: 6 de colores y 15 en
      `bitmaps.c`, que está completo)
- [ ] BSP 3D — `physics\bsp3d.c` (2)
- [ ] Objetos — `objects\objects.c` (13 de cientos)
- [ ] Componentes de Havok — `physics\havok_component.c` (14: la
      posición y la matriz (leerlas, fijarlas y llevar a ella un cuerpo
      «keyframed»), las velocidades
      (leerlas y fijarlas, y la de un punto), la rotación, la masa y
      el tipo de un cuerpo rígido)
- [x] Hilo de tareas y tareas de archivo — `cseries\async.c` (39, con la
      ruta de una referencia de archivo y abrir un archivo esperando)
- [x] Archivos de caché — `cache\cache_files.c` (46):
  - [x] acceso a los tags y carga de un mapa
  - [x] arranque (`cache_files_initialize`) y apertura de las copias
  - [x] copia de mapas al disco duro (máquina de 20 pasos)
- [x] Archivos del estado del juego — `saved games\game_state_xbox.c` (7,
      completo)
- [ ] Estado del juego y puntos de control — `saved games\game_state.c`
      (17; falta guardar/cargar núcleo y partida guardada)
- [x] Arranque del juego — `shell\shell.c` (12, con la memoria que piden
      las bibliotecas de la consola)
- [x] Lo que el juego hace con la voz — `networking\game_voice.c` (71:
      completo. Los dos fondos de memoria, arrancar y parar la voz, los
      modos, las máscaras y los mensajes, la sesión, sus consolas y sus
      jugadores, quién puede hablar y quién habla, y lo de cada
      fotograma; era `sound\sound_memory.c`. Va de `0x00053290` a
      `0x00055060`)
- [x] Preferencias guardadas — `main\global_preferences.c` (15: completo)
- [ ] Juego y tabla de sistemas — `game\game.c` (25): la tabla, lo que la
      recorre, las opciones (por defecto, validar, comparar, los jugadores
      locales), los ticks de un fotograma, un tick y un fotograma, partida
      perdida y terminada, y el tipo de simulación
- [ ] Bucle principal — `main\main.c` (16):
  - [x] el bucle, un fotograma y el reloj
  - [x] guardar, cambiar de bsp y avisar de que se guardó
  - [ ] la acción `0x0012bbd0` (pantallas de la interfaz)
- [ ] Escenario — `scenario\scenario.c` (3): cambiar de bsp, descargar
      el escenario y en qué hoja y cluster está un punto
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
- [x] ~~Archivos (`files.c`)~~ (hito 40).
- [x] ~~Resto de `game_state.c`~~ (hito 41): cargar un «core» y la
      partida guardada. Solo queda `0x001241b0`, que guarda el estado en la
      partida guardada (espera memoria de la caché de texturas con reloj,
      como `texture_cache_steal_memory`).
- [ ] **Los archivos de las partidas guardadas** (`0x002160b0` a
      `0x00217520`): lo que hay detrás de `saved_game_auxiliary_read` y
      `saved_game_auxiliary_write` (el archivo `auxilary.bin` de cada
      partida).
- [x] ~~Los mandos~~ (hito 42).
- [x] ~~El arranque del sonido~~ (`sound_initialize` y `sound_dispose`,
      hito 46).
- [x] ~~El sonido al cambiar de mapa~~ (hito 47: la fila del sonido en
      la tabla de sistemas está completa).
- [x] ~~`sound_idle` y las muestras que pide un sonido~~ (hito 48).
- [x] ~~Más del sonido~~ (hito 261): todas las funciones del sonido que
      estaban en la lista están descompiladas. El módulo vecino,
      `input/input_abstraction.c`, quedó completo en el hito 262.
- [x] ~~`lruv_block_new`~~ (hito 31).
- [x] ~~Caché de animaciones~~ (hito 32).
- [x] ~~Caché de texturas, primera parte~~ (hito 33): crear, abrir,
      cerrar, cada fotograma, lo que le da a `lruv_cache` y la memoria
      prestada.
- [x] ~~Caché de texturas, segunda parte~~ (hito 34): la carga de un
      bitmap. Quedan como externas `0x000359e0` (cuántos mipmaps) y
      `0x001365a0` (bytes por fila), de `bitmaps`.
- [x] ~~Caché de geometría~~ (hito 35). Quedan como externas
      `0x00223c70`, `0x00223d80` y `0x00223e00` (lo que hace falta de la
      geometría leída: apuntarla en su tag, saber si la consola aún dibuja
      con ella y esperarla) y `0x0012e480`.
- [x] ~~Cargar y descargar un mapa, primera parte~~ (hito 36):
      `main\main_game.c`, de `0x0018eb80` a `0x0018f170`.
- [x] ~~El cambio de partida~~ (hito 39): pedirlo, esperarlo, hacerlo,
      cargar el mapa, y qué pasa si falla.
- [x] ~~Lo que queda de `main_game.c`~~ (hito 43).
- [x] ~~`find_files_next`~~ (hito 44).
- [x] ~~Fondo de memoria (`memory_pool.c`)~~ (hito 37).
- [x] ~~El fondo de memoria de la interfaz~~ (hito 38).
- [x] ~~El fondo de la voz~~ (hito 263; el archivo es `game_voice.c`
      desde el hito 269).
- [x] ~~`voice.c`~~ (hito 268): completo.
- [x] ~~`game_voice.c`~~ (hito 278): completo.
- [x] ~~El resto de `voice_routing.c`~~ (hito 313).
- [ ] **La sesión de red**: lo que hay alrededor de lo ya hecho
      (`0x0005a400`, `0x00060400`, `0x00061050`), el observador
      (`0x00075970`, `0x00075da0`, `0x00075e80`, `0x00076ef0`),
      lo que sigue en `transport_security.c` (`0x0007afd0`), y lo que
      hace la partida en cada estado
      (`0x0006dec0`).
- [x] ~~Cinco direcciones repetidas en `symbols\functions.csv`~~ (hito
      296).
- [ ] **Los fondos de los objetos**: `0x000bc280`, `0x000bc380`,
      `0x000bf360`.
- [ ] **Los otros pares de la tabla de `0x00453c00`**: `0x00081780`,
      `0x002157e0`, `0x000b3d30`, `0x0021ebc0` y sus parejas.
- [ ] **La tabla de `0x00453c00`**: ocho pares de funciones (soltar y
      tomar memoria) de los que usan memoria al cargar un mapa; el par de
      la caché de animaciones ya está. La recorren `0x0018ec60` y
      `0x0018ed00`, que abren y cierran un mapa.
- [ ] **Matemáticas que quedan:** las grandes de `real_math.c`.

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

- La lista de Ghidra no tiene todas las funciones: las que solo se
  llaman por una tabla de funciones pueden faltar. Antes de dar un
  módulo por entero, mirar sus tablas en el ejecutable (hito 412).
- Al dar nombre a una función hay que cambiarlo también donde otras la
  llaman y en sus pruebas: si queda un `unknown_<dirección>` viejo, el
  arnés lo resuelve por la dirección y las pruebas pasan igual (hito
  398).
- Al declarar una función que aún no está descompilada, lo que recibe
  por puntero para escribir hay que darlo del tamaño que el juego le
  da (mirar los huecos de la pila de quien la llama): las pruebas no
  lo comprueban mientras la función esté sustituida (hito 383).
- Una cuenta con signo que puede desbordarse (multiplicar, sumar) hay
  que escribirla sin signo y convertir al final: el compilador de
  `source\` da por hecho que no se desborda y la simplifica, y el
  juego no (hito 369).
- `0x000619b0` necesita la potencia de la biblioteca C del juego
  (`_CIpow`, `0x00372d48`), que `tools\difftest_libc` no tiene.
- Donde el juego calcula con las instrucciones antiguas de reales
  (`fld`, `fmul`, `fsqrt`...) hay que escribir la cuenta con
  `long double` y redondear al final: con `real` el resultado difiere
  en números muy pequeños o muy grandes (hito 304).
- Una función que aún no está descompilada puede recibir argumentos en
  registros que no se ven hasta leer su código: lo que se apunta de ella
  en `symbols\functions.csv` sale solo de cómo la llama quien ya está
  descompilado. Al descompilarla hay que revisar a todos los que la
  llaman (pasó en los hitos 283 y 300).
- Una función que acaba saltando a otra (`jmp` en vez de `call` y `ret`)
  devuelve lo que esa devuelva. Si la otra aún no está descompilada no
  se sabe si es algo: quedan por mirar `0x00059200`, `0x00059230`,
  `0x00059360`, `0x000593a0`, `0x00059530`, `0x000595e0` y
  `game_voice_read_packet`.
- En la copia de comprobación (`C:\Users\steve\halo-2-check`) hay que
  cambiar de commit con `git checkout -f --detach <commit>`: las
  mutaciones dejan los archivos con otros finales de línea y sin `-f`
  el cambio no se hace, sin avisar.
- Antes de pasar las pruebas en una terminal nueva hay que poner delante
  en el `PATH` el clang de Visual Studio
  (`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin`):
  con el de llvm-mingw, que es el que sale si no, `game.c` no compila.
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

## Romper líneas: dónde quedó (2026-10-10, tras el hito 517)

- **Hecho:** hitos 415 a 455, completo. De los hitos 415 a 433, todas las
  líneas rotas se notan salvo cuatro que no se pueden notar: dos dan lo
  mismo rotas que enteras (hitos 420 y 432) y dos las pruebas no las pueden
  ver (un byte que se pone y se quita dentro de la misma función, y pasar un
  «sí» a 1).
- **Hitos 434 a 440** (`network_connection.c` desde
  `network_connection_reset`, `network_message_queue.c`, los dos mensajes
  del canal y `replication_event_set_data`): 108 líneas rotas, 106
  notadas. Las 2 que no:
  - en `network_message_queue_reset`, restar el tamaño de un mensaje a una
    cuenta que la misma función deja luego a cero: no se puede notar;
  - en `network_connection_acknowledge`, cambiar el suceso `5` por el `4`:
    era un hueco de la prueba, porque quien recibe el suceso hace lo mismo
    con los dos. La prueba se reforzó en el hito 450 y ya lo nota.
  No hizo falta corregir código. Informe: `build\line_breaking\batch_report6.txt`.
- **Hitos 441 a 455** (`network_message_queue.c` desde
  `network_message_queue_write`, `network_channel_handle_refused`,
  `simulation_world.c`, `simulation_view.c` y
  `network_message_handler.c`): 217 líneas rotas, 215 notadas. Las 2 que
  no, las dos en `network_message_queue.c`, no se pueden notar:
  - en `network_message_queue_send`, poner a cero un número del flujo
    propio que nadie lee antes de escribirlo;
  - en `network_message_queue_receive`, mirar si la memoria de un trozo
    es nula justo después de haber copiado de ella.
  Por el camino salieron cuatro huecos de las pruebas, ya cerrados: la de
  `network_message_queue_read` no daba al flujo su contenido (13 líneas
  sin notar; arreglada en el commit `5f526cf`), ni tenía ventanas anchas
  ni ventanas en las que `-1` fuera un número (3 líneas; reforzada ahora);
  y el mundo de la prueba de `simulation_view.c` era 16 bytes corto (1
  línea; arreglado en el hito 458). No hizo falta corregir código.
  Informes: `build\line_breaking\batch_report7.txt` y
  `batch_report8.txt`.
- **Hitos 491 a 500** (`network_blob.c` en sus dos primeras funciones,
  `network_channel_construct`, cuatro de `network_memory.c`,
  `transport_endpoint.c` y `network_link.c` enteros): 385 líneas rotas,
  384 notadas. La que no, en `network_link_packet_unpack`, da lo mismo
  rota que entera: quitar la mirada de si la voz mide menos que nada no
  cambia nada, porque la siguiente (que no mida más de lo que cabe, como
  número sin signo) ya la cubre. No hizo falta corregir código ni pruebas.
  Listas: `build\line_breaking\make_491_500.py`; informe:
  `batch_report9.txt`.
- **Pendiente, y por donde se sigue:** hitos 456 a 490 y 501 a 517. Sus
  listas de líneas no están escritas todavía.
- **Cómo se lanza una tanda suelta:** `python build\line_breaking\break_lines.py <fuente>
  <prueba> <funciones,separadas,por,comas> <archivo de líneas> <informe>`.
  Trabaja en la copia `C:\Users\steve\halo-2-check` (antes, `git checkout
  -q -f --detach <commit>` ahí). Los informes de las tandas hechas están en
  la misma carpeta (`batch_report*.txt`). La carpeta `build\` no entra en
  git.
