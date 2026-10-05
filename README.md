# halo-2-universal

Descompilación en curso de **Halo 2 para Xbox** (build `02.09.27.09809`,
`halo2ship.exe`, SHA-256 del `default.xbe`
`92746c937cd3d41e247cef7c9aea7d0dd1f3d09066b8f445c7665efaa0c6d17c`).

El objetivo a largo plazo es un port nativo del juego, como el que
[halo-ce-universal](https://github.com/cybersecurity/halo-ce-universal) hizo
con Halo: Combat Evolved.

**Estado: muy temprano.** El juego no arranca ni está cerca de hacerlo. Hoy
el proyecto abre los mapas del juego, lista su contenido y exporta los
niveles y los modelos con su textura principal. Del código del ejecutable
hay descompiladas 910 funciones de unas 11.265 (el 8,08 %), todas
comprobadas contra el original. Consulta
[ESTADO.md](ESTADO.md) para la lista de lo hecho y lo que falta, y
[CHANGELOG.md](CHANGELOG.md) para el detalle de cada paso.

## En qué se basa

Este proyecto sigue el método y la organización de
[cybersecurity/halo-ce-universal](https://github.com/cybersecurity/halo-ce-universal),
el port de la descompilación de Halo: Combat Evolved a Linux, Windows y
Android. Ese proyecto parte a su vez de
[bnunu/halo-1](https://github.com/bnunu/halo-1), que es un fork de
[punpckhdq/halo](https://github.com/punpckhdq/halo).

De halo-ce-universal se toma:

- La estructura de carpetas (`source/` por sistemas del motor, `tools/`).
- El estilo del código C.
- El código de Halo 1 como referencia: Halo 2 hereda parte de ese motor, y
  comparar funciones ayuda a identificarlas.
- A futuro, su capa de plataforma (`port/`), que sustituye a la consola Xbox
  (Direct3D 8, sonido, entrada, red). Halo 2 corre en la misma consola.

Este repositorio **no** es un fork de halo-ce-universal ni contiene su
código. Todo el código de aquí se escribe a partir del ejecutable de Halo 2.

## Datos del juego

El repositorio no incluye el juego ni nada sacado de él. Necesitas tu propia
copia de Halo 2 para Xbox, con `default.xbe` y la carpeta `maps/`.

- Copia `default.xbe` a `disc/`.
- Los mapas se pueden leer desde donde estén; no hace falta copiarlos.

`disc/` y `ghidra/` están en `.gitignore` y no se suben nunca.

## Herramientas

| Herramienta | Uso |
| --- | --- |
| [Ghidra](https://github.com/NationalSecurityAgency/ghidra) 12.0.3 con [ghidra-xbe](https://github.com/XboxDev/ghidra-xbe) | Análisis del ejecutable |
| JDK 21 | Lo necesita Ghidra |
| clang y ld.lld ([llvm-mingw](https://github.com/mstorsjo/llvm-mingw), o el componente Clang de Visual Studio Build Tools, firmado por Microsoft, si Smart App Control bloquea el otro) y [ninja](https://ninja-build.org/) | Compilar |
| Python 3 | Scripts de `tools/` |
| [Unicorn](https://www.unicorn-engine.org/) y [Capstone](https://www.capstone-engine.org/) (`pip install unicorn capstone`) | Emulador de procesador para probar las funciones, y desensamblador |

## Compilar y probar

```
ninja
python tools/test_functions.py
python tools/test_maps.py "<carpeta maps del juego>"
```

`ninja` comprueba los tamaños de las estructuras de `source/`. No genera
ningún programa: todas las herramientas son scripts de Python.

`test_functions.py` comprueba el código descompilado: ejecuta en un emulador
cada función de `source/` y la función del ejecutable de la que salió, con
los casos de `tests/`, y compara los resultados. Necesita `disc/default.xbe`.
Al final dice qué parte del código del juego está descompilada.

`test_maps.py` comprueba las herramientas de datos: abre todos los mapas,
comprueba que cada tag está donde el cargador lo espera y exporta la
geometría de cada nivel a `build/obj/`.

### Descompilar una función

1. Elegirla con `python tools/function_index.py leaves` (las más llamadas) y
   leerla con `function_index.py show <dirección>` (pseudocódigo) y
   `python tools/disassemble.py <dirección>` (instrucciones). El pseudocódigo
   de todas está en `ghidra/decompiled/all/`.
2. Buscar su equivalente de Halo 1 en `source/` de halo-ce-universal, para
   el nombre y como referencia de lo que hace.
3. Escribirla en `source/`, en el archivo del sistema al que pertenece, con
   su dirección en un comentario.
4. Añadirla a `symbols/functions.csv`, con el lugar de cada argumento en la
   función original (un registro o `stack`).
5. Si usa una variable global, definirla en `source/` y añadirla a
   `symbols/globals.csv` con su dirección.
6. Si llama a una función del juego que aún no está descompilada, añadir
   esa función a `symbols/functions.csv` sin archivo de origen. La prueba
   da entonces la misma función simulada a los dos lados
   (`externals={"nombre": función}`) y compara también las llamadas que
   recibe. Así se puede sustituir también una función ya descompilada,
   para probar solo la que la llama. Para un puntero a función dentro de
   una estructura se usa `Mock`. Si una variable global tiene punteros a
   sí misma (una tabla con sus entradas enlazadas en una lista), se le da
   un `GlobalData`; un puntero a ella como argumento es un `GlobalPointer`.
   Una función que nunca retorna se prueba cortándola desde una función
   simulada, con `raise StopCall`.
7. Añadir sus casos a `tests/` y ejecutar `test_functions.py`.

Listar el contenido de un mapa:

```
python tools/mapinfo.py "<carpeta maps>\zanzibar.map" --tags
```

Exportar la geometría de un nivel a un archivo `.obj` (se abre en Blender
con *File > Import > Wavefront*) y dibujarla como imagen:

```
python tools/bsp2obj.py "<carpeta maps>\zanzibar.map" zanzibar.obj
python tools/obj_preview.py zanzibar.obj zanzibar.png
```

El `.obj` lleva la geometría que dibuja el juego y los objetos que el nivel
coloca (escenografía, máquinas, cajas, vehículos), con un material por
shader y su textura principal como PNG en una carpeta al lado. Con
`--collision` lleva la geometría que usa la física.

Sacar un modelo o una textura sueltos (el nombre puede ser el final del
nombre del tag):

```
python tools/model2obj.py "<carpeta maps>\zanzibar.map" warthog\warthog warthog.obj
python tools/bitmap2png.py "<carpeta maps>\zanzibar.map" zanzibar_seawall seawall.png
```

### Smart App Control de Windows

Con Smart App Control activado, Windows puede negarse a ejecutar los
programas recién compilados porque no están firmados ("Una directiva de
Control de aplicaciones bloqueó este archivo"). Por eso todas las
herramientas son scripts de Python, y el código de `source/` se prueba
dentro de un emulador. Es un límite real para el futuro: el
ejecutable del juego, cuando exista, tendrá el mismo problema.

## Organización

| Carpeta | Contenido |
| --- | --- |
| `source/` | El código descompilado del juego. Cada función indica su dirección en el ejecutable. |
| `tools/` | Programas y scripts propios: inspección del disco, exportación desde Ghidra, pruebas. |
| `tools/ghidra_scripts/` | Scripts que se ejecutan dentro de Ghidra. |
| `disc/`, `ghidra/`, `build/` | Locales, fuera de git: el ejecutable, el proyecto de Ghidra y los resultados de compilar. |

### Analizar el ejecutable

Importar y analizar (una vez):

```
analyzeHeadless ghidra halo2 -import disc\default.xbe -scriptPath tools\ghidra_scripts -postScript ExportFunctions.java ghidra\functions.csv
```

Descompilar funciones por dirección o por una constante que usen:

```
analyzeHeadless ghidra halo2 -process default.xbe -noanalysis -readOnly -scriptPath tools\ghidra_scripts -postScript DecompileFunctions.java ghidra\decompiled 122870 const:68656164
```
