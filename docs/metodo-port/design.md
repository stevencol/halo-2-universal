# Método de port: diseño

## Flujo de un hito

```
traza del emulador ─► candidatos (grafo de llamadas real)
        │
        ▼
guía (otra descompilación / juego anterior) + desensamblado del binario
        │
        ▼
C propio  ─►  fila en symbols/functions.csv (registros, retorno, stdcall)
        │
        ▼
prueba diferencial ◄── casos al azar (≥400) + casos grabados en el emulador (R7)
        │                     │
        │                     └─ mocks con contrato (R4)
        ▼
cobertura del original (R6) ── saltos sin cubrir: cubrir o justificar
        │
        ▼
mutaciones (R5) ── 0 sin cazar; las equivalentes, fuera con su razón
        │
        ▼
check_milestone.py --run ─► TODO BIEN ─► milestone_docs.py ─► commit ─► push
        │
        └─ grandes avances: batería completa + integración en cadena (R4.4, R8)
```

## Piezas

| Pieza | Hoy | Qué cambiar |
| --- | --- | --- |
| `tools/difftest.py` (arnés) | Unicorn + ELF de `source/`; compara el valor devuelto, los buffers, las globales y las llamadas | Añadir cobertura de saltos (R6), modo estricto (R3.5) y reproducción de casos grabados (R7.3) |
| `tools/test_functions.py` | Pasa las pruebas por archivo o por función | Leer contratos y casos grabados |
| `tools/make_mutations.py` | Mutaciones por texto | Mutar sobre el AST de clang (R5.3) |
| `tools/mutation_check.py` | Aplica y restaura las mutaciones, informa de las que se escapan | Igual |
| `tools/check_milestone.py` | CSV solo con filas añadidas, tipos de retorno, `ret N` = stdcall, formato de las pruebas, mutaciones | Añadir las reglas de la tabla de trampas y el umbral de cobertura |
| `tools/menu_path.py` | Lee la traza de xemu y calcula el % del camino | Cruzar con la cobertura del emulador para priorizar |
| `build/scratch/ready.py` | Candidatos «listos» | Sustituirlo por `tools/candidates.py` con el grafo de llamadas real (R1.2) |
| Nuevo `tools/emulator_capture.py` | — | Por GDB (xemu `-s`): pone puntos de ruptura en N funciones, vuelca los registros y la memoria a la entrada y a la salida y escribe `tests/captured/<dirección>.json` |
| Nuevo `tests/fixtures/` | Generadores sueltos (`some_objects`, `big_objects`, `some_graph`…) | Una biblioteca común: data arrays, tablas de objetos, tags, grafos de animación, bsp, con sus punteros relativos al buffer |
| Nuevo `symbols/contracts.csv` | — | `función,retorno_mín,retorno_máx,escribe` para los mocks (R4.2) |

## Casos grabados (R7)

- Formato JSON por función: la entrada (los registros, la pila y las
  regiones de memoria que lee, con su dirección) y la salida (el valor
  devuelto y las regiones escritas).
- Al reproducirlo, las regiones se cargan en las mismas direcciones en las
  dos ejecuciones. Los punteros a funciones del juego se cambian por los
  nuestros con la tabla de símbolos.
- Se graban pocas funciones a la vez: con puntos de ruptura en todas, el
  emulador va demasiado lento.

## Cobertura (R6)

- Hook de Unicorn sobre el rango de la función original: el conjunto de
  direcciones ejecutadas y, en cada salto condicional, si se tomó y si no.
- Informe por función: «saltos 14/16; sin cubrir 0x0016dd9c (cuenta < 0)».
  Lo no cubierto se justifica en un comentario de la prueba:
  `# no cubierto: 0x… (el original falla con esta entrada)`.

## Tabla de trampas → regla automática (R9.3)

| Trampa vista | Cómo se detecta |
| --- | --- |
| `0x6E+index`: token inválido en C (exponente hex) | Expresión regular `0x[0-9A-Fa-f]*[eE][+-]` en el código nuevo |
| Una global que es un puntero, dada como bytes en la prueba | Ya en parte; comprobar el tipo declarado en C contra el valor de la prueba |
| Punteros de un `Buffer` relativos a otra base | El generador de fixtures los calcula siempre; prohibir offsets a mano en `pointers=` |
| Mock que devuelve valores imposibles | `contracts.csv` (R4.2) |
| Argumento no usado que clang no pasa | Análisis de los parámetros usados en el callee; el mock no los compara |
| Función decompilada con otro nombre ya existente | Ya en parte (fila duplicada); buscar también la dirección en los comentarios |
| Prototipo duplicado con otro tipo en el mismo archivo | Compilar con `-Werror=incompatible-pointer-types` en los archivos tocados |
| Tabla del original indexada fuera de rango (tono = 1 en `hsv_to_rgb`) | La cobertura marca el salto; la prueba lo justifica |
| `long double` necesario (x87 de 80 bits) | Aviso cuando el desensamblado tiene `fild`/`fmul` sin `fstp` intermedio |

## Por consola

| Consola | Emulador | Lo que hay que confirmar al empezar |
| --- | --- | --- |
| Xbox | xemu (QEMU) | `-d in_asm` para trazas; `-s` GDB (ya probado: lento con miles de puntos) |
| GameCube / Wii | Dolphin | Depurador, puntos de ruptura en memoria, volcados |
| PS2 | PCSX2 | Depurador y trazas |
| PS3 | RPCS3 | Depurador PPU/SPU |
| GBA | mGBA | Stub de GDB |

El arnés diferencial necesita una CPU emulable (Unicorn: x86, ARM, MIPS,
PPC) y un cargador del formato ejecutable de la consola (XBE, DOL, ELF…).
