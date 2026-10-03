# Registro de cambios

Los cambios más recientes van arriba.

## 2026-10-02 — Crear los jugadores y ganar la partida

Dos funciones. De 662 funciones se pasa a 664, el 5,89 % de las del
juego (el 3,031 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_create_players` | `0x00137e40` | `game/game.c` |
| `game_won` | `0x001388e0` | `game/game.c` |

`main_game_start` ya llamaba a la primera con un nombre provisional.

### Lo que se sabe

- `game_create_players`: al empezar una partida, se le dicen a la red las
  **consolas** que están en ella y si esta es una de ellas, y se crea cada
  **jugador** válido de las opciones (hasta 16). Después vienen dos pasos
  más que aún no conocemos. Un indicador global la salta por completo.
- `game_won` (nombre de Halo 1): en la **campaña**, si las opciones tienen
  un cierto valor (`+0x18`), se avisa a cuatro cosas con la **dificultad**,
  se muestra «guardando» y se pasa al **mapa siguiente**; en
  **multijugador**, la partida termina.

### Mutaciones

22 líneas rotas a propósito: las 22 detectadas.

## 2026-10-02 — Archivos dañados y el mensaje de «guardando»

Dos funciones. De 660 funciones se pasa a 662, el 5,88 % de las del
juego (el 3,022 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_files_damaged` | `0x0012bf00` | `main/main.c` |
| `main_print_saving_message` | `0x0012b980` | `main/main.c` |

`main_game.c` ya llamaba a la primera con un nombre provisional, y
`fonts_initialize` la tenía copiada dentro: ahora las dos la llaman.

### Lo que se sabe

- `main_files_damaged` es lo que hace el juego cuando encuentra **archivos
  dañados**: cuando ni siquiera puede abrir el menú principal, y cuando las
  fuentes no están bien. Avisa a varias partes del juego (y apunta cuándo) y,
  en dos estados de otra parte, hace algo más. Uno de los avisos es un campo
  de `main_globals` que antes teníamos como una variable aparte.
- `main_print_saving_message` muestra al primer jugador de la consola el
  texto «guardando» (`0x100006A4`) en la campaña, **una sola vez** hasta que
  se termina de guardar: el indicador `unknown4E` de `main_globals`, que
  `main_unknown4E_private` vuelve a apagar al mostrar «guardado».
- Los dos nombres son suposiciones.

### Mutaciones

14 líneas rotas a propósito: las 14 detectadas.

## 2026-10-02 — Hacia dónde mira un objeto

Una función. De 659 funciones se pasa a 660, el 5,86 % de las del juego
(el 3,015 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_orientation` | `0x000b9fc0` | `objects/objects.c` |

### Lo que se sabe

- Devuelve hacia dónde mira un objeto y qué es «arriba» para él, en el
  mundo. Se puede pedir solo uno de los dos.
- Un objeto suelto los tiene tal cual. Uno unido a otro los guarda respecto
  al **nodo** del otro al que está unido, y aquí se giran con la matriz de
  ese nodo (solo el giro: ni escala ni posición). Es la pareja de
  `object_get_origin`, que hace lo mismo con la posición.
- El nombre es el de Halo 1.

### Mutaciones

13 líneas rotas a propósito: las 13 detectadas.

## 2026-10-02 — Copiar un archivo entero

Dos funciones. De 657 funciones se pasa a 659, el 5,85 % de las del
juego (el 3,001 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_copy_file_work` | `0x001a0c90` | `cseries/async.c` |
| `async_copy_file_blocking` | `0x001a0da0` | `cseries/async.c` |

### Lo que se sabe

- Copiar un archivo entero en otro es una **tarea de cinco pasos** del hilo
  de disco: volver al principio del origen (y apuntar su tamaño), volver al
  principio del destino, y luego leer y escribir **64 KB** cada vez hasta
  que una lectura trae menos; entonces se da por copiado.
- Si un paso falla, la tarea acaba y dice que no se copió. Si la escritura
  falla sin decir cuánto escribió, se cuentan 64 KB igualmente.
- La versión que espera usa un búfer de 64 KB en su propia pila, añade la
  tarea con prioridad 6 y espera a que termine; si alguno de los dos
  archivos no está abierto, no hace nada y devuelve que no se copió.
- Los nombres son suposiciones.

### Mutaciones

28 líneas rotas a propósito: las 28 detectadas.

## 2026-10-02 — Tres funciones pequeñas de los objetos

Tres funciones. De 654 funciones se pasa a 657, el 5,83 % de las del
juego (el 2,994 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_and_verify_type` | `0x000bae20` | `objects/objects.c` |
| `object_iterator_new` | `0x000bae80` | `objects/objects.c` |
| `object_get_first_unflagged_ancestor` | `0x000baf40` | `objects/objects.c` |

### Lo que se sabe

- `object_get_and_verify_type` es **idéntica** a
  `object_try_and_get_and_verify_type`: en Halo 1 la primera para el juego
  si el objeto no está, y el juego se compiló sin esas comprobaciones, así
  que quedaron dos copias iguales.
- `object_iterator_new` prepara un recorrido de los objetos de unos tipos
  (ninguno quiere decir todos) con unas marcas en su cabecera, desde el
  primero. El recorrido mide 16 bytes, no 12: termina con la firma
  `0x86868686`.
- La tercera sube desde un objeto por aquello a lo que está unido hasta el
  primero que no tiene el bit 26 de sus marcas (o hasta ninguno). Qué es
  ese bit no se sabe aún. Para ello no mira la sal del índice.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas. Una pidió casos con
índices sin sal, que pueden valer 0.

## 2026-10-02 — Matrices, cuaterniones y orientaciones

Cuatro funciones. De 650 funciones se pasa a 654, el 5,81 % de las del
juego (el 2,988 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quaternion_to_axis_angle` | `0x0011d790` | `math/real_math.c` |
| `matrix4x3_from_point_and_quaternion` | `0x001421b0` | `math/matrix_math.c` |
| `matrix4x3_to_orientation` | `0x00142360` | `math/matrix_math.c` |
| `matrix4x3_rotation_between` | `0x001424f0` | `math/matrix_math.c` |

También los globales `real_math_pi` (`0x005476c4`) y `real_math_two_pi`
(`0x005476c8`).

### Lo que se sabe

- `quaternion_to_axis_angle`: el eje de un giro (de longitud 1) y su ángulo,
  de 0 a pi; un giro de más de pi se da como el giro al revés. Un cuaternión
  sin giro da un eje de longitud 0.
- El juego guarda **pi y dos pi** en dos variables que no vienen en el
  ejecutable: se ponen al arrancar.
- `matrix4x3_from_point_and_quaternion` (nombre de Halo 1): una matriz de
  escala 1 que gira como un cuaternión y está en un punto.
- `matrix4x3_to_orientation`: lo contrario, de matriz a giro, posición y
  escala (la «orientación» de 32 bytes que ya conocíamos).
- `matrix4x3_rotation_between`: el giro que lleva de una matriz a otra,
  como un vector: su eje por su ángulo. Se calcula con la inversa de la
  primera.

### Mutaciones

19 líneas rotas a propósito: las 19 detectadas. Una pidió cuaterniones de
media vuelta, cuyo ángulo es pi exacto.

## 2026-10-02 — La memoria del sonido

Dos funciones, en un archivo nuevo. De 648 funciones se pasa a 650, el
5,77 % de las del juego (el 2,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_memory_allocate` | `0x00053290` | `sound/sound_memory.c` |
| `sound_memory_free` | `0x000532e0` | `sound/sound_memory.c` |

`XMemAlloc` y `XMemFree` ya las llamaban; ahora llaman a estas.

### Lo que se sabe

- La memoria que piden las bibliotecas de sonido de la consola (los
  asignadores `0x89` y `0x8A`) sale de **uno de dos *pools* de memoria** del
  juego; un indicador global dice cuál se usa.
- Una petición de tamaño 0 recibe `0x200` bytes. Lo demás que dice la
  petición no se mira.
- Devolver «nada» no hace nada.
- Los nombres del archivo y de las funciones son suposiciones.

### Mutaciones

10 líneas rotas a propósito: las 10 detectadas.

## 2026-10-02 — La memoria que piden las bibliotecas de la consola

Cinco funciones. De 643 funciones se pasa a 648, el 5,75 % de las del
juego (el 2,973 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `XMemAlloc` | `0x00012280` | `shell/shell.c` |
| `XMemFree` | `0x000122c0` | `shell/shell.c` |
| `shell_bitmap_get_texture_blocking` | `0x00012310` | `shell/shell.c` |
| `shell_bitmap_get_texture` | `0x00012360` | `shell/shell.c` |
| `xmem_static_allocate` | `0x0012c090` | `shell/shell.c` |

### Lo que se sabe

- Las bibliotecas de la consola (Direct3D, el sonido...) piden memoria con
  `XMemAlloc` y la devuelven con `XMemFree`, diciendo **quién la pide** en
  los bits 16 a 23 de sus atributos. El juego sustituye estas dos funciones
  para repartirla él:
  - quien tiene el número `0x82` recibe, para las peticiones de tipo 1, **un
    búfer de `0xCC` bytes que el juego tiene guardado** (a cero si se pide);
    las demás van a la consola;
  - `0x89` y `0x8A` reciben memoria del **sonido**;
  - el resto, la de la consola, como si el juego no hiciera nada.
- El búfer guardado no se mira para ver si está libre: se da aunque ya esté
  dado. Al devolverlo solo se marca como libre.
- Las otras dos funciones son la manera habitual de pedir la textura de un
  bitmap (si ya la tuvo este fotograma, la misma; si no, se busca y, si no
  está, se lee), esperando por ella o sin esperar.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas. Cuatro pidieron casos
nuevos: atributos con bits altos y un búfer guardado en una dirección que
coincida con la que se devuelve.

## 2026-10-02 — Los perfiles y la variante de las preferencias

Dos funciones. De 641 funciones se pasa a 643, el 5,71 % de las del
juego (el 2,961 % de sus bytes). Con ellas no falta ninguna función entre
las de `global_preferences.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `global_preferences_find_player_profile` | `0x00120e20` | `main/global_preferences.c` |
| `global_preferences_get_game_variant` | `0x00120e70` | `main/global_preferences.c` |

### Lo que se sabe

- Las preferencias guardan **cuatro nombres de perfiles** de jugador y el
  nombre de **una variante de juego**. La primera función busca el perfil de
  uno de esos nombres; la segunda, la variante.
- Si la variante no está, se crea **la variante por defecto**: su nombre es
  un texto del juego (el `0x0E000780` de una lista de textos que nombran los
  globales del juego) y tiene unos valores fijos (2, 25, 480, 16, 5, 10...),
  que de momento no sabemos qué son. Entonces no hay archivo (`NONE`).
- La variante de juego mide `0x130` bytes y lleva su nombre (31 caracteres)
  a 4 bytes del principio; ya tiene sus campos, aunque casi todos sin nombre.

### Mutaciones

30 líneas rotas a propósito: 28 detectadas. Dos pidieron casos nuevos:
etiquetas de índice mayor que 255 y una búsqueda que dice un archivo aunque
no encuentre la variante.

### Sin comprobar

- Dos mutaciones son equivalentes: copiar un carácter más del nombre (el
  cero que se pone después lo tapa) y no poner ese cero (la variante ya
  está a cero).

## 2026-10-02 — Tres funciones pequeñas de `cseries.c`

Tres funciones. De 638 funciones se pasa a 641, el 5,69 % de las del
juego (el 2,945 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `csstrnlen` | `0x0011c9a0` | `cseries/cseries.c` |
| `csnzappendf` | `0x0011c9e0` | `cseries/cseries.c` |
| `system_allocate` | `0x0011ca20` | `cseries/cseries.c` |

Pruebas nuevas en `tests/test_cseries.py`.

### Lo que se sabe

- `csstrnlen`: cuántos caracteres tiene una cadena, sin pasar de un máximo.
- `csnzappendf`: como `csnzprintf`, pero escribe **detrás** de lo que ya
  hay en el búfer, sin pasarse de su tamaño y dejándolo siempre terminado.
  Los dos nombres son los que tienen en Halo 3.
- `system_allocate`: pide memoria a la consola, desde arriba, para leer y
  escribir; si no hay, pregunta el error (sin hacer nada con él) y devuelve
  nada. El nombre es una suposición.

### Mutaciones

13 líneas rotas a propósito: 12 detectadas.

### Sin comprobar

- Una mutación es equivalente: dejar que `vsnprintf` escriba un carácter
  más. El `vsnprintf` del juego (el de Microsoft) no termina la cadena
  cuando corta, así que ese carácter cae justo donde después se pone el
  cero.

## 2026-10-02 — Leer un archivo entero

Dos funciones. De 636 funciones se pasa a 638, el 5,66 % de las del juego
(el 2,941 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_read_whole_file_work` | `0x001a13a0` | `cseries/async.c` |
| `async_read_whole_file` | `0x001a1480` | `cseries/async.c` |

Es la lectura que usa `fonts_initialize`; ya no hace falta suplantarla.

### Lo que se sabe

- Es una tarea del hilo de disco que lee **un archivo entero**, o lo que
  quepa de él, en tres pasos (una vuelta del hilo cada uno): abrirlo (para
  leer, dejando que otros lo lean), saber cuánto ocupa y leerlo de una vez.
- Termina cuando un paso falla o cuando ha leído; entonces cierra el archivo
  y dice si se leyó todo lo pedido. Si se le pide, dice también el tamaño
  del archivo (aunque no quepa entero).
- Al empezar pone a «no leído» y a 0 lo que se le pide que diga, para que
  quien espera no vea lo de antes.
- En el juego, la parte que crea la tarea está copiada dentro de la función;
  aquí se llama a `async_task_add`, que hace lo mismo.

### Mutaciones

36 líneas rotas a propósito: las 36 detectadas. Dos pidieron arreglar la
prueba: lo que se pide que diga tenía que estar en búferes que se comparen.

## 2026-10-02 — Las fuentes que hay

Una función. De 635 funciones se pasa a 636, el 5,65 % de las del juego
(el 2,935 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `fonts_initialize` | `0x00121fc0` | `text/fonts.c` |

`shell.c` ya la llamaba con un nombre provisional; ahora llama a esta.

### Lo que se sabe

- Al arrancar, el juego lee **un archivo que dice qué fuentes hay**: está en
  `z:\fonts\` y su nombre lo da otra función (aún sin descompilar). Es un
  texto con nombres separados por espacios, tabuladores o saltos de línea;
  se leen como mucho `0x7ff` bytes.
- El primer nombre es la fuente 0, el segundo la 1, y así hasta 11. Cada
  fuente nueva se carga **esperando** a que esté; un nombre repetido es la
  misma fuente y no se carga dos veces. Las que el archivo no nombra quedan
  como «no hay» (`-2`).
- Antes, otra función comprueba que lo que dicen los archivos de las fuentes
  está bien; si no, se avisa a varias partes del juego (y se apunta cuándo),
  pero el archivo se lee igual.
- El juego espera a que el archivo se lea y, si no llega a leerse nunca, se
  queda esperando para siempre.
- Solo se hace la primera vez; las siguientes solo se vuelven a preparar los
  caracteres y su caché.

### Mutaciones

45 líneas rotas a propósito: 44 detectadas. Una pidió arreglar la prueba
(la lectura suplantada no borraba el aviso de «terminado», y la espera no se
veía).

### Sin comprobar

- Una mutación es equivalente: cortar el texto leído un byte más allá solo
  cambia algo con `0x7ff` bytes, y entonces las dos versiones cortan en el
  mismo sitio.

## 2026-10-02 — Pintar un texto

Una función. De 634 funciones se pasa a 635, el 5,64 % de las del juego
(el 2,909 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_draw_characters` | `0x0001f070` | `rasterizer/rasterizer_text.c` |

Tres funciones de la consola que aún no están descompiladas reciben el
nombre que tienen en Halo 1, porque hacen lo mismo:
`rasterizer_text_begin` (`0x00035b90`), `rasterizer_text_draw_character`
(`0x000363a0`) y `rasterizer_text_end` (`0x00036560`).

### Lo que se sabe

- Es la que **pinta un texto de verdad**: prepara la consola, recorre el
  texto con `draw_string_draw_characters` dándole la función que pinta cada
  carácter con su sombra, y avisa a la consola de que ha terminado.
- Sin rectángulo, el texto se coloca en la **ventana de los textos** (una
  zona guardada junto a la de la pantalla, medida desde la esquina de esta).
  Lo que se recorta nunca empieza antes de la pantalla ni acaba después.
- No se pinta nada si la caché de caracteres no está lista, si el texto está
  vacío o mientras dos indicadores globales lo impiden.
- Las coordenadas del bitmap de caracteres se pasan en píxeles o divididas
  por su tamaño, según la marca «lineal» del bitmap.
- Como en Halo 1, hay un contador `magic_number` que empieza en 12 y sube con
  cada texto pintado; no se ha visto a nadie leerlo.

### Mutaciones

47 líneas rotas a propósito: 46 detectadas.

### Sin comprobar

- Que `magic_number` empiece en 12: las pruebas siempre fijan su valor. El
  12 se ha leído del ejecutable.

## 2026-10-02 — Un carácter entra en el bitmap de la caché

Una función. De 633 funciones se pasa a 634, el 5,63 % de las del juego (el
2,890 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cache_hardware_format_character` | `0x0001f5c0` | `rasterizer/rasterizer_text.c` |

### Lo que se sabe

- Copia el dibujo de un carácter al bitmap de 256×256 del que se pintan, si
  no está ya. El nombre y los campos de la caché (dónde lee, dónde escribe,
  la posición siguiente y la altura de la fila) son los de Halo 1.
- Los dibujos se colocan **en filas**, de izquierda a derecha y separados un
  píxel; cada fila es tan alta como su dibujo más alto. Al llegar abajo se
  vuelve a empezar por arriba.
- Los 512 sitios se usan **en orden, como un anillo**: se libera siempre el
  más antiguo. Se liberan los dibujos que el nuevo va a pisar (todos los de
  abajo al volver arriba; los de la franja que ocupa la fila al crecer) y,
  si el anillo está lleno, uno más.
- Si se ha liberado algún sitio, antes de escribir se **espera a la consola**,
  por si aún está pintando desde ese trozo del bitmap.
- Alrededor del dibujo se escribe un marco de un píxel de **blanco
  transparente** (`0x0FFF`), para que el filtrado de la textura no mezcle
  dibujos vecinos.
- Después se pide la textura del bitmap a la caché de texturas, como en
  otros sitios del juego.

### Mutaciones

70 líneas rotas a propósito: las 70 detectadas. Dos pidieron casos nuevos:
la caché empezando en la columna o la fila 0, con la textura con margen
delante para ver lo que se escribiría fuera.

## 2026-10-02 — Pintar un carácter y su sombra

Dos funciones. De 631 funciones se pasa a 633, el 5,62 % de las del juego
(el 2,852 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_draw_character` | `0x0001f3a0` | `rasterizer/rasterizer_text.c` |
| `rasterizer_draw_character_with_dropshadow` | `0x0001f490` | `rasterizer/rasterizer_text.c` |

### Lo que se sabe

- `rasterizer_draw_character_with_dropshadow` es **la función que pinta de
  verdad**: la que se le da al recorrido de un texto cuando no se está
  midiendo. Busca el carácter (y si no lo conoce, lo pide y vuelve a
  buscarlo), hace que su dibujo esté en el bitmap de la caché y lo pinta
  desde ahí.
- La **sombra** es el mismo dibujo pintado antes, con el color de la sombra,
  desplazado hacia abajo y a la derecha tanto como la escala (un píxel a
  escala 1). Solo hay sombra si su color tiene algo de opacidad.
- Un carácter se pinta como un **rectángulo de cuatro esquinas**; cada una
  lleva dónde va en pantalla, de qué píxel del bitmap de la caché sale y el
  color. El tamaño en pantalla va por la escala; el del bitmap, no.
- Antes de pintar, las cuatro esquinas se le pueden pasar a una función
  (guardada en cómo se dibujan los textos, en `+0x34`, con un dato suyo en
  `+0x38`): puede cambiarlas, y decir que el carácter no se pinte.
- Los nombres de las dos funciones son los de Halo 1.

### Mutaciones

57 líneas rotas a propósito: las 57 detectadas.

## 2026-10-02 — Hito 100: la caché de los caracteres que se pintan

Dos funciones, en un archivo nuevo. De 629 funciones se pasa a 631, el
5,60 % de las del juego (el 2,844 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_text_cache_initialize` | `0x0001eff0` | `rasterizer/rasterizer_text.c` |
| `rasterizer_text_cache_flush` | `0x0001f2b0` | `rasterizer/rasterizer_text.c` |

También el global `hardware_character_cache` (`0x004b62a0`).

### Lo que se sabe

- Los caracteres no se pintan desde la caché de píxeles de las fuentes, sino
  desde **un solo bitmap de 256×256** (16 bits por píxel, 4 por canal) al
  que se van copiando. Ese bitmap no tiene memoria propia para los píxeles:
  solo existe como textura.
- En él caben **512 caracteres** a la vez. Cada sitio guarda qué carácter
  tiene y dónde están sus píxeles en el bitmap; son los «sitios de dibujo»
  que ya usaba `font_cache.c`, y en el juego van a continuación de esta
  estructura.
- La estructura es la misma que la de Halo 1 (de ahí los nombres), con el
  bitmap y la cantidad de sitios al doble.
- Al empezar, todos los sitios quedan libres y el siguiente carácter irá al
  píxel (1, 1). Si no se puede crear el bitmap, lo que había se queda como
  estaba; si no se puede crear su textura, el bitmap no se devuelve.
- Vaciar la caché deja libres todos los sitios y les quita el suyo a los
  caracteres que lo tenían.

### Mutaciones

27 líneas rotas a propósito: las 27 detectadas.

## 2026-10-02 — Crear un bitmap

Una función. De 628 funciones se pasa a 629, el 5,58 % de las del juego (el
2,832 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitmap_2d_new` | `0x001358e0` | `bitmaps/bitmaps.c` |

### Lo que se sabe

- Los bitmaps que crea el propio juego (no los que vienen en el mapa) piden
  su memoria a la consola: `0x74` bytes para el bitmap y, aparte, la de sus
  píxeles. Es la pareja de `bitmap_delete`, que devuelve las dos.
- El bitmap empieza por la firma `bitm`, tiene profundidad 1 y lleva la
  marca de «hecho por el juego». Además se le ponen solas tres marcas: si su
  ancho y su alto son potencias de dos, si su formato es comprimido (los tres
  DXT) y si es de paleta (el formato 17).
- Con la marca del bit 11 se crea **sin memoria para los píxeles**: es lo que
  usa la caché de caracteres, cuyo bitmap solo existe como textura.
- Si no hay memoria para el bitmap, no hay bitmap; si no la hay para los
  píxeles, el bitmap se queda sin ellos.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas.

## 2026-10-02 — Las líneas de un texto: `draw_string.c`, completo

Una función. De 627 funciones se pasa a 628, el 5,57 % de las del juego (el
2,824 % de sus bytes). Con ella están las 19 funciones de `draw_string.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_draw_characters` | `0x0013f0e0` | `text/draw_string.c` |

Era la función a la que llamaba la que mide un texto; ya no hace falta
suplantarla.

### Lo que se sabe

- Es la que **reparte un texto en líneas** dentro de un rectángulo. Para
  cada línea: calcula dónde empieza, mide cuánto cabe
  (`draw_string_measure_line`), la coloca a la izquierda, a la derecha o al
  centro, y la dibuja (`draw_string_draw_line`) si su línea base queda por
  encima del borde inferior.
- Una línea empieza en el borde izquierdo del rectángulo más una **sangría**:
  una para la primera línea y otra para las demás.
- Con **tabuladores**, lo que va tras un tabulador empieza en su parada y
  acaba en la siguiente; un tabulador de más no hace nada. Las paradas se
  miden desde el borde izquierdo de la pantalla, no del rectángulo.
- Si lo medido no cabe, sigue en la línea de debajo, desde la misma parada.
  Tras un fin de línea se baja por debajo de la columna que más haya bajado.
- La altura de una línea es lo que sube y lo que baja la fuente, más su
  interlineado, más el interlineado extra que se pida.
- A la derecha, el texto se pega al borde derecho menos el margen de la
  fuente (sin escala); al centro, se reparte lo que sobra.
- Al acabar deja, si se le pide, dónde ha terminado el texto.
- De la fuente se conoce un campo más: el margen con que empieza una línea.

### Herramientas

- Una función suplantada que devuelve una estructura de 8 bytes (`pair`)
  puede dar sus dos mitades.

### Mutaciones

74 líneas rotas a propósito: las 74 detectadas.

## 2026-10-02 — Dibujar una línea de texto

Una función. De 626 funciones se pasa a 627, el 5,57 % de las del juego (el
2,794 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_draw_line` | `0x0013f700` | `text/draw_string.c` |

### Lo que se sabe

- Recorre los caracteres de una línea, desde uno hasta antes de otro, y con
  cada uno llama a la función que se le da (la que lo pinta, o la que mide).
- Cada carácter se **recorta** contra dos rectángulos, que pueden no darse;
  sin ninguno, lo que se dibuja va de -32768 a 32767 en las dos direcciones.
  A la función se le dice qué trozo del carácter se ve: dónde queda, cuánto
  mide y cuánto se ha quitado por la izquierda y por arriba. Si no se ve
  nada, no se la llama.
- El carácter se coloca donde va la línea, más dónde empiezan sus píxeles y
  el ajuste con el anterior; en vertical, lo que sus píxeles suben desde la
  línea. Después la línea avanza lo que diga la fuente, se haya dibujado el
  carácter o no.
- No se dibujan los caracteres hasta el `0x1F`, ni los que la caché no
  tiene.
- Los **dibujos del juego** (los caracteres privados, como los botones del
  mando) se pintan en blanco: del color solo se les deja la opacidad.

### Herramientas

- Una función de pega del arnés (`Mock`) puede decir qué se compara de cada
  llamada (`record`), para cuando recibe direcciones de la pila.

### Mutaciones

82 líneas rotas a propósito: 80 detectadas. Cuatro pidieron casos nuevos
(texto más allá de 640×480 sin rectángulos, y una fuente del recorrido
distinta de la de los textos).

### Sin comprobar

- Dos mutaciones son equivalentes: tratar un «no es un número» de otra
  forma en la comprobación de que queda sitio y en la de que el carácter se
  ve. Los bordes nunca llegan a ser «no es un número» (un rectángulo así no
  los cambia), y el ancho y el alto recortados tampoco.

## 2026-10-02 — Cuánto texto cabe en una línea

Una función. De 625 funciones se pasa a 626, el 5,56 % de las del juego (el
2,765 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_measure_line` | `0x0013ef40` | `text/draw_string.c` |

### Lo que se sabe

- La función va tomando caracteres mientras quepan antes del borde derecho,
  y devuelve **dos cosas**: el ancho de lo que cabe y el índice del carácter
  que va después. Las devuelve en dos registros (`eax` y `edx`), como hace el
  compilador del juego con una estructura de 8 bytes.
- Lo que ocupa un carácter es su avance, más dónde empiezan sus píxeles, más
  el ajuste con el carácter anterior (*kerning*), todo por la escala.
- **El primer carácter cabe siempre**, aunque no quepa: así una línea nunca
  se queda vacía.
- Si el texto se parte donde puede (el bit 0 de las opciones), al llegar a un
  carácter que no cabe la línea vuelve al último sitio donde podía partirse.
  Si no hay tal sitio, la línea acaba ahí mismo, con ese carácter dentro pero
  sin contar su ancho.
- Si el texto solo se parte donde él dice, los caracteres siguen pasando por
  encima del borde y no se cuentan en el ancho.
- De la cabecera de un carácter se conocen ahora tres campos más: el avance
  y dónde están sus píxeles respecto al sitio en que se dibuja (a la derecha
  y hacia arriba).

### Herramientas

- El arnés compara funciones que devuelven una estructura de 8 bytes (tipo
  `pair` en `symbols/functions.csv`), y `source/` se compila devolviéndolas
  en registros, como el juego.

### Mutaciones

51 líneas rotas a propósito: las 51 detectadas.

## 2026-10-02 — Medir un texto

Cuatro funciones. De 621 funciones se pasa a 625, el 5,55 % de las del
juego (el 2,751 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_initialize` | `0x0013e8a0` | `text/draw_string.c` |
| `draw_string_bounds_character` | `0x0013e8e0` | `text/draw_string.c` |
| `draw_string_get_bounds` | `0x0013e9c0` | `text/draw_string.c` |
| `draw_string_get_characters_bounds` | `0x0013ea60` | `text/draw_string.c` |

También el global `global_real_argb_white` (`0x004686cc`, en
`math/real_math.c`) y `draw_string_bounds` (`0x004e7394`).

### Lo que se sabe

- Al empezar, los textos se dibujan con la primera fuente, en blanco, sin
  sombra, sin tabuladores y a la izquierda.
- Lo que se sabe de cómo se dibujan los textos ocupa `0x64` bytes, no
  `0x60`: al final hay dos sangrías, la de la primera línea y la de las
  demás.
- **Medir un texto es recorrerlo como si se dibujara**, pero dándole a la
  función que lo recorre (`0x0013f0e0`, aún sin descompilar) otra cosa que
  hacer con cada carácter: en vez de pintarlo, agrandar un rectángulo hasta
  que quepa. El rectángulo empieza «al revés» (`0x7FFF` arriba y a la
  izquierda, `-0x8000` abajo y a la derecha) y cada carácter lo estira.
- De cada carácter se le dice a esa función, entre otras cosas: por dónde va
  el recorrido, la fuente, el carácter, su color, dónde queda lo que se ve
  de él, cuánto mide y la escala. Son 14 argumentos, y la función los quita
  ella misma de la pila.
- Además del rectángulo, medir da **dónde acaba el texto**: un sitio de un
  píxel de ancho y tan alto como una línea de la fuente (lo que sube y lo
  que baja desde la línea).
- El texto del juego se convierte primero en caracteres de 32 bits (2047
  como mucho), cambiando cada barra `|` y su letra por la orden que es. El
  juego repite ese trozo en cada función que recibe un texto; aquí es una
  sola función en línea.

### Mutaciones

63 líneas rotas a propósito: las 63 detectadas.

## 2026-10-02 — Cargar una fuente: `fonts.c`, completo

Dos funciones. De 619 funciones se pasa a 621, el 5,51 % de las del juego
(el 2,738 % de sus bytes). Con ellas están las 6 funciones de `fonts.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_load_work` | `0x001222d0` | `text/fonts.c` |
| `font_load` | `0x001223a0` | `text/fonts.c` |

### Lo que se sabe

- Una fuente es un archivo de `z:\fonts\`, con el nombre que se le da (31
  caracteres como mucho). Se abre para leer de él a saltos.
- Su **cabecera** son `0x1C4` bytes que están a `0x200` del principio del
  archivo, y se leen tal cual en el sitio de la fuente. Empieza por la firma
  `0xF0000001`.
- Cargarla es una **tarea en dos pasos**, como las demás de disco: en el
  primero se abre el archivo (si no se puede, la fuente deja de estar
  cargándose y la tarea acaba) y en el segundo se lee la cabecera.
- Quien la pide puede esperar a que acabe. Si espera y lo leído no empieza
  por la firma, el juego **olvida las fuentes** que tenía apuntadas en las
  preferencias y las escribe en el disco en el momento: lo mismo que hace la
  caché de caracteres cuando una cabecera no es válida.
- La tarea tiene más prisa cuando se espera por ella que cuando no.

### Mutaciones

42 líneas rotas a propósito: las 42 detectadas.

## 2026-10-02 — Los píxeles de un carácter: `font_cache.c`, completo

Dos funciones. De 617 funciones se pasa a 619, el 5,49 % de las del juego
(el 2,729 % de sus bytes). Con ellas están las 28 funciones de
`font_cache.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_decode_pixels` | `0x00122610` | `text/fonts.c` |
| `font_character_get_pixels` | `0x00140960` | `text/font_cache.c` |

### Lo que se sabe

- En el archivo de una fuente los píxeles de un carácter están **comprimidos
  por tramos**. Cada píxel es un color de 12 bits y una opacidad; en el
  archivo la opacidad tiene 3 bits y al descomprimir pasa a 4 (repitiendo el
  bit bajo). Cada byte es una de cuatro cosas, según sus dos bits altos:
  - `00`: tantos píxeles transparentes como digan los otros 6 bits; con
    cero, un **color nuevo**, que viene en los dos bytes siguientes y cuenta
    él mismo como un píxel;
  - `01`: tantos píxeles opacos;
  - `10`: un píxel con su opacidad y, detrás, de 0 a 5 opacos o
    transparentes, según los tres bits bajos;
  - `11`: dos píxeles, cada uno con su opacidad.
  El color es blanco hasta que se pone otro.
- La función cuenta los píxeles aunque no se le dé dónde escribirlos: sirve
  para saber cuántos son.
- Para dibujar un carácter se piden sus píxeles esperando por ellos, y se
  descomprimen desde su bloque de la caché.

### Mutaciones

54 líneas rotas a propósito: las 54 detectadas.

## 2026-10-02 — Los caracteres que se esperan

Dos funciones. De 615 funciones se pasa a 617, el 5,48 % de las del juego
(el 2,709 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_cache_frame` | `0x001406b0` | `text/font_cache.c` |
| `font_character_precache` | `0x001414d0` | `text/font_cache.c` |

### Lo que se sabe

- Antes de dibujar un texto el juego pregunta por cada carácter si está, con
  sus píxeles. Si no está, lo pide **sin esperar** y lo apunta en una lista
  de «esperados» (256 como mucho), una sola vez por carácter y fuente.
- Cada fotograma la caché envejece un tic, el contador de fotogramas avanza,
  y se vuelve a pedir cada carácter esperado; el que ya está, con sus
  píxeles, se borra de la lista. Así un texto nuevo tarda unos fotogramas en
  tener todas sus letras, sin parar el juego.
- Eran las dos funciones sin nombre que llamaban el bucle principal y
  `draw_string_precache_characters`. La lista era el campo sin nombre de
  `font_cache_globals`.

### Sin comprobar

- Apuntar el carácter cuando la lista de esperados está llena: sin la
  comprobación se escribiría fuera de la lista.

### Mutaciones

32 líneas rotas a propósito: 31 detectadas (cuatro porque la prueba se
detiene: el juego roto pide más veces de las previstas), y la otra es la de
«Sin comprobar».

## 2026-10-02 — Pedir un carácter y sus píxeles

Seis funciones. De 609 funciones se pasa a 615, el 5,46 % de las del juego
(el 2,694 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_request` | `0x001407d0` | `text/font_cache.c` |
| `font_character_pixels_request` | `0x00140870` | `text/font_cache.c` |
| `font_character_get_header` | `0x00140900` | `text/font_cache.c` |
| `font_cache_block_wait` | `0x001412c0` | `text/font_cache.c` |
| `font_character_pixels_new` | `0x00141350` | `text/font_cache.c` |
| `font_cache_read` | `0x00141480` | `text/font_cache.c` |

### Lo que se sabe

- Quien dibuja pide un carácter con dos banderas: **esperar** (bit 0) y
  **darlo a conocer si no se conoce** (bit 1). Sin esperar, cada petición da
  un paso más de la carga y contesta si ya está listo; esperando, da vueltas
  hasta que lo está. En los dos casos se apunta que se usó en este fotograma.
- Los píxeles van aparte: cuando el carácter está listo se le busca sitio en
  la caché (tantos bytes como dice su cabecera), se apunta el bloque y una
  tarea los lee del archivo de la fuente, desde donde dice la cabecera (el
  campo `pixels_offset`, que no tenía nombre). Si no hay sitio, no hay
  píxeles.
- Cada vez que se piden los píxeles de un carácter, su bloque queda marcado
  como recién usado, para que la caché no lo suelte.
- De las banderas con las que se piden los píxeles solo cuenta la primera.
- `font_cache_read` es la lectura del archivo de una fuente que las funciones
  del hito anterior llevaban dentro.

### Mutaciones

56 líneas rotas a propósito: las 56 detectadas.

## 2026-10-02 — Leer un carácter del archivo de su fuente

Seis funciones. De 603 funciones se pasa a 609, el 5,41 % de las del juego
(el 2,666 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_read_location` | `0x00140dc0` | `text/font_cache.c` |
| `font_character_header_validate` | `0x00140e40` | `text/font_cache.c` |
| `font_character_location_wait` | `0x00140f00` | `text/font_cache.c` |
| `font_character_read_header` | `0x00140f50` | `text/font_cache.c` |
| `font_character_header_wait` | `0x00140fc0` | `text/font_cache.c` |
| `font_character_update` | `0x00141020` | `text/font_cache.c` |

### Lo que se sabe

- Un carácter se prepara en cuatro pasos, uno cada vez que se le da una
  vuelta (`font_character_update`), y cada lectura la hace una tarea del hilo
  de las tareas:
  1. se pide leer **dónde está** el carácter: 4 bytes de una tabla del
     archivo de la fuente, en `0x400 + 4 × carácter`;
  2. cuando eso llega, se pide leer **lo que el archivo dice de él**: 16
     bytes en `(sitio + 0x4040) × 16`;
  3. cuando llega, se comprueba y el carácter queda listo.
- Los estados y campos del carácter ya tienen nombre: nuevo, leyendo su
  sitio, sitio leído, leyendo su cabecera y listo.
- Si quien pide el carácter quiere esperar, la lectura se pide con más
  prioridad (6 en vez de 2), a la tarea se le sube la prioridad y el juego se
  queda cediendo el turno hasta que termina.
- La cabecera de un carácter dice su ancho y su alto (128 como mucho) y
  cuántos bytes ocupan sus píxeles en el archivo (de 1 a 4.096). Si no
  cumple, se recorta y además **el juego deja de fiarse de las fuentes del
  disco duro**: apunta en las preferencias que no hay ninguna copiada y las
  escribe en el acto.
- El archivo abierto de una fuente está en su hueco (`font.file`, que era un
  campo sin nombre).

### Cambiado

- Los estados `_font_character_state_unknown0…3` y los campos sin nombre de
  `struct font_character` tienen ahora los nombres de arriba.

### Mutaciones

59 líneas rotas a propósito: las 59 detectadas.

## 2026-10-02 — Un carácter nuevo

Una función. De 602 funciones se pasa a 603, el 5,35 % de las del juego (el
2,644 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_new` | `0x00140b80` | `text/font_cache.c` |

### Lo que se sabe

- Cuando hace falta un carácter que el juego no conoce, se le busca sitio
  entre los 512. Si no queda ninguno, se recorre la lista y se elige, de los
  que no están en uso, **el que lleva más fotogramas sin usarse**; se olvida y
  su sitio pasa al nuevo. Si todos están en uso, no hay carácter.
- El carácter nuevo empieza en el estado 0 (no se sabe nada de él), sin
  píxeles ni hueco entre los dibujos, con el fotograma actual como último
  uso, y se apunta en la tabla hash con su índice.
- Si el carácter ya se conocía, se devuelve el que había.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas (una porque la prueba se
detiene: el índice roto no existe).

## 2026-10-02 — Crear la caché de los caracteres

Cuatro funciones. De 598 funciones se pasa a 602, el 5,34 % de las del juego
(el 2,633 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_characters_new` | `0x001409d0` | `text/font_cache.c` |
| `font_cache_new` | `0x001410d0` | `text/font_cache.c` |
| `font_cache_block_deleted` | `0x001411e0` | `text/font_cache.c` |
| `font_cache_block_locked` | `0x00141290` | `text/font_cache.c` |

### Lo que se sabe

- El juego sabe de 512 caracteres a la vez, y de 256 «predichos» (los que
  están a punto de dibujarse). La tabla hash que los encuentra tiene 1.024
  listas y guarda 4 bytes con cada clave: el índice del carácter.
- Los píxeles de los caracteres viven en una caché de **30.720 bytes** que se
  pide a la consola, en páginas de 16 bytes, con 512 bloques como mucho. Es
  una caché del mismo tipo que la de texturas (`lruv_cache`), con el mismo
  modo de elegir hueco.
- Un bloque no puede salir de la caché mientras sus píxeles se están leyendo
  del disco. Cuando la caché suelta un bloque, el juego espera a que esa
  lectura termine —contando las tareas pendientes en cada vuelta— y entonces
  deja al carácter sin píxeles y borra el bloque.
- Una lista del juego ocupa su cabecera (0x4C bytes), sus elementos y un bit
  por elemento.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Olvidar un carácter

Dos funciones. De 596 funciones se pasa a 598, el 5,31 % de las del juego
(el 2,620 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_delete` | `0x00140cd0` | `text/font_cache.c` |
| `font_character_is_busy` | `0x00140d50` | `text/font_cache.c` |

- `struct font_character` (0x38 bytes): lo que el juego sabe de un carácter
  de una fuente; `struct font_cache_block`; y la tabla `font_glyph_slots`
  (`0x004b62b0`, 512 huecos).
- `font_cache_globals` tiene un campo más: el número de fotograma
  (`0x0054d588`).

### Lo que se sabe

- De cada carácter el juego guarda su clave (fuente y carácter), en qué paso
  de la carga está (cinco estados; el último, listo), el bloque de la caché
  con sus píxeles, el último fotograma en que se usó y su hueco entre los
  dibujos que se están pintando.
- Olvidar un carácter es: dejar libre su hueco entre los dibujos (y quitárselo
  al carácter que lo tuviera apuntado), sacar sus píxeles de la caché, quitarlo
  de la tabla hash y borrarlo de la lista.
- Un carácter no se puede olvidar ahora si se usó en este fotograma o en el
  anterior, si está a medio cargar (estados 1 y 3), o si tiene píxeles en la
  caché que no pueden salir de ella.

### Mutaciones

25 líneas rotas a propósito: 24 detectadas a la primera y la otra con un caso
nuevo (más de 256 caracteres: el índice es de 16 bits).

## 2026-10-02 — Vectores de bits

Cinco funciones. De 591 funciones se pasa a 596, el 5,29 % de las del juego
(el 2,613 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `highest_bit_set` | `0x0013e3a0` | `memory/bit_vector.c` (nuevo) |
| `bits_to_count` | `0x0013e3c0` | `memory/bit_vector.c` |
| `bit_vector_or` | `0x0013e3e0` | `memory/bit_vector.c` |
| `bit_vector_count_bits` | `0x0013e420` | `memory/bit_vector.c` |
| `bit_vector_highest_bit_set` | `0x0013e530` | `memory/bit_vector.c` |

- `tests/test_bit_vector.py`.

### Lo que se sabe

- Un vector de bits son números de 32 bits seguidos. Contar los bits
  encendidos y buscar el más alto solo miran, del último número, los bits que
  son del vector.
- Con un vector de cero bits las dos funciones miran el número que hay
  **antes** del vector: cuentan sus bits, o dan un índice negativo. Es del
  juego y se reproduce.
- `bits_to_count` da cuántos bits hacen falta para distinguir tantas cosas
  (uno como poco); `highest_bit_set`, el bit más alto de un número (0 si no
  tiene ninguno).

### Sin comprobar

- Dos de las máscaras con las que se cuentan los bits: los bits que quitaría
  una máscara más estrecha son siempre 0 en ese paso (dos mutaciones).
- En `bits_to_count`, que la cuenta sea «mayor que 1» o «mayor que 2»: con 2
  sale 1 de las dos formas.

### Mutaciones

38 líneas rotas a propósito: 35 detectadas, y las tres de «Sin comprobar».

## 2026-10-02 — UTF-8, y encontrar un carácter de una fuente

Seis funciones. De 585 funciones se pasa a 591, el 5,25 % de las del juego
(el 2,596 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `utf8_string_to_wchar_string` | `0x00140440` | `text/unicode.c` |
| `utf8_encode_character` | `0x001405a0` | `text/unicode.c` |
| `wchar_string_to_utf8_string` | `0x00140650` | `text/unicode.c` |
| `font_character_find` | `0x00140b20` | `text/font_cache.c` |
| `font_character_key_hash` | `0x00141090` | `text/font_cache.c` |
| `font_character_key_compare` | `0x001410b0` | `text/font_cache.c` |

### Lo que se sabe

- Los textos del juego están guardados en UTF-8 y se pasan a caracteres de
  16 bits al usarlos. Un carácter mal escrito (un byte de continuación
  suelto, un principio sin sus continuaciones) se salta, y se sigue desde el
  byte siguiente. De un carácter de más de 16 bits se guardan los 16 bajos:
  los emoji y demás salen como otro carácter. No se rechazan las formas
  demasiado largas ni los sustitutos.
- Al revés, un carácter se escribe en 1 a 4 bytes; si no cabe entero se corta
  por donde quepa, así que el texto puede acabar con un carácter a medias.
- La clave de un carácter en la tabla hash es el par (hueco de la fuente,
  carácter), y su hash es tres veces el hueco más el carácter. Lo que la
  tabla guarda con la clave es el índice de lo que el juego sabe del
  carácter.

### Cambiado

- `character_is_private_use` pasa de `font_cache.c` a `unicode.c`: está entre
  las funciones de caracteres, antes de las de UTF-8.
- `utf8_string_to_wchar_string` era la función sin nombre que usaba
  `unicode_string_list.c`.
- La tabla de los caracteres de `font_cache_globals` es una `struct
  hash_table`.

### Sin comprobar

- Las máscaras de los bits del primer byte de un carácter de 2, 3 o 4 bytes:
  el bit de más que dejaría pasar una máscara más ancha es siempre 0 en un
  principio válido (tres mutaciones). Lo mismo con el bit 6 de un byte de
  continuación.
- Seguir leyendo continuaciones después de una mala, y avanzar siempre un
  solo byte: el texto que sale es el mismo, porque las continuaciones sueltas
  se saltan.

### Mutaciones

66 líneas rotas a propósito: 58 detectadas a la primera, dos más con casos
nuevos (sin sitio para escribir, nada se escribe tampoco antes del destino),
y las seis de «Sin comprobar».

## 2026-10-02 — La tabla hash

Cinco funciones. De 580 funciones se pasa a 585, el 5,19 % de las del juego
(el 2,572 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `hash_table_new` | `0x0013e1a0` | `memory/hash_table.c` (nuevo) |
| `hash_table_clear` | `0x0013e210` | `memory/hash_table.c` |
| `hash_table_add` | `0x0013e270` | `memory/hash_table.c` |
| `hash_table_find` | `0x0013e2d0` | `memory/hash_table.c` |
| `hash_table_remove` | `0x0013e320` | `memory/hash_table.c` |

- `tests/test_hash_table.py`.

### Lo que se sabe

- Es una tabla de tamaño fijo: una cabecera de 0x3C bytes, las listas (una
  por cada resto del hash) y, detrás, todas las entradas que puede llegar a
  tener, cada una con su clave, su hash, el enlace a la siguiente y los datos
  que se guardan con la clave. Las entradas libres están en otra lista.
- Añadir no mira si la clave ya estaba, y la nueva entrada queda la primera
  de su lista. Si no quedan entradas libres, no se añade.
- Buscar compara primero los hashes y luego las claves, con la función que
  se dio al crear la tabla. Quitar solo compara las claves.
- No es la tabla hash de Halo 1 (que crece): los nombres son propios.
- La usa la caché de caracteres de las fuentes (1.024 listas, 4 bytes de
  datos por clave) y otros tres sitios del juego.

### Mutaciones

50 líneas rotas a propósito: las 50 detectadas.

## 2026-10-02 — La caché de los caracteres: el final

Cuatro funciones. De 576 funciones se pasa a 580, el 5,15 % de las del juego
(el 2,557 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `character_is_private_use` | `0x00140420` | `text/font_cache.c` (nuevo) |
| `font_characters_dispose` | `0x00140aa0` | `text/font_cache.c` |
| `font_cache_dispose` | `0x00141190` | `text/font_cache.c` |
| `font_cache_block_get_address` | `0x00141450` | `text/font_cache.c` |

- `struct font_cache_globals` (`0x0054d570`) y `tests/test_font_cache.py`.

### Lo que se sabe

- El juego guarda aparte los caracteres que tiene listos para dibujar: una
  tabla para encontrarlos, dos listas con lo que sabe de cada uno, y una
  caché (del mismo tipo que la de texturas) con sus píxeles. Las dos
  funciones sin nombre que `shell.c` llamaba al salir eran las que lo
  sueltan todo.
- Los píxeles de un bloque de la caché están en la base de la caché más su
  primera página por el tamaño de página.
- Los caracteres de `0xE000` a `0xF8FF` son los de uso privado de Unicode.

### Mutaciones

30 líneas rotas a propósito: las 30 detectadas.

## 2026-10-02 — Las fuentes cargadas

Tres funciones. De 573 funciones se pasa a 576, el 5,11 % de las del juego
(el 2,548 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_get` | `0x001224c0` | `text/fonts.c` (nuevo) |
| `font_get_line_height` | `0x00122540` | `text/fonts.c` |
| `font_get_kerning` | `0x00122570` | `text/fonts.c` |

- `struct font` (0x1D0 bytes) y la tabla `font_slots` (`0x004e2920`): los
  diez huecos para fuentes. Era el bloque sin nombre que `shell.c` borraba al
  salir.
- `tests/test_fonts.py`.

### Lo que se sabe

- Hay diez huecos para fuentes. Una fuente se carga con una tarea del hilo
  de las tareas; quien la pide mientras se carga le sube la prioridad a esa
  tarea y espera a que termine.
- La altura de una línea es la suma de tres medidas de la fuente (lo que sube,
  lo que baja y el interlineado); sin fuente, 10.
- El ajuste entre dos letras (*kerning*) solo existe para caracteres de 8
  bits que no sean el 0: hasta 96 pares ordenados, y un bit por cada carácter
  que empieza algún par, para no buscar en balde.
- Al buscar un par el juego mira el primer hueco de la lista antes de
  comprobar que haya alguno: con cero pares puede dar el ajuste de lo que
  haya en ese hueco. Se reproduce tal cual.

### Sin comprobar

- El hueco número 10 (uno más de los que hay): el juego lee lo que haya
  detrás de la tabla.
- Los caracteres de más de 8 bits: sin la comprobación tampoco encuentran
  par, porque los de los pares son de 8 bits (dos mutaciones).
- Comprobar que quedan pares antes de mirar el primero: da lo mismo, por lo
  dicho arriba.

### Mutaciones

35 líneas rotas a propósito: 25 detectadas a la primera, seis más con casos
nuevos (pares con el carácter 0, un par cuyo bit no está, y algo que parece
un par justo detrás de los pares), y las cuatro de «Sin comprobar».

## 2026-10-02 — El color del texto

Dos funciones. De 571 funciones se pasa a 573, el 5,09 % de las del juego
(el 2,538 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_set_color_exact` | `0x0013eb60` | `text/draw_string.c` |
| `draw_string_set_color` | `0x0013ec70` | `text/draw_string.c` |

### Lo que se sabe

- El juego **no dibuja texto casi blanco**: si las tres partes del color
  (rojo, verde y azul) pasan de 0,68, las escala para que la menor quede en
  0,68. El blanco puro se queda en un gris claro. La opacidad no se toca.
- Después cada parte se recorta a entre 0 y 1. Una parte que no es un número
  se deja como está, y basta una así para que no se escale nada.
- `draw_string_set_color` era la función sin nombre a la que llamaba
  `draw_string_set_font_and_format`.

### Sin comprobar

- Que las partes tengan que pasar de 0,68 o basta con llegar: con una parte
  justo en 0,68 la escala es 1 y no cambia nada.

### Mutaciones

34 líneas rotas a propósito: 32 detectadas a la primera, una más con casos
nuevos (una parte que no es un número y las otras casi blancas), y la otra es
la de «Sin comprobar».

## 2026-10-02 — Cómo se dibuja el texto

Siete funciones. De 564 funciones se pasa a 571, el 5,07 % de las del juego
(el 2,522 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_set_tab_stops` | `0x0013eb20` | `text/draw_string.c` |
| `draw_string_set_shadow_color` | `0x0013ed50` | `text/draw_string.c` |
| `draw_string_set_font` | `0x0013ed90` | `text/draw_string.c` |
| `draw_string_set_font_and_format` | `0x0013edb0` | `text/draw_string.c` |
| `draw_string_precache_string` | `0x0013ee20` | `text/draw_string.c` |
| `draw_string_precache_characters` | `0x0013eeb0` | `text/draw_string.c` |
| `draw_string_precache` | `0x0013ef30` | `text/draw_string.c` |

- `struct draw_string_globals` (`0x004e73a0`, 0x60 bytes): cómo se dibuja el
  texto. Sustituye a la variable suelta `draw_string_flags`, que era su
  segundo campo.

### Lo que se sabe

- El juego guarda en un solo sitio cómo dibuja el texto: la fuente, las
  banderas, el estilo, la alineación, el color, si hay sombra y de qué
  color, y hasta 16 tabuladores. Los nombres de las funciones que lo fijan
  son los de Halo 1 (`draw_string_set_font`, `…_set_tab_stops`…).
- Fijar la fuente espera a que esté cargada.
- Antes de dibujar un texto el juego prepara sus caracteres: lo pasa a
  caracteres de 32 bits traduciendo las órdenes de barra y letra (2.047
  como mucho; lo que sobra se corta), lo recorre con el iterador y pide a la
  fuente cada carácter que se dibuja. Dice si pudo con todos; aunque uno
  falle, sigue con los demás.
- Una barra seguida de una letra que no es una orden se queda como barra, y
  la letra se dibuja.

### Sin comprobar

- Con qué estilo, alineación y colores se hace el iterador al preparar los
  caracteres: no cambian qué caracteres salen, y el iterador está en la pila
  de la función (cuatro mutaciones).

### Mutaciones

50 líneas rotas a propósito: 46 detectadas (una porque la prueba se detiene
al leer un color que no hay), y las cuatro de «Sin comprobar».

## 2026-10-02 — Empezar a recorrer un texto

Una función. De 563 funciones se pasa a 564, el 5,01 % de las del juego (el
2,505 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `text_iterator_new` | `0x0013f470` | `text/draw_string.c` |

- Las variables `draw_string_flags` (`0x004e73a4`) y `font_slot_indices`
  (`0x004e28f4`).

### Lo que se sabe

- El iterador guarda la fuente (su número y la fuente misma, que se pide a
  la función que espera a que esté cargada), la alineación, y los colores del
  texto y de su sombra ya como números de 32 bits.
- Cada parte de un color (de 0 a 1) se multiplica por 255 y se **corta** lo
  que queda tras la coma, sin redondear; una parte fuera de rango pisa a las
  demás, porque no se recorta.
- La sombra nunca es más opaca que el texto: su opacidad es la menor de las
  dos. Sin sombra, su color es 0.
- Si las líneas se parten donde se puede lo dice el bit 0 de las banderas de
  cómo se dibuja el texto.
- La función `0x00322b0c`, que aparece ocho veces, es la de la biblioteca de
  C que pasa un real a entero.

### Sin comprobar

- Quedarse con el byte bajo de la opacidad antes de juntarla con las otras
  partes: lo que sobra se pierde al desplazar.

### Mutaciones

29 líneas rotas a propósito: 28 detectadas (una de ellas, que no compilaba,
se repitió bien hecha), y la otra es la de «Sin comprobar».

## 2026-10-02 — Dónde está cada píxel de un bitmap: el 5 % del juego

Seis funciones. De 557 funciones se pasa a 563: **el 5,00 % de las del
juego** (el 2,495 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `real16_to_real` | `0x00135880` | `bitmaps/bitmaps.c` |
| `bitmap_format_get_bits` | `0x001358c0` | `bitmaps/bitmaps.c` |
| `bitmap_delete` | `0x001359d0` | `bitmaps/bitmaps.c` |
| `bitmap_2d_get_pixel_address` | `0x00135a30` | `bitmaps/bitmaps.c` |
| `bitmap_3d_get_pixel_address` | `0x00135af0` | `bitmaps/bitmaps.c` |
| `bitmap_cube_map_get_pixel_address` | `0x00135c00` | `bitmaps/bitmaps.c` |

### Lo que se sabe

- Los píxeles de un bitmap están seguidos: un mipmap detrás de otro y, en
  cada uno, las filas una detrás de otra. En uno de tres dimensiones van
  además las capas seguidas, y en un mapa cúbico las seis caras de cada
  mipmap. La dirección de un píxel es la de los píxeles más los bits que hay
  antes de él, entre 8 redondeando hacia cero.
- De un mipmap al siguiente cada medida es la mitad, sin bajar del mínimo: 1,
  o 4 en un bitmap comprimido. El fondo de uno de tres dimensiones es su byte
  bajo y no baja de 1.
- Los números reales de 16 bits del juego no son los habituales: el signo es
  el bit alto, el **exponente son los 5 bits bajos** y la fracción está en
  medio. No se distingue ningún número, ni el cero.
- Borrar un bitmap es sacarlo de la caché de texturas, soltar su textura y,
  si lo hizo el juego (el bit 8 de sus banderas, que no tenía nombre),
  devolver a la consola la memoria de sus píxeles y la suya.

### Sin comprobar

- En `real16_to_real`, tomar el número con signo o sin él: los bits de más
  se pierden al desplazar.
- Que la mitad sea «mayor o igual» o «mayor» que el mínimo, y el fondo de 2
  o 3 capas: el resultado es el mismo de las dos formas (dos mutaciones).

### Mutaciones

68 líneas rotas a propósito: 62 detectadas a la primera (dos de ellas porque
el número real que sale no cabe en uno de 32 bits y la prueba se detiene),
tres más con casos nuevos (un píxel antes del primero, con 4 bits por píxel),
y las tres de «Sin comprobar».

## 2026-10-02 — Recorrer un texto, y cuánto ocupan los píxeles de un bitmap

Siete funciones. De 550 funciones se pasa a 557, el 4,94 % de las del juego
(el 2,470 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `text_iterator_next` | `0x0013f5a0` | `text/draw_string.c` (nuevo) |
| `character_is_private_picture` | `0x0013f660` | `text/draw_string.c` |
| `bitmap_pixels_size` | `0x00136490` | `bitmaps/bitmaps.c` (nuevo) |
| `bitmap_mipmap_pixel_count` | `0x001364e0` | `bitmaps/bitmaps.c` |
| `bitmap_row_size` | `0x001365a0` | `bitmaps/bitmaps.c` |
| `bitmap_dimensions_mipmap_size` | `0x00136600` | `bitmaps/bitmaps.c` |
| `bitmap_dimensions_size` | `0x001366d0` | `bitmaps/bitmaps.c` |

- `tests/test_draw_string.py`.
- La tabla `bitmap_format_bits_per_pixel` (`0x00453550`): los bits por píxel
  de los 24 formatos.

### Lo que se sabe

- Un texto se dibuja recorriéndolo con un iterador (44 bytes) sobre
  caracteres de 32 bits. Cada paso da una de cinco cosas: un carácter que se
  dibuja, un fin de línea (`\r`), un tabulador, un cambio de alineación
  (`0xE405` a `0xE407`) o el final. El salto de línea (`\n`) se pasa sin
  más. De cada carácter que se dibuja se apunta si la línea puede acabar
  antes de él, cuando el texto se parte en líneas.
- Los caracteres de `0xE000` a `0xE3FF` son dibujos propios del juego (los
  botones del mando y demás), salvo `0xE000`–`0xE002`, `0xE004`, `0xE008` y
  `0xE106`–`0xE131`.
- Un mipmap mide la mitad que el anterior y nunca menos de 1. Un bitmap
  comprimido (el bit 1 de sus banderas, que no tenía nombre) redondea ancho
  y alto a múltiplos de 4. Un mapa cúbico tiene seis caras. Del fondo solo se
  mira el byte bajo.
- Los bytes salen de multiplicar píxeles por bits y dividir entre 8
  redondeando hacia cero; los de una fila se pueden alinear a una potencia de
  dos y son un número de 16 bits.
- `bitmap_row_size` ya se usaba desde la caché de texturas como función sin
  descompilar.

### Sin comprobar

- El límite inferior de los dibujos propios (`0xE000`): los tres primeros
  están excluidos de todas formas, así que no cambia nada (tres mutaciones).
- Que un tamaño reducido sea «mayor que 1» o «mayor que 0», y lo mismo con
  el fondo: con 1 el resultado es 1 de las dos formas (dos mutaciones).
- Dividir entre 8 o desplazar 3 bits los bytes de una fila: el producto
  nunca es negativo (dos mutaciones). En el total de un bitmap sí puede
  serlo, cuando la cuenta de píxeles pasa de 32 bits, y está probado.

### Mutaciones

121 líneas rotas a propósito: 113 detectadas a la primera, una más con
casos nuevos (cuentas de píxeles que desbordan), y las siete de «Sin
comprobar».

## 2026-10-02 — Dónde puede acabar una línea de texto

Seis funciones. De 544 funciones se pasa a 550, el 4,88 % de las del
juego (el 2,440 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `binary_search_4byte` | `0x0013dd70` | `memory/binary_search.c` |
| `usnprintf` | `0x0013fb50` | `text/unicode.c` |
| `unicode_escape_character` | `0x0013fbd0` | `text/unicode.c` |
| `character_can_break_line` | `0x0013fd20` | `text/unicode.c` |
| `character_cannot_start_line` | `0x0013fde0` | `text/unicode.c` |
| `character_cannot_end_line` | `0x00140260` | `text/unicode.c` |

- Las variables `unicode_escape_note_time` (`0x0055e730`) y
  `white_space_ranges` (`0x004536cc`).

### Lo que se sabe

- En los textos del juego una barra «|» seguida de una letra es una orden:
  `|n` fin de línea, `|t` tabulador, `|l`, `|r` y `|c` alinear a la
  izquierda, a la derecha o al centro lo que sigue (son los caracteres
  `0xE405`, `0xE406` y `0xE407`), y `||` la barra misma. Una letra que no es
  ninguna de esas se anota, como mucho una vez por minuto, y en esta versión
  la nota no va a ningún sitio.
- Una línea puede acabar detrás de un carácter si es un espacio (doce rangos
  de Unicode) o si él o el siguiente son de escritura china, japonesa o
  coreana, y además el carácter puede acabar línea y el siguiente puede
  empezarla.
- No pueden empezar línea 80 caracteres: lo que cierra (paréntesis, comillas,
  puntos y comas) y las letras pequeñas del japonés. No pueden acabarla 26:
  lo que abre. Están los 106 en el código del juego como un salto por casos;
  aquí son una lista.
- `usnprintf` es `usnzprintf` sin poner el final: si el texto no cabe, queda
  sin terminar.
- La búsqueda binaria de números de 4 bytes da a la función que compara los
  números mismos, no punteros a ellos.

### Sin comprobar

- El último de los doce rangos de espacios (`0x3000`): es además un carácter
  de escritura asiática, así que la línea puede acabar tras él con el rango
  o sin él.
- `usnprintf` con `%c` y `%s`, como en `usnzprintf`; y lo que devuelve, que
  el juego no usa (aquí no devuelve nada).

### Mutaciones

162 líneas rotas a propósito, entre ellas cada uno de los 106 caracteres:
161 detectadas, y la otra es la primera de «Sin comprobar».

## 2026-10-02 — Crear un perfil y buscarlo por su nombre: `player_profile.c`, completo

Dos funciones. De 542 funciones se pasa a 544, el 4,83 % de las del juego (el
2,403 % de sus bytes). Con ellas están las 11 funciones de
`player_profile.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `player_profile_create` | `0x001a03a0` | `saved games/player_profile.c` |
| `player_profile_find` | `0x001a0560` | `saved games/player_profile.c` |

- `_wcsicmp` en la biblioteca de C de las pruebas (solo distingue las letras
  de ASCII).

### Lo que se sabe

- Un perfil nuevo se crea así: se crea su archivo en el disco duro con el
  nombre; se hace un perfil de invitado; se le quita la marca de «por
  defecto»; se le dan **al azar** cuatro colores (de los 16 primeros), el
  dibujo del emblema (de 64) y su fondo (de 32), con la segunda semilla del
  juego; se le pone el nombre (31 caracteres como mucho); y se escribe. Si
  no se puede escribir, se borra el archivo.
- El primer argumento de `player_profile_create` no se mira.
- Buscar un perfil por su nombre es recorrer los perfiles del disco duro y
  los de serie y comparar sin distinguir mayúsculas de minúsculas. Con el
  nombre vacío el perfil es uno de invitado, y no hay archivo.
- La lista de perfiles se pide con sitio para 4.096, en la pila: la función
  reserva más de 16 KB.

### Sin comprobar

- Nombres con letras que no sean de ASCII: la comparación de la biblioteca
  de las pruebas no es la de la consola.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Escribir el estado de una partida guardada: `saved_game_file_tasks.c`, completo

Una función. De 541 funciones se pasa a 542, el 4,81 % de las del juego (el
2,381 % de sus bytes: la función no está en la lista de Ghidra con la que se
cuentan los bytes, y no suma). Con ella están las 20 funciones de
`saved_game_file_tasks.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_write_work` | `0x002adb10` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El archivo del estado se escribe en cuatro pasos, uno cada vez que el hilo
  de las tareas le da el turno: (0) se abre y se escriben **ceros** donde va
  el principio; (1) se escribe el resto, detrás; (2) se escribe el
  principio; (3) se escribe la firma de los dos. Así un archivo que no se
  llegó a escribir entero no tiene un principio que valga.
- Si no hay principio que escribir, la tarea solo hace sitio: si el archivo
  ya es tan grande como hace falta, ha terminado bien; si no, escribe ceros
  al principio y al final de donde va el resto, y una firma de ceros.
- Los ceros salen del búfer de las tareas, a trozos de lo que quepa.
- Si en el paso 0 falla empezar la firma, el paso no avanza y la tarea vuelve
  a abrir el archivo la vez siguiente, sin cerrar el anterior. Es del juego.
- El paso es uno de los cuatro: el juego salta por una tabla sin comprobarlo.

### Sin comprobar

- En el paso 0, avanzar por lo escrito o por el tamaño del búfer: solo
  difieren en el último trozo, y ahí termina el bucle de las dos formas.
- Un búfer de tareas de tamaño cero con principio que escribir: el juego no
  termina nunca.

### Mutaciones

66 líneas rotas a propósito: 63 detectadas a la primera; dos más tras hacer
más frecuentes los fallos de la consola en las pruebas (800 casos); la otra
es la primera de «Sin comprobar».

## 2026-10-02 — Leer el estado de una partida guardada, paso a paso

Una función. De 540 funciones se pasa a 541, el 4,80 % de las del juego (el
2,381 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_read_work` | `0x002ad7c0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El archivo del estado de una partida guardada tiene un principio, el
  resto y, detrás, la firma de 20 bytes de los dos. El hilo de las tareas lo
  lee en tres pasos, uno cada vez que le da el turno: abre el archivo y lee
  el principio; lee el resto; y lee la firma y la compara con la que la
  consola ha ido haciendo de lo leído. Solo si coinciden la lectura salió
  bien.
- Si no se le da dónde dejar el resto, lo lee a trozos al búfer de las
  tareas (`async_buffer_globals`), tantos bytes como quepan cada vez: sirve
  para comprobar la firma sin quedarse con los datos.
- `async_buffer_globals` apunta además quién lo usó el último y si el búfer
  no es el de siempre (su cuarto campo, que faltaba).
- El progreso que ve el jugador es lo leído entre el total (principio,
  resto y firma); al acabar, bien o mal, es 1.
- Si quien pidió la lectura la cancela (el segundo byte del resultado), el
  paso siguiente cierra todo y termina.
- Los tamaños son números sin signo y el juego los pasa a reales como si
  tuvieran signo, sumando 4.294.967.296 a los negativos. Con la precisión
  simple con la que corre el juego esa suma se redondea, y el resultado no
  es el mismo que convirtiendo el número de una vez: hay que hacerlo igual.

### Sin comprobar

- Que el trozo que se lee no pase de lo que queda, escrito de otra forma
  (`>=` lo que queda más uno): es lo mismo salvo con un resto de 4 GB.

### Mutaciones

54 líneas rotas a propósito: 53 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El índice que un archivo tiene ahora: `saved_game_files.c`, completo

Una función. De 539 funciones se pasa a 540, el 4,79 % de las del juego (el
2,358 % de sus bytes). Con ella están las 48 funciones de
`saved_game_files.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_current_index` | `0x002170e0` | `saved games/saved_game_files.c` |

### Lo que se sabe

- El índice de un archivo deja de valer cuando la memoria de los archivos se
  devuelve y se vuelve a pedir (al cambiar de mapa): la sal es otra. Esta
  función da el índice de ahora a partir del de antes, o −1 si ya no se sabe
  cuál es.
- Con la memoria de siempre basta mirar la sal: si es la del índice, vale.
  El juego compara la sal del índice con el número entero de la memoria, sin
  quitarle los bits de más.
- Con la memoria recién pedida se usa lo que se guardó de cada archivo (hasta
  cuatro): un archivo del disco duro es el de su tipo cuyo directorio es el
  guardado, y se vuelve a guardar con el índice nuevo. Uno de serie conserva
  su sitio y solo cambia de sal.
- Si de un archivo del disco duro no se guardó nada, el juego mira lo
  guardado sin comprobar que haya algo (un puntero nulo). Solo se libra si no
  hay ningún archivo de su tipo. Se reproduce tal cual.
- Al volver a guardar, solo se apunta el índice: lo que se sabía del archivo
  es lo que hubiera en ese hueco.

### Sin comprobar

- El caso del puntero nulo, y el de guardar un quinto archivo (escribe fuera
  de la tabla de los guardados): en el juego es memoria ajena.

### Mutaciones

30 líneas rotas a propósito: las 30 detectadas.

## 2026-10-02 — Buscar los archivos de una unidad

Una función. De 538 funciones se pasa a 539, el 4,78 % de las del juego (el
2,343 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_find_unit_files` | `0x002164c0` | `saved games/saved_game_files.c` |

- `struct saved_game_find_data`: lo que la consola cuenta de cada partida
  guardada (`XGAME_FIND_DATA`, 0x344 bytes).

### Lo que se sabe

- El juego recorre las partidas guardadas de la unidad. De cada una prueba
  los once tipos de archivo, por orden, hasta dar con el que existe en su
  directorio (`profile`, `slayer`…): ese es su tipo. El nombre que le puso
  el jugador y el idioma salen del nombre visible. Lo que sabe de ella va a
  la tabla; con la tabla llena (4.096) se deja de buscar.
- Una partida guardada sin archivo de ningún tipo no es de las del juego: se
  compone un mensaje («unrecognized saved game file found by
  XFindNextSaveGame()…») que en esta versión no se enseña en ningún sitio.
- Era la función sin nombre que llaman el bucle principal y
  `saved_game_files_update`.

### Corregido

- `saved_game_files_count_saved_games` reservaba 0x33C bytes para lo que
  cuenta la consola de una partida guardada, y son 0x344. Las pruebas no lo
  veían porque la consola es suplente; en una consola de verdad se habría
  escrito fuera. Además, como en el juego, tiene en sí misma la función que
  da la raíz de una unidad en vez de llamarla.

### Sin comprobar

- El nombre corto del archivo y el relleno de `saved_game_find_data`: el
  juego no los mira.
- El mensaje, pasado a caracteres de 8 bits: no sale de la función.

### Mutaciones

34 líneas rotas a propósito: 33 detectadas, y la otra es la primera de «Sin
comprobar».

## 2026-10-02 — Crear un archivo

Una función. De 537 funciones se pasa a 538, el 4,78 % de las del juego (el
2,321 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_create` | `0x00215c00` | `saved games/saved_game_files.c` |

- `tools/test_functions.py` admite, detrás del nombre del archivo de pruebas,
  los nombres de las funciones cuyos casos se corren; el resto del archivo se
  recorre (para que los casos sean los mismos) pero no se emula.
  `tools/mutation_check.py` los toma de `FUNCTIONS`. Una pasada de mutación
  de `saved_game_files.c` baja de más de dos minutos a unos doce segundos.

### Lo que se sabe

- Crear un archivo en el disco duro es: esperar a que se hayan buscado los
  archivos de las unidades; pedir a la consola una partida guardada nueva con
  el nombre visible (el del tipo en el idioma de los archivos, y el que puso
  el jugador); crear en su directorio el archivo vacío del tipo; escribir su
  imagen; y apuntarlo en la tabla. Si falla cualquier paso después de crear
  la partida guardada, se borra.
- Por qué no se creó queda en `saved_game_files_last_error`: 1 si ya hay
  4.096 archivos en la tabla o 4.096 partidas guardadas en el disco, 2 si
  al disco no le caben los bloques de un archivo de ese tipo, 4 si le caben
  y falló otra cosa; 0 si se creó. El 1 es el mismo de cuando no queda
  nombre libre, así que `_saved_game_files_error_no_name` pasa a llamarse
  `_saved_game_files_error_too_many`.
- De la ruta que da la consola solo se guardan 19 caracteres.

### Sin comprobar

- La comprobación de que el índice recién hecho no sea −1: no puede serlo
  (un índice del disco duro no tiene el bit de los de serie).
- Lo que hace el juego si no se puede hacer el nombre visible: la función que
  lo hace siempre dice que sí, el compilador lo sabe al compilar `source/`, y
  la suplente de la prueba no puede decir otra cosa.

### Mutaciones

55 líneas rotas a propósito: 53 detectadas a la primera; una más tras hacer
que la suplente del nombre visible dé un nombre distinto cada vez (el juego
borra con el segundo nombre, no con el primero); la otra es la primera de
«Sin comprobar».

## 2026-10-02 — La lista de los archivos de un tipo, y cómo se ordena

Cinco funciones. De 532 funciones se pasa a 537, el 4,77 % de las del juego
(el 2,301 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `qsort_4byte` | `0x0013de30` | `cseries/sort.c` (nuevo) |
| `qsort_2byte` | `0x0013df80` | `cseries/sort.c` |
| `short_sort_4byte` | `0x0013e0e0` | `cseries/sort.c` |
| `short_sort_2byte` | `0x0013e140` | `cseries/sort.c` |
| `saved_game_files_enumerate` | `0x00215900` | `saved games/saved_game_files.c` |

- `tests/test_sort.py`.
- En `tools/mutation_check.py`, una mutación puede llevar un cuarto dato: el
  nombre con el que sale en el resultado. Hace falta cuando el texto que se
  cambia es largo porque la línea está repetida en varias funciones.

### Lo que se sabe

- El juego ordena números de 4 y de 2 bytes con el mismo algoritmo que la
  biblioteca de C de su compilador: se pone un número en su sitio, con los
  que no van después de él antes y los demás después, y se sigue con la
  parte pequeña, dejando la grande apuntada para luego (hasta 30). Con 8
  números o menos se va llevando al final el mayor de los que quedan. Los
  nombres `qsort_4byte` y `qsort_2byte` son los de Halo 1.
- A la función que compara se le pregunta si un número va **después** de
  otro, y recibe además un dato de quien ordena. El primer argumento de pila
  de las dos funciones de ordenar no se mira.
- La lista de archivos de un tipo junta los del disco duro, los de las dos
  unidades de memoria de un mando si se da uno (las unidades 1 + 2 × mando y
  la siguiente) y, si se piden, los que vienen con el juego. Con estos
  últimos la lista se ordena: primero los de las unidades y después los de
  serie, cada grupo por su índice.
- Cada unidad tiene su tabla detrás de la anterior, pero la memoria de los
  archivos solo tiene sitio para una: la cuenta de la tabla de la primera
  unidad de memoria es el mismo número que la sal de los índices, y sus
  archivos caen en el búfer de copia. Es del juego; se reproduce tal cual.
- El mando es uno de los cuatro, ninguno (−1) o `0xFF`: el juego salta por
  una tabla sin comprobar que sea uno de ellos.

### Cambiado

- `saved_game_file_index_after` saca ella sus argumentos de la pila, como
  toda función que se le da a otra: es la que compara al ordenar la lista.

### Sin comprobar

- En las funciones de ordenar, lo que pasa con una parte de un solo número:
  no hay nada que ordenar en ella, así que da igual apuntarla o no. Lo mismo
  la comprobación de que haya al menos dos números.
- En la lista, qué unidad sigue cuando no se da mando: solo se mira el disco
  duro y la cuenta no llega a usarse.
- Un mando que no sea uno de los seis valores: en el juego es un salto a
  donde caiga.

### Mutaciones

105 líneas rotas a propósito (67 en las funciones de ordenar y 38 en la
lista): las 105 detectadas.

## 2026-10-02 — Borrar un archivo y buscarle nombre a uno nuevo

Tres funciones. De 529 funciones se pasa a 532, el 4,72 % de las del juego
(el 2,256 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `usnzprintf` | `0x0013fb70` | `text/unicode.c` |
| `saved_game_file_delete` | `0x00215e60` | `saved games/saved_game_files.c` |
| `saved_game_file_get_unused_name` | `0x00215f40` | `saved games/saved_game_files.c` |

- `_vsnwprintf` en la biblioteca de C de las pruebas.

### Lo que se sabe

- Un archivo nuevo se llama por defecto **«Halo0001»**, «Halo0002»…: el
  juego prueba números desde 1 y pregunta a la consola si ya hay una
  partida guardada con ese nombre visible; el primero que no existe es el
  nombre. Si los 4.096 están cogidos, no hay nombre y queda apuntado un
  error.
- Sin disco duro tampoco hay nombre.
- Borrar un archivo es borrar su partida guardada en la consola (por el
  nombre visible, en el idioma en que se guardó) y quitarlo de la tabla.
  Un archivo de serie solo se quita de lo que el juego sabe: no hay nada
  que borrar en ningún disco.

### Sin comprobar

- En `usnzprintf`, si a la función de la biblioteca se le da el tamaño
  entero o uno menos: el último carácter se pone a cero después.
- `usnzprintf` con `%c` y `%s`: la biblioteca de C de las pruebas toma el
  formato como texto de 8 bits y solo sirve para números. En el juego la
  función llama a la de la consola, que sí los admite.

### Mutaciones

32 líneas rotas a propósito: 31 detectadas, y la otra es la primera de
«Sin comprobar».

## 2026-10-02 — La memoria de los archivos guardados

Siete funciones del principio de `saved_game_files.c`, que empieza antes
de lo que se creía (`0x00215790`). De 522 funciones se pasa a 529, el
4,70 % de las del juego (el 2,238 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_update` | `0x00215790` | `saved games/saved_game_files.c` |
| `saved_game_files_set_string_list` | `0x002157c0` | `saved games/saved_game_files.c` |
| `saved_game_files_allocate_memory` | `0x002157e0` | `saved games/saved_game_files.c` |
| `saved_game_files_free_memory` | `0x00215810` | `saved games/saved_game_files.c` |
| `saved_game_files_new` | `0x00215880` | `saved games/saved_game_files.c` |
| `saved_game_file_information_new` | `0x00215b40` | `saved games/saved_game_files.c` |
| `saved_game_file_get_name` | `0x00215b50` | `saved games/saved_game_files.c` |

### Cambiado

- En la tabla de los que piden memoria al cargar un mapa
  (`main_game_memory_users`), la pareja `unknown_00215810` /
  `unknown_002157e0` es `saved_game_files_free_memory` /
  `saved_game_files_allocate_memory`.

### Corregido

- **Solo hay una tabla de archivos**, no una por unidad como se dijo en el
  hito 63. La memoria de los archivos mide 0x6BF00 bytes, y la cuenta sale
  exacta con: los archivos de serie (0xBEF8), una tabla de 4.096 archivos
  (0x40004), la sal de los índices (4) y el búfer de las copias (0x20000).
  Eso explica también por qué la sal estaba «donde estaría la cuenta de la
  segunda tabla» (hito 51): no hay segunda tabla. Los bits 4-7 del índice
  de un archivo siguen siendo la unidad, pero para elegir tabla solo vale
  la 0.

### Lo que se sabe

- La memoria de los archivos se pide al cargar un mapa en la configuración
  de memoria 1, y se suelta con las demás. Antes de soltarla, el juego
  guarda aparte lo que sabe de hasta cuatro archivos, para poder seguir
  contestando por ellos sin la tabla.
- La sal de los índices cambia cada vez que se pide la memoria (de 0 a
  511): así un índice de archivo anterior ya no vale después.
- El juego escribe la sal sin mirar si le dieron memoria: si no se la
  dieran, escribiría en la dirección 0x4BEFC.
- Al fijar la etiqueta de los textos, los archivos de serie se quedan en
  uno solo: el perfil por defecto.
- El nombre de un archivo de serie está dentro del propio archivo: a 8
  bytes en un perfil y a 4 en una variante de juego.
- `saved_game_files_update` es lo que el bucle principal tiene copiado
  dentro de sí (ya estaba descompilado allí, sin saber que era una función
  aparte).

### Mutaciones

57 líneas rotas a propósito: las 57 detectadas.

## 2026-10-02 — Los primeros pasos de un cambio de nombre

Los dos pasos que la máquina de estados llamaba todavía por su dirección.
De 520 funciones se pasa a 522, el 4,63 % de las del juego (el 2,222 % de
sus bytes). El cambio de nombre de un archivo guardado queda descompilado
de principio a fin.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_rename_create` | `0x002ad020` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_rename_open_copy` | `0x002ad220` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El primer paso pide a la consola una partida guardada **nueva** con el
  nombre visible nuevo («tipo: nombre», en el idioma de la consola). Si ya
  hay una con ese nombre, falla.
- Dentro crea el archivo del tipo y le da ya su tamaño (0x1E0, 0x130 o
  0x15CB8 bytes), antes de escribir nada: así se sabe que cabe.
- De la ruta que devuelve la consola el juego se queda con 19 caracteres:
  es lo que mide el directorio en lo que sabe de cada archivo. Basta
  porque la consola da rutas cortas, del estilo de `u:\0123456789AB\`.
- El paso de la copia abre `auxilary.bin` en la partida vieja para leer en
  secuencia, lo crea en la nueva y le da el mismo tamaño. Que la vieja no
  lo tenga no es un fallo: se salta la copia.
- Si algo falla ahí se cierran los dos archivos, y esta vez bien (cada uno
  comparado con -1 y marcado como cerrado), a diferencia del descuido del
  hito 63.

### Sin comprobar

- Si el directorio se copia con 20 o con 19 caracteres: el último se pone
  a cero después.

### Mutaciones

50 líneas rotas a propósito: 49 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El trabajo de la tarea que escribe un archivo

Una función: lo que hace el hilo de tareas para escribir un archivo de los
jugadores. De 519 funciones se pasa a 520, el 4,62 % de las del juego (el
2,193 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_write_work` | `0x002acde0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El trabajo de una tarea puede decir que no ha terminado (devuelve 0), y
  entonces el hilo vuelve a llamarlo. Así se hace el cambio de nombre: un
  paso por llamada, hasta que la máquina de estados dice que ha acabado.
- Solo entonces se escribe el archivo, con su firma, en la ruta que da la
  descripción.
- Si el cambio de nombre falla, la tarea termina sin escribir nada y con el
  error del resultado a 0: lo único que dice que salió mal es que no se
  marca como bien hecha.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas.

## 2026-10-02 — Los pasos del cambio de nombre

Dos funciones más de `saved_game_file_tasks.c`. De 517 funciones se pasa a
519, el 4,61 % de las del juego (el 2,185 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_rename_step` | `0x002ad5f0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_get_path` | `0x002ad760` | `saved games/saved_game_file_tasks.c` |

### Cambiado

- En `struct saved_game_file_description`, `unknown9C` es `state` (por dónde
  va el cambio de nombre) y `unknown7C` es `debug`.

### Lo que se sabe

- Cambiar de nombre un archivo no se hace de una vez: es una máquina de
  siete estados, y cada vez que el hilo de tareas le da tiempo hace un paso.
  En orden: crear la partida guardada con el nombre nuevo, escribir su
  imagen, abrir el archivo del estado en la vieja y en la nueva, leer un
  trozo, escribirlo (y volver a leer mientras quede), y borrar la vieja.
- Si la partida vieja no tiene archivo de estado que copiar, se salta la
  copia.
- Si un paso falla se dice por qué (2 en los pasos sueltos, 4 en la copia),
  se cierran los dos archivos y se borra la partida nueva. El estado se
  queda donde estaba.
- Un archivo que no cambia de nombre nace ya en el último estado.
- Queda una rama de depuración: con un indicador puesto, el archivo se
  escribiría en `t:\blam` en vez de en su directorio. Nada de lo
  descompilado pone ese indicador.

### Mutaciones

41 líneas rotas a propósito: las 41 detectadas.

## 2026-10-02 — Cambiar de nombre un archivo guardado

Cuatro funciones más de `saved_game_file_tasks.c`. De 513 funciones se
pasa a 517, el 4,59 % de las del juego (el 2,171 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_copy_read` | `0x002ad380` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_copy_write` | `0x002ad400` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_rename_finish` | `0x002ad480` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_copy_close` | `0x002ad5b0` | `saved games/saved_game_file_tasks.c` |

- Función de la consola que se suple: `XDeleteSaveGame` (`0x00331055`).

### Corregido

- **Los bits del índice de un archivo**, que en los hitos anteriores estaban
  mal o sin aclarar: los bits 0-3 son el **tipo** del archivo y los bits
  4-7 la **unidad** en la que está (0 el disco duro, las demás las
  tarjetas). Las tablas de archivos son una por unidad. Las constantes se
  llaman ahora `SAVED_GAME_FILE_INDEX_TYPE_MASK` y
  `SAVED_GAME_FILE_INDEX_UNIT_*`.
- En `struct saved_game_file_description`, los campos de `+0x84` a `+0x98`
  tienen nombre: los dos archivos de la copia, el búfer, el tamaño y hasta
  dónde se ha leído y escrito.

### Lo que se sabe

- La consola nombra el directorio de una partida guardada a partir de su
  nombre visible, así que cambiar de nombre un archivo es **hacer otro y
  copiarlo**: se copia en trozos de 128 KB, con un búfer que está en la
  memoria de los archivos (`+0x4BF00`).
- Al terminar bien, se borra la partida guardada con el nombre viejo y el
  juego apunta el nuevo, en el idioma de la consola. Al terminar mal, lo
  que se borra es la partida con el nombre nuevo. En los dos casos se
  devuelve el turno de operaciones.
- **Descuido del juego** al cerrar los dos archivos de la copia: el segundo
  se da por abierto si no vale 0 (el primero, si no vale -1), y después de
  cerrarlo se marca como cerrado el primero otra vez. No tiene
  consecuencias porque la descripción no se vuelve a usar.

### Sin comprobar

- Que el nombre visible empiece vacío antes de componerlo: la función que
  lo compone lo vacía también.

### Mutaciones

43 líneas rotas a propósito: 41 detectadas a la primera, una más con un
caso nuevo (un nombre viejo que llena todo su sitio) y la otra es la de
«Sin comprobar».

## 2026-10-02 — Los archivos de los jugadores van firmados

Las dos funciones que leen y escriben un archivo entero de los jugadores.
De 511 funciones se pasa a 513, el 4,55 % de las del juego (el 2,151 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_read_signed` | `0x002adf70` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_write_signed` | `0x002ae090` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- Detrás de los bytes de cada archivo (perfil, variante, lista) van 20
  bytes de firma, que la consola calcula con la clave del juego
  (`XCalculateSignature…`).
- Al leer, el juego calcula la firma de lo leído y la compara con la del
  archivo. **Solo si coinciden da la lectura por buena.** Un archivo
  tocado a mano o corrupto se lee entero pero se da por malo.
- El indicador que acompaña a la lectura y a la escritura dice si la firma
  vale solo para esta consola o para cualquiera.
- Un resultado lleva apuntado hasta dónde se llegó: 3 al leer y 2 al
  escribir, que se cambian por 0 al acabar bien.
- La lectura es la de `file_read` copiada dentro de la función, con una
  diferencia: lo que la consola no escriba en «bytes leídos» si falla es la
  dirección de la referencia del archivo, y eso es lo que se sumaría a la
  posición.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Las tareas del archivo del estado de una partida

Las dos funciones que `saved_game_files.c` llamaba todavía por su
dirección. De 509 funciones se pasa a 511, el 4,54 % de las del juego (el
2,134 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_read_task_new` | `0x002ada70` | `saved games/saved_game_file_tasks.c` |
| `saved_game_state_write_task_new` | `0x002adec0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El estado de una partida guardada (`auxilary.bin`) se lee y se escribe en
  dos partes, la cabecera y el resto, cada una con su búfer y su tamaño.
  Las dos van en los 34 bytes de datos de una tarea genérica de `async.c`,
  de prioridad 2.
- Al escribir, si no se da cabecera, la tarea lo lleva apuntado.
- El progreso empieza en 0 para una lectura y en -1 para una escritura; el
  error empieza en 4 en las dos.
- Con esto está descompilado todo el camino desde que `game_state.c` pide
  guardar o cargar hasta que la tarea queda en la lista. Lo que hace
  después el hilo de tareas (`0x002ad7c0` y `0x002adb10`) no lo está.

### Mutaciones

32 líneas rotas a propósito: las 32 detectadas.

## 2026-10-02 — Las tareas de los archivos de los jugadores

Cómo llegan al hilo de tareas las lecturas y escrituras de los archivos de
los jugadores. De 502 funciones se pasa a 509, el 4,52 % de las del juego
(el 2,123 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_custom_task_work` | `0x001a17b0` | `cseries/async.c` |
| `async_task_add_custom` | `0x001a17d0` | `cseries/async.c` |
| `saved_game_file_read_work` | `0x002accd0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_read_task_new` | `0x002acd60` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_write_task_new` | `0x002acee0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_new` | `0x002acf40` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_new_renamed` | `0x002acf80` | `saved games/saved_game_file_tasks.c` |

- Archivos nuevos: `saved games/saved_game_file_tasks.c` y
  `tests/test_saved_game_file_tasks.py`.
- `struct async_custom_task` y `struct saved_game_file_description`.

### Cambiado

- Las cuatro funciones que `saved_game_files.c` llamaba por su dirección
  (`unknown_002acd60`, `unknown_002acee0`, `unknown_002acf40`,
  `unknown_002acf80`) tienen nombre y están descompiladas.

### Lo que se sabe

- Además de las tareas fijas de `async.c` (leer, escribir, copiar…), hay
  una genérica: sus parámetros son una función y hasta 35 bytes de datos
  para ella, que se copian. Así cualquier módulo puede mandar trabajo al
  hilo de tareas sin que `async.c` lo conozca.
- Si los datos no caben, esa función no hace nada y devuelve la dirección
  de los datos en lugar de un identificador de tarea: quien la llama solo
  comprueba que no sea -1, así que no se entera.
- La lectura y la escritura de un archivo de los jugadores son tareas de
  esas, de prioridad 2. Sus datos son un búfer, un tamaño, un indicador y
  el resultado, que es también la bandera de «terminado» de la tarea.
- El trabajo de una lectura empieza dando el archivo por no leído, con el
  error 4, y lee del archivo cuya ruta va dentro del propio resultado.
- De un archivo que se escribe se le dice al hilo qué archivo es y lo que
  el juego sabe de él; si además cambia de nombre, el nombre nuevo, el
  directorio que tenía y el turno de operaciones.

### Mutaciones

66 líneas rotas a propósito: las 66 detectadas.

## 2026-10-02 — Los perfiles de los jugadores

Nueve funciones, casi todas de los perfiles. De 493 funciones se pasa a
502, el 4,46 % de las del juego (el 2,099 % de sus bytes). **Se pasa de
las 500 funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `player_appearance_valid` | `0x00153750` | `saved games/player_profile.c` |
| `player_profile_load` | `0x001a0540` | `saved games/player_profile.c` |
| `player_profile_write` | `0x001a0660` | `saved games/player_profile.c` |
| `player_profile_write_async` | `0x001a0680` | `saved games/player_profile.c` |
| `player_color_get` | `0x001a06a0` | `saved games/player_profile.c` |
| `player_profile_get_unknownEC` | `0x001a06f0` | `saved games/player_profile.c` |
| `player_profile_new` | `0x001a07b0` | `saved games/player_profile.c` |
| `player_profile_read` | `0x001a0850` | `saved games/player_profile.c` |
| `async_set_file_position` | `0x001a08d0` | `cseries/async.c` |

- `struct player_appearance` (16 bytes) y más campos de
  `struct player_profile`; `tests/test_player_profile.py`.

### Lo que se sabe

- Un perfil nuevo se llama «Guest». El de serie del hito anterior es el
  mismo con otro nombre.
- El aspecto de un jugador son cuatro colores (de 18, o -1 para ninguno),
  un quinto valor de 0 a 3, un emblema con 64 primeros planos y 32 fondos,
  y cuatro indicadores. Por defecto: colores 10, 0, 0 y 10.
- Al leer un perfil el juego corrige lo que no puede ser: un aspecto
  imposible se pone entero a cero, y el nombre se corta a 31 caracteres.
  **Lo hace aunque la lectura falle**, sobre el perfil que le dieron.
- Lo que se escribe de un perfil empieza en su nombre (`+0x08`), que es lo
  que la función de escritura compara para saber si el archivo cambió de
  nombre.
- Los colores de los jugadores están en la etiqueta de globales
  (`+0x160`); sin mapa cargado, o con un índice que no existe, el color es
  blanco.
- Un perfil guarda cuatro números (`+0xEC`); una función da el mayor de
  los que valen y cuál de los cuatro es. Qué son no se sabe (los niveles
  alcanzados en cada dificultad, quizá).

### Sin comprobar

- Si el indicador de un perfil nuevo se pone con `|=` o con `=`: la memoria
  se acaba de poner a cero.

### Mutaciones

72 líneas rotas a propósito: 71 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los archivos que vienen con el juego

Cinco funciones pequeñas: cuatro de `saved_game_files.c`, al que ya solo
le faltan dos, y la primera de los perfiles. De 488 funciones se pasa a
493, el 4,38 % de las del juego (el 2,078 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_make_directory` | `0x00216440` | `saved games/saved_game_files.c` |
| `saved_game_files_add_default_profile` | `0x00216760` | `saved games/saved_game_files.c` |
| `saved_game_files_add_built_in_variant` | `0x00217080` | `saved games/saved_game_files.c` |
| `cached_saved_game_file_delete` | `0x002172a0` | `saved games/saved_game_files.c` |
| `player_profile_set_defaults` | `0x001a0750` | `saved games/player_profile.c` |

- Archivos nuevos: `saved games/player_profile.h` y `player_profile.c`.
- En la herramienta de pruebas, `clean` en un búfer: una función que dice
  qué parte de lo que queda en él se compara.

### Lo que se sabe

- Los archivos «de serie» son hasta 101, cada uno con su tipo delante. El
  juego los llena al arrancar con un perfil por defecto y con las variantes
  de juego que trae.
- El perfil por defecto se llama «Default» («Player» en japonés,
  «Predeterminada» en español…), en el idioma de los archivos.
- De un perfil se sabe el nombre (`+0x08`, 31 caracteres), un indicador que
  se pone al crearlo (`+0x04`, bit 0) y seis bytes con valores por defecto
  (10, 0, 0, 10 en `+0x118`; 3 en `+0x102`): qué son no se sabe todavía.
- **Rareza del juego:** al añadir una variante de serie copia a la tabla
  0x1E4 bytes de una variable local de la que solo ha rellenado 0x134. El
  resto es lo que hubiera en la pila en ese momento. No afecta a nada,
  porque de una variante solo se leen 0x130 bytes.
- De la lista corta de cuatro archivos se quita uno poniendo el último en
  su sitio.

### Sin comprobar

- Si el nombre de un perfil tiene 31 o 30 caracteres: los nombres por
  defecto son cortos y la memoria ya está a cero.

### Mutaciones

36 líneas rotas a propósito: 35 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Arrancar la lectura y la escritura de un archivo

Las dos funciones que el hito anterior llamaba sin estar descompiladas. De
486 funciones se pasa a 488, el 4,33 % de las del juego (el 2,060 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_read_async` | `0x002162b0` | `saved games/saved_game_files.c` |
| `saved_game_file_write_async` | `0x002173d0` | `saved games/saved_game_files.c` |

- De `struct saved_game_file_result`: `progress` (`+0x108`). De
  `struct saved_game_file_information`: `language` (`+0x3D`).
- `struct built_in_saved_game_file`: un archivo de los que vienen con el
  juego, 4 bytes y los 0x1E0 de un perfil.

### Lo que se sabe

- Los archivos que vienen con el juego (los perfiles de serie) no se leen
  de ningún disco: están en memoria y la lectura es una copia, que termina
  en el acto con el progreso a 1.
- Los de una unidad los lee otro hilo, del archivo que se llama como su
  tipo (`profile`, `slayer`…) dentro de su directorio.
- Un archivo cuyo nombre visible no empieza por el de su tipo en ningún
  idioma (idioma 0xFF) no se lee.
- Lo que se escribe empieza por el nombre del archivo. Si ese nombre ya no
  es el que el juego tenía apuntado, la escritura es además un cambio de
  nombre, y para eso se coge el turno de operaciones.
- El progreso de una operación va de 0 a 1, y vale -1 antes de empezar.

### Mutaciones

47 líneas rotas a propósito: las 47 detectadas.

## 2026-10-02 — Leer y escribir un archivo de los jugadores

Tres funciones más de `saved_game_files.c`. De 483 funciones se pasa a
486, el 4,31 % de las del juego (el 2,045 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_name_unique` | `0x00216120` | `saved games/saved_game_files.c` |
| `saved_game_file_read` | `0x002161d0` | `saved games/saved_game_files.c` |
| `saved_game_file_write` | `0x00216240` | `saved games/saved_game_files.c` |

- `saved_game_files_last_error` (`0x0055c154`), y el campo `error` de
  `struct saved_game_file_result`.
- Función de la consola que se suple: `XCreateSaveGame` (`0x00330df2`).

### Lo que se sabe

- Para saber si un nombre está libre, el juego no recorre sus tablas: le
  pide a la consola que **abra** la partida guardada que se llamaría así
  (con el nombre visible, «tipo: nombre»), y el nombre está libre si la
  consola no la encuentra. La ruta que la consola devolvería se tira.
- Sin nombre, sin la memoria de los archivos preparada o sin disco duro, la
  respuesta es que no está libre.
- Leer y escribir un archivo lo hace otro hilo; estas dos funciones lo
  piden y esperan, atendiendo el sonido. Si sale mal, el código de error de
  la consola queda guardado para quien quiera enseñarlo.

### Sin comprobar

- En `saved_game_file_name_unique`, que el nombre visible empiece vacío:
  la función que lo compone lo vacía también.

### Mutaciones

29 líneas rotas a propósito: 28 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El nombre visible de un archivo guardado

Dos funciones más de `saved_game_files.c`: la que compone el nombre que la
consola enseña de un archivo y la que lo descompone. De 481 funciones se
pasa a 483, el 4,29 % de las del juego (el 2,032 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_display_name` | `0x00216da0` | `saved games/saved_game_files.c` |
| `saved_game_file_parse_display_name` | `0x00216e50` | `saved games/saved_game_files.c` |

- Globales con nombre: `saved_game_files_string_list_index` (`0x0055c158`),
  `saved_game_files_language` (`0x0055c280`) y
  `saved_game_file_name_separator` (`0x00470054`, que apunta a `": "`).

### Lo que se sabe

- En el panel de la consola un archivo se llama «tipo: nombre», por ejemplo
  «Slayer: mi variante», con el tipo en el idioma en que se guardó. Hasta
  127 caracteres.
- Si no hay idioma, o la etiqueta de textos no tiene el del tipo, el nombre
  visible es solo el que puso el jugador.
- Para recuperar el nombre del jugador a partir del visible, el juego
  prueba los nueve idiomas, empezando por el suyo y dando la vuelta, hasta
  que el nombre empieza por «tipo: » en alguno. Se queda con lo que sigue,
  16 caracteres como mucho, y con qué idioma era.
- Si no empieza así en ninguno, el idioma es 0xFF y el nombre son los 16
  primeros caracteres del nombre visible.
- Como varios idiomas comparten el nombre de un tipo («Slayer» es igual en
  inglés, japonés, coreano y portugués), el idioma que sale es el primero
  que coincide, no necesariamente aquel en que se guardó.

### Sin comprobar

Cuatro escrituras que el juego hace y que no cambian nada, porque el valor
ya estaba puesto: el cero final tras añadir el separador, el cero inicial
y el final del nombre del tipo al descomponer, y el cero tras copiar el
nombre encontrado.

### Mutaciones

41 líneas rotas a propósito: 37 detectadas, y las otras cuatro son las de
«Sin comprobar».

## 2026-10-02 — Los textos del juego por idioma

Cómo encuentra el juego un texto en el idioma de la consola. De 475
funciones se pasa a 481, el 4,27 % de las del juego (el 2,017 % de sus
bytes: **se pasa del 2 % de los bytes**).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `unicode_string_list_has_string` | `0x001a0120` | `text/unicode_string_list.c` |
| `unicode_string_list_get_string` | `0x001a0180` | `text/unicode_string_list.c` |
| `language_texts_load` | `0x001a01e0` | `text/unicode_string_list.c` |
| `language_texts_find` | `0x001a02d0` | `text/unicode_string_list.c` |
| `language_texts_has` | `0x001a0310` | `text/unicode_string_list.c` |
| `language_texts_get` | `0x001a0350` | `text/unicode_string_list.c` |

- Archivos nuevos: `text/unicode_string_list.h`,
  `text/unicode_string_list.c`, `tests/test_unicode_string_list.py`.

### Corregido

- **`hud_get_text` recibía un argumento de menos.** La función del juego
  toma tres cosas: el índice de la etiqueta que tiene el texto (en `eax`),
  el identificador del texto y dónde escribirlo. `main.c` solo le pasaba
  las dos últimas. Las pruebas no lo veían porque la función estaba
  suplida y el registro no se comparaba. Ahora se llama
  `unicode_string_list_get_string`, recibe el índice, y la prueba de
  `main.c` comprueba que es el que debe ser.

### Cambiado

- `unknown_001a01e0`, que `main_game.c` llamaba al cargar un mapa, es
  `language_texts_load`.

### Lo que se sabe

- Los textos de todas las etiquetas de un mapa están juntos, por idioma, en
  el archivo del mapa. La etiqueta de globales dice dónde (una tabla de
  0x1C bytes por idioma, desde `+0x188`), y se leen a memoria al cargar el
  mapa, en sectores enteros de 512 bytes.
- De cada texto se guarda su identificador y dónde empieza; los textos
  están en UTF-8 y se pasan a 16 bits al pedirlos, con 255 caracteres como
  mucho.
- Una etiqueta con textos solo dice, para cada idioma, cuántos textos son
  suyos y a partir de cuál: buscar un texto es recorrer ese tramo.
- El idioma se pregunta a la consola la primera vez que hace falta.
- Si el mismo identificador está dos veces en el tramo, vale el primero.

### Mutaciones

56 líneas rotas a propósito: 55 detectadas a la primera y la otra con un
caso nuevo (un tramo que empieza antes del primer texto).

## 2026-10-02 — Los nombres por defecto de los archivos

El nombre que recibe un archivo de cada tipo cuando el jugador no le pone
ninguno, en cada idioma. De 461 funciones se pasa a 475, el 4,22 % de las
del juego (el 1,996 % de sus bytes).

### Añadido

| Función | Dirección |
| --- | --- |
| `saved_game_file_default_name_profile` | `0x00217620` |
| `saved_game_file_default_name_slayer` | `0x00217680` |
| `saved_game_file_default_name_koth` | `0x002176e0` |
| `saved_game_file_default_name_race` | `0x00217740` |
| `saved_game_file_default_name_oddball` | `0x002177a0` |
| `saved_game_file_default_name_juggernaut` | `0x002177f0` |
| `saved_game_file_default_name_headhunter` | `0x00217840` |
| `saved_game_file_default_name_ctf` | `0x002178a0` |
| `saved_game_file_default_name_assault` | `0x00217900` |
| `saved_game_file_default_name_territories` | `0x00217960` |
| `saved_game_file_default_name_playlist` | `0x002179c0` |
| `saved_game_file_default_name_unknown` | `0x00217a20` |
| `saved_game_file_get_default_name` | `0x00217a80` |
| `saved_game_file_name_length` | `0x00217b70` |

Todas en el archivo nuevo `saved games/saved_game_file_names.c`, con
`tests/test_saved_game_file_names.py`.

- En la herramienta de pruebas, el tipo de resultado `wstring`: como
  `string`, para textos de caracteres de 16 bits.

### Lo que se sabe

- Cada tipo de archivo tiene su función, que es un `switch` del idioma.
  Las doce funciones se han escrito con un script que lee las tablas de
  salto y los textos del ejecutable, no a mano.
- El juego traduce los nombres al japonés, alemán, francés, español,
  italiano, coreano y chino, pero **no todos a todos**: «Slayer» solo
  cambia en alemán («Showdown»), francés, español, italiano y chino;
  «Juggernaut» solo en alemán («Moloch»), francés y chino. El japonés y el
  coreano solo traducen «Profile» y «Playlist».
- El portugués, el noveno idioma de los textos del juego, está en todos los
  `switch` y en todos recibe el nombre en inglés.
- Los modos `race` y `headhunter`, que no salieron en el juego, tienen sus
  nombres traducidos igual que los demás.
- Hay un duodécimo texto («Default», «Player» en japonés) que no es de
  ningún tipo de archivo: su identificador es `0x1B0006E8`.

### Sin comprobar

- Si el nombre se copia con 255 o con 256 caracteres: todos los nombres son
  cortos y después se pone el cero final.

### Mutaciones

176 líneas rotas a propósito: 174 detectadas. De las otras dos, una es la
de «Sin comprobar» y la otra señaló una constante que no se usaba
(`_language_portuguese`): se ha quitado.

## 2026-10-02 — Crear archivos y la imagen de una variante

Tres funciones de archivos que faltaban y la que escribe la imagen de una
variante de juego. De 457 funciones se pasa a 461, el 4,09 % de las del
juego (el 1,966 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_reference_create_from_path` | `0x00136710` | `tag_files/files_xbox.c` |
| `find_files` | `0x00136770` | `tag_files/files_xbox.c` |
| `file_create` | `0x001367d0` | `tag_files/files_xbox.c` |
| `saved_game_file_write_image` | `0x00216f80` | `saved games/saved_game_files.c` |

- `file_reference_set_path`, en línea en `files.h`: el juego la tiene
  copiada donde hace una referencia a partir de una ruta.

### Corregido

- **El módulo de archivos no estaba completo**, como se dijo en el hito 44.
  Las tres primeras funciones de la tabla estaban antes de `file_delete` y
  no se habían visto; aparecieron al seguir una llamada desde
  `saved_game_files.c`. ESTADO.md lo recoge, con las que quedan por mirar.

### Lo que se sabe

- `file_create` hace un archivo vacío (y vacía el que hubiera) o un
  directorio, según la referencia sea de archivo o no.
- `find_files` es el envoltorio de las tres funciones de búsqueda: llena
  una lista de referencias con lo que hay en un directorio (`*.*`), hasta
  un máximo.
- Junto a cada variante de juego guardada se escribe `SaveImage.xbx`, la
  imagen que la consola enseña en su panel: 4.096 bytes que están dentro
  del ejecutable (`0x00449038`). Los perfiles y las listas no llevan
  imagen.
- **Esa imagen no se copia al repositorio**: es un recurso del juego, no
  código. En `source/` queda declarada y vacía; las pruebas le dan el
  mismo contenido al juego y a `source/`.

### Sin comprobar

- En `file_create`, que la ruta completa se ponga a cero antes de
  escribirla entera.
- En `saved_game_file_write_image`, el cero al final de la ruta antes de
  añadirle el nombre: `csstrcat256` lo pone también.

### Mutaciones

50 líneas rotas a propósito: 48 detectadas, y las otras dos son las de
«Sin comprobar».

## 2026-10-02 — Lo que el juego sabe de cada archivo

Cinco funciones más de `saved_game_files.c`: las que leen, cambian, añaden
y quitan lo que el juego guarda de cada archivo de una unidad. De 452
funciones se pasa a 457, el 4,06 % de las del juego (el 1,947 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_name_differs` | `0x00216060` | `saved games/saved_game_files.c` |
| `saved_game_file_get_information` | `0x00216800` | `saved games/saved_game_files.c` |
| `saved_game_file_set_information` | `0x002168b0` | `saved games/saved_game_files.c` |
| `saved_game_file_add_information` | `0x00216990` | `saved games/saved_game_files.c` |
| `saved_game_file_delete_information` | `0x002169e0` | `saved games/saved_game_files.c` |

- `struct saved_game_file_information` (0x40 bytes: una ruta de 0x14, un
  nombre de 18 caracteres de 16 bits, el tipo) y
  `struct saved_game_file_table` (una cuenta y 4.096 de esas).
- `wcsncmp` en la biblioteca de C de las pruebas.

### Cambiado

- **Corrección del hito anterior:** el campo de los bits 4-7 del índice de
  un archivo no es su tipo, sino en cuál de las tablas está. Las
  constantes `SAVED_GAME_FILE_INDEX_TYPE_*` pasan a llamarse
  `SAVED_GAME_FILE_INDEX_TABLE_*`. Qué distingue una tabla de otra no se
  sabe todavía.
- `unknown_00216800` es `saved_game_file_get_information`.

### Lo que se sabe

- Las tablas empiezan en `+0xBEF8` de la memoria de los archivos
  (`0x0051ea14`) y miden 0x40004 bytes cada una.
- **La «sal» de los índices está justo donde estaría la cuenta de la
  segunda tabla** (`+0x4BEFC`). O solo hay una tabla en la práctica, o las
  dos cosas son la misma; no se puede decidir con lo descompilado.
- Si esa memoria no está preparada, el juego guarda aparte lo de hasta
  cuatro archivos (`0x0055c164`), y de ahí lee.
- Cambiar lo de un archivo no se permite mientras hay otra operación en
  curso, salvo que sea sobre ese mismo archivo y la haya pedido **otro**
  hilo.
- Cuando el cambio se hace en esos cuatro archivos guardados aparte, la
  función contesta que no lo hizo aunque lo haya hecho.
- Una tabla llena (4.096) no admite más; pero la comparación es de
  igualdad, así que con una cuenta mayor el juego escribiría fuera.

### Sin comprobar

- Al borrar el último archivo de una tabla, si se entra o no a mover los
  de detrás: no hay ninguno que mover.
- `saved_game_file_set_information` y `saved_game_file_delete_information`
  con un índice de archivo «integrado» (bit 21): el juego no lo mira.

### Mutaciones

52 líneas rotas a propósito: 49 detectadas a la primera, dos más con
casos nuevos (un puesto mayor que 4.095; una tabla con más archivos de los
que caben) y la otra es la primera de «Sin comprobar».

## 2026-10-02 — Los índices de los archivos y el archivo del estado

Siete funciones más de `saved_game_files.c`. De 445 funciones se pasa a
452, el 4,01 % de las del juego (el 1,926 % de sus bytes). **Se pasa del
4 %.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_directory` | `0x002160b0` | `saved games/saved_game_files.c` |
| `saved_game_file_index_after` | `0x002163d0` | `saved games/saved_game_files.c` |
| `saved_game_file_index_after_delete` | `0x00217300` | `saved games/saved_game_files.c` |
| `saved_game_files_take_operation` | `0x00217370` | `saved games/saved_game_files.c` |
| `saved_game_auxiliary_read` | `0x002174b0` | `saved games/saved_game_files.c` |
| `saved_game_auxiliary_write` | `0x00217520` | `saved games/saved_game_files.c` |
| `saved_game_files_count_saved_games` | `0x00217590` | `saved games/saved_game_files.c` |

- Funciones de la consola que se suplen: `GetCurrentThreadId`
  (`0x002d2234`), `XFindFirstSaveGame` (`0x00331125`), `XFindNextSaveGame`
  (`0x0033122f`) y `XFindClose` (`0x00331276`).

### Cambiado

- `saved_game_auxiliary_read` y `saved_game_auxiliary_write`, que
  `game_state.c` llamaba sin estar descompiladas, ya lo están; sus
  prototipos pasan de `game_state.h` a `saved_game_files.h`.

### Lo que se sabe

- Un archivo se nombra con un número de 32 bits: la unidad (bits 0-3), el
  tipo (4-7), el número del archivo entre los de su tipo (8-20), un
  indicador de que no está en ninguna unidad sino en la memoria del juego
  (21) y una «sal» de 9 bits (22-30) que cambia cuando cambia la lista.
- Al borrar un archivo, los que iban detrás entre los de su tipo bajan un
  puesto y reciben la sal nueva.
- En las listas, los archivos que vienen con el juego van detrás de los de
  las unidades.
- La partida guardada tiene su estado en `auxilary.bin` (sic), en el
  directorio de la partida.
- Antes de preguntar por un archivo el juego espera a que se hayan buscado
  los de las unidades (`0x0055c14c`), atendiendo el sonido mientras tanto.
- Las operaciones con archivos van de una en una: hay un único «turno»
  (`0x0055c274`) con el archivo y el hilo que lo pidió.

### Sin comprobar

- El cero al final de la ruta en `saved_game_file_get_directory`: la ruta
  que se copia ya lo trae.

### Mutaciones

67 líneas rotas a propósito: 66 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los tipos de archivo de los jugadores

Se empieza `saved_game_files.c`: los archivos que los jugadores guardan en
el disco duro y en las tarjetas de memoria (perfiles, variantes de juego y
listas). De 434 funciones se pasa a 445, el 3,95 % de las del juego (el
1,903 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_get_unit_root` | `0x00216400` | `saved games/saved_game_files.c` |
| `saved_game_file_type_from_variant` | `0x00216a50` | `saved games/saved_game_files.c` |
| `saved_game_file_type_from_listed_game` | `0x00216ab0` | `saved games/saved_game_files.c` |
| `saved_game_file_type_to_listed_game` | `0x00216b00` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_name` | `0x00216b60` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_string_id` | `0x00216bd0` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_blocks` | `0x00216c40` | `saved games/saved_game_files.c` |
| `saved_game_file_size_to_blocks` | `0x00216c70` | `saved games/saved_game_files.c` |
| `saved_game_files_hard_disk_has_blocks` | `0x00216ce0` | `saved games/saved_game_files.c` |
| `saved_game_file_get_type_path` | `0x00216d70` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_size` | `0x00217480` | `saved games/saved_game_files.c` |

- Archivos nuevos: `saved games/saved_game_files.h`,
  `saved games/saved_game_files.c`, `tests/test_saved_game_files.py`.
- En la herramienta de pruebas, un tipo de resultado nuevo, `string`: la
  función devuelve un puntero a un texto y lo que se compara es el texto,
  que no está en la misma dirección en el juego y en `source/`.
- Funciones de la consola que se suplen: `XGetDiskClusterSize`
  (`0x002d5f65`) y `GetDiskFreeSpaceExA` (`0x002d5838`).

### Lo que se sabe

- Hay once tipos de archivo, y su nombre es el del directorio donde se
  guardan: `profile`, `slayer`, `koth`, `race`, `oddball`, `juggernaut`,
  `headhunter`, `ctf`, `assault`, `territories` y `playlist`. **`race` y
  `headhunter` están en la tabla aunque Halo 2 no salió con esos modos.**
- Un perfil ocupa 0x1E0 bytes, una variante 0x130 y una lista 0x15CB8.
- El espacio se cuenta en bloques de 16 KB, como lo enseña la consola: los
  bytes del archivo más seis clústeres del disco, redondeado hacia arriba.
- Tres numeraciones distintas para los juegos y funciones para pasar de
  una a otra: la del tipo de archivo, la del motor de la variante (campo
  `+0x44`: 1 ctf, 2 slayer, 3 oddball, 4 koth, 7 juggernaut, 8 territories,
  9 assault) y el orden en que salen en la lista (slayer, oddball,
  juggernaut, koth, ctf, assault, territories).
- Esas tres funciones de conversión no comprueban nada: con un valor que
  no sea de los siete juegos, el juego salta a una dirección cualquiera.

### Sin comprobar

- Las tres conversiones con valores que no son de los siete juegos (el
  juego se cae).
- En `saved_game_file_get_type_path`, el cero que se pone al final de la
  ruta: `csstrcat256` lo pone también.

### Mutaciones

81 líneas rotas a propósito: 80 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Atender el sonido y pedir sus muestras

Cinco funciones más del sonido. De 429 funciones se pasa a 434, el 3,85 %
de las del juego (el 1,881 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_idle` | `0x00125d60` | `sound/sound_manager.c` |
| `sound_load_environment` | `0x00125df0` | `sound/sound_manager.c` |
| `sound_source_request_data` | `0x00125e60` | `sound/sound_manager.c` |
| `sound_unknown_00125ef0` | `0x00125ef0` | `sound/sound_manager.c` |
| `sound_predict` | `0x00125f10` | `sound/sound_manager.c` |

- De la etiqueta común de sonidos de un mapa se conocen tres bloques más:
  los rangos de tono (`+0x20`, 0xC bytes), las grabaciones (`+0x28`, 0x10
  bytes) y los trozos de muestras (`+0x40`, 0xC bytes).

### Cambiado

- **`main_loading_idle` se llama ahora `sound_idle`**. El nombre anterior
  era una suposición sacada de quién la llama (las esperas de la carga, de
  las cachés y de las partidas guardadas); al descompilarla resulta que lo
  que hace es atender el sonido. Cambia en ocho archivos de `source/` y en
  siete de `tests/`.
- `unknown_00125df0` es `sound_load_environment`.

### Lo que se sabe

- `sound_idle` solo hace algo dentro de una partida y si ha pasado más de
  un tick (33 ms a 30 por segundo) desde que el sonido empezó o se
  reanudó. No se vuelve a entrar mientras ya se está dentro.
- Un sonido se compone así: sonido → rangos de tono → grabaciones
  (permutaciones) → trozos de muestras. Cada nivel guarda el índice del
  primero del siguiente, y un sonido que suena guarda cuánto se ha
  desplazado en cada nivel (`+0x9C`, `+0x9D`, `+0x9E`).
- Pedir muestras es pedir un trozo a la caché de sonido (`0x00218850`), que
  contesta con bits: con el 0 o el 1 se avisa al sonido de que no las
  tiene, y el 1 es lo que devuelve la función.
- `sound_predict` pide el primer trozo de un sonido antes de que suene.
- El «ambiente» del mapa sale de una etiqueta que nombra la etiqueta de
  globales (`+0x14` de su bloque de sonido); puede no haberla.

### Mutaciones

71 líneas rotas a propósito: 69 detectadas a la primera y las otras dos
con un caso nuevo (un tick de un segundo justo, para distinguir 1000 de
1001 y de 990).

## 2026-10-02 — El sonido al cambiar de mapa

El resto de la fila del sonido en la tabla de sistemas del juego: lo que
hace al cargar un mapa, al soltarlo y al cambiar de BSP; y dos funciones
más, si hay sonido y pararlo o reanudarlo. De 424 funciones se pasa a 429,
el 3,81 % de las del juego (el 1,867 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_initialize_for_new_map` | `0x00125690` | `sound/sound_manager.c` |
| `sound_dispose_from_old_map` | `0x001257b0` | `sound/sound_manager.c` |
| `sound_initialize_for_new_structure_bsp` | `0x00125920` | `sound/sound_manager.c` |
| `sound_available` | `0x00125a60` | `sound/sound_manager.c` |
| `sound_enable` | `0x00125a90` | `sound/sound_manager.c` |

### Cambiado

- `unknown_00125690`, `unknown_001257b0` y `unknown_00125920` de la tabla
  de sistemas tienen nombre: la fila del sonido está completa.
- De un oyente (`struct sound_listener`) se sabe más: la hoja y el cluster
  del BSP donde está, y su posición (`+0x30`).

### Lo que se sabe

- Al cargar un mapa, el sonido apunta a la etiqueta común de sonidos del
  mapa (su índice está en el bloque de sonido de la etiqueta de globales,
  en `+0x20`), apunta la hora y quita los oyentes.
- Al soltar un mapa, los sonidos que suenan se marcan para apagarse en
  300 ms y el juego **espera** esos 300 ms atendiendo el sonido. Si no
  suena ninguno no espera.
- En esa espera la hora de inicio se toma como número con signo y la hora
  actual sin signo: si la consola llevara más de 24,8 días encendida, la
  espera no ocurriría. El final se guarda como real de 32 bits, así que
  pasadas 4,6 horas ya no es exacto al milisegundo.
- Al cambiar de BSP se recalcula dónde están los sonidos que tienen
  posición (los del tipo 1) y los oyentes.
- «Hay sonido» es: está preparado, hay un mapa cargado y otro indicador
  (`+0x79`) que se pone al arrancar.
- `sound_enable(0)` para todos los canales; `sound_enable(1)` los reanuda y
  apunta la hora. Cualquier otro número cuenta como «reanudar» para el
  indicador pero no toca los canales.

### Sin comprobar

- `sound_initialize_for_new_map` cuando la etiqueta de globales no dice
  nada del sonido: el juego lee de la dirección 0x20 y se cae.

### Mutaciones

113 líneas rotas a propósito: 110 detectadas a la primera, dos más con
casos nuevos (un tipo de sonido con el bit 3 puesto; cargar un mapa sin
otro cargado antes) y la otra es la de «Sin comprobar».

Las pruebas encontraron un error antes de las mutaciones: había puesto la
espera de 300 ms fuera de la condición de que suene algún sonido.

## 2026-10-02 — El arranque del sonido

Las dos funciones que preparan y desmontan el sonido al arrancar y cerrar
el juego, y dos auxiliares que usan. De 420 funciones se pasa a 424, el
3,76 % de las del juego (el 1,850 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_initialize` | `0x001253d0` | `sound/sound_manager.c` |
| `sound_dispose` | `0x00125600` | `sound/sound_manager.c` |
| `shell_memory_data_new` | `0x0011cc20` | `shell/shell.c` |
| `bit_vector_fill` | `0x0012afc0` | `cseries/cseries.c` |

- `shell_memory_allocate`, en línea en `shell.h`: el juego la tiene copiada
  dentro de cada función que pide memoria.
- Globales con nombre: `sound_initialize_count`, `sound_channels`,
  `sound_sources`, `looping_sounds`, `sound_voices`, `sound_effects` y la
  tabla `sound_configuration`.

### Cambiado

- `unknown_001253d0` y `unknown_00125600`, que el arranque y la tabla de
  sistemas del juego llamaban sin saber qué eran, son `sound_initialize` y
  `sound_dispose`.
- En `shell_memory_globals`, los campos `unknown8` y `unknownC` son `size`
  y `size_checksum`.

### Lo que se sabe

- El bloque de memoria que el juego pide al arrancar (`shell_memory_globals`)
  es de donde cada sistema toma la suya, uno detrás de otro, en direcciones
  múltiplo de 4. Con cada petición se actualiza una suma de comprobación de
  los tamaños. **No** es la memoria del estado de la partida, aunque
  funciona igual: tiene sus propios contadores.
- El sonido pide 0x20C bytes para sus globales, una tabla «sound sources»
  de 384 elementos de 0xBC bytes y 128 canales de 0x24 bytes.
- Los canales son de tres tipos, y hay 36, 36 y 1 de cada uno (73 de los
  128); el máximo de voces es 51.
- El sonido se prepara una sola vez aunque se pida varias: lleva la cuenta,
  y solo se desmonta cuando se han pedido tantos cierres como arranques.
- Si alguna de las cinco partes del sonido no se puede preparar, el sonido
  se queda sin marcar como preparado, pero la cuenta sube igual.
- Al desmontar solo se ponen a cero los punteros: la memoria no es suya.

### Sin comprobar

- `bit_vector_fill` con una cuenta de bits negativa de más de 31: el juego
  escribiría casi 4 GB.
- Que el byte `+0x7D` de los globales se ponga a cero: el bloque entero se
  acaba de poner a cero.
- El orden entre pedir la memoria de los canales y guardar el puntero de las
  fuentes; y si el primer vector de bits mide 128 bits o 160, porque los 32
  que siguen los borra el segundo.

### Mutaciones

100 líneas rotas a propósito: 94 detectadas a la primera, tres más
haciendo que las funciones suplidas dejen algo escrito en los globales, y
las demás son las de «Sin comprobar».

## 2026-10-02 — Cómo se reproduce un sonido

Las primeras funciones del sonido: las que dicen a qué distancia se oye un
sonido, con qué cono, qué volumen y qué tono. De 409 funciones se pasa a
420, el 3,73 % de las del juego (el 1,821 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_class_unknown_00124f90` | `0x00124f90` | `sound/sound_manager.c` |
| `sound_get_minimum_distance` | `0x00124fc0` | `sound/sound_manager.c` |
| `sound_get_maximum_distance` | `0x00125010` | `sound/sound_manager.c` |
| `sound_unknown_00125060` | `0x00125060` | `sound/sound_manager.c` |
| `sound_get_inner_cone_angle` | `0x001250a0` | `sound/sound_manager.c` |
| `sound_get_outer_cone_angle` | `0x00125120` | `sound/sound_manager.c` |
| `sound_get_outer_cone_gain` | `0x001251a0` | `sound/sound_manager.c` |
| `sound_definition_random_gain` | `0x00125260` | `sound/sound_manager.c` |
| `sound_definition_random_pitch` | `0x001252f0` | `sound/sound_manager.c` |
| `sound_first_local_listener` | `0x00125360` | `sound/sound_manager.c` |
| `sound_class_get` | `0x00221810` | `sound/sound_classes.c` |

- Archivos nuevos: `sound/sound_definitions.h`, `sound/sound_manager.c`,
  `sound/sound_classes.c`, `tests/test_sound.py`.
- En la herramienta de pruebas, un tipo de resultado nuevo, `real:eax`:
  dos de estas funciones devuelven un real como sus 32 bits en `eax`.

### Cambiado

- `struct players_globals` y `MAXIMUM_LOCAL_PLAYERS` pasan de `main.c` a
  `main.h`, porque el sonido también mira los jugadores de la consola.

### Lo que se sabe

- Los nombres de los archivos y de las funciones son suposiciones: Halo 1
  no tiene estas funciones.
- Cada dato de cómo suena un sonido se busca en hasta tres sitios, por este
  orden: lo que diga quien lo reproduce (la «fuente», con un indicador por
  dato), la clase del sonido (solo las distancias, si el sonido tiene el
  indicador 10 u 11) y los parámetros de reproducción del sonido.
- Los parámetros de reproducción no están en la etiqueta del sonido: están
  todos juntos en una etiqueta común del mapa (`0x0051ebd4`), 0x38 bytes
  cada uno, y el sonido solo guarda un índice.
- Las clases de sonido están en otra etiqueta, a la que se llega por la
  etiqueta de globales (bloque en `+0xC0`); cada clase mide 0x5C bytes.
- Una fuente dice los ángulos del cono como una palabra de 16 bits: la
  parte de media vuelta que son (0xFFFF es π).
- El volumen de cada reproducción es la ganancia base más una parte al azar
  de la variación, con la **segunda** semilla del juego (la que no afecta a
  la partida); el tono, al azar entre dos límites enteros.
- El oyente es el del primer jugador de la consola, si está puesto.

### Sin comprobar

- `sound_class_get` cuando la etiqueta de globales no dice nada del sonido:
  el juego lee de la dirección 4 y se cae.
- En el ángulo de una fuente, los casos de 0 y de 0xFFFF aparte, y el
  término que se multiplica por cero: dan lo mismo que la fórmula general.
- En la ganancia al azar, restar y sumar 0,0: solo cambia el signo de un
  cero.

### Mutaciones

91 líneas rotas a propósito: 81 detectadas a la primera, cuatro más con
casos nuevos (índices de etiqueta de más de 255, índice de reproducción
negativo, ángulos donde dividir no da lo mismo que multiplicar por el
inverso) y las otras seis son las de «Sin comprobar».

## 2026-10-02 — Recorrer los archivos de un directorio

`find_files_next`, lo único que faltaba de los archivos: con ella
`files.c` y `files_xbox.c` quedan completos. De 408 funciones se pasa a
409, el 3,63 % de las del juego (el 1,789 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `find_files_next` | `0x00137000` | `tag_files/files_xbox.c` |

- `struct find_files_data` (el `WIN32_FIND_DATA` de la consola), que ahora
  es parte de `struct find_files_state` (0x388 bytes en total).
- Los indicadores de una búsqueda: recursiva (bit 0), decir los directorios
  (bit 1) y decir los archivos aunque se digan los directorios (bit 2).
- Funciones de la consola que se suplen en las pruebas: `FindFirstFileA`
  (`0x00331b9b`) y `FindNextFileA` (`0x00331ca8`).

### Lo que se sabe

- La búsqueda lleva una pila de hasta 16 directorios, cada uno con su
  búsqueda abierta en la consola. Al encontrar un directorio, si es
  recursiva, lo añade a la ruta y baja; cuando un directorio se acaba,
  cierra su búsqueda, quita el último elemento de la ruta y sube.
- `.` y `..` se saltan siempre.
- Sin indicadores solo se dicen los archivos. Con el bit 1 se dicen solo
  los directorios, y con el bit 2 las dos cosas.
- Con cada archivo se puede pedir la fecha en que se escribió por última
  vez.

### Sin comprobar

- Que la ruta completa se ponga a cero antes de empezar: se escribe entera
  antes de usarla.
- Al hacer la referencia de un archivo, el juego mira si tiene el indicador
  de nombre de archivo para quitarle el último elemento; pero la acaba de
  poner a cero, así que nunca lo tiene.

### Mutaciones

60 líneas rotas a propósito: 58 detectadas, y las otras dos son las de
«Sin comprobar».

## 2026-10-02 — El final de `main_game.c` y las partes de una ruta

Lo que quedaba suelto de `main_game.c`, que queda completo, y
`file_path_split` de los archivos. De 402 funciones se pasa a 408, el
3,62 % de las del juego (el 1,762 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_path_split` | `0x00137400` | `tag_files/files.c` |
| `main_game_next_map` | `0x0018e960` | `main/main_game.c` |
| `main_game_campaign_started` | `0x0018f0a0` | `main/main_game.c` |
| `main_game_goto_main_menu` | `0x0018f1c0` | `main/main_game.c` |
| `main_game_texture_allocation_allocate` | `0x0018f260` | `main/main_game.c` |
| `main_game_texture_allocation_deallocate` | `0x0018f350` | `main/main_game.c` |

- `time` (`0x00321aae`) y `rand` (`0x00321f75`), de la biblioteca de C,
  están en la tabla de símbolos para poder suplirlas en las pruebas.

### Cambiado

- `unknown_0018f1c0`, que el bucle principal y la carga de un estado
  llamaban sin saber qué era, es `main_game_goto_main_menu`; y
  `unknown_0018f0a0` es `main_game_campaign_started`.
- Los tamaños y posiciones de la tabla de `0x0054e8e0` (los perfiles de los
  jugadores, parece) pasan de `input_xbox.c` a `input.h`, porque
  `main_game.c` también la lee.

### Lo que se sabe

- Al acabar un mapa de la campaña, el juego pregunta cuál es el siguiente
  (`0x0019c440`) y pide una partida en él con las mismas opciones, una
  semilla nueva (`time ^ rand ^ GetTickCount`) y el nombre del mapa. Si no
  hay siguiente va al menú principal, con el motivo 7.
- En una sesión de red, antes de eso se escribe un informe de 0x33B bytes
  de la partida que acaba y se manda al servidor como
  `application/x-halo2campaign`.
- Ir al menú principal es pedir una partida de la clase 3 en
  `scenarios\ui\mainmenu\mainmenu`; si ya se está en ella solo se avisa a
  la interfaz.
- Al empezar una campaña de un jugador, si está puesto el indicador de
  `0x0054e7cc`, se carga la partida guardada del único perfil que hay (la
  guarda en `+0x1F8`).
- Hay un asignador de memoria (tabla de funciones en `0x00453c40`, objeto en
  `0x0047d8e4`) cuya memoria se la presta la caché de texturas, esperando
  poco: 30 intentos y una décima de segundo.
- `file_path_split` corta una ruta en sitio: pone ceros donde empiezan el
  nombre y la extensión. No mira el primer carácter de la ruta.

### Sin comprobar

- En `file_path_split`: si la ruta tiene que medir más de 0 o más de 1 para
  recorrerla, y la comparación de `*filename!=path` al final: dan lo mismo.
- En `main_game_next_map`: si el nombre del mapa se copia con 0x104 o con
  0x103 bytes, porque después se pone el cero final.
- En el asignador, el caso en que la memoria no llega nunca y se acaba el
  tiempo: depende del reloj del procesador.

### Mutaciones

96 líneas rotas a propósito: 91 detectadas a la primera, una más con un
caso nuevo (esperar a la consola tras 30 intentos) y las otras cuatro son
las de «Sin comprobar».

## 2026-10-02 — Los mandos

`input_xbox.c`, que ya existía en Halo 1: qué mandos hay enchufados, qué
botones se pulsan y cuánto tiempo, los joysticks y la vibración. De 392
funciones se pasa a 402, el 3,57 % de las del juego (el 1,730 % de sus
bytes). **Se pasa de las 400 funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `input_initialize` | `0x001248b0` | `input/input_xbox.c` |
| `input_dispose` | `0x00124920` | `input/input_xbox.c` |
| `input_update` | `0x00124950` | `input/input_xbox.c` |
| `input_get_gamepad_state` | `0x001249a0` | `input/input_xbox.c` |
| `input_get_gamepad_type` | `0x001249d0` | `input/input_xbox.c` |
| `input_get_unit_drive_letter` | `0x001249f0` | `input/input_xbox.c` |
| `input_set_gamepad_rumbler_state` | `0x00124a40` | `input/input_xbox.c` |
| `input_update_devices` | `0x00124aa0` | `input/input_xbox.c` |
| `input_update_gamepads` | `0x00124c00` | `input/input_xbox.c` |
| `input_flush_rumble` | `0x00124ee0` | `input/input_xbox.c` |

- Archivos nuevos: `input/input.h`, `input/input_xbox.c`,
  `tests/test_input.py`.
- Funciones de la consola que se suplen en las pruebas: `XInitDevices`,
  `XGetDeviceChanges`, `XInputOpen`, `XInputClose`, `XInputGetCapabilities`,
  `XInputGetState`, `XInputSetState`.

### Cambiado

- Tres funciones que el arranque y el bucle principal llamaban sin saber
  qué eran (`unknown_001248b0`, `unknown_00124920`, `unknown_00124950`) son
  `input_initialize`, `input_dispose` e `input_update`.
- Los 0x120 bytes que se ponen a cero al empezar una partida
  (`unknown_004e61dc`) son el estado de los cuatro mandos.

### Lo que se sabe

- De cada mando se guarda, por botón, el valor (los ocho botones de la
  consola son analógicos, de 0 a 255), cuántos fotogramas y cuántos
  milisegundos lleva pulsado, y los dos joysticks.
- Un botón analógico no tiene un umbral fijo para contar como pulsado: el
  umbral le sigue. Mientras está pulsado sube hasta 0x20 por debajo del
  valor, y mientras no lo está baja hasta 0x40 por encima. Así un botón que
  tiembla cerca del umbral no se pulsa y se suelta en cada fotograma.
- Los joysticks tienen una zona muerta de 9.000 (de 32.768), y lo que queda
  se estira a todo el rango. Hacia el lado negativo el rango es de 23.768 y
  hacia el positivo de 23.767, porque el valor va de -32.768 a 32.767.
- Un fotograma de mandos nunca cuenta más de 100 ms.
- La vibración se manda solo si la consola ha acabado con la orden
  anterior, y se apaga si no hay partida, si está en pausa o si los mandos
  están «suprimidos». Tampoco vibra el mando de un jugador que no quiere
  (dos indicadores de la tabla de `0x0054e8e0`, que parece el perfil).
- El módulo también dice la letra de unidad del disco duro (`u`) y de las
  tarjetas de memoria.
- Cuando los mandos están suprimidos, el juego recibe un mando con nada
  pulsado.

### Sin comprobar

- Al subir el umbral de un botón, si se compara con `>` o con `>=`: da el
  mismo valor.

### Mutaciones

75 líneas rotas a propósito: 74 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los estados guardados en archivos

Casi todo lo que faltaba de `game_state.c`: cargar el estado del juego desde
un archivo, sea un «core» del disco o una partida guardada del disco duro.
De 382 funciones se pasa a 392, el 3,48 % de las del juego (el 1,672 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_load_saved_game` | `0x001240b0` | `saved games/game_state.c` |
| `game_state_saved_game_read_header_async` | `0x00124360` | `saved games/game_state.c` |
| `game_state_load_core` | `0x00124380` | `saved games/game_state.c` |
| `game_state_core_file_reference` | `0x00124490` | `saved games/game_state.c` |
| `game_state_core_read` | `0x00124640` | `saved games/game_state.c` |
| `game_state_saved_game_create` | `0x00124770` | `saved games/game_state.c` |
| `game_state_saved_game_read_header` | `0x00124790` | `saved games/game_state.c` |
| `game_state_saved_game_read_data` | `0x00124800` | `saved games/game_state.c` |
| `game_state_saved_game_write` | `0x00124840` | `saved games/game_state.c` |
| `csprintf` | `0x000b66c0` | `cseries/cseries.c` |

- Archivo nuevo: `tests/test_game_state_files.py`.
- Funciones que se suplen en las pruebas: `saved_game_auxiliary_read`
  (`0x002174b0`) y `saved_game_auxiliary_write` (`0x00217520`).

### Lo que se sabe

- Un «core» es un volcado del estado del juego en un archivo del disco:
  `d:\core\<nombre>.bin`, o la ruta entera si el nombre empieza por una
  unidad. Es una herramienta de los desarrolladores que sigue en el juego.
- Cargar un estado tiene siempre los mismos pasos: leer la cabecera, ver
  que es válida, comprobar que es de la partida en curso (mismas opciones,
  mismo mapa), poner el bsp que dice, y solo entonces leer el estado
  entero. Con un «core», si la partida no coincide se empieza antes la suya;
  con una partida guardada, no se hace nada.
- Un estado cargado de un archivo pasa a ser de una partida local, fuera lo
  que fuera al guardarlo.
- La partida guardada tiene su estado en un archivo aparte, `auxilary.bin`
  (así escrito), que se lee y se escribe en otro hilo: el juego espera
  dando vueltas al bucle de carga.
- El estado ocupa 0x3FE000 bytes (unos 4 MB) y su cabecera 0x1288.

### Sin comprobar

- `csprintf` escribe como mucho 1.023 caracteres y pone el cero en el
  último sitio: si el tope fuera 1.024 daría lo mismo.
- `0x001241b0`, que guarda el estado en la partida guardada, sigue sin
  descompilar.

### Mutaciones

46 líneas rotas a propósito: 44 detectadas a la primera, una con un caso
nuevo y otra que es la de «Sin comprobar».

## 2026-10-02 — Los archivos

`files.c` y su parte de la consola, que en Halo 1 es `files_windows.c`:
abrir, leer, escribir y borrar archivos, y componer rutas. De 365 funciones
se pasa a 382, el 3,39 % de las del juego (el 1,633 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_delete` | `0x00136860` | `tag_files/files_xbox.c` |
| `file_exists` | `0x001368f0` | `tag_files/files_xbox.c` |
| `file_open` | `0x00136970` | `tag_files/files_xbox.c` |
| `file_close` | `0x00136bb0` | `tag_files/files_xbox.c` |
| `file_set_position` | `0x00136bf0` | `tag_files/files_xbox.c` |
| `file_set_eof` | `0x00136c40` | `tag_files/files_xbox.c` |
| `file_read` | `0x00136ca0` | `tag_files/files_xbox.c` |
| `file_write` | `0x00136d00` | `tag_files/files_xbox.c` |
| `file_read_from_position` | `0x00136d50` | `tag_files/files_xbox.c` |
| `file_write_to_position` | `0x00136d90` | `tag_files/files_xbox.c` |
| `file_get_last_modification_date` | `0x00136df0` | `tag_files/files_xbox.c` |
| `file_get_size` | `0x00136e90` | `tag_files/files_xbox.c` |
| `find_files_end` | `0x00136fc0` | `tag_files/files_xbox.c` |
| `file_location_get_full_path` | `0x001374c0` | `tag_files/files_xbox.c` |
| `find_files_start` | `0x00136f00` | `tag_files/files.c` |
| `file_path_add_name` | `0x00137320` | `tag_files/files.c` |
| `file_path_add_extension` | `0x00137370` | `tag_files/files.c` |

- Archivos nuevos: `tag_files/files.h`, `files.c`, `files_xbox.c`,
  `tests/test_files.py`.
- Funciones de la consola que se suplen en las pruebas: `GetLastError`,
  `RemoveDirectoryA`, `SetFileAttributesA`, `GetFileAttributesA`,
  `GetFileAttributesExA`.

### Lo que se sabe

- La referencia a un archivo es como la de Halo 1 (la firma `filo`, unos
  indicadores, la ruta de 256 caracteres) con dos campos más: el
  identificador del archivo abierto y la posición. Mide 272 bytes.
- Una ruta que no empieza por una unidad (`x:\`) es del disco del juego:
  se le pone `d:\` delante.
- El juego lleva la cuenta de la posición por su lado, y no le pide a la
  consola que se mueva si ya está donde quiere.
- Solo se abren archivos que existen (crear es cosa de otras funciones). Al
  abrir se dice por qué no se pudo: no existe, no hay permiso, no existe la
  ruta, la unidad no vale, está en uso, u otra cosa.
- Tras cada fallo el juego lee el error de la consola y lo borra, sin hacer
  nada con él: es lo que queda de los avisos de la versión de desarrollo.
  En `file_set_eof` lo hace dos veces.
- Una lectura que trae menos de lo pedido deja el error 0x26 (fin de
  archivo).

### Mutaciones

73 líneas rotas a propósito: 70 detectadas a la primera, y las otras 3 con
casos nuevos (rutas con minúsculas en los extremos del abecedario, una letra
sin dos puntos, una ruta que llena la referencia).

## 2026-10-02 — El cambio de partida

El resto del corazón de `main_game.c`: cómo se pide otra partida, cuándo se
hace el cambio y qué pasos tiene. Con esto está entero el camino que va de
«quiero jugar este mapa» a «la partida ha empezado», aunque muchas de las
piezas que llama sigan sin descompilar. De 359 funciones se pasa a 365, el
3,24 % de las del juego (el 1,568 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_game_restart` | `0x0018e700` | `main/main_game.c` |
| `main_game_change` | `0x0018e790` | `main/main_game.c` |
| `main_game_change_update` | `0x0018e810` | `main/main_game.c` |
| `main_game_change_immediate` | `0x0018e8b0` | `main/main_game.c` |
| `main_game_load_map` | `0x0018eaa0` | `main/main_game.c` |
| `main_game_load_failed` | `0x0018efa0` | `main/main_game.c` |

### Cambiado

- `main_game_load_globals` es más grande de lo que se sabía (0x1230 bytes):
  lleva también el cambio pedido (si hay uno, si es «a nada», cuándo se
  pidió y las opciones de la partida nueva).
- `unknown10C` y `unknown10D` ya tienen nombre: `restarting` y
  `change_pending`.

### Lo que se sabe

- Una partida nueva no empieza cuando se pide: `main_game_change` guarda las
  opciones y `main_game_change_update` la empieza al principio de un
  fotograma, si nada dice que espere.
- El cambio tiene estos pasos: soltar la partida y el mapa que haya, borrar
  lo que el bucle principal tuviera pendiente (guardar, revertir, cambiar de
  bsp), repartir la memoria, cargar el mapa y empezar la partida. Si se pide
  «nada», se suelta todo y queda abierta solo la caché de texturas.
- Cargar el mapa depende de si está en el disco duro: si está, se cargan sus
  tags y se abren las cachés; si no, se espera a que se copie (con la caché
  de texturas abierta, para enseñar algo mientras); si no se puede copiar,
  se pide soltarlo todo.
- Si la partida no puede empezar, el juego va al menú principal. Si lo que
  no puede empezar es el menú principal, se queda con la caché de texturas
  abierta y nada más.
- Reempezar la partida en el mismo mapa no vuelve a cargarlo: solo se
  desmontan y se montan otra vez los sistemas del juego.

### Sin comprobar

- Qué son `0x004e61dc` (0x120 bytes que se ponen a cero al empezar una
  partida), `0x0051ea10` (mientras es mayor que 0, el cambio espera) y los
  avisos a la partida en red (`0x00527330`, `0x000593e0`).

### Mutaciones

61 líneas rotas a propósito: 60 detectadas a la primera, y la otra con un
caso nuevo.

## 2026-10-02 — La memoria de la interfaz

El primer usuario de `memory_pool.c`: el fondo del que sale la memoria de
los menús («ui memory pool»), que es uno de los ocho que toman memoria al
cargar un mapa. De 355 funciones se pasa a 359, el 3,19 % de las del juego
(el 1,545 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `user_interface_memory_pool_new` | `0x001a474c` | `interface/user_interface_memory.c` |
| `user_interface_memory_pool_delete` | `0x001a479a` | `interface/user_interface_memory.c` |
| `user_interface_memory_allocate` | `0x001a47b1` | `interface/user_interface_memory.c` |
| `user_interface_memory_free` | `0x001a4826` | `interface/user_interface_memory.c` |

- Archivos nuevos: `interface/user_interface_memory.{c,h}`,
  `tests/test_user_interface_memory.py`.

### Cambiado

- `main_game_state_has_map` (`0x0018eea0`) se llamaba mal: lo que recibe no
  es la etapa de la carga sino el reparto de memoria, y dice si es uno de
  los dos de campaña en sesión de red. Ahora es
  `main_game_memory_configuration_is_session`.
- La tabla de los que toman memoria ya tiene tres de sus ocho pares con
  nombre: interfaz, animaciones y geometría.

### Lo que se sabe

- El fondo de la interfaz tiene 1 MB fuera de la campaña, 96 KB en la
  campaña y 384 KB en una campaña en sesión de red.
- Sus bloques no se mueven (no guarda los punteros de los dueños) y los
  nuevos rellenan huecos.
- Si no hay sitio para un bloque, la interfaz suelta lo que puede
  (`0x00147ebe`) y lo intenta una vez más.
- El otro fondo que se crea al cargar un mapa es el de la voz («voice
  pool»).

### Mutaciones

15 líneas rotas a propósito, las 15 detectadas.

## 2026-10-02 — El fondo de memoria

`memory_pool.c`, otro módulo que ya existía en Halo 1: un trozo de memoria
del que se van tomando bloques de cualquier tamaño, y que se puede volver a
compactar porque cada bloque sabe qué puntero de su dueño hay que cambiar
cuando se mueve. De 344 funciones se pasa a 355, el 3,15 % de las del juego
(el 1,538 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `memory_pool_new` | `0x0018e1f0` | `memory/memory_pool.c` |
| `memory_pool_delete` | `0x0018e230` | `memory/memory_pool.c` |
| `memory_pool_initialize` | `0x0018e250` | `memory/memory_pool.c` |
| `memory_pool_block_allocate` | `0x0018e2b0` | `memory/memory_pool.c` |
| `memory_pool_block_reallocate` | `0x0018e340` | `memory/memory_pool.c` |
| `memory_pool_block_free` | `0x0018e430` | `memory/memory_pool.c` |
| `memory_pool_compact` | `0x0018e470` | `memory/memory_pool.c` |
| `memory_pool_block_link` | `0x0018e4f0` | `memory/memory_pool.c` |
| `memory_pool_block_new_at_end` | `0x0018e560` | `memory/memory_pool.c` |
| `memory_pool_block_new_at_start` | `0x0018e600` | `memory/memory_pool.c` |
| `memory_pool_block_new_in_gap` | `0x0018e690` | `memory/memory_pool.c` |

- Archivos nuevos: `memory/memory_pool.{c,h}`, `tests/test_memory_pool.py`.

### Lo que se sabe

- Respecto a Halo 1, los bloques pierden las firmas de principio y fin
  (16 bytes en vez de 24) y el fondo gana tres opciones: no guardar los
  punteros de los dueños (y entonces no se puede compactar), rellenar los
  huecos (un bloque nuevo va primero entre dos bloques, luego antes del
  primero y solo al final después del último) y llevar la cuenta de quién
  pide cada bloque (16 bytes más por bloque: la firma `head`, dos datos de
  quien lo pide y la hora).
- Todo va a múltiplos de 16 bytes.
- A diferencia de Halo 1, pedir un bloque no compacta el fondo si no cabe:
  devuelve que no, y quien lo pide decide.
- Cambiar el tamaño de un bloque lo hace en el sitio si hay hueco detrás;
  si no, toma otro bloque, copia y suelta el viejo.
- Lo usan la interfaz («ui memory pool»), los objetos y algo más que toma
  memoria al cargar un mapa.

### Sin comprobar

- En `memory_pool_compact`, si un bloque que ya está en su sitio se mueve
  sobre sí mismo o no: queda igual.
- Qué son los dos datos que se apuntan de quien pide un bloque.

### Mutaciones

68 líneas rotas a propósito: 66 detectadas a la primera. De las otras 2,
una dio un caso nuevo (un fondo que no empieza en un múltiplo de 16) y la
otra es la de «Sin comprobar».

## 2026-10-02 — Cargar y descargar un mapa

`main_game.c`: el módulo que une las tres cachés con el resto del juego.
Lleva la cuenta de por dónde va la carga de un mapa, reparte la memoria
entre los que la usan y empieza la partida. De 332 funciones se pasa a 344,
el 3,05 % de las del juego (el 1,497 % de sus bytes). **Se pasa del 3 %.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_game_unload` | `0x0018eb80` | `main/main_game.c` |
| `main_game_load_begin` | `0x0018ec20` | `main/main_game.c` |
| `main_game_load_end` | `0x0018ec60` | `main/main_game.c` |
| `main_game_load_fail` | `0x0018ed00` | `main/main_game.c` |
| `main_game_close_caches` | `0x0018edb0` | `main/main_game.c` |
| `main_game_open_texture_cache` | `0x0018ee60` | `main/main_game.c` |
| `main_game_state_has_map` | `0x0018eea0` | `main/main_game.c` |
| `main_game_memory_configuration` | `0x0018eeb0` | `main/main_game.c` |
| `main_game_configure_memory` | `0x0018ef00` | `main/main_game.c` |
| `main_game_start` | `0x0018f000` | `main/main_game.c` |
| `main_game_unknown_0018f100` | `0x0018f100` | `main/main_game.c` |
| `main_game_launch_main_menu` | `0x0018f170` | `main/main_game.c` |

- Archivos nuevos: `main/main_game.{c,h}`, `tests/test_main_game.py`.
- La tabla de `0x00453c00` (`main_game_memory_users`): los ocho que toman
  memoria al cargar un mapa. Seis siguen sin descompilar.
- Arnés: una función sin descompilar a la que el código llama por un
  puntero «stdcall» se marca así en `symbols/functions.csv` y limpia la pila.

### Cambiado

- Tres globales de `main.c` que estaban sin identificar son campos de otras
  cosas, y ahora se leen de ellas: `0x004ed294` es la etapa de la carga
  (`main_game_load_globals.state`), `0x004e6470` y `0x004e64a0` son los
  contadores de «leerlo todo de una vez» de las cachés de texturas y de
  geometría, y `0x004ed39d` va con el estado de la carga.

### Lo que se sabe

- La carga de un mapa tiene etapas: nada, empezada, mapa cargado (con las
  cachés abiertas), cachés cerradas, y «solo la caché de texturas» para lo
  que se enseña cuando no hay mapa.
- El reparto de memoria tiene cinco maneras: 1 para todo lo que no es
  campaña, 2 y 3 para una campaña en una sesión de red (solo y con un
  segundo jugador), 4 y 5 para una campaña normal. Si la partida que empieza
  pide otro reparto que la anterior, los ocho de la tabla devuelven su
  memoria y la vuelven a tomar.
- Al terminar de cargar un mapa se abren, por este orden: la caché de
  animaciones, los textos del idioma de la consola, la de geometría y la de
  texturas (que antes lee los píxeles que vienen con el mapa).
- Los textos de cada idioma están en el tag de globales, 28 bytes por
  idioma a partir de `+0x188`.
- El menú principal es un mapa más: `scenarios\ui\mainmenu\mainmenu`, con
  el modo de juego 3.
- `main_game_unload` no tiene nada previsto para las etapas 1 y 3: el juego
  salta a la dirección 0. No se la llama entonces.

### Sin comprobar

- En `main_game_launch_main_menu`, el cero al final del nombre del mapa:
  `strncpy` ya rellena con ceros.
- Qué son `0x004cf770` (algo de la partida en red) y la tabla de datos de
  `0x00502104`.

### Mutaciones

79 líneas rotas a propósito: 76 detectadas a la primera. De las otras 3, dos
dieron casos nuevos y una es la de «Sin comprobar».

## 2026-10-02 — La caché de geometría

La tercera caché que usa `lruv_cache.c`: la que trae del disco los vértices
y los triángulos de los modelos y de los niveles. Halo 1 no la tiene; los
nombres siguen los de su caché de texturas. De 315 funciones se pasa a 332,
el 2,95 % de las del juego (el 1,454 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `geometry_cache_new` | `0x0012d8b0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_delete` | `0x0012d970` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_allocate_memory` | `0x0012d9f0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_free_memory` | `0x0012dad0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_open` | `0x0012daf0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_close` | `0x0012db40` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_reading_count` | `0x0012dba0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_update_predicted` | `0x0012dc10` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_predict` | `0x0012dcb0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_idle` | `0x0012dd80` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_delete` | `0x0012ddd0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_load` | `0x0012de70` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_flush` | `0x0012e0c0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_delete_tag_blocks` | `0x0012e150` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_locked` | `0x0012e210` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_deleted` | `0x0012e290` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_start_loading` | `0x0012e3a0` | `cache/xbox_geometry_cache.c` |

- Archivos nuevos: `cache/geometry_cache.h`, `cache/xbox_geometry_cache.c`,
  `tests/test_geometry_cache.py`.
- Funciones que se suplen en las pruebas: `geometry_block_attach_data`
  (`0x00223c70`), `geometry_block_busy` (`0x00223d80`),
  `geometry_block_wait` (`0x00223e00`), `unknown_0012e480`.

### Cambiado

- `struct geometry_block_info`: los seis bytes del final son del juego:
  `loaded`, `flags` y `cache_block_index`.
- En la caché de animaciones, el argumento de
  `animation_cache_allocate_memory` no es el número de jugadores sino el
  reparto de memoria (`configuration`), el mismo que recibe la de geometría.

### Lo que se sabe

- La caché tiene 7 MB, o 6,5 MB en dos de los seis repartos de memoria (el 2
  y el 4), en páginas de 4 KB, con hasta 512 bloques y 350 avisos. El juego
  no comprueba que el reparto sea uno de los seis.
- Cada bloque de geometría de un tag lleva al final si está leído, unos
  indicadores y su bloque en la caché. Al leerse, una función apunta en el
  tag dónde quedó la geometría; al echarlo, esos dos campos del tag se
  ponen a cero.
- Un bloque no se puede echar si se usó en este tick, si se está leyendo, o
  si se usó hace menos de 5 ticks y la consola aún dibuja con él.
- En esta versión el juego nunca espera por la geometría: hay un byte
  (`0x00468c4c`) que lo impide, y está puesto. Lo que no está se pide y se
  dibuja cuando llegue. La excepción es cuando el juego está leyendo todo
  de una vez (al cargar un nivel): entonces sí espera, a toda la geometría o
  solo a la de los niveles (los tags `sbsp` y `ltmp`).
- Al subir la prioridad de una lectura ya empezada, la tarea pasa siempre a
  prioridad 7, sea cual sea la nueva.
- Cada fotograma se empiezan lecturas de la geometría avisada hasta tener 5
  en marcha, si no hay más de 25 tareas pendientes.
- `geometry_cache_delete_tag_blocks` saca de la caché toda la geometría de
  un tag (lo usa quien descarga un tag).

### Sin comprobar

- Dos líneas que no cambian nada: poner `loaded` a falso tras echar el
  bloque (ya lo hace la función que lo echa) y la comprobación de que la
  geometría no estaba ya apuntada (no puede estarlo en ese punto).
- `0x0012e480`, a la que se llama cada fotograma tras el trabajo de la
  caché: se suple en las pruebas.

### Mutaciones

104 líneas rotas a propósito: 101 detectadas a la primera. De las otras 3,
una dio un caso nuevo (un bloque al que aún no se le ha dado su geometría)
y dos son las de «Sin comprobar».

## 2026-10-02 — La caché de texturas, segunda parte

Lo que faltaba de `xbox_texture_cache.c`: cómo se piden y se leen los
píxeles de un bitmap, y la espera de la memoria prestada. Con esto el módulo
está completo. De 301 funciones se pasa a 315, el 2,80 % de las del juego
(el 1,374 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `texture_cache_update_predicted` | `0x0012c450` | `cache/xbox_texture_cache.c` |
| `texture_cache_load_preloaded_pixels` | `0x0012c640` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_initialize_preloaded_format` | `0x0012c820` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_preloaded_format` | `0x0012c960` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_level` | `0x0012c990` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_choose_level` | `0x0012cb10` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_load` | `0x0012cb80` | `cache/xbox_texture_cache.c` |
| `texture_cache_usage_is_preloaded` | `0x0012ccb0` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_loaded` | `0x0012ccf0` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_hardware_format` | `0x0012ce00` | `cache/xbox_texture_cache.c` |
| `texture_cache_steal_memory` | `0x0012d400` | `cache/xbox_texture_cache.c` |
| `texture_cache_start_loading_bitmap` | `0x0012d5a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_initialize_hardware_format` | `0x0012d6d0` | `cache/xbox_texture_cache.c` |
| `system_ticks_to_seconds` | `0x00223bb0` | `cseries/cseries.c` |

- Las tablas de formatos de Direct3D por formato de bitmap (`0x00450768` y
  `0x004507c8`), reproducidas en el código.
- Funciones que se suplen en las pruebas: `bitmap_mipmap_count`
  (`0x000359e0`) y `bitmap_row_size` (`0x001365a0`).
- El apoyo de las pruebas (`tools/difftest_libc`) sabe dividir números de
  64 bits.

### Cambiado

- `struct bitmap_data`: nombres para `mipmap_count`, `mip_bias`, `usage`,
  `maximum_mip_bias` y la textura que el bitmap lleva dentro.
- En los globales de la caché: `has_preloaded_pixels`, `preloaded_pixels`,
  `blocking_count`, `blocking_usage`, `use_preloaded_pixels`.

### Lo que se sabe

- Al pedir la textura de un bitmap se dan cinco indicadores: esperar a los
  píxeles, leerlos si no están, «exacto» (el primer nivel y nada más), y dos
  de prisa. La prioridad de la lectura es 7 si el juego espera, 5 o 1 según
  la prisa, y 2 si no se dice nada.
- El nivel de detalle se elige con el sesgo: lo que pide quien dibuja, más
  lo que el bitmap cede de suyo (en centésimas de nivel), más el sesgo de la
  caché. Con sesgo 1 o más vale el segundo nivel; con 2 o más, el tercero.
  Un nivel que no sea el primero solo cuenta si ocupa más de 1 KB.
- Si el nivel elegido no está, vale cualquiera que esté. Si el juego tenía
  que esperar y no hay ninguno, se dibuja con una de tres texturas de
  reserva, según el tipo de bitmap, y se apunta el fallo para subir el sesgo.
- No se empiezan lecturas de poca prioridad si hay más de 25 tareas
  pendientes o ya van 10 lecturas en el fotograma. De los bitmaps previstos
  se atienden 5 por fotograma.
- Para los píxeles de un bitmap no se echan bloques usados en los últimos 5
  ticks; si el juego espera, en los últimos 2.
- El mapa puede traer un bloque de píxeles aparte (cabecera `+0x158` y
  `+0x15C`) que se lee entero al cargar; los bitmaps de ciertos usos (1 a
  11, 17 y 18) se dibujan con esos píxeles. Su textura se monta la primera
  vez que se pide, a partir de lo que el mapa dice de ella.
- La textura de Direct3D se monta a mano: formato, dimensiones como
  potencias de dos, número de mipmaps y dirección de los píxeles en 28 bits.
  Los bitmaps de tamaño que no es potencia de dos llevan el tamaño aparte.
- `texture_cache_steal_memory` deja pasar hasta 30 o 90 fotogramas de la
  caché para que los bloques envejezcan, y luego espera a la consola hasta
  0,1 o 1 segundo, midiendo con `rdtsc`. El procesador de la consola da
  733.333.333 ticks por segundo.
- El juego recorre los bitmaps de todos los grupos al cargar los píxeles
  del mapa y no hace nada con ellos: lo que hacía no está en esta versión.

### Sin comprobar

- La espera con reloj de `texture_cache_steal_memory` cuando no hay sitio
  ni dejando pasar fotogramas: depende del reloj del procesador, que en el
  emulador no da lo mismo dos veces.
- El valor exacto de los ticks por segundo: un tick más o menos no cambia
  el resultado con la precisión de un real.
- Tres escrituras que dejan lo que ya había (el bloque recién creado ya
  está a cero) y dos condiciones que no pueden darse de otra manera (los
  niveles van de más a menos detalle; con textura no se dibuja la de
  reserva).
- En `texture_cache_load_preloaded_pixels`, el recorrido de los bitmaps: en
  las pruebas no hay ninguno.

### Mutaciones

111 líneas rotas a propósito: 101 detectadas a la primera. De las otras 10,
cinco dieron casos nuevos (un bitmap previsto que ya está en la caché, la
profundidad con signo, filas de un tamaño múltiplo de 64) y cinco son las de
«Sin comprobar».

## 2026-10-02 — La caché de texturas, primera parte

`xbox_texture_cache.c`, que ya existía en Halo 1: la caché que guarda en
memoria los píxeles de los bitmaps. En este hito está todo menos la carga de
un bitmap: crearla, darle memoria, vaciarla, el trabajo de cada fotograma,
lo que le pregunta `lruv_cache` y la memoria que presta a otros. De 284
funciones se pasa a 301, el 2,67 % de las del juego (el 1,254 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `texture_cache_new` | `0x0012c0d0` | `cache/xbox_texture_cache.c` |
| `texture_cache_delete` | `0x0012c190` | `cache/xbox_texture_cache.c` |
| `texture_cache_open` | `0x0012c1e0` | `cache/xbox_texture_cache.c` |
| `texture_cache_close` | `0x0012c290` | `cache/xbox_texture_cache.c` |
| `texture_cache_update_mip_bias` | `0x0012c2e0` | `cache/xbox_texture_cache.c` |
| `texture_cache_stolen_memory_idle` | `0x0012c400` | `cache/xbox_texture_cache.c` |
| `texture_cache_reset_frame_index` | `0x0012c530` | `cache/xbox_texture_cache.c` |
| `texture_cache_tick` | `0x0012c5b0` | `cache/xbox_texture_cache.c` |
| `texture_cache_idle` | `0x0012c600` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_delete` | `0x0012c770` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_predict` | `0x0012cc10` | `cache/xbox_texture_cache.c` |
| `texture_cache_flush` | `0x0012d0a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_type` | `0x0012d160` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_locked` | `0x0012d1a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_deleted` | `0x0012d200` | `cache/xbox_texture_cache.c` |
| `texture_cache_steal_memory_now` | `0x0012d2f0` | `cache/xbox_texture_cache.c` |
| `texture_cache_return_memory` | `0x0012d520` | `cache/xbox_texture_cache.c` |

- Archivos nuevos: `cache/texture_cache.h`, `cache/xbox_texture_cache.c`,
  `tests/test_texture_cache.py`.
- Funciones de Direct3D que se suplen en las pruebas: `D3DDevice_KickOff`
  (`0x003fac30`), `D3DResource_IsBusy` (`0x003fb710`),
  `D3DResource_BlockUntilNotBusy` (`0x003fad50`).

### Cambiado

- `physical_memory_buffer_allocate` y `physical_memory_buffer_free` eran de
  la caché de texturas: ahora son `texture_cache_steal_memory`
  (`0x0012d400`, aún sin descompilar) y `texture_cache_return_memory`, como
  en Halo 1. La copia de mapas y la partida guardada las siguen usando igual.
- `struct bitmap_data`: tienen nombre los campos que la caché usa del final
  del bitmap (`hardware_format`, `base_address`, `frame_index`) y los que
  ocupan el sitio del cuarto al sexto nivel de detalle (`cache_block_indices`
  y `cache_addresses`).
- `struct d3d_device`: el puntero a los registros de la consola (`+0x934`).

### Lo que se sabe

- La caché se queda con toda la memoria física que sobra al cargar un mapa,
  en páginas de 4 KB, y tiene hasta 512 bloques. Un bitmap ocupa hasta tres:
  uno por nivel de detalle.
- El juego quita detalle a los bitmaps según lo llena que esté la caché. Cada 200 ms
  mira cuántas páginas se usaron en los últimos 5 ticks: con el 95 % o más
  (o si se dibujó un bitmap sin sus píxeles) sube el sesgo 0,05; con el 85 %,
  0,02; con el 70 %, 60 % y 50 % o menos lo baja 0,01, 0,02 y 0,05. El sesgo
  va de 0 a 2 niveles.
- Un bloque no se puede echar si es memoria prestada, si está marcado como
  necesario (salvo al vaciar la caché), si se está leyendo o si la consola
  aún dibuja con él. Al echar uno de un bitmap, el juego espera a la lectura
  y a la consola.
- La memoria que la caché presta (a la copia de mapas y a la partida
  guardada) empieza en una página, y justo antes lleva 36 bytes: el bloque,
  una firma (la dirección con `0x02281972`), el tamaño, un dato de quien la
  pide y dos funciones suyas, una para cada fotograma y otra para cuando la
  caché quiere la memoria de vuelta. Al vaciar la caché se les pide a todos.
- Tras mandar trabajo a la consola (`D3DDevice_KickOff`) el juego lee tres
  registros de la tarjeta gráfica y no hace nada con ellos; está tal cual.
- Los bitmaps tienen sitio para seis niveles de detalle en el mapa, pero el
  juego de la consola solo lee tres y guarda cosas suyas en el sitio de los
  otros tres.
- `texture_cache_delete` no libera la lista de bitmaps previstos.

### Sin comprobar

- En lo que impide echar un bloque, la comprobación de que es memoria
  prestada: un bloque prestado tiene además el resto a cero, y con eso ya
  cuenta como «leyéndose».
- Al prestar memoria, el borrado del bloque: `datum_new_at_index` ya lo deja
  a cero.
- `texture_cache_steal_memory` (`0x0012d400`) y la función que empieza a
  leer los bitmaps previstos (`0x0012c450`) siguen sin descompilar; en las
  pruebas de `texture_cache_idle` la segunda se suple.

### Mutaciones

100 líneas rotas a propósito: 95 detectadas a la primera. De las otras 5,
tres dieron casos nuevos (un bitmap en la lista de previstos sin su marca,
uno que está dos veces, memoria prestada con basura) y dos son las de «Sin
comprobar».

## 2026-10-02 — La caché de animaciones

El primer módulo que usa `lruv_cache.c` entero: la caché que trae del disco
los datos de las animaciones cuando hacen falta. Halo 1 no la tiene (allí las
animaciones van dentro de los tags); los nombres siguen los de su caché de
sonidos. De 266 funciones se pasa a 284, el 2,52 % de las del juego (el
1,187 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `animation_cache_new` | `0x00123180` | `cache/xbox_animation_cache.c` |
| `animation_cache_allocate_memory` | `0x00123230` | `cache/xbox_animation_cache.c` |
| `animation_cache_free_memory` | `0x001232e0` | `cache/xbox_animation_cache.c` |
| `animation_cache_delete` | `0x00123310` | `cache/xbox_animation_cache.c` |
| `animation_cache_open` | `0x001233a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_close` | `0x001233f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_idle` | `0x00123430` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_new` | `0x001234a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_request` | `0x00123550` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_read` | `0x001235b0` | `cache/xbox_animation_cache.c` |
| `animation_cache_find_predicted` | `0x00123680` | `cache/xbox_animation_cache.c` |
| `animation_cache_predict` | `0x001236f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_wait` | `0x001237a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_get_data` | `0x001237e0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_locked` | `0x001238b0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_deleted` | `0x001238f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_reading_count` | `0x00123970` | `cache/xbox_animation_cache.c` |
| `animation_cache_update_predicted` | `0x001239d0` | `cache/xbox_animation_cache.c` |

- Archivos nuevos: `cache/animation_cache.h`, `cache/xbox_animation_cache.c`,
  `tests/test_animation_cache.py`.
- En `memory/lruv_cache.h`, como código en línea (en Halo 1 son funciones;
  aquí el juego las repite en cada sitio): `lruv_idle`, `lruv_block_get`,
  `lruv_block_get_address`, `lruv_block_touch`.
- Global `heap_allocation` (`0x00468758`): el asignador con el que se crean
  las cachés.
- Arnés: un `Buffer` puede llevar punteros a funciones (`functions`) y
  sitios donde la llamada deja punteros a funciones o a globales (`places`),
  que se comparan por aquello a lo que apuntan.

### Cambiado

- `cache_file_read_async` devuelve el número de la tarea de lectura (antes
  estaba como `void`): la caché lo guarda para poder esperar la lectura.
- `shell_initialize` y `shell_dispose` llaman a `animation_cache_new` y
  `animation_cache_delete` (antes `unknown_00123180` y `unknown_00123310`).
- `CACHE_FILE_SECTOR_SIZE` pasa a `cache_files.h`.

### Lo que se sabe

- La caché tiene 4 MB (3 MB con la pantalla partida), en páginas de 2 KB, y
  hasta 512 bloques. La memoria se toma al cargar un mapa y se suelta al
  salir: las dos funciones están en una tabla de ocho pares en `0x00453c00`,
  la de todos los que toman memoria entonces.
- En el tag, los datos de unas animaciones son 16 bytes: un campo sin
  identificar, el tamaño, el desplazamiento en el mapa y el bloque de la
  caché (o ninguno).
- Hay tres urgencias, en una tabla en `0x00468810` (prioridad de la tarea,
  edad con la que nace el bloque, antigüedad mínima de los bloques que se
  pueden echar): ahora mismo (8, 0, 0), pronto (5, 10, 30) y cuando se pueda
  (2, 20, 50).
- Quien va a usar una animación avisa antes (`animation_cache_predict`); la
  caché apunta hasta 128 avisos y cada fotograma empieza a leer algunos, sin
  pasar de 8 lecturas a la vez ni si hay más de 25 tareas pendientes. Primero
  van los avisos urgentes.
- Si se pide una animación que no está (`animation_cache_get_data`), el
  juego se queda esperando la lectura. Para hacerle sitio puede echar hasta
  los bloques que se están leyendo, esperándolos, y lo intenta hasta tres
  veces más, pasando un tick de la caché cada vez.
- `animation_cache_get_data` no mira si consiguió bloque: una animación que
  no quepa en la caché entera no se puede pedir.
- Un bloque que se está leyendo no se echa. Al echar uno, la animación
  vuelve a quedar sin bloque.

### Corregido durante el hito

- `animation_cache_open` vaciaba dos veces cada tabla: `data_make_valid` ya
  lo hace. Lo encontró la comprobación por mutación.

### Sin comprobar

- Tres escrituras que dejan el valor que ya había: los dos `read_done=
  FALSE` repetidos al pedir y al leer un bloque, y si el ajuste del tick en
  `animation_cache_predict` es con `>` o con `>=`.
- Para qué sirve el segundo argumento de `animation_cache_get_data`
  (`0x7000180` cuando lo llama la propia caché): la función que lo recibe no
  lo usa.

### Mutaciones

89 líneas rotas a propósito: 82 detectadas a la primera. De las otras 7,
dos eran el error de `animation_cache_open`, dos dieron casos nuevos (un
aviso de una animación que ya está en la caché, un bloque sin lectura
empezada) y tres son las de «Sin comprobar».

## 2026-10-01 — Reservar un bloque en la caché

La función que faltaba de `lruv_cache.c`, y la más grande del módulo (1.210
bytes): la que busca sitio para un bloque nuevo y decide a cuáles echar. De
265 funciones se pasa a 266, el 2,36 % de las del juego (el 1,123 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `lruv_block_new` | `0x0013d370` | `memory/lruv_cache.c` |

### Cambiado

- Campos de la caché que ya tienen nombre: `hole_algorithm` (antes
  `unknown2C`) y `failed_page_counts` (antes `unknown44`); y los del hueco,
  `previous_block_index` y `first_page_index`.
- `tests/test_lruv_cache.py`: el constructor de cachés acepta el tamaño de la
  tabla, las edades de los bloques, los huecos entre ellos y ticks de 32 bits.

### Lo que se sabe

- La función recorre las páginas de la primera a la última, bloque a bloque
  y hueco a hueco. En cada sitio donde empieza un bloque o un hueco puede
  empezar un tramo; cada tramo crece hasta tener las páginas pedidas y
  entonces se compara con el mejor hasta el momento. Un bloque que no se
  puede echar (lo dice la función del dueño, o se usó hace menos ticks de
  los que se piden) corta todos los tramos que pasan por él.
- Lleva hasta 255 tramos abiertos a la vez, en un anillo de 256 en la pila.
- Hay tres maneras de comparar dos tramos, según un campo de la caché: el
  más viejo (y a igualdad, el de menos páginas); el más viejo con margen
  (`lruv_hole_is_better`); y el de más edad por página.
- Si no hay sitio, se apunta el número de páginas pedido en una tabla por
  edad, y la próxima petición igual o mayor con esa edad falla sin buscar.
  La tabla se borra cuando la caché cambia o pasa un tick.
- Si la tabla de bloques está llena, además de los bloques del tramo se echa
  al bloque que lleva más tiempo sin usarse.
- La tabla de fallos tiene ocho entradas y la edad se limita a ocho, no a
  siete: con edad 8 o más el juego lee y escribe el campo siguiente de la
  caché, que es el puntero a los bloques. Se ha dejado igual.

### Sin comprobar

- Al enlazar el bloque nuevo detrás del último, si el anterior se toma de
  `last_block_index` o del bloque previo: en una lista bien formada son el
  mismo.

### Mutaciones

47 líneas rotas a propósito: 40 detectadas a la primera. Las otras 7 dieron
casos nuevos de prueba (ticks con el bit alto, más de 256 tramos abiertos,
bloques bloqueados con la tabla llena) y ahora se detectan todas salvo la de
«Sin comprobar».

## 2026-10-01 — La caché de bloques de páginas

`lruv_cache.c`, un módulo que ya existía en Halo 1: una caché que reparte
páginas de memoria en bloques y, cuando no cabe uno nuevo, echa a los que
llevan más tiempo sin usarse. El juego la usa para las animaciones y las
texturas. De 258 funciones se pasa a 265, el 2,35 % de las del juego (el
1,082 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `lruv_initialize` | `0x0013d170` | `memory/lruv_cache.c` |
| `lruv_wrap_ticks` | `0x0013d230` | `memory/lruv_cache.c` |
| `lruv_flush` | `0x0013d2b0` | `memory/lruv_cache.c` |
| `lruv_hole_is_better` | `0x0013d320` | `memory/lruv_cache.c` |
| `lruv_block_delete` | `0x0013d830` | `memory/lruv_cache.c` |
| `lruv_resize` | `0x0013d8b0` | `memory/lruv_cache.c` |
| `lruv_compute_pages_in_use` | `0x0013d950` | `memory/lruv_cache.c` |

- Archivos nuevos: `memory/lruv_cache.{c,h}`; `tests/test_lruv_cache.py`.

### Lo que se sabe

- La caché de Halo 2 tiene los campos de la de Halo 1 y varios más: una
  tercera función, un modo de elegir sitio, ocho contadores y un asignador.
  Los bloques pierden el campo de datos del usuario (24 bytes en vez de 28).
- El contador de ticks llega hasta 0x7fffffff; al llegar vuelve a 0x101 y
  los bloques conservan su orden solo hasta 256 ticks atrás.
- Al elegir entre dos huecos gana el que lleva más tiempo sin usarse; si
  la diferencia es de menos de 150 ticks (y nunca se cuentan más de 300),
  gana el de menos páginas.
- El módulo de `0x00123180` es la caché de animaciones («xbox animation
  cache»), y `0x0012c0d0` la de texturas.

### Sin comprobar

- En `lruv_wrap_ticks`, si un bloque de exactamente 256 ticks de antigüedad
  entra por una rama o por la otra: las dos le dan el mismo valor.

### Pendiente

- `lruv_block_new` (`0x0013d370`), la función que reserva un bloque: es
  grande y tiene tres maneras de puntuar los huecos.

## 2026-10-01 — Las preferencias guardadas

El archivo `z:\preferences.dat`, donde el juego recuerda cosas de una vez
para otra: abrirlo, comprobar que lo firmó esta consola, escribirlo cuando
algo cambia. Es el primero de los sistemas que prepara el arranque que se
identifica. De 244 funciones se pasa a 258, el 2,29 % de las del juego (el
1,052 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `global_preferences_initialize` | `0x00121350` | `main/global_preferences.c` |
| `global_preferences_reset` | `0x00121220` | `main/global_preferences.c` |
| `global_preferences_validate` | `0x00121280` | `main/global_preferences.c` |
| `global_preferences_flush` | `0x00121120` | `main/global_preferences.c` |
| `global_preferences_flush_blocking` | `0x00120d80` | `main/global_preferences.c` |
| `global_preferences_set_name` | `0x00120df0` | `main/global_preferences.c` |
| `global_preferences_set_other_name` | `0x00120e40` | `main/global_preferences.c` |
| `global_preferences_set_unknown15C` | `0x00121040` | `main/global_preferences.c` |
| `global_preferences_set_unknown160` | `0x00121060` | `main/global_preferences.c` |
| `global_preferences_get_unknown160` | `0x001210a0` | `main/global_preferences.c` |
| `global_preferences_set_unknown164` | `0x001210c0` | `main/global_preferences.c` |
| `global_preferences_get_unknown164` | `0x00121100` | `main/global_preferences.c` |
| `xbox_signature_end` | `0x00213380` | `main/global_preferences.c` |
| `async_flush_file_blocking` | `0x001a0af0` | `cseries/async.c` |

- Archivos nuevos: `main/global_preferences.{c,h}`;
  `tests/test_global_preferences.py`.

### Cambiado

- Las tres variables sueltas que usaban los archivos de caché (el idioma y
  el build de las copias de mapas, y la marca de «algo cambió») son campos
  de `global_preferences_globals` (`0x00510818`).
- `shell_initialize` y el fotograma llaman ya a estas funciones por su
  nombre.
- La biblioteca C de las pruebas gana `wcsncpy` con caracteres de 16 bits.

### Lo que se sabe

- El archivo son 0x1fc bytes: una firma de 20 bytes hecha por la consola y
  0x1e8 de datos. Si la firma no coincide, la versión no es la 8 o no se
  lee entero, se descarta y se empieza con preferencias vacías.
- Guarda, entre otras cosas, cinco nombres de 31 caracteres, el idioma y el
  build con los que se hicieron las copias de los mapas (por eso al cambiar
  de idioma se vuelven a copiar), y varios ajustes de 0 a 3.
- Se escribe en segundo plano, y solo si lo que hay difiere de lo último
  escrito.
- Un descuido del juego, reproducido tal cual: al validar unas
  preferencias recién leídas, el final de cuatro de los nombres se pone en
  las preferencias que el juego ya tenía, no en las leídas.

### Sin comprobar

- En `global_preferences_initialize`, marcar la tarea como terminada
  después de validar: la lectura ya la había marcado.
- Qué son los ajustes sin nombre (`unknown15C`, `unknown160`...).

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
