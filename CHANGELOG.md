# Registro de cambios

Los cambios más recientes van arriba.

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
