# halo-2-universal

Descompilación en curso de **Halo 2 para Xbox** (build `02.09.27.09809`,
`halo2ship.exe`, SHA-256 del `default.xbe`
`92746c937cd3d41e247cef7c9aea7d0dd1f3d09066b8f445c7665efaa0c6d17c`).

El objetivo a largo plazo es un port nativo del juego, como el que
[halo-ce-universal](https://github.com/cybersecurity/halo-ce-universal) hizo
con Halo: Combat Evolved.

**Estado: muy temprano.** El juego no arranca ni está cerca de hacerlo. Hoy
el proyecto abre los mapas del juego y lista su contenido. Consulta
[CHANGELOG.md](CHANGELOG.md) para ver qué hay hecho.

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
| clang ([llvm-mingw](https://github.com/mstorsjo/llvm-mingw)) y [ninja](https://ninja-build.org/) | Compilar |
| Python 3 | Scripts de `tools/` |

## Compilar y probar

```
ninja
python tools/test_maps.py "<carpeta maps del juego>"
```

`ninja` genera `build/mapinfo.exe`. La prueba abre todos los mapas y
comprueba que cada tag está donde el cargador lo espera.

```
build\mapinfo.exe "<carpeta maps>\zanzibar.map" --tags
```

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
