---
inclusion: always
---

Antes de nada, lee y sigue #[[file:AGENTS.md]] (sobre todo «REGLAS
OBLIGATORIAS» y «Lista antes de cada commit») y lee `ESTADO.md`.

Las que más se rompieron:

- `symbols\functions.csv` y `symbols\globals.csv`: solo AÑADIR filas al final;
  nunca reescribirlos, ordenarlos ni filtrarlos.
- Las tablas globales se dan en las pruebas como bytes (`GlobalData`), nunca
  como `Buffer`; en C son arrays de su tamaño.
- Tipo de retorno en `functions.csv` SOLO de la lista de AGENTS.md (`int`,
  `bool`, `pointer`…): con `long` el valor devuelto no se compara.
- `ret N` al final = `stdcall`.
- Antes del commit: mutaciones (`tools\mutation_check.py`) y
  `git diff --stat symbols\` solo con líneas añadidas.
- Sin `python -c` ni `cd ...;` delante de los comandos.
- Si la misma prueba falla 3 veces, para y explica; no reintentes en bucle.
