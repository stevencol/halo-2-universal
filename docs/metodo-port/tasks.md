# Método de port: tareas

En orden de lo que más rigor añade por esfuerzo. Cada tarea termina con
`check_milestone.py` en `TODO BIEN` sobre el hito 879 y uno nuevo, y con la
batería completa sin regresiones.

- [ ] 1. **Cobertura de saltos del original** (R6)
  - [ ] 1.1 Hook en `difftest.py`: direcciones ejecutadas y saltos
        tomados y no tomados de la función original.
  - [ ] 1.2 Informe por función en `test_functions.py --coverage`.
  - [ ] 1.3 `check_milestone.py`: aviso por cada salto sin cubrir que no
        esté justificado con `# no cubierto: 0x…`.
  - [ ] 1.4 Pasarlo por los hitos 855–879 y anotar lo que no se cubra.

- [ ] 2. **Casos grabados con el emulador** (R7)
  - [ ] 2.1 `tools/emulator_capture.py` por GDB de xemu: puntos de ruptura
        en 1–10 funciones y volcado de la entrada y la salida.
  - [ ] 2.2 Formato `tests/captured/<dirección>.json`.
  - [ ] 2.3 Reproducción en `difftest.py` (las mismas direcciones en las dos
        ejecuciones; los punteros a funciones traducidos).
  - [ ] 2.4 Grabar 20 funciones del camino al menú y compararlas.

- [ ] 3. **Candidatos con el grafo de llamadas real** (R1.2)
  - [ ] 3.1 `tools/candidates.py`: los callees de cada función desde el
        desensamblado completo, sin filtro por cercanía.
  - [ ] 3.2 Marcar las externas sin fuente aparte de las hechas.

- [ ] 4. **Contratos de los mocks** (R4.2, R4.3)
  - [ ] 4.1 `symbols/contracts.csv` y su lectura en el arnés.
  - [ ] 4.2 Un mock que devuelve algo fuera del contrato hace fallar la
        prueba.
  - [ ] 4.3 No comparar los argumentos que el callee no usa.

- [ ] 5. **Biblioteca de fixtures** (`tests/fixtures/`)
  - [ ] 5.1 Data arrays, tablas de objetos, tags, grafos y bsp, con
        punteros relativos calculados.
  - [ ] 5.2 Pasar a ella las pruebas que copian `some_objects`,
        `big_objects` y `some_graph`.

- [ ] 6. **Mutaciones sobre el AST** (R5.3)
  - [ ] 6.1 Mutador con libclang: constantes, operadores, sentencias.
  - [ ] 6.2 Comparar con `make_mutations.py` en los hitos 855–879 (que
        encuentre lo que este se saltó, como en 873).

- [ ] 7. **Reglas de trampas en `check_milestone.py`** (R9.3): las de la
      tabla de `design.md`.

- [ ] 8. **Integración en cadena** (R4.4): un modo de la batería sin mocks
      de funciones descompiladas, solo de externas.

- [ ] 9. **Batería en CI** (R8.2): un trabajo que pasa la batería completa en
      cada push y deja un informe de regresiones.

- [ ] 10. **Puntos de control y sustitución progresiva** (R8.3, R8.4)
  - [ ] 10.1 Volcados de memoria de xemu en el arranque, al cargar y en el
         menú.
  - [ ] 10.2 Parche del XBE para desviar una función a la nuestra y
         comprobar que se llega al menú.

- [ ] 11. **Modo estricto** (R3.5): -0 y +0 distintos, NaN bit a bit, el
      orden de las escrituras.

- [ ] 12. **Tipos en vez de offsets** (R2.3): `struct` con `static_assert`
      para objetos, data arrays y tags, a medida que se entiendan.
