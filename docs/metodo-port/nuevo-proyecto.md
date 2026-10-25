# Empezar otro proyecto con este método

Pasos para arrancar la descompilación de otro juego (u otra consola)
reutilizando lo de este repositorio. Están en orden: no pasar al siguiente
hasta que el anterior funcione.

## 0. Antes de empezar

- [ ] Tener tu propia copia original del juego y de la consola o su BIOS.
      El repositorio no publica nunca nada del juego (ni el ejecutable, ni
      sus datos, ni capturas de su memoria): solo el código propio, las
      pruebas y las tablas de símbolos.
- [ ] Buscar si ya existe otro proyecto de descompilación del mismo juego
      o de la saga, y comprobar su licencia. Sirve de guía para leer, no
      para copiar (ver R1.3).
- [ ] Decir desde el principio en el README de qué proyecto se parte y cuál
      es la meta (por ejemplo: «llegar al menú principal»).

## 1. El emulador y el ejecutable

- [ ] Elegir el emulador de la consola y comprobar qué permite: trazas de
      instrucciones, stub de GDB, puntos de ruptura, volcados de memoria,
      estados guardados (tabla en `design.md`, «Por consola»).
- [ ] Hacer que el juego llegue a la meta en el emulador.
- [ ] Sacar el ejecutable del disco (XBE, DOL, ELF, NSO…) y saber cómo se
      carga: secciones, direcciones y, si las hay, claves o compresión.

## 2. El mapa del ejecutable

- [ ] Analizarlo en Ghidra (o IDA) y exportar la lista de funciones a
      `ghidra/functions.csv` (dirección, tamaño, nombre).
- [ ] Separar el código del juego del de las bibliotecas (SDK, CRT, motor de
      física): suelen estar en zonas distintas del ejecutable. Las
      bibliotecas no se descompilan; se buscan por su nombre.
- [ ] Grabar con el emulador la traza hasta la meta y sacar de ella la
      lista de funciones del camino (como `tools/menu_path.py` →
      `symbols/menu_path.csv`).

## 3. El arnés diferencial

Es la pieza que garantiza el rigor; sin ella no se empieza a descompilar.

- [ ] Copiar `tools/difftest.py`, `tools/test_functions.py`,
      `tools/mutation_check.py`, `tools/make_mutations.py` y
      `tools/check_milestone.py`.
- [ ] Adaptar a la nueva CPU (Unicorn admite x86, ARM, ARM64, MIPS y PPC) y
      al nuevo formato: el cargador del ejecutable, los registros de las
      convenciones de llamada, cómo se devuelven los reales.
- [ ] Fijar el compilador del código propio (como el clang 19.1.5 aquí) y
      dejarlo escrito en `AGENTS.md`.
- [ ] Primer hito: una función hoja muy pequeña (sin llamadas), con su
      prueba de 400 casos y sus mutaciones, hasta que `check_milestone.py`
      diga `TODO BIEN`. Esto demuestra que el arnés funciona.

## 4. Las tablas y el registro

- [ ] `symbols/functions.csv` y `symbols/globals.csv`, con el mismo
      formato. A estos archivos solo se añaden filas, nunca se reescriben.
- [ ] `CHANGELOG.md`, `ESTADO.md` y `README.md`, con `tools/milestone_docs.py`
      adaptado (un hito = un commit con su entrada y su porcentaje).

## 5. Los archivos para los agentes

Copiar y adaptar, cambiando el juego, la consola, las rutas y el
compilador:

- [ ] `AGENTS.md` con las «REGLAS OBLIGATORIAS» (empezar por las de este
      repositorio: casi todas valen para cualquier juego).
- [ ] `CLAUDE.md` (`@AGENTS.md`), `.cursor/rules/metodo-port.mdc`,
      `.kiro/steering/*.md` y `.clinerules`.
- [ ] Esta carpeta `docs/metodo-port/` entera. Al empezar, marcar en
      `tasks.md` lo que ya traiga hecho el arnés copiado.

## 6. Mejoras desde el principio

Lo que aquí se añadió tarde conviene tenerlo desde el primer hito (detalle
en `tasks.md`):

- [ ] Cobertura de los saltos del original (tarea 1).
- [ ] Casos grabados con el emulador (tarea 2).
- [ ] Candidatos con el grafo de llamadas real (tarea 3).
- [ ] Biblioteca de fixtures en vez de buffers hechos a mano (tarea 5).
- [ ] Las reglas de la tabla de trampas en `check_milestone.py` (tarea 7).

## 7. Ritmo de trabajo

- Hitos de 2 a 6 funciones del camino a la meta, de las más pequeñas a las
  más grandes, cada uno con su commit.
- Un push cada 5 hitos.
- La batería completa en los grandes avances (o en CI en cada push).
- Al pausar, dejar escrito en `ESTADO.md` en qué hito se está y qué sigue.

Para pedírselo a una herramienta: «empieza un proyecto nuevo para <juego>
de <consola> siguiendo `docs/metodo-port/nuevo-proyecto.md`, paso a paso,
sin saltarte ninguno».
