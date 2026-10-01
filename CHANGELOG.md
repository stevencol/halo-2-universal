# Registro de cambios

Los cambios más recientes van arriba.

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
