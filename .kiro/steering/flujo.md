---
inclusion: always
---

Antes de nada, lee y sigue #[[file:AGENTS.md]] (sobre todo «REGLAS
OBLIGATORIAS» y «Lista antes de cada commit») y lee `ESTADO.md`.

**Lo más importante: antes de dar un hito por terminado ejecuta**

```
python tools\check_milestone.py <hito> --run > build\scratch\salida.txt 2>&1
```

**y LEE `build\scratch\check_milestone.txt`.** Tu terminal no muestra bien
la salida de los comandos: lee siempre ese archivo, no adivines. Tiene que
acabar en `TODO BIEN`; si dice `ERROR`, corrige lo que dice y repite. No digas
que algo pasa sin haberlo leído ahí. No hagas commit: déjalo para revisión.

Copia la forma de `tests\test_small_741.py` y `tests\mutations_741.py`; no
inventes otra.

Las reglas que más se rompieron (el script las comprueba):

- `symbols\functions.csv` y `symbols\globals.csv`: solo AÑADIR filas al final;
  nunca reescribirlos, ordenarlos ni filtrarlos.
- Las tablas globales se dan en las pruebas como bytes (`GlobalData`), nunca
  como `Buffer`; en C son arrays de su tamaño.
- Tipo de retorno en `functions.csv` SOLO de la lista de AGENTS.md (`int`,
  `bool`, `pointer`…): con `long` el valor devuelto no se compara.
- `ret N` al final = `stdcall`.
- Sin ensamblador en el C. Sin `python -c` ni `cd ...;` delante de los comandos.
- Si la misma prueba falla 3 veces, para y explica; no reintentes en bucle.
