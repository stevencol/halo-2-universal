# Registro de cambios

Los cambios más recientes van arriba.

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
