# Método de port: requisitos

Método para descompilar a mano un juego de consola y comprobar que cada
función hace lo mismo que la original. Sale de lo aprendido en
halo-2-universal (hitos 1–879). Vale para este proyecto y para los próximos
ports (otras consolas u otros juegos de la saga).

Idea central: **no se busca un binario idéntico byte a byte, sino que cada
función sea indistinguible del original desde fuera**, demostrado ejecutando
las dos y comparando, y comprobando que la prueba sabe detectar un error.

Cada requisito dice si ya existe en este repositorio (**HECHO**), si existe a
medias (**PARCIAL**) o si está por hacer (**NUEVO**).

---

## R1. Elegir qué descompilar

- R1.1 **HECHO** El orden lo marca lo que el juego ejecuta de verdad: una
  traza del emulador hasta el objetivo (por ejemplo, el menú principal),
  guardada en `symbols/menu_path.csv`.
- R1.2 **PARCIAL** Una función es candidata cuando todas sus llamadas ya
  están descompiladas o son externas que se pueden simular. El cálculo tiene
  que usar el grafo de llamadas real: no ocultar callees cercanos (fallo de
  `ready.py`) y distinguir «hecha» de «externa sin fuente».
- R1.3 **HECHO** Primero se busca la función en las fuentes de guía (otro
  proyecto de descompilación con licencia compatible, el juego anterior de
  la saga), para leerla y no para copiarla.
- R1.4 **HECHO** Se descartan, o se dejan para más tarde, las funciones que
  dependen de tablas virtuales, de Direct3D o de x87 trascendental
  (`fcos`/`fsin`) sin un modelo exacto, o que esperan en bucle (vblank).

## R2. Reescribir en C propio

- R2.1 **HECHO** C nuevo, con nombres y comentarios propios. La convención
  de llamada se toma del binario (`push` y `ret N`) y se anota en
  `symbols/functions.csv`.
- R2.2 **HECHO** Se respeta todo lo que cambia el resultado: el orden de
  las operaciones con reales, la precisión del x87 (con `long double` cuando
  el original calcula en 80 bits), los NaN, el redondeo de `fistp`
  (`lrintf`), los desbordamientos de 8 y 16 bits y la lectura con o sin
  signo.
- R2.3 **NUEVO** Ir pasando de offsets (`*(long *)(p+0x5C)`) a `struct` con
  nombre a medida que se entienden, con un `static_assert` del tamaño y de
  los offsets.

## R3. Prueba diferencial de cada función (arnés)

- R3.1 **HECHO** La función original (en Unicorn, sobre el XBE) y la nuestra
  (compilada con el clang fijado) se ejecutan con las mismas entradas.
- R3.2 **HECHO** Se compara: el valor devuelto en su registro, los bytes de
  cada buffer de argumento y de cada global, y la secuencia de llamadas a
  mocks y externas con sus argumentos. Los punteros se comparan como
  «a qué apuntan».
- R3.3 **HECHO** Al menos 400 casos por función, al azar con semilla fija,
  con valores límite incluidos: NONE, 0, negativos, NaN, el último índice y
  uno más.
- R3.4 **HECHO** Las entradas que hacen fallar al original (porque lee
  memoria no mapeada o sale de su tabla) no se prueban, y se anota por qué en
  la prueba y en el CHANGELOG.
- R3.5 **NUEVO** Modo estricto opcional: -0 distinto de +0, NaN comparados
  bit a bit y el orden de las escrituras a las globales.

## R4. Mocks y contratos

- R4.1 **HECHO** Una llamada a otra función se puede simular. El mock
  registra lo que hay que comparar y puede escribir memoria
  (`harness.mu.mem_write`).
- R4.2 **NUEVO** Contrato por función descompilada: el rango del valor
  devuelto y la memoria que escribe. Un mock no puede devolver lo que la
  real no devolvería (clang deduce rangos: el caso de `e6fe0`, que solo
  devuelve 0 o 1).
- R4.3 **NUEVO** No se comparan los argumentos que el callee no usa (clang
  no los pasa).
- R4.4 **NUEVO** Pruebas en dos niveles: con mocks (aislada) y en cadena
  con los callees reales (integración), al menos en los grandes avances.

## R5. Mutaciones: la prueba tiene que poder fallar

- R5.1 **HECHO** Se rompen líneas de nuestro C (constantes ±1, operadores
  dados la vuelta, líneas borradas) y cada mutación tiene que hacer fallar
  la prueba.
- R5.2 **HECHO** Una mutación que no cambia nada observable se quita con un
  comentario que diga por qué.
- R5.3 **NUEVO** Las mutaciones se generan sobre el AST de clang, no sobre
  el texto, para que no se escapen funciones con ayudantes inline ni
  archivos con otros finales de línea.

## R6. Cobertura del original

- R6.1 **NUEVO** El arnés registra qué instrucciones y qué saltos del
  original se ejecutaron en los casos de cada función.
- R6.2 **NUEVO** `check_milestone.py` avisa de cada salto no cubierto. Hay
  que cubrirlo o justificarlo (inalcanzable, o un caso que hace fallar al
  original).

## R7. Casos reales grabados con el emulador

- R7.1 **PARCIAL** El emulador de la consola es la fuente de verdad. En
  Xbox, xemu: traza `-d in_asm` (ya se usa) y stub de GDB `-s`. En otras
  consolas, el que tenga depurador (Dolphin, PCSX2, RPCS3, mGBA…); al
  empezar el port hay que confirmar qué permite cada uno.
- R7.2 **NUEVO** Un script graba unas cuantas llamadas reales de las
  funciones elegidas (pocas a la vez: con puntos de ruptura en todas, xemu
  es demasiado lento): los registros, los argumentos, la memoria que leen y
  lo que devuelven y escriben.
- R7.3 **NUEVO** El arnés reproduce esos casos en las dos versiones,
  además de los casos al azar.

## R8. Batería y puntos de control del juego

- R8.1 **HECHO** La batería completa (todas las pruebas) solo en los grandes
  avances y en un worktree aparte, para no bloquear el trabajo.
- R8.2 **NUEVO** La batería completa en una máquina aparte o en CI en cada
  push, con un informe de las regresiones.
- R8.3 **NUEVO** Volcados de memoria del emulador en momentos clave
  (arranque, carga del mapa, menú) para compararlos con los del port cuando
  arranque.
- R8.4 **NUEVO** Sustitución progresiva: desviar en el emulador una función
  del original a la nuestra (parcheando el ejecutable) y comprobar que el
  juego sigue llegando igual al objetivo.

## R9. Registro y reglas

- R9.1 **HECHO** Cada hito tiene su commit, su entrada en español en el
  CHANGELOG y su porcentaje (global y del camino), con `milestone_docs.py`.
- R9.2 **HECHO** `check_milestone.py` tiene que acabar en `TODO BIEN` antes
  de cada commit.
- R9.3 **NUEVO** Cada trampa encontrada pasa a ser una comprobación
  automática, no solo una nota (ver la tabla de trampas en `design.md`).
