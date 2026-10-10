# Registro de cambios

Los cambios más recientes van arriba.

## 2026-10-09 — Qué fue de cada paquete: contestado, perdido, o contestado antes de tiempo

Dos funciones en `networking/network_connection.c`, 557 bytes entre las dos. De 1499
funciones se pasa a 1501, el 13,32 % de las del juego (el 8,977 % de sus bytes).
**Son ya más de 1500 funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_connection_next_event` | `0x000965e0` | `networking/network_connection.c` |
| `network_connection_next_result` | `0x00096b00` | `networking/network_connection.c` |

### Lo que se sabe

- `network_connection_next_result` dice **qué fue del paquete al que le
  toca** (los paquetes se resuelven en orden):
  - si fue contestado: 1, o 2 si eso ya se había dicho antes de tiempo;
  - si no, se da por **perdido** (3) cuando se ha esperado por él el
    tiempo de espera más la «pérdida» acumulada, **o cuando ya se han
    contestado tantos paquetes posteriores** (un número del juego, en
    `0x004cf6f0`): es la retransmisión rápida de TCP. Entonces la pérdida
    sube, como en el hito 432.
  - Con una cosa u otra, pasa el turno al siguiente y su tamaño se resta
    de los bytes mandados sin resolver.
- `network_connection_next_event` es lo que el canal llama en bucle (hito
  411): primero lo anterior; y si al paquete que le toca aún no se le
  sabe nada, **avisa del primer paquete posterior que ya fue contestado**
  (4), una sola vez por paquete.
- Con esto se entienden los números que el canal recibía: 1 y 4 son «ha
  llegado respuesta» (el canal pasa a conectado y apunta la hora), 1 y 2
  «contestado», 3 «perdido», y 6 «el más viejo se ha soltado» (hito 434).
- **Todo lo que el canal llama de su pieza principal está ya
  descompilado.**

### Sin comprobar

- **Romper líneas, pendiente en las dos** y en las cuatro del hito 434.
  De la comprobación repetida de la tanda anterior, tres de cuatro ya se
  notan; la que no (devolver el «sí» de la clase sin pasarlo a 1) no la
  pueden ver las pruebas, que comparan un «sí o no» solo como verdadero o
  falso.
- En este hito se pasaron las pruebas de `network_connection` (11
  funciones) y `network_channel` (25).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El número de cada paquete que se manda

Cuatro funciones en `networking/network_connection.c`, 596 bytes entre las cuatro. De 1495
funciones se pasa a 1499, el 13,31 % de las del juego (el 8,958 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_connection_reset` | `0x00095cf0` | `networking/network_connection.c` |
| `network_connection_new_sequence` | `0x00096510` | `networking/network_connection.c` |
| `network_connection_drop_oldest_sent` | `0x00096ce0` | `networking/network_connection.c` |
| `network_connection_window_advance` | `0x001a4840` | `networking/network_connection.c` |

### Lo que se sabe

- `network_connection_reset` pone una conexión a cero. **El número del
  primer paquete sale de la hora** (multiplicada por un número del juego,
  en milésimas, y recortada a un byte), así dos conexiones seguidas no
  empiezan igual. Los tiempos de ida y vuelta empiezan en cuatro valores
  que el juego tiene en `0x004cf70c` a `0x004cf718`.
- `network_connection_new_sequence` da **el número del siguiente paquete
  que se manda**, o nada si no se puede: si la conexión dice que no puede
  mandar, o si el número quedaría a 128 o más del último contestado o del
  primero de la cuenta, o si el anillo sigue lleno. Entonces apunta que
  está llena. Si el anillo estaba lleno, antes suelta el más viejo.
- `network_connection_drop_oldest_sent` **da por perdido el paquete más
  viejo sin contestar** cuando toca: si ya tiene uno de dos bits, si se le
  obliga, o si se ha esperado por él el tiempo de espera del hito 432 más
  un margen. Avisa con el número 6 y el del paquete: es de aquí de donde
  le llegaba al canal «un paquete no fue contestado» (hito 411).
- `network_connection_window_advance` adelanta el principio de una
  ventana. Está lejos del resto en el ejecutable (`0x001a4840`), junto a
  otras cosas genéricas.
- Tres de estas eran funciones que el canal llamaba sin estar hechas. De
  las seis que llamaba de esta pieza queda una: `0x000965e0`.

### Sin comprobar

- **Romper líneas, pendiente en las cuatro.** Tanda de los hitos 420 a
  424, hecha: 61 líneas rotas, 56 notadas; cuatro de las otras cinco ya
  tienen su caso en las pruebas (se comprueba en la tanda que corre
  ahora) y la quinta no se puede notar desde fuera.
- En este hito se pasaron las pruebas de `network_connection` (9
  funciones) y `network_channel` (25).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las dos ventanas de paquetes de una conexión

Cuatro funciones en `networking/network_connection.c`, 427 bytes entre las cuatro. De 1491
funciones se pasa a 1495, el 13,27 % de las del juego (el 8,938 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_connection_get_sent_packet` | `0x000966d0` | `networking/network_connection.c` |
| `network_connection_set_sent_size` | `0x00096810` | `networking/network_connection.c` |
| `network_connection_received` | `0x00096860` | `networking/network_connection.c` |
| `network_connection_advance_received` | `0x00096c50` | `networking/network_connection.c` |

### Lo que se sabe

- La pieza principal de un canal lleva **dos ventanas deslizantes**, una
  de los paquetes que han llegado y otra de los que ha mandado. Las dos
  tienen la misma forma (28 bytes: si está en uso, cuántos sitios tiene
  el anillo, el número del último paquete, el de antes del primero, y
  dónde empieza y cuántos hay) y lo de cada paquete se guarda en un
  anillo:
  - de los que llegaron, una palabra por paquete (desde `0x4C`), con un
    bit que dice «llegó»;
  - de los mandados, 16 bytes por paquete (desde `0x14C`): cuándo, de qué
    tamaño y unos bits.
- `network_connection_received`: **ha llegado el paquete número tal.** La
  ventana avanza hasta él, uno por uno, y se marca. Es lo que el canal
  hacía con el número que leían sus piezas (hito 414).
- `network_connection_advance_received`: los primeros de la ventana que
  ya llegaron se sueltan, hasta el primero que falta.
- `network_connection_get_sent_packet` da lo guardado de un paquete
  mandado, y `network_connection_set_sent_size` apunta su tamaño y lo suma
  a los bytes mandados (lo que el canal hacía al mandar, hito 415).
- Dos de estas eran funciones que el canal llamaba sin estar hechas
  (`unknown_00096860` y `unknown_00096810`).

### Corregido

- Al probar salió un fallo mío antes de entrar nada en el repositorio:
  había puesto la ventana de los mandados cuatro bytes más allá de donde
  está (cada ventana empieza con un byte que dice si está en uso). La
  prueba contra el juego lo detectó a la primera.

### Sin comprobar

- **Romper líneas, pendiente** aquí y en las de los hitos 426 a 432.
- El juego no mira si el paquete existe antes de apuntarle el tamaño o
  marcarlo: con un número de fuera de la ventana escribiría en la
  dirección cero. Las pruebas solo usan números de dentro.
- En este hito se pasaron las pruebas de `network_connection` (5
  funciones) y `network_channel` (25).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuánto tarda en contestar la otra consola

Una función en un archivo nuevo, `networking/network_connection.c`, de 268 bytes. De 1490
funciones se pasa a 1491, el 13,24 % de las del juego (el 8,924 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_connection_update_times` | `0x00096d80` | `networking/network_connection.c` |

### Lo que se sabe

- Primera función de la pieza principal de un canal (las «cosas de
  `unknown_004d87d8`» de los hitos 406 a 415, la que lleva la cuenta de
  los paquetes mandados y sus respuestas). El nombre del archivo es una
  suposición.
- `network_connection_update_times` es **el cálculo del tiempo de ida y
  vuelta**, y es el mismo que usa TCP:
  - Con cada respuesta, el tiempo suavizado se acerca al nuevo en una
    parte de la diferencia, y aparte se lleva cuánto varía.
  - La espera antes de dar un paquete por perdido es el tiempo suavizado
    más unas veces la variación, con dos mínimos.
  - Se guardan también el tiempo más corto visto y los datos de la última
    respuesta.
  - Hay un número de «pérdida» que sube con cada paquete sin respuesta
    (hasta un tope) y baja con cada respuesta (hasta cero).
- Los ocho números que gobiernan el cálculo (cuánto se suaviza, por
  cuánto se multiplica, mínimos y topes) son globales, de `0x004cf6f8` a
  `0x004cf724`. En el ejecutable están a cero: alguien los pone al
  arrancar.
- De aquí salen dos de los números que el observador y el canal ya leían
  de esta pieza sin saber qué eran: `0x96C` es el tiempo suavizado y
  `0x970` su variación (hito 380), y `0x968` el tiempo más corto (hito
  411).

### Sin comprobar

- **Romper líneas, pendiente** aquí y en las de los hitos 426 a 431.
- En este hito se pasaron las pruebas de `network_connection` (1
  función, 3000 casos).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las vistas de los sucesos: vaciar una

Una función en un archivo nuevo, `networking/replication_event_view.c`, de 457 bytes. De 1489
funciones se pasa a 1490, el 13,23 % de las del juego (el 8,915 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_event_view_reset` | `0x00096ed0` | `networking/replication_event_view.c` |

### Lo que se sabe

- Los sucesos (hito 419) tienen sus propias vistas, como las cosas: lo que
  a cada una de las otras consolas le falta por saber.
- `replication_event_view_reset` vacía una: devuelve al montón lo que
  guardaba de los paquetes sin contestar, y cada suceso que la vista
  tenía pendiente deja de estarlo. **Un suceso que ya no espera a ninguna
  vista se tira**: se avisa, se saca de la lista y se suelta, igual que
  al mandarlo sin nadie a quien contárselo.
- Era la función que el gestor de sucesos llamaba para cada vista sin
  saberse qué hacía: `replication_event_manager_unknown_00089dc0` pasa a
  llamarse `replication_event_manager_reset_views`.
- Del suceso se conoce un campo más (`0x24`): un bit por cada vista que
  lo ha mandado en un paquete del que todavía no se sabe nada.

### Corregido

- Las pruebas de `simulation_entity_database_read_update` no notaban que
  se quitara la línea que pone a cero los bits leídos, porque la clase de
  la cosa siempre los escribía. Ahora a veces no los escribe. La función
  estaba bien.

### Sin comprobar

- **Romper líneas, pendiente** aquí y en las de los hitos 426 a 430.
- En este hito se pasaron las pruebas de `replication_event_view` (1
  función), `replication_event_manager` (6) y
  `simulation_entity_database` (23).
- Del módulo quedan 8 en la lista, de `0x00096d80` a `0x00097d80`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Qué pasa cuando un paquete llega, o se pierde

Una función en `networking/replication_entity_view.c`, de 748 bytes. De 1488
funciones se pasa a 1489, el 13,22 % de las del juego (el 8,900 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_packet_result` | `0x000992d0` | `networking/replication_entity_view.c` |

### Lo que se sabe

- `replication_entity_view_packet_result` es **lo que da sentido a todo
  lo que la vista guardaba de cada paquete** (hito 427): se le dice el
  número de un paquete y si llegó a la otra consola o se perdió.
  - **Lo que se contó de una cosa.** Si el paquete se perdió, esos bits
    vuelven a estar pendientes, **menos los que ya se volvieron a contar
    en un paquete posterior** (no hace falta repetirlos). Si llegó, solo
    se cuenta.
  - **Una creación anunciada.** Si llegó, la cosa pasa al estado 3 («la
    otra consola la tiene»). Si se perdió, vuelve al 1 para anunciarse
    otra vez.
  - **Un borrado anunciado.** Si se perdió, vuelve al estado 3 para
    anunciarse otra vez. Si llegó, la cosa sale de la vista y la vista
    quita su bit de la cosa; cuando ninguna vista la tiene ya, se avisa y
    su sitio queda libre. Es el final de la vida de una cosa.
  - Después todo lo guardado del paquete se devuelve al montón de la red.
- Es la novena función de la tabla de una vista: la que
  `replication_entity_view_reset` (hito 425) llamaba para dar por
  perdidos todos los paquetes pendientes al vaciar la vista.
- Con esta función la réplica aguanta paquetes perdidos sin repetir de
  más: es el mecanismo de fiabilidad del juego en red, encima de un
  transporte que no garantiza nada.

### Sin comprobar

- **Romper líneas, pendiente** aquí y en las de los hitos 426 a 429.
- En este hito se pasaron las pruebas de `replication_entity_view` (17
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que una vista hace con un anuncio que llega

Dos funciones en `networking/replication_entity_view.c`, 735 bytes entre las dos. De 1486
funciones se pasa a 1488, el 13,21 % de las del juego (el 8,874 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_handle_message` | `0x00098fb0` | `networking/replication_entity_view.c` |
| `replication_entity_view_make_way` | `0x000991d0` | `networking/replication_entity_view.c` |

### Lo que se sabe

- `replication_entity_view_handle_message` es **el otro lado de los
  hitos 427 y 428: lo que hace la consola que recibe**, según los cinco
  anuncios:
  - 1 y 3, crear (una cosa o una cadena): lo que hubiera en esos sitios
    se aparta, y las cosas quedan puestas aquí con el mismo número que
    allí. Para una cadena se usa la función del hito 417; para una sola,
    lo mismo escrito en línea. Quedan en el estado 3.
  - 2 y 4, borrar: solo se hace caso si la vista tiene esa cosa con ese
    número y en el estado 3. Para borrar una cadena tiene que **coincidir
    entera**: las mismas cosas, con los mismos números y en el mismo orden.
  - 5, cómo está ahora: se pasa a la función 8 de la tabla `0x00450d60`
    (hito 423).
- `replication_entity_view_make_way` decide **si lo que hay en un sitio se
  aparta para una cosa nueva**. Compara las «veces usado» de los dos
  números, que dan la vuelta en 16: la nueva tiene que ser la misma o
  hasta tres más reciente (o estar a cuatro o más por detrás, que es
  estar lejos por delante dando la vuelta). Si no, se toma por un anuncio
  viejo que llega tarde y no se hace nada. Si se aparta, se borra ya, sola
  o con toda su cadena.
- Con esto el camino entero de una cosa está descompilado: se crea (hito
  422), se anuncia (427), se recibe y se pone (este), y lo mismo al
  borrarla.

### Sin comprobar

- **Romper líneas, pendiente en las dos** y en las de los hitos 426 a
  428.
- Un anuncio con un número fuera del 1 al 5 haría saltar al juego por una
  tabla de cinco sitios; no se prueba.
- En este hito se pasaron las pruebas de `replication_entity_view` (16
  funciones), `replication_entity_manager` (15) y
  `simulation_entity_database` (23).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Anunciar de una vez las cosas que se hicieron juntas

Dos funciones en `networking/replication_entity_view.c`, 639 bytes entre las dos. De 1484
funciones se pasa a 1486, el 13,19 % de las del juego (el 8,865 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_write_chain_creation` | `0x00098750` | `networking/replication_entity_view.c` |
| `replication_entity_view_write_chain_deletion` | `0x000988f0` | `networking/replication_entity_view.c` |

### Lo que se sabe

- Faltaban dos de los anuncios de tres bits del hito 427: **3 es «se han
  creado estas cosas juntas» y 4 «se han borrado juntas»**. Son las
  cadenas de hasta cuatro cosas del hito 416.
  - Detrás de los tres bits va cuántas son menos dos, en dos bits (de 2 a
    5; el juego nunca hace más de cuatro), y luego cada una como en el
    anuncio de una sola.
  - **O van todas o no va ninguna**: si una no se puede escribir, o no
    quedan los bits pedidos, el paquete vuelve a donde estaba.
  - Si va, la vista guarda lo mandado de cada una, como en el hito 427.

### Sin comprobar

- **Romper líneas, pendiente en las dos** y en las de los hitos 426 y
  427. Tanda del hito 425 (las tres primeras de las vistas), hecha: 19
  líneas rotas y las 19 notadas.
- Una cadena de una sola cosa escribiría «menos uno» en los dos bits; el
  juego deja escrito un aviso que nadie lee y sigue. Se hace igual.
- En las pruebas, el montón daba el mismo trozo de memoria cada vez y con
  dos peticiones seguidas la lista de lo mandado se enlazaba consigo
  misma: ahora da un trozo distinto cada vez.
- En este hito se pasaron las pruebas de `replication_entity_view` (14
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que una vista escribe en un paquete, y lo que guarda de ello

Cinco funciones en `networking/replication_entity_view.c`, 823 bytes entre las cinco. De 1479
funciones se pasa a 1484, el 13,17 % de las del juego (el 8,843 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_write_creation` | `0x00098620` | `networking/replication_entity_view.c` |
| `replication_entity_view_write_deletion` | `0x000986d0` | `networking/replication_entity_view.c` |
| `replication_entity_view_sent_creation` | `0x00098ac0` | `networking/replication_entity_view.c` |
| `replication_entity_view_sent_deletion` | `0x00098b60` | `networking/replication_entity_view.c` |
| `replication_entity_view_sent_update` | `0x00098bf0` | `networking/replication_entity_view.c` |

### Lo que se sabe

- **Cómo una vista le anuncia una cosa a su consola.** Cada anuncio
  empieza con tres bits: 1 es «se ha creado» y 2 «se ha borrado»; detrás
  va el número de la cosa (`unknown_000b5650`).
  - `replication_entity_view_write_creation` escribe la creación: lo de
    la cosa lo escribe la primera función de la tabla `0x00450d60`, que es
    la del hito 422. Aquí se cierra ese círculo: se ve quién la llamaba.
  - `replication_entity_view_write_deletion` escribe el borrado.
  - Las dos guardan dónde iba el paquete antes de escribir; **si después
    no quedan los bits que se piden, el paquete vuelve a donde estaba** y
    no se cuenta nada.
- **Lo que la vista guarda de lo que ha mandado**, hasta saber si el
  paquete llegó. Son trozos del montón de la red colgados del paquete que
  se está escribiendo:
  - `..._sent_creation` y `..._sent_deletion`: 12 bytes (la cosa, si es
    creación o borrado, y el siguiente). La cosa pasa al estado 2
    («anunciada, esperando») o al 4 («borrado anunciado»).
  - `..._sent_update`: 16 bytes (la cosa, los bits que se contaron, y un
    enlace a lo que se contó después de la misma cosa en otro paquete).
    Esos bits dejan de estar pendientes en la vista.
  - Si el montón no tiene memoria, la vista lo apunta en un byte.
- Con esto se conocen los estados de una cosa en una vista: 0 no está,
  1 por anunciar, 2 anunciada, 3 la otra consola la tiene, 4 borrado
  anunciado.

### Sin comprobar

- **Romper líneas, pendiente en las cinco** y en las cuatro del hito 426.
- En este hito se pasaron las pruebas de `replication_entity_view` (12
  funciones).
- Del módulo quedan 8 en la lista.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El estado de una cosa en una vista

Cuatro funciones en `networking/replication_entity_view.c`, 636 bytes entre las cuatro. De 1475
funciones se pasa a 1479, el 13,13 % de las del juego (el 8,816 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_new` | `0x00097f60` | `networking/replication_entity_view.c` |
| `replication_entity_view_next_entity` | `0x00098480` | `networking/replication_entity_view.c` |
| `replication_entity_view_unknown_00099640` | `0x00099640` | `networking/replication_entity_view.c` |
| `replication_entity_view_set_entity_state` | `0x00099690` | `networking/replication_entity_view.c` |

### Lo que se sabe

- `replication_entity_view_set_entity_state` es **la función central de
  una vista: poner una cosa en un estado**. Era la que la tabla de cosas
  llamaba al crear una cosa (hito 416) sin estar hecha. Lleva las cuentas
  de la vista al día:
  - cuántas cosas están en el primer estado (`0x502C`),
  - de las del tercero, cuántas se están borrando y la vista tiene
    todavía (`0x503C`), o si no cuántas tienen algo pendiente de contar
    (`0x5034`).
  - Una cosa que entra en la vista se pone la primera de **una lista
    enlazada de las cosas que están en ella** (la cabeza en `0x5024`, y en
    cada cosa el sitio de la siguiente). Una que sale se quita de la
    lista, buscándola entre las 1024.
- `replication_entity_view_new` prepara una vista: de qué tabla es, cuál
  de las quince es y su bit, y todo vacío.
- `replication_entity_view_next_entity` da la siguiente cosa que está en
  el primer o segundo estado (las que aún hay que anunciar a la otra
  consola).
- `replication_entity_view_unknown_00099640` dice si una cosa todavía no
  está creada, o está en el tercer estado y no se está borrando.
- Con esto el estado 0 es «no está en la vista», el 1 y el 2 «anunciándose»
  y el 3 «la otra consola ya la tiene».

### Sin comprobar

- **Romper líneas, pendiente en las cuatro.** Tanda de los hitos 416 a
  418 (la tabla de cosas), hecha: 41 líneas rotas y las 41 notadas. La de
  los hitos 420 a 425 está corriendo.
- En este hito se pasaron las pruebas de `replication_entity_view` (7
  funciones) y de `replication_entity_manager` (15).
- Del módulo quedan 13 en la lista.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las vistas: lo que otra consola sabe de cada cosa

Tres funciones en un archivo nuevo, `networking/replication_entity_view.c`, 428 bytes entre las tres. De 1472
funciones se pasa a 1475, el 13,09 % de las del juego (el 8,795 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_view_reset` | `0x00097fe0` | `networking/replication_entity_view.c` |
| `replication_entity_view_mark_entity` | `0x000980d0` | `networking/replication_entity_view.c` |
| `replication_entity_view_delete_entity` | `0x000995c0` | `networking/replication_entity_view.c` |

### Lo que se sabe

- Empieza el módulo de **las vistas**: cada una es lo que una de las
  otras consolas sabe de cada cosa (en qué estado está allí) y lo que
  todavía hay que contarle. Eran tres de las funciones que la tabla de
  cosas llamaba sin estar hechas.
- `replication_entity_view_reset` vacía una vista: recoge lo que había
  mandado y esperaba respuesta, y saca cada cosa de la vista. **Aquí se ve
  qué es la «cuenta de quién la tiene todavía» de una cosa** (hito 416):
  no es una cuenta, son bits, uno por vista. Una cosa borrada espera hasta
  que todas las vistas la han soltado; al soltarla la última, se avisa y
  su sitio queda libre.
- `replication_entity_view_mark_entity` suma a una vista lo que hay que
  contar de una cosa. Es lo mismo que el hito 418 hacía con las quince
  vistas de una vez.
- `replication_entity_view_delete_entity` avisa a una vista de que una
  cosa se borra: si la otra consola aún no sabía de ella, se olvida sin
  más; si sabía, la vista pone su bit en la cosa, que así espera a que
  esa consola se entere.

### Sin comprobar

- **Romper líneas, pendiente en las tres** y en las de los hitos 420 a
  424. Tanda del hito 415, hecha: en `network_channel_update`, 32 líneas
  rotas y las 32 notadas (una dejaba la prueba colgada). La de la tabla
  de cosas está corriendo.
- En este hito se pasaron las pruebas de `replication_entity_view` (3
  funciones) y de `replication_entity_manager` (15).
- Del módulo quedan 17 en la lista, de `0x00097f00` a `0x000998e0`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Leer de un paquete una cosa nueva y un cambio

Tres funciones: dos en `networking/simulation_entity_database.c` y una en el archivo nuevo
`networking/network_heap.c`, 1 144 bytes entre las tres. De 1469
funciones se pasa a 1472, el 13,07 % de las del juego (el 8,780 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `simulation_entity_database_decode_creation` | `0x0008a6f0` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_decode_update` | `0x0008aac0` | `networking/simulation_entity_database.c` |
| `network_heap_allocate_block` | `0x00096e90` | `networking/network_heap.c` |

### Lo que se sabe

- `simulation_entity_database_decode_creation` es el reverso de escribir
  una cosa (hito 422): **lee de un paquete una cosa que hizo otra
  consola**. Su clase en 5 bits; memoria para lo que se dijo al crearla,
  que la clase lee; el estado, que la clase construye a partir de eso; y,
  si un bit del paquete lo dice, el estado tal como viene. Contesta 0 si
  fue bien, 2 si no hay memoria y 3 si lo leído no vale (una clase que no
  existe, o bits que la clase no tiene).
- `simulation_entity_database_decode_update` **lee un cambio de una cosa
  que ya existe**: parte de una copia de cómo está aquí (o de lo que la
  clase construye, si el paquete viene en su cuarto estado) y la clase
  lee encima lo que ha cambiado.
- Las dos dejan la memoria en una lista de trozos de 8 bytes (tamaño,
  clase de trozo y dónde está) en vez de tocar la cosa: quien las llama
  decide luego. Son los trozos que la función 3 de la tabla se quedaba en
  el hito 423; ahora se ve de dónde salen.
- `network_heap_allocate_block` es pedir memoria al montón de la red, en
  el único sitio donde el juego no lo tiene escrito en línea.
- De la tabla `0x00450d60` quedan solo las dos que hacen cuentas con
  reales: `0x0008a460` y `0x0008a9c0`.

### Sin comprobar

- **Romper líneas, pendiente en las tres** y en las de los hitos 420 a
  423.
- En este hito se pasaron las pruebas de `simulation_entity_database`
  (23 funciones, con la del montón).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Varias cosas de una vez, y siete funciones más de la tabla

Ocho funciones en `networking/simulation_entity_database.c`. De 1461
funciones se pasa a 1469, el 13,04 % de las del juego (el 8,747 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `simulation_entity_database_create_entities` | `0x0008a210` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_created_remote` | `0x0008a920` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_set_state` | `0x0008ac40` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_unknown_0008aca0` | `0x0008aca0` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_deleting` | `0x0008ace0` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_deleted` | `0x0008ad20` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_adopt` | `0x0008ad30` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_adopted` | `0x0008ad70` | `networking/simulation_entity_database.c` |

### Lo que se sabe

- `simulation_entity_database_create_entities` crea **hasta cuatro cosas
  de una vez, encadenadas** (un vehículo con su torreta, por ejemplo):
  memoria para cada una, sitios para todas (hito 416) y cada una apuntada
  con su clase. Si falta algo, se devuelve toda la memoria.
- **La tabla `0x00450d60` está leída entera**: son dieciséis funciones, y
  es por donde la tabla de cosas del hito 416 habla con este módulo. Las
  seis que aquella llamaba sin saber a quién ya tienen nombre y cuerpo:
  - la 3, `..._created_remote`: llega una cosa que hizo otra consola. Se
    queda con la memoria que venía con ella, y su clase dice si ya está
    para contarse.
  - la 11, `..._deleting`, y la 13, `..._deleted`: una cosa se está
    borrando, y ya está borrada.
  - la 14, `..._adopt`, y la 15, `..._adopted`: esta consola se queda con
    las cosas de otra. Cada número cuenta un uso más de su sitio, igual
    que hace la tabla de cosas con los suyos (hito 418).
  - la 16 era la del hito 421 (qué contar a una consola nueva).
- Además: la 8, `..._set_state`, pone el estado de una cosa tal como
  llegó y avisa a su clase; y la 10 da un real de una cosa: nada, o 0,85.
- De la tabla de una clase se conocen ya dieciséis funciones.

### Sin comprobar

- **Romper líneas, pendiente en las ocho** y en las de los hitos 420 a
  422. Tanda de los hitos 415 y 419, hecha: 26 de 26 en
  `network_channel_write_packet` y 20 de 20 en el gestor de sucesos.
- Siete de las ocho no están en la lista de Ghidra y cuentan de más en el
  porcentaje (son ya once funciones así).
- De la tabla faltan cuatro: `0x0008a460`, `0x0008a6f0`, `0x0008a9c0` y
  `0x0008aac0` (y la `0x002ed7f0`, que es de otro sitio).
- En este hito se pasaron las pruebas de `simulation_entity_database`
  (20 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Crear una cosa, y escribirla y leerla en un paquete

Cuatro funciones en `networking/simulation_entity_database.c`. De 1457
funciones se pasa a 1461, el 12,97 % de las del juego (el 8,730 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `simulation_entity_database_create_entity` | `0x0008a110` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_unknown_0008a580` | `0x0008a580` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_write_creation` | `0x0008a5c0` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_read_update` | `0x0008aa50` | `networking/simulation_entity_database.c` |

### Lo que se sabe

- `simulation_entity_database_create_entity` es **crear una cosa de una
  clase, de principio a fin**: pide la memoria de sus dos trozos (hito
  421), le busca sitio y número en la tabla (hito 416) y la apunta (hito
  420). Si falta memoria o sitio, devuelve lo que había pedido y no crea
  nada. Junta tres hitos anteriores, y todo lo que llama está ya
  descompilado.
- Las otras tres **solo están en la tabla de funciones** `0x00450d60`
  (Ghidra no las tiene en su lista), que es la tabla con la que este
  módulo se presenta a la tabla de cosas del hito 416:
  - `simulation_entity_database_write_creation` **escribe una cosa en un
    paquete para una consola que no la tiene**: su clase en 5 bits, lo que
    se dijo al crearla, un bit que dice si hay estado, y el estado para
    los bits pedidos. Va bien si la clase escribió todos los bits.
  - `simulation_entity_database_read_update` **lee de un paquete el
    estado de una cosa**, por la misma función de la clase (la 15) con el
    primer argumento al revés. Va bien si leyó algún bit.
  - `simulation_entity_database_unknown_0008a580` pasa una cosa y tres
    números a la función 11 de su clase.
- De la tabla de una clase se conocen ya once funciones: 3, 4, 5, 6, 11,
  13, 15, 19, 20 y 21.

### Sin comprobar

- **Romper líneas, pendiente en las cuatro** y en las de los hitos 420 y
  421.
- Dos de las tres funciones de la tabla no están en la lista de Ghidra,
  así que cuentan de más en el porcentaje (son ya cuatro con las del hito
  412).
- En este hito se pasaron las pruebas de `simulation_entity_database`
  (12 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Activar una cosa, y la memoria de lo que se dice de ella

Cinco funciones: cuatro en `networking/simulation_entity_database.c` y una en
`networking/replication_event_manager.c`. De 1452
funciones se pasa a 1457, el 12,93 % de las del juego (el 8,722 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `simulation_entity_database_activate_entity` | `0x0008a400` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_get_initial_update` | `0x0008add0` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_allocate_creation_data` | `0x0008af70` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_allocate_state_data` | `0x0008b010` | `networking/simulation_entity_database.c` |
| `replication_event_set_data` | `0x0008b0a0` | `networking/replication_event_manager.c` |

### Lo que se sabe

- **La tabla de funciones de una clase de cosa** se va viendo. Cada clase
  dice cuánto ocupa el estado de una cosa suya (función 3), cuánto lo que
  se dice al crearla (4) y cuántos bits de «esto ha cambiado» tiene (5);
  se le avisa de que una cosa ya está (19), construye el estado a partir
  de lo que se dijo al crearla (20) y dice qué hay que contar de ella (21).
- `simulation_entity_database_activate_entity` pone una cosa en marcha:
  avisa a su clase, le hace construir el estado y **marca todos sus bits
  como cambiados**, para que se cuente entera la primera vez.
- `simulation_entity_database_get_initial_update` dice qué hay que contar
  de una cosa a una consola que no sabe nada de ella: el estado se
  construye de nuevo en un búfer de ceros de hasta 1024 bytes y se
  pregunta a la clase con todos los bits. Es, por su forma, la función 16
  de la tabla del hito 417 (la que se pregunta al llegar una vista nueva).
- `simulation_entity_database_allocate_creation_data` y
  `..._allocate_state_data` piden al montón de la red la memoria de los
  dos trozos de una cosa, a ceros. La primera no pide nada si la clase no
  dice nada al crear; la segunda pide siempre.
- `replication_event_set_data` guarda en un suceso una copia de lo que
  lleva aparte. Lo que en el hito 419 era un número sin nombre del suceso
  (`0x1C`) es el tamaño de eso.
- Pedir memoria al montón (pedir, hacer sitio si no hay, pedir otra vez)
  está también una sola vez, en `networking/network_heap.h`.

### Sin comprobar

- **Romper líneas, pendiente en las cinco** y en las cuatro del hito 420.
- El búfer del estado es de 1024 bytes: una clase que dijera más se
  saldría. No se prueba.
- En este hito se pasaron las pruebas de `simulation_entity_database` (8
  funciones) y de `replication_event_manager` (6).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que cada cosa es: su clase y lo que se dijo de ella

Cuatro funciones en un archivo nuevo, `networking/simulation_entity_database.c`. De 1448
funciones se pasa a 1452, el 12,89 % de las del juego (el 8,700 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `simulation_entity_database_reset` | `0x0008a030` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_flush_updates` | `0x0008a090` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_set_entity` | `0x0008ae80` | `networking/simulation_entity_database.c` |
| `simulation_entity_database_free_entity` | `0x0008aed0` | `networking/simulation_entity_database.c` |

### Lo que se sabe

- El cuarto módulo de la zona es **lo que las cosas son**. La tabla del
  hito 416 solo sabía de números y sitios; esta guarda, para cada una de
  las 1024, 32 bytes: su número, su clase, si ya está para contarse, los
  bits de lo que ha cambiado, y dos trozos de memoria del montón de la red
  (lo que se dijo de ella al crearla y cómo está ahora). El nombre del
  archivo es una suposición.
- `simulation_entity_database_set_entity` apunta una cosa, con su clase y
  sus dos trozos.
- `simulation_entity_database_free_entity` la quita y devuelve los trozos
  al montón.
- `simulation_entity_database_reset` las quita todas (la primera vez solo
  pone la tabla a cero).
- `simulation_entity_database_flush_updates` es **cómo los cambios llegan
  a las otras consolas**: por cada cosa con algo cambiado, su clase dice
  (función 21 de su tabla) qué hay que contar de ella, y eso se marca en
  todas las vistas con la función del hito 418. Aquí se juntan los dos
  módulos.
- Lo de devolver memoria al montón, que el juego tiene escrito igual en
  cada sitio, está ahora una sola vez, en `networking/network_heap.h`, y lo
  usa también el gestor de sucesos del hito 419.

### Aviso

- La tabla de funciones `0x00450d60` es de este módulo y tiene funciones
  que **no están en la lista de Ghidra** (`0x0008a460`, `0x0008a580`,
  `0x0008a5c0`, `0x0008a920`, `0x0008aa50`, `0x0008ac40`). Hay que
  hacerlas mirando la tabla, como en el hito 412.

### Sin comprobar

- **Romper líneas, pendiente en las cuatro.** La tanda de los hitos 415 a
  418 está corriendo; la del 419 ya se hizo: veinte líneas rotas, veinte
  notadas.
- En este hito se pasaron las pruebas de `simulation_entity_database` (4
  funciones) y de `replication_event_manager` (5).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — La lista de lo que pasa en la partida

Cinco funciones en un archivo nuevo, `networking/replication_event_manager.c`. De 1443
funciones se pasa a 1448, el 12,85 % de las del juego (el 8,685 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_event_manager_unknown_00089dc0` | `0x00089dc0` | `networking/replication_event_manager.c` |
| `replication_event_manager_new_event` | `0x00089df0` | `networking/replication_event_manager.c` |
| `replication_event_manager_remove_event` | `0x00089e70` | `networking/replication_event_manager.c` |
| `replication_event_free` | `0x00089eb0` | `networking/replication_event_manager.c` |
| `replication_event_manager_send_event` | `0x00089f20` | `networking/replication_event_manager.c` |

### Lo que se sabe

- Detrás de la tabla de cosas hay un tercer módulo pequeño: **la lista de
  lo que pasa en la partida y hay que contar a las otras consolas** (un
  disparo, un daño...). Las cosas duran; esto pasa una vez. El nombre del
  archivo es una suposición.
- Cada suceso son `0x2C` bytes que se piden al **montón de la red**
  (`unknown_004d87f8`, que aparece aquí por primera vez): algo con una
  tabla de funciones que da memoria, la recoge y hace sitio, y una cuenta
  de lo que tiene dado.
  - `replication_event_manager_new_event` pide uno; si no hay memoria,
    manda hacer sitio y lo intenta otra vez. Lo pone al final de la lista.
  - `replication_event_free` devuelve al montón lo que el suceso lleva
    aparte y, con el primer bit, el suceso.
  - `replication_event_manager_remove_event` lo saca de la lista y lo
    suelta.
- `replication_event_manager_send_event` es **mandar contar un suceso**:
  se apunta la hora y, por cada vista de la máscara que exista, el suceso
  queda pendiente en esa vista. Si no hay ninguna vista para él, se avisa
  a lo que los sucesos son y se tira.
- `replication_event_manager_unknown_00089dc0` hace algo con cada vista
  (`unknown_00096ed0`, sin descompilar).

### Sin comprobar

- **Romper líneas, pendiente en las cinco**, en las quince del gestor de
  cosas (hitos 416 a 418) y en `network_channel_write_packet` (hito 415).
- Al soltar un suceso el juego pregunta al montón cuánto ocupa y no hace
  nada con la respuesta: resto de la versión de pruebas.
- En este hito se pasaron las pruebas de `replication_event_manager`
  (5 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Marcar una cosa para las vistas, y quedarse con las de otra consola

Dos funciones en `networking/replication_entity_manager.c`, 952 bytes entre las dos. De 1441
funciones se pasa a 1443, el 12,81 % de las del juego (el 8,666 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_manager_mark_entity` | `0x00089710` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_adopt_entities` | `0x00089a20` | `networking/replication_entity_manager.c` |

### Lo que se sabe

- `replication_entity_manager_mark_entity` es **cómo se dice que algo de
  una cosa ha cambiado**: a cada vista se le suman los bits de lo que
  tiene que contar de esa cosa. Si la cosa está en su tercer estado en la
  vista y la vista no tenía nada pendiente de ella, la vista cuenta una
  cosa más con algo que decir (salvo que la cuenta de la cosa tenga el bit
  de esa vista).
  - De una vista se sabe ya algo: cuál es (`0xC`), su tabla (`0x14`), 20
    bytes por cosa desde `0x24` (lo pendiente y el estado) y la cuenta de
    cosas con algo pendiente (`0x5034`).
  - El juego tiene el bucle de las quince vistas desenrollado de cinco en
    cinco; en C es un bucle.
- `replication_entity_manager_adopt_entities` es lo que pasa cuando **esta
  consola se queda con las cosas que hizo otra** (por lo que hace, cuando
  pasa a mandar ella): cada cosa en uso y sin crear queda creada y se
  pregunta por ella a lo que las cosas son; si dice que no, se borra. Las
  cadenas van juntas desde su primera. Después todos los sitios en uso
  cuentan un uso más, y los números de «la siguiente» se ponen al día.
  - Al poner al día el número de la siguiente solo se guardan el sitio y
    las veces usado: los bits de en medio se pierden.
- De la tabla de lo que las cosas son se conocen ya seis funciones: 3, 11,
  13, 14, 15 y 16.

### Sin comprobar

- **Romper líneas, pendiente en las dos**, en las trece de los hitos 416
  y 417 y en `network_channel_write_packet` (hito 415).
- En este hito se pasaron las pruebas de `replication_entity_manager`
  (15 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Borrar cadenas, una vista más y las cosas que hizo otra consola

Cuatro funciones en `networking/replication_entity_manager.c`. De 1437
funciones se pasa a 1441, el 12,79 % de las del juego (el 8,634 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_manager_delete_chain` | `0x00089570` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_delete_entities_now` | `0x00089690` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_add_view` | `0x000898c0` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_create_entities_remote` | `0x00089950` | `networking/replication_entity_manager.c` |

### Lo que se sabe

- `replication_entity_manager_delete_chain` borra una cosa y las que van
  detrás en su cadena, de la última a la primera: como el borrado de una
  del hito 416, pero mirando solo si alguien tiene todavía la primera.
- `replication_entity_manager_delete_entities_now` borra ya una lista de
  cosas, después de soltarlas de su cadena.
- `replication_entity_manager_add_view` **apunta una vista nueva** (otra
  consola que entra) y le cuenta todas las cosas que ya existen: las que
  están en uso, creadas y sin borrarse. Para cada una pregunta a lo que
  las cosas son (función dieciséis de su tabla) si tiene algo que darle.
- `replication_entity_manager_create_entities_remote` es el otro lado de
  crear: **las cosas que hizo otra consola se ponen aquí con el mismo
  número que tienen allí** (sitio y veces usado), encadenadas igual, y se
  avisa de cada una a lo que las cosas son (función tres), con los tres
  datos que venían con ella.
- De la tabla de lo que las cosas son se conocen ya cuatro funciones: la
  3 (creada por otra consola), la 11 y la 13 (borrándose y borrada) y la
  16 (algo para una vista nueva).

### Sin comprobar

- **Romper líneas, pendiente en las cuatro**, en las nueve del hito 416
  y en `network_channel_write_packet` (hito 415).
- Una cadena tiene cuatro cosas como mucho: con más,
  `replication_entity_manager_delete_chain` se saldría de su lista. No se
  prueba.
- En este hito se pasaron las pruebas de `replication_entity_manager`
  (13 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — La tabla de las cosas que las consolas comparten

Nueve funciones en un archivo nuevo, `networking/replication_entity_manager.c`. De 1428
funciones se pasa a 1437, el 12,76 % de las del juego (el 8,611 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `replication_entity_manager_new` | `0x000893c0` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_reset` | `0x00089430` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_create_entities` | `0x00089470` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_delete_entity` | `0x00089500` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_delete_entity_now` | `0x00089660` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_allocate_index` | `0x00089c40` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_create_entity` | `0x00089cc0` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_get_chain` | `0x00089d20` | `networking/replication_entity_manager.c` |
| `replication_entity_manager_find_chain` | `0x00089d50` | `networking/replication_entity_manager.c` |

### Lo que se sabe

- Justo después del canal en el ejecutable hay otro módulo: **una tabla de
  1024 cosas que las consolas de una partida tienen en común**, con hasta
  quince vistas de ellas (una por cada una de las otras consolas). Por la
  forma es lo que en otros juegos de Bungie se llama el gestor de
  entidades replicadas; el nombre del archivo es una suposición.
- Cada cosa ocupa 8 bytes: unos bits (en uso, borrándose, creada, primera
  de un grupo, otra del grupo), cuántas veces se ha usado su sitio (de 16),
  una cuenta de quién la tiene todavía, y el número de la siguiente de su
  grupo.
- **El número de una cosa** es su sitio en los 10 bits bajos y las veces
  que se ha usado el sitio en los 4 altos: así un número viejo no se
  confunde con la cosa nueva que ocupa el mismo sitio.
- Lo que hacen las nueve:
  - Preparar la tabla y vaciarla (avisando a cada vista).
  - Buscar un sitio libre, desde el siguiente al último que se dio y
    dando la vuelta.
  - Crear una cosa en un sitio, avisando a cada vista.
  - **Crear hasta cuatro cosas de una vez, encadenadas**: la primera
    lleva su bit y cada una el número de la siguiente.
  - Borrar una cosa: se avisa a lo que las cosas son y a las vistas, y el
    sitio queda libre cuando nadie la tiene ya. Y borrarla ya, sin
    esperar ni avisar a las vistas.
  - Dar los números de una cadena, y buscar la cadena en la que está una
    cosa.

### Corregido

- `0x00088060` y `0x000893c0` no eran del canal, como puse en el hito 415:
  la segunda es de este módulo y la primera compara dos cosas de otro. El
  canal está entero con sus 25, y su tabla de funciones (`0x00450d40`)
  no tiene más funciones suyas que las dos del hito 413.

### Sin comprobar

- **Romper líneas, pendiente en las nueve**, y en las dos del hito 415.
- Si al crear varias cosas falta sitio para una, los sitios que ya se
  habían encontrado se quedan como en uso. Se deja igual que el juego.
- En este hito se pasaron las pruebas de `replication_entity_manager`
  (9 funciones).
- Del módulo quedan 14 en la lista, de `0x00089570` a `0x0008a110`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que un canal hace cada vez, y cómo escribe un paquete

Dos funciones en `networking/network_channel.c`, 1573 bytes entre las dos. De 1426
funciones se pasa a 1428, el 12,68 % de las del juego (el 8,584 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_update` | `0x000883c0` | `networking/network_channel.c` |
| `network_channel_write_packet` | `0x00088980` | `networking/network_channel.c` |

### Cambio en la forma de trabajar

- Desde este hito, por acuerdo con el usuario, para ir más deprisa: cada
  función entra con su prueba contra el juego, como siempre, pero **romper
  líneas se hace por tandas**, no en cada hito. Lo que se encuentre luego
  se corrige en un hito de corrección.

### Lo que se sabe

- `network_channel_update` es lo que se hace cada vez con un canal:
  - Si está pidiendo conexión, sigue intentándolo (hito 407). Si lleva en
    el cuarto estado más de lo que dice el cuarto número de lo que tiene
    en `0xC` (que no se conocía), se cierra con el motivo 7.
  - Cada pieza de la clase `0x10` dice si el canal debe cerrarse, y por
    qué. Con la primera que lo dice, se cierra.
  - Cada pieza que escribe dice si tiene algo que mandar y si es urgente.
    Con eso **el observador decide si el canal puede mandar ahora** (la
    función del hito 401) y da el tamaño y lo que la voz tiene para esa
    consola. Sin observador, se manda si alguna pieza tiene algo.
  - Se escribe y se manda el paquete, se apuntan las horas, y se atiende
    lo que le ha pasado a la pieza principal (hito 411).
- `network_channel_write_packet` escribe el paquete, y es el reverso de la
  lectura del hito 414:
  - Con número, se lo pide a la pieza principal (`unknown_00096510`); si
    no hay, no se manda nada.
  - Cada pieza dice cuántos bits necesita como poco. Si caben todas, cada
    una escribe lo suyo, sabiendo lo que las demás no necesitan.
  - **Si el canal quería más caudal del que tiene, el paquete se rellena
    con ceros hasta el final.** Es lo que la lectura tiraba sin mirar:
    relleno para medir si la línea aguanta más.
  - Se manda (`unknown_000931a0`) con la voz, y se avisa del tamaño a la
    pieza principal (`unknown_00096810`) y al observador (hito 412).
- Con esto se conoce la tabla de una pieza entera salvo la primera
  función: ¿cerrar?, ¿tienes algo?, ¿cuántos bits?, escribe, lee, y los
  dos avisos.

### Corregido

- En el hito 413 quité de `ESTADO.md` dos funciones pendientes del módulo
  que no están hechas: `0x00088060` y `0x000893c0`. Vuelven a la lista.
- Las pruebas del módulo podían fallar por casualidad: una hora al azar
  que coincidiera con la dirección de una función se tomaba por ella. Ya
  no se eligen esas horas.

### Sin comprobar

- **Romper líneas, pendiente en las dos funciones.** De
  `network_channel_update` se rompieron doce líneas antes de parar: once
  notadas, y la otra (el motivo de cierre empieza en cero) ya tiene su
  caso en las pruebas.
- El relleno se escribe en `0x600` bytes de ceros: un paquete mayor se
  saldría. No se prueba, y el juego no los hace mayores.
- En este hito se pasaron las pruebas de `network_channel` (25 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Leer un paquete que llega por un canal

Una función en `networking/network_channel.c`, de 544 bytes. De 1425
funciones se pasa a 1426, el 12,66 % de las del juego (el 8,531 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_read_packet` | `0x00088750` | `networking/network_channel.c` |

### Lo que se sabe

- `network_channel_read_packet` es **lo que un canal hace con cada paquete
  que le llega**:
  - Cada pieza del canal lee lo suyo, por la sexta función de su tabla, y
    dice cómo le fue: bien, bien y el paquete se acaba aquí, o mal. Solo
    leen las piezas con el primer bit, salvo que lo que el canal tiene en
    `0x3C` tenga su segundo byte: entonces leen todas, y el canal lo
    apunta en `0x1D`.
  - Después un bit dice si hay más, y catorce bits cuántos bits son
    (`0x3000` como mucho: 1536 bytes). Se leen y **se tiran**: el juego
    final no hace nada con ellos.
  - Un paquete que se pasa de su final no vale, y uno que empieza con la
    palabra `debg` tampoco: es la marca de la versión de pruebas.
- Con un paquete bueno, el canal apunta la hora, atiende lo que le ha
  pasado a su pieza principal (hito 411), le da el número que las piezas
  han leído (`unknown_00096860`) y **reparte los mensajes que su segunda
  pieza tiene ya enteros** (`unknown_00095840` los da uno por uno, y
  `unknown_00093aa0` los lleva a quien toca). Por lo que hace, la segunda
  pieza es la que junta los mensajes que llegan en trozos.
- Al final avisa al observador por la segunda función de su tabla, que es
  la del hito 412 que no hace nada.
- El último argumento hace que el paquete solo se mire: se lee desde donde
  esté, y no se apunta ni se reparte nada.

### Comprobado rompiendo líneas

- Treinta y cuatro líneas rotas, una cada vez: las pruebas fallaron en
  treinta y tres. La que no se nota es quitar la comprobación de que el
  paquete se ha pasado justo después de leer el resto: se vuelve a
  comprobar dos líneas más abajo, así que no cambia nada.

### Sin comprobar

- Las piezas solo pueden contestar del 0 al 3: con otro número el juego
  salta por una tabla de cuatro sitios. No se prueba.
- En este hito se pasaron las pruebas de `network_channel` (23 funciones).
- Del módulo quedan dos: `0x000883c0` y `0x00088980`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — La pieza que el canal lleva dentro

Dos funciones en `networking/network_channel.c`, 104 bytes entre las dos. De 1423
funciones se pasa a 1425, el 12,65 % de las del juego (el 8,521 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_unknown18_write` | `0x00089340` | `networking/network_channel.c` |
| `network_channel_unknown18_read` | `0x00089390` | `networking/network_channel.c` |

### Lo que se sabe

- La tercera pieza de un canal del juego, la que lleva dentro (en `0x18`,
  hito 408), es muy poca cosa: **un bit en cada paquete**.
  - `network_channel_unknown18_write` escribe su byte de `0x4` como un
    bit.
  - `network_channel_unknown18_read` lee un bit y dice que el paquete no
    vale si no es su byte de `0x5`.
- Las dos están en una tabla de funciones (`0x00450d50`), que es como el
  canal las llama al recorrer sus piezas (hito 410).

### Corregido

- En el hito 412 escribí que la tabla del observador tiene siete
  funciones. Son **seis** (y la quinta no hace nada): la séptima
  dirección que hay después, la de leer un paquete sin canal (hito 405),
  es ya la tabla de lo que manda esos mensajes.

### Comprobado rompiendo líneas

- Cuatro líneas rotas, una cada vez: las pruebas fallaron en las cuatro.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_channel` (22 funciones).
- Del módulo quedan las tres grandes: `0x000883c0`, `0x00088750` y
  `0x00088980` (escribir y leer un paquete).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Dos funciones del observador que no estaban en la lista

Dos funciones en `networking/network_observer.c`, 199 bytes entre las dos. De 1421
funciones se pasa a 1423, el 12,63 % de las del juego (el 8,517 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_sent` | `0x00076670` | `networking/network_observer.c` |
| `network_observer_unknown_00076640` | `0x00076640` | `networking/network_observer.c` |

### Corregido

- **En el hito 401 dije que el observador estaba entero, y no lo estaba.**
  Le faltaban dos funciones que no salen en la lista de Ghidra, porque
  nada las llama por su dirección: solo están en la tabla de funciones del
  observador (`0x00450e10`, la del hito 411), y Ghidra no las vio. Las
  encontré al mirar esa tabla. Con ellas son 74.
- Queda dicho en `ESTADO.md`: «entero» solo vale lo que valga la lista. Un
  módulo con tablas de funciones puede tener más.

### Lo que se sabe

- La tabla del observador tiene siete funciones, y es por donde un canal
  del juego le habla: mandé algo, llegó algo, me contestaron o no, ¿puedo
  mandar?, y ha llegado un paquete sin canal.
- `network_observer_sent` es la primera: **un canal del juego ha mandado
  algo de un tamaño**. El observador lo suma a lo que lleva mandado (la
  cuenta de `0x4E30`, de donde el hito 400 sacaba cuánto se manda de
  verdad) y, si el canal tiene caudal, a los bytes de su intervalo (de
  donde salen los bits por segundo del hito 380).
- `network_observer_unknown_00076640` es la segunda: busca el canal y no
  hace nada con él. Es lo que queda de algo que hacía la versión de
  pruebas.

### Comprobado rompiendo líneas

- Nueve líneas rotas, una cada vez: las pruebas fallaron en las nueve.

### Sin comprobar

- Si ningún canal del observador es el del juego, `network_observer_sent`
  mira un byte de antes del primer canal; se deja igual.
- En este hito se pasaron las pruebas de `network_observer` (74 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que le pasa a la pieza principal de un canal

Una función en `networking/network_channel.c`, de 533 bytes. De 1420
funciones se pasa a 1421, el 12,61 % de las del juego (el 8,517 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_update_unknown10` | `0x00088db0` | `networking/network_channel.c` |

### Lo que se sabe

- `network_channel_update_unknown10` es lo que un canal hace cada vez con
  su pieza de la primera tabla (`unknown_004d87d8`): la deja trabajar y
  luego **atiende, una por una, las cosas que le han pasado**
  (`unknown_000965e0` las va dando, con una clase del 1 al 4 y tres
  números).
  - Clases 1 y 4: ha llegado algo. El canal apunta la hora, pasa del
    cuarto estado al quinto si estaba esperando (**así es como un canal
    queda conectado del todo**) y avisa al observador.
  - Clase 4: se avisa a las piezas de la clase 4 del canal.
  - Clases 1, 2 y 3: se avisa a las piezas de la clase 8, y al observador,
    de si algo que se mandó fue contestado (1 y 2) o no (3), y de si llegó
    a tiempo.
- **Aquí se cierra el círculo con el observador.** El canal le avisa por
  dos funciones de una tabla que el observador tiene al principio, y son
  dos que ya estaban descompiladas sin saber quién las llamaba: la del
  hito 377 («algo de un tamaño ha pasado por un canal») y la del hito 372
  («a un canal se le dice si algo fue así o no»). Está comprobado en el
  ejecutable: la tabla está en `0x00450e10` y las tiene en tercer y
  cuarto lugar; en el sexto está la del hito 401, la que dice si un canal
  puede mandar. El control del caudal de los hitos 377 a 399 se alimenta
  de aquí: de cada paquete contestado o perdido.
- Por lo que hace, la pieza de la primera tabla es **lo que lleva la
  cuenta de los paquetes mandados y de sus respuestas**.

### Comprobado rompiendo líneas

- Trece líneas rotas, una cada vez: las pruebas fallaron en las trece.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_channel` (20 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Recorrer las piezas de un canal del juego

Dos funciones en `networking/network_channel.c`, 338 bytes entre las dos. De 1418
funciones se pasa a 1420, el 12,61 % de las del juego (el 8,499 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_parts_next` | `0x000891e0` | `networking/network_channel.c` |
| `network_channel_unknown3c_new` | `0x000892f0` | `networking/network_channel.c` |

### Lo que se sabe

- `network_channel_parts_next` **recorre las piezas de un canal**, una por
  llamada: primero las suyas (las del hito 408, hasta tres) y luego las de
  lo que el canal tiene en `0x3C`. Se le puede pedir solo las de cierta
  clase: cada pieza lleva unos bits, y vale la que tiene todos los que se
  piden. Es como las funciones grandes del módulo (las que escriben y leen
  un paquete, que faltan) hablan con cada pieza sin saber cuáles hay.
- Así se sabe por fin **qué es lo que un canal tiene en `0x3C`**, que el
  observador miraba desde el hito 398 sin entenderlo: un segundo juego de
  piezas, de hasta cuatro, que alguien de fuera le cuelga al canal, con
  una función a la que se avisa cuando el canal se cierra (hito 406) y
  dos bytes. `network_channel_unknown3c_new` lo prepara.

### Comprobado rompiendo líneas

- Dieciséis líneas rotas, una cada vez: las pruebas fallaron en quince. La
  otra no cambia nada: un índice que se pasa del último número posible
  acaba igual en la segunda lista.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_channel` (19 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las dos piezas que toma un canal del juego

Dos funciones en `networking/network_channel.c`, 195 bytes entre las dos. De 1416
funciones se pasa a 1418, el 12,59 % de las del juego (el 8,488 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_take_unknown10` | `0x000820f0` | `networking/network_channel.c` |
| `network_channel_take_unknown14` | `0x00082150` | `networking/network_channel.c` |

### Lo que se sabe

- Son las que el hito 408 llamaba para dar a un canal sus piezas: cada una
  busca **la primera libre** de una de las dos tablas (`unknown_004d87d8`,
  de `0x97C` bytes cada pieza, y `unknown_004d87dc`, de `0x2850`), la pone
  a cero con su función, apunta de quién es y la marca como ocupada.
- Hay tantas piezas de cada tabla como canales: las tres tablas comparten
  la misma cuenta (`unknown_004d87d0`).
- A las de la segunda tabla se les da además un sitio dentro de lo que
  comparten todos los canales (`0x588` bytes más allá de
  `unknown_00528000`).

### Comprobado rompiendo líneas

- Ocho líneas rotas, una cada vez: las pruebas fallaron en las ocho.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_channel` (17 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Tomar un canal del juego y empezar a conectar

Tres funciones en `networking/network_channel.c`, 700 bytes entre todas. De 1413
funciones se pasa a 1416, el 12,57 % de las del juego (el 8,481 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_allocate` | `0x00082060` | `networking/network_channel.c` |
| `network_channel_new` | `0x00088110` | `networking/network_channel.c` |
| `network_channel_connect` | `0x00088220` | `networking/network_channel.c` |

### Lo que se sabe

- Son las dos primeras cosas que le pasan a un canal del juego, y las dos
  las pedía el observador sin saber qué eran (hitos 387 y 392).
- `network_channel_allocate` (antes `unknown_00082060`) busca **el primer
  canal sin usar** y lo prepara. Una sorpresa: **el nombre que se le da no
  se mira**. El observador componía con cuidado un texto con la dirección
  de internet y la de la tarjeta de red de la consola (hito 392), y aquí
  se tira: es otro resto de la versión de pruebas.
- `network_channel_new` prepara el canal: sus bits, su número y las cuatro
  cosas que comparten todos los canales (de qué son, por dónde mandan los
  mensajes sin canal —que es la pasarela del hito 405—, una tercera cosa y
  sus plazos de reintento). Según tres de sus bits, el canal toma hasta
  **tres piezas**: una de cada una de las dos tablas que ya se conocían
  (`unknown_004d87d8` y `unknown_004d87dc`) y una que lleva dentro. El
  número `0x38` con que el observador pedía un canal son esos tres bits.
- `network_channel_connect` (antes `unknown_00088220`) empieza a conectar
  con una dirección: tercer estado, un **número nuevo para esta conexión**
  (el siguiente de un contador que comparten todos los canales) y, si se
  le pide, el primer intento (hito 407). Ese número es el que viaja en la
  petición y el que el hito 397 comparaba para saber si la otra consola
  había vuelto a empezar.

### Comprobado rompiendo líneas

- Diecinueve líneas rotas, una cada vez: las pruebas fallaron en las
  diecinueve.

### Sin comprobar

- Los tamaños de tres de las cuatro cosas que comparten los canales
  (`unknown_00528000`, `unknown_00528b28`, `unknown_00529188`) salen de la
  distancia entre sus direcciones; solo se usan sus direcciones.
- En este hito se pasaron las pruebas de `network_channel` (15 funciones) y
  las de `network_observer`, que usan los nombres nuevos.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un canal del juego pide conectar, y le contestan

Tres funciones en `networking/network_channel.c`, 387 bytes entre todas. De 1410
funciones se pasa a 1413, el 12,54 % de las del juego (el 8,458 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_update_connecting` | `0x000890b0` | `networking/network_channel.c` |
| `network_channel_accept` | `0x00088360` | `networking/network_channel.c` |
| `network_channel_send_established` | `0x00089180` | `networking/network_channel.c` |

### Lo que se sabe

- Son **las dos puntas de una conexión**, vistas desde el canal del juego.
  Con ellas se puede seguir el saludo entero:
  1. `network_channel_update_connecting`: el canal que quiere conectar
     manda cada cierto tiempo **una petición** (mensaje de la cuarta
     clase, con su número y sus bits), un número limitado de veces. Si
     pasa el plazo sin respuesta, se cierra con el motivo 4.
  2. La otra consola recibe la petición: es el hito 397. Si la acepta,
     llama a `network_channel_accept` (antes `unknown_00088360`), que
     guarda el número del otro, pone el canal en marcha y...
  3. ...contesta con `network_channel_send_established`: **un mensaje de la
     sexta clase** con los dos números, el suyo y el del otro.
- Ya se conocen cuatro de las 45 clases de mensaje: 4, pedir conexión; 5,
  negarla (hito 397); 6, conexión hecha; 7, conexión cerrada (hito 406).
- Un canal con una de las cosas de `unknown_004d87dc` manda el mensaje de
  conexión hecha por ella, y no por el camino de los mensajes sin canal.

### Comprobado rompiendo líneas

- Dieciséis líneas rotas, una cada vez: las pruebas fallaron en las
  dieciséis.

### Sin comprobar

- `network_channel_update_connecting` no cuenta el intento que hace: el
  número de intentos (`0x90`) lo sube otra función.
- En este hito se pasaron las pruebas de `network_channel` (12 funciones) y
  las de `network_observer`, que usan un nombre nuevo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Los canales del juego: cerrar uno, soltarlo, y lo que el observador les pregunta

Nueve funciones en un archivo nuevo, `networking/network_channel.c`, 650 bytes entre todas. De 1401
funciones se pasa a 1410, el 12,52 % de las del juego (el 8,445 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_channel_flags_valid` | `0x000880b0` | `networking/network_channel.c` |
| `network_channel_close` | `0x00088650` | `networking/network_channel.c` |
| `network_channel_release` | `0x000886e0` | `networking/network_channel.c` |
| `network_channel_reset_times` | `0x00088d20` | `networking/network_channel.c` |
| `network_channel_set_time` | `0x00088d70` | `networking/network_channel.c` |
| `network_channel_unknown_00088fd0` | `0x00088fd0` | `networking/network_channel.c` |
| `network_channel_unknown_00089000` | `0x00089000` | `networking/network_channel.c` |
| `network_channel_unknown_00089030` | `0x00089030` | `networking/network_channel.c` |
| `network_channel_unknown_00089070` | `0x00089070` | `networking/network_channel.c` |

### Lo que se sabe

- Empieza el módulo de **los canales del juego**, los que el observador
  maneja desde fuera (hitos 371 a 401). Son nueve de las funciones que el
  observador llamaba sin que se supiera qué hacían.
- `network_channel_close` era `unknown_00088650`, «lo que se le dice a un
  canal del juego, con un número». Es **cerrar el canal, con un motivo**:
  - Si estaba conectado, y no es la otra consola la que cierra (motivo 6),
    se le avisa: un mensaje de la séptima clase, por el camino de los
    mensajes sin canal (hito 404), con el motivo.
  - El canal vuelve a su segundo estado y guarda el motivo en `0x58`: lo
    que el hito 386 leía para decidir si reintentar.
  - Los números que el observador pasaba (3, 6, `0xD`, `0xE`, `0xF`,
    `0x10`, `0x11`) son, pues, **motivos de cierre**.
- `network_channel_release` (antes `unknown_000886e0`) deja un canal sin
  usar: lo cierra si hace falta y suelta las dos cosas que tenía.
- `network_channel_flags_valid` (antes `unknown_000880b0`) dice si los
  bits de una petición de conexión (hito 397) son una combinación que
  vale.
- `network_channel_reset_times` y `network_channel_set_time`: un canal
  lleva **seis horas**, cada una por el reloj y por la cuenta de imágenes.
- Las cuatro que quedan dan números de lo que el canal tiene suyo; el
  observador las usa para saber cuánto hay mandado sin contestar y cuánto
  espera para salir (hitos 370 y 401).

### Comprobado rompiendo líneas

- Veinticuatro líneas rotas, una cada vez: las pruebas fallaron en las
  veinticuatro.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_channel` (9 funciones) y
  las de `network_observer` (72), que las sustituyen por sus nombres
  nuevos.
- Del módulo quedan unas quince funciones, de `0x00088060` a `0x000893c0`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un paquete de mensajes se lee: el módulo, entero

Una función en `networking/network_message_gateway.c`, de 368 bytes. De 1400
funciones se pasa a 1401, el 12,44 % de las del juego (el 8,423 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_message_gateway_receive` | `0x0007afd0` | `networking/network_message_gateway.c` |

### Lo que se sabe

- **`network_message_gateway.c` está entero**: las cuatro funciones del
  tramo `0x0007afd0` a `0x0007b3d2` y las dos de la cabecera de un mensaje
  que el juego tiene en otro sitio (hitos 394 y 403 a 405).
- `network_message_gateway_receive` es **el camino de vuelta del hito
  404**: lee un paquete que ha llegado de una dirección.
  - Si el paquete empieza por la palabra de una versión de pruebas, no
    vale (hito 402).
  - Mientras un bit diga que hay otro mensaje: lee su clase y su tamaño
    (hito 403), deja que la función de su clase lo lea en un hueco de
    hasta `0x10000` bytes puesto a cero, y **se lo da a quien le toque**
    (`unknown_000938e0`, la que reparte por clases; sin descompilar).
  - Un mensaje malo corta la lectura, pero los anteriores ya se han
    repartido.
- En el juego tiene escritas dentro dos funciones del flujo de bits
  (empezar a leer y leer un bit); aquí se las llama.

### Comprobado rompiendo líneas

- Diez líneas rotas, una cada vez: las pruebas fallaron en las diez.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_message_gateway` (6
  funciones).
- En las pruebas, leer la cabecera de cada mensaje y leer el mensaje están
  sustituidos por algo que solo hace avanzar el flujo; la cabecera tiene
  sus propias pruebas (hito 403).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un mensaje entra en el paquete

Una función en `networking/network_message_gateway.c`, de 478 bytes. De 1399
funciones se pasa a 1400, el 12,43 % de las del juego (el 8,410 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_message_gateway_send` | `0x0007b140` | `networking/network_message_gateway.c` |

### Lo que se sabe

- `network_message_gateway_send` era `unknown_0007b140`, la que el
  observador y las sesiones llaman desde hace muchos hitos para **mandar
  un mensaje a una dirección sin pasar por un canal**. No lo manda en el
  momento: lo mete en el paquete pendiente.
  - Si el paquete pendiente es para otra dirección, sale antes (hito 394).
  - Si no hay paquete, empieza uno de `0x518` bytes para esa dirección.
  - El mensaje entra detrás de **un bit a 1, que dice «hay otro mensaje»**:
    su clase y su tamaño (hito 403) y lo que escribe la función de su
    clase. El bit que el hito 394 añadía al cerrar el paquete es el
    contrario: «no hay más».
  - Si no cabe, **se deshace** (hito 402: para esto guardaba el flujo
    posiciones a las que volver), se manda el paquete como estaba y el
    mensaje va en uno nuevo. Si no cabe ni en uno nuevo, no se manda, y la
    función lo dice.
- Devuelve si el mensaje entró. Quienes la llaman no lo miran.
- En el juego tiene escritas dentro varias funciones del flujo de bits
  (empezar a escribir, guardar una posición, escribir un bit); aquí se
  llama a las que ya están descompiladas.

### El arnés

- La función de cada clase de mensaje se sustituye en las pruebas, y tiene
  que hacer avanzar el flujo como si escribiera. Lo que sustituye a una
  función llamada por un puntero no recibía la memoria; las pruebas la
  toman del arnés (`t.harness.mu`).

### Comprobado rompiendo líneas

- Diecisiete líneas rotas, una cada vez: las pruebas fallaron en quince.
  - Una faltaba en las pruebas (que el paquete nuevo no herede un número
    del anterior) y ahora falla.
  - La otra no cambia nada: mandar el paquete pendiente cuando no lo hay
    no hace nada.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_message_gateway` (5
  funciones) y las de `network_observer` y `network_life_cycle`, que la
  sustituyen por su nombre nuevo.
- Del módulo queda recibir un paquete (`0x0007afd0`).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Con qué empieza un mensaje, y un texto del flujo de bits

Tres funciones, 315 bytes entre todas: dos en `networking/network_message_gateway.c` y una en
`networking/bitstream.c`. De 1396 funciones se pasa a 1399, el 12,42 % de las del juego
(el 8,394 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_message_header_write` | `0x000937e0` | `networking/network_message_gateway.c` |
| `network_message_header_read` | `0x00093860` | `networking/network_message_gateway.c` |
| `bitstream_read_wide_string` | `0x00194fa0` | `networking/bitstream.c` |

### Lo que se sabe

- **Cada mensaje de la red empieza igual**: su clase, en 8 bits, y su
  tamaño, en 16.
  - `network_message_header_write` escribe eso.
  - `network_message_header_read` lo lee y dice si vale: la clase tiene
    que existir y el tamaño estar entre el mínimo y el máximo de esa
    clase. Hay **45 clases de mensaje** (`0x2D`), y de cada una se guarda
    en `0x20` bytes si existe, sus dos tamaños y las dos funciones que la
    escriben y la leen.
  - El juego las tiene lejos de las demás del módulo (`0x00093xxx`), junto
    a la que reparte cada mensaje recibido a quien le toca (`0x000938e0`,
    sin descompilar): ahí se ve que la petición de conexión del hito 397
    es una de esas clases.
- `bitstream_read_wide_string` lee un texto de caracteres de 16 bits. Si no
  termina en el sitio que se le da, lo corta y marca el flujo como malo.
  Así se sabe que el byte del flujo que el hito 402 tomaba por «empieza por
  `debg`» es más general: **algo ha ido mal al leer**. Lo llamo `error`.

### Comprobado rompiendo líneas

- Trece líneas rotas, una cada vez: las pruebas fallaron en once.
  - Una faltaba en las pruebas (dar por buena la clase número 45, que no
    existe) y ahora falla.
  - La otra no cambia nada: se mira dos veces si el flujo se ha pasado, y
    quitar la primera da lo mismo.

### Sin comprobar

- En este hito se pasaron las pruebas de `bitstream` (11 funciones) y de
  `network_message_gateway` (4).
- Lo que queda del flujo de bits hace cuentas con logaritmos y ángulos del
  procesador antiguo (comprimir reales y direcciones): no va con el arnés
  de ahora.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Más del flujo de bits: empezar a leer, un bit y volver atrás

Cinco funciones en `networking/bitstream.c`, 354 bytes entre todas. De 1391
funciones se pasa a 1396, el 12,39 % de las del juego (el 8,384 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitstream_overflowed` | `0x001946f0` | `networking/bitstream.c` |
| `bitstream_begin_reading` | `0x001947a0` | `networking/bitstream.c` |
| `bitstream_write_bool` | `0x00194830` | `networking/bitstream.c` |
| `bitstream_write_integer_checked` | `0x001947e0` | `networking/bitstream.c` |
| `bitstream_pop_position` | `0x00194710` | `networking/bitstream.c` |

### Lo que se sabe

- Vuelvo al flujo de bits del hito 10, porque lo que manda los mensajes sin
  canal (hito 394) lo usa por dentro. El flujo **es más largo de lo que se
  creía**: después de su posición guarda un byte, **hasta cuatro
  posiciones a las que volver** y dos números (`0x34` bytes en total).
  - `bitstream_begin_reading` prepara un flujo para leerlo. Mira si empieza
    por la palabra `debg`, la que escribe una versión de pruebas del
    juego, y lo apunta.
  - `bitstream_overflowed` dice si el flujo se ha pasado del final de sus
    datos... o si empezaba por `debg`: esta versión del juego **da por
    malo un paquete de una versión de pruebas**.
  - `bitstream_write_bool` escribe un bit.
  - `bitstream_write_integer_checked` escribe un número en tantos bits. Si
    no cabe, compone un texto de aviso («%u exceeds max value of %u») con
    el que no hace nada: es lo que queda de un aviso de la versión de
    pruebas.
  - `bitstream_pop_position` deja de guardar la última posición y, si se
    le dice, **vuelve a ella**: en un flujo que se está escribiendo borra
    lo que se había escrito desde allí. Es como se deshace un mensaje que
    al final no cabe en el paquete.

### La batería completa

- La del hito 400 terminó: **1390 de 1390 funciones** iguales que las del
  juego.

### Comprobado rompiendo líneas

- Veintidós líneas rotas, una cada vez: las pruebas fallaron en
  dieciocho. Las otras cuatro faltaban en las pruebas y ahora fallan: tres
  escribían justo después de los datos, donde las pruebas no miraban, y la
  cuarta era el texto de aviso, que no dejaba rastro.

### Sin comprobar

- En este hito se pasaron las pruebas de `bitstream` (10 funciones).
- La entrada de `ESTADO.md` decía que al flujo de bits le quedaban 2
  funciones. En la lista de Ghidra hay unas veinte más por esa zona
  (`0x00194870` a `0x00195ed0`); no he mirado aún cuáles son suyas.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuándo puede mandar un canal: el observador, entero

Una función en `networking/network_observer.c`, de 1369 bytes. De 1390
funciones se pasa a 1391, el 12,35 % de las del juego (el 8,372 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_game_channel_can_send` | `0x00075fc0` | `networking/network_observer.c` |

### Lo que se sabe

- **Con esta función `network_observer.c` está entero**: sus 72 funciones
  están descompiladas y comprobadas contra el juego (hitos 326, 369 a 392
  y 395 a 401). Van de `0x00075970` a `0x0007a740`, más unas pocas que
  el juego tiene en otro sitio. En ese tramo la lista de Ghidra tiene una
  entrada más, `0x000778ec`, que no es una función: es el final de
  `network_observer_channel_update_tries`.
- `network_observer_game_channel_can_send` es **para lo que sirve todo lo
  demás**: un canal del juego pregunta si puede mandar un paquete ahora, y
  de qué tamaño. Aquí se juntan el caudal de cada canal (hitos 377 a 399)
  y lo que el observador deja mandar entre todos (hito 400).
  - **Cada cuánto**: según los paquetes por segundo que le tocan (los de
    su caudal, o su parte del reparto). El tiempo no lo mide con el
    reloj sino **en imágenes**: tienen que haber pasado, desde la última
    vez que pudo mandar, las imágenes que hay entre dos paquetes. Por eso
    el canal guardaba aquel número de 64 bits del hito 386.
  - **Cuánto**: lo que dan sus bits por segundo en las imágenes que han
    pasado, menos los 45 bytes que lleva de más cada paquete; y no más de
    lo que cabe en lo que puede tener mandado sin contestar. Si no llega
    a 48 bytes, no manda.
  - Si algo corre prisa (la pregunta del hito 382 a las sesiones), manda
    sin mirar nada de eso.
  - **La voz va dentro del mismo paquete**: se le da una parte del sitio
    (un real, según lo que el canal del juego tenga en `0x3C`), escribe lo
    suyo (hito 395) y el resto es para el juego. Un paquete entero no pasa
    de `0x516` bytes.
  - De paso anota en el caudal del canal **qué se quedó corto** (los
    avisos del hito 385): si la voz no cupo entera, o si no había sitio
    por lo ya mandado. Es de ahí de donde el control del caudal sabe qué
    subir.
- Tres funciones nuevas de los canales del juego, aún sin descompilar:
  `unknown_00089030`, `unknown_00088fd0` y `unknown_00089000`. Y un real
  nuevo, `unknown_004cf5d0`.

### La batería completa

- Se lanzó entera en el hito 400, en el otro árbol de trabajo, y sigue
  corriendo al cerrar este hito. La última que terminó es la del hito 385
  (1354 de 1354).

### Comprobado rompiendo líneas

- Cuarenta y tres líneas rotas, una cada vez: las pruebas fallaron en
  treinta y ocho.
  - Una faltaba en las pruebas (que queden justo 48 bytes) y ahora falla.
  - Las otras cuatro no cambian nada: un byte que se pone cuando ya no se
    mira; dividir de otra manera un número que, si es negativo, lleva a no
    mandar igual; un tamaño de partida que se recorta al mismo tope; y
    dar un byte por otro que a esas alturas vale lo mismo.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (72 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuánto deja mandar el observador, y cómo lo reparte

Una función en `networking/network_observer.c`, de 1414 bytes. De 1389
funciones se pasa a 1390, el 12,34 % de las del juego (el 8,326 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_update_bandwidth` | `0x000779f0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_update_bandwidth` era `unknown_000779f0`, la primera de
  las tres cosas que `network_observer_update` hace cada vez (hito 373).
  Es **el otro control de la conexión**, distinto del caudal de cada canal
  (hitos 377 a 399): este lleva una sola cifra para toda la consola,
  **cuánto deja mandar entre todos los canales por los que va el juego**,
  y la reparte.
  - Cada cierto tiempo cuenta **cuántos canales van mal**: los conectados
    cuya segunda serie de últimos números (hito 372) da una media alta.
    Si son una parte suficiente, es una vez mala más; si no, una buena.
  - Tras varias veces malas seguidas, **baja** lo que deja mandar: no más
    de lo que de verdad ha mandado últimamente, menos un margen. Tras
    varias buenas, lo **sube** un paso, siempre que quede por debajo del
    tope. Es el mismo esquema que el del caudal de un canal: bajar deprisa
    y subir despacio.
  - Lo cuenta en unidades de 1024 bits por segundo. Son los tres números
    de `0x4E20`, `0x4E24` y `0x4E28` que el hito 376 ponía a su valor de
    partida.
  - Después **lo reparte a partes iguales** entre los canales conectados
    cuyo canal del juego tiene algo en `0x3C`: a cada uno sus bits por
    segundo (`0x480`), sus paquetes por segundo (`0x484`, completos o a la
    mitad según lo que tenga en `0x3C`) y cuánto puede mandar de golpe
    (`0x488`), que sale de dos números que el canal del juego tiene para
    él. Los demás canales se quedan sin parte.
- En el juego tiene escritas dentro, tres veces, las funciones del hito
  388 que pasan de bits a paquetes por segundo; aquí se las llama.

### Comprobado rompiendo líneas

- Treinta y dos líneas rotas, una cada vez: las pruebas fallaron en
  treinta y una. La otra faltaba en las pruebas (que no haya nada que
  repartir) y ahora falla.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (71 funciones).
- Del observador solo queda una función: `0x00075fc0`, la que decide
  cuándo le toca mandar a un canal.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El caudal de todos los canales, cada vez

Una función en `networking/network_observer.c`, de 923 bytes. De 1388
funciones se pasa a 1389, el 12,33 % de las del juego (el 8,278 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_update_flow` | `0x00078ac0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_update_flow` era `unknown_00078ac0`, una de las tres
  cosas que `network_observer_update` hace cada vez. Es **la que mueve
  todo el control del caudal**: con ella, todo lo de los hitos 377 a 398
  queda enlazado desde arriba, sin ninguna pieza suelta.
  - Primero decide **qué canales tienen caudal**: lo pierde el que ya no
    está conectado (o cuyo canal del juego ha cambiado en lo que tiene en
    `0x3C`), y lo estrena el que acaba de conectarse (hito 398). El byte
    de `0x4E00` del observador lo apaga todo.
  - De cada canal con caudal apunta en un byte si **alguien lo necesita
    ahora**: una sesión que lo usa (por su segunda función; con esta se
    conocen ya las seis de esa tabla) o la voz, si tiene algo para esa
    consola.
  - Si un canal que estaba probando a mandar más se retrasa demasiado,
    le baja el caudal sin esperar al final del intervalo.
  - Y cada cierto tiempo hace la ronda: calcula cuántos canales pueden
    probar a la vez (una parte de los que tienen caudal), **ordena los
    canales** por el real del hito 388 —los que menos tienen, primero— y
    con cada uno hace lo del hito 389. Así se ve para qué era ese real:
    decide a quién le toca antes subir.
- En el juego está escrita dentro la función que da la sesión de la voz,
  con las dos que esta llama; aquí se las llama.

### Comprobado rompiendo líneas

- Veinticuatro líneas rotas, una cada vez: las pruebas fallaron en las
  veinticuatro.

### Sin comprobar

- No mira que un canal en uso tenga canal del juego; en las pruebas
  siempre lo tiene.
- En este hito se pasaron las pruebas de `network_observer` (70 funciones).
- Del observador quedan dos funciones grandes: `0x00075fc0` (cuándo le
  toca mandar a un canal) y `0x000779f0`.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El caudal de un canal empieza

Dos funciones en `networking/network_observer.c`, 1049 bytes entre las dos. De 1386
funciones se pasa a 1388, el 12,32 % de las del juego (el 8,247 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_start_flow` | `0x00078e60` | `networking/network_observer.c` |
| `network_observer_compare_channels` | `0x00078a90` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_start_flow` es **donde nace el caudal de un
  canal**: lo que pone el byte de `0x48C` que todas las del caudal miran
  («el canal tiene caudal»).
  - Empieza con unos bits por segundo fijos (`0x158`), salvo que con eso
    se pase de **lo que hay para todos**: la mitad del número de `0x4E04`
    del observador (o tres cuartos, según un byte), con un tope. Entonces
    se reparte: el canal nuevo se lleva su parte y **a los demás se les
    baja el caudal en proporción**.
  - Durante un rato después de una hora del observador (`0x4F2C`) hay un
    mínimo garantizado por canal.
  - Borra todo lo que el canal llevaba de su caudal (los `0x94` bytes
    desde `0x48C`), apunta en tres bytes qué tiene su canal del juego en
    `0x3C`, y toma como primer valor de su media (`0x4B0`, hito 389) el
    número que el canal del juego tiene para él.
  - Con esto queda claro que `0x4E04` es **lo que la conexión de esta
    consola da de sí**, en bits por segundo: lo mismo que el hito 385
    comparaba para decidir a cuántas consolas llega la voz.
- `network_observer_compare_channels` compara dos canales por un real de
  cada uno: es con lo que se ordenan (`qsort_4byte`) en la función que
  falta, `0x00078ac0`.
- Dentro del reparto el juego da los dos bytes **del canal nuevo**, no los
  de cada canal, al calcular los paquetes por segundo de los otros. Se
  deja igual.

### Corregido

- **Un descuido mío del hito 391.** Al dar nombre a
  `network_observer_update_voice_mode` no cambié la llamada que
  `network_observer_update` le hace: seguía con el nombre viejo
  (`unknown_0007a4a0`), y su prueba también. Las pruebas pasaban porque el
  arnés resuelve por su dirección cualquier nombre de esa forma. Ya están
  los dos con el nombre nuevo.
- De paso busqué más nombres viejos así en todo el código y las pruebas:
  había otros siete, todos en comentarios. Corregidos.

### Comprobado rompiendo líneas

- Veinticinco líneas rotas, una cada vez: las pruebas fallaron en
  veintitrés. Las otras dos no cambian nada: dividir de otra manera lo
  que hay para todos solo se nota si es negativo, y entonces no se usa.

### Sin comprobar

- Divide entre números del canal del juego sin mirar que el canal tenga
  uno; en las pruebas siempre lo tiene.
- En este hito se pasaron las pruebas de `network_observer` (69 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Una consola pide que se hable con ella

Una función en `networking/network_observer.c`, de 673 bytes. De 1385
funciones se pasa a 1386, el 12,30 % de las del juego (el 8,213 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_handle_connect_request` | `0x000785d0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_handle_connect_request` es **el otro lado del hito
  387**: allí un canal intentaba conectar; aquí llega la petición de otra
  consola, con un número y unos bits.
- Puede decir que no, con un mensaje de la quinta clase mandado a la
  dirección (por lo que manda los mensajes sin canal, hito 394), que lleva
  el número que se dio y **el motivo**:
  - 4: las bibliotecas de la consola no están hablando con esa dirección.
  - 1: es la dirección de esta misma consola, o los bits no valen.
  - 3: no hay ningún canal, usado por alguna sesión, para esa consola.
- Si hay canal: si su canal del juego ya estaba conectado con otra
  dirección, o con esta pero con otro número (la consola ha vuelto a
  empezar), se le corta (`unknown_00088650`, con un 6). Y si no está
  conectado, el canal toma esa dirección, con cómo se conoce a la sesión y
  su clave, el canal del juego empieza a conectar y el canal del
  observador pasa al **sexto estado**: el mismo al que llega por su
  cuenta el que intenta conectar.
- De los bits de la petición se usan seis; los dos primeros se dan la
  vuelta antes de preguntar si esta consola puede con ellos
  (`unknown_000880b0`, nueva): lo que para una es «yo» para la otra es
  «tú».

### Comprobado rompiendo líneas

- Veintitrés líneas rotas, una cada vez: las pruebas fallaron en las
  veintitrés.

### Sin comprobar

- En el juego, cuando el canal del juego hablaba con otra dirección, se
  preguntan dos cosas de esa dirección cuyo resultado no se usa. Se dejan.
- No mira que el canal encontrado tenga canal del juego; en las pruebas
  siempre lo tiene.
- En este hito se pasaron las pruebas de `network_observer` (67 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El final de cada intervalo del caudal

Una función en `networking/network_observer.c`, de 541 bytes. De 1384
funciones se pasa a 1385, el 12,29 % de las del juego (el 8,191 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_update_interval` | `0x00079260` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_update_interval` es **lo que el observador hace con el
  intervalo del caudal cada vez**: la que cierra el ciclo de los hitos 385
  a 391. Solo actúa cuando el intervalo ha durado lo suyo, o cuando algo
  pidió hacerlo ya (el byte de `0x4F3C`, que ponen las funciones que
  cambian un caudal).
  - Si en el intervalo **bajó un caudal del todo**, los canales que
    estaban probando a mandar más vuelven al caudal que tenían guardado:
    la prueba ya no vale.
  - Si no, y el intervalo ha durado bastante, cada canal **guarda los
    bits por segundo que ha hecho** (los dos números de `0x510` y
    `0x514`, los que el hito 389 comparaba para ver «quién tiene más»), y
    el observador recibe el resumen.
  - Luego empieza un intervalo nuevo (hito 385).
- Con el resumen se entiende por fin `network_observer_unknown_00077f90`
  (hito 385): los tres números que recibía son **cuántos canales hay,
  cuántos bits por segundo hacen entre todos, y cuántos de ellos van
  bien**. De ahí salen el máximo de bits por segundo que se ha visto
  (`0x4E08`), su media (`0x4E0C`) y las dos cuentas de intervalos buenos
  y malos. Y `0x4E04`, lo que el hito 385 comparaba con las tablas de la
  voz, es de la misma familia: lo que la conexión da de sí.

### Comprobado rompiendo líneas

- Dieciocho líneas rotas, una cada vez: las pruebas fallaron en
  diecisiete. La otra faltaba en las pruebas (ningún canal que cuente) y
  ahora falla.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (66 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — La voz que va por un canal

Dos funciones en `networking/network_observer.c`, 269 bytes entre las dos. De 1382
funciones se pasa a 1384, el 12,29 % de las del juego (el 8,172 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_voice_has_packet` | `0x00076520` | `networking/network_observer.c` |
| `network_observer_voice_build_packet` | `0x000765c0` | `networking/network_observer.c` |

### Lo que se sabe

- Son las dos por las que **la voz sale por un canal**:
  - `network_observer_voice_has_packet` dice si la voz tiene algo que
    mandar a la consola de un canal, y cuánto ocupa.
  - `network_observer_voice_build_packet` lo escribe
    (`game_voice_build_packet`).
- Las dos preguntan a la sesión con la que va la voz **qué consola es la
  de ese canal**, por la sexta función de lo que la sesión tiene en el
  observador. Con esta son cinco las funciones conocidas de esa tabla, y
  tiene al menos seis.
- En el juego tienen escrita dentro la función que da la sesión de la voz
  (`game_voice_get_session`); aquí se la llama.

### Comprobado rompiendo líneas

- Nueve líneas rotas, una cada vez: las pruebas fallaron en ocho. La otra
  faltaba en las pruebas (usar una sesión que quedó a medio dar cuando no
  hay ninguna) y ahora falla.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (65 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El paquete de mensajes que no van por un canal

Dos funciones en un archivo nuevo, `networking/network_message_gateway.c`, 158 bytes entre las dos. De 1380
funciones se pasa a 1382, el 12,27 % de las del juego (el 8,163 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_message_gateway_flush` | `0x0007b330` | `networking/network_message_gateway.c` |
| `network_message_gateway_flush_address` | `0x0007b390` | `networking/network_message_gateway.c` |

### Lo que se sabe

- Empieza otro módulo, el que está justo antes del de los intentos del
  hito 393 (`0x0007afd0` a `0x0007b3d2`): **lo que manda los mensajes que
  no van por un canal**, los que el observador y las sesiones mandan
  directamente a una dirección (`unknown_0007b140`, que aún falta). Es la
  «cosa de la red que el observador recibe» y guarda en `0x8`.
- Junta en **un solo paquete** los mensajes para una misma dirección:
  tiene si hay un paquete sin mandar (`0x14`), para qué dirección
  (`0x618`) y lo que lo va escribiendo bit a bit (`0x62C`, un
  `bitstream` del hito 10).
  - `network_message_gateway_flush` **manda el paquete pendiente**:
    escribe un bit más para cerrarlo, calcula cuántos bytes ocupa
    (redondeando al múltiplo que el `bitstream` pide) y lo manda
    (`unknown_00093100`, nueva).
  - `network_message_gateway_flush_address` lo manda solo si es para una
    dirección dada. El observador la llama cuando un canal se queda sin
    dirección (hito 384): antes de soltarla, sale lo que hubiera
    pendiente para ella.
- El nombre del archivo es una suposición: es el que Halo 3 da a lo que
  hace este trabajo.

### Comprobado rompiendo líneas

- Trece líneas rotas, una cada vez: las pruebas fallaron en las trece.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_message_gateway` (2
  funciones) y la del observador que la usa.
- Quedan las dos grandes del módulo: recibir un paquete (`0x0007afd0`) y
  añadir un mensaje (`0x0007b140`).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Los intentos de medir la conexión con otra consola

Ocho funciones en un archivo nuevo, `networking/transport_qos.c`, 1109 bytes entre todas. De 1372
funciones se pasa a 1380, el 12,25 % de las del juego (el 8,158 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `transport_qos_initialize` | `0x0007b3e0` | `networking/transport_qos.c` |
| `transport_qos_dispose` | `0x0007b430` | `networking/transport_qos.c` |
| `transport_qos_new` | `0x0007b4c0` | `networking/transport_qos.c` |
| `transport_qos_service_new` | `0x0007b5e0` | `networking/transport_qos.c` |
| `transport_qos_delete` | `0x0007b650` | `networking/transport_qos.c` |
| `transport_qos_is_complete` | `0x0007b6c0` | `networking/transport_qos.c` |
| `transport_qos_get_state` | `0x0007b720` | `networking/transport_qos.c` |
| `transport_qos_get_result` | `0x0007b7b0` | `networking/transport_qos.c` |

### Lo que se sabe

- Es **un módulo entero y pequeño**, el que el observador usaba desde el
  hito 375 sin saber qué era («algo de la red que se pide sobre una
  consola»). Son los **intentos de medir cómo es la conexión con otras
  consolas**: cuánto tarda una respuesta y cuántos bits por segundo pasan
  en cada sentido. Lo hacen las bibliotecas de la consola
  (`XNetQosLookup`); el juego solo lleva la cuenta de los intentos.
- El nombre del archivo sale del que el juego da a su tabla: `transport
  qos attempts`. Los de las funciones son míos.
- La tabla (`transport_qos_attempts`) es una de las del hito 7, de 32
  intentos como mucho; cada uno guarda lo que las bibliotecas llevan de
  él.
  - `transport_qos_new` empieza un intento sobre hasta 64 consolas; de
    cada una da cómo se conoce a la sesión, su clave y cómo se conoce a
    la consola. Hay **dos clases** de intento, con distinto número de
    pruebas y de bits por segundo (8 y `0x4000`, o ninguna y `0x1000`).
  - `transport_qos_service_new` empieza uno sobre el servicio.
  - `transport_qos_is_complete` dice si ha terminado con todas.
  - `transport_qos_get_state` dice cómo va con una consola: sin probar,
    probando, con algo, terminado sin respuesta, consola que no admite
    intentos, o terminado con respuesta.
  - `transport_qos_get_result` da lo que encontró: pruebas mandadas y
    contestadas, el tiempo de ida y vuelta menor y el mediano, los bits
    por segundo de subida y de bajada, y lo que la consola mandó.
  - `transport_qos_delete` lo suelta; `transport_qos_dispose`, todos.
- Con esto se sabe qué son los `0x20` bytes que un canal del observador
  guarda en `0x74` (hito 375): el resultado de su intento.

### Con cuidado

- La función de las bibliotecas que empieza un intento sobre el servicio
  (`0x003cd365`) no tiene nombre en la lista de Ghidra. La llamo
  `XNetQosServiceLookup` porque es la de esa biblioteca que recibe esas
  tres cosas y cuyo resultado se suelta con `XNetQosRelease`.
- `transport_qos_initialize` tiene escrita dentro, en el juego, la función
  que crea una tabla (`data_new`); aquí se la llama.

### Comprobado rompiendo líneas

- Veintinueve líneas rotas, una cada vez: las pruebas fallaron en
  veintiocho. La que queda no cambia nada: recortar a 64 consolas cuando
  ya son 64.

### Sin comprobar

- En este hito se pasaron las pruebas de `transport_qos` (8 funciones) y
  las de `network_observer` (63), que usan sus nombres nuevos.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El canal para una consola

Dos funciones en `networking/network_observer.c`, 1344 bytes entre las dos. De 1370
funciones se pasa a 1372, el 12,18 % de las del juego (el 8,121 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_acquire_channel` | `0x00076aa0` | `networking/network_observer.c` |
| `network_observer_take_game_channels` | `0x00078900` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_acquire_channel` era `unknown_00076aa0`, la que las
  sesiones llaman (desde `network_session.c`) para pedir **el canal de una
  consola**. Es donde nacen los canales del observador:
  - Si ya hay un canal para esa consola, es ese.
  - Si no, toma el primero libre; y si están los quince en uso, **echa al
    primero que ninguna sesión usa**.
  - El canal nuevo toma un canal del juego (`unknown_00082060`, nueva), al
    que da un nombre hecho con la dirección de internet de la consola y
    la de su tarjeta de red (por ejemplo `10.0.0.5!MAC=00:50:F2:01:02:03`). Si no queda
    ninguno, no hay canal.
  - Empieza en el segundo estado, sin intentos ni dirección, y con sus
    cuentas en marcha.
  - La sesión que lo pide queda apuntada como una de las que lo usan.
- Por el nombre se sabe ya qué son los `0x24` bytes que el canal guarda en
  `0x14`, «cómo se conoce a la consola»: su dirección de internet en los
  cuatro primeros y la de su tarjeta de red en `0xA`. Es la forma del
  `XNADDR` de las bibliotecas de la consola.
- `network_observer_take_game_channels` es la contraria de la del hito
  382: cada canal en uso **vuelve a tomar un canal del juego** (aquí el
  nombre es solo la dirección de internet) y se hace con él lo de cada
  vez.

### Comprobado rompiendo líneas

- Veinticuatro líneas rotas, una cada vez: las pruebas fallaron en
  veintitrés. La otra faltaba en las pruebas (dar por la misma consola a
  una que solo cambia en el último byte) y ahora falla.

### Sin comprobar

- En el juego, echar a un canal está escrito dentro de la función; aquí se
  llama a `network_observer_channel_delete`, que hace lo mismo.
- En este hito se pasaron las pruebas de `network_observer` (63 funciones)
  y las de `network_life_cycle`, que la sustituyen por su nombre nuevo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El caudal de un canal sube, y el modo de la voz

Tres funciones en `networking/network_observer.c`, 1729 bytes entre todas. De 1367
funciones se pasa a 1370, el 12,16 % de las del juego (el 8,076 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_next_packet_rate` | `0x00078210` | `networking/network_observer.c` |
| `network_observer_channel_raise_flow` | `0x00079de0` | `networking/network_observer.c` |
| `network_observer_update_voice_mode` | `0x0007a4a0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_raise_flow` (antes `unknown_00079de0`) es la
  que faltaba del control del caudal: **el canal prueba a mandar más**.
  Con ella está entero el ciclo: probar más (este hito), ver cómo va y
  quedarse o volver atrás (hitos 385 y 389), y bajar (hito 390).
  - Hay tres cosas que pueden subir: los bits por segundo (una parte de
    lo que hay, con un máximo por vez y un techo), el otro número del
    caudal (lo que ocupa un paquete; por eso se sabe ya que `0x498` son
    **bytes**) y los paquetes por segundo (el siguiente ritmo de la
    lista, `network_observer_next_packet_rate`).
  - Solo sube **una** cada vez, y solo si en el intervalo se anotó que
    hacía falta (los tres avisos del hito 385 son eso: qué se quedó
    corto).
  - Antes de subir guarda el caudal que tenía, para poder volver (hito
    388), y pone la fase a 2.
  - No prueban todos los canales a la vez: el observador cuenta cuántos
    están probando (`0x4F40`) y tiene un máximo (`0x4F44`). El observador
    es, pues, más largo de lo que se creía: pasa de `0x4F40`.
  - Si no puede subir nada, lo dice con un byte, y la fase vuelve a 0.
- `network_observer_update_voice_mode` es la segunda que une el observador
  con la voz (la primera, en el hito 385): decide **cómo va la voz entre
  las consolas** (`unknown_004c987c` de `game_voice.c`: directa, o a
  través de la consola elegida) según cómo van los caudales: si alguno
  está retenido, o se anotó algo de alguno. Usa un byte nuevo,
  `unknown_004cf73c`.

### La batería completa

- Se pasó entera en el hito 385, en el otro árbol de trabajo: **1354 de
  1354 funciones** iguales que las del juego.

### Comprobado rompiendo líneas

- Treinta y una líneas rotas, una cada vez: las pruebas fallaron en
  veintinueve. Las otras dos faltaban en las pruebas y ahora fallan:
  redondear en vez de truncar lo que suben los bits por segundo (todos
  los casos daban números enteros), y contar un ritmo de cero paquetes
  como si no lo fuera (solo se nota con el otro número negativo).

### Sin comprobar

- Divide entre los bits por segundo del canal sin mirar que no sean cero;
  las pruebas no dan ceros ahí.
- En este hito se pasaron las pruebas de `network_observer` (61 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El caudal de un canal baja

Tres funciones en `networking/network_observer.c`, 897 bytes entre todas. De 1364
funciones se pasa a 1367, el 12,13 % de las del juego (el 8,017 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_packet_rate_bits_per_second` | `0x00078150` | `networking/network_observer.c` |
| `network_observer_channel_lower_flow` | `0x00079c00` | `networking/network_observer.c` |
| `network_observer_channel_drop_flow` | `0x00079850` | `networking/network_observer.c` |

### Lo que se sabe

- Eran las dos «cosas que se hacen con un canal» que quedaban sin nombre
  desde los hitos 379 y 381, y son **las dos maneras de bajar el caudal**:
  - `network_observer_channel_drop_flow` (antes `unknown_00079850`) es la
    **bajada pasajera**, cuando se pierden demasiados de los últimos
    avisos: retiene el caudal que había (para devolverlo después, hito
    389) y deja el canal con una parte de él (un real, `0x180`), sin
    bajar de unos mínimos. Cada vez suma al número de `0x4D8`, y cuando
    llega a un tope baja el caudal del todo.
  - `network_observer_channel_lower_flow` (antes `unknown_00079c00`) es la
    **bajada definitiva**: quita una parte de los bits por segundo (un
    real, `0x198`, con un máximo), baja el otro número en proporción y
    recalcula los paquetes por segundo. Si había un caudal retenido, el
    que baja es ese, y el del canal no puede quedar por encima.
- `network_observer_packet_rate_bits_per_second` es la inversa de la del
  hito 388: de paquetes por segundo a bits por segundo.
- Con la bajada pasajera se ve qué limita el otro número del caudal
  (`0x498`): lo que cabe en los paquetes que hay en los milisegundos del
  número de `0x4F0`, más dos. Parece **cuánto se puede mandar de golpe**.
- Un detalle del juego que se deja igual: con caudal retenido, la bajada
  definitiva da los paquetes por segundo del canal sin decimales.

### Comprobado rompiendo líneas

- Veinticinco líneas rotas, una cada vez: las pruebas fallaron en las
  veinticinco.

### Sin comprobar

- Las dos dividen entre números del canal sin mirar que no sean cero; las
  pruebas no dan ceros ahí.
- En este hito se pasaron las pruebas de `network_observer` (58 funciones).
- Del caudal queda `0x00079de0`, la que prueba a subirlo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que se hace con el caudal de un canal cada vez

Tres funciones en `networking/network_observer.c`, 1160 bytes entre todas. De 1361
funciones se pasa a 1364, el 12,11 % de las del juego (el 7,987 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_unknown_00079660` | `0x00079660` | `networking/network_observer.c` |
| `network_observer_channel_update_flow` | `0x0007a330` | `networking/network_observer.c` |
| `network_observer_channel_unknown_00079a10` | `0x00079a10` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_update_flow` es **lo que se hace con el caudal
  de un canal cada vez**, según el número de `0x4DC` (la fase):
  - En la 0, cuando lleva un tiempo así y ya son bastantes los intervalos
    seguidos con avisos (hito 385), pasa a la 1.
  - En cualquier otra, tras un intervalo sin avisos vuelve a la 0.
  - En la 2 pregunta cómo va el caudal (hito 385): si va bien, pasa a la
    3; si regular, recupera el caudal guardado (hito 388); si mal, lo
    cambia (`unknown_00079c00`, con un 1).
  - En las demás, si no hay caudal retenido: si el número de `0x4B0` se
    pasa, cuenta una vez más que algo ha ido bien (hito 381); si no,
    prueba otra cosa (`unknown_00079de0`, nueva), que puede devolverlo a
    la fase 0.
- `network_observer_channel_unknown_00079660` es lo que el hito 377
  llamaba «algo que se le dice al canal con un tamaño». Suma el tamaño a
  los bytes del intervalo, y con el primer número que recibe lleva una
  **media que se acerca poco a poco** (`0x4B0`), y otra del tiempo entre
  una vez y la siguiente (`0x4B4`). Además se ve que un canal puede tener
  **un caudal retenido** (`0x4C0` a `0x4D4`), distinto del guardado del
  hito 388: se le devuelve cuando ha pasado un tiempo proporcional a esa
  media.
- `network_observer_channel_unknown_00079a10` busca, entre los canales en
  uso, **el que más tiene de algo** (el mayor de sus números de `0x510` y
  `0x514`), siempre que supere al canal que se da por un margen, y le
  cambia el caudal (`unknown_00079c00`, con un 0). No lo hace más de una
  vez cada cierto tiempo.

### Comprobado rompiendo líneas

- Veintiséis líneas rotas, una cada vez: las pruebas fallaron en las
  veintiséis.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (55 funciones).
- Quedan del caudal las que hacen cuentas con reales y redondeo
  (`0x00079850`, `0x00079c00`, `0x00079de0`).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuántos paquetes por segundo manda un canal

Cinco funciones en `networking/network_observer.c`, 586 bytes entre todas. De 1356
funciones se pasa a 1361, el 12,08 % de las del juego (el 7,948 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_packet_rate_low` | `0x00078190` | `networking/network_observer.c` |
| `network_observer_packet_rate` | `0x00078090` | `networking/network_observer.c` |
| `network_observer_channel_set_flow` | `0x00079600` | `networking/network_observer.c` |
| `network_observer_channel_restore_flow` | `0x0007a110` | `networking/network_observer.c` |
| `network_observer_channel_unknown_0007a2a0` | `0x0007a2a0` | `networking/network_observer.c` |

### Lo que se sabe

- **El caudal de un canal son tres números**, y ya se sabe qué son dos:
  los bits por segundo (`0x494`) y los **paquetes por segundo** (`0x49C`,
  que es un real: en el hito 385 se daba por entero). Los pone
  `network_observer_channel_set_flow`, que además apunta en un byte si esos
  paquetes son pocos.
- `network_observer_packet_rate` saca los paquetes por segundo de los bits
  por segundo: divide entre lo que ocupa un paquete (uno de dos tamaños en
  bytes, más `0x168` bits de cabecera) y se queda con el primer ritmo de
  una lista que eso alcanza a pagar. Los ritmos son **partes de las veces
  por segundo que va el juego**: 30, o 25 con la imagen a 50 por segundo
  (PAL). Con un byte, no pasa de la mitad.
- `network_observer_packet_rate_low` dice si un ritmo es poco: menos que
  una parte de esas veces por segundo, o menos que todas (o la mitad),
  según tres bytes del canal.
- `network_observer_channel_restore_flow`: un canal puede tener **un caudal
  guardado** (`0x4F4` a `0x500`); esta se lo devuelve.
- `network_observer_channel_unknown_0007a2a0` da un real de un canal cuyo
  número de `0x4DC` es 1: el tiempo desde una hora suya por un número, con
  un tope, menos sus bits por segundo por otro. Parece cuánto puede subir.

### El arnés

- Una de estas recibe un real en un registro que el arnés no conocía
  (`xmm5`). `tools\difftest.py` admite ahora de `xmm0` a `xmm7`.

### Comprobado rompiendo líneas

- Veintidós líneas rotas, una cada vez: las pruebas fallaron en veinte.
  - Una no se veía: subir el tope en uno. En las pruebas el tope era
    siempre un número entero, como lo que se compara con él. Ahora hay
    topes con decimales, y falla.
  - La que queda no cambia nada: mirar un ritmo más de la lista da el
    mismo que se toma al final.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (52 funciones).
- Los bytes que se dan a estas funciones son 0 o 1 en las pruebas.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Los intentos de un canal

Una función en `networking/network_observer.c`, de 637 bytes. De 1355
funciones se pasa a 1356, el 12,04 % de las del juego (el 7,929 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_update_tries` | `0x000776a0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_update_tries` era `unknown_000776a0`, la
  tercera de las cosas que se hacen con cada canal cada vez. Con ella **el
  ciclo de estados de un canal del observador está entero** (hitos 371,
  386 y este): es la que decide cuándo se vuelve a intentar.
  - Tras un tiempo en el primer estado, el canal empieza de nuevo: segundo
    estado, sin intentos, sin dirección y sin sesiones fallidas; y se hace
    otra vez todo lo de cada vez, llamándose a sí misma.
  - En el cuarto estado (ya hay dirección), si se le pidió conectar, pasa
    al quinto.
  - En el segundo, el quinto o el noveno hay **un número de intentos, cada
    uno con su espera** desde que entró en el estado. Son tres listas de
    hasta 8 esperas en lo que el observador guarda en `0x10`: para buscar
    dirección (`0x0`), para conectar (`0x24`) y para volver a conectar un
    canal que ya estuvo conectado (`0x48`).
  - Cuando toca: en el segundo estado busca dirección (hito 383) y pasa al
    tercero, o al primero si no hay; en los otros, el canal del juego
    empieza a conectar con la dirección (`unknown_00088220`, nueva) y pasa
    al sexto estado desde el quinto, o al octavo desde el noveno.
  - Sin intentos, vuelve al primer estado sin dirección.
- Después del primer intento, o si el canal ya estuvo conectado, **alguna
  sesión tiene que seguir queriéndolo**: se pregunta por la tercera
  función de lo que cada sesión tiene en el observador. Si ninguna lo
  quiere, el canal se suelta cuando lleva bastante sin usarse (hito 374).
  Con esta son cuatro las funciones conocidas de las cinco.
- El tercer bit del byte de `0x8` del canal, que el hito 386 ponía al
  conectarse, es eso: el canal ya estuvo conectado alguna vez.

### Comprobado rompiendo líneas

- Veinte líneas rotas, una cada vez: las pruebas fallaron en las veinte.

### Sin comprobar

- Las pruebas sustituyen `network_observer_channel_set_state` por algo que
  solo cambia el estado (sin eso, la función se llamaría a sí misma sin
  fin en el primer estado: en el juego lo evita el mismo cambio).
- En este hito se pasaron las pruebas de `network_observer` (47 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un canal sigue el estado de su canal del juego

Una función en `networking/network_observer.c`, de 821 bytes con sus tablas de saltos. De 1354
funciones se pasa a 1355, el 12,03 % de las del juego (el 7,907 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_update_game_channel` | `0x00076ff0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_update_game_channel` era `unknown_00076ff0`,
  una de las tres cosas que se hacen con cada canal cada vez (hito 371). Es **la mitad que faltaba de los estados de un canal**: la
  del hito 371 sigue cómo va su dirección; esta sigue cómo va su canal del
  juego.
  - Sin canal del juego: se queda sin dirección y vuelve al primer estado.
  - En el sexto o el octavo estado: si el canal del juego llega a su quinto
    estado, pasa al **séptimo** (está conectado). Si el canal del juego ha
    caído a su segundo estado o antes, manda el número que este tiene en
    `0x58` (por qué cayó): con 1, 2 o 5 se rinde (sin dirección, anotado
    con `0xF`, primer estado); con 9 se queda sin dirección y sin
    intentos; y en todos los demás casos cuenta un intento más y vuelve a
    probar (quinto estado desde el sexto, noveno desde el octavo).
  - En el séptimo: si el canal del juego ya no está en su quinto estado,
    pasa al octavo, o al noveno si cayó del todo.
- Después **avisa a las sesiones**. Cada sesión que usa el canal tiene en
  el observador algo con cinco funciones; por la cuarta se le dice que el
  canal está conectado, o que ha dejado de estarlo, con el número de
  `0x4C` del canal del juego (que el canal guarda en `0x10`). Con esta
  son tres las funciones conocidas de las cinco.
- Al conectarse, el canal **pone a cero todo lo que mide**: sus dos
  cuentas por tramos de tiempo (`0xA0` y `0x178`; ahora se sabe que la
  primera también lo es), sus dos series de últimos números, tres números
  más y dos horas. Una de esas horas no es de reloj: es un número de 64
  bits de otra parte del juego (`unknown_004e6398`, nuevo) menos las
  imágenes por segundo de la consola (60 si no lo dice), o sea, hace un
  segundo contado en imágenes.

### Comprobado rompiendo líneas

- Veintitrés líneas rotas, una cada vez: las pruebas fallaron en veintidós.
  La que no se veía (tratar un estado del canal del juego mayor que el
  quinto como si fuera el quinto) faltaba en las pruebas: ningún canal del
  juego pasaba del quinto. Ahora alguno sí, y falla.

### Sin comprobar

- Las pruebas sustituyen `network_observer_channel_set_state` por algo que
  solo cambia el estado; la de verdad además apunta la hora y avisa a las
  sesiones, y tiene sus propias pruebas (hito 371).
- El juego elige con una tabla por el estado del canal, sin mirar que
  esté entre 1 y 9; las pruebas no salen de ahí.
- En este hito se pasaron las pruebas de `network_observer` (46 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El intervalo del caudal, y a cuántas consolas llega la voz

Cinco funciones en `networking/network_observer.c`, 1054 bytes entre todas. De 1349
funciones se pasa a 1354, el 12,02 % de las del juego (el 7,887 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_get_unknown4b0` | `0x00078a10` | `networking/network_observer.c` |
| `network_observer_new_interval` | `0x00079480` | `networking/network_observer.c` |
| `network_observer_channel_unknown_0007a1c0` | `0x0007a1c0` | `networking/network_observer.c` |
| `network_observer_unknown_00077f90` | `0x00077f90` | `networking/network_observer.c` |
| `network_observer_update_voice_console_counts` | `0x0007a740` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_update_voice_console_counts` es la que más enseña: une
  el observador con la voz. Con el número de `0x4E04` del observador (lo
  que la conexión da de sí) y **dos tablas de mínimos**, una entrada por
  cada número de consolas de 1 a 16 (`unknown_004ce12c` y
  `unknown_004ce174`, nuevas), decide **a cuántas consolas puede llegar lo
  que dice un jugador** desde esta. Son los dos números de `game_voice.c`
  (`unknown_004c9880` y `unknown_004c9884`) que ninguna función
  descompilada escribía. Si no llega ni para una, cero.
- `network_observer_new_interval` **cierra el intervalo** en el que se
  cuentan los bytes de cada canal (el de los bits por segundo del hito
  380) y abre otro. De cada canal: guarda los tres avisos que se anotaron
  en el intervalo (solo si pasó algún byte), lleva la cuenta de cuántos
  intervalos seguidos han ido sin avisos y cuántos con ellos, y baja el
  número de `0x4F0` si el de `0x4B0` es menor.
  - Pide la hora dos veces y calcula los bits por segundo de `0x4A8` sin
    usar ninguna de las dos cosas: se deja igual, para que se llame lo
    mismo que en el juego.
- `network_observer_channel_unknown_0007a1c0` dice **cómo va el caudal de
  un canal** con un número: 2 si el número de `0x4B0` pasa de un tope
  (`0x504`) más un margen; 0 si va por encima de los bits por segundo de
  `0x508` sin pasarse del otro tope; 1 en lo demás, y antes de que el
  intervalo haya durado lo mínimo. El margen es uno de dos, según un byte
  del canal.
- `network_observer_unknown_00077f90` recibe tres números. Con dos de ellos
  decide **cuál de las dos cuentas del observador sube** (las del hito 326,
  de las que se saca una fracción) y solo la sube cuando lleva un tiempo
  siendo la misma. Con el otro lleva un máximo (`0x4E08`) y un número que
  se le acerca poco a poco, solo hacia arriba (`0x4E0C`).
- `network_observer_channel_get_unknown4b0` da cuatro números de un canal:
  tres tal cual y un **tanto por ciento** (la suma de los últimos números
  de `0x250` entre cuántos son).

### Comprobado rompiendo líneas

- Veinte líneas rotas, una cada vez: las pruebas fallaron en diecinueve.
  - Una no se veía: dar por bueno el canal número 15, que no existe. Las
    pruebas no ponían nada detrás del observador. Ahora sí, y falla.
  - La que queda no cambia nada: subir el número de `0x4E0C` cuando el
    que llega es igual lo deja como estaba.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (45 funciones).
- No se sabe quién rellena las dos tablas de mínimos. Su tamaño (17 números cada
  una) sale de lo que esta función lee; entre las dos quedan 4 bytes sin
  dueño.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un canal olvida su dirección

Una función en `networking/network_observer.c`, de 214 bytes. De 1348
funciones se pasa a 1349, el 11,98 % de las del juego (el 7,852 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_forget_address` | `0x000784a0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_forget_address` era `unknown_000784a0`, la que
  en los hitos 371 y 383 «anotaba algo con un byte y un número». Es **lo
  contrario del hito 383**: el canal se queda sin dirección.
  - Si el canal del juego ya había pasado de su segundo estado, se le dice
    el número que se da (es el motivo: `0xD` al perder la conexión, `0xE`
    al vaciar el canal).
  - Si tenía dirección: se avisa a `unknown_0007b390`; con el byte a 1, la
    sesión de la que era la dirección **queda marcada como fallida** (para
    que la búsqueda del hito 383 pruebe con otra); las bibliotecas de la
    consola sueltan la dirección (`XNetUnregisterInAddr`); y queda vacía.
- Con esta, el ciclo de la dirección de un canal está entero: buscarla
  (hito 383), seguir cómo va (hito 371) y soltarla.

### Comprobado rompiendo líneas

- Cuatro líneas rotas, una cada vez: las pruebas fallaron en las cuatro.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (40 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — La dirección de la consola de un canal

Dos funciones en `networking/network_observer.c`, 354 bytes entre las dos. De 1346
funciones se pasa a 1348, el 11,97 % de las del juego (el 7,844 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_session_get_address` | `0x000783d0` | `networking/network_observer.c` |
| `network_observer_channel_find_address` | `0x00078330` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_session_get_address` (antes `unknown_000783d0`) saca
  **la dirección del juego de una consola para una sesión**: se la pide a
  las bibliotecas de la consola con la clave registrada de la sesión y el
  puerto 1000, y comprueba que se puede conectar. Devuelve con ella el
  número de la clave, lo que identifica a la sesión y su clave.
- `network_observer_channel_find_address` busca dirección para un canal:
  prueba con cada sesión que usa el canal y que no haya fallado antes (un
  bit por sesión en el `0x38` del canal), y la primera que da dirección
  queda como **la sesión del canal** (`0x3C`), con sus datos copiados en el
  canal (`0x40` a `0x5C`). Si la dirección que ya tiene está conectando o
  conectada, no hace nada.
- Con esto se sabe qué son los `0x48` bytes del canal entre `0x14` y
  `0x70`: cómo se conoce a la consola, las sesiones que fallaron, la
  sesión elegida, sus datos y la dirección.

### Corregido

- **Un fallo mío del hito 370.** `network_observer_channel_send` llamaba a
  esta función dándole, para lo que identifica a la sesión y su clave, dos
  huecos de 4 bytes; la función escribe 8 y 16. Las pruebas no lo veían
  porque en ellas la función estaba sustituida y no escribía ahí. En el
  juego esos huecos tienen su tamaño; ahora aquí también.

### Comprobado rompiendo líneas

- Seis líneas rotas, una cada vez: las pruebas fallaron en las seis.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (39 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Soltar los canales del juego, y una pregunta a las sesiones

Dos funciones en `networking/network_observer.c`, 280 bytes entre las dos. De 1344
funciones se pasa a 1346, el 11,95 % de las del juego (el 7,832 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_release_game_channels` | `0x00078880` | `networking/network_observer.c` |
| `network_observer_channel_unknown_00077940` | `0x00077940` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_release_game_channels`: cada canal del observador que
  está en uso **suelta su canal del juego** y se queda sin él. Al que ya
  había pasado de su segundo estado se le avisa antes con un 3
  (`unknown_00088650`).
- `network_observer_channel_unknown_00077940`: pregunta algo de un canal a lo
  que cada sesión que lo usa tiene en el observador, por su **primera
  función**, y devuelve si alguna dice que sí. Solo pregunta si el canal
  del juego está en su cuarto estado o más allá y lleva callado el plazo de
  `0x70`. Con la del hito 371 se conocen ya dos de las cinco funciones de
  eso que cada sesión tiene en el observador.

### Comprobado rompiendo líneas

- Cinco líneas rotas, una cada vez: las pruebas fallaron en las cinco.

### Sin comprobar

- `network_observer_release_game_channels` no mira si el canal en uso tiene
  canal del juego; en las pruebas siempre lo tiene.
- En este hito se pasaron las pruebas de `network_observer` (37 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Dos piezas del caudal de un canal

Dos funciones en `networking/network_observer.c`, 151 bytes entre las dos. De 1342
funciones se pasa a 1344, el 11,93 % de las del juego (el 7,823 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_reset_unknown4dc` | `0x00079d90` | `networking/network_observer.c` |
| `network_observer_channel_unknown_0007a160` | `0x0007a160` | `networking/network_observer.c` |

### Lo que se sabe

- Son dos piezas del control del caudal de un canal (hitos 376 a 380):
  - `network_observer_channel_unknown_0007a160` cuenta **las veces que algo
    ha ido bien**. Cada tantas (el número de `0x1E0` de lo que el observador
    guarda en `0x10`) hace `unknown_00079c00` con un cero; y cuando ya son
    muchas (`0x1E4`), sube otro número del canal de golpe (`0x1E8`). Parece
    la subida gradual del caudal cuando la conexión aguanta, la contraria
    de la del hito 379.
  - `network_observer_channel_reset_unknown4dc` pone a cero el número que
    dice en qué fase está el caudal del canal, y apunta la hora.

### Comprobado rompiendo líneas

- Tres líneas rotas, una cada vez: las pruebas fallaron en las tres.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (35 funciones).
- Lo que queda de esta parte (`0x00079850`, `0x00079c00`, `0x00079600`,
  `0x0007a2a0`...) hace cuentas con reales y con el redondeo del procesador
  antiguo (`fistp`): va más despacio.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuántos bits por segundo lleva un canal

Dos funciones en `networking/network_observer.c`, 156 bytes entre las dos. De 1340
funciones se pasa a 1342, el 11,91 % de las del juego (el 7,818 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_unknown4a8_bits_per_second` | `0x00079560` | `networking/network_observer.c` |
| `network_observer_channel_unknown4a4_bits_per_second` | `0x000795b0` | `networking/network_observer.c` |

### Lo que se sabe

- Son dos funciones gemelas: cada una toma un número de bytes de un canal
  (el de `0x4A8` o el de `0x4A4`), lo multiplica por 8000 y lo divide por
  los milisegundos que han pasado desde la hora de `0x4F38` del observador.
  Bytes por 8 bits, por 1000 milisegundos: **son bits por segundo**.
- Esto confirma lo que en los hitos 376 a 379 era una sospecha: esta parte
  del observador **mide y limita el caudal** de cada conexión. Los números
  de `0x4A4` y `0x4A8` son bytes contados en el intervalo que empezó en
  `0x4F38`.

### Comprobado rompiendo líneas

- Tres líneas rotas, una cada vez: las pruebas fallaron en las tres.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (33 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuántas de las últimas cosas de un canal fallaron

Una función en `networking/network_observer.c`, de 155 bytes. De 1339
funciones se pasa a 1340, el 11,90 % de las del juego (el 7,813 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_unknown_000797b0` | `0x000797b0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_unknown_000797b0` (antes `unknown_000797b0`) es la
  tercera cuenta que un canal lleva de su conexión, y la más directa: **un
  bit por cada una de las últimas cosas que le dijeron, puesto si esa cosa
  no se cumplió**, y cuántos bits hay puestos. Cuántas «últimas» cuentan lo
  dice el observador (el número de `0x174` de lo que guarda en `0x10`).
- Cuando los bits puestos llegan a ese número por un real (el de `0x178`),
  es decir, **cuando falla demasiada parte de lo último**, se hace
  `unknown_00079850` y la cuenta empieza de cero. Encaja con que sea la
  cuenta de lo que se pierde por el camino y la reacción a perder
  demasiado, pero `unknown_00079850` aún no está descompilada.
- Es a lo que llama `network_observer_unknown_00076810` (hito 372) cuando el
  canal tiene su byte de `0x48C`.

### Batería completa

- **La batería completa del hito 371 terminó: 1.328 funciones, las 1.328
  bien.**

### Comprobado rompiendo líneas

- Cinco líneas rotas, una cada vez: las pruebas fallaron en las cinco.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (31 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las dos cuentas empiezan

Dos funciones en `networking/network_observer.c`, 182 bytes entre las dos. De 1337
funciones se pasa a 1339, el 11,89 % de las del juego (el 7,808 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_rate_new` | `0x00092870` | `networking/network_observer.c` |
| `network_window_reset` | `0x000929d0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_rate_new` prepara una cuenta por tramos (hito 377) para un
  tiempo total en milisegundos. Con ella se aclaran los dos datos que
  faltaban de `struct network_rate`: el de `0x1C` es **ese tiempo total**
  (los veinte tramos), y el de `0x24` es un real, **1000 dividido por ese
  tiempo**: lo que convierte una suma de los veinte tramos en «tanto por
  segundo». Confirma que es un medidor de tráfico por segundo.
- `network_window_reset` deja a cero la cuenta de los últimos números (hito
  372), con todas sus horas en «ahora».

### Comprobado rompiendo líneas

- Cuatro líneas rotas, una cada vez: las pruebas fallaron en las cuatro.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (30 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cuánto pasa por un canal

Dos funciones en `networking/network_observer.c`, 445 bytes entre las dos. De 1335
funciones se pasa a 1337, el 11,87 % de las del juego (el 7,801 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_rate_update` | `0x000928e0` | `networking/network_observer.c` |
| `network_observer_unknown_00076720` | `0x00076720` | `networking/network_observer.c` |

### Lo que se sabe

- `network_rate_update` lleva **la cuenta de algo por tramos de tiempo**
  (`struct network_rate`, `0xD4` bytes): cuántas cosas y de qué tamaño en
  total, cuántas en el tramo de ahora, y las de los últimos veinte tramos
  con sus sumas. Cuando un tramo se acaba, lo contado en él ocupa el sitio
  del más viejo. Si pasan más de veinte tramos sin que nadie pregunte,
  empieza de cero. Con las sumas y la duración del tramo sale una media:
  es **el medidor de tráfico**.
- `network_observer_unknown_00076720`: al observador le dicen que algo de un
  tamaño pasó por un canal del juego, y lo apunta en la cuenta del canal
  que le corresponde (en su `0x178`): una cosa más, tantos bytes más, en el
  total (64 bits) y en el tramo de ahora.
- Con la del hito 372 son las dos maneras en que cada canal mide su
  conexión: una por tramos de tiempo (esta) y otra por los últimos números
  (aquella).
- Como `network_window_add`, `network_rate_update` está en el ejecutable
  lejos del observador (`0x000928e0`).

### Comprobado rompiendo líneas

- Seis líneas rotas, una cada vez: las pruebas fallaron en las seis.

### Sin comprobar

- Como la del hito 372, `network_observer_unknown_00076720` no mira si
  encontró el canal; en las pruebas siempre lo hay.
- En este hito se pasaron las pruebas de `network_observer` (28 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Las cuentas del observador empiezan de nuevo

Una función en `networking/network_observer.c`, de 390 bytes. De 1334
funciones se pasa a 1335, el 11,85 % de las del juego (el 7,786 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_reset_unknown4e08` | `0x00075af0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_reset_unknown4e08` deja como al principio todo lo que el
  observador guarda después de sus canales (de `0x4E08` a `0x4F3C`): tres
  horas que pasan a ser «ahora», seis números que quedan en NONE, un byte a
  1 y el resto a cero. Dos números salen de lo que el observador tiene en
  `0x10`: uno tal cual, y otro que es el mayor entre ese y el número de
  `0x4E04` dividido por 1024 menos un margen. Por las unidades, parecen
  **los límites de lo que se puede mandar por segundo**, pero no está
  confirmado.
- Además quita el byte de `0x48C` de cada canal en uso (el que hace que al
  canal se le lleve más cuenta, hito 372).
- Con esto tienen nombre (provisional) todos los sitios de esa parte del
  observador, que es lo que usan las funciones grandes que quedan.

### Comprobado rompiendo líneas

- Cinco líneas rotas, una cada vez: las pruebas fallaron en las cinco.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (26 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Un canal pregunta por su consola

Una función en `networking/network_observer.c`, de 252 bytes. De 1333
funciones se pasa a 1334, el 11,84 % de las del juego (el 7,773 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_update_probe` | `0x00077480` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_update_probe` (antes `unknown_00077480`): cada
  canal del observador **pregunta una vez a la red por su consola**. Lo
  pide con lo que identifica a la primera sesión que usa el canal, su clave
  y lo que el canal tiene en `0x14` (`unknown_0007b4c0`), y guarda el
  número de la petición en `0x70` (el mismo número que
  `network_observer_channel_delete` suelta, hito 371).
- Cuando la petición termina, guarda lo encontrado en `0x74` y marca dos
  bits del byte de `0x8` del canal: «ya preguntó» (8) y «tiene respuesta»
  (`0x10`). Por lo que se pide y cómo se usa, parece **la medida de la
  calidad de la conexión** con esa consola (lo que las bibliotecas de la
  consola llaman QoS), pero no está confirmado.

### Corregido

- En la cabecera del observador había quedado, desde el hito 372, un
  comentario pegado al `#define` de la línea siguiente. Compilaba igual,
  pero estaba mal puesto; ya está en su línea.

### Comprobado rompiendo líneas

- Seis líneas rotas, una cada vez: las pruebas fallaron en las seis.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (25 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Dos plazos de un canal del observador

Dos funciones en `networking/network_observer.c`, 499 bytes entre las dos. De 1331
funciones se pasa a 1333, el 11,83 % de las del juego (el 7,765 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_unused_expired` | `0x00077580` | `networking/network_observer.c` |
| `network_observer_channel_unknown_000773a0` | `0x000773a0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_unused_expired` (antes `unknown_00077580`)
  decide **cuándo se suelta un canal que ya no usa ninguna sesión**. No se
  suelta de golpe: se le da tiempo a que salga lo que tenía por mandar.
  - Si el canal del juego tiene poca cola (menos de `0x4000`), ya no le
    queda nada; si tiene más, se le avisa (`unknown_00088650`).
  - Se suelta cuando lleva sin uso más de `unknown_004cf56c` milisegundos;
    o cuando lleva con cosas por mandar más de `unknown_004cf568`; o
    enseguida, si no tiene dirección a la que mandar.
- `network_observer_channel_unknown_000773a0` (antes `unknown_000773a0`): con
  el canal del juego conectado y con actividad reciente, si llevan callados
  bastante tanto el canal del observador como el del juego, le dice `0x10`
  al canal del juego. Parece **el aviso para mantener viva la conexión**.
  Los tres plazos están en lo que el observador guarda en `0x10`.
- Corregido en la entrada del hito 373: las funciones que el paso del
  observador llama sin estar descompiladas eran ocho, no siete; con este
  hito quedan seis.

### Comprobado rompiendo líneas

- Ocho líneas rotas, una cada vez: las pruebas fallaron en siete. La otra
  no cambia nada: es un caso que no puede darse donde está.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (24 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Lo que el observador hace en cada vuelta

Una función en `networking/network_observer.c`, de 151 bytes. De 1330
funciones se pasa a 1331, el 11,82 % de las del juego (el 7,748 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_update` | `0x00075da0` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_update` es **el paso del observador**, como
  `network_session_update` lo es de la sesión (hito 357). Recorre sus quince
  canales y, con cada uno que está en uso:
  - si ninguna sesión lo usa ya y `unknown_00077580` lo confirma, lo deja
    vacío (`network_observer_channel_delete`, hito 371);
  - si sigue en uso, sigue su conexión
    (`network_observer_channel_update_connection`, hito 371) y hace cuatro
    cosas más que aún no están descompiladas.
- Al final hace tres cosas con el observador entero, tampoco descompiladas
  todavía.
- Sirve de **mapa de lo que queda del observador**: las ocho funciones que
  llama y no están (`0x00076ff0`, `0x000773a0`, `0x00077480`,
  `0x00077580`, `0x000776a0`, `0x000779f0`, `0x00078ac0`, `0x0007a4a0`)
  quedan apuntadas con cómo se las llama.

### Comprobado rompiendo líneas

- Cuatro líneas rotas, una cada vez: las pruebas fallaron en las cuatro.

### Sin comprobar

- En las pruebas todo lo que llama está sustituido; lo que sustituye a
  dejar vacío el canal lo deja de verdad sin uso la mayoría de las veces,
  para comprobar que la función lo vuelve a mirar.
- En este hito se pasaron las pruebas de `network_observer` (22 funciones).
- La batería completa del hito 371 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Los últimos números de un canal

Dos funciones en `networking/network_observer.c`, 246 bytes entre las dos. De 1328
funciones se pasa a 1330, el 11,81 % de las del juego (el 7,743 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_window_add` | `0x00092a30` | `networking/network_observer.c` |
| `network_observer_unknown_00076810` | `0x00076810` | `networking/network_observer.c` |

### Lo que se sabe

- `network_window_add` lleva **la cuenta de los últimos números de algo**:
  hasta 32, cada uno con la hora en que llegó, la suma de todos y cuánto
  hace del más viejo. El nuevo ocupa el sitio del más viejo. Es una
  estructura de `0x110` bytes (`struct network_window`), y cada canal del
  observador tiene dos, en `0x250` y en `0x360`. Con la suma y el tiempo se
  puede sacar una media por segundo: parece lo que usa el juego para medir
  cómo va la conexión con cada consola.
- `network_observer_unknown_00076810`: al observador le dicen dos cosas de un
  canal del juego, y las apunta en esas dos cuentas del canal que le
  corresponde: en la primera si la primera cosa *no* se cumple, y en la
  segunda el segundo byte tal cual.
- `network_window_add` está en el ejecutable lejos del observador
  (`0x00092a30`); de momento vive en el mismo archivo.

### Comprobado rompiendo líneas

- Seis líneas rotas, una cada vez: las pruebas fallaron en las seis.

### Sin comprobar

- `network_observer_unknown_00076810` no mira si encontró el canal: si no hay
  ninguno que sea ese canal del juego, el juego escribe antes del primer
  canal. En las pruebas siempre lo hay.
- En este hito se pasaron las pruebas de `network_observer` (21 funciones).
- La batería completa del hito 371 está corriendo en la copia de
  comprobación.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — El estado de un canal del observador

Cuatro funciones en `networking/network_observer.c`, 434 bytes entre todas. De 1324
funciones se pasa a 1328, el 11,79 % de las del juego (el 7,735 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_channel_connect` | `0x00076a40` | `networking/network_observer.c` |
| `network_observer_channel_delete` | `0x00076ef0` | `networking/network_observer.c` |
| `network_observer_channel_update_connection` | `0x00076f50` | `networking/network_observer.c` |
| `network_observer_channel_set_state` | `0x00077330` | `networking/network_observer.c` |

### Lo que se sabe

- Cada canal del observador tiene **un estado** (su primer número) y la
  hora en que entró en él. Se van viendo los estados:
  - 0: el canal no se usa;
  - 1: recién puesto a cero, o caído. Al entrar en él se avisa a lo que
    cada sesión que usa el canal tiene en el observador (su quinta
    función). Es el estado que la sesión mira para echar a una consola
    (hito 357);
  - 2: se le ha pedido conectar;
  - 3: conectando: sigue cómo va la dirección de la consola; si llega a
    conectarse pasa al 4, si falla vuelve al 2 y cuenta un intento más;
  - 4 en adelante: conectado; si la conexión se pierde vuelve al 2;
  - 7: listo para mandar (lo que la sesión espera en muchos sitios).
- `network_observer_channel_set_state` es quien cambia el estado.
- `network_observer_channel_update_connection` es el paso que sigue a la
  conexión, como se cuenta arriba.
- `network_observer_channel_connect` (antes `unknown_00076a40`) es lo que
  la sesión llama para **abrir el canal** con una consola: del estado 1
  pasa al 2 y hace ya lo de cada vuelta.
- `network_observer_channel_delete` (antes `unknown_00076ef0`) deja el
  canal vacío: suelta el canal del juego y lo que su número de `0x70` dice.

### Comprobado rompiendo líneas

- Ocho líneas rotas, una cada vez: las pruebas fallaron en las ocho.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (19 funciones)
  y las de `network_life_cycle` (223 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Mandar algo por un canal

Tres funciones en `networking/network_observer.c`, 383 bytes entre todas. De 1321
funciones se pasa a 1324, el 11,75 % de las del juego (el 7,720 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_unknown_00075910` | `0x00075910` | `networking/network_observer.c` |
| `network_channel_get_address` | `0x00075930` | `networking/network_observer.c` |
| `network_observer_channel_send` | `0x00075e80` | `networking/network_observer.c` |

### Lo que se sabe

- `network_observer_channel_send` era `unknown_00075e80`, **la función con
  la que la sesión manda todos sus mensajes** (la llamaba desde 21 sitios
  sin que estuviera descompilada). Ahora se sabe qué son sus «dos maneras»,
  que en los hitos anteriores se llamaban así sin más:
  - **por el canal** (primer byte a 0): el mensaje se pone en la cola del
    canal del juego, si ese canal ha pasado de su segundo estado, y la
    clase deja de estar pendiente en el canal. Es la manera fiable.
  - **directo a la dirección** (primer byte a 1): se manda tal cual a la
    dirección de la consola (`unknown_0007b140`), la del canal o, si el
    canal no tiene, la que encuentre `unknown_000783d0`. Es la manera que
    no espera a que el canal esté listo, y por eso es la que usan el noveno
    estado y las respuestas a quien aún no está conectado.
- `network_channel_get_address`: la dirección de un canal del juego, que
  la tiene desde su tercer estado.
- `network_unknown_00075910`: una pregunta de una línea sobre algo que
  todavía no se sabe qué es.

### Comprobado rompiendo líneas

- Siete líneas rotas, una cada vez, entre este hito y el 369: las pruebas
  fallaron en las siete.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (16 funciones)
  y las de `network_life_cycle` (223 funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 368 están
  pendientes.

## 2026-10-09 — Cinco funciones del observador que la sesión usaba a ciegas

Cinco funciones en `networking/network_observer.c`, 519 bytes entre todas. De 1316
funciones se pasa a 1321, el 11,73 % de las del juego (el 7,707 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_sessions_find` | `0x00075800` | `networking/network_observer.c` |
| `network_observer_channel_unknown_000768b0` | `0x000768b0` | `networking/network_observer.c` |
| `network_observer_channel_set_pending` | `0x00076930` | `networking/network_observer.c` |
| `network_observer_channel_alive` | `0x000769a0` | `networking/network_observer.c` |
| `network_observer_channel_connect_status` | `0x00078580` | `networking/network_observer.c` |

### Lo que se sabe

- Las cinco eran funciones «sin nombre» que la sesión de red llamaba desde
  hace varios hitos. Ya tienen código y nombre, y dicen qué mira la sesión
  de cada canal:
  - `network_observer_channel_alive` (antes `unknown_000769a0`): si la
    consola del otro lado **sigue ahí**. Mira dos horas del canal del juego
    contra dos plazos que el observador tiene en lo que guarda en `0x10`.
    Es lo que decide, en los hitos 354 y 366, si se da por perdida a la
    consola que lleva la sesión.
  - `network_observer_channel_connect_status` (antes `unknown_00078580`):
    cómo va la conexión con la dirección del canal. Es lo que el primer
    estado de la sesión espera que valga 2 (hito 354).
  - `network_observer_channel_unknown_000768b0` y
    `network_observer_channel_set_pending`: cada canal tiene **un bit por
    cada clase de mensaje que tiene pendiente de mandar** (64 bits en su
    `0x520`). La primera dice si el canal admite otro mensaje de una clase
    (según cuánto tiene en cola el canal del juego); la segunda apunta la
    clase como pendiente. Es lo que usa el reparto del hito 356.
  - `network_sessions_find` (antes `unknown_00075800`): cuál de las tres
    sesiones de una tabla se conoce en la red por unos ocho bytes.
- Las cuatro del canal solo hacen algo si el canal del juego está en su
  quinto estado.

### Lo que enseñó

- `network_observer_channel_unknown_000768b0` falló a la primera en un caso
  rebuscado: multiplica por cuatro un número que puede no caber, y el
  compilador de `source\` **da por hecho que una cuenta con signo no se
  desborda** y la simplifica; el juego multiplica sin más. Hay que escribir
  esas cuentas sin signo. Queda apuntado en los avisos de `ESTADO.md`.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_observer` (13 funciones)
  y las de `network_life_cycle` (223 funciones).
- `0x000619b0` (cuál de dos consolas es mejor para llevar la sesión) queda
  sin descompilar: usa la potencia de la biblioteca C del juego
  (`_CIpow`), que el arnés de pruebas todavía no tiene.
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355, 368 y 369 están
  pendientes.

## 2026-10-09 — Tres preguntas más a la sesión de la partida

Tres funciones en `networking/network_life_cycle.c`, 248 bytes entre todas. De 1313
funciones se pasa a 1316, el 11,68 % de las del juego (el 7,688 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_unknown_00063e90` | `0x00063e90` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00063ec0` | `0x00063ec0` | `networking/network_life_cycle.c` |
| `network_life_cycle_set_unknown4db0` | `0x00064060` | `networking/network_life_cycle.c` |

### Lo que se sabe

- Son de la misma familia que las de los hitos 348 y anteriores: funciones
  cortas con las que el resto del juego le pide algo a la sesión de la
  partida en red, sin tocarla directamente.
- `network_life_cycle_set_unknown4db0` cambia la cosa de `0x4DB0` de la
  sesión (la de `0x130` bytes) y, con ella, el número de `0x4DAC`: 0 si la
  sesión no es de red, 1 si no se da ninguna cosa, y si no lo que diga
  `unknown_00161c90` de ella (1 o 2). Solo si esta consola dirige la sesión.
- `network_life_cycle_unknown_00063e90` y `network_life_cycle_unknown_00063ec0`
  devuelven lo que calculan dos funciones aún sin descompilar
  (`unknown_00066050` y `unknown_0007e100`) con la sesión de la partida.

### Líneas rotas del hito 367

- Dieciocho líneas rotas: las pruebas fallaron en dieciséis. De las dos que
  se escaparon, una **no cambia nada** (pone a cero un contador de una
  consola que acaba de dejarse entera a cero) y la otra era un caso que
  faltaba: un jugador que entra y ya está en la sesión. Se añadió y ahora
  se detecta.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (223
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-09 — Recibir «quién está»

Una función en `networking/network_session.c`, de 2.578 bytes: la más grande de la
sesión. De 1312 funciones se pasa a 1313, el 11,66 % de las del juego (el
7,680 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_membership_update_received` | `0x0005cb80` | `networking/network_session.c` |

### Lo que se sabe

- Es **el otro extremo del hito 365**: lo que hace una consola cuando la que
  lleva la sesión le manda «quién está» (el mensaje de la clase `0x19`).
- Si el mensaje trae solo lo que cambió desde una versión que no es la que
  la consola tiene, da por perdida a la que lleva la sesión
  (`network_session_host_lost`) y no hace más.
- Si no, guarda una copia de cómo estaba todo y va por pasos:
  1. **Repasa las consolas del mensaje** sin tocar nada: la que entra tiene
     que traer todo lo suyo y un sitio válido; la que sale y la que sigue
     tienen que conocerse igual que la del sitio que dicen, y cada sitio
     solo puede nombrarse una vez.
  2. **Las coloca**: la nueva en su sitio (que no puede ser el de una que
     sigue); la que cambió de sitio se copia desde la copia de antes; a la
     que cambió se le aplica lo que cambió
     (`network_session_peer_properties_apply_update`, hito 349).
  3. **Cuenta cuántas quedan**: con «todo», son las que vienen, desde el
     primer sitio; con «lo que cambió», no puede quedar ningún hueco entre
     ellas.
  4. **Lo que la sesión guarda de cada consola va con ella** a su sitio
     nuevo; para una consola nueva se pide canal (`unknown_00076aa0`) y se
     empieza de cero. Si no hay canal, falla.
  5. La consola que lleva la sesión, esta consola y la consola de cada
     jugador pasan a su sitio nuevo. Las dos primeras tienen que seguir
     estando; un jugador cuya consola ya no está, deja de estar.
  6. La consola que dirige la sesión, si viene (con «todo» tiene que venir).
  7. **Los jugadores**: sale, entra (no puede estar ya, y su sitio en su
     consola tiene que estar libre) o cambia. Al que entra se le pone además
     lo mismo que al que cambia.
- Si algo falla, lo hecho hasta ahí se queda hecho, se pone a 1 el byte de
  `0x48` y **la sesión se desconecta**.
- Un detalle que no cuadra con quien manda el mensaje: al que entra o cambia
  se le copian siempre su número y sus dos trozos, sin mirar el byte que
  dice si vienen. Está descompilado como lo hace el juego.
- Con este hito **el camino de «quién está» también está entero**. Entre
  `0x00059e50` y `0x000631f0` solo queda sin descompilar `0x000619b0` (cuál
  de dos consolas es mejor para llevar la sesión). Alrededor quedan
  `0x00058ee0` y `0x000590b0` antes, y once funciones de `0x00063220` a
  `0x00064060` después.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (220
  funciones).
- Las dieciocho líneas rotas del hito 366 se detectaron todas.

## 2026-10-08 — Lo que llega de una consola en su noveno estado

Dos funciones en `networking/network_session.c`, 1.280 bytes entre las dos. De 1310
funciones se pasa a 1312, el 11,65 % de las del juego (el 7,593 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_time` | `0x00075870` | `networking/network_session.c` |
| `network_session_election_received` | `0x0005e7f0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_election_received` es **el otro extremo del hito 359**:
  atiende el mensaje de la clase `0x16`, el que una consola manda en su
  noveno estado con lo que sabe de las consolas conocidas y a cuál ha
  elegido para llevar la sesión.
- Antes de nada decide si le toca. No lo toma, y dice por qué con un
  número: si la sesión no está en ningún estado (1), si no dice quién está
  en ella (2), si esta consola lleva la sesión (5), si la que la lleva sigue
  ahí (4 o 6, y devuelve cómo se la conoce), si aún no sabe quién está (8),
  o si no viene de otra consola de la sesión (9). En el octavo estado, y
  cuando esta consola lleva la sesión, contesta con el mensaje `0x12` (el
  que espera el octavo estado, hito 355).
- Si la que llevaba la sesión ya no contesta, **la sesión entra en su
  noveno estado** aquí, y compara lo que llega con lo que ella tiene:
  - gana lo que trae una versión mayor de «quién está»;
  - con la misma versión, si las dos consolas eligieron a la misma, **están
    de acuerdo** y la que lo mandó queda apuntada como conforme;
  - si eligieron distinto, se queda con **la mejor candidata**: la que
    conoce a más consolas, luego la del número mayor, luego la que está
    antes en la lista;
  - si lo que llega elige a una consola que no es de la sesión, o no está
    donde dice, motivo 7, y la que lo mandó pasa a las que no contestan.
- Cuando se toma lo que llega, la sesión copia lo de la otra consola, apunta
  a la elegida y la hora, y se da a sí misma por conforme.
- `network_time` es la hora en milisegundos como función suelta: lo mismo
  que el juego lleva escrito dentro de las demás funciones de la sesión.
- Con este hito **el noveno estado está entero**: entrar, repetir (hito
  359), mandar y recibir.

### Lo que enseñó

- En las pruebas del hito 364 llamé `known` a una variable, y así se llama
  una función de las pruebas que las de este hito usan: las pruebas
  reventaban antes de empezar. Era del archivo de pruebas, no del código.
- La línea rota que se escapó en el hito 365 (un jugador cuya consola es
  NONE no se distinguía) ya tiene su caso, y ahora se detecta: 15 de 15.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (219
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — Preparar «quién está» para mandarlo

Una función en `networking/network_session.c`, de 1.494 bytes. De 1309
funciones se pasa a 1310, el 11,63 % de las del juego (el 7,550 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_membership_get_update` | `0x00060400` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_membership_get_update` era `unknown_00060400`, la otra
  función que quedaba sin descompilar detrás de
  `network_session_send_membership_updates` (hito 356). **Prepara el
  mensaje de la clase `0x19`**: quién está en la sesión.
- El mensaje (`0x489C` bytes) ya tiene forma conocida: lo que identifica a
  la sesión, la versión de ahora y la de antes, hasta 32 **consolas** de
  `0x104` bytes y hasta 32 **jugadores** de `0x140`, y la consola que
  dirige la sesión. Los dos trozos son estructuras en `network_session.h`.
- Con lo que había la última vez, solo va lo que cambió:
  - **Consolas.** Una consola de antes es la de ahora que se conoce igual
    (sus `0x24` bytes). La que ya no está va como «fuera»; la nueva va con
    todo lo que la sesión sabe de ella; la que sigue va solo si lo que se
    sabe de ella cambió (y entonces solo lo que cambió,
    `network_session_peer_properties_get_update`, hito 325) o si **cambió
    de sitio** en la lista.
  - **Jugadores.** Un jugador de antes es el de ahora en su mismo sitio si
    se le conoce igual, está en el mismo sitio de su consola y su consola
    es la misma. Si no, va como «fuera» (salvo que su consola ya no esté:
    se sobreentiende). El nuevo va con cómo se le conoce, su consola y su
    sitio; y él, o el que cambió, con su número, sus dos trozos de `0x90`
    bytes y su último número. Un jugador cuyo número es NONE va sin eso.
- Sin lo de la última vez, van todas las consolas y todos los jugadores.

### Batería completa y líneas rotas

- **La batería completa del hito 358 terminó: 1.299 funciones, las 1.299
  bien.** Es la segunda completa con el arnés cambiado en el hito 322.
- Las veinticinco líneas rotas de los hitos 363 y 364 se detectaron todas.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (217
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 365 están
  pendientes.

## 2026-10-08 — Preparar «lo que la sesión sabe» para mandarlo

Una función en `networking/network_session.c`, de 1.471 bytes. De 1308
funciones se pasa a 1309, el 11,62 % de las del juego (el 7,500 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_parameters_get_update` | `0x000609e0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_parameters_get_update` era `unknown_000609e0`, la función
  que `network_session_send_parameters_updates` (hito 356) llamaba sin que
  estuviera descompilada. **Prepara el mensaje de la clase `0x21`**: lo que
  identifica a la sesión, la versión de lo que sabe, y cada cosa detrás de
  un byte que dice que viene.
- Si se le da lo que la sesión sabía la última vez, pone también la versión
  de eso y **solo las cosas que no son iguales**; si no se le da, la versión
  va a NONE y van todas.
- Es el espejo exacto de `network_session_parameters_apply_update` (hito
  363): las mismas veinticinco cosas en los mismos sitios. Dos detalles:
  - con los números de `0x4980` y `0x497C` va el byte de `0x765C` de la
    sesión, que es el que **pide respuesta** (el mensaje `0x24`);
  - la cosa de `0x49CC` va como su clase y los números con que se hace esa
    clase (cuatro para la primera, uno para la segunda y la tercera).
- Los dos textos se comparan como textos: lo que haya después de su final
  no cuenta. Los bytes de relleno entre una cosa y la siguiente tampoco.
- Con esto **el camino de «lo que la sesión sabe» está entero**, de punta a
  punta: pedir el cambio (`0x22`), atenderlo (hito 362), preparar el mensaje
  (este), repartirlo (hito 356), recibirlo y aplicarlo (hito 363) y
  contestar (`0x24`).

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (216
  funciones).

## 2026-10-08 — Recibir «lo que la sesión sabe»

Dos funciones en `networking/network_session.c`, 1.295 bytes entre las dos. De 1306
funciones se pasa a 1308, el 11,61 % de las del juego (el 7,450 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_parameters_apply_update` | `0x0005d5a0` | `networking/network_session.c` |
| `network_session_parameters_update_received` | `0x0005d9e0` | `networking/network_session.c` |

### Lo que se sabe

- Es **el otro extremo del hito 356**: lo que hace una consola cuando la que
  lleva la sesión le manda «lo que la sesión sabe» (el mensaje de la clase
  `0x21`).
- `network_session_parameters_apply_update` calcula cómo queda lo que la
  sesión sabe después del mensaje, sin tocar todavía la sesión:
  - si el mensaje trae **solo lo que cambió** desde una versión que no es la
    que la consola tiene, no vale;
  - si vale, toma la versión nueva y cada una de las veinticinco cosas que
    el mensaje puede traer, cada una detrás de un byte que dice si viene.
    Algunas llevan un segundo byte que dice si la cosa «está»;
  - la cosa de `0x49CC` viene de una de cuatro maneras, y se monta con las
    cuatro funciones de `network_life_cycle.c` que ya estaban
    descompiladas (`network_life_cycle_request_new1` a `new4`);
  - como en el hito 362, la de `0x4A00` y la de `0x5DDC` tienen que ser
    correctas; si no, se dejan como estaban y la función devuelve FALSE.
- `network_session_parameters_update_received` lo aplica: si salió bien, eso
  pasa a ser lo que la sesión sabe, y si el mensaje lo pide contesta a la
  consola que lleva la sesión con un mensaje de la clase `0x24` (los dos
  números de `0x4980` y `0x497C`). Si salió mal, da por perdida a la que
  lleva la sesión (`network_session_host_lost`).
- Con esto **el mensaje `0x21` tiene nombre para todos sus trozos**
  (`NETWORK_SESSION_PARAMETERS_UPDATE_...` en `network_session.h`), que es
  lo que hará falta para descompilar quien lo prepara, `unknown_000609e0`.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (215
  funciones).

## 2026-10-08 — La consola que lleva la sesión atiende los cambios que le piden

Una función en `networking/network_session.c`, de 986 bytes. De 1305
funciones se pasa a 1306, el 11,59 % de las del juego (el 7,407 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_parameters_requested` | `0x0005dac0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_host_parameters_requested` atiende el mensaje de la
  clase `0x22`, el que la consola que dirige la sesión manda para **cambiar
  lo que la sesión sabe** (las funciones que lo mandan ya estaban
  descompiladas). Recorre las dieciséis cosas que pueden cambiar; cada una lleva
  delante un byte que dice si viene, y solo se toma si no es lo que la
  sesión ya tenía.
- Quince de las dieciséis solo valen si el mensaje viene de la consola que
  dirige la sesión. La otra, el byte de `0x49FC`, la puede cambiar
  cualquier consola de la sesión.
- Dos de ellas pueden «estar o no estar» y además tienen que ser correctas:
  la de `0x4A00` (`network_session_roster_valid`) y la de `0x5DDC`
  (`network_session_unknown_0005c190`). Si no lo son, no se toman y la
  función devuelve FALSE, pero lo demás del mensaje sí se toma.
- El idioma no usa su byte: se toma si el número que viene no es cero.
- Aparece una cosa que no se conocía: en `0x4EE0`, justo después de la de
  `0x4DB0`, la sesión guarda **un texto de hasta 31 caracteres de 16 bits**.
- Si algo cambió, sube la versión de «lo que la sesión sabe» (`0x4978`), y
  con eso `network_session_send_parameters_updates` lo reparte (hito 356).

### Comprobado rompiendo líneas

- Quince líneas rotas, una cada vez, entre este hito y el 361: las pruebas
  fallaron en trece. Las dos que se escaparon eran de este hito: **hasta
  dónde se comparan los dos textos** (con un carácter menos, las pruebas
  seguían pasando, porque ningún caso tenía dos textos iguales salvo en el
  último carácter). Se añadieron esos casos y ahora fallan las dos.
- Con esto las mutaciones del hito 361 ya no están pendientes.

### Sin comprobar

- En este hito se pasaron solo las pruebas de su función; las del archivo
  entero de `network_life_cycle` se pasan con el hito 363.
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 siguen
  pendientes.

## 2026-10-08 — La consola que lleva la sesión atiende a quien pide entrar

Tres funciones en `networking/network_session.c`, 714 bytes entre todas. De 1302
funciones se pasa a 1305, el 11,58 % de las del juego (el 7,374 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_accept_join` | `0x0005c720` | `networking/network_session.c` |
| `network_session_host_answer_join` | `0x000630f0` | `networking/network_session.c` |
| `network_session_host_join_requested` | `0x00063080` | `networking/network_session.c` |

### Lo que se sabe

- Es **el otro extremo del hito 360**: lo que hace la consola que lleva la
  sesión cuando le llega la petición de entrar (el mensaje de la clase `8`).
- `network_session_host_join_requested` saca la petición del mensaje y
  pregunta si puede entrar (`network_session_host_check_join`, hito 333).
  Si puede y la sesión tiene algo en `0x78A8`, **se lo deja a eso**: llama
  a su primera función con la dirección y la petición. Con esto se conocen
  las cuatro funciones de lo que hay en `0x78A8`: una consola pide entrar,
  una consola ha salido, la sesión se ha desconectado, y si un jugador
  puede entrar.
- `network_session_host_answer_join` es la respuesta: si no hay razón en
  contra intenta meterla; si no entra, manda a su dirección un mensaje de
  la clase `0xA` con lo que identifica a la sesión y **el motivo**: es la
  negativa.
- `network_session_host_accept_join` mete a la consola en la sesión:
  - si ya estaba (por los seis bytes de su tarjeta de red): si la petición
    es la misma de hace poco (`unknown_004cf4a8` milisegundos) no hace
    nada; si no, la saca primero y la vuelve a meter;
  - le busca canal; si no hay, motivo 7 (y motivo 5 si estaba pero sin
    canal apuntado);
  - con el byte de la petición, apunta a sus jugadores como que piden
    entrar, olvidando lo que hubieran pedido antes;
  - la añade al final de «quién está», le abre el canal
    (`unknown_00076a40`) y suben los contadores.
- Los motivos de la negativa que se conocen: 2, 3 (no lleva la sesión), 4
  (llena), 5 y 7.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (212
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353, 355 y 361 están
  pendientes.

## 2026-10-08 — Entrar en la sesión de otra consola

Dos funciones en `networking/network_session.c`, 876 bytes entre las dos. De 1300
funciones se pasa a 1302, el 11,56 % de las del juego (el 7,350 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_enter_state1` | `0x00061050` | `networking/network_session.c` |
| `network_session_join` | `0x00059e50` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_join` es **la pareja de `network_session_start`**: en vez
  de crear una sesión con esta consola sola, pide entrar en la de otra.
  Recibe catorce cosas; las importantes: lo que identifica a la sesión, su
  clave, cómo se conoce a la consola que la lleva, y los jugadores que
  entran.
  1. Si ya había una sesión con ese mismo identificador, la desconecta.
  2. La consola tiene que ser conocida en la red; se registra la clave
     (`transport_security_set_key`); se pide la dirección de la consola que
     lleva la sesión, con el puerto 1001, y **no puede ser la de esta misma
     consola** (`127.0.0.1`); y tiene que haber un canal para ella
     (`unknown_00076aa0`). Si algo falla, la sesión se queda sin
     identificador y devuelve FALSE.
  3. Deja en blanco «quién está» y «lo que la sesión sabe» (a la espera de
     que lleguen, hito 354) y prepara **la petición de entrar**: cuántos
     jugadores y cómo se conoce a cada uno, dos números de cada uno, ocho
     bytes, un byte con su número, `0x2C` bytes que pueden no darse, y cómo
     se conoce a esta consola.
- `network_session_enter_state1` pone la sesión en su primer estado con
  todo eso: el canal, la consola que lleva la sesión (que queda la primera
  de «quién está», y esta la segunda), su dirección, la petición y la hora.
  A la petición le añade ocho bytes al azar en su `0x180`. Y manda la
  petición por primera vez (`network_session_state1_send`, hito 354).
- De la petición de entrar (`0x1AC` bytes) se conocen ahora todos los
  trozos, con nombre en `network_session.h`.

### Lo que enseñó

- En una prueba comparé sin querer **una dirección de la pila**, que no es
  la misma en el juego y en nuestro código, y la prueba falló con las dos
  funciones haciendo lo mismo. Lo que sustituye a una función que recibe un
  puntero a algo de la pila tiene que devolver qué comparar (lo apuntado, o
  nada), no dejar que se comparen sus argumentos.

### Comprobado rompiendo líneas

- Once líneas rotas, una cada vez, entre las dos funciones: las pruebas
  fallaron en las once.

### Sin comprobar

- En las pruebas de `network_session_join` todo lo que llama está
  sustituido.
- En este hito se pasaron solo las pruebas de las dos funciones; las del
  archivo entero de `network_life_cycle` se pasan con el hito 361.
- La batería completa del hito 358 sigue corriendo.
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — El noveno estado: ponerse de acuerdo en quién lleva la sesión

Una función en `networking/network_session.c`, de 833 bytes. De 1299
funciones se pasa a 1300, el 11,54 % de las del juego (el 7,320 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_state9_update` | `0x00061ef0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_state9_update` es lo que la sesión repite en su noveno
  estado, al que se llega cuando **se pierde la consola que la llevaba**
  (`network_session_host_lost`): las que quedan se ponen de acuerdo en cuál
  la lleva ahora.
  1. **Dar por perdidas a las que no contestan.** Cada consola tiene un bit
     en uno de dos números: el de `0xD0` (se espera algo de ella) o el de
     `0xD4` (ya no se espera). Pasa del primero al segundo a los
     `unknown_004cf4cc` milisegundos de la hora que la sesión guarda de
     ella (una por consola, desde `0xD8`); y las que no estaban en ninguno,
     cuando el estado lleva ese tiempo.
  2. **Si la elegida es esta consola** (`0xC`): cuando cada una de las demás
     o está dada por perdida o está entre las conocidas con su bit puesto
     (lo que pone `network_session_election_set_unknownBC`, hito 347), la
     sesión pasa al octavo estado, que es donde esta consola empieza a
     llevarla (hito 355). Si hay alguna dada por perdida, antes se espera a
     que el estado lleve `unknown_004cf4cc` milisegundos y a que la elegida
     no haya cambiado en `unknown_004cf4d0`.
  3. **Si la elegida es otra y está dada por perdida**, se empieza de nuevo
     con esta consola como elegida (`network_session_election_new`).
  4. **Contarlo a las demás**: un mensaje de la clase `0x16` (`0xC8` bytes:
     lo que identifica a la sesión y lo que se sabe de las consolas
     conocidas) a cada consola, la primera vez, cada `unknown_004cf4d4`
     milisegundos, y medio segundo después del último si algo cambió desde
     entonces.
  5. A los `unknown_004cf4c8` milisegundos de empezar el estado, la sesión
     se desconecta.
- Con este hito **todos los estados de la sesión que tienen función propia
  la tienen descompilada** (el tercero y el quinto no tienen), y
  `network_session_update` ya no llama a nada sin descompilar para ninguno.
- Es el único sitio visto hasta ahora en que `unknown_00075e80` se llama
  con su primer byte a 1 para todas las consolas a la vez (la «otra manera»
  de mandar).

### Comprobado rompiendo líneas

- Doce líneas rotas, una cada vez (los cuatro tiempos, los bits de las
  consolas, cuándo se pasa al octavo estado y cuándo se manda el mensaje):
  las pruebas fallaron en las doce.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (207
  funciones).
- La batería completa del hito 358 se lanzó en la copia de comprobación;
  su resultado irá en una entrada posterior.
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — El séptimo estado: buscar otra consola que lleve la sesión

Una función en `networking/network_session.c`, de 828 bytes. De 1298
funciones se pasa a 1299, el 11,53 % de las del juego (el 7,292 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_state7_update` | `0x00061ac0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_state7_update` es lo que la sesión repite en su séptimo
  estado: la consola que la lleva **busca a otra para pasarle la sesión**
  (lo que pasa cuando el anfitrión se va y la partida sigue). Va por pasos:
  1. **Elegir.** De las consolas candidatas (los bits de `0x7424`) que ya
     tienen al día «quién está» y «lo que la sesión sabe», se queda con la
     mejor; cuál es mejor de dos lo dice `unknown_000619b0`, que aún no
     está descompilada. Si el estado no empezó con su primer byte, esta
     consola también cuenta, y si resulta la mejor no se elige a ninguna.
     Sin ese byte solo valen las consolas cuyo número de `0x9C` tiene un
     bit por cada consola de la sesión.
  2. **Avisar a todas** con un mensaje de la clase `0xF` de cuál es la
     elegida (una sola vez), y apuntar la hora.
  3. **Esperar las respuestas** (los bits de `0x7438`, que pone
     `network_session_unknown_0005e340`, hito 349). Cuando han contestado
     todas, o a los `unknown_004cf4b4` milisegundos si ha contestado la
     elegida, **se lo dice a la elegida** con un mensaje `0x11`, y la
     apunta en `0x44`.
  4. Si la elegida no contesta a tiempo (`unknown_004cf4b4`, o
     `unknown_004cf4bc` después del mensaje `0x11`), se descarta y se
     vuelve a elegir (`network_session_unknown_00061950`).
- El estado se acaba, y la sesión vuelve al quinto, cuando no quedan
  candidatas, cuando esta consola es la mejor, o a los `unknown_004cf4b8`
  milisegundos sin ninguna elegida. Si el estado empezó con su primer byte,
  además la consola se sale de la sesión.
- Encaja con lo ya descompilado: `0xF` → `0x10` (hito 352) → respuestas →
  `0x11` → `0x13` y `0x14` (hito 354).
- El número de `0x9C` de cada consola parece decir **con qué consolas
  tiene trato**; es el primero de los cuatro de `unknown74` de lo que la
  sesión sabe de ella.

### Comprobado rompiendo líneas

- Diez líneas rotas a mano, una cada vez (comparaciones de tiempo, lo que
  se apunta, lo que se manda y cómo se elige): las pruebas fallaron en las
  diez.

### Sin comprobar

- `unknown_000619b0` está sustituida en las pruebas por algo que contesta
  siempre lo mismo a la misma pregunta.
- En este hito se pasaron las pruebas de `network_life_cycle` (206
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — Lo que la sesión hace en cada vuelta

Dos funciones en `networking/network_session.c`, 590 bytes entre las dos. De 1296
funciones se pasa a 1298, el 11,52 % de las del juego (el 7,264 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_state8_update` | `0x00061e00` | `networking/network_session.c` |
| `network_session_update` | `0x0005a090` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_update` es **el paso de la sesión**: lo que hace una y
  otra vez, según su estado. Tiene una tabla de saltos (`0x0005a1f8`) con
  una entrada por estado, y con ella quedan ordenadas las piezas de los
  hitos anteriores:

| Estado | Lo que hace |
| --- | --- |
| 1 | `network_session_state1_update` (hito 354) |
| 2 | `network_session_unknown_000618d0` |
| 3 | nada propio |
| 4 | `network_session_unknown_00061910` |
| 5 | nada propio |
| 6 | `network_session_disconnect` |
| 7 | `network_session_state7_update` (`0x00061ac0`, sin descompilar) |
| 8 | `network_session_state8_update` |
| 9 | `network_session_state9_update` (`0x00061ef0`, sin descompilar) |

- Después, en todos los estados:
  - la consola que lleva la sesión olvida a los jugadores que pidieron
    entrar hace demasiado y, fuera de sus tres últimos estados, echa (con el
    mensaje `0xD`) a las consolas cuyo canal ha vuelto a su primer dato 1:
    son las que **se han caído**;
  - la que no la lleva mira lo mismo del canal de la que la lleva, y si se
    ha caído hace `network_session_host_lost`; si sigue en la sesión, hace
    lo del tercer estado (hito 345);
  - `network_session_unknown_00062240` (hito 355);
  - y la que lleva la sesión reparte «quién está» y «lo que la sesión sabe»
    (hito 356).
- `network_session_state8_update`: en el octavo estado la sesión espera a
  que todas las consolas contesten, o a que pasen `unknown_004cf4c0`
  milisegundos. Entonces vuelve al quinto estado, echa a las que no
  contestaron (o contestaron lo que pone su bit en `0x14`), reparte las dos
  cosas, y según los dos bytes con que empezó el estado se sale de la
  sesión o pasa al séptimo.
- Las funciones de los estados séptimo y noveno quedan apuntadas con su
  nombre, aún sin descompilar.

### Corregido

- `network_session_send_unknown12` (hito 355) es solo del octavo estado: la
  llaman `network_session_enter_state8` y `network_session_state8_update`.
  En el hito 355 puse «séptimo y octavo» sin haberlo visto; está corregido
  en el código y en esa entrada.

### Comprobado rompiendo líneas

- Cinco líneas rotas a mano, una cada vez: las pruebas fallaron en las
  cinco.

### Sin comprobar

- En las pruebas de `network_session_update` todo lo que llama está
  sustituido; lo sustituido cambia a veces el estado de la sesión, para
  comprobar que la función vuelve a leerlo donde el juego lo hace.
- En este hito se pasaron las pruebas de `network_life_cycle` (205
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — Repartir «quién está» y «lo que la sesión sabe»

Dos funciones en `networking/network_session.c`, 845 bytes entre las dos. De 1294
funciones se pasa a 1296, el 11,50 % de las del juego (el 7,245 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_send_membership_updates` | `0x00062640` | `networking/network_session.c` |
| `network_session_send_parameters_updates` | `0x000627e0` | `networking/network_session.c` |

### Lo que se sabe

- Son dos funciones gemelas: con ellas la consola que lleva la sesión
  **mantiene al día a las demás**. Una reparte «quién está» (mensaje de la
  clase `0x19`, `0x489C` bytes) y la otra «lo que la sesión sabe» (clase
  `0x21`, `0x14D8` bytes).
- Aquí se ve para qué sirven dos cosas que estaban sin explicar:
  - el número con que empieza cada una de esas dos cosas (`0x4C` y
    `0x4978`) es **su versión**: sube cada vez que algo cambia (son los
    «contadores» de los hitos anteriores);
  - de cada consola la sesión guarda qué versión tiene ya de cada cosa (los
    números de `0x8` y `0xC` de lo que guarda de ella), y la copia de
    `0x24E0` y `0x5E28` es **lo último que se repartió**.
- Con eso decide qué mandar a cada consola: nada si ya tiene la versión de
  ahora; **solo lo que cambió** si tiene la última repartida; **todo** si
  tiene otra o ninguna. Lo que se manda lo preparan `unknown_00060400` y
  `unknown_000609e0`, que aún no están descompiladas.
- No se manda a una consola cuyo canal todavía tiene por enviar un mensaje
  de la misma clase, ni en el octavo estado.

### Comprobado rompiendo líneas

- Cuatro líneas rotas a mano, una cada vez: las pruebas fallaron en las
  cuatro (cada gemela en lo suyo).

### Sin comprobar

- Las dos reservan en la pila dos mensajes enteros (`0x914C` y `0x29C4`
  bytes); en las pruebas lo que los rellena está sustituido.
- En este hito se pasaron las pruebas de `network_life_cycle` (203
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes.

## 2026-10-08 — El octavo estado de la sesión y los canales de las consolas

Tres funciones en `networking/network_session.c`, 734 bytes entre todas. De 1291
funciones se pasa a 1294, el 11,49 % de las del juego (el 7,216 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_00062240` | `0x00062240` | `networking/network_session.c` |
| `network_session_send_unknown12` | `0x00062550` | `networking/network_session.c` |
| `network_session_enter_state8` | `0x00061570` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_enter_state8`: **la sesión entra en su octavo estado**.
  Lo que guarda desde su estado empieza con un byte que le dan, la consola
  que la llevaba, la hora y el bit de esta consola en dos sitios. Si venía
  de un estado que no es de los cuatro últimos, olvida a los jugadores que
  pedían entrar y esta consola pasa a ser la de `0x40`. Con una sola
  consola no hay octavo estado: va al quinto.
- `network_session_send_unknown12`: en el octavo estado, manda
  un mensaje de la clase `0x12` a cada consola con canal listo (su primer
  dato vale 7) que aún no lo haya recibido. A la consola que llevaba la
  sesión solo se le manda cuando todas las demás ya están apuntadas, o
  cuando han pasado `unknown_004cf4c4` milisegundos.
- `network_session_unknown_00062240` recorre las consolas y hace
  `unknown_00076a40` con el canal de cada una, si el primer dato del canal
  no es 1. Parece lo que **abre el canal** con las consolas que aún no lo
  tienen; la consola que no lleva la sesión solo lo hace con las que ya
  dijeron lo que la sesión debe saber de ellas (el byte `0x24`, hito 351).
- De lo que la sesión guarda en el octavo estado se conocen ya seis datos,
  con nombre en `network_session.h`.

### Sin comprobar

- `unknown_00076a40` es del observador de la red y no está descompilada.
- En este hito se pasaron las pruebas de `network_life_cycle` (201
  funciones).
- Las mutaciones de los hitos 280 a 308, 321 a 353 y 355 están pendientes
  (del 354 se rompieron cuatro líneas a mano).

## 2026-10-08 — El primer estado de la sesión, y cuando la lleva otra consola

Tres funciones en `networking/network_session.c`, 918 bytes entre todas. De 1288
funciones se pasa a 1291, el 11,46 % de las del juego (el 7,191 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_0005e3f0` | `0x0005e3f0` | `networking/network_session.c` |
| `network_session_state1_send` | `0x00062300` | `networking/network_session.c` |
| `network_session_state1_update` | `0x000617c0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_state1_update` es **lo que la sesión repite en su primer
  estado**, el de quien está entrando en una sesión de otra consola:
  - mientras `unknown_00078580` no diga 2 del número que la sesión tiene en
    `0x7420`, espera; si pasa más de `unknown_004cf498` milisegundos desde
    que entró en el estado, se sale de la sesión;
  - cuando dice 2, apunta la hora (`0x760C`) la primera vez. Si ya le llegó
    «quién está» y «lo que la sesión sabe» (los dos empiezan en NONE), pasa
    al tercer estado y manda un mensaje de la clase `0x15` a la consola que
    dirige la sesión. Si no le llegan en `unknown_004cf49c` milisegundos,
    se sale.
- `network_session_state1_send` es lo que manda mientras tanto a la
  dirección que guarda: un mensaje de la clase `8`, de `0x1B8` bytes, con un
  2, lo que identifica a la sesión y los `0x1AC` bytes de `0x745C`. No más
  de una vez cada `unknown_004cf494` milisegundos; cuenta las veces en
  `0x7610`. Es **la petición de entrar**.
- `network_session_unknown_0005e3f0`: llega algo de la dirección de otra
  consola de la sesión, y esa consola **pasa a ser la que la lleva** (la de
  `0x40`). Vale siempre en el noveno estado; en los demás, si es la que la
  sesión tenía apuntada en `0x44` (hito 352) o si la que la llevaba ya no
  responde (`unknown_000769a0`). Si esta consola era la que la llevaba, se
  lo dice a las demás con un mensaje `0x13` y olvida a los jugadores que
  pedían entrar. Después contesta a la nueva con un mensaje `0x14`.
- Con esto se ve la cadena entera de cuando cambia quien lleva la sesión:
  `0x10` (hito 352) → `0x13` → `0x14`.
- Tres tiempos más con nombre provisional: `unknown_004cf494`,
  `unknown_004cf498` y `unknown_004cf49c`.

### Comprobado rompiendo líneas

- Cuatro líneas rotas a mano, una cada vez (un `>` por `>=`, una clase de
  mensaje, un argumento y un dato del mensaje): las pruebas fallaron las
  cuatro veces.

### Sin comprobar

- `unknown_00078580` y `unknown_000769a0` son del observador de la red y
  no están descompiladas: aquí solo se sabe cómo se las llama.
- En este hito se pasaron las pruebas de `network_life_cycle` (198
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 353 están pendientes.

## 2026-10-08 — Lo que esta consola le dice a la que lleva la sesión

Seis funciones en `networking/network_session.c`, 1.581 bytes entre todas. De 1282
funciones se pasa a 1288, el 11,43 % de las del juego (el 7,161 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_set_local_peer_properties` | `0x0005a790` | `networking/network_session.c` |
| `network_session_add_local_player` | `0x0005a880` | `networking/network_session.c` |
| `network_session_set_local_player` | `0x0005a9c0` | `networking/network_session.c` |
| `network_session_remove_local_player` | `0x0005aae0` | `networking/network_session.c` |
| `network_session_change_leader` | `0x0005aba0` | `networking/network_session.c` |
| `network_session_leader_remove_peer` | `0x0005acc0` | `networking/network_session.c` |

### Lo que se sabe

- Son **el otro extremo** de seis avisos ya descompilados: lo que hace la
  consola que quiere el cambio. Las seis siguen el mismo dibujo: si esta
  consola lleva la sesión, hace el cambio ella misma; si no, se lo manda a
  la que la lleva (la de `0x40`), que lo atiende con la función pareja.

| Lo que pide | Mensaje | Quien lo atiende |
| --- | --- | --- |
| `network_session_set_local_peer_properties` | `0x1A`, `0xD0` bytes | `network_session_host_set_peer_properties` (hito 351) |
| `network_session_change_leader` | `0x1B`, `0x2C` bytes | `network_session_host_change_leader` (hito 350) |
| `network_session_leader_remove_peer` | `0x1C`, `0x2C` bytes | `network_session_host_leader_removes_peer` (hito 350) |
| `network_session_add_local_player` | `0x1D`, `0xB0` bytes | `network_session_host_add_peer_player` (hito 351) |
| `network_session_remove_local_player` | `0x1F`, `0xC` bytes | `network_session_host_remove_peer_player` (hito 337) |
| `network_session_set_local_player` | `0x20`, `0xA4` bytes | `network_session_host_set_peer_player` (hito 351) |

- Con esto las clases de mensaje `0x1A` a `0x20` tienen nombre, y los
  tamaños y los sitios de cada dato coinciden en los dos extremos.
- Las dos últimas de la lista solo las hace la consola que dirige la
  sesión (la que «quién está» dice), y no valen para sí misma.
- Quedaron dos trozos que el juego repite en todas, escritos una vez:
  `network_session_send_to_host` y `network_session_membership_changed`.

### Sin comprobar

- `network_session_set_local_player` y `network_session_remove_local_player`
  no miran que el sitio esté entre 0 y 3; en las pruebas siempre lo está.
- En la entrada del hito 351 se corrigió una referencia:
  `network_session_add_peer_player` es del hito 324, no del 327.
- En este hito se pasaron las pruebas de `network_life_cycle` (195
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 353 están pendientes.

## 2026-10-08 — La hora de la sesión, y dos avisos más

Tres funciones en `networking/network_session.c`, 881 bytes entre todas. De 1279
funciones se pasa a 1282, el 11,38 % de las del juego (el 7,107 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_synchronize_time` | `0x0005e030` | `networking/network_session.c` |
| `network_session_unknown_0005e210` | `0x0005e210` | `networking/network_session.c` |
| `network_session_unknown_0005e6b0` | `0x0005e6b0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_synchronize_time` atiende el mensaje de la clase `0x18`,
  el que una sesión en su tercer estado manda una y otra vez a la consola
  que la lleva (hito 345). Ahora se ve para qué: **es para poner en hora**.
  - La consola que lleva la sesión, si el mensaje pregunta (sus 16 bits
    valen 0) y viene de otra consola de la sesión, lo devuelve tal cual a la
    dirección de la que vino, con los 16 bits a 1.
  - La otra consola, cuando recibe la respuesta de la que lleva la sesión,
    calcula la mitad de (tercer número − primero) + (cuarto − segundo) y lo
    guarda en `0x78B0`: es la cuenta de siempre para saber cuánto va
    adelantado un reloj respecto de otro con cuatro horas apuntadas. Pone a
    1 el byte de `0x78AC` y apunta la hora en `0x7654`.
  - Que los cuatro números sean horas es lo que la cuenta da a entender; no
    se ha visto todavía quién los rellena.
- `network_session_unknown_0005e210`: a la consola le hablan de una consola
  de la sesión (cuál es y cómo se la conoce). Si la sesión no dice quién
  está en ella, se sale de la sesión. Si la consola es de la sesión y no es
  la de `0x40`, se apunta en `0x44` y se le pasa lo mismo a la de `0x40` con
  un mensaje de la clase `0x10`. Es el primer salto del aviso que atiende
  `network_session_unknown_0005e340` (hito 349).
- `network_session_unknown_0005e6b0`: llega algo por un canal. La consola
  que lleva la sesión solo pone a 0 el cuarto byte de lo que guarda de quien
  lo manda. Cualquier otra **contesta con un mensaje de la clase `0x13`**:
  lo que identifica a la sesión, si el canal es de una consola de la sesión
  y, si lo sabe, cómo se conoce a la consola de `0x40`. Parece la manera de
  decir «yo no llevo esta sesión, la lleva esta otra».

### Sin comprobar

- En `network_session_unknown_0005e6b0` el juego busca la consola del canal
  con el bucle escrito dentro; aquí se llama a
  `network_session_find_peer_by_channel`, que hace lo mismo.
- En este hito se pasaron las pruebas de `network_life_cycle` (189
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 352 están pendientes.

## 2026-10-08 — Lo que una consola dice de sí misma y de sus jugadores

Tres funciones en `networking/network_session.c`, 837 bytes entre todas. De 1276
funciones se pasa a 1279, el 11,35 % de las del juego (el 7,078 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_set_peer_properties` | `0x0005ed90` | `networking/network_session.c` |
| `network_session_host_add_peer_player` | `0x0005efd0` | `networking/network_session.c` |
| `network_session_host_set_peer_player` | `0x0005f230` | `networking/network_session.c` |

### Lo que se sabe

- Las tres son avisos que otra consola de la sesión le manda a la que la
  lleva. Solo valen si esta consola lleva la sesión y el aviso viene de
  otra consola que está en ella.
- `network_session_host_set_peer_properties`: la consola dice **lo que la
  sesión debe saber de ella** (los dos textos y los números, `0xC8` bytes
  desde el `0x28` de la consola). Se pone a 1 el byte `0x24` de la consola,
  y si algo cambió suben los dos contadores.
- `network_session_host_add_peer_player`: la consola dice que **entra un
  jugador** en uno de sus cuatro sitios. Si entra
  (`network_session_add_peer_player`, hito 324), se guardan su número, sus
  primeros `0x90` bytes y el número del final (`0x138`). Si no entra, la
  consola recibe un mensaje de la clase `0x1E` con lo que identifica a la
  sesión, el sitio y lo que identifica al jugador: es la **negativa**.
- `network_session_host_set_peer_player`: la consola dice **cómo es ahora
  el jugador** de uno de sus sitios: las mismas tres cosas, y los contadores
  suben si alguna era distinta.
- Del jugador se conoce ahora un número más, el de `0x138`, que es lo
  último que tiene.

### Sin comprobar

- En `network_session_host_add_peer_player` el juego no mira que el sitio
  esté entre 0 y 3 antes de llamar a `network_session_add_peer_player`; en
  las pruebas esa función está sustituida y el sitio siempre es válido.
- En este hito se pasaron las pruebas de `network_life_cycle` (186
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 351 están pendientes.

## 2026-10-08 — Cuatro avisos más que atiende la sesión

Cuatro funciones en `networking/network_session.c`, 674 bytes entre todas. De 1272
funciones se pasa a 1276, el 11,33 % de las del juego (el 7,050 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_0005ece0` | `0x0005ece0` | `networking/network_session.c` |
| `network_session_host_change_leader` | `0x0005ee70` | `networking/network_session.c` |
| `network_session_host_leader_removes_peer` | `0x0005ef20` | `networking/network_session.c` |
| `network_session_unknown_0005f3c0` | `0x0005f3c0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_host_change_leader`: la consola que dirige la sesión le
  dice a la que la lleva que **pasa a dirigirla otra**; «quién está» lo
  apunta y suben los contadores.
- `network_session_host_leader_removes_peer`: la que dirige le pide a la
  que la lleva que **eche a una consola**. Si es otra, sale con el mensaje
  `0xE` (hito 321); si es la propia consola que lleva la sesión, la sesión
  pasa al décimo estado con un 1.
- Las dos solo valen si el aviso viene de la consola que dirige la sesión,
  que no puede ser esta, y si la consola de la que habla está en la sesión.
- `network_session_unknown_0005ece0`: en el noveno estado, llega algo de la
  dirección de otra consola: pierde su bit en un número, lo gana en otro y
  lo pierde en la lista de consolas (hito 347).
- `network_session_unknown_0005f3c0` dice algo de la consola de un canal.

### Lo que enseñó

- En `network_session_unknown_0005ece0` leí mal una etiqueta del final:
  cuando lo de dentro no cambia nada el juego **también devuelve TRUE**, y
  solo se salta apuntar la hora. La prueba lo cazó a la primera. Antes de
  dar con ello sospeché del arnés y del compilador; no era ninguno. Para
  mirar nuestro código compilado quedó un guion pequeño que desensambla una
  función de la imagen de las pruebas.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (183
  funciones).
- La batería completa del hito 343 terminó después: 1.253 funciones, las
  1.253 bien. Es la primera completa con el arnés cambiado en el hito 322.
- Las mutaciones de los hitos 280 a 308 y 321 a 350 están pendientes.

## 2026-10-08 — Sacar a una consola de otra manera, y aplicar lo que cambió

Tres funciones en `networking/network_session.c`, 557 bytes entre todas. De 1269
funciones se pasa a 1272, el 11,29 % de las del juego (el 7,027 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_remove_peer_with_message_c` | `0x0005c9e0` | `networking/network_session.c` |
| `network_session_peer_properties_apply_update` | `0x0005cac0` | `networking/network_session.c` |
| `network_session_unknown_0005e340` | `0x0005e340` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_remove_peer_with_message_c` es la tercera manera de
  sacar a una consola (las de `0xD` y `0xE` son del hito 321): se le manda
  algo de la clase `0xC`, sale, y se avisa a lo que la sesión tiene en
  `0x78A8` (su **segunda** función, con los dos números con que la consola
  entró). En los tres últimos estados no se la saca: en el séptimo y el
  octavo solo se apunta.
- `network_session_peer_properties_apply_update` es la pareja de
  `network_session_peer_properties_get_update` (hito 325): pone en lo que
  se sabe de una consola las partes que el mensaje dice que han cambiado.
- `network_session_unknown_0005e340`: en el séptimo estado, una consola
  dice de qué consola habla (cuál es y cómo se la conoce); si es la que la
  sesión tiene apuntada, quien lo dice recibe su bit en `0x7438`.
- De lo que la sesión tiene en `0x78A8` se conocen ya tres de sus cuatro
  funciones: una consola ha salido, la sesión se ha desconectado, y si un
  jugador puede entrar.

### Sin comprobar

- Los dos números que recibe la segunda función de `0x78A8` van en la pila
  y el arnés no deja mirarlos desde ahí: solo se comprueba que se la llama.
- En este hito se pasaron las pruebas de `network_life_cycle` (179
  funciones).
- La batería completa del hito 343 seguía corriendo.
- Las mutaciones de los hitos 280 a 308 y 321 a 349 están pendientes.

## 2026-10-08 — El mapa de la sesión de la partida

Tres funciones en `networking/network_life_cycle.c`, 247 bytes entre todas. De 1266
funciones se pasa a 1269, el 11,26 % de las del juego (el 7,008 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_set_map` | `0x00063f50` | `networking/network_life_cycle.c` |
| `network_life_cycle_set_unknown49a4` | `0x00063fb0` | `networking/network_life_cycle.c` |
| `network_life_cycle_set_map_and_unknown49a8` | `0x00063ff0` | `networking/network_life_cycle.c` |

### Lo que se sabe

- `network_life_cycle_set_map` era `unknown_00063f50`, la que `main_game.c`
  llama al cargar un mapa («si otra cosa arranca la partida en el mapa»,
  hito 190). Le pasa la campaña, el mapa y **el nombre del mapa**, y eso
  es lo que la sesión guarda en `0x4D08`, `0x4D0C` y el texto de `0x4D10`:
  la función del hito 340 pasa a llamarse `network_session_leader_set_map`.
  Si esta consola no dirige la sesión devuelve TRUE sin hacer nada, y el
  juego carga el mapa por su cuenta.
- `network_life_cycle_set_unknown49a4` pone el número de `0x49A4` de la
  sesión, y `network_life_cycle_set_map_and_unknown49a8` pone el mapa y
  después hace `network_session_unknown_0005bf10` con todo a cero; las dos
  solo si esta consola lleva la sesión y su número de `0x4980` es 1.
- La segunda recibe su argumento en `esi`, que ella misma no toca: lo deja
  pasar a la función de la sesión a la que salta.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (176) y de
  `main_game` (23).
- La batería completa del hito 343 seguía corriendo.
- Las mutaciones de los hitos 280 a 308 y 321 a 348 están pendientes.

## 2026-10-08 — Dos más del noveno estado

Dos funciones en `networking/network_session.c`, 248 bytes entre las dos. De 1264
funciones se pasa a 1266, el 11,24 % de las del juego (el 7,000 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_election_add_other` | `0x00062c20` | `networking/network_session.c` |
| `network_session_election_set_unknownBC` | `0x00062ca0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_election_add_other`: de dos consolas, una tiene que ser
  esta; la otra recibe su bit en el primero de los dos números de la lista
  del noveno estado (hito 346), y entra en la lista si no estaba. No vale
  si ya lo tenía.
- `network_session_election_set_unknownBC` pone o quita el bit de una
  consola en el segundo número, y dice si eso cambió algo.
- El primer número parece «con quién puedo hablar» (se marca por parejas,
  yo y otra) y el segundo algo que cada consola da o retira.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (173
  funciones).
- La batería completa del hito 343 seguía corriendo.
- Las mutaciones de los hitos 280 a 308 y 321 a 347 están pendientes.

## 2026-10-08 — Lo que la sesión guarda cuando se queda sin quien la lleva

Cinco funciones en `networking/network_session.c`, 648 bytes entre todas. De 1259
funciones se pasa a 1264, el 11,22 % de las del juego (el 6,992 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_set_unknown49a8` | `0x0005df30` | `networking/network_session.c` |
| `network_session_election_new` | `0x00062b70` | `networking/network_session.c` |
| `network_session_election_find` | `0x00063ba0` | `networking/network_session.c` |
| `network_session_election_add_unknownB8` | `0x00063c00` | `networking/network_session.c` |
| `network_session_election_add_unknownBC` | `0x00063c50` | `networking/network_session.c` |

### Lo que se sabe

- En el noveno estado (hito 344), tras las horas, la sesión tiene 192 bytes
  que ahora tienen estructura (`network_session_election`, nombre
  conjeturado): cómo se conoce a la consola que la llevaba y a esta, el
  contador de «quién está», qué consola es esta y un número suyo, y **la
  lista de consolas de las que se sabe**, cada una por seis bytes (los de
  su tarjeta de red), con un bit por consola en dos números.
  `network_session_election_new` la empieza con esta consola sola; las
  otras tres buscan una consola en la lista o la añaden marcando su bit en
  uno u otro número. Son las que quedaron sin hacer en el hito 335 por no
  saber de qué eran.
- `network_session_host_set_unknown49a8` era `unknown_0005df30`, a la que
  llegaban los mensajes de la clase `0x23` (hitos 337 y 341): pone un byte,
  tres números y doce bytes de «lo que la sesión sabe» (`0x49A8` a
  `0x49C4`). El byte solo lo cambia la consola que dirige la sesión; los
  doce bytes (lo que mide lo que identifica a un jugador) solo se guardan
  con un 1 en el segundo número.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (171
  funciones).
- La batería completa del hito 343 seguía corriendo.
- Las mutaciones de los hitos 280 a 308 y 321 a 346 están pendientes.

## 2026-10-08 — El segundo estado de la sesión y lo que repite en el tercero

Dos funciones en `networking/network_session.c` y una en `networking/network_observer.c`,
624 bytes entre todas. De 1256 funciones se pasa a 1259, el 11,18 % de las del
juego (el 6,970 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_enter_state2` | `0x00061180` | `networking/network_session.c` |
| `network_session_state3_update` | `0x00062990` | `networking/network_session.c` |
| `network_time_since` | `0x00075890` | `networking/network_observer.c` |

### Lo que se sabe

- `network_session_enter_state2` (era `unknown_00061180`) es salir de una
  sesión en la que se estaba **entrando**: lo que la sesión tenía a partir
  del estado se conserva, recolocado (la dirección de quien la lleva pasa
  de `0x7448` a `0x7444`, que es lo que se veía en el hito 332), con la
  hora; lo demás se vacía, y se manda el primer aviso
  (`network_session_unknown_000623e0`, hito 344).
- `network_session_state3_update` (era `unknown_00062990`): en el tercer
  estado la sesión manda a quien la lleva algo de la clase `0x18`, con sus
  ocho bytes y cuatro NONE, respetando dos esperas (`unknown_004cf4d8` y
  `unknown_004cf4dc`).
- `network_time_since` dice cuántos milisegundos hace de una hora. Es lo
  mismo que la sesión tiene metido dentro en muchos sitios; aquí el juego
  lo tiene como función.
- Con esto los estados de la sesión quedan así: 1 entrando, 2 dejando de
  entrar, 3 dentro, 4 saliendo, 5 llevándola, 6 deshaciéndola, 7 y 8 otros
  dos de quien la lleva, 9 sin quien la llevaba, 10 acabada.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (166) y de
  `network_observer` (7).
- La batería completa del hito 343 seguía corriendo.
- Las mutaciones de los hitos 280 a 308 y 321 a 345 están pendientes.

## 2026-10-08 — Lo que la sesión repite, y su noveno estado

Tres funciones en `networking/network_session.c`, 578 bytes entre todas. De 1253
funciones se pasa a 1256, el 11,15 % de las del juego (el 6,959 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_enter_state9` | `0x000616e0` | `networking/network_session.c` |
| `network_session_unknown_000623e0` | `0x000623e0` | `networking/network_session.c` |
| `network_session_state4_update` | `0x00062480` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_state4_update` (era `unknown_00062480`): en el cuarto
  estado, la sesión manda a quien la lleva algo de la clase `0xB` con sus
  ocho bytes, la primera vez y luego cada `unknown_004cf4b0` milisegundos.
  El cuarto estado es **salir avisando**, y eso es el aviso, repetido hasta
  que contesten o se agote el tiempo (hito 336).
- `network_session_unknown_000623e0`: lo mismo hacia la dirección que la
  sesión guarda en su segundo estado, con algo de la clase 9 (los ocho
  bytes y dos números), cada `unknown_004cf4a0` milisegundos. No va por un
  canal sino directo a la dirección (`0x0007b140`).
- `network_session_enter_state9` (era `unknown_000616e0`) pone el noveno
  estado, el de cuando **se ha perdido a quien llevaba la sesión** (hito
  343): cuándo empezó, una cosa que rellena `0x00062b70`, un bit de esta
  consola y otro de la que la llevaba. Es donde las consolas que quedan
  eligen quién la lleva ahora.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (164
  funciones).
- La batería completa del hito 343 estaba corriendo al cerrar este hito.
- Las mutaciones de los hitos 280 a 308 y 321 a 344 están pendientes.

## 2026-10-08 — Desconectar la sesión, y su séptimo estado

Tres funciones en `networking/network_session.c`, 561 bytes entre todas. De 1250
funciones se pasa a 1253, el 11,12 % de las del juego (el 6,940 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_disconnect` | `0x0005a520` | `networking/network_session.c` |
| `network_session_enter_state7` | `0x000614a0` | `networking/network_session.c` |
| `network_session_host_lost` | `0x00062ab0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_disconnect` era `unknown_0005a520`, la que aparecía por
  todas partes como «lo que se hace con una sesión»: la **desconecta**.
  Primero esta consola sale (`network_session_leave`) y se avisa a lo que
  la sesión tiene en `0x78A8` (su tercera función); después no queda nada
  de ninguna consola, la sesión no se conoce por nada, nadie está ni pide
  entrar, y se queda sin estado, como recién preparada.
- `network_session_enter_state7` (era `unknown_000614a0`) pone el séptimo
  estado: lo que la sesión tiene a partir del estado empieza con un byte,
  un bit por cada consola a la que afecta (una, o todas menos esta), la
  hora y NONE. Con una sola consola no hay séptimo estado.
- `network_session_host_lost` (era `unknown_00062ab0`) es lo que pasa
  cuando se pierde a quien lleva la sesión: casi siempre se desconecta;
  solo en una sesión que lleva otra consola y con más de una consola se
  avisa al canal y se hace `0x000616e0`, que será buscar quién la lleva
  ahora.

### Mutaciones

Las de los hitos 309 a 320 terminaron: `voice_routing` 40 de 40,
`transport_security` 19 de 19 y `network_session` 72 de 73. La que escapa
es quitar el «mientras quepan» de `network_session_request_players`, y es
equivalente: cuando no queda sitio libre ninguna petición posterior entra,
así que seguir o parar da lo mismo.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (161) y de
  `game_voice` (71).
- Las mutaciones de los hitos 280 a 308 y 321 a 343 están pendientes.

## 2026-10-08 — Salir de una sesión

Tres funciones en `networking/network_session.c`, 588 bytes entre todas. De 1247
funciones se pasa a 1250, el 11,10 % de las del juego (el 6,922 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_set_unknown4980` | `0x0005a220` | `networking/network_session.c` |
| `network_session_leave` | `0x0005a400` | `networking/network_session.c` |
| `network_session_request_unknown49fc` | `0x0005a6e0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_leave` era `unknown_0005a400`, a la que llama la partida
  (hito 288): **salir de la sesión**, según su estado. Es una tabla de
  saltos de diez casillas:
  - sin estado, o si `network_session_unknown_00058d90` dice que sí (los
    estados 2, 4 y 6, que son los de «estoy saliendo»), o con el byte de
    `0x48`, nada;
  - en el **primero** (entrando), `0x00061180`;
  - en el **tercero**, pasa al cuarto;
  - en el **quinto** y el **séptimo** (llevando la sesión): con el
    argumento pasa al sexto, que avisa a todas; sin él, `0x000614a0`;
  - en el **octavo**, solo pone un byte;
  - en el **noveno**, con el argumento manda a las demás algo de la clase
    `0x17`; sin él, `0x0005a520`;
  - en el **décimo**, `0x0005a520` si no lleva el argumento.
  Se ve así que los estados pares 2, 4 y 6 son los de salida de los impares
  1, 3 y 5.
- `network_session_host_set_unknown4980` cambia el número de `0x4980` (el
  que se compara con las respuestas de las consolas, hito 323): al
  cambiarlo, la sesión se pone a **esperar** a que contesten todas las
  consolas con las que habla, y suben el número de `0x497C` y el contador.
  Es lo que `network_session_leader_unknown_0005b1e0` pide (hito 338).
- `network_session_request_unknown49fc` pone el byte de `0x49FC`, o lo pide
  a quien lleva la sesión (otro sitio del mensaje de petición: `0x30`).

### Sin comprobar

- Con un estado mayor que diez el juego salta a donde diga lo que haya
  detrás de la tabla; aquí no se hace nada. No se prueba.
- En este hito se pasaron las pruebas de `network_life_cycle` (158
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 342 están pendientes; las
  de los hitos 314 a 320 seguían corriendo.

## 2026-10-08 — La petición de la clase 0x23 y la última del molde

Dos funciones en `networking/network_session.c`, 633 bytes entre las dos. De 1245
funciones se pasa a 1247, el 11,07 % de las del juego (el 6,908 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_0005bf10` | `0x0005bf10` | `networking/network_session.c` |
| `network_session_leader_set_unknown5ddc` | `0x0005c010` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_unknown_0005bf10` es **la que manda** lo que
  `network_session_host_unknown_0005dea0` recibe (hito 337): un mensaje de
  la clase `0x23` y 32 bytes, con los ocho bytes de la sesión, un byte, dos
  números y, si el segundo número es 1, doce bytes más (lo que mide lo que
  identifica a un jugador). Si esta consola lleva la sesión no manda nada:
  hace directamente lo mismo que haría al recibirlo (`0x0005df30`, para sí
  misma). Las dos puntas del mensaje cuadran campo a campo.
- `network_session_leader_set_unknown5ddc` es la última del molde de los
  hitos 338 a 340: pone el bloque de 68 bytes de `0x5DDC` o dice que no lo
  hay, y no lo admite si lleva por identificador el de esta misma sesión.
  Con ella se conocen catorce sitios del mensaje de petición.

### Mutaciones

Las de los hitos 309 a 313 (`voice_routing`) terminaron: **40 de 40
cazadas**. Con las 19 de 19 de `transport_security` (hito 316), quedan
corriendo las 73 de la sesión (hitos 314 a 320).

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (155
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 341 están pendientes.

## 2026-10-08 — Tres peticiones grandes de quien dirige la sesión

Tres funciones en `networking/network_session.c`, 976 bytes entre todas. De 1242
funciones se pasa a 1245, el 11,05 % de las del juego (el 6,887 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_leader_set_roster` | `0x0005b650` | `networking/network_session.c` |
| `network_session_leader_set_unknown4d08` | `0x0005b790` | `networking/network_session.c` |
| `network_session_leader_set_unknown4db0` | `0x0005b9d0` | `networking/network_session.c` |

Del mismo molde que los hitos 338 y 339, con datos más grandes.

### Lo que se sabe

- `network_session_leader_set_roster` pone la lista de grupos y jugadores
  (hito 334) o dice que no hay; antes comprueba que la lista esté bien.
- `network_session_leader_set_unknown4d08` pone los dos números de
  `0x4D08` y lo de `0x4D10`, que resulta ser un **texto** de hasta 127
  caracteres (se copia con `strncpy` y se le pone el cero final).
- `network_session_leader_set_unknown4db0` pone un bloque de 304 bytes en
  `0x4DB0` (nuevo), que solo cuenta si el número de su `0x44` no es cero:
  si no, va todo a ceros.
- Del mensaje de petición se conocen ya trece sitios; el mayor es la lista,
  de 776 bytes, en `0x3C`.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (153
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 340 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Cuatro peticiones más de quien dirige la sesión

Cuatro funciones en `networking/network_session.c`, 902 bytes entre todas. De 1238
funciones se pasa a 1242, el 11,03 % de las del juego (el 6,854 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_leader_set_unknown49a1` | `0x0005bb20` | `networking/network_session.c` |
| `network_session_leader_set_unknown5dd0` | `0x0005bc10` | `networking/network_session.c` |
| `network_session_leader_set_language` | `0x0005bd00` | `networking/network_session.c` |
| `network_session_leader_set_unknown498c` | `0x0005bde0` | `networking/network_session.c` |

Son del molde del hito 338: la consola que dirige la sesión cambia un dato
si además la lleva, y si no se lo pide a la que la lleva.

### Lo que se sabe

- `network_session_leader_set_language` cambia el **idioma** de la sesión
  (`0x4988`).
- `network_session_leader_set_unknown498c` cambia el número de `0x498C`,
  el que tiene que ser cero para que la sesión admita jugadores (hito 335)
  y no pasar de 1 para que alguien pueda entrar (hito 333): es **lo
  cerrada que está** la sesión.
- `network_session_leader_set_unknown5dd0` cambia los 16 bits de `0x5DD0`,
  que tienen que ser NONE para que la sesión admita jugadores.
- `network_session_leader_set_unknown49a1` cambia un byte (`0x49A1`).
- Del mensaje de petición (clase `0x22`) se conocen ya diez sitios.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (150
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 339 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Pedir un cambio a quien lleva la sesión

Seis funciones en `networking/network_session.c`, 1.325 bytes entre todas. De 1232
funciones se pasa a 1238, el 10,99 % de las del juego (el 6,824 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_leader_unknown_0005b1e0` | `0x0005b1e0` | `networking/network_session.c` |
| `network_session_leader_set_unknown49a4` | `0x0005b2c0` | `networking/network_session.c` |
| `network_session_leader_set_unknown49c4` | `0x0005b3a0` | `networking/network_session.c` |
| `network_session_leader_set_unknown49c8` | `0x0005b490` | `networking/network_session.c` |
| `network_session_leader_clear_unknown49f8` | `0x0005b570` | `networking/network_session.c` |
| `network_session_leader_set_unknown4dac` | `0x0005b8e0` | `networking/network_session.c` |

### Lo que se sabe

- En una sesión hay **dos consolas que mandan** y pueden no ser la misma:
  la que «quién está en ella» dice que la lleva (`0x50`: la llamo la que la
  **dirige**) y la que la lleva de verdad en la red (`0x40`, estados 5 a
  8). Estas seis funciones son de la primera: si además la lleva de verdad,
  cambia el dato y sube el contador (como en el hito 332); si no, **se lo
  pide** a la otra.
- La petición es un mensaje de la clase `0x22` y 1.436 bytes: los ocho
  bytes por los que se conoce a la sesión y, para cada dato que se puede
  cambiar, un byte que dice si cambia y luego el dato. De momento se
  conocen seis sitios de ese mensaje (`0x8`, `0x20`, `0x28`, `0x2A`, `0x32`
  y `0x3D0`).
- Dos de ellas eran las que usaba `network_life_cycle.c`:
  `network_session_leader_unknown_0005b1e0` y
  `network_session_leader_clear_unknown49f8` (antes `unknown_0005b570`).

### Lo que enseñó

- `0x0005b570` parecía recibir un valor en `ebx` que quien la llama no
  pone. Antes de darlo por un argumento escondido se miró entera: el `ebx`
  es un cero que la propia función pone al empezar. No había fallo en el
  hito anterior.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (146
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 338 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Lo que llega de las otras consolas

Seis funciones en `networking/network_session.c`, 713 bytes entre todas. De 1226
funciones se pasa a 1232, el 10,94 % de las del juego (el 6,779 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_unknown_0005bec0` | `0x0005bec0` | `networking/network_session.c` |
| `network_session_host_unknown_0005dea0` | `0x0005dea0` | `networking/network_session.c` |
| `network_session_unknown_0005e5b0` | `0x0005e5b0` | `networking/network_session.c` |
| `network_session_unknown_0005e640` | `0x0005e640` | `networking/network_session.c` |
| `network_session_local_player_accepted` | `0x0005f120` | `networking/network_session.c` |
| `network_session_host_remove_peer_player` | `0x0005f190` | `networking/network_session.c` |

Todas empiezan igual: de por qué canal del juego llega algo sacan qué
consola de la sesión lo manda (el observador dice qué canal suyo es, y la
sesión qué consola tiene ese canal).

### Lo que se sabe

- Los 52 bytes de `0x761C` (hito 291) son **cuatro entradas de 13 bytes**,
  una por cada jugador que esta consola puede tener: si la consola que
  lleva la sesión ha dicho que ese jugador entra, y por qué se le conoce.
  `network_session_local_player_accepted` las rellena cuando llega ese
  aviso (solo si viene de quien lleva la sesión, y si esta consola no tiene
  ya un jugador en ese sitio).
- `network_session_host_remove_peer_player` es lo contrario, en la consola
  que lleva la sesión: otra consola avisa de que su jugador de un sitio se
  va, y sale de la sesión.
- `network_session_unknown_0005e640`: en el octavo estado, la consola que
  manda algo recibe su bit en lo que la sesión tiene a partir del estado
  (`0x7430`) y se le borra el cuarto byte de lo que se guarda de ella.
- Las otras tres pasan lo que llega a funciones aún sin descompilar
  (`0x0005df30`, `0x00062ab0`, `0x0005a400`). `0x0005df30` recibe además un
  argumento en `edi`, que solo se ve en quien la llama.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (140
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 337 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — La sesión cambia de estado

Diez funciones en `networking/network_session.c`, 739 bytes entre todas. De 1216
funciones se pasa a 1226, el 10,88 % de las del juego (el 6,755 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_0005e150` | `0x0005e150` | `networking/network_session.c` |
| `network_session_unknown_0005e1a0` | `0x0005e1a0` | `networking/network_session.c` |
| `network_session_get_channel_peer` | `0x0005f5c0` | `networking/network_session.c` |
| `network_session_enter_state3` | `0x000612c0` | `networking/network_session.c` |
| `network_session_enter_state4` | `0x00061330` | `networking/network_session.c` |
| `network_session_enter_state6` | `0x00061450` | `networking/network_session.c` |
| `network_session_enter_state10` | `0x000616b0` | `networking/network_session.c` |
| `network_session_unknown_000618d0` | `0x000618d0` | `networking/network_session.c` |
| `network_session_unknown_00061910` | `0x00061910` | `networking/network_session.c` |
| `network_session_unknown_00061950` | `0x00061950` | `networking/network_session.c` |

### Lo que se sabe

- Cambiar de estado es siempre lo mismo: se borran los 504 bytes que la
  sesión tiene a partir del estado y se pone el nuevo. En casi todos, esos
  bytes empiezan con **cuándo empezó el estado**.
  - **Tercero** (`network_session_enter_state3`): empieza ahora, y los 52
    bytes de `0x761C` vuelven a empezar.
  - **Cuarto** (`network_session_enter_state4`): empieza ahora, con NONE
    detrás de la hora.
  - **Sexto** (`network_session_enter_state6`): antes se manda a todas las
    consolas algo de la clase `0xD`, con los ocho bytes de la sesión. Es lo
    mismo que se manda a una consola al sacarla (hito 321): el sexto estado
    es el de **la consola que lleva la sesión cuando la deshace**.
  - **Décimo** (`network_session_enter_state10`): con un número, que es por
    qué se acaba.
- Dos funciones miran si un estado lleva demasiado tiempo
  (`unknown_004cf4a4` y `unknown_004cf4ac`, en milisegundos): si se ha
  pasado se hace `0x0005a520` con la sesión, y si no, lo de cada vez.
- `network_session_unknown_0005e150` y `…0005e1a0` atienden algo que llega
  de la consola que lleva la sesión: si es la dirección que la sesión
  guarda en su primer estado se hace `0x0005a520`; si no, una hace
  `0x00062ab0` y la otra pasa la sesión al décimo estado con un 1.
- `network_session_get_channel_peer` es
  `network_session_find_peer_by_channel` (hito 320) con los argumentos en
  otros sitios: el juego la tiene dos veces.

### Sin comprobar

- El juego pregunta dos veces seguidas si la dirección es la que la sesión
  guarda; las pruebas dan la misma respuesta las dos veces.
- En este hito se pasaron las pruebas de `network_life_cycle` (134
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 336 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Contar bits, comparar jugadores y dos preguntas a la partida

Dos funciones en `networking/network_session.c` y dos en `networking/network_life_cycle.c`,
371 bytes entre todas. De 1212 funciones se pasa a 1216, el 10,79 % de las del juego
(el 6,731 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_count_bits` | `0x00063ca0` | `networking/network_session.c` |
| `network_player_identifier_equal` | `0x00063d00` | `networking/network_session.c` |
| `network_life_cycle_session_is_open` | `0x00063e00` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00063f00` | `0x00063f00` | `networking/network_life_cycle.c` |

### Lo que se sabe

- `network_count_bits` cuenta los bits puestos de un número, con el truco
  de sumar por pares, de cuatro en cuatro, de ocho en ocho…
- `network_player_identifier_equal` dice si dos jugadores son el mismo por
  sus doce bytes: cuentan los ocho primeros y, si se pide, dos bits de los
  otros cuatro. Es la misma comparación que hace la voz con quien habla
  (`VOICE_TALKER_FLAGS_MASK`, hito 264): los doce bytes de un jugador son
  un número de 64 bits y unos indicadores.
- `network_life_cycle_session_is_open` dice si la sesión de la partida
  admite jugadores: dice quién está, su número de `0x498C` es cero, tiene
  menos jugadores que su máximo, la partida no está en sus estados sexto a
  noveno y los 16 bits de `0x5DD0` son NONE.
- `network_life_cycle_unknown_00063f00` dice si esta consola lleva la
  sesión de la partida y su número de `0x4980` es 1.
- Las dos últimas están en el juego lejos del resto de
  `network_life_cycle.c` (en `0x00063e00`), pero leen sus variables; van
  ahí hasta que se vea de qué archivo son.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (124
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 335 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Los grupos y los jugadores de la lista de la sesión

Seis funciones en `networking/network_session.c`, 466 bytes entre todas. De 1206
funciones se pasa a 1212, el 10,76 % de las del juego (el 6,718 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_roster_find_player` | `0x00063190` | `networking/network_session.c` |
| `network_session_roster_group_maximum_unknown248` | `0x000631f0` | `networking/network_session.c` |
| `network_session_roster_find_group_by_unknown84` | `0x00063280` | `networking/network_session.c` |
| `network_session_roster_find_group_by_unknown4` | `0x000632e0` | `networking/network_session.c` |
| `network_session_roster_valid` | `0x00063340` | `networking/network_session.c` |
| `network_session_roster_has_any_player` | `0x00063510` | `networking/network_session.c` |

### Lo que se sabe

- El bloque de 776 bytes que la sesión guarda en `0x4A00` (hito 332) tiene
  ya estructura: hasta **dieciséis grupos**, cada uno conocido por ocho
  bytes y por doce, y hasta **dieciséis jugadores**, cada uno con sus doce
  bytes, dos números (el primero de -1 a 127, el segundo de -1 a
  `0x3FFFFFFF`) y el grupo al que pertenece. Lo llamo «lista»
  (`network_session_roster`), que es una conjetura: por los límites, el
  primer número parece un nivel y el segundo una puntuación.
- `network_session_roster_valid` era `unknown_00063340`, la que comprueba
  la lista antes de que la sesión la acepte: cuentas dentro de sus
  límites, cada jugador en un grupo que existe y sus dos números dentro de
  lo que pueden ser.
- Las otras buscan en ella: un jugador por sus doce bytes, un grupo por sus
  ocho o por sus doce, el mayor primer número de los jugadores de un grupo,
  y si alguno de varios jugadores está en la lista.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (120
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 334 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Mandar algo a las consolas, y quién puede entrar

Seis funciones en `networking/network_session.c`, 526 bytes entre todas. De 1200
funciones se pasa a 1206, el 10,71 % de las del juego (el 6,703 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_reset_unknown761c` | `0x00062b40` | `networking/network_session.c` |
| `network_session_send_to_peer` | `0x00062d20` | `networking/network_session.c` |
| `network_session_send_to_peers` | `0x00062da0` | `networking/network_session.c` |
| `network_session_requests_expire` | `0x00062de0` | `networking/network_session.c` |
| `network_session_requests_remove` | `0x00062fa0` | `networking/network_session.c` |
| `network_session_host_check_join` | `0x00062fe0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_send_to_peer` manda algo (una clase, un tamaño y los
  datos) a una consola de la sesión por su canal, de una manera, de la otra
  o de las dos; `network_session_send_to_peers`, a todas. Es lo que las dos
  funciones del hito 321 tenían metido dentro con «las dos maneras».
- El último número de una petición de entrada (hito 318) es **cuánto dura**,
  en milisegundos: `network_session_requests_expire` quita las que han
  pasado de ahí (NONE es que no caducan).
  `network_session_requests_remove` quita las de una consola, por sus ocho
  bytes.
- `network_session_host_check_join` decide si una consola que pide entrar
  puede: solo lo dice la consola que lleva la sesión, y solo en el quinto
  estado (3 si no); no con más de 1 en `0x498C` (2); una consola que ya
  está, puede; otra necesita sitio para una consola y para sus jugadores
  (4 si no cabe). De lo que manda la que pide se conocen ya cuatro cosas:
  cuántos jugadores trae, por qué se conoce a cada uno, un byte y los 36
  bytes de la consola.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (114
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 333 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — La consola que lleva la sesión cambia sus datos

Dieciséis funciones en `networking/network_session.c`, 1.426 bytes entre todas. De 1184
funciones se pasa a 1200, el 10,65 % de las del juego (el 6,685 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_0005c190` | `0x0005c190` | `networking/network_session.c` |
| `network_session_host_set_unknown49f0` | `0x0005c1c0` | `networking/network_session.c` |
| `network_session_host_clear_unknown49fc` | `0x0005c240` | `networking/network_session.c` |
| `network_session_host_set_unknown49f8` | `0x0005c290` | `networking/network_session.c` |
| `network_session_host_set_unknown49cc` | `0x0005c2e0` | `networking/network_session.c` |
| `network_session_host_set_unknown4a00` | `0x0005c350` | `networking/network_session.c` |
| `network_session_host_set_unknown5ddc` | `0x0005c3f0` | `networking/network_session.c` |
| `network_session_host_set_maximum_players` | `0x0005c490` | `networking/network_session.c` |
| `network_session_host_set_random` | `0x0005c4e0` | `networking/network_session.c` |
| `network_session_host_clear_unknown49c4` | `0x0005c530` | `networking/network_session.c` |
| `network_session_host_set_unknown4da8` | `0x0005c570` | `networking/network_session.c` |
| `network_session_host_set_unknown4f24` | `0x0005c5c0` | `networking/network_session.c` |
| `network_session_host_set_unknown4999` | `0x0005c660` | `networking/network_session.c` |
| `network_session_host_set_chosen_peer` | `0x0005c6d0` | `networking/network_session.c` |
| `network_session_matches_state2_address` | `0x0005c940` | `networking/network_session.c` |
| `network_session_unknown_0005c990` | `0x0005c990` | `networking/network_session.c` |

Trece de ellas son del mismo molde: solo hacen algo si esta consola lleva
la sesión (estados 5 a 8), cambian un dato de «lo que la sesión sabe» (el
bloque de `0x4978`) y suben el número de su principio, que es su
**contador de cambios**; las demás consolas se enteran por él.

### Lo que se sabe

- Los datos que se cambian: el máximo de jugadores (`0x4994`), los ocho
  bytes al azar (`0x4DA0`), la consola elegida para la voz (`0x5E20`:
  `network_session_host_set_chosen_peer` era `unknown_0005c6d0`, la que
  usa la voz en el hito 277), y otros que aún no tienen nombre: 28 bytes en
  `0x49CC`, dos números en `0x49F0`, un número en `0x49F8`, 776 bytes en
  `0x4A00`, ocho bytes en `0x4999`, 68 en `0x5DDC` y dos bloques grandes
  (108 y 3.648 bytes) en `0x4F24` y `0x4F90`. Los opcionales llevan delante
  un byte que dice si están.
- Varias solo escriben si el dato es distinto, y entonces el contador no
  sube: no se molesta a las demás consolas por nada.
- El bloque de `0x5DDC` lleva, tras sus cuatro primeros bytes, por qué se
  conoce a una sesión, y no se admite si es esta misma
  (`network_session_unknown_0005c190`): parece la sesión a la que hay que
  pasar.
- En el segundo estado la sesión guarda también una dirección, cuatro bytes
  antes que en el primero (`0x7444`):
  `network_session_matches_state2_address` la compara.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (108) y de
  `game_voice` (71).
- Las mutaciones de los hitos 280 a 308 y 321 a 332 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Diez consultas más a la sesión

Diez funciones en `networking/network_session.c`, 384 bytes entre todas. De 1174
funciones se pasa a 1184, el 10,51 % de las del juego (el 6,637 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_unknown_0005aeb0` | `0x0005aeb0` | `networking/network_session.c` |
| `network_session_get_unknown49c4` | `0x0005b040` | `networking/network_session.c` |
| `network_session_get_unknown49c8` | `0x0005b060` | `networking/network_session.c` |
| `network_session_get_unknown4a00` | `0x0005b080` | `networking/network_session.c` |
| `network_session_get_chosen_peer` | `0x0005b0b0` | `networking/network_session.c` |
| `network_session_get_unknown4d08` | `0x0005b0d0` | `networking/network_session.c` |
| `network_session_get_random` | `0x0005b120` | `networking/network_session.c` |
| `network_session_get_language` | `0x0005b150` | `networking/network_session.c` |
| `network_session_get_maximum_players` | `0x0005b180` | `networking/network_session.c` |
| `network_session_get_unknown5ddc` | `0x0005b1a0` | `networking/network_session.c` |

Casi todas son iguales: si la sesión dice quién está en ella (estados 3 a
8) dan un dato suyo, y si no, un valor por omisión.

### Lo que se sabe

- `network_session_get_chosen_peer` era `unknown_0005b0b0`, la que usa la
  voz para saber por qué consola se reparte (hito 277): es el número de
  `0x5E20`.
- `network_session_get_random` da los ocho bytes al azar de la sesión
  (hito 314) en dos registros; `network_session_get_language`, su idioma, o
  el del juego si no hay sesión; `network_session_get_maximum_players`, su
  máximo de jugadores, o 16.
- Las demás dan datos que aún no tienen nombre: un byte (`0x49C4`), un
  número (`0x49C8`), una cosa en `0x4A00` si un byte dice que está, dos
  números con una cosa en `0x4D10`, y 68 bytes en `0x5DDC` si un byte dice
  que están.
- `network_session_host_unknown_0005aeb0` llama a `0x000614a0` (sin
  descompilar) en el quinto estado: esa pone la sesión en el séptimo.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (92) y de
  `game_voice` (71).
- Las mutaciones de los hitos 280 a 308 y 321 a 331 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Lo que hace con la sesión la consola que la lleva

Seis funciones en `networking/network_session.c`, 510 bytes entre todas. De 1168
funciones se pasa a 1174, el 10,42 % de las del juego (el 6,624 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_host_remove_peer` | `0x0005ade0` | `networking/network_session.c` |
| `network_session_host_set_local` | `0x0005ae50` | `networking/network_session.c` |
| `network_session_host_set_player_unknownA8` | `0x0005aed0` | `networking/network_session.c` |
| `network_session_in_last_three_states` | `0x0005af50` | `networking/network_session.c` |
| `network_session_is_full_for` | `0x0005af70` | `networking/network_session.c` |
| `network_session_has_room_for_players` | `0x0005afc0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_host_remove_peer` es **echar a una consola**: solo lo
  hace la que lleva la sesión. A otra consola se le avisa y se la saca
  (hito 321). Si se echa a sí misma, la sesión pasa a un **décimo estado**
  que no se había visto, con un 1 al principio de lo que tiene a partir del
  estado: es deshacer la sesión.
- `network_session_host_set_local` apunta en «quién está» que la consola
  que lleva la sesión es esta.
- `network_session_host_set_player_unknownA8` cambia, en el quinto estado,
  el segundo de los dos bloques de 144 bytes de un jugador.
- Los números de `0x4990` y `0x4994` (16 los dos al arrancar) son **el
  máximo de consolas y el de jugadores**: `network_session_is_full_for`
  dice si no caben tantas consolas o tantos jugadores más.
- `network_session_has_room_for_players` dice si caben unos jugadores,
  contando con que los que ya habían pedido entrar tienen su sitio guardado.
- Cada cambio en «quién está» sube los dos contadores (`0x4C` y `0x7618`),
  como en los hitos 322 a 324.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (82
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 330 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Cuatro cosas que se le preguntan a la sesión

Cuatro funciones en `networking/network_session.c`, 203 bytes entre todas. De 1164
funciones se pasa a 1168, el 10,37 % de las del juego (el 6,607 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_get_identifier` | `0x0005a600` | `networking/network_session.c` |
| `network_session_get_security` | `0x0005a620` | `networking/network_session.c` |
| `network_session_get_membership` | `0x0005a680` | `networking/network_session.c` |
| `network_session_has_channel_peer` | `0x0005a6a0` | `networking/network_session.c` |

Son consultas pequeñas, de las que hay muchas en esta zona (quedan unas
ochenta funciones de menos de 160 bytes entre `0x00058d00` y `0x00064000`).

### Lo que se sabe

- `network_session_get_identifier` da los ocho bytes por los que se conoce
  a la sesión, o nada si no tiene estado o no se la conoce por nada;
  `network_session_get_security` da además la clave y los dos números que
  van con ellos, cada cosa solo si hay dónde ponerla.
- `network_session_get_membership` da «quién está en la sesión», y de paso
  qué consola es esta y cuál lleva la sesión.
- `network_session_has_channel_peer` dice si alguna consola de la sesión
  tiene un canal.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (76
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 329 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — De una dirección a quién es la consola

Dos funciones en `networking/transport_security.c`, 426 bytes entre las dos. De 1162
funciones se pasa a 1164, el 10,33 % de las del juego (el 6,600 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `transport_security_peer_address_from_address` | `0x0007ab60` | `networking/transport_security.c` |
| `transport_address_equal` | `0x0007af80` | `networking/transport_security.c` |

### Lo que se sabe

- `transport_security_peer_address_from_address` era `unknown_0007ab60`, la
  que usa la sesión para saber qué consola suya tiene una dirección (hito
  320). De una dirección del juego saca los 36 bytes con que se conoce a la
  consola:
  - con su segundo argumento (el número de `0x3C` de la sesión, que se ve
    así que es **si la sesión no sale a la red de verdad**), son la propia
    dirección y ceros;
  - sin él, se los pide a la biblioteca (`XNetInAddrToXnAddr`), que da
    también por qué se conoce a la sesión de esa consola; y si alguna de las
    ocho sesiones guardadas se conoce por eso, da cuál es y su clave.
- `transport_address_equal` dice si dos direcciones del juego son la misma,
  con el puerto o sin él. Es lo que `network_session_matches_state1_address`
  (hito 321) tenía metido dentro.

### Sin comprobar

- En este hito se pasaron las pruebas de `transport_security` (16) y de
  `network_life_cycle` (72).
- Las mutaciones de los hitos 280 a 308 y 321 a 328 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Las direcciones de la red y los nombres de la biblioteca de la consola

Diez funciones en `networking/transport_security.c`, 773 bytes entre todas. De 1152
funciones se pasa a 1162, el 10,32 % de las del juego (el 6,586 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `transport_security_get_addresses` | `0x0007aaa0` | `networking/transport_security.c` |
| `transport_security_address_from_peer_address` | `0x0007ab10` | `networking/transport_security.c` |
| `transport_address_connect` | `0x0007acc0` | `networking/transport_security.c` |
| `transport_address_get_connect_status` | `0x0007acf0` | `networking/transport_security.c` |
| `transport_security_random_identifier` | `0x0007ad50` | `networking/transport_security.c` |
| `transport_security_random_bytes` | `0x0007ad80` | `networking/transport_security.c` |
| `transport_security_address_from_key` | `0x0007adf0` | `networking/transport_security.c` |
| `transport_security_get_unknown7cc` | `0x0007ae70` | `networking/transport_security.c` |
| `transport_address_get_secure_in_address` | `0x0007aec0` | `networking/transport_security.c` |
| `transport_address_valid` | `0x0007af40` | `networking/transport_security.c` |

### Lo que se sabe

- La lista de funciones de Ghidra **sí tiene los nombres** de la biblioteca
  de red de la consola. Confirman lo supuesto en los hitos 315 y 316, y las
  funciones que tenían nombre provisional pasan a llamarse como son:
  `XNetCreateKey`, `XNetRegisterKey`, `XNetUnregisterKey`, `XNetQosListen`
  y `XNetGetTitleXnAddr`. Las de este hito: `XNetRandom`,
  `XNetXnAddrToInAddr`, `XNetConnect` y `XNetGetConnectStatus`.
- `transport_address_valid` dice si una dirección del juego lo es: de
  cuatro bytes y distinta de cero, o de dieciséis y no toda ceros.
- `transport_address_get_secure_in_address` la pasa a como la tiene la
  biblioteca, y dice si es de las que la biblioteca da (empiezan por cero).
  Con eso, `transport_address_connect` empieza a hablar con ella y
  `transport_address_get_connect_status` dice cómo va (0 a 3, o 4).
- `transport_security_address_from_key` hace el camino contrario: de cómo
  se conoce a una consola a su dirección del juego, con la clave de una
  sesión; `transport_security_address_from_peer_address` prueba con las
  ocho si no se dice cuál.
- `transport_security_random_bytes` era `unknown_0007ad80` (la usaban
  `game.c` y la sesión): bytes al azar, de la biblioteca si está, y si no
  de la hora, `rand` y `GetTickCount` con un generador sencillo.
  `transport_security_random_identifier` saca ocho que no sean todo ceros.

### Sin comprobar

- En este hito se pasaron las pruebas de `transport_security` (14),
  `network_life_cycle` (72), `game` y `game_voice`.
- Las mutaciones de los hitos 280 a 308 y 321 a 327 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — El observador de la red: sesiones y canales

Seis funciones en un archivo nuevo, `networking/network_observer.c`, 482 bytes entre todas.
De 1146 funciones se pasa a 1152, el 10,23 % de las del juego (el 6,560 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_observer_dispose` | `0x00075a40` | `networking/network_observer.c` |
| `network_observer_set_session` | `0x00075a90` | `networking/network_observer.c` |
| `network_observer_get_unknown4e08` | `0x00075c80` | `networking/network_observer.c` |
| `network_observer_find_channel` | `0x00075ce0` | `networking/network_observer.c` |
| `network_observer_find_channel_by_unknown1e` | `0x00075d30` | `networking/network_observer.c` |
| `network_observer_channel_unknown_00075e40` | `0x00075e40` | `networking/network_observer.c` |

Es «lo que una sesión recibe al prepararse» (hito 291), que hasta ahora no
tenía nombre: lo llamo **observador**, una conjetura como el nombre del
archivo.

### Lo que se sabe

- Tiene algo de cada una de las cuatro sesiones (36 bytes desde `0x14`: la
  sesión, dos números, los ocho bytes por los que se la conoce y su clave)
  y **quince canales** de `0x528` bytes desde `0xA8`, uno por consola con
  la que esta habla. De cada canal: un estado (cero si no se usa), un bit
  por sesión que lo usa, cuál de los canales del juego es (los de
  `0x004d87d4`, de `0xF8` bytes) y seis bytes por los que se le puede
  buscar.
- `network_observer_set_session` y `network_observer_find_channel` eran
  `unknown_00075a90` y `unknown_00075ce0`, a las que llama la sesión (hitos
  315 y 320): el «número» que la sesión guarda de cada consola es el de su
  canal aquí, y el que recibían `network_session_channel_is_host` y
  `network_session_peer_answered` es el del canal del juego.
- `network_observer_get_unknown4e08` da dos números y qué parte es una
  cuenta de la suma de dos: parece una estadística de lo que se pierde.

### Lo que enseñó

- La suma de las dos cuentas se desborda en el juego como en el procesador
  (dos números muy grandes suman menos que nada, y entonces no hay
  estadística). En C eso no está definido y el compilador lo aprovechaba:
  la prueba lo cazó. Se suma sin signo.

### Sin comprobar

- `0x00075970`, que prepara el observador, no está en la lista de funciones
  de Ghidra; queda para cuando se vea cómo contarla.
- En este hito se pasaron las pruebas de `network_observer` (6) y de
  `network_life_cycle` (72).
- Las mutaciones de los hitos 280 a 308 y 321 a 326 están pendientes; las
  de los hitos 309 a 320 seguían corriendo.

## 2026-10-08 — Lo que ha cambiado de una consola

Una función en `networking/network_session.c`, de 333 bytes. De 1145 funciones se pasa a
1146, el 10,17 % de las del juego (el 6,543 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_peer_properties_get_update` | `0x000602b0` | `networking/network_session.c` |

### Lo que se sabe

- Lo que la sesión sabe de una consola, aparte de cómo se la conoce en la
  red y de sus jugadores, son 200 bytes desde su `0x28`: dos textos (de 16
  y de 32 caracteres), dos números, tres números con cuatro más (los que
  la voz ya leía en `0x9C`–`0xA8`), 64 bytes y un número. Ahora tienen
  estructura: `network_session_peer_properties`.
- Esta función los compara con los de antes y apunta **solo lo que ha
  cambiado**: cinco partes, cada una detrás de un byte que dice si ha
  cambiado. Sin nada con que comparar, ha cambiado todo. Es como la sesión
  prepara lo que manda a las demás consolas.
- Los textos se comparan como textos: lo que haya detrás de su final no
  cuenta.
- La parte de 64 bytes está alineada a cuatro: no son bytes sueltos.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (72
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 325 están pendientes; las
  de los hitos 309 a 320 estaban corriendo al cerrar este hito
  (`transport_security`: 19 de 19 cazadas).

## 2026-10-08 — El jugador de una consola entra en la sesión

Dos funciones en `networking/network_session.c`, 291 bytes entre las dos. De 1143
funciones se pasa a 1145, el 10,16 % de las del juego (el 6,532 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_add_peer_player` | `0x00060000` | `networking/network_session.c` |
| `network_session_free_player_places` | `0x00062e30` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_add_peer_player` es la que decide si el jugador que una
  consola dice tener en uno de sus cuatro sitios entra en la sesión, y
  devuelve qué jugador de la sesión es (o NONE):
  - si la consola ya lo tiene ahí, nada cambia;
  - si tiene a otro, ese sale;
  - no entra si ya está en la sesión, si lo que la sesión tiene en `0x78A8`
    dice que no, o si no había pedido entrar y no queda sitio;
  - si entra, ocupa el primer sitio libre de los dieciséis.
- Lo de `0x78A8` es un objeto con tabla de funciones; la cuarta dice si un
  jugador puede entrar. Es la primera vez que se ve para qué sirve.
- `network_session_free_player_places` cuenta el sitio que queda: el
  número de `0x4994` (16 al arrancar: es **el máximo de jugadores**) menos
  los que hay y menos los que han pedido entrar y aún no están.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (71
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 324 están pendientes; las
  de los hitos 309 a 320 estaban corriendo al cerrar este hito.

## 2026-10-08 — Un jugador entra, y las consolas que faltan por contestar

Cuatro funciones en `networking/network_session.c`, 550 bytes entre todas. De 1139
funciones se pasa a 1143, el 10,15 % de las del juego (el 6,523 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_stop_waiting_if_done` | `0x0005a2e0` | `networking/network_session.c` |
| `network_session_peer_answered` | `0x0005a340` | `networking/network_session.c` |
| `network_session_add_player` | `0x000600f0` | `networking/network_session.c` |
| `network_session_remove_player_and_count` | `0x000601e0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_add_player` es la contraria de
  `network_session_remove_player` (hito 322): pone a un jugador en su sitio
  con los doce bytes por los que se le conoce, su consola y su sitio entre
  los de ella, y sube las cuentas. Si había pedido entrar, su petición pasa
  a decir que la sesión ya lo tiene.
- El byte `0x765C` de la sesión es que **está esperando**: la consola que
  lleva la sesión ha cambiado algo y espera a que las demás contesten. El
  tercer byte de lo que guarda de cada consola es que **esa aún no ha
  contestado** (al entrar en el quinto estado se copia del segundo, que es
  si se habla con ella).
- `network_session_peer_answered` apunta la respuesta de una consola: vale
  si la sesión espera, la consola no es esta y los dos números de la
  respuesta son los de la sesión (`0x4980` y `0x497C`). Cuando no queda
  ninguna por contestar, deja de esperar.
- `network_session_stop_waiting_if_done` (era `unknown_0005a2e0`) hace esa
  misma comprobación cuando una consola sale de la sesión.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (69
  funciones).
- Las mutaciones de los hitos 280 a 308 y 321 a 323 están pendientes; las
  de los hitos 309 a 320 estaban corriendo al cerrar este hito.

## 2026-10-08 — Una consola y sus jugadores salen de la sesión

Tres funciones en `networking/network_session.c`, 731 bytes entre todas. De 1136
funciones se pasa a 1139, el 10,11 % de las del juego (el 6,504 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_remove_peer` | `0x0005fe20` | `networking/network_session.c` |
| `network_session_remove_player` | `0x00060200` | `networking/network_session.c` |
| `network_session_find_request` | `0x00062f40` | `networking/network_session.c` |

`network_session_remove_peer` era `unknown_0005fe20`, con la que acababan
las dos funciones del hito 321.

### Lo que se sabe

- De cada consola la sesión sabe **cuántos jugadores tiene** (`0xF8`) y
  **cuáles son** (los cuatro números de `0xFC`, NONE donde no hay). Y de
  cada jugador, su consola (`0xC`) y qué sitio ocupa entre los de ella
  (`0x10`). La cuenta de jugadores de la sesión está en `0x10CC` de «quién
  está en ella», antes del bit de cada uno.
- `network_session_remove_player` quita a un jugador de todo eso. En los
  cuatro últimos estados (los de la consola que lleva la sesión), si ese
  jugador había pedido entrar, su petición deja de decir que la sesión lo
  tiene (`network_session_find_request` la busca).
- `network_session_remove_peer` quita a los jugadores de la consola, baja
  un sitio a las consolas que van detrás (y a lo que se guarda aparte de
  cada una) y corrige todos los números de consola que quedan por encima:
  el de cada jugador, el de quien lleva la sesión, el de esta consola y el
  del `0x40`. Si la que sale llevaba la sesión, pasa a llevarla la primera.
- El primer número de «quién está en ella» (`0x4C`) y el de `0x7618`
  cuentan uno más por cada cambio: son **contadores de versión**.

### Lo que enseñó

- `network_session_remove_peer` acaba saltando a otra función en vez de
  llamarla, así que no tiene `ret`. El arnés buscaba el primer `ret` a
  partir de una función de `source/` para saber cuántos argumentos quita de
  la pila, y encontraba el de la función siguiente: las pruebas del hito
  321, que la sustituyen, se rompieron. Ahora `tools/difftest.py` mira solo
  dentro de la función (su tamaño está en la tabla de símbolos), y si no
  hay `ret` vale lo que dice `symbols/functions.csv`.

### Batería completa

La del hito 313 terminó: 1.109 funciones, las 1.109 bien. Es anterior al
cambio del arnés; después de él se pasaron las pruebas de `network_life_cycle`
(65), `voice*` (69), `game_voice` (71) y `transport_security` (4).

### Sin comprobar

- Las mutaciones de los hitos 280 a 308, 321 y 322 están pendientes; las de
  los hitos 309 a 320 (132) se lanzan con este hito.

## 2026-10-08 — Avisar a una consola y sacarla de la sesión

Tres funciones en `networking/network_session.c`, 337 bytes entre todas. De 1133
funciones se pasa a 1136, el 10,08 % de las del juego (el 6,479 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_matches_state1_address` | `0x0005c8f0` | `networking/network_session.c` |
| `network_session_remove_peer_with_message_e` | `0x0005fd10` | `networking/network_session.c` |
| `network_session_remove_peer_with_message_d` | `0x0005fda0` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_matches_state1_address` era `unknown_0005c8f0` (hito
  320): en su primer estado la sesión guarda una dirección del juego en
  `0x7448`, y esto dice si otra es la misma (mismo tamaño, mayor que cero,
  y mismos bytes). Es la dirección de quien lleva la sesión cuando aún no
  se sabe quién más está: el primer estado es el de **estar entrando**.
- Las otras dos mandan algo a una consola y la sacan de la sesión
  (`0x0005fe20`, sin descompilar). Lo que mandan lleva los ocho bytes por
  los que se conoce a la sesión, y es de la clase `0xE` en una y `0xD` en
  la otra; la de `0xE` no hace nada en los tres últimos estados.
- Solo se manda a una consola con el segundo byte de lo que la sesión
  guarda de ella (el que recibe al entrar): es, entonces, **si se habla
  con ella directamente**. Se manda con `0x00075e80` de dos maneras si su
  canal está en el estado 7, y de una sola si no.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (62
  funciones).
- Las mutaciones de los hitos 280 a 308 y la del 321 están pendientes; las
  de los hitos 309 a 320 están escritas (132) y esperan a que termine la
  batería completa del hito 313.

## 2026-10-08 — Más maneras de buscar una consola en la sesión

Cinco funciones en `networking/network_session.c`, 401 bytes entre todas. De 1128
funciones se pasa a 1133, el 10,06 % de las del juego (el 6,468 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_channel_is_host` | `0x0005f600` | `networking/network_session.c` |
| `network_session_find_peer_by_channel` | `0x0005f670` | `networking/network_session.c` |
| `network_session_find_peer_by_transport_address` | `0x0005f6a0` | `networking/network_session.c` |
| `network_session_find_peer_by_unknownA` | `0x0005f6f0` | `networking/network_session.c` |
| `network_session_address_is_host` | `0x0005f7c0` | `networking/network_session.c` |

### Lo que se sabe

- Una consola de la sesión se puede buscar de cuatro maneras: por los 36
  bytes con que se la conoce (hito 319), por el número que la sesión guarda
  de ella (`network_session_find_peer_by_channel`), por una dirección del
  juego (`network_session_find_peer_by_transport_address`, que antes la
  pasa a esos 36 bytes con `0x0007ab60`) y por seis bytes de los 36, desde
  el `0xA` (`network_session_find_peer_by_unknownA`): es donde las
  bibliotecas de la consola tienen los de su tarjeta de red.
- Las otras dos dicen si algo **viene de la consola que lleva la sesión**
  (la del `0x40`): por un número (`network_session_channel_is_host`) o por
  una dirección (`network_session_address_is_host`). En el primer estado,
  cuando aún no se sabe quién está, vale la dirección que la sesión guarda
  (`0x0005c8f0`).
- El `0x40` de la sesión es, entonces, **la consola que la lleva**, y el
  número que se guarda de cada consola es el de su canal en lo que la sesión
  recibió al prepararse (`0x00075ce0` lo busca ahí).

### Lo que enseñó

- `network_session_find_peer_by_transport_address` recibe la dirección en
  `ecx` y no la toca: solo la deja pasar a `0x0007ab60`. No se veía en su
  código, sino en quien la llama. Es el aviso de los argumentos escondidos
  en registros, otra vez.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (59
  funciones).
- Las mutaciones de los hitos 280 a 320 están pendientes.

## 2026-10-08 — Buscar una consola en una sesión

Tres funciones en `networking/network_session.c`, 392 bytes entre todas. De 1125
funciones se pasa a 1128, el 10,01 % de las del juego (el 6,455 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_find_peer` | `0x0005f760` | `networking/network_session.c` |
| `network_session_has_players_of` | `0x0005f810` | `networking/network_session.c` |
| `network_session_peer_state_delete` | `0x0005f970` | `networking/network_session.c` |

### Lo que se sabe

- `network_session_find_peer` era `unknown_0005f760`, la que usaba la voz
  (`game_voice.c`) para saber qué consola de una sesión es una de otra: las
  compara por los 36 bytes con que se las conoce en la red.
- `network_session_has_players_of` dice si todos los jugadores de una
  sesión están también en otra (por sus doce bytes). Sirve para pasar de
  una sesión a la otra de la partida.
- `network_session_peer_state_delete` borra lo que la sesión guarda aparte
  de una consola. Si tenía número, ese número es el de una cosa de `0x528`
  bytes en lo que la sesión recibió al prepararse, que tiene un byte con un
  bit por sesión: se le quita el de esta. Parece el canal por el que se
  habla con esa consola.
- Antes de eso el juego busca, en los cuatro últimos estados, otra consola
  con los tres primeros bytes puestos, y no hace nada con lo que encuentra:
  es lo que queda de una comprobación que el juego final no hace.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (54) y de
  `game_voice` (71).
- Las mutaciones de los hitos 280 a 319 están pendientes.

## 2026-10-08 — Los jugadores que piden entrar y el quinto estado de la sesión

Cinco funciones en `networking/network_session.c`, 641 bytes entre todas. De 1120
funciones se pasa a 1125, el 9,99 % de las del juego (el 6,441 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_find_player` | `0x0005f890` | `networking/network_session.c` |
| `network_session_reset_requests` | `0x00060fa0` | `networking/network_session.c` |
| `network_session_enter_state5` | `0x00061390` | `networking/network_session.c` |
| `network_session_request_players` | `0x00062e70` | `networking/network_session.c` |
| `network_session_request_player` | `0x00062eb0` | `networking/network_session.c` |

Con estas, todo lo que `network_session_start` (hito 314) hace con la sesión
está descompilado; solo le queda fuera `0x0007ad80` (los bytes al azar).

### Lo que se sabe

- En `0x7668` la sesión tiene sitio para **dieciséis jugadores que piden
  entrar**, de 36 bytes cada uno: si lo hay, si la sesión ya tiene a ese
  jugador, ocho bytes de la consola que pide, los doce bytes por los que
  se conoce al jugador, un número suyo, cuándo pidió y un número común a
  los que piden juntos.
- `network_session_request_player` ocupa el primer sitio libre;
  `network_session_request_players` lo hace con varios hasta que uno no
  cabe. `network_session_find_player` busca a un jugador de la sesión por
  sus doce bytes.
- `network_session_enter_state5` pone la sesión en su quinto estado y borra
  lo que tiene a partir del estado (504 bytes). Viniendo de un estado
  anterior, antes olvida lo que la sesión «iba a ser» y quién pedía entrar
  (`network_session_reset_requests`), y apunta que la consola que lleva la
  sesión es esta. Es lo último que hace una sesión al arrancar: el quinto
  estado es el de **la consola que lleva la sesión**.

### Lo que enseñó

- En el hito 314 dos argumentos de `network_session_start` estaban
  nombrados al revés: el sexto es cuántos jugadores piden entrar, y el
  décimo es otro número (el común a todos ellos). Se comportaba igual; solo
  cambian los nombres.

### Batería completa

La del hito 313 seguía en marcha al cerrar este hito.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (51
  funciones).
- Las mutaciones de los hitos 280 a 318 están pendientes.

## 2026-10-08 — Una consola entra en la sesión

Dos funciones en `networking/network_session.c`, 415 bytes entre las dos. De 1118
funciones se pasa a 1120, el 9,94 % de las del juego (el 6,420 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_peer_state_new` | `0x0005f900` | `networking/network_session.c` |
| `network_session_add_peer` | `0x0005fbd0` | `networking/network_session.c` |

`network_session_add_peer` era `unknown_0005fbd0`, con la que
`network_session_start` (hito 314) mete a esta consola en la sesión recién
arrancada.

### Lo que se sabe

- Las consolas de una sesión están en «quién está en ella» desde `0x58`,
  `0x10C` bytes cada una (lo mismo que ya se leía en la voz), con la cuenta
  en `0x54`.
- Al entrar, de una consola solo se sabe **cómo se la conoce en la red**
  (los 36 bytes del principio) y, si se dan, dos números en `0xF0`. Sus dos
  textos (de 16 y de 32 caracteres de 16 bits) quedan vacíos y cuatro
  números de `0xFC` a NONE.
- Aparte, la sesión guarda de cada consola 20 bytes en `0x72DC`: si la hay,
  tres bytes, un número que recibe (NONE para esta consola), dos a NONE y
  cuándo entró (`GetTickCount`). Eso lo arranca
  `network_session_peer_state_new`.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (46
  funciones).
- Las mutaciones de los hitos 280 a 317 están pendientes.

## 2026-10-08 — La dirección de la consola y las claves de las sesiones

Cuatro funciones en un archivo nuevo, `networking/transport_security.c`, 493 bytes entre
todas. De 1114 funciones se pasa a 1118, el 9,92 % de las del juego (el 6,406 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `transport_security_create_key` | `0x0007a8a0` | `networking/transport_security.c` |
| `transport_security_set_key` | `0x0007a920` | `networking/transport_security.c` |
| `transport_security_forget_address` | `0x0007a9a0` | `networking/transport_security.c` |
| `transport_security_get_address` | `0x0007a9b0` | `networking/transport_security.c` |

Son las funciones de las que dependían los hitos 314 y 315: quién es esta
consola en la red y qué clave tiene cada sesión. El nombre del archivo es
una conjetura.

### Lo que se sabe

- `transport_security_get_address` pregunta a la biblioteca de la consola
  (`0x003cd17d`) cómo se la conoce en la red: 36 bytes, que van a
  `0x004cf793`. La respuesta trae un número: 0 es «todavía no» (se volverá
  a preguntar), 1 es «nada» (no se conoce, pero ya se preguntó), y con
  cualquiera de los bits `0x1C` los cuatro primeros bytes son una dirección,
  que se guarda al revés en `0x004cf7b8` como dirección del juego: 16 bytes,
  un puerto y cuántos bytes cuentan (4).
- `transport_security_forget_address` hace que se vuelva a preguntar.
- `transport_security_set_key` guarda el identificador (8 bytes) y la clave
  (16) de una sesión en su entrada de la tabla `0x004cf7d4`, sacando antes
  de la biblioteca lo que hubiera (`0x003cd0e1`) y dándole lo nuevo
  (`0x003cd0d6`) salvo que se le diga que no.
- `transport_security_create_key` los hace: todo ceros si el primer
  argumento no es cero (una sesión que no sale a la red), y si es cero los
  hace la biblioteca (`0x003cd0cb`); el bit alto del primer byte del
  identificador lo pone el juego.
- Las variables `0x004cf792`, `0x004cf793`, `0x004cf7d4` y `0x004cf8d4`
  pasan de `network_session.c` a este archivo, que es de quien son.

### Sin comprobar

- Si la biblioteca no puede hacer la clave, el juego sigue con lo que
  hubiera en la pila. En las pruebas la biblioteca siempre escribe algo.
- En este hito se pasaron las pruebas de `transport_security` (4) y de
  `network_life_cycle` (44).
- Las mutaciones de los hitos 280 a 316 están pendientes.

## 2026-10-08 — El identificador y la clave de una sesión

Cuatro funciones en `networking/network_session.c`, 410 bytes entre todas. De 1110
funciones se pasa a 1114, el 9,89 % de las del juego (el 6,390 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_create_identifier` | `0x0005fa30` | `networking/network_session.c` |
| `network_session_use_identifier` | `0x0005fac0` | `networking/network_session.c` |
| `network_session_set_identifier` | `0x0005fb00` | `networking/network_session.c` |
| `network_session_forget_identifier` | `0x0005fb60` | `networking/network_session.c` |

Son las dos maneras de preparar una sesión para la red que usa
`network_session_start` (hito 314), y las dos piezas de que están hechas.

### Lo que se sabe

- Una sesión se conoce en la red por **ocho bytes** (ya vistos en el hito
  290, en su `0x1C`) y tiene además una **clave de dieciséis**, en `0x25`
  (sin alinear), y un número en `0x3C`. El byte de `0x24` dice si los
  tiene.
- Fuera de la sesión se guardan en una tabla, `0x004cf7d4`: ocho entradas
  de 32 bytes, por el número que la sesión tiene en `0x38`.
- `network_session_forget_identifier` los suelta: avisa a la biblioteca de
  la consola (`0x003cd147` con `0x10`, y `0x003cd0e1` para la entrada de
  la tabla), deja a NONE el primer número de lo que la sesión recibió al
  prepararse, y borra identificador, clave y número.
- `network_session_set_identifier` los pone, avisa a `0x00075a90` y a la
  biblioteca (`0x003cd147` con 1).
- `network_session_use_identifier` usa el identificador y la clave que le
  dan (es entrar en la sesión de otro);
  `network_session_create_identifier` los hace nuevos con `0x0007a8a0` (es
  crear una sesión propia) y los recoge de la tabla.
- Los tamaños (8 y 16 bytes) y el par «empezar / acabar» cuadran con los
  identificadores y claves de sesión de las bibliotecas de red de la
  consola; los nombres de esas funciones no están confirmados.

### Lo que enseñó

- `network_session_use_identifier` no usa su cuarto argumento. Al estar ya
  en el mismo archivo que quien la llama, el compilador deja de pasarlo, y
  la prueba de `network_session_start` tuvo que dejar de compararlo. Es el
  aviso de siempre: lo que una función no usa no se puede comparar.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (44
  funciones).
- Las mutaciones de los hitos 280 a 315 están pendientes.

## 2026-10-08 — Arrancar una sesión de red

Una función en `networking/network_session.c`, de 628 bytes. De 1109 funciones se pasa a
1110, el 9,85 % de las del juego (el 6,376 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_start` | `0x00059bd0` | `networking/network_session.c` |

Era `unknown_00059bd0`, a la que llama `network_life_cycle_unknown_00059890`
(hito 290) cuando la partida quiere una sesión. Tiene diez argumentos y
llama a once funciones, seis de ellas sin descompilar.

### Lo que se sabe

- El primer número que recibe (se guarda en el `0x18` de la sesión) dice
  si la sesión es de red. Con cero no lo es, y esta consola entra con una
  dirección a ceros.
- Si lo es, la consola tiene que saber **cómo se la conoce en la red**:
  36 bytes en `0x004cf793` (el mismo tamaño que la dirección de una
  consola en sus bibliotecas), con un byte delante (`0x004cf792`) que dice
  si los tiene. Si hace falta se piden antes (`0x0007a9b0`). Sin ellos,
  o si `0x0005fa30` / `0x0005fac0` no pueden preparar la sesión, no
  arranca.
- Después, lo que la sesión sabe empieza de cero: cuándo arrancó
  (`GetTickCount`, o el tiempo que guarda `main.c`), el idioma del juego,
  dos números a 16, ocho bytes al azar, y un `0xDEADBEEF` en `0x4DA8`.
- En ella no hay nadie más que esta consola, que entra la primera
  (`0x0005fbd0`) y es la que la lleva; lo que la sesión «va a ser» no dice
  nada (NONE).
- La sesión mide al menos `0x78B8` bytes, ocho más de lo apuntado: al final
  hay `GetTickCount` cambiado de signo y la hora de la biblioteca de C.
- Si recibe una cuenta de cosas (identificadores de 12 bytes, que es lo que
  mide el de un jugador), van a `0x00062e70`.

### Batería completa

La del hito 313 estaba en marcha al cerrar este hito.

### Sin comprobar

- En este hito se pasaron las pruebas de `network_life_cycle` (40
  funciones).
- Las mutaciones de los hitos 280 a 314 están pendientes.

## 2026-10-08 — La voz colocada en el espacio: `voice_routing.c` completo

Una función en `networking/voice_routing.c`, de 737 bytes, y con ella el archivo queda
completo. De 1108 funciones se pasa a 1109, el 9,84 % de las del juego (el 6,355 %
de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_get_mix_bins` | `0x000586f0` | `networking/voice_routing.c` |

Era `unknown_000586f0`, la que dice por qué altavoces sale la voz de un
jugador que se oye «de la tercera manera» y cuánto por cada uno.
`voice_routing.c` queda completo: 40 funciones, de `0x00055960` a
`0x000589e0`.

### Lo que se sabe

- El número de 16 bits del `0x28` de un jugador (hitos 307 y 309) es **cuál
  de los que oyen los sonidos del juego** le corresponde (`sound_listener`,
  en `game_sound_globals+0x88`). Renombrado a
  `VOICE_PLAYER_DATUM_LISTENER_OFFSET`.
- La posición de quien habla se pasa a como se ve desde ese oyente
  (`matrix4x3_inverse_transform_point`), se mide la distancia y se
  normaliza la dirección.
- `0x00192d40` (sin descompilar) reparte la dirección entre cinco
  altavoces, que son los pares 0, 1, 4, 5 y 2 de la biblioteca de sonido
  (delante izquierda y derecha, detrás izquierda y derecha, y centro). El
  que menos tiene se descarta: salen cuatro pares.
- El volumen de cada uno baja con lo poco que le toca (por el número del
  `0x18C` de «quién oye a quién») y con la distancia (por el del `0x188`),
  y mucho más deprisa (por 150) pasada la distancia a la que se oye una
  voz. Queda entre -10.000 y 0.
- Si nadie oye por el jugador, un solo par: el centro, a todo volumen.
- La distancia se calcula con las instrucciones antiguas de reales
  (`long double`), como en `voice_get_distance`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo (40 funciones).
- La batería completa se lanza con este hito, en la copia de comprobación.
- Las mutaciones de los hitos 280 a 313 están pendientes.

## 2026-10-08 — Por dónde sale la voz según cómo se oye

Una función en `networking/voice_routing.c`, de 339 bytes. De 1107 funciones se pasa a
1108, el 9,84 % de las del juego (el 6,330 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_route_player_to_output` | `0x00058590` | `networking/voice_routing.c` |

Era `unknown_00058590`, a la que `voice_channels_route_player` (hito 310)
da el jugador de esta consola más cercano de los que oyen a uno de otra de
la tercera manera.

### Lo que se sabe

- Recibe la manera en `eax` y el jugador en `edi`; el juego solo la llama
  con la tercera, pero trata las cuatro:
  - **no se oye**: prioridad `NONE` en la «salida 4» y nada más;
  - **segunda**: los tres pares delanteros `{0,0}`, `{1,0}`, `{2,0}`;
  - **tercera**: si el jugador tiene voz y objeto, pregunta dónde está el
    objeto y `0x000586f0` (sin descompilar) dice por dónde sale la voz y
    cuánto para quien le oye: hasta cuatro pares;
  - y en todas las que se oye, prioridad 0.
- Con esto la tercera manera queda clara: es la voz **colocada en el
  espacio**, que sale por los altavoces según dónde está quien habla.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 280 a 312 están pendientes.

## 2026-10-08 — Los mandos que oyen a un jugador de otra consola

Una función en `networking/voice_routing.c`, de 461 bytes. De 1106 funciones se pasa a
1107, el 9,83 % de las del juego (el 6,319 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_route_player_to_controllers` | `0x000583c0` | `networking/voice_routing.c` |

Era `unknown_000583c0`, a la que `voice_channels_route_player` (hito 310)
da los jugadores de esta consola que oyen a uno de otra de la primera
manera.

### Lo que se sabe

- Apunta en `0x108` de «quién oye a quién» (nuevo: un número por jugador
  de esta consola) a quién oye cada uno de la primera manera.
- De cada jugador que le oye saca su mando, de lo que la sesión dice de
  quién está en ella, y junta un bit por mando.
- Luego recorre los cuatro mandos: en los que le oyen, prioridad 0 para
  ese jugador (después de dar a `0x00332e34` el par `{2,0}` para la
  «salida 4»); en los demás, prioridad `NONE`. Cuadra con que la primera
  manera es la voz por el auricular de cada mando.
- `voice_next_controller` pasa también de `voice.c` a `voice.h`: el juego
  la tiene metida en cada bucle sobre los mandos, y este está fuera de
  `voice.c`.

### Sin comprobar

- En este hito se pasaron las pruebas de la voz (67 funciones de
  `voice`, `voice_routing` y `voice_mute_list`).
- Las mutaciones de los hitos 280 a 311 están pendientes.

## 2026-10-08 — Por dónde sale la voz de un jugador de otra consola

Una función en `networking/voice_routing.c`, de 1.287 bytes. De 1105 funciones se pasa a
1106, el 9,82 % de las del juego (el 6,304 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_route_player` | `0x00057eb0` | `networking/voice_routing.c` |

Era `unknown_00057eb0`, «la grande» que quedaba de quién oye a quién: lo
que se hace con cómo oye cada jugador de esta consola a uno de otra. Es
grande porque el compilador desenrolló su bucle de cuatro en cuatro; en C
son unas sesenta líneas.

### Lo que se sabe

- Los que le oyen de la primera manera van, un bit por cada uno, a
  `0x000583c0` (sin descompilar), que recorre los cuatro mandos: es la voz
  que sale por el auricular.
- De cada uno de los demás se apunta en `0xC8` de «quién oye a quién»
  (nuevo: un número por jugador de esta consola) si oye al jugador de la
  segunda manera o de la tercera.
- Si alguno le oye de la segunda manera, su voz sale por la «salida 4» (la
  que sigue a los cuatro mandos) con tres pares `{0,0}`, `{1,0}`, `{2,0}`,
  que cuadran con izquierda, derecha y centro delanteros de la biblioteca
  de sonido de la consola, y con prioridad 0.
- Si no, el más cercano de los que le oyen de la tercera manera (la que
  lleva distancia) va a `0x00058590` (sin descompilar), que coloca la voz
  según dónde está cada uno.
- Y si no hay ninguno, prioridad `NONE` en la salida 4: nunca suena ahí.
- La función nueva de la biblioteca es `0x00332e58`: da ese número a un
  jugador de otra consola en una salida. Los nombres de «salida» y
  «prioridad» son una conjetura, por los valores que se le pasan.
- `voice_talker_new` pasa de `voice.c` a `voice.h`, porque el juego la
  tiene metida en cada función que la usa, también fuera de `voice.c`.

### Sin comprobar

- En este hito se pasaron las pruebas de `voice_routing`, `voice` y
  `game_voice`.
- No se prueba que la biblioteca apague la voz entre las dos llamadas del
  caso de la segunda manera (el juego vuelve a mirar si está arrancada).
- Las mutaciones de los hitos 280 a 310 están pendientes.

## 2026-10-08 — El botón de hablar al equipo

Una función en `networking/voice_routing.c`, de 358 bytes. De 1104 funciones se pasa a
1105, el 9,81 % de las del juego (el 6,261 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_team_button` | `0x00057620` | `networking/voice_routing.c` |

Era `unknown_00057620`, la que `voice_channels_update_team` (hito 307)
llama por cada jugador de esta consola para saber si está hablando a su
equipo.

### Lo que se sabe

- Mira dos botones del mando del jugador: el sexto de los analógicos y el
  primero de los digitales. Con el mando desenchufado no hace nada.
- Guarda dos momentos por jugador en «quién oye a quién»: en `0x194` cuándo
  empezó a hablar al equipo, y en `0x1D4` (nuevo) la última vez que pulsó o
  dijo algo.
- Empieza al pulsar, con el sonido 0 de la voz, si
  `voice_player_unknown_000589e0` dice que sí del jugador (está vivo).
- Sigue mientras pulse o hable (`game_voice_player_is_talking`), como mucho
  `unknown_004cf738` segundos.
- Termina, con el sonido 1, tras `unknown_004cf734` segundos (nuevo) sin
  pulsar ni hablar, al agotarse el tiempo o si el jugador muere. Si se agotó
  con el botón pulsado, no vuelve a empezar hasta que lo suelte.
- Los dos campos del jugador del hito 307 estaban nombrados al revés: el
  mando es el de `0x24` (es el que se pasa a esta función) y el de `0x28`,
  de 16 bits, es otro número sin nombre. Corregido.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 280 a 309 están pendientes.

## 2026-10-08 — Los que están fuera de la partida hablan entre ellos

Una función en `networking/voice_routing.c`, de 379 bytes. De 1103 funciones se pasa a
1104, el 9,80 % de las del juego (el 6,249 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_dead` | `0x000577d0` | `networking/voice_routing.c` |

Era `unknown_000577d0`, la última de las cuatro cosas que
`voice_channels_update` hace cada fotograma con una partida en marcha. Las
cuatro están ya descompiladas.

### Lo que se sabe

- Rellena el «tercer número» de cada jugador de esta consola: si
  `voice_player_unknown_000589e0` dice que no de él (está muerto), un bit
  por cada jugador de otra consola que también lo esté y con el que pueda
  hablar según la partida: cualquiera si la voz no va por equipos, y los
  de su equipo si va.
- Con esto los tres números de «quién oye a quién» tienen sentido: **cerca**
  (`0x4`, hito 305), **equipo** (`0x44`, hito 307) y **muertos** (`0x84`).
  `voice_channels_get_player` (hito 279) da los muertos a un muerto y
  cerca más equipo a un vivo.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 280 a 308 están pendientes.

## 2026-10-08 — El canal de equipo: quién oye a cada jugador de esta consola

Una función en `networking/voice_routing.c`, de 378 bytes. De 1102 funciones se pasa a
1103, el 9,79 % de las del juego (el 6,236 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_team` | `0x00057360` | `networking/voice_routing.c` |

Era `unknown_00057360`, la tercera de las cuatro cosas que
`voice_channels_update` hace cada fotograma con una partida en marcha.

### Lo que se sabe

- Rellena el «segundo número» de cada jugador de esta consola: un bit por
  cada jugador de otra consola que le oye por el canal de equipo. Solo
  mientras su «momento» sea reciente (`voice_channels_time_is_recent`,
  hito 296), y solo los que pueden oírle según la partida (hito 299).
- Ese momento lo pone `0x00057620` (sin descompilar) mirando los botones
  del mando del jugador: es el botón de hablar al equipo. Con eso cuadra
  todo: por cercanía se habla siempre, y al equipo mientras se pulsa.
- De cada jugador de la partida se leen dos cosas nuevas: un número en su
  `0x24` y su mando en `0x28` (16 bits); con cualquiera en NONE el jugador
  no se juega en esta consola.

### Batería completa

La del hito 296 terminó: 1.090 funciones, las 1.090 bien.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 280 a 307 están pendientes.

## 2026-10-08 — Llega algo que dijo un jugador, con la partida en marcha

Una función en `networking/voice_routing.c`, de 358 bytes. De 1101 funciones se pasa a
1102, el 9,78 % de las del juego (el 6,223 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_player_talked` | `0x00056df0` | `networking/voice_routing.c` |

Era `unknown_00056df0`, la que `voice_queue_read` (hito 283) llama cada vez
que llega algo que dijo un jugador de otra consola.

### Lo que se sabe

- Junta las tres funciones de los hitos 302 a 304: para cada jugador de
  esta consola pregunta cómo propone la partida que oiga al que habló
  (`voice_channels_get_how`), lo ajusta a ese jugador
  (`voice_player_hears`) y, si le oye por cercanía, apunta a qué distancia
  está (`voice_players_get_distance`). La lista de 16 se entrega a
  `0x00057eb0`.
- La tercera cosa de cada entrada de esa lista, que el hito 301 dejó sin
  nombre, es esa distancia: un real.
- Si alguien le oye por el canal de equipo, se guarda la hora en el `0x148`
  del que habló: es el «momento» que miraba el hito 298. Aquella función
  hace sonar el pitido cuando hace más de... no: cuando hace **menos** de
  tres décimas que llegó lo último, y luego lo borra; es decir, suena una
  vez por cada tanda de voz por el canal de equipo.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 306 están pendientes.

## 2026-10-08 — La voz por cercanía: quién está cerca de quién

Una función en `networking/voice_routing.c`, de 489 bytes. De 1100 funciones se pasa a
1101, el 9,77 % de las del juego (el 6,211 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_near` | `0x00057080` | `networking/voice_routing.c` |

Era `unknown_00057080`, la segunda de las cuatro cosas que
`voice_channels_update` hace cada fotograma con una partida en marcha.

### Lo que se sabe

- Es la voz por cercanía de Halo 2. Cada fotograma, para cada jugador de
  esta consola, se mira a qué distancia está cada jugador de otra consola,
  y se le da un bit si está cerca: a no más de lo que diga la partida (un
  real en `0x290` de lo que apunta `0x00510c94`) o de **8 unidades** si no
  dice nada, más **una y media** de margen.
- Ese es el «primer número» de «quién oye a quién» que usaba la tercera
  manera de oír (hito 303): la tercera es la de cercanía.
- La posición de cada jugador sale de su objeto, con la reserva del hito
  304: quien acaba de morir sigue oyendo y sonando desde donde cayó.

### Corregido

- `voice_channels_time_is_recent` (hito 296) queda exacta también con más
  de 16 millones de ticks: está escrita con el mismo tipo de real que el
  juego (hito 304), y la prueba incluye ahora esos casos (1.380 en total).
  El «sin comprobar» de aquel hito ya no hace falta.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 305 están pendientes.

## 2026-10-08 — A qué distancia están dos jugadores: 1.100 funciones

Una función en `networking/voice_routing.c`, de 219 bytes. De 1099 funciones se pasa a
1100, el 9,76 % de las del juego (el 6,195 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_players_get_distance` | `0x00057a80` | `networking/voice_routing.c` |

### Lo que se sabe

- Da la distancia entre el objeto de un jugador y el de otro (dónde está
  cada objeto lo dice `0x000caf90`, sin descompilar), o -1 si la voz no
  tiene a alguno o alguno no tiene objeto. Es la base de la voz por
  cercanía.
- Del primer jugador vale el objeto de su `0x2c` o, si no tiene, el de su
  `0x30`; del segundo, solo el de `0x2c`. Así que `0x2c` es el objeto que
  el jugador lleva ahora (su cuerpo) y `0x30` uno de reserva (seguramente
  el último que tuvo, para seguir oyendo desde donde murió).

### La cuenta, igual que el juego bit a bit

- El juego hace esta cuenta con las instrucciones antiguas de reales del
  procesador, que guardan los pasos intermedios con más sitio para el
  exponente. Con las de `source/` (las modernas) el resultado difería en
  las últimas cifras cuando las distancias eran minúsculas (del orden de
  10⁻²⁰): los cuadrados no cabían en un real corriente.
- Escrita con el mismo tipo de real que usa el juego ahí, las 160 pruebas
  dan exactamente lo mismo, con distancias de 10⁻²⁰ a 10¹⁰.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 304 están pendientes.

## 2026-10-08 — Cómo hace la partida que se oiga a un jugador

Una función en `networking/voice_routing.c`, de 307 bytes. De 1098 funciones se pasa a
1099, el 9,76 % de las del juego (el 6,187 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_get_how` | `0x00057c20` | `networking/voice_routing.c` |

Es la que propone la manera (1, 2 o 3) que `voice_player_hears` (hito 302)
ajusta después, con una partida en marcha.

### Lo que se sabe

- Aquí se ve por fin qué son las tres maneras:
  - **la segunda** es la de los dos jugadores de los que
    `voice_player_unknown_000589e0` dice que no: por lo que mira esa función
    (un jugador sin objeto, o con poco tiempo desde un momento de su
    objeto), son los muertos, que se oyen entre ellos;
  - **la primera** es la del canal de equipo: cuando la función recibe un
    1 y los dos pueden oírse según las reglas de la partida (hito 299);
  - **la tercera** es la de cercanía: cuando el otro está en el primer
    número que «quién oye a quién» guarda del jugador de esta consola, que
    será «a quién tiene cerca».
- Y a un muerto no le oye un vivo: si esa función dice que no solo del
  otro, nada.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 303 están pendientes.

## 2026-10-08 — Cómo oye un jugador de esta consola a uno de otra

Una función en `networking/voice_routing.c`, de 309 bytes. De 1097 funciones se pasa a
1098, el 9,75 % de las del juego (el 6,177 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_player_hears` | `0x00057d60` | `networking/voice_routing.c` |

Era `unknown_00057d60`, la que decide cada entrada de la lista del hito
301.

### Lo que se sabe

- Hay tres maneras de oír a alguien, además de «no le oye». La partida
  propone una (1, 2 o 3), y esta función la ajusta al jugador que escucha:
  - no le oye si alguno de los dos ha silenciado al otro, si el que
    escucha no tiene mando o su ajuste es el 3 (sin voz);
  - con el ajuste 2, siempre de la primera manera;
  - con el ajuste 1, de la segunda (salvo que fuera la tercera);
  - la manera 1 que propone la partida pasa a ser la segunda si el que
    escucha no tiene voz él mismo.
- Y una regla más: si se le oye de la segunda o la tercera manera y
  **cualquier** jugador de esta consola le ha silenciado, no se le oye. Por
  lo que hace, la segunda y la tercera son las que suenan por los altavoces
  de la tele, que oyen todos los de la consola, y la primera la de los
  auriculares.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 302 están pendientes.

## 2026-10-08 — Quién oye a quién, cada fotograma, fuera de la partida

Dos funciones en `networking/voice_routing.c`, de 464 bytes entre las dos. De 1095 funciones se pasa a
1097, el 9,74 % de las del juego (el 6,167 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_by_team` | `0x00057960` | `networking/voice_routing.c` |
| `voice_players_same_team` | `0x00057b60` | `networking/voice_routing.c` |

`voice_channels_update_by_team` era `unknown_00057960`, lo que
`voice_channels_update` (hito 279) hace cada fotograma en los estados 2 y
3 de la red.

### Lo que se sabe

- Para cada jugador de otra consola que tiene voz, el juego arma una lista
  de 16 entradas, una por jugador: cómo le oye cada jugador de **esta**
  consola (0 si no le oye). Esa lista se entrega a `0x00057eb0`, que es la
  función grande de este archivo (1.287 bytes) y la que al final le dice a
  la biblioteca cómo mezclar las voces.
- Los jugadores a los que alguien de esta consola oye quedan apuntados con
  un bit en `0xc4`.
- `voice_players_same_team` dice si dos jugadores son del mismo equipo, y
  solo importa en el tercer estado: en los demás contesta siempre que sí.
- Cómo se oye a alguien lo decide `0x00057d60` (sin descompilar), que
  recibe ese «mismo equipo» en un registro: visto al leerla antes de
  escribir a quien la llama, como manda el aviso del hito 300.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 301 están pendientes.

## 2026-10-08 — Hito 300: los sonidos de la voz

Dos funciones en `networking/voice_routing.c`, de 217 bytes entre las dos. De 1093 funciones se pasa a
1095, el 9,72 % de las del juego (el 6,151 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_play_sound_by_number` | `0x000574e0` | `networking/voice_routing.c` |
| `voice_play_sound` | `0x00057530` | `networking/voice_routing.c` |

`voice_play_sound_by_number` era `unknown_000574e0`, el sonido que el hito
298 hace sonar.

### Lo que se sabe

- Los sonidos de la voz (el pitido de cuando alguien deja de hablar, y los
  demás) están en los datos del juego: las «globals» del juego apuntan a
  un tag (`0x16c`) que tiene una lista de sonidos, y se piden por su
  número.
- El sonido es **para un jugador de esta consola**. Si ese jugador no
  quiere voz (ajuste 3) o su cuenta no la permite, no suena. Si tiene el
  ajuste 1 o no tiene voz, suena para todos, por los altavoces. Y si no,
  suena solo en los auriculares de su mando: hay una salida de sonido por
  mando (`0x14000140` más el número del mando).
- Es la primera función de la voz que lee los tags del mapa.

### Corregido

- En `voice_channels_update_ended` (hito 298) a la llamada del sonido le
  faltaba un argumento: **para qué jugador** suena, que el juego pasa en un
  registro. La prueba no lo notaba porque la función estaba simulada con
  los argumentos que se le conocían. Ahora compara también el jugador.
- Es el mismo tipo de descuido que el del hito 283: una función aún sin
  descompilar puede recibir cosas en registros que no se ven hasta leerla.
  Apuntado en `ESTADO.md`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- A `voice_play_sound_by_number` el juego le da un argumento más, que no
  mira. El compilador de `source/` no lo pasa, y no se compara.
- Un mando con un número fuera de 0 a 3: el juego salta por una tabla de
  cuatro.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 300 están pendientes.

## 2026-10-08 — Si un jugador de esta consola puede oír a uno de otra

Una función en `networking/voice_routing.c`, de 230 bytes. De 1092 funciones se pasa a
1093, el 9,70 % de las del juego (el 6,144 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_players_can_hear` | `0x00057270` | `networking/voice_routing.c` |

Era `unknown_00057270`, la condición que `voice_channels_update_ended`
(hito 298) dejó sin nombre.

### Lo que se sabe

- Un jugador de esta consola puede oír a uno de otra según el tipo de
  partida. Si la partida no restringe la voz
  (`game_voice_unknown_00053d40` dice que no), siempre. Si la restringe,
  solo a los de su mismo equipo; salvo en un caso
  (`game_voice_unknown_00053d90`) en que lo decide otra cosa de cada
  jugador (`0x00192950`, sin descompilar): se oyen si ninguno de los dos la
  cumple.
- Con esto `game_voice_unknown_00053d40` queda bastante claro: es «en esta
  partida la voz va por equipos».

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Una sesión sin jugadores con la voz por equipos: el juego lee los
  equipos de una lista que no hay.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 299 están pendientes.

## 2026-10-08 — Quién oye a quién, dentro de la partida: el primer paso de cada fotograma

Una función en `networking/voice_routing.c`, de 276 bytes. De 1091 funciones se pasa a
1092, el 9,69 % de las del juego (el 6,136 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_update_ended` | `0x00056f60` | `networking/voice_routing.c` |

Era `unknown_00056f60`, la primera de las cuatro cosas que
`voice_channels_update` (hito 279) hace cada fotograma con una partida en
marcha.

### Lo que se sabe

- «Quién oye a quién» guarda otro momento por jugador, en `0x148`. Cada
  fotograma mira a cada jugador de otra consola: si ese momento fue hace
  menos de tres décimas de segundo (y `voice_player_unknown_000589e0` dice
  que no), suena un sonido de la voz (el 2, por `0x000574e0`) para cada
  jugador de esta consola que cumpla una condición (`0x00057270`, sin
  descompilar) y con el que no haya silencio en ninguno de los dos
  sentidos; y el momento se borra.
- Lo que aún no se sabe es qué momento es ese ni qué condición es aquella:
  el nombre de la función es una suposición.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 298 están pendientes.

## 2026-10-08 — Quién oye a quién fuera de la partida: todos, o los del equipo

Una función en `networking/voice_routing.c`, de 368 bytes. De 1090 funciones se pasa a
1091, el 9,68 % de las del juego (el 6,127 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_get_player_by_team` | `0x00056c60` | `networking/voice_routing.c` |

Era `unknown_00056c60`, la que `voice_targets_update` (hito 280) llama en
los estados 2 y 3 de la red.

### Lo que se sabe

- Con esto se entienden los tres estados que la voz saca de la red
  (hito 274). En el **1** hay partida en marcha y quién oye a quién sale
  del mundo del juego (lo que queda por descompilar: distancias). En el
  **2** se oyen todos los jugadores de la sesión. En el **3**, solo los del
  mismo equipo.
- El equipo de cada jugador es un byte con signo en su `0x124` dentro de la
  lista de la sesión; -1 es «sin equipo», y sin equipo no se oye a nadie.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- El tercer estado con una sesión sin jugadores: el juego lee los equipos
  de una lista que no hay y se para.
- La batería completa del hito 296 corre aparte; las mutaciones de los
  hitos 280 a 297 están pendientes.

## 2026-10-08 — Limpieza de la lista de funciones, las mutaciones de la voz y un reloj

Una función en `networking/voice_routing.c`, de 62 bytes. De 1089 funciones se pasa a
1090, el 9,68 % de las del juego (el 6,114 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_channels_time_is_recent` | `0x00057790` | `networking/voice_routing.c` |

### Lo que se sabe

- «Quién oye a quién» guarda un momento de la partida por jugador
  (`0x194`). La función dice si ese momento fue hace menos de cierto tiempo
  (`0x004cf738`, en segundos): seguramente «acaba de hablar».
- El juego hace esta cuenta con las instrucciones antiguas de reales del
  procesador, y toma los ticks como un número sin signo.

### Cambiado: la lista de funciones, sin direcciones repetidas

- Cinco funciones ya descompiladas en sesiones anteriores seguían
  llamándose por su dirección desde `main.c` y `cache_files.c`, con una
  fila de más cada una en `symbols\functions.csv`: `main_game_unload`,
  `main_game_restart`, `main_game_change_update`, `main_game_load_failed` y
  `geometry_cache_delete_tag_blocks`. Ahora se llaman por su nombre y las
  cinco filas sobrantes ya no están. La lista no tiene ninguna dirección
  repetida.
- Corregida una frase de la entrada del hito 295, que decía que la cuenta
  de funciones apuntadas bajaba y no era así.

### Mutaciones

Las de `game_voice.c` y `voice_routing.c` tal como estaban en el hito 279
(hitos 269 a 279) terminaron: setenta y un cambios a mano, y las pruebas
notan sesenta y nueve.

- Uno no altera nada: mirar que la sesión tenga jugadores antes de un
  bucle que sin ellos no encuentra a nadie.
- El otro era un hueco de verdad en la prueba de la consola elegida, ya
  cerrado en el hito 294.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `main`,
  `main_game`, `game`, `scenario`, `cache` y `map`.
- `voice_channels_time_is_recent` con más de 16 millones de ticks desde
  aquel momento (unos tres días de partida): el juego guarda ahí más cifras
  del tiempo que `source/`, y el resultado podría diferir en el último bit.
- Las mutaciones de los hitos 280 a 296 están pendientes.

## 2026-10-08 — La partida en red, cada fotograma

Una función en `networking/network_life_cycle.c`, de 106 bytes. De 1088 funciones se pasa a
1089, el 9,67 % de las del juego (el 6,112 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_update` | `0x00059940` | `networking/network_life_cycle.c` |

Era `unknown_00059940`, la que el bucle principal (`main.c`) llama cada
fotograma. Con ella, de `0x000591e0` a `0x00059ab0` no queda nada por
descompilar.

### Lo que se sabe

- Cada fotograma, si hay partida en red: lo que la partida está haciendo
  hace su trabajo (`0x0006dec0`, que recibe dónde se guarda el estado) y lo
  hacen dos cosas más (`0x0047708c` y `0x0047712c`, del mismo tipo).
- Si se está vigilando una sesión (sus ocho bytes en `0x00510528`) y ya no
  aparece, se levanta el primer flag y se baja el segundo: es el aviso de
  «la sesión que seguías ha desaparecido».

### Cambiado

- `main.c`, `main_game.c` y `game_state.c` llamaban a esta función y a
  `network_life_cycle_end` (hito 289) por su dirección, con una fila aparte
  en la lista de funciones. Ahora las llaman por su nombre y las dos filas
  sobrantes ya no están. (La cuenta de funciones «apuntadas sin descompilar»
  sube aun así en dos: este hito apunta cuatro nuevas.)
- Quedan en la lista cinco direcciones repetidas de sesiones anteriores,
  con el mismo arreglo pendiente (`0x0018eb80`, `0x0018e700`, `0x0018e810`,
  `0x0018efa0` y `0x0012e150`). Apuntado en `ESTADO.md`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `main`,
  `main_game`, `game_state` y `fonts`.
- Los tamaños de `0x0047708c` y `0x0047712c` son suposiciones.
- Las mutaciones de los hitos 280 a 295 están pendientes.

## 2026-10-08 — Dos encargos más de la partida en red, y un hueco de las pruebas cerrado

Dos funciones en `networking/network_life_cycle.c`, de 86 bytes entre las dos. De 1086 funciones se pasa a
1088, el 9,66 % de las del juego (el 6,109 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_unknown_00059200` | `0x00059200` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059230` | `0x00059230` | `networking/network_life_cycle.c` |

Con ellas, de `0x000591e0` a `0x00059ab0` solo queda `0x00059940`.

### Lo que se sabe

- Las dos levantan los dos flags (si `0x00510518` no es 0) y pasan tres
  números a algo que vive en `0x005273f8` (`0x0006f200` y `0x0006f3a0`, sin
  descompilar). Lo que apunta `0x00527400` (hito 287) está dentro de eso
  mismo, en su `0x8`.

### Mutaciones: un hueco de verdad

Las mutaciones de los hitos 269 a 279, que siguen corriendo, encontraron
que la prueba de `game_voice_update_chosen_peer` (hito 276) no notaba si
se quitaba una de las condiciones para elegir consola: el mínimo del
«segundo número». Los casos al azar nunca dejaban a una consola que
fallara **solo** en eso sin otra mejor que elegir.

- Añadidos doce casos a mano: una sola consola candidata que falla en una
  única cosa cada vez (o en ninguna).
- Comprobado a mano: con la condición quitada las pruebas ahora fallan, y
  con el código bien pasan las 312.

El otro cambio sin notar hasta ahora no altera nada (hito 293).

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de
  `game_voice_update_chosen_peer`.
- Las dos funciones de este hito acaban en una llamada y devuelven lo que
  esa devuelva, si es que devuelve algo: se suman a la lista de
  `ESTADO.md`.
- Las mutaciones de los hitos 280 a 294 están pendientes.

## 2026-10-08 — Pedir la lista de silenciados a la biblioteca: `voice_mute_list.c`, completo

Tres funciones en `networking/voice_mute_list.c`, de 359 bytes entre las tres: las que faltaban. De 1083 funciones se pasa a
1086, el 9,64 % de las del juego (el 6,106 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_mute_list_startup` | `0x00058a90` | `networking/voice_mute_list.c` |
| `voice_mute_list_stop` | `0x00058b00` | `networking/voice_mute_list.c` |
| `voice_mute_list_request` | `0x00058b70` | `networking/voice_mute_list.c` |

`voice_mute_list_request` era `unknown_00058b70`. Con ellas el archivo
tiene sus 7 funciones, de `0x00058a50` a `0x00058cb0`.

### Lo que se sabe

- Lo que el juego le pide a la biblioteca de red no se contesta en el acto:
  se apunta en una tabla de «cosas pedidas» (`0x0006b730` da una nueva,
  `0x0006b910` dice dónde está, `0x0006b640` la da por terminada) con su
  clase, para quién es y lo que la biblioteca dio para preguntar cómo va.
- Aquí hay dos clases: la `0xb`, «prepárate para llevar las listas», una
  para todos los mandos; y la `0xc`, «dame la lista de este mando», una
  por mando: hasta 250 jugadores, en la memoria de `game_voice.c`, con el
  sitio donde la biblioteca dejará cuántos son.
- Mientras la biblioteca no conteste, la cuenta del mando es NONE, que es
  lo que el resto del código entiende por «lista no conocida» (hito 277).
- Al dejar de pedir la lista de un mando, si ya no se pide ninguna, la
  biblioteca deja de llevar las listas.
- Un descuido del juego: si la biblioteca no puede prepararse, el juego da
  por terminado lo que tenía apuntado (que era nada) y no lo que acababa de
  pedir, que se queda en la tabla. Está escrito igual.

### Mutaciones

Las de los hitos 269 a 279 siguen corriendo aparte (van por la mitad, con
un solo cambio sin notar, que no altera nada: mirar que haya jugadores en
la sesión antes de un bucle que sin ellos no encuentra nada).

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 280 a 293 están pendientes.

## 2026-10-08 — Preparar una sesión de red

Una función en `networking/network_session.c`, de 242 bytes. De 1082 funciones se pasa a
1083, el 9,61 % de las del juego (el 6,094 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_initialize` | `0x00059ad0` | `networking/network_session.c` |

### Lo que se sabe

- Una sesión mide `0x78b0` bytes (casi 31 KB). Al prepararla queda sin
  estado, sin saber qué consola es esta, y apuntada en dos sitios: la tabla
  de las sesiones y algo que tiene una entrada de `0x24` bytes por sesión.
- Dentro lleva **dos** listas de quién está (`0x4c` y `0x24e0`, de `0x2494`
  bytes cada una: la de ahora y, por lo que parece, la que viene) y otras
  dos cosas de `0x14b0` bytes (`0x4978` y `0x5e28`). Las cuatro empiezan
  con un NONE que significa «no dice nada»: es lo que `game_voice.c` mira
  antes de fiarse de la lista (hito 271).
- El tamaño de la lista casa con el que salió de la voz: 16 jugadores de
  `0x13c` bytes desde su `0x10d4`.

### Aviso

- Una función que acaba saltando a otra devuelve lo que esa devuelva. En
  el hito 291 dos funciones dadas por «no devuelven nada» sí devolvían. De
  este archivo y del anterior quedan cinco que acaban así en funciones aún
  sin descompilar (`0x00059360`, `0x000593a0`, `0x00059530`, `0x000595e0` y
  `game_voice_read_packet`): hay que mirarlas cuando se sepa qué devuelve
  aquello a lo que saltan. Apuntado en `ESTADO.md`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 269 a 279 corren aparte; las de 280 a 292
  están pendientes.

## 2026-10-08 — Arrancar la sesión de la partida en red

Dos funciones en `networking/network_life_cycle.c`, de 213 bytes entre las dos. De 1080 funciones se pasa a
1082, el 9,60 % de las del juego (el 6,085 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_unknown_00059890` | `0x00059890` | `networking/network_life_cycle.c` |
| `network_life_cycle_request_new1` | `0x00059a30` | `networking/network_life_cycle.c` |

`network_life_cycle_unknown_00059890` era `unknown_00059890`, la que
llaman las dos primeras funciones del archivo (hito 287).

### Lo que se sabe

- Arranca la sesión si no hay ninguna: se lo pide a `0x00059bd0` (diez
  argumentos, sin descompilar) y, si se pudo, hace una cosa más con la
  sesión recién arrancada (`0x00065770`). Devuelve si arrancó.
- La petición de la primera clase lleva cuatro números, cada uno con su
  tope: 15, 15, 63 y 1.023.

### Corregido

- `network_life_cycle_unknown_000591e0` y `..._000591f0` (hito 287) no
  devolvían nada. Devuelven lo que diga la función de arriba: si la sesión
  arrancó. Las pruebas no lo notaban porque no comparaban ningún resultado;
  ahora sí.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 269 a 279 corren aparte; las de 280 a 291
  están pendientes.

## 2026-10-08 — Más de la partida en red: qué sesión, y lo que se le pide

Siete funciones en `networking/network_life_cycle.c`, de 388 bytes entre las siete. De 1073 funciones se pasa a
1080, el 9,59 % de las del juego (el 6,078 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_get_any_session` | `0x00059780` | `networking/network_life_cycle.c` |
| `network_life_cycle_get_state_session` | `0x000597d0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059840` | `0x00059840` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000598f0` | `0x000598f0` | `networking/network_life_cycle.c` |
| `network_life_cycle_request_new2` | `0x000599b0` | `networking/network_life_cycle.c` |
| `network_life_cycle_request_new3` | `0x000599f0` | `networking/network_life_cycle.c` |
| `network_life_cycle_request_new4` | `0x00059ab0` | `networking/network_life_cycle.c` |

`network_life_cycle_unknown_000598f0` era `unknown_000598f0`, la que el
hito 287 dejó apuntada.

### Lo que se sabe

- Hay cuatro maneras de pedir «la sesión»: la primera, la otra, cualquiera
  de las dos (la otra primero) y la del estado en que está la partida. La
  última se parece a la «de ahora» del hito 289, sin el caso especial del
  sexto estado.
- Una sesión se conoce por ocho bytes (en su `0x1c`, y un byte al lado que
  dice si los tiene). El juego guarda los de una (`0x00510528`) y levanta
  dos flags cuando se toca esa misma sesión.
- A la partida en red se le piden cosas con una estructura de 28 bytes: de
  qué clase es la petición y unos números que valen 0 salvo uno, que no
  puede pasar de 1.023. Hay tres funciones que la rellenan, para las
  clases 2, 3 y 4.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- `network_life_cycle_unknown_000598f0` con una sesión que no tiene sus
  ocho bytes: el juego compara con lo que haya en la dirección 0 y se
  para.
- Las mutaciones de los hitos 269 a 279 corren aparte; las de 280 a 290
  están pendientes.

## 2026-10-08 — La partida en red deja lo que hace, y la sesión de cada momento

Tres funciones en `networking/network_life_cycle.c`, de 351 bytes entre las tres. De 1070 funciones se pasa a
1073, el 9,53 % de las del juego (el 6,064 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_end` | `0x000593e0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000594a0` | `0x000594a0` | `networking/network_life_cycle.c` |
| `network_life_cycle_get_current_session` | `0x000596d0` | `networking/network_life_cycle.c` |

`network_life_cycle_get_current_session` era `unknown_000596d0`, la que
`game_voice_update_session_source` (hito 272) llama para saber con qué
sesión va la voz.

### Lo que se sabe

- Cada una de las diez cosas que la partida en red puede estar haciendo
  tiene «quien la hace»: un objeto con sus funciones
  (`network_life_cycle_handlers`, `0x00527338`). Al dejar lo que hace, la
  partida suelta las dos sesiones, avisa a quien lo hacía de que se pasa a
  no hacer nada, y a «no hacer nada» de que se viene de aquello. Es la
  primera vez que las pruebas siguen llamadas por las funciones de un
  objeto del juego.
- La sesión «de ahora» depende de lo que se esté haciendo: la primera en
  los cinco primeros estados, la otra en los tres últimos, y en el sexto la
  primera salvo en un caso en que se da la otra.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `game_voice`.
- Las mutaciones de los hitos 269 a 279 corren aparte; las de 280 a 289
  están pendientes.

## 2026-10-08 — La lista de «a quién no quiero oír», y un jugador y su objeto

Cinco funciones: cuatro en un archivo nuevo, `networking/voice_mute_list.c`, y una en
`networking/voice_routing.c`; 472 bytes entre las cinco. De 1065 funciones se pasa a
1070, el 9,50 % de las del juego (el 6,052 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_mute_list_reset` | `0x00058a50` | `networking/voice_mute_list.c` |
| `voice_mute_list_add` | `0x00058c10` | `networking/voice_mute_list.c` |
| `voice_mute_list_remove` | `0x00058c60` | `networking/voice_mute_list.c` |
| `voice_mute_list_has_player` | `0x00058cb0` | `networking/voice_mute_list.c` |
| `voice_player_unknown_000589e0` | `0x000589e0` | `networking/voice_routing.c` |

`voice_player_unknown_000589e0` era `unknown_000589e0`, la que
`voice_channels_get_player` (hito 279) llamaba sin conocer.

### Lo que se sabe

- La lista de jugadores a los que el jugador de un mando no quiere oír
  (hito 277) la guarda la biblioteca de red de la consola. El juego le dice
  «a este no quiero oírle» (`0x003a084c`) o «a este sí otra vez»
  (`0x003a086e`), y si la biblioteca lo acepta vuelve a pedirle la lista
  entera (`0x00058b70`, sin descompilar). Solo lo hace con una sesión de
  red en su primer estado y con la lista ya conocida.
- Preguntar si un jugador está en la lista es recorrerla comparando sus
  doce bytes, como hace `game_voice_update_local_player` por dentro.
- `voice_player_unknown_000589e0` mira el objeto de un jugador de la
  partida: dice que sí si el jugador tiene cierto número, si no tiene
  objeto, o si no ha pasado más de un segundo desde un momento apuntado en
  su objeto. Es la primera función de la voz que mira el mundo del juego
  (la tabla de objetos y el reloj de la partida), y con reales: la prueba
  incluye los límites exactos y un tiempo que no es un número.

### Batería completa

La del hito 278 terminó: 1.023 funciones, las 1.023 bien. Es la primera
con `voice.c` y `game_voice.c` completos.

### Sin comprobar

- En este hito se pasaron las pruebas de los dos archivos.
- Las mutaciones de los hitos 269 a 279 corren aparte; las de 280 a 288
  están pendientes.

## 2026-10-08 — Nueve funciones pequeñas más de la partida en red

Nueve funciones en `networking/network_life_cycle.c`, de 430 bytes entre las nueve. De 1056 funciones se pasa a
1065, el 9,45 % de las del juego (el 6,038 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_life_cycle_unknown_000591e0` | `0x000591e0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000591f0` | `0x000591f0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059280` | `0x00059280` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059360` | `0x00059360` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000593a0` | `0x000593a0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059530` | `0x00059530` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059590` | `0x00059590` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000595e0` | `0x000595e0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059610` | `0x00059610` | `networking/network_life_cycle.c` |

Todas llevan su dirección en el nombre: se sabe lo que hacen, pero aún no
para qué. Casi todas miran el estado de la partida en red o de una de sus
sesiones y pasan el encargo a una función que todavía no está descompilada.

### Lo que se sabe

- Tres cosas más de la partida en red tienen una sesión: lo que apunta
  `0x00527400` (en su `0x30`), lo que apunta `0x00527fa0` (en su `0x38`) y
  lo del sexto estado, `0x00527500`, que en su `0x8` apunta a algo que la
  tiene en `0x30`.
- Dos funciones hacen lo mismo con cada una de las dos sesiones
  (`0x00059360` y `0x000593a0`): si hay sesión, `0x000598f0` y luego una de
  dos cosas según un sí o un no.
- Varias repiten «la sesión dice quién está y esta consola la lleva», que
  es `network_session_local_peer_is_host` metida dentro.
- Quedan apuntadas siete funciones de las sesiones.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 287
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — La partida en red y sus sesiones: catorce funciones pequeñas

Catorce funciones en dos archivos nuevos, `networking/network_session.c` (4) y
`networking/network_life_cycle.c` (10), de 480 bytes entre todas. De 1042 funciones se pasa a
1056, el 9,37 % de las del juego (el 6,022 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `network_session_unknown_00058d20` | `0x00058d20` | `networking/network_session.c` |
| `network_session_local_peer_is_host` | `0x00058d50` | `networking/network_session.c` |
| `network_session_established` | `0x00058d70` | `networking/network_session.c` |
| `network_session_unknown_00058d90` | `0x00058d90` | `networking/network_session.c` |
| `network_life_cycle_in_state5` | `0x00059260` | `networking/network_life_cycle.c` |
| `network_life_cycle_get_state` | `0x000592d0` | `networking/network_life_cycle.c` |
| `network_life_cycle_local_peer_is_host` | `0x000592f0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059330` | `0x00059330` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059470` | `0x00059470` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_000594f0` | `0x000594f0` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059500` | `0x00059500` | `networking/network_life_cycle.c` |
| `network_life_cycle_unknown_00059570` | `0x00059570` | `networking/network_life_cycle.c` |
| `network_life_cycle_get_session` | `0x00059670` | `networking/network_life_cycle.c` |
| `network_life_cycle_get_other_session` | `0x000596a0` | `networking/network_life_cycle.c` |

Las dos últimas eran `unknown_00059670` y `unknown_000596a0`, de donde
`game_voice.c` saca la sesión con la que va la voz (hito 271). Lo que falta
de la voz («quién oye a quién») son funciones grandes con distancias; antes
van estas, que son la base de todo lo de red.

### Lo que se sabe

- Una sesión tiene un estado, de 0 (no hay) a 8. Del 3 al 8 dice quién
  está en ella: es la condición que `game_voice.c` miraba sin nombre.
- La consola que lleva la sesión está en su `0x50`; esta consola, en
  `0x72d8`. «Esta consola lleva la sesión» es que sean la misma.
- La partida en red tiene dos sesiones (`0x00527364` y `0x0052736c`) y un
  número que dice qué está haciendo (`0x00527334`, de 0 a 9), que ya
  estaba en `main_game.c`.
- Los nombres de los dos archivos son suposiciones (el segundo es el del
  archivo de Halo 3 para lo mismo).

### Sin comprobar

- En este hito se pasaron las pruebas de los dos archivos y las de
  `game_voice`.
- El tamaño de `0x00527500` no se conoce.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 286
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — A qué consolas va cada cosa: directa o por la consola elegida

Dos funciones en `networking/voice_routing.c`, de 969 bytes entre las dos. De 1040 funciones se pasa a
1042, el 9,25 % de las del juego (el 6,007 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_route_peers` | `0x000565c0` | `networking/voice_routing.c` |
| `voice_route_assign` | `0x00056790` | `networking/voice_routing.c` |

`voice_route_peers` era `unknown_000565c0`. Es la decisión que le faltaba al
camino de la voz: los dos números de 16 bits que acompañan a cada cosa de
una cola.

### Lo que se sabe

- **El primer número** es un bit por cada consola a la que esta manda la
  cosa directamente. **El segundo**, un bit por cada consola a la que tiene
  que reenviarla la consola elegida para la voz.
- Va directa a las consolas a las que esta está unida, hasta un máximo
  (`game_voice_unknown_00054cc0`: cuántas consolas admite cada envío, según
  el estado de la red). Si la sesión sale del segundo sitio, primero a las
  consolas que las dos sesiones comparten.
- Lo que no cabe en ese máximo, y las consolas a las que esta no está
  unida, va por la elegida, siempre que haya una, no sea esta y esta esté
  unida a ella. Entonces el último hueco del máximo se guarda para la
  elegida.
- `0x004c987c` es el modo: con 1 o 2, todo va por la elegida cuando se
  puede; con 2 y sin poder, a una sola consola.
- `voice_route_assign` es quien reparte los bits, de la primera consola en
  adelante.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 285
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — De jugadores a consolas

Una función en `networking/voice_routing.c`, de 335 bytes. De 1039 funciones se pasa a
1040, el 9,23 % de las del juego (el 5,974 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_targets_to_peers` | `0x00056990` | `networking/voice_routing.c` |

Era `unknown_00056990`, la que `voice_unknown_00527104_get_words` (hito
280) llama con el número de «a quién habla un jugador».

### Lo que se sabe

- Convierte jugadores en consolas: de un bit por jugador saca un bit por
  cada consola en la que está alguno de ellos, sin contar esta. Es el paso
  que faltaba entre «a quién habla cada jugador» (hito 280) y «a qué
  consolas va cada cosa de la cola» (hito 281).
- Sin sesión, o si no se sabe qué consola es esta, no va a ninguna.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 284
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — Una cola dentro de lo que va de una consola a otra

Dos funciones en `networking/voice_routing.c`, de 937 bytes entre las dos. De 1037 funciones se pasa a
1039, el 9,22 % de las del juego (el 5,963 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_queue_write` | `0x00055c40` | `networking/voice_routing.c` |
| `voice_queue_read` | `0x00055e10` | `networking/voice_routing.c` |

Eran `unknown_00055c40` y `unknown_00055e10`. Con ellas está entero el
camino de lo que dice un jugador: de la biblioteca a su cola, de la cola a
lo que se manda a otra consola, y allí de vuelta a la biblioteca.

### Lo que se sabe

- **La consola elegida para la voz reenvía.** A las demás consolas cada
  cosa les llega con 11 bytes (un flag y los 10 de lo dicho) y la dan a su
  biblioteca. A la elegida le llegan 13: dos bytes más con un bit por cada
  consola a la que va. Si su propio bit está, la oye ella también; y si
  quedan otros, la mete en la cola del jugador para mandarla a esas
  consolas. Por eso lo que va a la elegida ocupa más (hito 281).
- Al escribir, cada cosa escrita deja de ser «para esa consola»; la que ya
  no es para ninguna se saca de la cola (`voice_queue_keep`).
- Una cosa sin consolas que va a la elegida lleva el bit de la propia
  elegida.
- Cada cola sabe de qué jugador es (`0xc`).
- El flag de cada cosa solo puede ser 0 o 1, y con él se avisa a «quién oye
  a quién» (`0x00056df0`, sin descompilar).

### Corregido

- En `voice_queues_build_packet` (hito 282) a quien escribe cada cola le
  faltaba un argumento: la cola, que el juego le da en un registro. Las
  pruebas no lo notaban porque esa función estaba simulada.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 283
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — Lo que una consola le manda a otra de la voz

Tres funciones en `networking/voice_routing.c`, de 642 bytes entre las tres. De 1034 funciones se pasa a
1037, el 9,21 % de las del juego (el 5,931 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_queue_keep` | `0x00056000` | `networking/voice_routing.c` |
| `voice_queues_build_packet` | `0x00056380` | `networking/voice_routing.c` |
| `voice_queues_read_packet` | `0x000564c0` | `networking/voice_routing.c` |

Las dos últimas eran `unknown_00056380` y `unknown_000564c0`.

### Cambiado

- Con ellas se sabe qué eran cuatro funciones de `game_voice.c` que
  llevaban su dirección en el nombre (hitos 271 y 272):
  `game_voice_build_packet` (`0x00053b00`), `game_voice_read_packet`
  (`0x00054810`), `game_voice_has_packet_for_peer` (`0x00054850`) y
  `game_voice_packet_size_for_peer` (`0x00054990`). Sus argumentos tienen
  ya nombre y tipo.

### Lo que se sabe

- Lo que una consola le manda a otra de la voz empieza con dos bytes: uno
  cuyo primer bit dice que la otra es la consola elegida para la voz, y
  cuántas colas vienen detrás (de 1 a 15). Cada cola empieza con su jugador
  (de 0 a 15) y cuántas cosas trae (de 1 a 15).
- Al escribir, no se escribe nada con menos de 15 bytes de sitio, y se
  para en la primera cola que no cabe. Al leer, lo que dice ser para la
  consola elegida solo lo acepta esa consola, y se deja de leer en cuanto
  algo no cuadra.
- Al leer hay un descuido del juego: una cola con una cuenta de cosas fuera
  de rango se salta sin avanzar a la siguiente, así que se vuelve a mirar
  la misma hasta agotar la cuenta de colas. Está escrito igual.
- `voice_queue_keep` deja en una cola solo las cosas cuyo bit está en un
  número, en su orden.
- Escribir y leer cada cola lo hacen `0x00055c40` y `0x00055e10`, sin
  descompilar.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `game_voice`.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 282
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — La cola de lo que dice un jugador, y cuánto hay que mandar a cada consola

Dos funciones en `networking/voice_routing.c`, de 590 bytes entre las dos. De 1032 funciones se pasa a
1034, el 9,18 % de las del juego (el 5,910 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_queue_add` | `0x00055b40` | `networking/voice_routing.c` |
| `voice_queues_size_for_peer` | `0x00056220` | `networking/voice_routing.c` |

Eran `unknown_00055b40` y `unknown_00056220`.

### Lo que se sabe

- Cada cosa de una cola mide 18 bytes: los dos números de 16 bits que dicen
  adónde va, un flag, el primer número otra vez y los 10 bytes de lo que
  dijo el jugador. En una cola caben 15; cuando va a entrar la decimosexta,
  la cola se vacía entera y se empieza de nuevo.
- Con cada cosa que entra, la cola suma uno a la cuenta de cada consola a
  la que va y apunta el bit de esa consola.
- `voice_queues_size_for_peer` dice cuántos bytes hay que mandar a una
  consola: 2, más 2 por cada cola que tenga algo para ella, más 11 por cada
  cosa (13 si esa consola es la elegida para la voz, que por tanto recibe
  algo más con cada una: seguramente a quién hay que reenviarla).
- `voice_queue_add` pregunta qué consola es esta y no usa la respuesta.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `game_voice`.
- Una cola con una cuenta que no es de una cola (negativa o enorme): el
  juego escribe donde le lleve.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 281
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — A quién habla cada jugador, cada fotograma

Dos funciones en `networking/voice_routing.c`, de 469 bytes entre las dos. De 1030 funciones se pasa a
1032, el 9,16 % de las del juego (el 5,890 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_targets_update` | `0x00055990` | `networking/voice_routing.c` |
| `voice_unknown_00527104_get_words` | `0x00056590` | `networking/voice_routing.c` |

Eran `unknown_00055990` y `unknown_00056590`, las dos últimas que
`game_voice.c` llamaba de este archivo sin conocerlas.

### Lo que se sabe

- `voice_targets_update` decide, cada fotograma, a quién habla cada jugador
  de esta consola que tiene voz. Parte de los jugadores de otras consolas
  que hay en la sesión y va quitando: a los que él no quiere oír, a los que
  no le oyen según «quién oye a quién» (que depende del estado de la red),
  y a cada uno que no quiere oírle a él, no quiere voz o no tiene permiso.
  Lo que queda es un bit por jugador, y es el número que el hito 275 vio
  pasar camino de las colas.
- Es la función que junta todo lo de los hitos 270 a 277: los silencios,
  los tres flags de cada jugador y quién tiene voz.
- `voice_unknown_00527104_get_words` pide, para ese número, los dos números
  de 16 bits que da lo de `0x00527104` (dos funciones suyas quedan
  apuntadas). El primero es un bit por consola a la que va lo dicho.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `game_voice`.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 280
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — La voz entre consolas: a quién habla cada uno, las colas y quién oye a quién

Siete funciones en un archivo nuevo, `networking/voice_routing.c`, de 570 bytes entre las siete. De 1023 funciones se pasa a
1030, el 9,14 % de las del juego (el 5,874 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_targets_get` | `0x00055960` | `networking/voice_routing.c` |
| `voice_queues_reset` | `0x000560e0` | `networking/voice_routing.c` |
| `voice_queues_add` | `0x00056130` | `networking/voice_routing.c` |
| `voice_queues_go_to_peer` | `0x00056160` | `networking/voice_routing.c` |
| `voice_channels_update` | `0x00056b70` | `networking/voice_routing.c` |
| `voice_channels_get_player` | `0x00056bd0` | `networking/voice_routing.c` |
| `voice_channels_get_unknown84` | `0x00056dd0` | `networking/voice_routing.c` |

Cinco de ellas eran las `unknown_...` que `game_voice.c` llamaba sin
conocer. Es lo que viene en el juego después de `voice.c`.

### Lo que se sabe

- Las cuatro «cosas» a las que la voz entrega lo que dicen los jugadores
  (hito 275) están una detrás de otra en la memoria del juego, y cada una
  empieza con un byte que dice si está en marcha:
  - `0x005259b8`: a quién habla cada jugador (un número por jugador);
  - `0x00525a00`: una cola por jugador con lo que ha dicho, de `0x170`
    bytes: si está en uso, cuántas cosas tiene, un bit por consola a la que
    van, las cosas y cuántas van a cada consola. Mide `0x1704` bytes, no 4
    como se supuso;
  - `0x00527104`: cuatro bytes de los que solo se conoce el primero;
  - `0x00527108`: quién oye a quién, tres números por jugador.
- A una cola solo entra lo dicho si mide exactamente 10 bytes.
- `voice_channels_update` hace cuatro cosas cada fotograma en el estado 1
  de la red y una sola en el 2 y el 3 (las cinco, sin descompilar).

### Mutaciones

Las de los hitos 266 a 268 terminaron: 24 de 24, 32 de 32 y 13 de 14. La
que las pruebas no notan no cambia nada: `voice_text_append` escribiendo
un carácter más, que luego se pisa con el cero del final.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `game_voice`.
- La batería completa del hito 278 y las mutaciones de los hitos 269 a 279
  corren aparte o están pendientes; su resultado irá en una entrada
  posterior.

## 2026-10-08 — Si los jugadores de la sesión son los mismos: `game_voice.c`, completo

Una función en `networking/game_voice.c`, de 582 bytes: la última del archivo. De 1022 funciones se pasa a
1023, el 9,08 % de las del juego (el 5,855 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_update_session_players` | `0x00054590` | `networking/game_voice.c` |

Era `unknown_00054590`. Con ella `game_voice.c` tiene sus 71 funciones, de
`0x00053290` a `0x00055060`: todo lo que el juego hace con la voz de los
jugadores, encima de `voice.c` (22 funciones). Son los dos primeros
archivos de red del proyecto que quedan completos.

### Cambiado

- Lo que el hito 274 llamó «lo de los jugadores que hablan» (`0x004c9898` y
  los 192 bytes de `0x004c989c`) son los jugadores que la sesión tenía la
  última vez: un bit por cada uno y los doce bytes por los que se conoce a
  cada uno. Pasan a ser `game_voice_session_players` y
  `game_voice_session_player_identifiers`.

### Lo que se sabe

- Cada fotograma el juego mira si los jugadores de la sesión son los de la
  última vez: los mismos bits y cada uno con los mismos doce bytes. Si algo
  cambió, guarda los nuevos y se lo dice a lo que vive en `0x00525a00`
  (`0x000560e0`, sin descompilar).
- También arma un texto que dice cuál de las dos cosas cambió («session
  player mask change» o «player identifier change») y no hace nada con él:
  otro resto de un aviso de depuración que esta versión ya no tiene, como
  el del hito 268.

### Corregido

- `ESTADO.md` llevaba desde el hito 275 la cuenta de funciones del archivo
  sin actualizar (63). Son 71.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- La batería completa y las mutaciones de los hitos 266 a 278 corren
  aparte; su resultado irá en una entrada posterior.

## 2026-10-08 — Cada jugador, cada fotograma: quién tiene voz y a quién no quiere oír

Dos funciones en `networking/game_voice.c`, de 983 bytes entre las dos. De 1020 funciones se pasa a
1022, el 9,07 % de las del juego (el 5,836 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_update_players` | `0x00054030` | `networking/game_voice.c` |
| `game_voice_update_local_player` | `0x000541a0` | `networking/game_voice.c` |

Eran `unknown_00054030` y `unknown_000541a0`. Con ellas se entienden los
flags y los números de 16 bits que los hitos 270 a 274 dejaron sin nombre.

### Cambiado

- Los tres flags de la voz de un jugador (`0x004c9988`) son: **puede
  hablar**, **no quiere voz** (el ajuste 3 de su mando) y **no se le
  permite** (lo que dice `0x001906da` de su cuenta). Pasan a llamarse así
  ellos y las funciones que preguntan por ellos:
  `game_voice_player_voice_off`, `game_voice_player_not_allowed` y
  `game_voice_player_can_be_heard` (era `game_voice_unknown_00053b40`).
- El número de 16 bits de cada jugador en `0x004c9968` es **a quién no
  quiere oír**: `game_voice_player_mutes`, y
  `game_voice_player_has_muted` la función que lo pregunta.
- `0x004c9966` no es solo de los jugadores de otras consolas: es un bit por
  cada jugador con voz (`game_voice_talkers`). Uno de esta consola lo tiene
  si puede hablar.

### Lo que se sabe

- Cada fotograma se recorren los 16 jugadores. Al que no está en la sesión
  se le quita la voz en la biblioteca y su bit. Al de otra consola se le da
  o se le quita la voz (hito 274). Al de esta consola se le pone el modo de
  su mando (el 2 si quiere y puede tener voz, el 0 si no) y sus tres flags.
- Cada mando tiene una lista de hasta 250 jugadores a los que su jugador
  no quiere oír (`0x004c99d4`, 16 bytes cada uno), y cuántos hay
  (`0x004cd854`). A un jugador se le reconoce por sus doce primeros bytes,
  que son los mismos con los que empieza cada jugador de la sesión. De ahí
  sale el número de «a quién no quiere oír».

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- `game_voice_update_local_player` sin voz y con lista: el juego compara
  con los jugadores de una sesión que no hay y se para.
- Las mutaciones de los hitos 266 a 277 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — La consola elegida para la voz de la partida

Dos funciones en `networking/game_voice.c`, de 536 bytes entre las dos. De 1018 funciones se pasa a
1020, el 9,05 % de las del juego (el 5,803 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_update_chosen_peer` | `0x00053e30` | `networking/game_voice.c` |
| `game_voice_unknown_00054890` | `0x00054890` | `networking/game_voice.c` |

`game_voice_update_chosen_peer` era `unknown_00053e30`, una de las tres que
`game_voice_update` (hito 275) dejó apuntadas.

### Lo que se sabe

- La consola de `0x004c9878` es **la elegida para la voz**: por lo que se
  ve, la que reparte la voz entre las demás. La consola que lleva la sesión
  la elige cada fotograma: de las otras, la de mayor «segundo número» entre
  las que se pueden elegir, llegan a dos mínimos (`0x004ce120` y
  `0x004ce124`) y están unidas a todas las consolas de la partida. Si no
  hay ninguna, se queda ella misma, salvo en un caso. Luego se lo dice a la
  sesión (`0x0005c6d0`) si esta tenía otra.
- Las demás consolas no eligen: toman la que dice la sesión (`0x0005b0b0`).
- El número de `0xa8` de cada consola (hito 272) es un bit por cada consola
  a la que está unida.
- `game_voice_unknown_00054890` dice si a una consola le llega la voz de
  esta: siempre si esta es la elegida, y si no, cuando lo que dice algún
  jugador suyo va dirigido a ella.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 266 a 276 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — La voz de cada fotograma, y lo que dice un jugador de esta consola

Tres funciones en `networking/game_voice.c`, de 573 bytes entre las tres. De 1015 funciones se pasa a
1018, el 9,04 % de las del juego (el 5,785 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_update` | `0x00053610` | `networking/game_voice.c` |
| `game_voice_local_data_ready` | `0x00053a20` | `networking/game_voice.c` |
| `game_voice_unknown_00053c70` | `0x00053c70` | `networking/game_voice.c` |

`game_voice_local_data_ready` era `unknown_00053a20`, la que `voice.c`
llama cuando la biblioteca da lo que dijo un jugador (hito 267).

### Lo que se sabe

- `game_voice_update` es la voz de cada fotograma. El motor hace su
  trabajo; con la voz completa se mira de dónde sale la sesión y el estado
  de la red; sin red, los jugadores de otras consolas pierden la voz y se
  olvida todo lo de los jugadores; con red, se hace lo demás (tres
  funciones del archivo aún sin descompilar y dos de lo que viene después
  de `voice.c`).
- Lo que dice un jugador de esta consola sigue su camino solo si su mando
  tiene jugador en la sesión, el jugador no tiene ni el segundo flag ni el
  tercero (lo que apunta a que son «silenciado» de dos maneras), y dos
  cosas que aún no se conocen (`0x005259b8` y `0x00527104`) dan algo para
  él. Entonces se entrega a `0x00056130`, que por lo que recibe lo manda a
  las otras consolas.
- `game_voice_unknown_00053c70` da, con la sesión del segundo sitio, un bit
  por cada consola suya que también está en la del primero.

### Batería completa

La del hito 265 terminó: 940 funciones, las 940 bien.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `voice`.
- En `game_voice_unknown_00053c70`, una sesión que no dice quién está en
  ella: el juego lee de la dirección 8 y se para.
- Los tamaños de `0x005259b8`, `0x00527104` y `0x00527108` son
  suposiciones.
- Las mutaciones de los hitos 266 a 275 corren aparte (las de 266 a 268 se
  lanzaron y no llegaron a correr: la copia de comprobación no cambió de
  commit; están relanzadas).

## 2026-10-08 — La voz, cada fotograma: el motor, el estado de la red y cada jugador

Ocho funciones en `networking/game_voice.c`, de 890 bytes entre las ocho. De 1007 funciones se pasa a
1015, el 9,01 % de las del juego (el 5,766 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_library_update` | `0x00053580` | `networking/game_voice.c` |
| `game_voice_unknown_00053d40` | `0x00053d40` | `networking/game_voice.c` |
| `game_voice_reset_players` | `0x00053fb0` | `networking/game_voice.c` |
| `game_voice_update_player` | `0x00054410` | `networking/game_voice.c` |
| `game_voice_update_network_state` | `0x000544e0` | `networking/game_voice.c` |
| `game_voice_unknown_00054df0` | `0x00054df0` | `networking/game_voice.c` |
| `game_voice_get_player_datum` | `0x00054e60` | `networking/game_voice.c` |
| `game_voice_update_profile` | `0x00054fe0` | `networking/game_voice.c` |

### Cambiado

- El número de 16 bits de `0x004c9966` es un bit por cada jugador de otra
  consola al que la biblioteca le ha dado voz: `unknown_004c9966` pasa a ser
  `game_voice_remote_talkers`, y `game_voice_unknown_00054d00`,
  `game_voice_get_remote_talkers`.

### Lo que se sabe

- Cada fotograma el motor de la voz hace su trabajo
  (`game_voice_library_update`). Antes se olvida qué mandos han hablado: el
  motor lo vuelve a decir mientras trabaja, y de ahí sale el «está
  hablando» del hito 273.
- `game_voice_update_player` copia de la sesión los dos números de 16 bits
  de un jugador y hace que la biblioteca le dé voz o se la quite, según
  diga `game_voice_unknown_00053b40` (tiene el primer bit y no el segundo).
  Si la biblioteca no puede dársela, el jugador queda sin su bit.
- `0x004c9888` es lo que la voz saca del estado de la partida en red
  (`0x00527334`): 0 si no hay red, 1 en los estados 3 y 8 con una partida
  en marcha, 3 en el 7 (y en el 8 sin partida) si lo dice un flag de la
  red, y 2 en lo demás.
- `game_voice_update_profile` apunta, en lo que el juego guarda del perfil
  del jugador de un mando, el mando y los dos números de la voz de su
  jugador.
- El juego tiene 192 bytes más sobre los jugadores que hablan
  (`0x004c989c`), que por ahora solo se vacían.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- `game_voice_update_player` sin voz o sin sesión: el juego lee los
  jugadores de una sesión que no hay y se para.
- Los tamaños de dos tablas son suposiciones (`0x004cd904`, de 4, y
  `0x0055e4d0`, de 16).
- Las mutaciones de los hitos 266 a 274 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — Quién puede hablar y quién está hablando: más de mil funciones

Diez funciones en `networking/game_voice.c`, de 620 bytes entre las diez. De 997 funciones se pasa a
1007, el 8,94 % de las del juego (el 5,733 % de sus bytes). **El proyecto pasa de las mil funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_controller_can_talk` | `0x000536b0` | `networking/game_voice.c` |
| `game_voice_player_is_talking` | `0x00053750` | `networking/game_voice.c` |
| `game_voice_get_local_players` | `0x00054d20` | `networking/game_voice.c` |
| `game_voice_unknown_00054ec0` | `0x00054ec0` | `networking/game_voice.c` |
| `game_voice_unknown_00054ef0` | `0x00054ef0` | `networking/game_voice.c` |
| `game_voice_get_session_source` | `0x00054f20` | `networking/game_voice.c` |
| `game_voice_controller_player_words` | `0x00054f40` | `networking/game_voice.c` |
| `game_voice_get_controller_setting` | `0x00054f90` | `networking/game_voice.c` |
| `game_voice_set_controller_setting` | `0x00054fc0` | `networking/game_voice.c` |
| `game_voice_text_printf` | `0x00055060` | `networking/game_voice.c` |

`game_voice_controller_can_talk` y `game_voice_get_local_players` eran
`unknown_000536b0` y `unknown_00054d20`.

### Cambiado

- Con `game_voice_player_is_talking` se aclaran tres nombres de los hitos
  266 y 272. El «número por el que la voz conoce a un jugador» es, en los
  jugadores de esta consola, **su mando**: `game_voice_player_identifier`
  pasa a ser `game_voice_player_controller`. Y lo que la biblioteca dice de
  un jugador de otra consola en `0x000551c0` es **si está hablando**:
  `voice_remote_talker_unknown_000551c0` pasa a ser
  `voice_remote_talker_is_talking`. A los jugadores de otras consolas la
  voz los conoce por su número de jugador en la partida.

### Lo que se sabe

- Un jugador de esta consola «está hablando» si la biblioteca ha dado algo
  que dijo desde la última vez; uno de otra consola, si la biblioteca lo
  dice.
- El jugador de un mando «puede hablar» si el mando tiene auriculares, su
  ajuste de voz no es el 3 y, cuando lo que el juego sabe del jugador tiene
  cierto bit, si además lo permite `0x001906da` (por lo que mira, la cuenta
  en línea del jugador).
- Los jugadores de esta consola son los de la sesión cuya consola es esta:
  `game_voice_get_local_players` da un bit por cada uno.
- `game_voice_text_printf` escribe un texto de 128 caracteres; lo usa la
  función que avisa de un cambio de jugador (`0x00054590`).

### Batería completa

Se lanzó en el hito 265 y sigue corriendo aparte; su resultado irá en una
entrada posterior, con el de las mutaciones de los hitos 266 a 273.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `voice`.

## 2026-10-08 — La voz: las consolas y los jugadores de la sesión

Quince funciones en `networking/game_voice.c`, de 880 bytes entre las quince. De 982 funciones se pasa a
997, el 8,85 % de las del juego (el 5,711 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_update_session_source` | `0x000547e0` | `networking/game_voice.c` |
| `game_voice_unknown_00054810` | `0x00054810` | `networking/game_voice.c` |
| `game_voice_unknown_00054850` | `0x00054850` | `networking/game_voice.c` |
| `game_voice_unknown_00054990` | `0x00054990` | `networking/game_voice.c` |
| `game_voice_unknown_000549d0` | `0x000549d0` | `networking/game_voice.c` |
| `game_voice_get_local_peer` | `0x00054a20` | `networking/game_voice.c` |
| `game_voice_session_unknown40` | `0x00054a70` | `networking/game_voice.c` |
| `game_voice_local_peer_unknownA8` | `0x00054ac0` | `networking/game_voice.c` |
| `game_voice_get_players` | `0x00054b10` | `networking/game_voice.c` |
| `game_voice_get_player_array` | `0x00054b40` | `networking/game_voice.c` |
| `game_voice_find_local_player` | `0x00054b70` | `networking/game_voice.c` |
| `game_voice_player_identifier` | `0x00054c20` | `networking/game_voice.c` |
| `game_voice_unknown_00054c70` | `0x00054c70` | `networking/game_voice.c` |
| `game_voice_unknown_00054cc0` | `0x00054cc0` | `networking/game_voice.c` |
| `game_voice_unknown_00054d00` | `0x00054d00` | `networking/game_voice.c` |

### Lo que se sabe

- La sesión guarda qué consola es esta (`0x72d8`) y, en lo que dice de quién
  está, una lista de hasta 16 jugadores desde `0x10d4`, de `0x13c` bytes
  cada uno: de cada jugador, de qué consola es (`0xc`) y el número por el
  que lo conoce la voz (`0x14`), que es el que `voice.c` convierte en el
  jugador de la biblioteca.
- `game_voice_find_local_player` busca, entre los jugadores de esta
  consola, el que tiene un número; `game_voice_player_identifier` hace el
  camino contrario.
- De dónde sale la sesión (`0x004c988c`) lo decide
  `game_voice_update_session_source` con lo que le da `0x000596d0`: 1 o 2,
  y 0 si no hay nada.
- Hay una consola apuntada en `0x004c9878` (NONE si ninguna); dos funciones
  preguntan si es esta y si es otra. Con lo que se sabe, podría ser la que
  manda en la voz de la partida.
- Tres funciones pasan el encargo a algo que vive en `0x00525a00`
  (`0x000564c0`, `0x00056160` y `0x00056220`, sin descompilar).
- Varias llevan su dirección en el nombre porque aún no se sabe qué
  preguntan.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 266 a 272 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — La voz y la sesión de red: quién está en la partida y qué se sabe de su voz

Once funciones en `networking/game_voice.c`, de 550 bytes entre las once. De 971 funciones se pasa a
982, el 8,72 % de las del juego (el 5,678 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_unknown_00053b00` | `0x00053b00` | `networking/game_voice.c` |
| `game_voice_unknown_00053b40` | `0x00053b40` | `networking/game_voice.c` |
| `game_voice_player_unknown1` | `0x00053b90` | `networking/game_voice.c` |
| `game_voice_player_unknown2` | `0x00053bb0` | `networking/game_voice.c` |
| `game_voice_player_flags` | `0x00053be0` | `networking/game_voice.c` |
| `game_voice_get_session` | `0x00053c30` | `networking/game_voice.c` |
| `game_voice_unknown_00053d90` | `0x00053d90` | `networking/game_voice.c` |
| `game_voice_message_is_sound_file` | `0x00053db0` | `networking/game_voice.c` |
| `game_voice_get_membership` | `0x00053de0` | `networking/game_voice.c` |
| `game_voice_library_begin` | `0x00053f50` | `networking/game_voice.c` |
| `game_voice_library_end` | `0x00053f80` | `networking/game_voice.c` |

Las dos últimas eran `unknown_00053f50` y `unknown_00053f80`, las que el
hito 270 dejó apuntadas.

### Lo que se sabe

- La voz va con una sesión de la partida en red, que viene de uno de dos
  sitios según `0x004c988c` (`0x00059670` o `0x000596a0`, sin descompilar).
  De la sesión se mira quién está en ella: un bit por jugador en `0x10d0`
  de lo que hay en su `0x4c`.
- De cada jugador el juego guarda 16 bits sobre su voz (`0x004c9988`), que
  solo valen si el jugador está en la sesión. Tres funciones preguntan por
  sus tres primeros bits; qué significa cada uno aún no se sabe.
- Antes de que trabaje la biblioteca de la voz el juego le pone los reales
  como ella los quiere y hace que ningún error en uno pare el programa; al
  acabar olvida los errores que quedaron apuntados y los deja como los
  tiene el juego.
- `game_voice_message_is_sound_file` es la comprobación de `RIFF`, `WAVE`
  y `fmt ` que `voice_play_message` (hito 267) tiene metida dentro.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- En `game_voice_library_begin` y `game_voice_library_end`, cómo queda el
  registro del procesador que cambian (el de los reales de SSE): las
  pruebas solo comparan lo que se le pide a la biblioteca de C.
- Las mutaciones de los hitos 266 a 271 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — Once funciones pequeñas de la voz: modos, máscaras y mensajes

Once funciones en `networking/game_voice.c`, de 475 bytes entre las once. De 960 funciones se pasa a
971, el 8,62 % de las del juego (el 5,660 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_has_remote_talker` | `0x00053720` | `networking/game_voice.c` |
| `game_voice_set_controller_mode` | `0x000537b0` | `networking/game_voice.c` |
| `game_voice_get_controller_mode` | `0x000537f0` | `networking/game_voice.c` |
| `game_voice_set_controller_mask` | `0x00053810` | `networking/game_voice.c` |
| `game_voice_unknown_00053850` | `0x00053850` | `networking/game_voice.c` |
| `game_voice_remote_talker_data` | `0x00053890` | `networking/game_voice.c` |
| `game_voice_play_message` | `0x000538e0` | `networking/game_voice.c` |
| `game_voice_stop_message` | `0x00053940` | `networking/game_voice.c` |
| `game_voice_message_busy` | `0x00053970` | `networking/game_voice.c` |
| `game_voice_record_message` | `0x000539a0` | `networking/game_voice.c` |
| `game_voice_stop_message_with_headset` | `0x000539e0` | `networking/game_voice.c` |

Casi todas son la misma idea: si el juego tiene que ver con la voz y la voz
está arrancada, pasan el encargo a `voice.c`; si no, no hacen nada.

### Cambiado

- Los cuatro reales por mando de `voice_globals` no eran «niveles»: son lo
  que la biblioteca usa para **cambiar la voz** del jugador (una máscara).
  `voice_update_controller_levels` pasa a ser `voice_set_controller_mask`,
  `voice_reset_controller_levels` a `voice_reset_controller_masks`, y el
  campo a `controller_masks`. La función no pide nada a la biblioteca: le
  dice la máscara y, si la acepta, la guarda.

### Lo que se sabe

- El juego tiene dos máscaras (`game_voice_masks`, `0x0043fe48`): la que no
  cambia nada (cuatro -1) y una que deforma la voz (0,95, 0,5 y 0,35), para
  que no se reconozca al que habla.
- Cada mando tiene un ajuste de voz del jugador (`0x004c99a8`): con el 3 no
  se le reproducen mensajes, y con el 1 se reproducen sin auriculares.
- Cada jugador de la partida (hasta 16) tiene un número de 16 bits con un
  bit por cada uno de los demás (`0x004c9968`); aún no se sabe si es a
  quién oye o a quién ha silenciado.
- Antes y después de dar a la biblioteca lo que dijo un jugador de otra
  consola, el juego cambia cómo redondea el procesador (`0x00053f50` y
  `0x00053f80`, sin descompilar): la biblioteca lo quiere de otra manera.
- En tres de ellas el juego tiene dentro una función de `voice.c` de la que
  no guarda copia aparte (cambiar el modo de un mando, leerlo y parar un
  mensaje).

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `voice`.
- Las mutaciones de los hitos 266 a 270 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — `sound_memory.c` pasa a ser `game_voice.c`, con el fondo de la voz y del menú

Cuatro funciones en `networking/game_voice.c`, de 302 bytes entre las cuatro. De 956 funciones se pasa a
960, el 8,52 % de las del juego (el 5,641 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_voice_menu_pool_new` | `0x00053420` | `networking/game_voice.c` |
| `game_voice_menu_pool_delete` | `0x000534a0` | `networking/game_voice.c` |
| `game_voice_start` | `0x00053510` | `networking/game_voice.c` |
| `game_voice_stop` | `0x00053550` | `networking/game_voice.c` |

### Cambiado

- `sound/sound_memory.c` se llama ahora `networking/game_voice.c`, y sus
  funciones y variables `game_voice_...` en vez de `sound_memory_...`. En el
  hito 263 se dio el archivo por completo, pero sigue: lo que viene después
  de sus cuatro funciones es lo que el juego hace con la voz de los
  jugadores, hasta `0x00055060` (unas 70 funciones), y el fondo de memoria
  era solo el principio. Las pruebas son `tests\test_game_voice.py`.
- El «otro fondo» (`0x004c9960`) es `game_voice_menu_pool`: el juego lo
  llama «voice y menu pool».

### Lo que se sabe

- Hay dos fondos y nunca los dos a la vez. En una partida que no es
  campaña, el de la voz (`0x78000` bytes, hito 263). En una campaña dentro
  de una sesión de red, el de la voz y del menú (`0x61800` bytes), que se
  pide a la reserva cuya memoria presta la caché de texturas
  (`0x0046dd54`) y, si no la hay, a la general.
- `0x004c99b8` dice si el juego tiene que ver con la voz
  (`game_voice_enabled`): casi todas las funciones del módulo lo miran
  primero, junto con si la voz está arrancada.
- La voz se arranca si es completa, o si es la de una sesión y la partida
  es una campaña.
- Al parar la voz se pone a 0 un número de 16 bits (`0x004c9966`) del que
  aún no se sabe más.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo, las de `shell` y las de
  `main_game`.
- Las mutaciones de los hitos 266 a 269 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — Lo que dijo un jugador de otra consola: `voice.c`, completo

Dos funciones en `networking/voice.c`, de 307 bytes entre las dos: las últimas del archivo. De 954 funciones se pasa a
956, el 8,49 % de las del juego (el 5,630 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_remote_talker_data` | `0x000553b0` | `networking/voice.c` |
| `voice_text_append` | `0x00055920` | `networking/voice.c` |

Con ellas `voice.c` tiene sus 22 funciones, de `0x00055080` a `0x00055920`.

### Lo que se sabe

- `voice_remote_talker_data` le da a la biblioteca lo que dijo un jugador
  de otra consola, tal como llegó por la red (`0x00332df4`).
- Si la biblioteca no lo acepta, el juego calcula la suma de comprobación
  de lo que llegó y arma un texto, `voice_modes:(c#0 2)(c#1 2)...`, con el
  modo de cada mando... y no hace nada con ninguno de los dos. Es lo que
  queda de un aviso de depuración que esta versión del juego ya no tiene.
  Está escrito igual, con sus llamadas.
- `voice_text_append` añade texto con formato a un texto de 128
  caracteres. Es `csnzappendf` de `cseries.c` con el tamaño fijo.
- `0x00055960` y `0x00055990`, que vienen después, no son de este archivo:
  trabajan con otra estructura y con las variables del módulo de encima.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Las mutaciones de los hitos 266, 267 y 268 corren aparte; su resultado
  irá en una entrada posterior.

## 2026-10-08 — Los mensajes de voz y lo que la biblioteca le avisa al juego

Ocho funciones en `networking/voice.c`. De 946 funciones se pasa a
954, el 8,47 % de las del juego (el 5,620 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_play_message` | `0x000554c0` | `networking/voice.c` |
| `voice_record_message` | `0x00055520` | `networking/voice.c` |
| `voice_callback_headset_changed` | `0x000555b0` | `networking/voice.c` |
| `voice_callback_data_ready` | `0x000555f0` | `networking/voice.c` |
| `voice_callback_unknownC` | `0x00055620` | `networking/voice.c` |
| `voice_callback_unknown8` | `0x00055630` | `networking/voice.c` |
| `voice_callback_message_recorded` | `0x00055640` | `networking/voice.c` |
| `voice_callback_message_stopped` | `0x00055680` | `networking/voice.c` |

Las ocho son nuevas en la lista. Seis de ellas no las llama el juego: las
llama la biblioteca de la consola, por una tabla (`voice_callbacks`,
`0x0045095c`) a la que apunta lo primero de `voice_globals`. Ghidra no las
tiene en su lista, así que no suman bytes.

### Lo que se sabe

- Con la tabla se entiende el resto de `voice_globals`, que queda sin
  huecos: si cada mando tiene auriculares (`0x64`; la biblioteca avisa
  cuando se enchufan y cuando se quitan), dónde dejar el tamaño y la
  duración del mensaje que graba (`0x68` y `0x78`), si está grabando o
  reproduciendo un mensaje (`0x88`) y si su jugador ha hablado (`0x8C`).
- Un mando graba y reproduce mensajes solo en el modo 3, el de la campaña
  en sesión. Un mensaje que se reproduce es un archivo de sonido: el juego
  mira que empiece por `RIFF`, `WAVE` y `fmt ` y que mida más de 16 bytes.
- Al grabar, el juego vacía el búfer y pone a 0 el tamaño y la duración; la
  biblioteca avisa cuando el mensaje está hecho y entonces se rellenan.
- Cuando un jugador habla, la biblioteca da lo que dijo y el juego se lo
  pasa a `0x00053a20`, del módulo de encima, aún sin descompilar.
- Dos de los avisos no hacen nada: devuelven «no implementado».
- Los nombres de los campos y de los avisos son suposiciones, por cómo se
  usan.

### Mutaciones

Las del hito 265 terminaron: treinta y un cambios a mano, y las pruebas
notan los treinta y uno.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `sound_memory`.
- Las mutaciones de los hitos 266 y 267 corren aparte; su resultado irá en
  una entrada posterior.

## 2026-10-08 — Las voces de los jugadores de otras consolas

Seis funciones en `networking/voice.c`, de 580 bytes entre las seis. De 940 funciones se pasa a
946, el 8,40 % de las del juego (el 5,612 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_has_remote_talker` | `0x00055130` | `networking/voice.c` |
| `voice_remote_talker_unknown_000551c0` | `0x000551c0` | `networking/voice.c` |
| `voice_update_controller_levels` | `0x00055210` | `networking/voice.c` |
| `voice_remote_talker_unknown_00055250` | `0x00055250` | `networking/voice.c` |
| `voice_remove_remote_talkers` | `0x000552a0` | `networking/voice.c` |
| `voice_remote_talker_set` | `0x00055330` | `networking/voice.c` |

`voice_remove_remote_talkers` era `unknown_000552a0`, la que `voice_dispose`
(hito 265) llama al parar la voz. Las otras cinco son nuevas en la lista.

### Lo que se sabe

- La biblioteca conoce a un jugador de otra consola por doce bytes: un
  número de 64 bits y unos flags (`voice_talker`). El juego lo hace siempre
  a partir de un número de 32 bits, con su signo, y sin flags.
- El motor guarda hasta 30 jugadores de otras consolas con voz, y da la
  lista entera cuando se le pide (`0x00332def`). Para saber si uno tiene voz
  el juego pide la lista y la recorre: dos jugadores son el mismo si tienen
  el mismo número y los dos bits bajos de los flags iguales.
- Solo una voz que usa el modo 2 tiene jugadores de otras consolas: es la
  de las partidas que no son campaña. Casi todas las funciones miran eso
  antes de nada.
- Dar y quitar la voz a un jugador es una sola función con un sí o un no
  (`voice_remote_talker_set`). Al parar la voz se le quita a todos, y ahí el
  juego usa solo la mitad baja del número de cada uno.
- `voice_update_controller_levels` pide a la biblioteca los cuatro reales de
  un mando y, si se los da, los guarda en `voice_globals` (los que el hito
  264 pone a -1).
- De dos funciones no se sabe qué preguntan o dicen de un jugador
  (`0x000551c0` y `0x00055250`): llevan su dirección en el nombre.
- Seis funciones más de la biblioteca quedan apuntadas.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo.
- Una lista de más de 30 jugadores: el juego no tiene sitio para ella.
- Las mutaciones de este hito corren aparte; su resultado irá en una
  entrada posterior.

## 2026-10-08 — Arrancar y parar la voz de los jugadores

Tres funciones en `networking/voice.c`, de 428 bytes entre las tres. De 937 funciones se pasa a
940, el 8,34 % de las del juego (el 5,592 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_start` | `0x00055810` | `networking/voice.c` |
| `voice_initialize` | `0x00055080` | `networking/voice.c` |
| `voice_dispose` | `0x000550b0` | `networking/voice.c` |

`voice_start` y `voice_dispose` eran `unknown_00055810` y
`unknown_000550b0`, las dos que `sound_memory.c` dejó pendientes en el hito
263. `voice_initialize` es nueva en la lista: el juego la tiene también
metida dentro de `sound_memory_pool_new`, que ahora la llama.

### Lo que se sabe

- La voz va sobre una biblioteca de la consola: nueve funciones suyas
  quedan apuntadas (`0x00333055`, que crea el motor, y ocho más, de
  `0x00332d8f` a `0x003330f8`). Qué hace cada una se deduce de cómo las
  llama el juego; sus nombres de verdad no se saben.
- Hay cuatro «modos» de voz (`voice_modes`, `0x0052731c`: lo que la
  biblioteca entiende por cada uno). El juego usa los cuatro, salvo en una
  campaña dentro de una sesión, donde no usa el 2. Cada mando empieza en el
  modo 2, o en el 3 en esa campaña (hito 264): el campo de `0xC` de
  `voice_globals` es qué modos se usan, un bit por modo.
- Al arrancar, cada mando recibe una voz y su modo, y se apunta si lo
  primero que la biblioteca dice de él es 0 (`0x64`); dos bytes más por
  mando empiezan en falso (`0x88` y `0x8C`).
- `voice_start` solo devuelve falso si no había con qué pedir el motor
  (`voice_engine_parameters_new`). Si la biblioteca dice que algo no se
  pudo, el juego deja lo demás sin hacer y aun así da la voz por
  arrancada. Está escrito igual.
- Al parar: se quita la voz de cada mando, se llama a `0x000552a0` si se
  usaba el modo 2 y se suelta el motor.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `sound_memory`.
- Las mutaciones de este hito corren aparte; su resultado irá en una
  entrada posterior.

## 2026-10-08 — La voz de los jugadores: lo que se prepara antes de arrancarla

Tres funciones en un archivo nuevo, `networking/voice.c`, de 299 bytes entre las tres. De 934 funciones se pasa a
937, el 8,32 % de las del juego (el 5,578 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `voice_reset_controller_levels` | `0x000556a0` | `networking/voice.c` |
| `voice_reset_controllers` | `0x00055720` | `networking/voice.c` |
| `voice_engine_parameters_new` | `0x00055780` | `networking/voice.c` |

Las dos primeras eran `unknown_000556a0` y `unknown_00055720`, las que
`sound_memory_pool_new` (hito 263) llama antes de arrancar la voz. La
tercera la llama quien la arranca, `0x00055810`, que es lo siguiente.

### Lo que se sabe

- `voice_globals` (`0x00476fc8`) pasa a `networking/voice.h` y gana dos
  campos: dieciséis reales desde `0x14` (cuatro por mando, que empiezan en
  -1) y un número por mando desde `0x54` (3 en una campaña dentro de una
  sesión, 2 en lo demás).
- A la biblioteca de la consola se le piden nueve números para arrancar la
  voz (`voice_engine_parameters`). Con la voz completa los de `0` y `8` son
  15 y 5, y si no 0: casan con quince jugadores remotos, que son los de una
  partida de dieciséis. El de `4` es siempre 4, como los mandos.
- El de `0x10` es lo primero de `unknown_00510c90`, que ya estaba en
  `sound_manager.c`: lo que el juego sabe de los efectos del procesador de
  sonido de la consola. Sin eso la voz no arranca.
- Los bucles sobre los mandos no cuentan de 0 a 3: el juego pregunta cada
  vez cuál es el mando siguiente con una tabla de saltos (0, 1, 2, 3 y
  fin). Está escrito igual, en `voice_next_controller`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `sound_memory`.

### Mutaciones

Veinte cambios a mano: las pruebas notan los veinte.

## 2026-10-08 — El fondo de memoria de la voz, al cargar un mapa

Dos funciones en `sound/sound_memory.c`, que queda completo. De 932 funciones se pasa a
934, el 8,29 % de las del juego (el 5,568 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_memory_pool_new` | `0x00053310` | `sound/sound_memory.c` |
| `sound_memory_pool_delete` | `0x000533e0` | `sound/sound_memory.c` |

Eran `unknown_00053310` y `unknown_000533e0`: el cuarto par de la tabla de
los que toman memoria al cargar un mapa (`main_game_memory_users`,
`0x00453c00`). Con ellas la tabla tiene cinco pares de ocho.

### Lo que se sabe

- El fondo de `sound_memory.c` se llama en el juego «voice pool»: es la
  memoria de la voz de los jugadores en las partidas en red, no la del
  sonido en general. Mide `0x78000` bytes y se pide a la misma reserva que
  da su memoria a los archivos de los jugadores (`0x004508fc`).
- Según cómo se reparte la memoria de la partida, el juego tiene toda la
  voz (cualquier partida que no sea campaña), una parte (campaña dentro de
  una sesión de red) o ninguna (campaña sin sesión). Lo apunta en
  `0x004c9890` y `0x004c9894`. Solo en el primer caso hay fondo.
- Al crear el fondo se arranca la voz, si no lo estaba: `0x00476fc8` es su
  estructura (`voice_globals`, `0x90` bytes), de la que aquí se conocen tres
  campos. Lo hacen tres funciones aún sin descompilar (`0x000556a0`,
  `0x00055720` y `0x00055810`); `0x000550b0` la para antes de devolver el
  fondo.
- El juego no mira si hubo memoria para el fondo antes de escribir en él.
- Ghidra no tiene estas dos funciones en su lista (solo se llega a ellas por
  la tabla), así que no suman bytes al porcentaje de código.

### Entorno

- En una terminal nueva el `clang` que sale primero es el de llvm-mingw
  (22), que ya no compila `game.c`: da por error lo que el de Visual Studio
  (19) da por aviso. Hay que poner delante en el `PATH` el de
  `Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin`. Apuntado en
  `ESTADO.md`.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `main_game`.
- El caso de que no haya memoria para el fondo: el juego escribe en la
  dirección `0x3c` y se para.
- En la prueba, las funciones de la reserva son de la prueba: el arnés no
  sabe simular una función no descompilada a la que `source/` llama con su
  primer argumento en `ecx`.

### Mutaciones

Veinte cambios a mano: las pruebas notan los veinte.

## 2026-10-05 — Lo que dice cada mando, cada fotograma

Una función en `input/input_abstraction.c`, de 1.993 bytes. De 931 funciones se pasa a
932, el 8,27 % de las del juego (el 5,568 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `input_abstraction_update` | `0x00217c40` | `input/input_abstraction.c` |

Era `unknown_00217c40`. La llama `main_loop_body` justo después de
`input_update`. No recibe ni devuelve nada. Con ella, las seis funciones
del módulo `input_abstraction` están descompiladas.

### Lo que se sabe

- Por cada mando enchufado rellena `0x50` bytes de
  `input_abstraction_globals` (hasta ahora un hueco sin nombre): los
  dieciséis botones en el orden que digan las preferencias del mando
  (cuántos fotogramas y cuántos milisegundos lleva pulsado cada uno), los
  dos gatillos de 0 a 1, cinco reales de las dos palancas y tres bytes.
- Las palancas se «estiran»: el juego divide por el mayor de |seno| y
  |coseno| de su ángulo, para que lleguen a 1 en todas las direcciones y no
  solo a lo largo de los ejes.
- El número de `0x18` de las preferencias dice qué palanca mueve y cuál
  mira (cuatro combinaciones). En las combinaciones 2 y 3 las palancas solo
  van en cuatro direcciones: cerca de una diagonal (a menos de 35 grados la
  izquierda y de 10 la derecha) el eje menor se va encogiendo hasta la
  diagonal; fuera, solo queda el eje mayor.
- Mirar arriba y abajo va al revés con el byte de `0x1A`, o con el de
  `0x1B` cuando `input_abstraction_controller_unknown_00218510` (hito 258)
  dice que sí: será la opción de invertir solo al volar.
- Los ángulos se calculan con las instrucciones del procesador (arco
  tangente, seno y coseno), y las pruebas comparan los reales bit a bit. A
  la biblioteca de las pruebas se le ha añadido `atan2l`.

### Batería completa

La del hito 261 terminó: 931 funciones, las 931 bien. Es la primera con
todo el sonido descompilado.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo y las de `main`.

### Mutaciones

Cuarenta y tres cambios a mano: las pruebas notan cuarenta y uno. Los dos
que no notan son de casos justos: una palanca a exactamente el ángulo
límite de la diagonal, y una con los dos ejes iguales fuera de ese ángulo,
que solo pasa con la palanca en el centro (y entonces da lo mismo).

## 2026-10-05 — Un sonido en bucle empieza, sigue o acaba: el sonido, completo

Una función en `sound/sound_manager.c`, de 2.497 bytes: la última del sonido. De 930 funciones se pasa a
931, el 8,26 % de las del juego (el 5,501 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_start` | `0x0021a250` | `sound/sound_manager.c` |

Era `unknown_0021a250`, la más grande de las que quedaban. La llaman
`sound_effect_start_sounds` y una función de fuera del sonido
(`0x0018a7b0`). Recibe en la pila el tag, un número, el dueño, el dueño del
mando de sonidos, lo que el juego dice del sonido, qué se pide (0 empezar,
1 seguir, 2 acabar), unas marcas y unos segundos; devuelve un byte. Nuevas
sin descompilar: `0x00192d70` y `0x00192ce0`.

**Con esta función, todo el sonido del juego que estaba en la lista está
descompilado**: `sound/sound_manager.c` reúne las funciones de los rangos
`0x00124fc0`–`0x0012b0d0` y `0x00218510`–`0x00222a00`.

### Lo que se sabe

- Busca el sonido en bucle del tag y del dueño (`looping_sound_find`) y lo
  crea si hace falta para empezar o seguir. Uno de un sonido con sitio cuyo
  tag tiene la marca 7 es «propio»: su dueño es su tag con las marcas
  `0x6000`, y cada vez que se pide se suma a sus cuatro ganancias cómo lo
  oye su oyente (`0x00192d70` y `0x00192ce0` dicen cuánto, por lo lejos que
  está y por dónde).
- Si se pide acabar (o tiene la marca 3) y no tiene sonidos sonando, se
  suelta.
- Cada pista tiene uno de siete estados: entrada, bucle y salida; sus tres
  alternativos (3, 4 y 6), para cuando se pide con la marca 0 y la pista
  tiene sonidos para ellos; y el 5. Las tablas `looping_sound_unknown_0044a150`
  y `looping_sound_unknown_0044a16c` dicen a qué estado se pasa desde cada
  uno, por un camino o por el alternativo.
- Al empezar suena el sonido de entrada: en lugar de lo que la pista
  tuviera, o con ello fundiéndose si la pista tiene la marca 0.
- Al empezar o seguir se busca el estado al que toca ir: suena el sonido
  del primer estado del camino que tenga uno, cruzándose con el anterior en
  medio segundo o en lo que diga la pista; los de los estados que las
  marcas de la pista dejan fuera solo se arrancan.
- Al acabar, los sonidos de la pista se apagan en los segundos que se
  piden; si no se piden, en lo que diga la pista, y suena el sonido de
  salida, o el primer sonido de la pista queda marcado para pasar a él
  (tipo 3).
- Al final, uno sin sonidos sonando y sin oyente lo bastante cerca se
  suelta; y se queda con tres de las marcas y con lo que se pidió.
- Lo que devuelve: verdadero si no había nada que hacer. Una rareza: si el
  sonido no está en marcha devuelve si se pedía acabar, porque el juego
  guarda eso en el mismo sitio de la pila donde luego guarda el resultado.
  Lo cazaron las pruebas: al principio creí que devolvía basura.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_effect_start_sounds`. La última batería completa es la del hito
  258 (928 funciones, todas bien); la siguiente se lanza con este hito.

### Mutaciones

Setenta y dos cambios a mano: las pruebas notan setenta. Los dos que no
notan no cambian nada: son dos bits de las marcas de la pista (los de los
estados 0 y 6) que el juego prepara pero a los que el camino entre estados
nunca llega.

## 2026-10-05 — Dónde está cada oyente

Una función en `sound/sound_manager.c`, de 2.290 bytes. De 929 funciones se pasa a
930, el 8,26 % de las del juego (el 5,417 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_listeners_update` | `0x00127400` | `sound/sound_manager.c` |

Era `unknown_00127400`, la última de las funciones grandes que
`sound_update` llama cada fotograma. Recibe un byte en la pila. Nuevas sin
descompilar: `0x00155760` y `0x000caf60`. Nueva variable:
`unknown_004e9c8c`.

### Lo que se sabe

- Solo hace algo con una partida en marcha. Hay cuatro oyentes, uno por
  jugador de la consola; el del jugador que no está se queda sin sitio
  (`NONE`).
- Los demás están donde dice `unknown_004e9c8c` para su jugador (`0x358`
  bytes cada uno: un punto, dónde cae en el nivel, una velocidad, hacia
  dónde mira y hacia dónde es arriba; será la cámara).
- Si la unidad del jugador va en un asiento y `0x00155760` da 1 para el
  oyente, el oyente se acerca a la unidad (`0x000caf60` dice dónde está) y
  gira hacia donde mira la unidad, en la parte que diga el asiento en
  `0x40`; entonces se mueve como la unidad y está en el nivel donde caiga
  su punto.
- El oyente se queda con sus ejes (adelante, izquierda y arriba), su punto
  y lo deprisa que se mueve visto a lo largo de sus ejes.
- Su byte de `0x7` dice que su cluster tiene algo del nivel (el byte de
  `0x70` del cluster, en el bloque de `0x68` del structure bsp): sin más si
  ese byte no tiene el bit alto, y solo por detrás de su plano si lo
  tiene.
- Con el byte de la pila puesto, después se le dice a la consola quién oye
  (`sound_console_set_listener`): en el origen y mirando al frente, porque
  los sonidos se colocan donde los ven los oyentes; y con los dos estados
  del sonido de los sitios (hito 207) como sus dos sitios, lo que confirma
  lo que allí se suponía.
- El giro por un cuaternión está escrito en línea, operación por
  operación, como lo tiene el juego: las pruebas comparan los reales bit a
  bit.
- El nombre es inventado.

### Sin comprobar

- Con el índice del nivel a `NONE` el juego leería de la dirección 0; las
  pruebas siempre tienen nivel.
- El juego divide la velocidad por la escala del oyente si no es 1, pero
  acaba de ponerla a 1: no se ha escrito.
- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_update`. La última batería completa es la del hito 258 (928
  funciones, todas bien).

### Mutaciones

Cuarenta y cuatro cambios a mano: las pruebas notan los cuarenta y cuatro,
uno de ellos solo después de dar a las pruebas la tabla completa del
nivel.

## 2026-10-05 — Se prepara el sonido de la consola

Una función en `sound/sound_manager.c`, de 1.584 bytes. De 928 funciones se pasa a
929, el 8,25 % de las del juego (el 5,340 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_initialize` | `0x0021e4b0` | `sound/sound_manager.c` |

Era `unknown_0021e4b0`. La llama `sound_initialize`, con cuántas voces de
cada tipo y cuántas casillas quiere el juego. Devuelve si todo fue bien.
Once funciones nuevas sin descompilar, casi todas de la consola
(`0x0037d797`, `0x0037b5ae`, `0x0037d506`, `0x0037d5cd`, `0x0037b5ca`,
`0x0037b637`, `0x0037d52a`, `0x00379f5b`, `0x00191300`, `0x0012c040` y
`0x00191420`). Nueva variable: `unknown_0047f088`.

### Lo que se sabe

- La memoria del sonido de la consola (`0x2AD8` bytes) sale de la que el
  juego toma al arrancar (`shell_memory_allocate`).
- La consola crea su sonido y se le preguntan y dicen varias cosas. El byte
  de `0x1` dice si los altavoces son de un tipo (un 2 con la marca 16). Si
  algo de esto falla, o el procesador de sonido no recibe su programa
  (`0x00191300`), se desmonta todo y devuelve falso.
- Quien oye empieza mirando al frente (`sound_console_set_listener`).
- Las voces reciben sus streams, tantas de cada uno de tres tipos como diga
  el juego; dónde empieza y acaba cada tipo se guarda en `0x1A0C` y
  `0x1A12`. Después, las casillas. En cuanto uno falla no se crean más.
- Para el sonido de los sitios se crean tres parejas de cosas (en `0x2A58`
  y `0x2A64`, de las salidas 23 a 25), quien lleva ese sonido pasa a ser
  `unknown_0047f088`, y cada uno de los dos sitios tiene cuatro streams (en
  `0x2A70`, de las salidas 15 a 22 hacia las salidas 6 a 9).
- Los dos streams de `0x2A90` empiezan en silencio, y las ganancias en 1.
- Si todo fue bien, el primer byte del sonido de la consola pasa a 1.
- El nombre es una suposición.

### Arreglos

- `sound_console_voice_create_stream` (hito 215) devuelve si la consola
  creó el stream; estaba como si no devolviera nada, y esta función lo
  mira.
- `unknown_0047f0d0` estaba apuntada con `0x20` bytes, y así pisaba a
  `unknown_0047f0e0` y `unknown_0047f0e4`, que son dos de sus números. Se
  ha dejado en `0x10` bytes.

### Batería completa

La del hito 258 terminó: 928 funciones, las 928 bien.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de las que
  tienen que ver con ella.

### Mutaciones

Cuarenta y dos cambios a mano: las pruebas notan los cuarenta y dos.

## 2026-10-05 — Un archivo nuevo: lo que el juego hace con los mandos

Cuatro funciones nuevas y una que cambia de sitio, en el archivo nuevo `input/input_abstraction.c`.
De 924 funciones se pasa a 928, el 8,24 % de las del juego (el 5,287 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `input_abstraction_initialize` | `0x00217bc0` | `input/input_abstraction.c` |
| `input_abstraction_dispose` | `0x00217c20` | `input/input_abstraction.c` |
| `input_abstraction_handle_gamepad_changes` | `0x002184a0` | `input/input_abstraction.c` |
| `input_abstraction_controller_unknown_00218510` | `0x00218510` | `input/input_abstraction.c` |

Dos de las funciones que quedaban en la lista «del sonido» (`0x002184a0` y
`0x00218510`) no eran del sonido: estaban en su rango de direcciones, pero
son de un módulo vecino, el que convierte los mandos en lo que hace cada
jugador. Su función grande (`0x00217c40`, 1.993 bytes) sigue sin
descompilar. El nombre del archivo es una suposición, tomada de juegos
posteriores.

### Lo que se sabe

- `input_abstraction_initialize` y `input_abstraction_dispose` son las dos
  funciones de uno de los sistemas del juego (estaban en la tabla de
  `game.c` como `unknown_00217bc0` y `unknown_00217c20`). La primera deja
  todo a cero, pone cada mando como lo deja
  `input_abstraction_preferences_set_defaults`, apunta qué mandos están
  enchufados y la hora.
- `input_abstraction_preferences_set_defaults` (`0x00218420`) ya estaba
  descompilada dentro de `sound_manager.c` como
  `unknown_00218420_set_defaults`, sin saber de qué era. Se ha movido aquí.
- `input_abstraction_handle_gamepad_changes` es a la que
  `input_update_devices` avisa de los mandos que se enchufan y
  desenchufan. Si cambia alguno pasados dos segundos del arranque (o de
  enchufar el primero), para algo que está en marcha (`0x001565a0`, que
  puede volver al menú principal), salvo que eso tenga una de dos marcas.
  Y se queda con la hora del primer mando que se enchufa.
- `input_abstraction_controller_unknown_00218510` dice si la unidad del
  jugador de un mando va en un asiento con la marca 2, de algo cuyo tag
  tiene un 3 o un 5 en `0x1F0`. La llama `0x00217c40`.
- Nuevas variables: `input_abstraction_globals` (`0x0051ea18`),
  `player_data` (`0x004e8c24`, el array de los jugadores; el nombre es una
  suposición), `unknown_0055e77c`, `unknown_004e9188` y
  `unknown_004e9190`. `players_globals` tiene, tras los jugadores de la
  consola, el jugador de cada mando.

### Sin comprobar

- En este hito se pasaron las pruebas del archivo nuevo y las de los
  archivos tocados (`input`, `game`, `main`). La última batería completa
  es la del hito 254 (921 funciones, todas bien).

### Mutaciones

Veinticinco cambios a mano: las pruebas notan los veinticinco.

## 2026-10-05 — Cada fotograma, con cada sonido en bucle

Una función en `sound/sound_manager.c`, de 1.917 bytes. De 923 funciones se pasa a
924, el 8,20 % de las del juego (el 5,277 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sounds_update` | `0x0021b0e0` | `sound/sound_manager.c` |

Era `unknown_0021b0e0`. La llama `sound_update` una vez por fotograma, con
un byte del sonido del juego. No devuelve nada.

### Lo que se sabe

- Con el byte puesto, un sonido en bucle que no se ha mantenido este
  fotograma (su byte de `0x53` no es el del sonido del juego) se suelta,
  salvo que tenga la marca 1 y sonidos sonando.
- Para cada pista de los demás, la cuenta de sus sonidos con canal vuelve a
  0 y, si el sonido en bucle no está en el estado 2, el real de la pista
  se acerca al del sonido en bucle: tan deprisa como diga la reproducción
  del sonido de su primer sonido (por segundo), o de golpe si no dice nada.
- Después, con el byte puesto, se mira qué rango de tono toca para el tono
  que resulta. Un sonido con la marca 13 hace sonar a la vez todos sus
  rangos de tono: cada rango que tiene ese tono tiene un sonido del tipo 2
  en el anillo de la pista (se crea si falta), tan fuerte como dice la
  pista más lo que dice `sound_pitch_range_fade_decibels`; los de los
  rangos que no lo tienen se sueltan (motivo 8). Los demás sonidos pasan al
  sonido del rango nuevo, encontrado o creado, en tres cuartos de segundo
  (`sound_sources_cross_fade`), y ese pasa a ser el primero de la pista.
- La semilla es la del mando de sonidos del sonido en bucle, que saca un
  número nuevo si el mando no tiene nada en `0x4`.
- Al final, si el sonido en bucle no está en el estado 2 ni tiene la marca
  2, suena cada detalle de su tag al que le ha llegado la hora
  (`looping_sound_detail_request`, `sound_source_new`), y se le pone la
  hora siguiente: dentro de unos segundos al azar entre sus dos límites,
  en milésimas. Un detalle con la marca 0 no suena si el sonido en bucle
  tiene la marca 0, y uno con la marca 1 solo suena entonces.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_update`. La última batería completa es la del hito 254 (921
  funciones, todas bien).

### Mutaciones

Cuarenta y tres cambios a mano: las pruebas notan cuarenta y dos, dos de
ellos solo después de añadir casos (una velocidad entre una diezmilésima y
una milésima, y una hora que es justo la de ahora). El que no notan es un
cambio mío flojo: cortar el recorrido del anillo a los seis pasos, que con
los anillos de las pruebas da lo mismo.

## 2026-10-05 — Cada vez, con el sonido de una pista que tiene voz

Una función en `sound/sound_manager.c`, de 1.929 bytes. De 922 funciones se pasa a
923, el 8,19 % de las del juego (el 5,212 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_channel_update_voice` | `0x0021c400` | `sound/sound_manager.c` |

Era `unknown_0021c400`. La llama la función que cada fotograma atiende a
los canales, para los sonidos de las pistas de los sonidos en bucle (los
tipos 1 a 4). Recibe en la pila el canal, cuánto se oye y lo que calculó
`sound_channel_compute_parameters`. Es la hermana de
`sound_channel_update_voice` (hito 205), que hace lo mismo para los demás
sonidos.

### Lo que se sabe

- Como su hermana, da a la voz lo que rellena
  `sound_channel_build_voice_parameters`, con el tono (torcido por el real
  de la pista, menos el de las grabaciones tal como se hicieron, y con el
  efecto Doppler si el sonido tiene voz de sonido) y el volumen, más la
  ganancia de la grabación. Un sonido sin la marca 9 arranca; los demás
  siguen (`sound_console_voice_set_parameters`).
- Lo propio de las pistas es cómo se elige el trozo siguiente cuando no hay
  ninguno esperando: el siguiente de la grabación; o, al acabarse esta,
  otra grabación (con los bits de las ya usadas: los de la pista para su
  primer sonido, los del propio sonido para los demás), salvo que el tag
  del sonido en bucle tenga la marca 1, que acaba el sonido (pasa al tipo
  4).
- El primer sonido de una pista a la que le toca cambiar de estado
  (`looping_sound_unknown_0021a050`) cambia al acabar su grabación, o antes
  si la pista lo permite (`looping_sound_unknown_0021a080`) y el trozo
  tiene su bit más alto: `looping_sound_source_next_state` (hito 239). Si
  eso suelta el sonido, no se hace nada más.
- Un trozo que espera se pone en cola cuando sus muestras están listas,
  salvo que la voz esté en el estado 3. Si con él se acaba la grabación de
  un sonido de entrada (tipo 1) o de salida (tipo 3) que no va a cambiar de
  estado, el sonido pasa al tipo siguiente.
- El primer sonido de una pista se queda al final con la semilla y las
  grabaciones usadas de la pista.
- El juego solo mira el byte bajo de lo que devuelve
  `looping_sound_unknown_0021a050`.
- El nombre es inventado.

### Sin comprobar

- Las pruebas dan a `looping_sound_unknown_0021a050` solo los valores que
  esa función puede dar (0 y 1): con otros, el compilador de `source/` no
  hace la conversión a byte que el juego sí hace.
- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 254 (921 funciones, todas bien).

### Mutaciones

Cuarenta y dos cambios a mano: las pruebas notan los cuarenta y dos.

## 2026-10-05 — Una voz de la consola toma cómo se oye su sonido

Una función en `sound/sound_manager.c`, de 1.893 bytes. De 921 funciones se pasa a
922, el 8,18 % de las del juego (el 5,148 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voice_set_parameters` | `0x0021fa80` | `sound/sound_manager.c` |

Era `unknown_0021fa80`. La llaman tres sitios, entre ellos
`sound_channel_update_voice`. Recibe en `edi` lo que se pide para la voz
y, en la pila, la voz, sus efectos y un byte. Nuevas sin descompilar, de la
consola: `0x0037b83f`, `0x0037b7e3`, `0x0037b7ff`, `0x0037b804` y
`0x0037b71b`. Quien lleva el sonido de los sitios resulta tener siete
funciones, no tres: las cuatro nuevas rellenan las salidas por las que se
oye una voz.

### Lo que se sabe

- A la consola solo se le dice lo que ha cambiado: el volumen y lo demás,
  si cambian en una décima o más; el tono, en 1,5 unidades o más. Todo se
  dice si la voz es nueva (no tiene la marca 0 de su byte de `0x3`, o el
  sonido de la consola tiene un 0 al principio).
- El volumen (de -64 a 0 decibelios, en centésimas) va al stream de la voz.
  Si la voz tiene una casilla del sonido de la consola (un número que no
  queda por debajo del short de `0x8`), el volumen va a la salida de la
  casilla (una vez, o dos si los tres bits de la voz valen 1) y el stream
  suena al máximo.
- Dónde se oye lo decide quien lleva el sonido de los sitios, que rellena
  las salidas. Con casilla, son las de lo que hace sonar la casilla en
  `0x38`, que arranca o se para según haya o no salidas, y el volumen de lo
  de `0x34`. Sin casilla, las del stream de la voz.
- Con la marca 5 de lo que se pide, los cuatro volúmenes de `0x38` se
  juntan en uno (la raíz de la suma de los cuadrados de sus ganancias).
- El tono va en las unidades de la consola (4.096 por octava), desde una
  base por cada uno de tres tipos de muestras (-4597, -501 y -2396), entre
  -32767 y 8191. No se dice si el byte de la pila está puesto.
- Al final llama a `sound_console_voice_apply_effects` (hito 254) con un 0.
- El nombre es inventado.

### Batería completa

La del hito 254 terminó: 921 funciones, las 921 bien.

### Sin comprobar

- El juego tiene un camino al que no se puede llegar: con casilla y sin
  volumen de casilla le diría a la consola -10000, pero una voz con casilla
  siempre lo trae. Está escrito como en el juego y las pruebas no lo pisan.
- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_channel_update_voice`.

### Mutaciones

Cincuenta cambios a mano: las pruebas notan cuarenta y nueve, tres de
ellos solo después de añadir casos (diferencias de justo una décima y de
poco más, y una casilla negativa que no es `NONE`). El que no notan es el
del camino al que no se llega.

## 2026-10-05 — El stream de una voz toma los efectos de su sonido

Una función en `sound/sound_manager.c`, de 1.344 bytes. De 920 funciones se pasa a
921, el 8,18 % de las del juego (el 5,084 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voice_apply_effects` | `0x002201f0` | `sound/sound_manager.c` |

Era `unknown_002201f0`. La llaman `sound_console_voice_start` y
`0x0021fa80`. Recibe la voz en `eax`, los efectos en `ebx` y un byte en la
pila. Nuevas sin descompilar, de la consola: `0x0037b813` (el filtro de un
stream) y `0x0037b809`.

### Lo que se sabe

- Los efectos de una voz empiezan con una semilla y una fracción, y tienen
  hasta tres cosas: un filtro (en `0x10`) y otras dos (en `0x14` y `0x18`),
  más la lista de `sound_console_apply_effect_list` (en `0x1C`, con su
  tamaño en `0x41C`).
- Cada número sale de cuatro reales con `sound_unknown_00219880`, la
  semilla y la fracción.
- El filtro es de tres tipos, con dos frecuencias y dos volúmenes. A la
  consola se le dice un 2, un 1 o un 3 según el tipo; el tercero solo vale
  para una voz que tiene a 0 tres bits de su byte de `0x1`.
- Con el byte de la pila a 0 solo se dice el filtro, y ni eso si la
  fracción es la que la voz ya tenía (con una diezmilésima de margen). Con
  el byte puesto se dicen también las otras dos cosas: la primera con tres
  números (hasta `0xFFFF`, hasta `0x3FF` y de -128 a 127) y la segunda con
  cuatro (los dos últimos de -128 a 127 y de -128 a 128); y lo que los
  efectos no traen se dice como «nada».
- La voz se queda con la fracción, y al final la lista va a la consola.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 244 (911 funciones, todas bien).

### Mutaciones

Treinta y cuatro cambios a mano: las pruebas notan treinta y dos. Los dos
que no notan no cambian nada (sumar un cero, y quitar una condición que
solo deja pasar a donde no se hace nada).

## 2026-10-05 — La consola aplica lo que cambió de su sonido

Una función en `sound/sound_manager.c`, de 1.005 bytes. De 919 funciones se pasa a
920, el 8,17 % de las del juego (el 5,039 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_commit` | `0x0021ee80` | `sound/sound_manager.c` |

Era `unknown_0021ee80`. La llaman dos sitios, una vez por fotograma. No
recibe ni devuelve nada. Nuevas sin descompilar: `0x0037d141` (la consola
hace de una vez todo lo que se le ha dicho) y `0x00191550`. Nueva tabla:
`sound_console_place_effects` (`0x0044a250`).

### Lo que se sabe

- Es la otra mitad de `sound_console_set_listener` (hito 240): aquella
  guardaba cómo suenan los dos sitios que se oyen y dejaba dos avisos por
  sitio; esta mira los avisos y se lo dice a la consola.
- Si ha cambiado cómo suena un sitio, le da su efecto del procesador de
  sonido (el 9 al primero y el 8 al segundo) con `sound_console_set_reverb`.
- Si ha cambiado lo demás, pone el volumen de sus cuatro streams (en
  `0x2A70`), uno por cada medio círculo alrededor del oyente: delante a la
  izquierda, delante a la derecha y los dos de detrás. Los dos primeros
  números del sitio son un arco (de un ángulo a otro); lo que de ese arco
  cae en el medio círculo de un stream, partido por π, se acerca al cuarto
  número según lo que al tercero le falte para 1, y se multiplica por
  cuánto hay del sitio. El resultado va en centésimas de decibelio, de
  -6400 a 0.
- Antes de todo, si `unknown_0047005c` está puesta, le pasa
  `unknown_00470060` a quien lleva el sonido de los sitios, como hace
  `sound_console_set_environment_handler`.
- Al final le dice a la consola que lo haga todo (`0x0037d141`), pasa cada
  voz a `0x002ae820` y llama a `0x00191550`.
- El juego tiene alrededor de la llamada de cada voz dos saltos por tabla
  que van todos al mismo sitio: no hacen nada (restos de algo que se quitó).
- El nombre es inventado.

### Sin comprobar

- Las tablas de ángulos están en la pila del juego y no se pueden comparar
  directamente: se comparan los volúmenes que salen de 1.500 casos.
- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 244 (911 funciones, todas bien).

### Mutaciones

Veintisiete cambios a mano: las pruebas notan veintiséis. El que no notan
es mover uno de los ángulos una millonésima: el volumen sale en centésimas
de decibelio y no llega a cambiar. Los ángulos están copiados a mano de
los del juego (múltiplos de π/4), y las pruebas solo los comprueban hasta
ahí. La prueba compara además la tabla de los efectos con la del juego.

## 2026-10-05 — Se crea el sonido de una pista de un sonido en bucle

Una función en `sound/sound_manager.c`, de 832 bytes. De 918 funciones se pasa a
919, el 8,16 % de las del juego (el 5,005 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_source_new` | `0x0021b940` | `sound/sound_manager.c` |

Era `unknown_0021b940`. La llaman tres sitios, entre ellos
`looping_sound_get_or_new_source`. Recibe en la pila el sonido en bucle, el
sonido, el rango de tono (o `NONE`), la pista, qué es el sonido de la pista
y dónde está la semilla; devuelve el sonido que suena, o `NONE`. Nueva
tabla: `looping_sound_track_callbacks` (`0x0044a110`), con las tres
funciones de los hitos 249 a 251, `looping_sound_get_source` y
`0x000a5750`, que solo dice que sí.

### Lo que se sabe

- No crea nada si el sonido no puede sonar ahora
  (`sound_definition_unknown_00126bd0`), si ningún oyente está lo bastante
  cerca, o si no queda sitio.
- El sonido nuevo dice de sí mismo lo que dice el sonido en bucle, suena
  tan fuerte como dice la pista y tiene de tono el límite bajo del tono del
  sonido. Aquí el juego tiene una rareza: saca un número de la semilla para
  elegir el tono entre dos límites, pero usa el mismo límite las dos veces,
  así que el número no cambia nada (la semilla sí avanza).
- Se le avisa por `looping_sound_track_callbacks`, con la pista y lo que
  la pista tiene de semilla y de grabaciones ya usadas; y empieza solo en
  su anillo.
- Si el tag del sonido en bucle tiene la marca 5, cuenta en el mando de
  sonidos del sonido en bucle.
- Sin rango de tono, se elige por el tono, torcido por el real de `0x10`
  de la pista entre los dos límites de la escala del sonido.
- La grabación se elige como en `looping_sound_source_next_state` (hito
  239), y después el sonido entra en el anillo de su pista y el sonido en
  bucle cuenta uno más.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de
  `looping_sound_get_or_new_source`. La última batería completa es la del
  hito 244 (911 funciones, todas bien).

### Mutaciones

Treinta y nueve cambios a mano: las pruebas notan los treinta y nueve. La
prueba compara además la tabla con la del juego.

## 2026-10-05 — Un sonido de una pista de un sonido en bucle se suelta

Una función en `sound/sound_manager.c`, de 252 bytes. De 917 funciones se pasa a
918, el 8,15 % de las del juego (el 4,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_track_source_deleted` | `0x0021adb0` | `sound/sound_manager.c` |

No estaba apuntada: es la tercera de las funciones de la tabla de
`0x0044a110` (la de las pistas de los sonidos en bucle), y no está en la
lista de Ghidra.

### Lo que se sabe

- Los sonidos de una pista están en un anillo: cada uno dice cuál va
  después y cuál antes. Al soltarse uno, sale del anillo: el de antes y el
  de después se apuntan el uno al otro.
- Si el sonido en bucle sigue ahí, cuenta un sonido menos (su byte de
  `0x51`); y si el que se suelta era el primero de su pista, el primero
  pasa a ser el de después, o ninguno si estaba solo.
- Si el sonido tiene mando de sonidos, el mando sigue ahí y tiene algo en
  `0x4`, y el byte de `0x6` de los bytes de a quién se avisa es `0xFF`, la
  cuenta del mando baja en uno. A diferencia de
  `sound_playback_controller_add_count`, aquí se pierde la marca que
  comparte byte con la cuenta.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 244 (911 funciones, todas bien).

### Mutaciones

Catorce cambios a mano: las pruebas notan los catorce, uno de ellos solo
después de añadir un caso (un «siguiente» que cae en la misma casilla que
el propio sonido pero es otro).

## 2026-10-05 — Cómo se oye un sonido de una pista de un sonido en bucle

Una función en `sound/sound_manager.c`, de 223 bytes. De 916 funciones se pasa a
917, el 8,14 % de las del juego (el 4,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_track_source_heard` | `0x0021acc0` | `sound/sound_manager.c` |

No estaba apuntada: es otra de las funciones de la tabla de `0x0044a110`
(la de las pistas de los sonidos en bucle), y no está en la lista de
Ghidra.

### Lo que se sabe

- Solo contesta un sonido en bucle que sigue ahí y cuyo número de `0x8` es
  su tag con las marcas `0x6000`.
- Contesta con cuatro reales, en `0x28` de lo que se pregunta: los
  decibelios de cada una de las cuatro ganancias que el sonido en bucle
  guarda al final (`0xC4`), tal como los tiene el sonido
  (`sound_definition_decibels`, con el real de `0x4` de lo que el sonido
  en bucle dice de sus sonidos), más el real de `0x8` de eso mismo. Y pone
  la marca 4 de lo que se pregunta.
- Devuelve falso si no contesta.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 244 (911 funciones, todas bien).

### Mutaciones

Once cambios a mano: las pruebas notan los once.

## 2026-10-05 — Una pista de un sonido en bucle cuenta sus canales

Una función en `sound/sound_manager.c`, de 142 bytes. De 915 funciones se pasa a
916, el 8,13 % de las del juego (el 4,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_track_source_got_channel` | `0x0021ac30` | `sound/sound_manager.c` |

No estaba apuntada: solo se llega a ella por la tabla de `0x0044a110`, la
de a quién se avisa de los sonidos de las pistas de un sonido en bucle, así
que no está en la lista de Ghidra. Esa tabla hace falta para `0x0021b940`,
y tiene otras tres funciones que irán en los hitos siguientes.

### Lo que se sabe

- Se llama cuando un sonido de una pista consigue canal. La pista (el byte
  de `0x4` de los bytes de a quién se avisa) cuenta uno más en su byte de
  `0xE`.
- No cuenta si el sonido se está apagando (la marca 7 y el tiempo de
  apagado por delante del de `0xB8`), ni si el sonido en bucle ya no está.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 244 (911 funciones, todas bien).

### Mutaciones

Ocho cambios a mano: las pruebas notan los ocho.

## 2026-10-05 — Un efecto de sonido arranca sus sonidos

Una función en `sound/sound_manager.c`, de 820 bytes. De 914 funciones se pasa a
915, el 8,12 % de las del juego (el 4,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_start_sounds` | `0x0021d630` | `sound/sound_manager.c` |

Era `unknown_0021d630`. La llaman seis sitios, con un 0 cuando el efecto
empieza, un 1 mientras dura y un 2 cuando acaba. Nueva sin descompilar:
`0x0021a250`, la que arranca un sonido en bucle. Nuevas tablas:
`sound_effect_looping_sound_states` (`0x0044a200`) y
`sound_effect_looping_sound_unknown_states` (`0x0044a20c`); nueva variable:
`unknown_004ed28c`.

### Lo que se sabe

- La definición de un efecto de sonido tiene una lista de sonidos, de 16
  bytes cada uno: el grupo del tag (`snd!` o `lsnd`), el tag, un real y unas
  marcas.
- Un sonido normal solo arranca al empezar (si no tiene la marca 0) o al
  acabar (si tiene la marca 1). Arranca como cualquier otro del juego
  (`sound_start_test`, `sound_promotion_update`, `sound_source_create`),
  con lo que `sound_effect_get_source` dice del efecto y el real como
  ganancia. Al empezar lleva además el mando de sonidos del dueño del
  efecto, y se le avisa por `sound_effect_sound_callbacks` (hito 247) con
  el efecto apuntado.
- Un sonido en bucle se pasa a `0x0021a250` en los tres momentos, con el
  real sumado al de `0x8` de lo que el juego dice del sonido, el estado
  que toca (0, 1 o 2) y dos marcas: la 0, si el sonido de la lista tiene
  la marca 3, o la 4 cuando el efecto tiene las suyas 1 y 4; y la 1, al
  acabar.
- En un efecto del tipo 2, un sonido en bucle con la marca 5 toma el
  estado y la primera marca de otra cosa del juego (`unknown_004ed28c`),
  que no se sabe qué es.
- El short de `0x6` del efecto va sin su bit 0 si el tag del sonido en
  bucle tiene la marca 3, y sin su bit 1 si tiene la 6.
- Nada arranca si el sonido no está en marcha (los tres bytes de `0x78`
  del sonido del juego).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de dos
  que la llaman. La última batería completa es la del hito 244 (911
  funciones, todas bien).

### Mutaciones

Treinta y ocho cambios a mano: las pruebas notan treinta y seis. Los dos
que no notan no cambian nada: mirar la otra cosa del juego también en los
efectos del tipo 1 (no se usa) y comparar con 1 lo que devuelve
`sound_promotion_update`, que el compilador sabe que solo da 0 o 1. La
prueba compara además las dos tablas con las del juego.

## 2026-10-05 — Lo primero que se dice a un sonido que arranca un efecto

Una función en `sound/sound_manager.c`, de 121 bytes. De 913 funciones se pasa a
914, el 8,11 % de las del juego (el 4,949 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_sound_unknown0` | `0x0021d970` | `sound/sound_manager.c` |

No estaba apuntada: solo se llega a ella por una tabla, así que no está en
la lista de Ghidra (cuenta en las funciones, no en el total). Es la única
función de la tabla `sound_effect_sound_callbacks` (`0x0044a1e0`), que
también es nueva: la usa `0x0021d630` para los sonidos que un efecto de
sonido arranca por su cuenta.

### Lo que se sabe

- Recibe lo mismo que la primera función de `sound_effect_callbacks` (hito
  219): los bytes de a quién se avisa, con el efecto al final, y dónde
  dejar lo que el juego dice del sonido.
- Dice que no si el efecto ya no está (no hay índice, el índice no está en
  uso o el identificador no es el suyo) o si tiene la marca 0.
- Si está, dice que sí; y si no tiene la marca 4, lo que el juego dice del
  sonido pasa a ser lo del efecto (`sound_effect_get_source`).
- La diferencia con la del hito 219 es que aquella no mira si el efecto
  sigue ahí.
- El nombre es inventado.

### Batería completa

La del hito 244 terminó: 911 funciones, las 911 bien.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función.

### Mutaciones

Once cambios a mano: las pruebas notan ocho. De los otros tres, dos no
cambian nada (quitar la comprobación de que hay índice: un índice `NONE`
nunca está en uso; y comparar el identificador sin signo) y el tercero es
devolver 2 en vez de 1, que las pruebas dan por bueno porque comparan lo
que devuelve como verdadero o falso. La prueba compara además la tabla con
la del juego.

## 2026-10-05 — Un resto de comprobación de los sonidos en bucle

Una función en `sound/sound_manager.c`, de 162 bytes. De 912 funciones se pasa a
913, el 8,10 % de las del juego (el 4,949 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_check_distances` | `0x0021d060` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021a250`, la que arranca un sonido en
bucle. Recibe el tag en `eax` y la fuente del sonido en `ebx`; no devuelve
nada. Nueva variable: `unknown_004e636d`.

### Lo que se sabe

- Solo hace algo con `unknown_004e636d` puesta y una fuente del tipo 1.
- Pide las dos distancias (`sound_get_minimum_distance` y
  `sound_get_maximum_distance`) del sonido de la primera pista que tenga
  uno. Tiene una rareza: mira siempre la primera pista, tantas veces como
  pistas hay.
- Si no hay ninguna, o la distancia más corta es 0, recorre los detalles
  hasta el primero que tenga sonido.
- No hace nada con lo que encuentra: lo único que se nota desde fuera son
  las dos llamadas. Será una comprobación que avisaba de algo en las
  versiones de desarrollo y de la que en esta solo queda el recorrido.
- El nombre es inventado.

### Sin comprobar

- El recorrido de los detalles no deja nada que comparar: se ha escrito
  como lo hace el juego, pero las pruebas no pueden notarlo.
- En este hito solo se pasaron las pruebas de esta función. La batería
  completa del hito 244 sigue en marcha; la última terminada es la del
  hito 240 (907 funciones, todas bien).

### Mutaciones

Diez cambios a mano: las pruebas notan los diez.

## 2026-10-05 — Un stream de la consola toma la forma de una función

Una función en `sound/sound_manager.c`, de 381 bytes. De 911 funciones se pasa a
912, el 8,10 % de las del juego (el 4,944 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_stream_set_function` | `0x0021f720` | `sound/sound_manager.c` |

Era `unknown_0021f720`. La llaman cinco sitios, entre ellos
`sound_channel_update_voice`. Recibe en la pila lo que una voz tiene de su
entorno. Nuevas sin descompilar, de la consola: `0x0037b7b3` y
`0x0037b797`.

### Lo que se sabe

- El sonido de la consola tiene dos streams aparte (en `0x2A90`), y de cada
  uno recuerda un número (en `0x2A98`). Lo que una voz tiene de su entorno
  dice cuál de los dos (el short del principio) y trae una función de un
  tag y un número.
- Si el número no es el de la última vez,
  `sound_console_stream_set_function` para el stream, pide a la consola
  dónde escribir mil bytes y escribe en cada uno, de 0 a 255, lo que vale
  la función (`0x0013b390`) de 0 a 1 en pasos de una milésima, llevado
  entre los dos reales que trae la función salvo que tenga alguna de cuatro
  marcas (lo mismo que en el hito 244). Sin función, ceros. Después lo
  suelta y se queda con el número.
- Siempre, al final, le dice dos cosas al stream con un 0 (`0x0037b797` y
  `0x0037b66f`; la segunda es la que en otros sitios recibe -10000, así que
  será el volumen).
- No hace nada si el short de `0x2` no está entre 0 y 31, o si el del
  principio no es 0 o 1.
- El juego tiene un tope de 255 para el byte que no puede pasar nunca: un
  byte ya no pasa de 255. Se escribe el byte bajo del entero.
- El nombre es inventado.

### Aclaración del hito 241

- Allí puse como «arreglo» que `unknown_00020670` quita sus argumentos de
  la pila. Es verdad que la del juego lo hace, pero no era un error: la
  tabla solo tiene que decir lo mismo que el prototipo de C, y las dos
  cosas decían que no. Hay unas cuarenta funciones sin descompilar
  apuntadas así, y está bien mientras no se descompilen. El cambio se deja
  porque no molesta.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_channel_update_voice`. La batería completa del hito 244 está en
  marcha; la última terminada es la del hito 240 (907 funciones, todas
  bien).

### Mutaciones

Veinticuatro cambios a mano: las pruebas notan los veinticuatro.

## 2026-10-05 — Los números de un efecto de sonido para la consola

Una función en `sound/sound_manager.c`, de 722 bytes. De 910 funciones se pasa a
911, el 8,09 % de las del juego (el 4,931 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_entry_build` | `0x002222c0` | `sound/sound_manager.c` |

Era `unknown_002222c0`. La llama `sound_unknown_002225a0` con cada elemento
de los efectos de un tag. Recibe la entrada que escribe en `eax` y, en la
pila, el elemento, lo que hay en su lugar (o nada), los números del sonido
y un número de 0 a 3 (o `NONE`). Nuevas sin descompilar: `0x00191660` y
`0x0013b390`.

### Lo que se sabe

- Una entrada empieza con tres números: para qué efectos de la consola es
  (lo dice `0x00191660` con el nombre del elemento y el número) y los dos
  que el elemento trae en `0x8`.
- Después va un número por cada parámetro del elemento. Si el elemento
  tiene la marca 0, antes va uno más que dice de qué tipo es: 10 para el
  nombre `0x0D00012E`, 4 para `0x1300012D` y `NONE` para los demás; y los
  reales se escriben tal cual. Sin la marca, los reales se escriben como
  los da `sound_real_to_fixed24`. Los enteros se redondean al más cercano.
- Cada parámetro (16 bytes) dice su tipo (0 entero, 1 real), su valor de
  partida y entre qué dos valores tiene que quedar.
- Lo que hay en lugar del elemento puede cambiar un parámetro (un bit por
  cada uno) por un real fijo o por una función de un tag
  (`0x0013b390`) de dos números del sonido. El resultado de la función se
  lleva entre los dos reales que trae, salvo que tenga alguna de cuatro
  marcas.
- De los números del sonido, el 1 se divide por un real de la función y se
  queda solo con lo que hay tras la coma (un ciclo que se repite); el 3 se
  multiplica por lo que dice `sound_unknown_00222250` de la dirección que
  hay en `0x10` y el número de la entrada.
- El nombre es inventado.

### Un error que cazaron las pruebas

- Tomé `0x00373b6a` por la potencia de la biblioteca de C (elevar a 1).
  Las pruebas daban distinto, y la tabla que usa dice `fmod`: es el resto
  de dividir por 1. A la biblioteca de las pruebas se le ha añadido
  `fmodl`.

### Sin comprobar

- Funciones sin datos (puntero a 0): el juego lee un byte de ellos antes de
  mirar si los hay, y las pruebas no tienen memoria en esa dirección.
- En este hito solo se pasaron las pruebas de esta función y las de
  `sound_unknown_002225a0`. La última batería completa es la del hito 240
  (907 funciones, todas bien).

### Mutaciones

Treinta y cuatro cambios a mano: las pruebas notan los treinta y cuatro.

## 2026-10-05 — Cuánto tapa el nivel un sonido

Una función en `sound/sound_manager.c`, de 793 bytes. De 909 funciones se pasa a
910, el 8,08 % de las del juego (el 4,907 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_query_compute` | `0x00221e30` | `sound/sound_manager.c` |

Era `unknown_00221e30`, apuntada en el hito 241. La llama
`sound_listener_compute_queries` con cada consulta del oyente. Recibe la
consulta en `edi` y, en la pila, la tabla de lo que el oyente oye de cada
cluster y el oyente. Nueva sin descompilar: `0x001de630` (si una línea
choca con el nivel, y dónde). Nueva variable: `unknown_004ed280`.

### Lo que se sabe

- Si el oyente está en la misma hoja del nivel que el sonido, no hace nada.
- Si no, traza una línea del oyente al sonido. Si choca con el nivel, traza
  otra de vuelta, del sonido al oyente. Lo que queda entre los dos choques
  (de 0 a 1 del camino) es cuánto tapa el nivel el sonido, y se suma al
  primer número de la consulta. Los dos choques se guardan en la consulta
  (los 32 bytes de `0x4`, que no se sabía qué eran) y se pone la marca de
  `0x45`. Si la de vuelta no choca, cuenta todo lo que hay después del
  primer choque.
- Las consultas con la marca de `0x47` no trazan líneas. Sin la marca de
  `0x44` y con el sonido en otro cluster, se suma cuánto del camino «oyente
  → cluster del sonido → sonido» es el rodeo (`0x00249d60` da lo que hay
  entre los dos clusters).
- Los otros dos números solo se tocan si el sonido está en otro cluster:
  son los de `sound_query_portal_results` (hito 242), con 0 en el primero
  si los dos clusters tienen el mismo número en `0x7A`; con la marca de
  `0x44` son 0 y lo que el oyente oye del cluster del sonido.
- El nombre es inventado.

### Arreglo del hito 242

- La mutación que las pruebas no notaban (hacer la cuenta con reales de 32
  bits) no se nota porque el juego tiene el procesador puesto a 32 bits de
  precisión, y las pruebas también; no porque saliera el mismo código,
  como decía. Se ha corregido esa entrada y se han añadido casos donde las
  dos cuentas darían distinto con otra precisión.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y las de las dos
  que tienen que ver con ella. La última batería completa es la del hito
  240 (907 funciones, todas bien).

### Mutaciones

Veintiocho cambios a mano: las pruebas notan veintisiete. El que no notan
es tratar un 0 justo como negativo: solo cambia un `-0` por un `0`.

## 2026-10-05 — Lo que una consulta saca del portal más cercano al oyente

Una función en `sound/sound_manager.c`, de 121 bytes. De 908 funciones se pasa a
909, el 8,07 % de las del juego (el 4,880 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_query_portal_results` | `0x00222150` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x00221e30`, la que resuelve cada consulta.
Recibe el cluster del oyente en `ecx`, dónde está el oyente en `esi`, dónde
dejar dos números en `ebx` y, en la pila, la tabla de lo que el oyente oye
de cada cluster (la del hito 241), dónde está el sonido y su cluster.
Nuevas sin descompilar: `0x0018cfd0` y `0x0014b240`.

### Lo que se sabe

- Es para un sonido que está en otro cluster que el oyente.
  `sound_query_portal_results` busca el portal del cluster del oyente que
  queda más cerca del oyente (`0x0018cfd0`; los portales están en `0x60`
  del structure bsp, de `0x24` bytes cada uno).
- El primer número es `1 - 1/(d + 1)`, con `d` lo lejos que está el sonido
  de ese portal (`0x0014b240`): 0 en el portal y cerca de 1 muy lejos. El
  segundo es lo que el oyente oye del cluster del sonido.
- Sin portal, los números son 1 y 0.
- El nombre es inventado.

### Batería completa

La del hito 240 terminó mientras se hacía este: 907 funciones, las 907
bien.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función.

### Mutaciones

Doce cambios a mano: las pruebas notan once. El que no notan es hacer la
cuenta con reales de 32 bits en vez de con los del procesador: da lo
mismo, porque el juego tiene el procesador puesto a 32 bits de precisión
(y las pruebas también).

## 2026-10-05 — Lo que un oyente oye de cada cluster

Una función en `sound/sound_manager.c`, de 804 bytes. De 907 funciones se pasa a
908, el 8,06 % de las del juego (el 4,876 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_listener_compute_queries` | `0x00221a70` | `sound/sound_manager.c` |

Era `unknown_00221a70`. La llama `sound_update`, una vez por jugador local.
Recibe en la pila el oyente y las consultas que preparó
`sound_channels_build_queries`. Nuevas sin descompilar: `0x00249d10`,
`0x00249c90`, `0x0018cac0` y `0x00221e30`.

### Lo que se sabe

- El nivel (el structure bsp) tiene, en `0x224`, unas aberturas para el
  sonido: cada una dice algo (un byte, en la tabla de `0x30`; `NONE` es
  nada) y tiene a cada lado un grupo de clusters.
- `sound_listener_compute_queries` mira las aberturas del cluster del oyente
  (`0x00249d10` dice cuáles) y, de cada una que dice algo, los clusters de
  sus dos lados (`0x00249c90`). Si el cluster del oyente está en un lado,
  los del otro se oyen con el número que `0x0018cac0` saca de lo que dice
  la abertura; si un cluster queda detrás de varias, con el mayor.
- El cluster del propio oyente no se toca nunca: se queda en 0. Hay sitio
  para 512 clusters.
- Con esa tabla se resuelve cada consulta de ese oyente (`0x00221e30`, que
  recibe la consulta en `edi`).
- Un oyente sin cluster (`NONE`) no hace nada.
- El nombre es inventado.

### Arreglo

- `unknown_00020670` (la que llama a una función con cada elemento de un
  array) quita sus cuatro argumentos de la pila al volver, y estaba
  apuntada como si no. Se ha corregido en `symbols/functions.csv` y en
  `saved games/saved_game_files.c`, que también la llama; sus pruebas
  siguen pasando.

### Sin comprobar

- Clusters del oyente negativos que no sean `NONE`, y niveles con más de
  512 clusters: el juego lee y escribe fuera de sus tablas.
- En este hito solo se pasaron las pruebas de esta función, las de
  `sound_update` y las de `saved_game_files`. La última batería completa es
  la del hito 233 (900 funciones, todas bien); la del hito 240 está en
  marcha.

### Mutaciones

Diecinueve cambios a mano: las pruebas notan los diecinueve.

## 2026-10-05 — La consola sabe quién oye y cómo suenan sus sitios

Una función en `sound/sound_manager.c`, de 725 bytes. De 906 funciones se pasa a
907, el 8,05 % de las del juego (el 4,849 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_set_listener` | `0x00220790` | `sound/sound_manager.c` |

No estaba apuntada. La llaman `0x00127400` (la de los oyentes) y
`0x0021e4b0` (la que prepara la consola). Recibe en la pila lo que hay que
decirle a la consola. Nuevas sin descompilar, de la consola: `0x0037d598` y
`0x0037d54e`. También la tabla `sound_environment_default` (`0x0044a2a0`).

### Lo que se sabe

- `sound_console_set_listener` le dice a la consola dónde está quien oye
  (solo la primera vez: después el oyente no se mueve, son los sonidos los
  que se pasan a sus ejes), hacia dónde mira (cuando ha cambiado 0,05 o más
  en algún número) y cómo suenan hasta dos sitios, con cuánto hay de cada
  uno.
- Lo que se dice se guarda en el sonido de la consola, y para cada uno de
  los dos sitios un byte avisa de que ha cambiado cómo suena y otro de que
  ha cambiado lo demás. Son los avisos que mira `sound_console_update`.
- Un sitio que no se da suena como `sound_environment_default`: silencio
  (-100 decibelios en todo), y con nada de él.
- Con esto se ve que los dos «estados» de `sound_unknown_states_set` (hito
  207) son seguramente esos dos sitios entre los que se pasa, aunque sigue
  sin comprobarse.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 233 (900 funciones, todas bien).

### Mutaciones

Catorce cambios a mano: las pruebas notan trece, uno de ellos solo después
de añadir un caso. El que no notan era un cambio mío que no cambiaba nada
(el tope de dos sitios escrito de otra forma). La prueba compara además la
tabla del silencio con la del juego.

## 2026-10-05 — El sonido de una pista pasa al estado siguiente

Una función en `sound/sound_manager.c`, de 616 bytes. De 905 funciones se pasa a
906, el 8,04 % de las del juego (el 4,825 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_source_next_state` | `0x0021c190` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021c400`. Recibe en la pila el sonido en
bucle, su estado, el sonido que suena y una semilla; devuelve si ese sonido
se ha soltado.

### Lo que se sabe

- Un sonido en bucle pasa por estados (entrada, bucle, salida y variantes),
  y cada pista tiene un sonido para cada uno.
  `looping_sound_source_next_state` hace que el sonido que suena de una
  pista pase a ser el del estado siguiente (el byte de `0xD` del estado; el
  de `0xC` lo sigue al terminar).
- Elige el rango de tono por su tono y la grabación como en cualquier
  sonido, con los bits de las ya usadas: los de la pista, si el tag tiene
  las marcas 4 o 5 y el sonido en bucle tiene mando de sonidos, o los del
  rango de tono si no.
- Si el sonido tiene canal y ya hay tantos iguales como deja su clase, o
  tantos de su sonido, se suelta uno de ellos (motivos 5 y 6): devuelve
  verdadero si el que se suelta es este mismo.
- El nombre es inventado.

### Sin comprobar

- La prueba da a `sound_pitch_range_get_unknown4` solo valores que esa
  función puede dar: con otros, el compilador de `source/` no hace la
  conversión que el juego sí hace.
- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 233 (900 funciones, todas bien).

### Mutaciones

Catorce líneas rotas a mano: las pruebas notan trece. La que no notan es
quitar la puesta a 0 de los bits de la pista: se vuelven a escribir justo
después.

## 2026-10-05 — El sonido de la consola tiene una casilla más

Una función en `sound/sound_manager.c`, de 618 bytes. De 904 funciones se pasa a
905, el 8,03 % de las del juego (el 4,804 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_slot_new` | `0x00220b90` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021e4b0`, la que prepara el sonido de la
consola. Recibe unas marcas en `ecx` y un número en la pila; devuelve la
casilla, o `NONE`.

### Lo que se sabe

- `sound_console_slot_new` añade una casilla al sonido de la consola, hasta
  64.
- La consola crea para ella dos cosas. La de `0x38` suena por las salidas 0
  a 5 y arranca enseguida; si la consola no la crea, la casilla sigue sin
  ella.
- La de `0x34` solo se crea con la marca 0: suena por las salidas 6, 8, 7,
  9 y 10, con las dos distancias al máximo, y con la marca 1 es de otro
  tipo y arranca enseguida. Si la consola no la crea, no hay casilla: la
  cuenta vuelve atrás y devuelve `NONE`.
- El número de la pila va a la consola en las dos, y la casilla se lo queda
  al principio.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 233 (900 funciones, todas bien).

### Mutaciones

Trece líneas rotas a mano: las pruebas notan las trece.

## 2026-10-05 — El sonido de la consola recibe sus ganancias

Una función en `sound/sound_manager.c`, de 526 bytes. De 903 funciones se pasa a
904, el 8,02 % de las del juego (el 4,783 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_set_gains` | `0x00220fd0` | `sound/sound_manager.c` |

No estaba apuntada. La llaman dos funciones de fuera del sonido. Recibe en
`eax` cuatro ganancias, una frecuencia y unos decibelios. También el global
`unknown_0047005c`.

### Lo que se sabe

- `sound_console_set_gains` guarda en el sonido de la consola (desde
  `0x2ABC`) cuatro ganancias, recortadas entre 0 y 1, una frecuencia y unos
  decibelios. Son los números que usa `sound_console_slot_update_room`
  (hito 196).
- Con ellos rellena `unknown_00470060`, que ahora se sabe qué es: cómo se
  oye un sonido en una sala cuando nada lo tapa (las dos primeras ganancias
  para el sonido directo y la tercera para la sala, en centésimas de
  decibelio), y deja la frecuencia y los decibelios del filtro en
  `unknown_00470084` y `unknown_00470088`.
- Cada casilla pierde la marca 1, para que la próxima vez se le diga todo
  de nuevo, y `unknown_0047005c` queda puesto.
- El nombre es inventado.

### Comprobado

- **La batería completa del hito 233 terminó: 900 funciones de 900, todas
  bien.**
- A `looping_sound_new` (hito 236) se le rompieron doce líneas a mano: las
  pruebas notan las doce.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan las diez.

## 2026-10-05 — Un sonido en bucle se crea

Una función en `sound/sound_manager.c`, de 432 bytes. De 902 funciones se pasa a
903, el 8,02 % de las del juego (el 4,765 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_new` | `0x0021aeb0` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021a250`. Recibe el tag en `eax` y tres
números en la pila; devuelve el sonido en bucle, o `NONE`.

### Lo que se sabe

- `looping_sound_new` crea un sonido en bucle a partir de su tag, con un
  mando de sonidos para quien lo pide. No lo crea si el sonido no puede
  sonar ahora o no queda sitio.
- A cada sonido suelto del tag le apunta cuándo sonará por primera vez: un
  tiempo al azar entre los dos que dice, contado desde ahora, o ahora mismo
  si tiene la marca 2.
- Con las marcas 4 o 5 del tag, cada pista se queda con un número al azar
  del mando de sonidos.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función (700 casos). La
  batería completa del hito 233 (900 funciones) quedó corriendo aparte: hay
  que mirar su resultado.

### Mutaciones

Pendientes las de este hito: no se rompió ninguna línea a mano.

## 2026-10-05 — Cómo suena un sitio, para la consola

Una función en `sound/sound_manager.c`, de 410 bytes. De 901 funciones se pasa a
902, el 8,01 % de las del juego (el 4,751 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_set_reverb` | `0x0021ece0` | `sound/sound_manager.c` |

No estaba apuntada. Recibe en `esi` cómo suena un sitio del nivel (los
reales de su tag) y en la pila a qué efecto del procesador de sonido de la
consola dárselo. También la estructura `sound_console_reverb` y el global
`unknown_00468834` (una ganancia, 1).

### Lo que se sabe

- `sound_console_set_reverb` traduce cómo suena un sitio (una cueva, un
  pasillo, el exterior) a lo que entiende el efecto de reverberación de la
  consola: doce números.
- Cuatro son volúmenes: la sala, sus agudos, los primeros ecos y la cola.
  A cada uno se le suma el de `unknown_00468834` y se pasa a centésimas de
  decibelio: los dos primeros de -6400 a 0, los ecos hasta 1000 y la cola
  hasta 2000.
- Los otros ocho pasan tal cual (tiempos, retrasos, frecuencia de
  referencia), salvo la difusión y la densidad, que van en tantos por
  ciento.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 900 funciones del hito 233.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan las diez.

## 2026-10-05 — La lista de efectos llega a la consola

Una función en `sound/sound_manager.c`. De 900 funciones se pasa a
901, el 8,00 % de las del juego (el 4,737 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_apply_effect_list` | `0x0021f960` | `sound/sound_manager.c` |

No estaba apuntada. Recibe en `eax` cuántos bytes ocupa la lista y en la
pila la lista. Nuevas sin descompilar, de la consola: `0x0037ba6f` y
`0x0037b60d`.

### Lo que se sabe

- Con esta función se aclara qué es la lista que hacen
  `sound_unknown_002225a0` y `sound_unknown_002228d0` (hitos 200 y 217):
  son los valores que deben tener los efectos del procesador de sonido de
  la consola (la reverberación y demás).
- `sound_console_apply_effect_list` recorre la lista. Cada entrada dice,
  con un bit por efecto, a cuáles se aplica (hasta 32), y trae unos bytes:
  o todo lo que el efecto tiene (marca 0), o solo una parte, desde el punto
  que dice la entrada.
- Una parte solo se manda a la consola si no coincide con lo que el juego
  sabe que el efecto ya tiene: así no se repiten órdenes cada fotograma.
- Las entradas con la marca 1 se saltan. Los efectos tocados se apuntan en
  una palabra de `unknown_00510c90`.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 900 funciones del hito 233.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan las diez.

## 2026-10-05 — Función número 900

Una función en `sound/sound_manager.c`. De 899 funciones se pasa a
900, el 7,99 % de las del juego (el 4,728 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_00222250` | `0x00222250` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x002222c0`, la que escribe las entradas de
la lista de `sound_unknown_002225a0`. Recibe una dirección en `ecx` y un
número en `esi`, y devuelve un real en `xmm0`. Nueva sin descompilar:
`0x00192b90`.

### Lo que se sabe

- `sound_unknown_00222250` deja la dirección con longitud 1 y pregunta a
  `0x00192b90` por ella y por una de cinco cosas (el número, de 0 a 4); lo
  que responde se recorta entre 0 y 1.
- Con un número que no es de ninguna de las cinco devuelve 1.
- De qué son esas cinco cosas no se sabe todavía (por dónde se usa, pueden
  ser las cinco salidas de un sonido envolvente); el nombre lleva la
  dirección.

### Dónde queda el sonido

- Con esta son **900 funciones descompiladas**. En esta tanda (hitos 193 a
  233) se han hecho 51, casi todas del sonido: el efecto Doppler, cómo se
  oye un canal, la voz de la consola de cada canal, la caché de muestras
  entera, los efectos de sonido, los sonidos sueltos de los sonidos en
  bucle y buena parte de lo que habla con la consola.
- Del sonido quedan 25 funciones, unos 23.000 bytes: las más grandes
  (`0x00127400`, los oyentes; `0x0021a250`, `0x0021b0e0` y `0x0021c400`,
  los sonidos en bucle; `0x0021e4b0`, `0x0021fa80` y `0x002201f0`, la
  consola) y otras que dependen de funciones de fuera del sonido.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa se lanza aparte con las 900 funciones de este hito.

### Mutaciones

Siete líneas rotas a mano: las pruebas notan las siete.

## 2026-10-05 — Lo que el sonido de la consola hace cada fotograma

Una función en `sound/sound_manager.c`. De 898 funciones se pasa a
899, el 7,98 % de las del juego (el 4,724 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_update` | `0x0021ec00` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021ec00`); la llaman
`sound_update` y `sound_idle`. Nuevas sin descompilar: `0x001915f0` y
`0x0037b844` (de la consola). También el global `unknown_00386ad8`.

### Lo que se sabe

- `sound_console_update` es lo que el sonido de la consola hace cada
  fotograma. Primero compara un número que guarda (`0x2AD4`) con
  `unknown_00386ad8`: si no coinciden llama a `0x001915f0` (por las cadenas
  que usa, la que carga el programa del procesador de sonido), pone dos
  bytes y se queda con el número nuevo.
- Después deja que la consola haga su trabajo (`0x0037b844`).
- Por último mira cada voz que suena: si su flujo dice que ya no suena (el
  bit 16 de lo que hace), la para, igual que `sound_console_voice_stop`
  pero sin tocar su volumen.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_update` y de `sound_idle`. La última batería completa es la del
  hito 227 (893 funciones, todas bien).

### Mutaciones

Nueve líneas rotas a mano: las pruebas notan las nueve.

## 2026-10-05 — La consola hace sonar silencio sin parar

Una función en `sound/sound_manager.c`. De 897 funciones se pasa a
898, el 7,97 % de las del juego (el 4,717 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_create_silence` | `0x00221490` | `sound/sound_manager.c` |

No estaba apuntada. Recibe en la pila dónde dejar lo que crea. Nuevas sin
descompilar, de la consola: `0x0037d7de` (la que crea), `0x0037cc4a`,
`0x0037b6a7`, `0x0037b66f` y `0x0037b6df`.

### Lo que se sabe

- `sound_console_create_silence` hace que la consola cree algo que suena
  sin parar: mil bytes de silencio (muestras de 8 bits, un canal, mil por
  segundo), por la salida 14 y a -100 decibelios, en bucle.
- Los mil bytes salen de la memoria que el juego reserva al arrancar.
- Para qué sirve no se sabe (puede ser lo que mantiene en marcha una parte
  del procesador de sonido de la consola). El nombre es inventado.
- La descripción que la consola recibe para crear un sonido ya tiene sus
  campos con nombre: tamaño, marcas, bytes, formato y salidas.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de las que usan
  las estructuras que cambian. La última batería completa es la del hito
  227 (893 funciones, todas bien).

### Mutaciones

Nueve líneas rotas a mano: las pruebas notan las nueve.

## 2026-10-05 — Por dónde se oye un sonido en la consola, y qué ha cambiado

Dos funciones en `sound/sound_manager.c`. De 895 funciones se pasa a
897, el 7,96 % de las del juego (el 4,707 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_stream_set_mixbins` | `0x002215d0` | `sound/sound_manager.c` |
| `sound_console_slot_stream_set_mixbins` | `0x002216f0` | `sound/sound_manager.c` |

Ninguna estaba apuntada. Las dos las llama `0x0021fa80`. Son gemelas: una
es para el flujo de una voz y la otra para lo que hace sonar una casilla.
Reciben en `eax` por dónde se oía el sonido, y en la pila por dónde se oye
ahora y a quién decírselo. Nuevas sin descompilar, de la consola:
`0x0037c70a`, `0x0037b81d` y `0x0037b6c3`.

### Lo que se sabe

- Un sonido se oye por varias salidas de la consola, cada una con su
  volumen: es la lista de parejas que rellena `sound_mixbins_add`.
- Estas funciones comparan la lista de antes con la de ahora. Si las
  salidas son las mismas y en el mismo orden, solo le dicen a la consola
  los volúmenes (y nada si tampoco han cambiado). Si no, se lo dicen todo
  de nuevo.
- Una salida que está dos veces y cambia obliga a decirlo todo de nuevo. Al
  comprobarlo, el juego mira el volumen por el puesto de la pareja y no por
  el número de la salida, y puede leer algo que nunca se escribió: aquí se
  hace igual, y las pruebas lo comprueban.
- Los nombres son inventados.

### Sin comprobar

- En este hito solo se pasaron las pruebas de estas funciones. La batería
  completa corre aparte con las 893 funciones del hito 227.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan las diez, dos de ellas solo
después de añadir casos con una salida repetida en el puesto de su propio
número.

## 2026-10-05 — Una casilla de la consola recibe lo que hace sonar su sonido

Una función en `sound/sound_manager.c`. De 894 funciones se pasa a
895, el 7,94 % de las del juego (el 4,688 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_slot_create_stream` | `0x00220a70` | `sound/sound_manager.c` |

No estaba apuntada. Recibe el número de la casilla en `eax` y devuelve si
pudo. Nuevas sin descompilar, todas de la consola: `0x0037d4be` (la que
crea), `0x0037c5e4`, `0x0037c620`, `0x0037c644`, `0x0037c6c1`,
`0x0037c69d` y `0x0037c600`.

### Lo que se sabe

- `sound_console_slot_create_stream` crea lo que hace sonar el sonido de una
  casilla (las casillas son los sitios desde los que se oye un sonido con
  posición). Lo pide a la consola con una descripción de `0x18` bytes y las
  marcas `0x2010`.
- Después lo configura: que se oiga por cinco salidas (las 6, 8, 7, 9 y
  10), con las dos distancias al máximo que cabe en un real y a 0 las otras
  tres cosas que la consola ajusta por distancia y velocidad: el juego hace
  esas cuentas por su lado (`sound_source_spatial_gain`,
  `sound_listener_doppler_pitch`).
- La casilla empieza con `0x1F` en su primer número y sin nada en `0x38`.
- Qué es cada función de la consola es una suposición: no están en este
  ejecutable por nombre. El nombre de esta es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 893 funciones del hito 227.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan las diez.

## 2026-10-05 — Un sonido arranca con su efecto de sonido

Una función en `sound/sound_manager.c`, de 427 bytes. De 893 funciones se pasa a
894, el 7,94 % de las del juego (el 4,678 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_source_new` | `0x0021d110` | `sound/sound_manager.c` |

No estaba apuntada. Recibe en `eax` lo que el juego da para arrancar un
sonido y en la pila el sonido; devuelve el sonido que suena, o `NONE`.

### Lo que se sabe

- `sound_effect_source_new` arranca un sonido. Si los tags tienen un efecto
  de sonido para él (`sound_effect_definition_get`) y queda sitio para otro
  efecto, crea el efecto, convierte el sonido en sonido del efecto
  (`sound_effect_prepare_request`) y lo arranca; el efecto se queda con el
  sonido y pasa a `0x0021d630`. Si el sonido no llega a arrancar, el efecto
  se suelta.
- Si no hay efecto, el sonido arranca como cualquier otro.
- Arrancar es lo mismo que hace `sound_source_new` (comprobar que se puede,
  mirar su promoción y crearlo), que aquí el juego tiene escrito en línea
  dos veces.
- Si lo que da `unknown_0018d090` para el sonido tiene la marca 0, lo que
  el juego dice del sonido recibe la marca 8.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 893 funciones del hito 227.

### Mutaciones

Doce líneas rotas a mano: las pruebas notan once. La que no notan es el
valor con el que empieza el número de la regla de la promoción: quien lo
rellena lo escribe siempre.

## 2026-10-05 — Los sonidos sueltos de un sonido en bucle

Tres funciones en `sound/sound_manager.c`. De 890 funciones se pasa a
893, el 7,93 % de las del juego (el 4,664 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_detail_request` | `0x0021cc60` | `sound/sound_manager.c` |
| `looping_sound_source_unknown0` | `0x0021bfb0` | `sound/sound_manager.c` |
| `looping_sound_source_matches` | `0x0021c140` | `sound/sound_manager.c` |

Ninguna estaba apuntada. Las dos últimas son las de la tabla
`looping_sound_callbacks` (`0x0044a130`), que también es nueva; la última no
figura en la lista de Ghidra.

### Lo que se sabe

- `looping_sound_detail_request` prepara lo que el juego da para arrancar
  un sonido suelto de un sonido en bucle (los que suenan de vez en cuando).
  Lo que dice del sonido es lo que dice del sonido en bucle.
- Si el sonido en bucle tiene sitio, el suelto suena en un sitio al azar a
  partir del suyo (`looping_sound_detail_random_position`), y el sonido en
  bucle recibe sus avisos (`looping_sound_callbacks`).
- Si no lo tiene, suena en un sitio al azar alrededor del oyente de uno de
  los jugadores de la consola, elegido al azar; sin jugadores no hay
  sonido. El sitio en el nivel que se apunta es el del oyente, no el del
  sonido: así lo hace el juego.
- `looping_sound_source_unknown0` es el primer aviso: lo que el juego dice
  del sonido suelto sigue a lo que dice del sonido en bucle (dirección,
  velocidad, sitio en el nivel, distancias y cono), y su sitio se cuenta
  desde el del sonido en bucle.
- `looping_sound_source_matches` dice si dos sonidos sueltos son el mismo:
  si sus tres reales coinciden con menos de una diezmilésima.

### Sin comprobar

- `looping_sound_detail_request` le pasa a `looping_sound_source_unknown0`
  el sonido, que esa función no usa: el compilador de `source/` no lo pasa,
  y la prueba no lo compara.
- En este hito solo se pasaron las pruebas de estas funciones. La última
  batería completa es la del hito 219 (881 funciones, todas bien).

### Mutaciones

Diecinueve líneas rotas a mano: las pruebas notan las diecinueve. La prueba
compara además la tabla con la del juego.

## 2026-10-05 — Guardar un vector en 32 bits

Una función en `math/real_math.c`. De 889 funciones se pasa a
890, el 7,90 % de las del juego (el 4,634 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `vector3d_compress` | `0x00188dd0` | `math/real_math.c` |

No estaba apuntada. La llaman 19 funciones del juego; recibe el vector en
`eax`.

### Lo que se sabe

- `vector3d_compress` es la pareja de `vector3d_decompress` (hito 204):
  guarda un vector en 32 bits, 11 para `i`, 11 para `j` y 10 para `k`, cada
  parte redondeada al número más cercano que cabe en sus bits.
- Lo que se sale de -1 a 1 se toma como el extremo que pasa.
- El juego la usa para guardar direcciones en poco sitio: la del cono de un
  sonido, por ejemplo.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `vector3d_decompress`. La última batería completa es la del hito 219
  (881 funciones, todas bien).

### Mutaciones

Siete líneas rotas a mano: las pruebas notan las siete.

## 2026-10-05 — Un sitio al azar para un sonido suelto de un sonido en bucle

Una función en `sound/sound_manager.c`. De 888 funciones se pasa a
889, el 7,89 % de las del juego (el 4,627 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_detail_random_position` | `0x0021ce60` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021cc60`, de los sonidos en bucle. Recibe
en `ecx` la parte del sonido en bucle que describe un sonido suelto, y en
`esi` dónde dejar el sitio.

### Lo que se sabe

- Un sonido en bucle puede tener sonidos sueltos que suenan de vez en cuando
  alrededor (un pájaro, un goteo). `looping_sound_detail_random_position`
  elige dónde suena uno: a una distancia, una elevación y un giro al azar
  entre los dos valores que dice para cada cosa (en `0x2C`, `0x24` y
  `0x1C`).
- Usa la segunda semilla del juego, una vez para cada número. Si la
  distancia sale 0, el sitio es el origen y los otros dos números no se
  sacan.
- El nombre es inventado.

### Herramientas

- La biblioteca de C de las pruebas tiene ahora `cosl`.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 219 (881 funciones, todas bien).

### Mutaciones

Ocho líneas rotas a mano: las pruebas notan las ocho.

## 2026-10-05 — Los sonidos de un anillo se apagan en unos segundos

Una función en `sound/sound_manager.c`. De 887 funciones se pasa a
888, el 7,88 % de las del juego (el 4,618 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_ring_fade_out_in_seconds` | `0x0021bd00` | `sound/sound_manager.c` |

No estaba apuntada. La llama una función de los sonidos en bucle que falta
por hacer. Recibe los segundos y el primer sonido del anillo, en la pila.

### Lo que se sabe

- Los sonidos de una pista de un sonido en bucle están enlazados en anillo.
  `sound_source_ring_fade_out_in_seconds` los recorre todos y hace que
  empiecen a apagarse en los segundos que recibe, cada uno desde el volumen
  al que se oye ahora.
- La manera de apagarse la dice la clase del sonido: con la marca 10 de la
  clase, la manera 0; sin ella, la 1.
- Es hermana de `sound_source_ring_fade_out`, que hace lo mismo pero con la
  manera dada y solo para los sonidos de un tipo.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 219 (881 funciones, todas bien).

### Mutaciones

Ocho líneas rotas a mano: las pruebas notan las ocho.

## 2026-10-05 — La caché del sonido se cierra

Una función en `sound/sound_manager.c`. De 886 funciones se pasa a
887, el 7,87 % de las del juego (el 4,608 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_close` | `0x002186f0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_002186f0`); la llaman
`main_game_load_fail` y `main_game_close_caches`, al soltar un mapa, antes
de cerrar las cachés de texturas y de geometría.

### Lo que se sabe

- `sound_cache_close` vacía la caché de muestras del sonido: recorre todos
  sus trozos y los echa de la caché (`lruv_block_delete`).
- Al trozo que no se puede echar (retenido, en uso o aún leyéndose) lo
  espera, contando mientras las tareas que quedan por hacer, pero no más de
  cinco segundos en total desde que empezó.
- Al final deja la tabla de trozos como no válida.
- El nombre es una suposición, por `texture_cache_close`, que hace lo mismo
  con las texturas.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de las dos que
  la llaman. La última batería completa es la del hito 219 (881 funciones,
  todas bien).

### Mutaciones

Ocho líneas rotas a mano: las pruebas notan las ocho.

## 2026-10-05 — La caché del sonido se crea

Tres funciones en `sound/sound_manager.c`. De 883 funciones se pasa a
886, el 7,87 % de las del juego (el 4,599 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_initialize` | `0x002185d0` | `sound/sound_manager.c` |
| `sound_cache_block_locked` | `0x002189a0` | `sound/sound_manager.c` |
| `sound_cache_block_deleted` | `0x002189e0` | `sound/sound_manager.c` |

La primera estaba apuntada sin descompilar (`unknown_002185d0`); la llama
`sound_initialize`. Las otras dos no estaban apuntadas ni figuran en la
lista de Ghidra: solo se llega a ellas porque la primera se las da a la
caché.

### Lo que se sabe

- `sound_cache_initialize` crea la caché de muestras del sonido: la tabla
  de sus trozos (512) y la caché que dice dónde está cada uno, de 192
  páginas de 16.384 bytes. Son tres megas, que se toman del final de la
  memoria de la consola.
- `sound_cache_block_locked` le dice a la caché si un trozo no se puede
  echar: el que está retenido, el que está en uso o el que aún no se ha
  terminado de leer.
- `sound_cache_block_deleted` es lo que se hace cuando la caché echa un
  trozo: el trozo de la grabación deja de tener sitio en la caché y su
  entrada en la tabla se suelta.
- Con esto la caché del sonido está completa: crear, pedir, retener,
  soltar, echar y deshacer.

### Comprobado

- **La batería completa del hito 219 terminó: 881 funciones de 881, todas
  bien.**

### Sin comprobar

- En este hito solo se pasaron las pruebas de estas funciones y de
  `sound_initialize`. Las dos últimas cuentan en las funciones pero no en
  los bytes, como las del hito 219.

### Mutaciones

Once líneas rotas a mano: las pruebas notan las once.

## 2026-10-05 — Pedir un trozo de las muestras de un sonido

Una función en `sound/sound_manager.c`. De 882 funciones se pasa a
883, el 7,84 % de las del juego (el 4,592 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_chunk_request` | `0x00218850` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00218850`). La llaman seis
funciones del sonido, todas ya descompiladas: es la puerta de la caché de
muestras.

### Lo que se sabe

- `sound_cache_chunk_request` pide un trozo de las muestras de un sonido.
  Lo que se le pide son tres marcas: la 1, que el trozo se meta en la caché
  si no está y se empiecen a leer sus muestras (`sound_cache_chunk_load`);
  la 0, esperar hasta que estén leídas (cediendo el turno al hilo que las
  lee); la 2, retener el trozo: sube su cuenta.
- Un trozo que está en la caché se marca como usado ahora, para que no sea
  el primero en salir de ella.
- Lo que devuelve son tres bits: el 1, que las muestras están; el 0, que se
  están leyendo; el 2, que el trozo quedó retenido.
- Con esta función, pedir, retener y soltar muestras queda completo de
  principio a fin. El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de cinco de
  las que la llaman. La batería completa corre aparte con las 881
  funciones del hito 219.

### Mutaciones

Once líneas rotas a mano: las pruebas notan diez. La que no notan es
quitar el último «y» que borra el bit 0 del resultado: en ese punto nunca
está puesto.

## 2026-10-05 — Los mandos de sonidos se preparan

Una función en `sound/sound_manager.c`. De 881 funciones se pasa a
882, el 7,83 % de las del juego (el 4,583 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_playback_controllers_initialize` | `0x00219b40` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00219b40`); la llama
`sound_initialize` al arrancar el sonido.

### Lo que se sabe

- `sound_playback_controllers_initialize` prepara los mandos de sonidos: la
  tabla, con sitio para 64, y uno suelto que no está en la tabla
  (`unknown_0051ebdc`). Los dos salen de la memoria que el juego reserva al
  arrancar.
- El mando suelto queda como nuevo y sin dueño, con un número hecho con la
  hora, un número al azar y el reloj de la consola (es la semilla de sus
  números al azar), y con la marca que sigue al byte de `0x7F` de los
  globales del sonido.
- Devuelve si hay memoria para las dos cosas.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_initialize`. La batería completa corre aparte con las 881
  funciones del hito 219.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan nueve. La que no notan es
devolver siempre que sí al final: en las pruebas siempre hay memoria para
el mando suelto.

## 2026-10-05 — Lo que se le avisa a un efecto de sonido de sus sonidos

Cinco funciones en `sound/sound_manager.c`. De 876 funciones se pasa a
881, el 7,82 % de las del juego (el 4,575 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_source_unknown0` | `0x0021d9f0` | `sound/sound_manager.c` |
| `sound_effect_source_got_channel` | `0x0021da30` | `sound/sound_manager.c` |
| `sound_effect_source_playing` | `0x0021da50` | `sound/sound_manager.c` |
| `sound_effect_source_heard` | `0x0021da70` | `sound/sound_manager.c` |
| `sound_effect_source_stopped` | `0x0021db30` | `sound/sound_manager.c` |

Son las cinco funciones de `sound_effect_callbacks` que faltaban: con ellas
la tabla está completa.

### Lo que se sabe

- `sound_effect_source_unknown0` dice si el efecto no tiene la marca 4; si
  no la tiene, lo que el juego dice del sonido pasa a ser lo del efecto
  (`sound_effect_get_source`).
- `sound_effect_source_got_channel`, `sound_effect_source_playing` y
  `sound_effect_source_heard` reciben los avisos de que el sonido tiene
  canal, por dónde va sonando y cómo se oye, y se los pasan tal cual a
  quien los iba a recibir antes que el efecto, si lo había.
- `sound_effect_source_stopped` hace lo mismo que cuando el sonido se
  suelta (`sound_effect_source_deleted`, con el motivo 1) y desengancha el
  sonido del efecto: a partir de ahí los avisos van a quien los recibía
  antes.

### Aviso sobre las cuentas

- Estas cinco funciones no están en la lista de funciones de Ghidra (solo
  se llega a ellas por la tabla), así que cuentan en las funciones
  descompiladas pero no en las 11.265 del total ni en los bytes. El
  porcentaje queda unas centésimas por encima de lo real hasta que se
  rehaga la lista.

### Sin comprobar

- En este hito solo se pasaron las pruebas de estas funciones y de
  `sound_effect_prepare_request`. La última batería completa es la del hito
  210 (867 funciones, todas bien).

### Mutaciones

Nueve líneas rotas a mano: las pruebas notan las nueve.

## 2026-10-05 — Un sonido pasa a ser de un efecto de sonido

Una función en `sound/sound_manager.c`. De 875 funciones se pasa a
876, el 7,78 % de las del juego (el 4,575 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_prepare_request` | `0x0021dc60` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021d110`. Recibe el efecto en `ecx` y lo
que el juego da para arrancar un sonido en `esi`. También la tabla
`sound_effect_callbacks` (`0x0044a1c0`), con cinco funciones nuevas sin
descompilar: `0x0021d9f0`, `0x0021da30`, `0x0021da50`, `0x0021da70` y
`0x0021db30`.

### Lo que se sabe

- `sound_effect_prepare_request` hace que un sonido que va a arrancar sea de
  un efecto de sonido: quien recibe los avisos del sonido pasa a ser el
  efecto (`sound_effect_callbacks`), y al final de los bytes de los avisos
  quedan el índice del efecto y quién los iba a recibir antes. Si no había
  nadie, el efecto se queda con la marca 3.
- El sonido recibe además un mando de sonidos: el que ya iba a tener o uno
  nuevo del efecto (con `0x4000` en el índice), que el efecto se guarda.
- De las seis funciones de la tabla solo estaba descompilada la quinta
  (`sound_effect_source_deleted`). Las otras cinco no figuran en la lista
  de funciones de Ghidra: nadie las llama por su dirección, solo están en
  esta tabla. Hay, por tanto, más funciones en el juego que las 11.265 de
  la lista.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 210 (867 funciones, todas bien).

### Mutaciones

Diez cambios a mano, dos de ellos en la tabla: las pruebas notan nueve y
el compilador el otro (una función de otro tipo en la tabla). La prueba
compara la tabla con la del juego, función por función.

## 2026-10-05 — Otra lista para la consola, y un real en 24 bits

Dos funciones en `sound/sound_manager.c`. De 873 funciones se pasa a
875, el 7,77 % de las del juego (el 4,571 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_002228d0` | `0x002228d0` | `sound/sound_manager.c` |
| `sound_real_to_fixed24` | `0x002221d0` | `sound/sound_manager.c` |

Ninguna de las dos estaba apuntada. La primera recibe un número en `edx`,
dónde dejar el resultado en `esi` y otro número en la pila; la segunda, un
real en la pila.

### Lo que se sabe

- `sound_unknown_002228d0` hace la misma lista que
  `sound_unknown_002225a0`, pero para lo que `unknown_0018d090` da sin
  sonido (con `NONE`) y un número: si su primer elemento dice un tag y trae
  un bloque propio con algo, la lista va al principio del resultado y su
  tamaño en `0x400`. Si no, no hace nada. De qué es la lista sigue sin
  saberse.
- `sound_real_to_fixed24` pasa un real entre -1 y 1 a un número de 24 bits
  con signo, como los que usa el procesador de sonido de la consola: 1 es
  `0x7FFFFF` y -1 es `0x800000`. Lo que se sale de -1 a 1 se recorta. La
  llama `0x002222c0`, la que escribe las entradas de la lista.
- El nombre de la segunda es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de estas funciones. La última
  batería completa es la del hito 210 (867 funciones, todas bien).

### Mutaciones

Once líneas rotas a mano: las pruebas notan las once (entre ellas, tratar
el 0 negativo y lo que no es un número de otra manera).

## 2026-10-05 — El sonido de la consola se suelta

Una función en `sound/sound_manager.c`. De 872 funciones se pasa a
873, el 7,75 % de las del juego (el 4,564 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_dispose` | `0x0021eae0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021eae0`); la llama
`sound_dispose`. Nuevas sin descompilar: `0x00379f45` y `0x00379f2a`, de la
consola (sueltan algo de su sonido), y `0x002ae170`.

### Lo que se sabe

- `sound_console_dispose` deshace el sonido de la consola cuando se deshace
  el del juego: para todas las voces (`sound_console_voices_stop`), suelta
  el flujo de cada una (la función de `4` de su tabla) y deja la cuenta de
  voces a 0.
- De cada casilla suelta las dos cosas de la consola que tiene (en `0x34` y
  `0x38`), y del sonido de la consola, la de `0x2AB0`.
- Al final llama a `0x002ae170` con `unknown_0047f0d0` (lo que acompaña a
  las muestras sin comprimir), pone a 0 el primer byte del sonido de la
  consola y deja sin memoria a las bibliotecas de la consola
  (`xmem_static_buffer`, que ya estaba en `shell.c`).
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_dispose` y de las que usan las estructuras que cambian. La última
  batería completa es la del hito 210 (867 funciones, todas bien).

### Mutaciones

Once líneas rotas a mano: las pruebas notan las once.

## 2026-10-05 — Todas las voces de la consola se paran

Una función en `sound/sound_manager.c`. De 871 funciones se pasa a
872, el 7,74 % de las del juego (el 4,556 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voices_stop` | `0x0021f290` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021f290`); la llama
`sound_dispose_from_old_map` al soltar un mapa.

### Lo que se sabe

- `sound_console_voices_stop` para todas las voces del sonido de la consola:
  a cada una le hace lo mismo que `sound_console_voice_stop` (parar su
  flujo, avisar a su otra cosa, dejarla sin hacer nada y a -64 decibelios),
  y lo repite si después de eso la voz sigue sonando.
- Además deja a 0 los dos números de tres bits que la voz tiene en `2`.
- El nombre es una suposición.

### Comprobado

- **La batería completa del hito 210 terminó: 867 funciones de 867, todas
  bien.**

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_stop_all`, de `sound_dispose_from_old_map` y de
  `sound_console_voice_stop`.

### Mutaciones

Siete líneas rotas a mano: las pruebas notan las siete.

## 2026-10-05 — El volumen de un rango de tono fuera de sus tonos

Una función en `sound/sound_manager.c`. De 870 funciones se pasa a
871, el 7,73 % de las del juego (el 4,550 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_pitch_range_fade_decibels` | `0x00219f80` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021b0e0`, la de los sonidos en bucle que
falta por hacer. Recibe unas marcas en `ecx`, el rango de tono en `edx` y el
tono en la pila.

### Lo que se sabe

- `sound_pitch_range_fade_decibels` da cuánto baja el volumen de un rango de
  tono cuando el tono al que suena queda fuera de los suyos, en decibelios.
- Los parámetros de un rango de tono son cinco números: el segundo y el
  cuarto marcan por abajo dónde empieza a oírse y dónde se oye entero; el
  quinto y el tercero, por arriba, dónde deja de oírse entero y dónde se
  apaga del todo. Entre el cuarto y el quinto el resultado es 0.
- Así un motor puede pasar de un rango de tono al siguiente sin saltos: uno
  se apaga mientras el otro sube. Solo se hace con la marca 13.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 867 funciones del hito 210.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan nueve, una de ellas solo
después de añadir casos con dos límites iguales. La que no notan es
cambiar un «mayor» por «mayor o igual»: el caso de igualdad ya lo descarta
la comprobación anterior.

## 2026-10-05 — Una voz de la consola recibe su flujo

Una función en `sound/sound_manager.c`. De 869 funciones se pasa a
870, el 7,72 % de las del juego (el 4,543 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voice_create_stream` | `0x0021f4e0` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021e4b0`, la que prepara el sonido de la
consola. Recibe el tipo de muestras en `ebx` y el número de la voz en la
pila. Nueva sin descompilar: `0x002ae660`, que es la que crea el flujo.
También la estructura `sound_console_stream_description`.

### Lo que se sabe

- `sound_console_voice_create_stream` le da a una voz su flujo de sonido,
  para muestras de un tipo: pide el formato a `sound_console_get_format` y
  rellena lo que la consola necesita para crear el flujo (el formato, hasta
  dos trozos de muestras a la vez, y `sound_console_voice_stream_callback`
  como aviso, con el número de la voz).
- Con esto queda completa la cadena del formato de una voz: tipo de
  muestras, formato, flujo y aviso.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 867 funciones del hito 210.

### Mutaciones

Seis líneas rotas a mano: las pruebas notan las seis.

## 2026-10-05 — Lo que la consola llama para el flujo de una voz

Una función en `sound/sound_manager.c`. De 868 funciones se pasa a
869, el 7,71 % de las del juego (el 4,538 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voice_stream_callback` | `0x00220730` | `sound/sound_manager.c` |

No estaba apuntada. No la llama nadie del juego: `0x0021f4e0` se la da a la
consola, junto con el número de la voz, al preparar el flujo de la voz.
Nueva sin descompilar: `0x002ae820`.

### Lo que se sabe

- `sound_console_voice_stream_callback` es el aviso que da la consola sobre
  el flujo de una voz. Recibe el número de la voz y dos números.
- Si hay algo que avisar y la voz existe, se lo pasa a la cosa que la voz
  tiene en `0x30` (la función de `0x18` de su tabla).
- Si el último número es 0 y la voz no tiene las marcas 2 ni 3, llama a
  `0x002ae820` con la voz (seguramente para darle más muestras).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_console_voice_stop`. La batería completa corre aparte con las 867
  funciones del hito 210.

### Mutaciones

Siete líneas rotas a mano: las pruebas notan las siete.

## 2026-10-05 — Cómo son las muestras de un sonido para la consola

Una función en `sound/sound_manager.c`. De 867 funciones se pasa a
868, el 7,71 % de las del juego (el 4,535 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_get_format` | `0x0021e410` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0021f4e0`, que le da a una voz de la consola
su formato. Recibe el tipo de muestras en `eax`, dónde dejar el formato en
`esi` y dónde dejar un puntero en `edi`. También la estructura
`sound_console_format` y los globales `unknown_0047f0d0` y
`unknown_0047f0f0`.

### Lo que se sabe

- `sound_console_get_format` dice cómo son las muestras de un sonido según
  su tipo, tal como lo quiere la consola, siempre a 44.100 muestras por
  segundo.
- El tipo 2 son muestras sin comprimir: 16 bits y dos canales.
- Los demás son muestras comprimidas (el formato `0x69` de la consola): 4
  bits por muestra, 64 muestras en cada bloque de 36 bytes por canal. El
  tipo 1 tiene dos canales y el 0 uno.
- Con el formato da también un objeto del juego, distinto para las
  comprimidas y las que no: cada uno tiene su tabla de funciones (lo que
  descomprime, seguramente).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 867 funciones del hito 210.

### Mutaciones

Nueve líneas rotas a mano: las pruebas notan las nueve.

## 2026-10-05 — Subir y bajar las clases de sonidos por su nombre

Una función en `sound/sound_manager.c`. De 866 funciones se pasa a
867, el 7,70 % de las del juego (el 4,530 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_classes_set_decibels` | `0x00221980` | `sound/sound_manager.c` |

No estaba apuntada. La llaman cuatro funciones de fuera del sonido. También
la tabla `sound_class_names` (`0x00470090`): los nombres de las 54 clases de
sonidos.

### Lo que se sabe

- `sound_classes_set_decibels` recibe un texto, unos decibelios y unos
  segundos. Todas las clases de sonidos cuyo nombre contiene el texto pasan
  a ese volumen (entre -64 y 0 decibelios) en esos segundos; con 0 segundos,
  de golpe. Quien las lleva hasta ahí es `sound_classes_update`.
- Es lo que usan los guiones de los niveles para subir y bajar grupos de
  sonidos: con «weapon» cambian todas las clases de armas, con «dialog» las
  de diálogos, y así.
- De las 54 clases, 34 tienen nombre (`projectile_impact`, `weapon_fire`,
  `unit_dialog`, `music`, `ambient_nature`, `cortana_mission`, `ui`...); las
  que no lo tienen no se tocan nunca.
- El nombre de la función es una suposición.

### Herramientas

- La biblioteca de C de las pruebas (`tools/difftest_libc`) tiene ahora
  `strstr`.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_classes_update`. La última batería completa es la del hito 200
  (857 funciones, todas bien).

### Mutaciones

Seis cambios a mano (un nombre mal escrito, un nombre de más, las clases
sin nombre, los segundos menores que 0 y el volumen que se pone de golpe):
las pruebas notan los seis. La prueba pasa cada nombre entero, así que un
nombre distinto del que tiene el juego se nota.

## 2026-10-05 — Una voz del sonido de la consola se para

Una función en `sound/sound_manager.c`. De 865 funciones se pasa a
866, el 7,69 % de las del juego (el 4,525 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_voice_stop` | `0x0021f5d0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021f5d0`); la llama
`sound_channel_resume`. Nuevas sin descompilar: `0x002ae8e0` y
`0x002ae6f0`, de una voz de la consola.

### Lo que se sabe

- `sound_console_voice_stop` para una voz del sonido de la consola y la deja
  como nueva: a su flujo le pregunta qué hace y le dice que pare (dos
  funciones de su tabla, en `0xC` y `0x18`), avisa a la otra cosa que tiene
  la voz (en `0x30`, función de `0x1C`), deja su estado en 0 y su volumen
  en -64 decibelios.
- El número de `0x2C` de la voz pasa a ser el que recibe la función, o 0 si
  es menor.
- Si la voz tiene mando de sonidos, se lo vuelve a decir a la consola
  (`0x0037b822`).
- Mientras lo hace, la voz lleva la marca 2.
- El primer argumento es un número entero, no una palabra: el juego usa los
  32 bits.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_channel_resume`. La última batería completa es la del hito 200
  (857 funciones, todas bien).

### Mutaciones

Nueve líneas rotas a mano: las pruebas notan las nueve, dos de ellas solo
después de mirar la marca de la voz mientras se para y de probar con todos
los estados.

## 2026-10-05 — El sonido de la consola cambia de quién lleva el sonido de los sitios

Una función en `sound/sound_manager.c`. De 864 funciones se pasa a
865, el 7,68 % de las del juego (el 4,521 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_set_environment_handler` | `0x0021e3b0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021e3b0`); la llama
`sound_console_set_environment`. También las estructuras
`sound_console_environment_handler` y la de sus tres funciones, y el global
`unknown_00470060` (`0x24` bytes).

### Lo que se sabe

- En `0x2AB4` del sonido de la consola hay un objeto con tres funciones, que
  lo reciben en `ecx` y quitan sus argumentos de la pila.
- `sound_console_set_environment_handler` lo cambia por otro: al que había,
  si había, le avisa de que se suelta (la segunda función); al nuevo le da
  lo que guarda `sound_console_set_environment` (la primera) y después
  `unknown_00470060` (la tercera), las dos veces con el byte de `1` del
  sonido de la consola.
- Qué hacen esas funciones no se sabe: no están en este ejecutable por
  nombre. El nombre de la función es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_console_set_environment`. La última batería completa es la del
  hito 200 (857 funciones, todas bien).

### Mutaciones

Seis líneas rotas a mano: las pruebas notan las seis.

## 2026-10-05 — Los dos estados que el sonido guarda en 0x1A8

Una función en `sound/sound_manager.c`, de 625 bytes. De 863 funciones se pasa a
864, el 7,67 % de las del juego (el 4,518 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_states_set` | `0x00126660` | `sound/sound_manager.c` |

No estaba apuntada. Solo la llama `0x0018bb80`, que no es del sonido. Recibe
en `eax` dos cosas de `0x1C` bytes. También la estructura
`sound_unknown_state`.

### Lo que se sabe

- Los globales del sonido guardan en `0x1A8` dos estados iguales: de qué es
  cada uno (`NONE` si de nada), cuánto hay de él (de 0 a 1), en cuántos
  segundos entra y sale, y cuatro reales.
- `sound_unknown_states_set` les dice lo que tienen que ser. Cada cosa que
  recibe se queda con el estado que ya es de lo mismo o, si no hay, con el
  último que esté libre (uno que no se ha usado esta vez y del que no queda
  más de una milésima).
- El estado se acerca a lo que recibe según lo que los segundos de esta vez
  son de sus segundos: los cuatro reales (los dos primeros como ángulos, pi
  veces más rápido; o de golpe, si no quedaba nada de él) y cuánto hay de
  él, hacia 1.
- De un estado que nadie pide queda cada vez menos, hacia 0.
- De qué son esos estados no se sabe todavía (por cómo entran y salen,
  pueden ser los dos entornos de sonido entre los que se pasa); el nombre
  es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 200 (857 funciones, todas bien).

### Mutaciones

Quince líneas rotas a mano: las pruebas notan las quince.

## 2026-10-05 — El volumen de una clase de sonidos

Una función en `sound/sound_manager.c`, de 678 bytes. De 862 funciones se pasa a
863, el 7,66 % de las del juego (el 4,497 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_class_get_decibels` | `0x00127010` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00127010`); la llama
`sound_definition_decibels`.

### Lo que se sabe

- `sound_class_get_decibels` da el volumen de una clase de sonidos, en
  decibelios: el suyo de ahora (el que `sound_classes_update` va llevando a
  su sitio) más lo que añade el estado del sonido del juego, salvo que el
  cuarto número de la clase tenga la marca 0.
- Los globales del sonido guardan el estado en que está el juego (`0x1F8`),
  con los segundos que lleva en él (`0x1FC`), y el estado anterior
  (`0x200`), con los suyos (`0x204`). Es lo que cambia
  `sound_set_unknown1f8`.
- Cada clase tiene, para cada estado, cuatro reales en `0x28`: unos
  decibelios, los segundos en los que entran, los que esperan antes de
  irse y los segundos en los que se van.
- Los decibelios del estado de ahora entran desde 0, y los del anterior se
  van hacia 0. Si se vuelve al estado anterior antes de que se haya ido,
  entran desde donde estaban.
- Parece el mecanismo que baja unas clases de sonido mientras suena otra
  cosa (un diálogo, por ejemplo), pero eso no está comprobado.
- El nombre es una suposición.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_definition_decibels`. La última batería completa es la del hito
  200 (857 funciones, todas bien).

### Mutaciones

Dieciocho líneas rotas a mano (los límites de una milésima, los topes de
las fracciones, las esperas y los signos): las pruebas notan las dieciocho.

## 2026-10-05 — Cómo se oye el sonido de un canal

Una función en `sound/sound_manager.c`, de 1152 bytes. De 861 funciones se pasa a
862, el 7,65 % de las del juego (el 4,474 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_compute_parameters` | `0x00129aa0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00129aa0`); la llama
`sound_channels_update_sources` para cada canal con voz, y lo que calcula
es lo que reciben `sound_channel_update_voice` y `0x0021c400`. En
`sound_source_callbacks` ya se sabe qué es la función de `0xC`: quien
recibe los avisos de un sonido puede cambiar cómo se oye.

### Lo que se sabe

- `sound_channel_compute_parameters` calcula cómo se oye el sonido de un
  canal. El canal tiene dos marcas de cómo va su sonido a la consola: tal
  cual (la 0) o por una casilla con sitio (la 1).
- Un sonido sin sitio (tipo 0) va tal cual: todo, o nada si tiene la marca
  8.
- Un sonido con sitio (tipos 1 y 2) recibe lo que dice
  `sound_source_spatial_gain` de dónde está y hacia dónde apunta, vistos
  desde su oyente: la posición se pasa a los ejes del oyente, y la dirección
  del cono sale de 32 bits (`vector3d_decompress`) y se gira igual. Con voz
  de sonido va por su casilla con esa ganancia; si no, la pierde de sus
  decibelios y va tal cual.
- Cuando las marcas del canal cambian, pasa de una forma a la otra en un
  segundo (`sound_unknown_0012a6d0`), desde el tiempo de `0x14` del canal.
  Al calcular cuánto queda, el juego resta el tiempo donde parece que
  debería sumarlo; aquí se hace igual, y las pruebas lo comprueban.
- El resultado (`0x38` bytes) lleva las marcas, tres volúmenes (dos en
  decibelios, en `4` y `8`, y uno como ganancia, en `0x14`), la ganancia de
  `sound_source_spatial_gain` (`0x18`) y dónde está el sonido (`0x1C`).
- La velocidad del sonido también se pasa a los ejes del oyente, pero no se
  usa después.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_channels_update_sources`. La última batería completa es la del
  hito 200 (857 funciones, todas bien).

### Mutaciones

Treinta líneas rotas a mano: las pruebas notan veintinueve, dos de ellas
solo después de añadir casos (un tiempo menor que un segundo y un comienzo
muy negativo). La que no notan es tratar las marcas del canal como un byte
con signo: nunca pasan de 3.

## 2026-10-05 — Un vector guardado en 32 bits

Una función en `math/real_math.c`. De 860 funciones se pasa a
861, el 7,64 % de las del juego (el 4,435 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `vector3d_decompress` | `0x00188ea0` | `math/real_math.c` |

No estaba apuntada. La llama `0x00129aa0`, la del sonido que falta por
hacer, para saber hacia dónde apunta un sonido. Recibe el número en `ecx` y
dónde dejar el vector en `esi`, y devuelve ese mismo sitio.

### Lo que se sabe

- `vector3d_decompress` saca un vector de un número de 32 bits: 11 bits
  para `i`, 11 para `j` y 10 para `k`. Cada parte va de -1 (el 0) a 1 (lo
  más que cabe en sus bits), y al final el vector se deja de longitud 1.
- Lo que el juego dice de un sonido guarda así, en `0x18` (el `0x30` del
  sonido que suena), hacia dónde apunta su cono.
- El nombre es inventado, y el archivo también: la función está lejos del
  resto de `real_math.c` (en `0x00188ea0`), pero no se sabe de qué archivo
  es.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `normalize3d`. La última batería completa es la del hito 200 (857
  funciones, todas bien).

### Mutaciones

Siete líneas rotas a mano: las pruebas notan seis. La que no notan es
cambiar el «mayor o igual» del tope por «mayor»: con el valor más alto la
cuenta da 1 de todas formas.

## 2026-10-05 — El sonido de un canal y su voz de la consola

Una función en `sound/sound_manager.c`, de 1057 bytes. De 859 funciones se pasa a
860, el 7,63 % de las del juego (el 4,428 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_update_voice` | `0x00128ca0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00128ca0`); la llama
`sound_channels_update_sources` para cada canal con voz cuyo sonido no es de
los tipos 1 a 4. Nueva sin descompilar: `0x0021f720`, del sonido de la
consola.

### Cambiado

- Tres funciones reciben argumentos en registros que no estaban apuntados:
  `sound_console_voice_start` recibe además en `ebx` lo que hay en `0x48`
  de lo que rellena `sound_channel_build_voice_parameters`, y se lo pasa a
  `0x002201f0`, que lo toma ahí; y `0x0021fa80` recibe en `edi` eso mismo
  entero. Las pruebas de antes no lo notaban porque solo comparaban los
  argumentos que estaban apuntados.

### Lo que se sabe

- `sound_channel_update_voice` es lo que se hace cada vez con el sonido de
  un canal que tiene voz de la consola.
- Calcula el tono (el de la escala del sonido y el suyo propio, menos el de
  las grabaciones tal como se hicieron; y, para un sonido con voz de sonido,
  el efecto Doppler: el que guarda su mando de sonidos o el que dice
  `sound_listener_doppler_pitch`) y el volumen (lo que dice
  `sound_definition_decibels` más la ganancia de la grabación, entre -64 y
  0 decibelios).
- Un sonido sin la marca 9 arranca: arranca su voz
  (`sound_console_voice_start`), se pone en cola su primer trozo de muestras
  y su mando de sonidos tiene uno menos por arrancar. Los demás siguen
  (`0x0021fa80`), si su canal tiene grabación.
- Después se ocupa del trozo siguiente: el que espera (marca 3) se pone en
  cola cuando el canal no tiene segunda grabación y sus muestras se pueden
  pedir; si ninguno espera, el sonido pasa al trozo siguiente de su
  grabación y lo pide, o, si no quedan trozos, se marca como terminado
  (marca 6), y su voz también.
- El nombre es inventado.

### Comprobado

- **La batería completa del hito 200 terminó: 857 funciones de 857, todas
  bien.**

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_channels_update_sources` y de `sound_console_voice_start`.

### Mutaciones

Veintisiete líneas rotas a mano (los límites del volumen, las partes del
tono, el Doppler, las marcas, la cuenta del mando de sonidos, las ganancias
de las grabaciones y todo lo del trozo siguiente): las pruebas notan las
veintisiete. Pendientes las listas completas de los hitos 156 a 203.

## 2026-10-05 — Lo que una voz de la consola toma del sonido de un canal

Una función en `sound/sound_manager.c`, de 620 bytes. De 858 funciones se pasa a
859, el 7,63 % de las del juego (el 4,392 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_build_voice_parameters` | `0x0012a1b0` | `sound/sound_manager.c` |

No estaba apuntada. La llaman `0x00128ca0` y `0x0021c400`, las dos que
`sound_channels_update_sources` usa para pasar a la consola cómo se oye cada
canal. Recibe el canal en `ax`, lo que calculó `unknown_00129aa0` en `ebx`,
dónde dejar el resultado en `edi` y el sonido que suena en la pila.

### Lo que se sabe

- `sound_channel_build_voice_parameters` rellena lo que una voz de la
  consola necesita del sonido de un canal (unos `0x478` bytes).
- Del tag del sonido y de su clase salen dos números, un real de la clase y
  las marcas de `0x30`: cuatro son marcas de la clase, una dice si el byte
  de `0x4C` de la clase es 1, otra si el sonido trae cuatro números más
  (que van a `0x38`) y otra si el número de `0x12` no es `NONE`.
- La clase se busca aquí sin llamar a `sound_class_get`: el juego tiene esa
  función escrita en línea.
- Del sonido que suena salen su semilla (`0x48`) y su escala (`0x4C`), que
  con la marca 8 del sonido se multiplica por la raíz de lo que le falta a
  un real de `unknown_00129aa0` para llegar a 1.
- En `0x48` va además lo que rellena `sound_unknown_00222770`, a la que se
  le pasan los segundos que lleva sonando el sonido, y en `0x468` lo que
  rellena `sound_unknown_00222930`.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 857 funciones del hito 200.

### Mutaciones

Diecisiete líneas rotas a mano (las marcas, los signos de los números, la
división de los segundos, la raíz y lo que se copia): las pruebas notan las
diecisiete. Pendientes las listas completas de los hitos 156 a 202.

## 2026-10-05 — Lo que una voz de la consola toma de un sonido, de los tags

Una función en `sound/sound_manager.c`. De 857 funciones se pasa a
858, el 7,62 % de las del juego (el 4,371 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_00222770` | `0x00222770` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x0012a1b0`. Recibe el sonido en `eax`, un
número en `edx`, y en la pila otro número y dónde dejar el resultado (de
`0x420` bytes).

### Lo que se sabe

- `sound_unknown_00222770` pregunta a `unknown_0018d090` y a
  `unknown_0018d170` por el sonido. Cada una puede dar algo con cuatro
  bloques, en `0x14`, `0x1C`, `0x24` y `0x2C`.
- En `0x10`, `0x14` y `0x18` del resultado deja el primer elemento de los
  tres primeros bloques: los de `unknown_0018d170`, o los de
  `unknown_0018d090` donde esta los tenga.
- En `0x1C` deja la lista que hace `sound_unknown_002225a0`, y en `0x41C`
  su tamaño. Hay lista si el primer elemento del cuarto bloque de alguna de
  las dos dice un tag y trae un bloque propio con algo; si no, el tamaño
  es 0.
- La lista se hace siempre con lo que da `unknown_0018d090`, aunque quien
  la tenga sea la otra: así lo hace el juego, y aquí se hace igual.
- Qué son esos datos no se sabe todavía; el nombre lleva la dirección.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa corre aparte con las 857 funciones del hito 200.

### Mutaciones

Doce líneas rotas a mano: las pruebas notan once. La que no notan es
quitar la comparación de que las dos funciones no den lo mismo: repetir el
trabajo con lo mismo deja el mismo resultado. Pendientes las listas
completas de los hitos 156 a 201.

## 2026-10-05 — Una lista para el sonido de la consola, de un bloque de un tag

Una función en `sound/sound_manager.c`. De 856 funciones se pasa a
857, el 7,61 % de las del juego (el 4,360 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_002225a0` | `0x002225a0` | `sound/sound_manager.c` |

No estaba apuntada. La llama `0x00222770`, que es parte de lo que rellena
`0x0012a1b0` para una voz de la consola. Nueva sin descompilar:
`0x002222c0`, que escribe cada entrada de la lista.

### Lo que se sabe

- `sound_unknown_002225a0` hace una lista a partir de los elementos (de
  `0x18` bytes) de un bloque de un tag: primero cuántos son, y luego lo que
  `0x002222c0` escribe para cada uno, uno detrás de otro, en trozos de
  cuatro bytes. La palabra de `0xA` de cada entrada dice cuántos bytes ocupa
  además de los `0xC` fijos.
- Un elemento con el nombre `0x11000138` se escribe cuatro veces, con los
  números 0 a 3; cualquier otro, una vez, con `NONE`.
- El primer argumento trae, en `4`, algo de `0x1C` bytes para cada elemento
  cuyo bit está puesto en la máscara de `0x10`; se le pasa a `0x002222c0`.
- El último argumento recibe cuántos bytes ocupa la lista.
- De qué es la lista no se sabe todavía (por las funciones que la usan,
  parecen los efectos de sonido de un entorno); el nombre lleva la
  dirección.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La batería
  completa se lanza aparte con las 857 funciones de este hito.

### Mutaciones

Diez líneas rotas a mano (los dos valores iniciales, el bit de la máscara,
los tamaños, los límites de las cuatro veces, el nombre y las dos cuentas
de bytes): las pruebas notan las diez. Pendientes las listas completas de
los hitos 156 a 200.

## 2026-10-05 — Cuatro datos de un sonido, de los tags, para una voz de la consola

Una función en `sound/sound_manager.c`. De 855 funciones se pasa a
856, el 7,60 % de las del juego (el 4,351 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_00222930` | `0x00222930` | `sound/sound_manager.c` |

No estaba apuntada. La llaman cuatro funciones, entre ellas `0x0012a1b0`,
que rellena lo que una voz de la consola necesita de un sonido. Recibe el
sonido en `ecx`, un número en `edx` y dónde dejar el resultado en `esi`.

### Lo que se sabe

- `sound_unknown_00222930` rellena cuatro datos (16 bytes) a partir de lo
  que `unknown_0018d090` da para el sonido: el primer elemento dice un tag,
  el mismo que mira `sound_request_unknown_002226b0`.
- En `2` deja el número que `sound_unknown_001914f0` tiene para el nombre
  del primer elemento de un bloque de ese tag (en `0xC` del tag).
- En `4` y `8`, un número y dónde hay cuatro bytes más: los del propio
  elemento del tag o, si el elemento de `unknown_0018d090` trae un bloque
  suyo con algo, los de ese bloque. En `0xC` repite lo de `8`.
- Sin tag: `NONE` en `2` y en `0xC`, y 0 en `8`.
- Qué son esos datos no se sabe todavía; el nombre lleva la dirección.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función. La última
  batería completa es la del hito 192 (849 funciones, todas bien).

### Mutaciones

Once líneas rotas a mano (las cuentas de los bloques, el tag que no hay,
los cuatro desplazamientos y el nombre): las pruebas notan las once.
Pendientes las listas completas de los hitos 156 a 199.

## 2026-10-05 — Cada sonido que suena: si sigue, y quién lo oye

Una función en `sound/sound_manager.c`, de 684 bytes. De 854 funciones se pasa a
855, el 7,59 % de las del juego (el 4,346 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_sources_update` | `0x00128020` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00128020`); la llama
`sound_update` cada fotograma, justo antes de
`sound_sources_get_channels`.

### Lo que se sabe

- `sound_sources_update` recorre los sonidos que suenan cuyo canal no está
  en pausa, y decide si cada uno sigue.
- Se suelta el sonido con las marcas 6 y 9 cuando la voz de su canal ya no
  hace nada (motivo 2); el que su promoción no deja sonar: es de una regla
  por debajo de la que tiene la promoción, y de la última vez que se miró
  la promoción (motivo 15); y el que `sound_source_unknown_00127fc0` dice
  que no sigue (motivo 3).
- A los demás se les busca el oyente que los oye (`sound_source_get_listener`),
  hasta la distancia a la que se oye el sonido. Sin oyente, el sonido empieza
  a apagarse, en dos segundos, y su marca 2 lo recuerda; cuando vuelve a
  tener oyente, vuelve en medio segundo (`sound_sources_cross_fade`) y pierde
  la marca.
- Un sonido cuyo tag tiene la marca 3 recibe la marca 0 en lo que el juego
  dice de él.
- El juego mira el canal de un sonido sin canal (el de antes del primero)
  cuando el sonido tiene las marcas 6 y 9: aquí se hace igual.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_update`. La última batería completa es la del hito 192 (849
  funciones, todas bien).

### Mutaciones

Quince líneas rotas a mano (la pausa del canal, las dos marcas, el estado
de la voz, los motivos, las tres comparaciones de la promoción, la marca del
tag, el tiempo de apagado, el orden de los sonidos al volver y las marcas
que se ponen y se quitan): las pruebas notan las quince. Pendientes las
listas completas de los hitos 156 a 198.

## 2026-10-05 — Los sonidos a los que les llega la hora consiguen canal

Una función en `sound/sound_manager.c`, de 558 bytes. De 853 funciones se pasa a
854, el 7,58 % de las del juego (el 4,323 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_sources_get_channels` | `0x001282d0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_001282d0`); la llama
`sound_update` cada fotograma. En `sound_source_callbacks` ya se sabe qué es
la función de `4`: el aviso de que el sonido tiene canal.

### Lo que se sabe

- `sound_sources_get_channels` recorre los sonidos que suenan y, a los que
  les ha llegado la hora (el número de `0x8C` no pasa del de `0x84` de los
  globales del sonido), les busca canal.
- Si sus muestras se pueden pedir (`sound_source_request_data`), el sonido
  se queda con el canal que diga `sound_source_get_channel`. Sin canal, se
  suelta (motivo 7). Si el canal era de otro sonido, ese se suelta con el
  motivo que dio la función, y el canal también. A quien hay que avisar del
  sonido se le avisa de que ya tiene canal, salvo que tenga la marca 5.
- Si sus muestras no se pueden pedir y no tiene canal, se suelta (motivo 14)
  cuando el número de `0xE` de su clase es 0 y no tiene la marca 4. Antes,
  su grabación se aparta para ser la siguiente de su rango de tono, si no
  hay ya una apartada y la clase no tiene la marca 5.
- El nombre es inventado.

### Comprobado

- **La batería completa del hito 192 terminó: 849 funciones de 849, todas
  bien.** Es la primera completa desde el hito 173.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_update`, de `sound_source_stop` y de `sound_source_delete`.

### Mutaciones

Doce líneas rotas a mano (la comparación de la hora, los dos motivos, las
dos marcas, el número de la clase, la grabación apartada, el signo del
rango de tono, y lo que se apunta en el canal y en el sonido): las pruebas
notan las doce. Pendientes las listas completas de los hitos 156 a 197.

## 2026-10-05 — Cómo se oye en una sala el sonido de una casilla, y su filtro

Una función en `sound/sound_manager.c`, de 676 bytes. De 852 funciones se pasa a
853, el 7,57 % de las del juego (el 4,304 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_slot_update_room` | `0x002211e0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar desde el hito anterior
(`unknown_002211e0`); la llama `sound_console_slot_set_parameters`. Nuevas
sin descompilar, las dos de la consola: `0x0037c6e5` (cómo se oye un sonido
en una sala) y `0x0037b68b` (su filtro). También las estructuras
`sound_console_i3dl2_source` (`0x24` bytes) y `sound_console_filter`
(`0x18`), y los globales `unknown_00470084` (una frecuencia, 8000) y
`unknown_00470088` (unos decibelios, 0).

### Lo que se sabe

- `sound_console_slot_update_room` le dice a la consola cómo se oye en una
  sala el sonido de una casilla, y con qué filtro.
- En `0x2ABC` del sonido de la consola hay tres ganancias. Se pasan a
  centésimas de decibelio (2000 por el logaritmo en base 10), de -6400 a 0:
  las dos primeras son para el sonido directo y la tercera para la sala.
- A la sala se le quitan además hasta 64 decibelios según lo lejos de 0 que
  estén los decibelios que recibe la función, sin bajar de -100.
- Los dos reales de la casilla que rellena
  `sound_console_slot_set_parameters` son cuánto tapa el sonido lo que hay
  en medio (`0x30`) y una pared (`0x2C`): a la consola va lo que queda hasta
  1, también en centésimas de decibelio.
- El filtro es siempre del mismo modo, con dos números: el de la frecuencia
  de `unknown_00470084` y el de los decibelios de `unknown_00470088`.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_console_slot_set_parameters`. La batería completa corre aparte con
  las 849 funciones del hito 192.

### Mutaciones

Diez líneas rotas a mano: las pruebas notan nueve. La que no notan es el
tope de 0 para la sala: no se puede pasar de 0, porque a un número que ya
no es mayor que 0 solo se le resta. Pendientes las listas completas de los
hitos 156 a 196.

## 2026-10-05 — Una casilla del sonido de la consola recibe dónde está su sonido

Una función en `sound/sound_manager.c`. De 851 funciones se pasa a
852, el 7,56 % de las del juego (el 4,281 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_console_slot_set_parameters` | `0x00220e00` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00220e00`); la llama
`sound_channel_update_slot`. Nuevas sin descompilar: `0x0037c668` (de la
consola: le dice dónde está un sonido) y `0x002211e0`. También la
estructura `sound_console_slot` (`0x3C` bytes).

### Lo que se sabe

- `sound_console_slot_set_parameters` le da a una casilla del sonido de la
  consola dónde está su sonido y dos reales más.
- A la consola se le dice el sitio la primera vez (casilla sin la marca 1, o
  cuando el primer byte del sonido de la consola es 0) y cuando se ha movido
  0,05 o más en algún eje. La consola recibe la altura en segundo lugar: el
  juego le pasa `x`, `z`, `y`.
- Cada uno de los dos reales (en `0x10` y `0x14` de lo que recibe) pasa a la
  casilla tal cual la primera vez, cuando cambia la marca 0 o cuando su paso
  (en `0x1C` y `0x20`) no es mayor que 0; si no, la casilla se acerca a él
  como mucho ese paso cada vez.
- Si no hay nada nuevo (los dos reales a menos de una milésima y la misma
  marca) no hace más; si lo hay, llama a `0x002211e0` con la casilla y el
  real de `0x18`.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_channel_update_slot`. La batería completa corre aparte con las 849
  funciones del hito 192.

### Mutaciones

Siete líneas rotas a mano (los dos límites de 0,05 y 0,001, los dos pasos
que no son mayores que 0, la suma del paso, el orden de los ejes y la
condición de la primera vez): las pruebas notan las siete. Pendientes las
listas completas de los hitos 156 a 195.

## 2026-10-05 — Cuánto se oye un sonido desde donde está su oyente

Una función en `sound/sound_manager.c`. De 850 funciones se pasa a
851, el 7,55 % de las del juego (el 4,266 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_spatial_gain` | `0x0012ad30` | `sound/sound_manager.c` |

No estaba apuntada: solo la llama `0x00129aa0`, que aún no está hecha.
Recibe en `eax` el camino del oyente al sonido, en `ecx` lo que el juego
dice del sonido, y en la pila el sonido y hacia dónde apunta.

### Lo que se sabe

- `sound_source_spatial_gain` da cuánto se oye un sonido desde donde está su
  oyente, de 0 a 1: lo que dice `sound_source_distance_gain` de lo lejos que
  está, por lo que dice su cono.
- El cono: dentro del ángulo interior se oye todo; fuera del exterior, lo
  que diga la ganancia del cono exterior (en decibelios); entre los dos,
  según el ángulo (`sound_interpolation_fraction`). El ángulo es el que hay
  entre hacia dónde apunta el sonido y el camino del sonido al oyente.
- El cono no cuenta si el sonido no trae cono propio y su tag tiene la marca
  12 (la misma condición que `sound_unknown_00125060`), si ya casi no se oye
  por la distancia (menos de 0,0001) o si el oyente está a menos de una
  milésima del sonido.
- Con la marca 9 de lo que el juego dice del sonido, el resultado es lo que
  *no* se oye (1 menos lo anterior).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_source_distance_gain`. La batería completa corre aparte con las 849
  funciones del hito 192.

### Mutaciones

Seis líneas rotas a mano entre este hito y el anterior (el sentido de
`sound_interpolation_fraction`, la longitud mínima, el signo de la marca 9,
la constante de la milésima al cuadrado, un signo del producto del Doppler y
su límite de arriba): las pruebas notan las seis. La constante solo se notó
al añadir un caso cuyo cuadrado de la longitud es el real anterior al
límite. Pendientes las listas completas de los hitos 156 a 194.

## 2026-10-05 — El efecto Doppler de un sonido para un oyente

Una función en `sound/sound_manager.c`. De 849 funciones se pasa a
850, el 7,55 % de las del juego (el 4,250 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_listener_doppler_pitch` | `0x0012a9d0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0012a9d0`). En `sound_listener`
ya se sabe qué son los bytes de `8` a `0x48`: una matriz (dónde está el
oyente y cómo está girado) y la velocidad a la que se mueve.

### Lo que se sabe

- `sound_listener_doppler_pitch` da el efecto Doppler de un sonido para un
  oyente, en cents (1200 por octava): más agudo cuanto más rápido se acercan
  el sonido y el oyente, más grave cuanto más rápido se alejan.
- La velocidad del sonido es 111,54855 unidades del mundo por segundo: 340
  metros. El resultado va de tres octavas abajo (-3600) a dos arriba (2400).
- Detrás del sitio del sonido, a `0x10`, va su velocidad. La del oyente está
  en sus propios ejes: se gira con su matriz antes de usarla.
- Un mando de sonidos se queda con este número para cada jugador de la
  consola (`sound_playback_controller_unknown_00219ed0`).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de las que
  usan a los oyentes. La batería completa corre aparte con las 849 funciones
  del hito 192.

### Mutaciones

Pendientes las de los hitos 156 a 193.

## 2026-10-04 — La lista de sonidos de los que hay que calcular cómo se oyen

Una función en `sound/sound_manager.c`. De 848 funciones se pasa a
849, el 7,54 % de las del juego (el 4,233 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channels_build_queries` | `0x00129100` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00129100`); la llama
`sound_update` cada fotograma. También la estructura `sound_query` (`0x48`
bytes) y el global `unknown_00468788`.

### Lo que se sabe

- `sound_channels_build_queries` les consigue voz a los canales que suenan
  (`sound_channel_get_voice`) y hace la lista de los sonidos de los que hay
  que calcular cómo se oyen donde están: como mucho 72.
- Entra uno por cada casilla de un sonido con sitio que no tenga ninguna de
  tres marcas, y uno por cada sonido del tipo 0 con la marca 12 que sea del
  bsp cargado; a estos se les pregunta en el punto de `unknown_00468788`.
- Cada entrada dice el oyente, el sitio, dónde está en el nivel y cuatro
  marcas, y tiene sitio para tres reales. Dónde quedan esos reales se
  apunta en dos tablas, por casilla y por voz, que luego lee
  `sound_channels_unknown_00129380` para pasárselos a
  `sound_channel_update_slot`.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_update`. La batería completa del hito 186 (843 funciones) se
  lanzó aparte y se interrumpió al apagar el equipo: hay que volver a
  lanzarla. La última completa es la del hito 173 (824 funciones, todas
  bien).

### Mutaciones

Pendientes las de los hitos 156 a 192.

## 2026-10-04 — El sonido de un canal y cómo se oye donde está

Una función en `sound/sound_manager.c`, de 784 bytes. De 847 funciones se
pasa a 848, el 7,53 % de las del juego (el 4,211 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_update_slot` | `0x00129790` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00129790`): su segundo argumento
es un puntero a tres reales, o nulo. Nueva sin descompilar: `0x00220e00`.
También la estructura `sound_slot_parameters` (`0x24` bytes).

### Lo que se sabe

- `sound_channel_update_slot` es lo que se hace cada vez con el sonido de un
  canal y tres reales que dicen cómo se oye donde está (o ninguno).
- Un sonido del tipo 0 con la marca 12 pierde de su real de `0x1C` lo que
  diga el tercer real, según su clase; sin reales, lo pierde todo si no es
  del bsp cargado.
- Un sonido de los tipos 1 y 2 que suena con una voz de sonido, en un canal
  con casilla, le da a la casilla lo que toma `0x00220e00`: dónde está el
  sonido visto desde su oyente (`matrix4x3_inverse_transform_point` con la
  matriz del oyente, que está en `8`), dos de los reales según su clase, y
  tres números de su clase. Una vez por voz de sonido cada vez.
- La prueba descubrió que `sound_source_get_position` deja en `eax` el sitio
  que recibió ahí y que el juego lo sigue usando después: ahora lo devuelve.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función, de
  `sound_source_get_position` y de `sound_channels_unknown_00129380`. La
  batería completa corre aparte con las 843 funciones del hito 186.

### Mutaciones

Pendientes las de los hitos 156 a 191.

## 2026-10-04 — La voz de sonido de un sonido que suena

Una función en `sound/sound_manager.c`. De 846 funciones se pasa a
847, el 7,52 % de las del juego (el 4,185 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_voice_get_for_source` | `0x0021de90` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_0021de90`) y sin su argumento:
recibe el sonido en `ebx`. `sound_channel_get_voice` ya se lo pasa.

### Lo que se sabe

- `sound_voice_get_for_source` da la voz de sonido por la que se oye un
  sonido que suena, y le apunta uno más que la usa: una que el sonido pueda
  compartir (`sound_voice_matches_source`), o una nueva, con una casilla de
  los globales del sonido y lo que dice el sonido: su clase, su marca, a
  quién avisar y con qué, y su sitio.
- Da `NONE` si no hay sitio para otra voz o no queda casilla.
- Varios sonidos iguales que suenan en el mismo sitio comparten así una
  sola voz de sonido.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_channel_get_voice`. La batería completa corre aparte con las 843
  funciones del hito 186.

### Mutaciones

Pendientes las de los hitos 156 a 190.

## 2026-10-04 — Un canal consigue una voz de la consola

Una función en `sound/sound_manager.c`. De 845 funciones se pasa a
846, el 7,51 % de las del juego (el 4,173 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_get_voice` | `0x001295e0` | `sound/sound_manager.c` |

Nuevas sin descompilar: `0x0037b75b`, de la consola, y `0x0021de90`.

### Lo que se sabe

- `sound_channel_get_voice` le consigue a un canal una voz del sonido de la
  consola para tocar su sonido: la primera de las de su tipo que esté libre
  y sin tomar, mirando los vectores de bits que rellena
  `sound_console_get_state`.
- Con la voz va una casilla: la que diga el sonido (su byte `0xA2`), si su
  flujo no está ocupado (`0x0037b75b`); o, para un sonido con la marca 2 en
  `6`, la de una voz de sonido que da `0x0021de90`.
- Un canal que ya tiene voz se la queda, salvo que su sonido tenga esa
  marca y el canal no tenga casilla.
- Si no consigue voz, suelta el sonido y el canal con el motivo 7 y
  devuelve falso.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasó la prueba de esta función. La batería completa
  corre aparte con las 843 funciones del hito 186.

### Mutaciones

Pendientes las de los hitos 156 a 189.

## 2026-10-04 — El sonido de cada canal, cada vez

Una función en `sound/sound_manager.c`. De 844 funciones se pasa a
845, el 7,50 % de las del juego (el 4,159 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channels_update_sources` | `0x001293f0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_001293f0`); la llaman `sound_idle`
y `sound_update`. Nuevas sin descompilar: `0x00129aa0`, `0x0021c400` y
`0x00128ca0`. También la tercera función de la tabla de a quién se avisa de
un sonido (`playing`).

### Lo que se sabe

- `sound_channels_update_sources` es lo que se hace cada vez con el sonido
  de cada canal que suena y no está en pausa.
- Un sonido que ya se apagó (terminó su fundido y se oye a -64 decibelios o
  menos) se suelta con el motivo 4; uno que terminó de encenderse deja de
  fundirse.
- Para un canal con voz, `0x00129aa0` calcula cómo se oye el sonido; a eso
  se le suman los decibelios del real que sigue a la pausa del juego, y el
  resultado se le da a `0x0021c400` (sonidos de los tipos 1 a 4) o a
  `0x00128ca0`.
- Después se avisa a quien el sonido diga de por dónde va sonando (su
  tercera función), si el sonido no está marcado.
- Antes de todo rellena lo que el juego guarda del sonido de la consola
  (`sound_console_get_state`).
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de las que la
  llaman. La batería completa corre aparte con las 843 funciones del hito
  186.

### Mutaciones

Pendientes las de los hitos 156 a 188.

## 2026-10-04 — El reloj del sonido y el fundido al pausar

Una función en `sound/sound_manager.c`. De 843 funciones se pasa a
844, el 7,49 % de las del juego (el 4,143 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_update_time` | `0x001269f0` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_001269f0`); la llama `sound_idle`.

### Lo que se sabe

- `sound_update_time` hace avanzar el tiempo del sonido con el reloj de la
  consola: sus milésimas (`0x84`), los segundos de esta vez (`0x1E0`) y los
  dos reales que cuentan segundos para `sound_set_unknown1f8`.
- Además mueve el real de `0x1E4`, que sigue a la pausa del juego en una
  campaña en curso: baja a 0 mientras el juego está en pausa, tras una
  espera y a la velocidad que dicen dos números del sonido de los sitios
  del mapa, y vuelve a 1 al reanudar, a la velocidad que dice un tercero,
  salvo que el bucle principal tenga algo pendiente. En cualquier otra
  partida vale 1.
- En los globales del juego, `0x1120` dice si hay partida en curso y
  `0x1121` si está en pausa.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función y de
  `sound_idle`. La batería completa corre aparte con las 843 funciones del
  hito 186.

### Mutaciones

Pendientes las de los hitos 156 a 187.

## 2026-10-04 — Dos datos más para arrancar un sonido

Dos funciones en `sound/sound_manager.c`. De 841 funciones se pasa a
843, el 7,48 % de las del juego (el 4,127 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_001914f0` | `0x001914f0` | `sound/sound_manager.c` |
| `sound_request_unknown_002226b0` | `0x002226b0` | `sound/sound_manager.c` |

La segunda estaba apuntada sin descompilar (`unknown_002226b0`) y mal: recibe
además el sonido en `edi`, y cambia lo que el juego da para arrancar el
sonido. `sound_source_create` ya le pasa el sonido. También el global
`unknown_00510c90`.

### Lo que se sabe

- `sound_request_unknown_002226b0` completa lo que el juego da para arrancar
  un sonido con dos datos de los tags: lo que `0x0018d090` o `0x0018d170`
  tienen para el sonido dice un tag, y el primer elemento de un bloque de
  ese tag tiene un nombre. Para uno de los nombres toma un número de
  `unknown_00510c90`; para los que conoce `sound_unknown_001914f0`, su
  número.
- `sound_unknown_001914f0` da un número para cada uno de seis nombres (los
  números de seis textos del juego) y `NONE` para los demás. Lo más
  probable es que sea de otro archivo.
- No se sabe qué son los nombres ni los números.

### Sin comprobar

- En este hito solo se pasaron las pruebas de estas dos funciones, de
  `sound_source_create` y de `sound_source_new`. La batería completa pasó
  con las 824 funciones del hito 173.

### Mutaciones

Pendientes las de los hitos 156 a 186.

## 2026-10-04 — Crear el sonido que empieza a sonar

Una función en `sound/sound_manager.c`, de 864 bytes. De 840 funciones se
pasa a 841, el 7,47 % de las del juego (el 4,118 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_create` | `0x00126000` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00126000`). Nueva sin
descompilar: `0x002226b0`. También el global `unknown_00547f14`.

### Lo que se sabe

- `sound_source_create` crea el sonido que suena, para un oyente, con lo
  que el juego da para arrancarlo y la regla de su promoción. No lo crea si
  se oiría a -64 decibelios o menos, o si no hay sitio en la tabla.
- Le pone una ganancia y un tono al azar; lo que el juego dice de él; su
  controlador de reproducción, a quién avisar y con qué, si el juego los
  da; y el rango de tono y la grabación que toca: los que da el juego, o el
  rango de la regla de su promoción, o el que corresponde a su tono, y una
  grabación elegida con `sound_choose_permutation`.
- Un sonido que está lejos empieza más tarde: el retraso sale de su
  distancia al oyente por `unknown_00547f14`, y solo cuenta si pasa de 100
  y el sonido no tiene ninguna de dos marcas.
- Lo que el juego da para arrancar un sonido lleva unas marcas que dicen
  qué campos trae: de la ganancia (bit 7), del controlador (4), de a quién
  avisar (5 y 3) y del rango de tono y la grabación (10), entre otros.
- Con ella, `sound_source_new` (arrancar un sonido) ya no llama a nada sin
  descompilar. Todo el camino desde que el juego pide un sonido hasta que
  queda creado y con sus muestras pedidas está descompilado y comprobado.
- El nombre es inventado.

### Sin comprobar

- En este hito solo se pasaron las pruebas de esta función (900 casos) y de
  `sound_source_new`. La batería completa pasó con las 824 funciones del
  hito 173.

### Mutaciones

Pendientes las de los hitos 156 a 185.

## 2026-10-04 — Si un sonido debe empezar, y su promoción

Dos funciones en `sound/sound_manager.c`. De 838 funciones se pasa a
840, el 7,46 % de las del juego (el 4,089 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_start_test` | `0x00126c30` | `sound/sound_manager.c` |
| `sound_promotion_update` | `0x00126ec0` | `sound/sound_manager.c` |

Las dos estaban apuntadas sin descompilar (`unknown_00126c30` y
`unknown_00126ec0`). Con ellas se corrigen los argumentos de
`sound_source_new`: recibe el sonido en `ebx` y, en la pila, lo que el juego
da para arrancarlo (que lleva en `0xC` lo que dice del sonido).

### Lo que se sabe

- `sound_start_test` decide si un sonido debe empezar a sonar, y si no, por
  qué. No, si no hay sonido o el sonido es de un tipo que no se reproduce
  (motivo 5). No, al azar, tantas veces como digan su escala y su forma de
  reproducirse que se salta (motivo 1). Y no si no lo oye ningún oyente, o
  se oiría a -64 decibelios o menos y ninguna de dos marcas dice que suene
  igualmente (motivo 4). Dice también qué oyente lo oye.
- `sound_promotion_update` lleva la promoción de un sonido cuando va a
  empezar: en qué regla está y si el sonido no debe sonar por un rato. Cada
  regla tiene un tiempo propio que baja con el tiempo y sube cada vez que
  el sonido empieza; cuando llega a lo que dice la regla, se pasa a la
  siguiente. Una regla alcanzada impide que el sonido vuelva a sonar, en
  esa regla o en una menor, durante los segundos que diga.
- De lo que llama `sound_source_new` solo queda `0x00126000`, la que crea
  el sonido.
- Los nombres son inventados.

### Comprobado

- La batería completa que corría aparte terminó: las 824 funciones del hito
  173 pasan contra el ejecutable original. En este hito se pasaron las
  pruebas de estas dos funciones y la de `sound_source_new`.

### Mutaciones

Pendientes las de los hitos 156 a 184.

## 2026-10-04 — El canal que recibe un sonido

Una función en `sound/sound_manager.c`. De 837 funciones se pasa a
838, el 7,44 % de las del juego (el 4,063 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_get_channel` | `0x00128700` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_source_get_channel` decide qué canal recibe un sonido que va a
  sonar, o por qué no recibe ninguno. Une las funciones de los cuatro hitos
  anteriores y no llama a nada sin descompilar.
- Si el sonido ya tiene canal, es ese.
- Un sonido de una clase de las que la misma cosa solo toca uno a la vez
  (bit 1 de las marcas de la clase) ocupa el sitio del que esa cosa estaba
  tocando: en el mismo canal si es de su tipo (motivo 13); si no, se suelta
  el otro sonido con su canal y se busca uno.
- Cualquier otro sonido no recibe canal si uno igual empezó en este mismo
  instante (motivo 5); si ya hay tantos iguales como deja su clase, o
  tantos de su sonido, le quita el canal a uno de ellos (motivos 5 y 6); y
  si no, se busca un canal con `sound_source_find_channel`.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasó la prueba de esta función.

### Mutaciones

Pendientes las de los hitos 156 a 183.

## 2026-10-04 — Los canales que ya tocan el mismo sonido

Una función en `sound/sound_manager.c`. De 836 funciones se pasa a
837, el 7,43 % de las del juego (el 4,045 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_find_same` | `0x00128500` | `sound/sound_manager.c` |

También la estructura `sound_same_channels` (`0x4A` bytes) y la octava
función de la tabla de a quién se avisa de un sonido (`same`).

### Lo que se sabe

- `sound_source_find_same` hace dos listas para un sonido que va a sonar:
  los canales que ya tocan su mismo sonido, y de esos, los que lo tocan
  para lo mismo (avisan al mismo, y con lo mismo o con algo que su octava
  función dice que es igual). Con cada lista va el máximo que admite la
  clase del sonido (sus dos primeras palabras). Dice también si alguno de
  la segunda lista empezó a sonar en este mismo instante.
- Sirve para no pasarse de tantas copias del mismo sonido a la vez.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasó la prueba de esta función.

### Mutaciones

Pendientes las de los hitos 156 a 182.

## 2026-10-04 — Buscar un canal para un sonido

Una función en `sound/sound_manager.c`. De 835 funciones se pasa a
836, el 7,42 % de las del juego (el 4,031 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_find_channel` | `0x00128920` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_source_find_channel` busca un canal para un sonido que va a sonar,
  entre los del tipo de su sonido (el byte `4` del tag del sonido contra el
  tipo del canal): el primero que esté libre; si todos suenan, el de un
  sonido ante el que el nuevo tenga preferencia
  (`sound_source_goes_before`), y de esos, el del sonido con menos
  preferencia de todos. Da `NONE` si no hay ninguno.
- Dice también por qué se quitó un canal que sonaba: el motivo 9, que es
  el que recibe después a quien se avisa del sonido que pierde el canal.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasó la prueba de esta función.

### Mutaciones

Pendientes las de los hitos 156 a 181.

## 2026-10-04 — Qué canal toma un sonido y qué sonido va antes

Dos funciones en `sound/sound_manager.c`. De 833 funciones se pasa a
835, el 7,41 % de las del juego (el 4,020 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_choose_channel` | `0x00128a60` | `sound/sound_manager.c` |
| `sound_source_goes_before` | `0x00128b90` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_source_choose_channel` elige, entre varios canales, el que un
  sonido nuevo le quita al sonido que lo ocupa: el del sonido que lleva más
  tiempo sonando, entre los que llevan al menos lo que dice la clase del
  sonido nuevo (el número en `4` de la clase); o uno cuyo sonido tenga
  menos que el nuevo en `0xA0`. Da `NONE` si no puede quitar ninguno.
- `sound_source_goes_before` dice si un sonido va antes que otro: si su
  clase tiene más prioridad (la palabra en `0xC` de la clase); a igual
  prioridad y siendo del mismo sonido, si tiene más en `0xA0`; y si no, si
  el otro está más lejos de su oyente que una distancia dada.
- En un sonido que suena, `0xA1` es su oyente y `0x8C` el momento en que
  empezó a sonar.
- Los nombres son inventados.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasaron las pruebas de estas dos.

### Mutaciones

Pendientes las de los hitos 156 a 180.

## 2026-10-04 — Quién oye un sonido

Dos funciones en `sound/sound_manager.c`. De 831 funciones se pasa a
833, el 7,39 % de las del juego (el 4,001 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_get_listener` | `0x00127d00` | `sound/sound_manager.c` |
| `sound_unknown_0012a6d0` | `0x0012a6d0` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_source_get_listener` dice qué oyente oye un sonido y a qué
  distancia. Un sonido sin sitio lo oye el primer jugador local, a
  distancia 0. Uno con sitio lo oye el oyente más cercano
  (`sound_source_unknown_00127e20`), salvo que
  `sound_source_unknown_00221da0` diga que no; después, si el sonido no
  tiene ninguna de dos marcas (bits 8 y 11), más allá de una distancia que
  se le da no lo oye nadie; y si tiene alguna y no quedó nadie, lo oye el
  primer jugador local.
- `sound_unknown_0012a6d0` da los decibelios de algo que sube desde el
  silencio según el tiempo que lleva y lo que tarda (de 0 a 1, o al revés
  si la duración es negativa), con las maneras 1 y 2 de
  `sound_decibels_interpolate`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasaron las pruebas de estas dos.

### Mutaciones

Pendientes las de los hitos 156 a 179.

## 2026-10-04 — Elegir al azar una grabación de un rango de tono

Una función en `sound/sound_manager.c`. De 830 funciones se pasa a
831, el 7,38 % de las del juego (el 3,981 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_pitch_range_choose_permutation` | `0x00219110` | `sound/sound_manager.c` |

Estaba apuntada sin descompilar (`unknown_00219110`).

### Lo que se sabe

- `sound_pitch_range_choose_permutation` elige al azar una grabación de un
  rango de tono que no haya sonado desde que sonaron todas. Lleva un bit por
  grabación; cuando ya sonaron todas, empieza de nuevo dejando marcada solo
  la última, para no repetirla enseguida.
- Cada grabación dice con qué frecuencia se la salta (la palabra en `2`, de
  65.535): si le toca saltarse, se queda marcada como sonada y se busca
  otra, hasta el doble de veces que grabaciones hay.
- Puede avisar de que con esta ya han sonado todas.
- Con ella queda completa la elección de la grabación:
  `sound_choose_permutation` ya no llama a nada sin descompilar.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasaron la prueba de esta función y la de
  `sound_choose_permutation`.

### Mutaciones

Pendientes las de los hitos 156 a 178.

## 2026-10-04 — El rango de tono de un sonido para un tono

Una función en `sound/sound_manager.c`. De 829 funciones se pasa a
830, el 7,37 % de las del juego (el 3,968 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_choose_pitch_range` | `0x00218f50` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_choose_pitch_range` elige el rango de tono de un sonido para un
  tono: el que se pide, si admite ese tono y tiene grabaciones; si no, el
  primero con grabaciones que lo admita; y si ninguno lo admite, el que
  quede más cerca. Da `NONE` si ningún rango tiene grabaciones.
- Los parámetros de un rango de tono son cinco números de 16 bits (tonos en
  centésimas de semitono): el primero es el tono con el que se grabó, el
  cuarto y el quinto dicen entre qué tonos se reproduce, y el segundo y el
  tercero se usan para medir lo lejos que queda un tono.
- Recibe el tono en `xmm1`.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasó la prueba de esta función.

### Mutaciones

Pendientes las de los hitos 156 a 177.

## 2026-10-04 — Si un sonido puede compartir una voz de sonido

Una función en `sound/sound_manager.c`. De 828 funciones se pasa a
829, el 7,36 % de las del juego (el 3,959 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_voice_matches_source` | `0x0021e0b0` | `sound/sound_manager.c` |

También el global `unknown_0055e780` y la séptima función de la tabla de a
quién se avisa de un sonido (`matches`).

### Lo que se sabe

- `sound_voice_matches_source` dice si un sonido que suena puede compartir
  una voz de sonido: tiene que avisar al mismo, tener la misma marca y una
  clase que coincida (`sound_classes_match`). Después, si hay a quién
  avisar, tiene que ser con lo mismo y lo decide su séptima función; si no
  lo hay, tienen que estar en el mismo sitio del nivel y coincidir seis
  reales a menos de 0,0001. Nunca mientras `unknown_0055e780` esté puesto.
- La prueba descubrió una diferencia que no es del juego: si a
  `sound_classes_match` sustituida se le hace devolver 256, el juego mira
  solo el byte bajo y el código de aquí todo el número, porque el
  compilador sabe que la de verdad solo da 0 o 1. La prueba usa 0 y 1.
- El nombre es inventado.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasó la prueba de esta función.

### Mutaciones

Pendientes las de los hitos 156 a 176.

## 2026-10-04 — Los bits de las grabaciones que sonaron de una pista

Dos funciones en `sound/sound_manager.c`. De 826 funciones se pasa a
828, el 7,35 % de las del juego (el 3,950 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_track_get_played_bits` | `0x0021cb90` | `sound/sound_manager.c` |
| `looping_sound_track_set_played_bits` | `0x0021cbe0` | `sound/sound_manager.c` |

### Lo que se sabe

- Cada pista de un sonido en bucle puede llevar por su cuenta los bits de
  las grabaciones que ya sonaron (en `0x8C` del sonido en bucle, cada `0x14`
  bytes), si tiene alguna de dos marcas y el sonido en bucle tiene
  controlador de reproducción. Si no, se usan los del rango de tono del
  sonido, como en cualquier otro sonido.
- `looping_sound_track_get_played_bits` los lee (y puede borrarlos al
  leerlos) y `looping_sound_track_set_played_bits` los escribe; esta última,
  cuando van en el rango de tono, apunta también cuál fue la última
  grabación.
- Los nombres son inventados.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasaron las pruebas de estas dos.

### Mutaciones

Pendientes las de los hitos 156 a 175.

## 2026-10-04 — Leer las muestras de un trozo y un byte de lo que un sonido tiene de más

Dos funciones en `sound/sound_manager.c`. De 824 funciones se pasa a
826, el 7,33 % de las del juego (el 3,944 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_chunk_load` | `0x00218a10` | `sound/sound_manager.c` |
| `sound_unknown_00218e50` | `0x00218e50` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_cache_chunk_load` empieza a leer del mapa las muestras de un trozo
  de una grabación y las mete en la caché de los sonidos: crea un bloque de
  la caché (`lruv_block_new`), apunta en la tabla `unknown_00502104` de qué
  trozo es y lanza la lectura con `cache_file_read_async`, en múltiplos de
  512 bytes. El byte `2` del elemento de la tabla dice cuándo terminó la
  lectura. Si no hay sitio en la caché no hace nada.
- `sound_unknown_00218e50` da un byte, como real de 0 a 1, de lo que los
  sonidos de un mapa tienen de más de algunos sonidos (bloque en `0x50`, de
  `0x2C` bytes; el número en `0xE` del sonido dice cuál): el de una
  grabación de un rango de tono, en un sitio de sus bytes. No se sabe qué
  son esos bytes.
- Los nombres son inventados.

### Sin comprobar

- La batería completa corre aparte con las 824 funciones del hito 173; en
  este hito solo se pasaron las pruebas de estas dos.

### Mutaciones

Pendientes las de los hitos 156 a 174.

## 2026-10-04 — Soltar la caché de sonidos, elegir una grabación y el sonido de una pista

Tres funciones en `sound/sound_manager.c`. De 821 funciones se pasa a
824, el 7,31 % de las del juego (el 3,932 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_dispose` | `0x002186b0` | `sound/sound_manager.c` |
| `sound_choose_permutation` | `0x00219070` | `sound/sound_manager.c` |
| `looping_sound_get_or_new_source` | `0x0021b910` | `sound/sound_manager.c` |

`sound_cache_dispose` estaba apuntada sin descompilar (`unknown_002186b0`).
Nuevas sin descompilar: `0x00219110` y `0x0021b940`.

### Lo que se sabe

- `sound_cache_dispose` suelta la caché de los sonidos: la tabla que cuenta
  quién está en cada trozo de muestras y la caché misma, que libera quien
  la creó.
- `sound_choose_permutation` decide qué grabación de un rango de tono de un
  sonido suena ahora: la que se había apartado para ser la siguiente (byte
  `5` del rango), si la hay; si no, la que elija `0x00219110` con los bits
  de las que ya sonaron, la última que sonó y la semilla de los números al
  azar del juego. La guarda como la última (byte `4`).
- `looping_sound_get_or_new_source` da el sonido de una pista de un sonido
  en bucle que es de un sonido y un rango de tono; si no lo hay, lo crea
  `0x0021b940`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de `sound_dispose`. Se lanza ahora aparte, con las 824
  funciones.

### Mutaciones

- Hito 155: terminó la comprobación que corría aparte. Sin detectar: 0 de 40 líneas rotas a propósito.
- Hitos 156 a 173: pendientes.

## 2026-10-04 — Un efecto de sonido nuevo y lo que dice de su sonido

Tres funciones en `sound/sound_manager.c`. De 818 funciones se pasa a
821, el 7,29 % de las del juego (el 3,923 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_effect_new` | `0x0021d2c0` | `sound/sound_manager.c` |
| `sound_effect_get_source` | `0x0021db80` | `sound/sound_manager.c` |
| `sound_effect_definition_get` | `0x0021dd30` | `sound/sound_manager.c` |

Nuevas sin descompilar: `0x0018d090`, `0x0018d170` y `0x0018cbc0`.

### Lo que se sabe

- `sound_effect_definition_get` busca lo que un tag dice de un efecto de
  sonido: los elementos de un bloque (en `0x2C`) de lo que da `0x0018d090`,
  y dice si el primero tiene algo en `8`. Lo que da `0x0018d170` solo se
  mira cuando el número es `NONE`, y no cambia el resultado: en ese caso, si
  la primera no dio nada, el juego lee de un puntero nulo.
- `sound_effect_new` crea un efecto de sonido con un real y un número de 16
  bits, y se lo pasa a `0x0021d630` con un 0. Recibe el real en `xmm0`.
- `sound_effect_get_source` da lo que el juego dice del sonido de un efecto
  según su tipo: para el tipo 1 copia lo que dice su sonido, con las
  distancias a las que se oye si su marca 8 las pide; para el tipo 2 lo
  pide a `0x0018cbc0`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Las voces al cambiar de bsp, arrancar una voz de la consola y dos clusters

Tres funciones en `sound/sound_manager.c`. De 815 funciones se pasa a
818, el 7,26 % de las del juego (el 3,907 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_voices_initialize_for_new_structure_bsp` | `0x0021dde0` | `sound/sound_manager.c` |
| `sound_console_voice_start` | `0x0021f8a0` | `sound/sound_manager.c` |
| `sound_source_unknown_00221da0` | `0x00221da0` | `sound/sound_manager.c` |

La primera estaba apuntada sin descompilar (`unknown_0021dde0`). Nuevas sin
descompilar: `0x002ae7a0`, `0x0037c705`, `0x002201f0`, `0x0021fa80` y
`0x00249d60`.

### Lo que se sabe

- `sound_voices_initialize_for_new_structure_bsp` vuelve a calcular en qué
  sitio del nivel está el punto de cada voz de sonido cuando cambia el bsp.
  En el juego lleva dentro el código de `scenario_location_from_point`;
  aquí la llama.
- `sound_console_voice_start` arranca una voz del sonido de la consola con
  lo que dice algo que recibe: su controlador de reproducción y su casilla,
  cuyo flujo se le da. Llama a cinco funciones de la consola que no se sabe
  qué hacen.
- `sound_source_unknown_00221da0` responde algo de un sonido y un oyente:
  verdadero sin oyente; para un sonido con sitio, según si están en la
  misma hoja o cluster del nivel y, si no, según lo que diga `0x00249d60`
  de los dos clusters por un real (256 o más).
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de la que llama a la primera.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Apagar un anillo, lo que dice un sonido en bucle y buscar uno

Tres funciones en `sound/sound_manager.c`. De 812 funciones se pasa a
815, el 7,23 % de las del juego (el 3,891 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_ring_fade_out` | `0x0021be20` | `sound/sound_manager.c` |
| `looping_sound_get_source` | `0x0021bf00` | `sound/sound_manager.c` |
| `looping_sound_find` | `0x0021cf80` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_source_ring_fade_out` apaga en unos segundos los sonidos de un
  anillo (los de una pista de un sonido en bucle) que son del tipo 2, igual
  que la primera mitad de `sound_sources_cross_fade`.
- `looping_sound_get_source` copia lo que el juego dice de un sonido en
  bucle (los `0x44` bytes de `0xC`), si no tiene controlador de
  reproducción o la marca de su controlador está puesta. Si sus dos números
  de `4` y `8` son el mismo salvo dos bits, pone a 0 el tipo de la copia.
  No usa su segundo ni su tercer argumento.
- `looping_sound_find` busca el sonido en bucle que tiene dos números en
  `4` y en `8`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — El anillo de una pista, los efectos cada vez y el estado de la consola

Cinco funciones en `sound/sound_manager.c`. De 807 funciones se pasa a
812, el 7,21 % de las del juego (el 3,872 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `looping_sound_find_source` | `0x0021b870` | `sound/sound_manager.c` |
| `looping_sound_add_source` | `0x0021bc80` | `sound/sound_manager.c` |
| `sound_effects_update` | `0x0021d390` | `sound/sound_manager.c` |
| `sound_console_get_state` | `0x0021f360` | `sound/sound_manager.c` |
| `sound_console_unknown_0021f430` | `0x0021f430` | `sound/sound_manager.c` |

Las dos últimas estaban apuntadas sin descompilar (`unknown_0021f360` y
`unknown_0021f430`). Nuevas sin descompilar, de la consola: `0x0037b822` y
`0x0037b657`.

### Lo que se sabe

- Los sonidos que suenan de cada pista de un sonido en bucle forman un
  anillo: cada uno dice cuál es el siguiente (`0x6C`) y cuál el anterior
  (`0x70`), y el sonido en bucle guarda el primero de cada pista (`0x88`,
  cada `0x14` bytes). `looping_sound_find_source` busca en el anillo el que
  es de un sonido (y de un rango de tono, si se da), y
  `looping_sound_add_source` mete uno detrás del primero.
- `sound_effects_update` es lo que se hace con los efectos de sonido cada
  vez: los que tienen el bit 0 se sueltan, y de los demás, los que diga
  `sound_effect_unknown_0021d5a0` se pasan a `0x0021d630` con un 1.
- `sound_console_get_state` rellena lo que el juego guarda del sonido con
  lo que tiene del sonido de la consola: qué voces son de cada tipo (un
  vector de bits por tipo), qué casillas no tienen una voz sonando y seis
  números.
- `sound_console_unknown_0021f430` pasa a `0x0037b822` cada voz que suena y
  es de un controlador de reproducción; si hubo alguna, llama a
  `0x0037b657`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las dos que las llaman.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Recorrer las tablas del sonido

Cinco funciones en `sound/sound_manager.c` que recorren una tabla entera.
De 802 funciones se pasa a 807, el 7,16 % de las del juego (el
3,844 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_playback_controllers_update` | `0x00219cc0` | `sound/sound_manager.c` |
| `sound_playback_controllers_unknown_00219d90` | `0x00219d90` | `sound/sound_manager.c` |
| `looping_sounds_initialize_for_new_structure_bsp` | `0x0021a100` | `sound/sound_manager.c` |
| `looping_sounds_delete_all` | `0x0021a1e0` | `sound/sound_manager.c` |
| `sound_effects_delete_all` | `0x0021d4d0` | `sound/sound_manager.c` |

Las cinco estaban apuntadas sin descompilar con nombres `unknown_...`.
Nueva sin descompilar: `0x0021f430`.

### Lo que se sabe

- `sound_playback_controllers_update` hace avanzar el número al azar de
  cada controlador de reproducción (lo llama `sound_update` cada fotograma).
- `sound_playback_controllers_unknown_00219d90` es lo que se hace con los
  controladores al activar y al desactivar el sonido: a los que tienen algo
  y la cuenta de `3` a cero, y cuya marca está como toca, se los pasa a
  `0x0021f430`; al desactivar les pone la marca.
- `looping_sounds_initialize_for_new_structure_bsp` vuelve a calcular en qué
  sitio del nivel está cada sonido en bucle que tiene sitio, cuando cambia
  el bsp.
- `looping_sounds_delete_all` y `sound_effects_delete_all` sueltan todos los
  sonidos en bucle y todos los efectos de sonido.
- En el juego estos recorridos van escritos dentro de cada función; aquí
  usan `data_next_index`, y las pruebas les dan tablas de verdad, con su
  vector de bits, en vez de sustituir el recorrido.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las cinco que las llaman.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Fundir dos sonidos, la distancia a un oyente y cuánto se oye a una distancia

Cuatro funciones en `sound/sound_manager.c`. De 798 funciones se pasa a
802, el 7,12 % de las del juego (el 3,812 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_sources_cross_fade` | `0x00126df0` | `sound/sound_manager.c` |
| `sound_source_unknown_00127e20` | `0x00127e20` | `sound/sound_manager.c` |
| `sound_channel_queue_chunk` | `0x00129f20` | `sound/sound_manager.c` |
| `sound_source_distance_gain` | `0x0012ac20` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_sources_cross_fade` hace que un sonido se apague y otro se
  encienda en unos segundos (cualquiera de los dos puede faltar): les pone
  los decibelios de partida, la manera y los tiempos que luego lee
  `sound_source_fade_decibels`.
- `sound_source_unknown_00127e20` da la distancia de un sonido a un oyente
  según su tipo: 0 para el tipo 0; para el tipo 1 (con sitio), el cuadrado
  de la distancia, o si su sitio es una columna (byte `2`), el de la
  distancia en el suelo, y el mayor real posible si el oyente no está a la
  altura de la columna (su alto está en `0x30`). Para los demás tipos, la
  longitud de sus tres reales.
- `sound_channel_queue_chunk` le da a un canal un trozo de muestras de una
  grabación para que lo reproduzca: se lo pasa a su voz de la consola y lo
  pide. Es el primero del canal si no tenía ninguno, o el siguiente.
- `sound_source_distance_gain` dice cuánto se oye un sonido a una
  distancia, de 0 a 1: todo hasta la distancia a la que suena más fuerte, y
  de ahí en adelante menos, hasta la distancia a la que deja de oírse.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Empezar un sonido y el volumen de las clases

Cinco funciones en `sound/sound_manager.c`. De 793 funciones se pasa a
798, el 7,08 % de las del juego (el 3,782 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_new` | `0x00125f70` | `sound/sound_manager.c` |
| `sound_source_hold_chunk` | `0x001268e0` | `sound/sound_manager.c` |
| `sound_channels_unknown_00129380` | `0x00129380` | `sound/sound_manager.c` |
| `sound_console_set_environment` | `0x0021f6d0` | `sound/sound_manager.c` |
| `sound_classes_update` | `0x00221900` | `sound/sound_manager.c` |

Cuatro estaban apuntadas sin descompilar (`unknown_001268e0`,
`unknown_00129380`, `unknown_0021f6d0` y `unknown_00221900`). Nuevas sin
descompilar: `0x00126c30`, `0x00126ec0`, `0x00126000`, `0x00129790` y
`0x0021e3b0`. También el global `sound_class_gains` (`0x00502118`).

### Lo que se sabe

- `sound_source_new` es la entrada para que un sonido empiece a sonar:
  `0x00126c30` y `0x00126ec0` miran lo que el juego dice de él y
  `0x00126000` lo crea. Devuelve el sonido, o `NONE` con el motivo (2 si la
  segunda lo rechaza, 3 si no se pudo crear). Recibe dos de sus argumentos
  en `ebx` y `edi`.
- `sound_source_hold_chunk` hace que un sonido cuente en el trozo de
  muestras en el que está, si no contaba, y pide el trozo. Es la pareja de
  `sound_source_release_chunk`.
- `sound_channels_unknown_00129380` da a cada canal que suena y no está en
  pausa un número de una de dos tablas (la de su casilla o la de su voz de
  la consola), con `0x00129790`.
- `sound_console_set_environment` guarda el sonido de los sitios del mapa
  (dos pares de números y `0x30` bytes) en lo que el juego tiene del sonido
  de la consola, y llama a `0x0021e3b0`.
- `sound_classes_update` hace pasar unos segundos para las 54 clases de
  sonido: cada una tiene adónde va, dónde está y los segundos que le
  quedan para llegar.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las tres que las llaman.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Un número al azar, dos de los sonidos en bucle y un byte de un rango de tono

Cuatro funciones en `sound/sound_manager.c`. De 789 funciones se pasa a
793, el 7,04 % de las del juego (el 3,763 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_unknown_00219880` | `0x00219880` | `sound/sound_manager.c` |
| `looping_sound_unknown_0021a050` | `0x0021a050` | `sound/sound_manager.c` |
| `looping_sound_unknown_0021a080` | `0x0021a080` | `sound/sound_manager.c` |
| `sound_unknown_0021cc30` | `0x0021cc30` | `sound/sound_manager.c` |

También las tres tablas de `0x0044a150`, `0x0044a16c` y `0x0044a188` (siete
números cada una), y `tools/milestone_docs.py`: el script que escribe estas
notas, el `README.md` y el `ESTADO.md` de cada hito.

### Lo que se sabe

- `sound_unknown_00219880` saca un número de cuatro reales y una fracción:
  entre los dos primeros según la fracción, más el tercero, más uno al
  azar entre menos el cuarto y el cuarto.
- `looping_sound_unknown_0021a050` y `looping_sound_unknown_0021a080` miran
  los bytes `0xC` y `0xD` de algo (lo más probable, el estado de un sonido
  en bucle y el siguiente, de siete posibles) con las tres tablas.
- `sound_unknown_0021cc30` da el byte `4` de un rango de tono de un sonido,
  como `sound_pitch_range_get_unknown4`, o `NONE` si hay alguna de dos
  marcas.
- Los nombres dicen la dirección porque no se sabe para qué sirven.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Cuánto dura una grabación

Dos funciones en `sound/sound_manager.c`. De 787 funciones se pasa a
789, el 7,00 % de las del juego (el 3,755 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_permutation_get_maximum_milliseconds` | `0x00218b80` | `sound/sound_manager.c` |
| `sound_get_maximum_milliseconds` | `0x00218ca0` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_permutation_get_maximum_milliseconds` dice cuánto dura como mucho
  una grabación de un sonido, en milésimas de segundo, a partir del número
  que la grabación tiene en `8`: lo divide por las muestras por segundo y
  lo ajusta por el tono con el que se grabó (parámetros del rango de tono,
  bloque en `0x18` de lo que comparten los sonidos, de `0xA` bytes) y por
  lo más grave que pueden ponerla la forma de reproducir el sonido y su
  escala. Un tono va en centésimas de semitono: 1200 son el doble de
  rápido.
- `sound_get_maximum_milliseconds` hace lo mismo para una grabación de un
  rango de tono de un sonido, reproducida a un tono dado.
- Usan las tablas del hito 160, que quedan confirmadas: las muestras por
  segundo y los dos reales de cómo se guardan las muestras.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  dos funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Decibelios entre dos y los de un sonido que se apaga

Tres funciones en `sound/sound_manager.c`. De 784 funciones se pasa a
787, el 6,99 % de las del juego (el 3,743 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_fade_decibels` | `0x0012a810` | `sound/sound_manager.c` |
| `sound_decibels_interpolate` | `0x00219650` | `sound/sound_manager.c` |
| `sound_decibels_fade_in` | `0x002197b0` | `sound/sound_manager.c` |

`sound_source_fade_decibels` estaba apuntada sin descompilar
(`unknown_0012a810`); devuelve un real, no un entero, y las dos funciones
que guardan su resultado ya lo guardan como real. La biblioteca de las
pruebas tiene ahora `sqrtl`.

### Lo que se sabe

- `sound_decibels_interpolate` da unos decibelios entre otros dos, según
  una fracción y de una de cuatro maneras: por los decibelios (0), por las
  ganancias (1), por las ganancias con raíz cuadrada, que al principio va
  rápido hacia la más alta (2), y por las ganancias, suave en los dos
  extremos (3). Con cualquier otra manera da los segundos.
- `sound_decibels_fade_in` da unos decibelios subiendo desde el silencio
  (-64), con la fracción recortada de 0 a 1.
- `sound_source_fade_decibels` dice cuánto se oye un sonido que se está
  apagando o encendiendo, de -64 a 0 decibelios; 0 si no lo hace. Sus dos
  tiempos (`0xB4` y `0xB8`) se cuentan desde ahora la primera vez. Si el
  segundo no es posterior, sube desde el silencio hasta sus decibelios de
  `0xB0`; si lo es, va de ellos a 0. La palabra de `0xAE` elige entre las
  maneras 1 y 2 de `sound_decibels_interpolate`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las dos que llaman a la primera.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Números para el sonido de la consola

Cinco funciones de cálculo en `sound/sound_manager.c`. De 779 funciones se
pasa a 784, el 6,96 % de las del juego (el 3,715 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_gain_to_decibels` | `0x002197f0` | `sound/sound_manager.c` |
| `sound_mixbins_add` | `0x0021e230` | `sound/sound_manager.c` |
| `sound_frequency_to_filter_coefficient` | `0x0021e270` | `sound/sound_manager.c` |
| `sound_unknown_0021e2d0` | `0x0021e2d0` | `sound/sound_manager.c` |
| `sound_unknown_0021e330` | `0x0021e330` | `sound/sound_manager.c` |

`sound_gain_to_decibels` estaba apuntada sin descompilar
(`unknown_002197f0`). Para probarlas, la biblioteca de las pruebas
(`tools/difftest_libc`) tiene ahora `logl` y `log10l`, hechas con las mismas
instrucciones del procesador que usa el juego.

### Lo que se sabe

- `sound_gain_to_decibels` pasa una ganancia de 0 a 1 a decibelios, de -64
  a 0 (20 por el logaritmo en base 10).
- `sound_mixbins_add` añade un par a una lista de la consola que dice
  cuánto suena cada salida: la salida y unos decibelios de -64 a 0 pasados
  a centésimas de decibelio (de -6400 a 0). La lista tiene sitio para ocho
  pares y después la cuenta.
- `sound_frequency_to_filter_coefficient` saca de una frecuencia un número
  de 16 bits para un filtro de la consola: `0x8000` por debajo de 30, 0 por
  encima de 8000, y entre medias el logaritmo de dos veces el seno de la
  parte de media vuelta que es la frecuencia de 48.000, por 5909,28.
- `sound_unknown_0021e2d0` pasa unos decibelios que restan (hasta 22,5) a
  la ganancia que son por 32.768, hasta `0xFFFF`.
- `sound_unknown_0021e330` pasa unos decibelios a la ganancia que son menos
  uno, por 8192; hasta `0x7FFF`, y para los negativos `0xC000` menos el
  número, hasta `0xDFFF`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de `sound_definition_decibels`.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Una marca de un controlador y tres funciones de los efectos de sonido

Cuatro funciones en `sound/sound_manager.c`. De 775 funciones se pasa a
779, el 6,92 % de las del juego (el 3,698 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_playback_controller_unknown_00219ed0` | `0x00219ed0` | `sound/sound_manager.c` |
| `sound_effect_unknown_0021d360` | `0x0021d360` | `sound/sound_manager.c` |
| `sound_effect_unknown_0021d5a0` | `0x0021d5a0` | `sound/sound_manager.c` |
| `sound_effect_source_deleted` | `0x0021da90` | `sound/sound_manager.c` |

La primera estaba apuntada sin descompilar (`unknown_00219ed0`). Nuevas sin
descompilar: `0x0014de10`, `0x0012a9d0` y `0x0021d630`.

### Lo que se sabe

- `sound_playback_controller_unknown_00219ed0` le cambia una marca (el bit
  alto del byte `2`) a un controlador de reproducción, si no la tenía ya.
  Para un sonido con sitio guarda además un real por cada jugador local
  (lo que diga `0x0012a9d0` del jugador y del sitio); para los demás pone
  sus cuatro reales a 0.
- `sound_effect_unknown_0021d360` marca un efecto de sonido (bits 3 y 4), se
  lo pasa a `0x0021d630` con un 2 y le pone el bit 0.
- `sound_effect_unknown_0021d5a0` responde algo de un efecto según su tipo
  (byte `3`): para el tipo 1 pregunta por su sonido a
  `sound_source_unknown_00127f10`.
- `sound_effect_source_deleted` es lo que se le dice a un efecto cuando se
  suelta su sonido: avisa primero a quien el sonido tenía que avisar antes
  de que el efecto ocupara su sitio (lo guarda en `0x88` del sonido, y el
  efecto en `0x84`) y luego hace lo mismo que `sound_effect_unknown_0021d360`.
  Con el motivo 10 (se paran todos) no hace nada.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las que las llaman.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Bytes por segundo, los bits de un rango de tono y un controlador nuevo

Seis funciones en `sound/sound_manager.c`. De 769 funciones se pasa a
775, el 6,88 % de las del juego (el 3,683 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_seconds_to_bytes` | `0x00218b00` | `sound/sound_manager.c` |
| `sound_pitch_range_get_played_bits` | `0x002194c0` | `sound/sound_manager.c` |
| `sound_pitch_range_set_played_bits` | `0x00219560` | `sound/sound_manager.c` |
| `sound_playback_controller_new` | `0x00219a90` | `sound/sound_manager.c` |
| `sound_playback_controller_unknown_00219c30` | `0x00219c30` | `sound/sound_manager.c` |
| `sound_playback_controller_initialize` | `0x00219e90` | `sound/sound_manager.c` |

`sound_seconds_to_bytes` estaba apuntada sin descompilar
(`unknown_00218b00`). Nuevas sin descompilar: `0x001462b0` y `0x00219ed0`.
También las cinco tablas de `0x0044a054` a `0x0044a0a0`.

### Lo que se sabe

- `sound_seconds_to_bytes` dice cuántos bytes de muestras de un sonido se
  reproducen en unos segundos, en bloques enteros. Usa cinco tablas según
  los tres números del tag que dicen cómo se guardan las muestras: las
  muestras por segundo (22.050, 44.100 o 32.000) y dos números por cada
  forma de los bytes `4` (tres formas) y `5` (cinco formas) del tag. Una de
  las formas del byte `5` tiene bloques de 36 bytes.
- `sound_pitch_range_get_played_bits` y `sound_pitch_range_set_played_bits`
  leen y escriben los bits de las grabaciones de un rango de tono (uno por
  grabación, en el bloque de `0x38` de lo que comparten los sonidos; la
  palabra en `6` del rango dice dónde empiezan). Con una grabación o menos,
  o si la clase no deja guardarlos, no hay bits.
- `sound_playback_controller_new` le suma un sonido al controlador de algo,
  y si no lo había lo crea (`datum_new`) y lo prepara con
  `sound_playback_controller_initialize`, que le pone a quién sirve, un
  número de los relojes de la consola (`0x001462b0`) y lo demás a cero.
- `sound_playback_controller_unknown_00219c30` le pasa el controlador de
  algo, si lo tiene, a `0x00219ed0`.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de `sound_channel_resume`, que llama a la primera.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Los decibelios de un sonido y buscar un controlador de reproducción

Siete funciones pequeñas en `sound/sound_manager.c`. De 762 funciones se
pasa a 769, el 6,83 % de las del juego (el 3,659 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `unknown_00218420_set_defaults` | `0x00218420` | `sound/sound_manager.c` |
| `sound_definition_get_minimum_decibels` | `0x00218d70` | `sound/sound_manager.c` |
| `sound_definition_get_maximum_decibels` | `0x00218de0` | `sound/sound_manager.c` |
| `sound_permutation_get_unknown6` | `0x00219430` | `sound/sound_manager.c` |
| `sound_playback_controller_try_and_get` | `0x00219910` | `sound/sound_manager.c` |
| `sound_playback_controller_find` | `0x00219960` | `sound/sound_manager.c` |
| `sound_playback_controller_acquire` | `0x00219a30` | `sound/sound_manager.c` |

Las dos de los decibelios estaban apuntadas sin descompilar
(`unknown_00218d70` y `unknown_00218de0`).

### Lo que se sabe

- `sound_definition_get_minimum_decibels` y
  `sound_definition_get_maximum_decibels`: lo menos y lo más que se oye un
  sonido, en decibelios: lo que dice su escala (byte `0xB` del sonido;
  bloque en `8` de lo que comparten los sonidos, de `0x14` bytes) más lo
  que dice su clase (reales en `0x20` y `0x24`).
- `sound_permutation_get_unknown6` da la palabra en `6` de una grabación
  solo si la clase del sonido no deja guardar lo que se reprodujo.
- `sound_playback_controller_try_and_get` da un controlador de reproducción,
  o `NULL` si el índice no es de ninguno.
- `sound_playback_controller_find` busca el controlador de algo (el que lo
  tiene en `4`); `sound_playback_controller_acquire` además le suma un
  sonido a su cuenta de 7 bits.
- `unknown_00218420_set_defaults` rellena algo de `0x1C` bytes con dos
  reales (120 y 60) y veinte bytes fijos. No se sabe de qué es.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de `sound_definition_decibels`, que llama a dos de ellas.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Diez funciones pequeñas más del sonido

Diez funciones pequeñas, en `sound/sound_manager.c` aunque lo más probable
es que sean de otros archivos del sonido. De 752 funciones se pasa a
762, el 6,76 % de las del juego (el 3,632 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_chunk_get_address` | `0x00218950` | `sound/sound_manager.c` |
| `sound_permutation_unknown6` | `0x00218c60` | `sound/sound_manager.c` |
| `sound_definition_get_maximum_distance` | `0x00218d30` | `sound/sound_manager.c` |
| `sound_pitch_range_set_unknown4` | `0x002192b0` | `sound/sound_manager.c` |
| `sound_pitch_range_set_unknown5` | `0x00219310` | `sound/sound_manager.c` |
| `sound_pitch_range_get_unknown4` | `0x00219370` | `sound/sound_manager.c` |
| `sound_pitch_range_get_unknown5` | `0x002193d0` | `sound/sound_manager.c` |
| `looping_sound_delete` | `0x0021b070` | `sound/sound_manager.c` |
| `sound_voice_unknown_0021dff0` | `0x0021dff0` | `sound/sound_manager.c` |
| `sound_classes_match` | `0x0021e1d0` | `sound/sound_manager.c` |

También el global `sound_cache_base` (`0x00502108`).

### Lo que se sabe

- `sound_cache_chunk_get_address` dice dónde están en la caché de los
  sonidos las muestras de un trozo de una grabación (la primera página de
  su bloque, por el tamaño de página, más `sound_cache_base`), y cuenta que
  se pidieron, hasta 255 veces.
- `sound_permutation_unknown6` da como real la palabra en `6` de una
  grabación de un rango de tono de un sonido.
- `sound_definition_get_maximum_distance`: hasta dónde se oye un sonido: lo
  que diga su clase, si el sonido lo pide (bit 11 de sus marcas), o cómo se
  reproduce el sonido.
- `sound_pitch_range_set_unknown4`/`5` y `sound_pitch_range_get_unknown4`/`5`
  ponen y leen los bytes `4` y `5` de un rango de tono, salvo que la clase
  del sonido no deje guardarlos (bit 5 de sus segundas marcas): entonces no
  se ponen, y al leer sale `NONE`.
- `looping_sound_delete` suelta un sonido en bucle y le quita un sonido a
  su controlador de reproducción.
- `sound_voice_unknown_0021dff0` le da a una voz de sonido `0x24` bytes de
  algo, una vez por cada arranque del reloj del sonido, y dice si los tomó
  y si era la primera vez.
- `sound_classes_match` dice si dos clases de sonido son la misma, o tienen
  el mismo real en `0x10` y reales en `0x50` a menos de 0,0001.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones.

### Mutaciones

Pendientes; corren aparte, en segundo plano.

## 2026-10-04 — Doce funciones pequeñas de otros archivos del sonido

Doce funciones pequeñas, en `sound/sound_manager.c` aunque lo más probable
es que sean de otros archivos del sonido. De 740 funciones se pasa a
752, el 6,68 % de las del juego (el 3,604 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_cache_idle` | `0x00218810` | `sound/sound_manager.c` |
| `sound_permutation_chunks_size` | `0x00218ac0` | `sound/sound_manager.c` |
| `sound_definition_promotion_rule` | `0x00218f10` | `sound/sound_manager.c` |
| `sound_definition_get_custom_playback` | `0x002194a0` | `sound/sound_manager.c` |
| `sound_playback_controller_get` | `0x002198f0` | `sound/sound_manager.c` |
| `sound_playback_controller_add_count` | `0x00219f60` | `sound/sound_manager.c` |
| `looping_sounds_initialize` | `0x0021a0c0` | `sound/sound_manager.c` |
| `sound_effects_initialize` | `0x0021d490` | `sound/sound_manager.c` |
| `sound_effect_delete` | `0x0021dcf0` | `sound/sound_manager.c` |
| `sound_voices_initialize` | `0x0021dda0` | `sound/sound_manager.c` |
| `sound_console_streams_pause` | `0x0021f570` | `sound/sound_manager.c` |
| `sound_console_streams_resume` | `0x0021f5a0` | `sound/sound_manager.c` |

Siete estaban apuntadas sin descompilar con el nombre `unknown_...`
(`0x00218810`, `0x00218ac0`, `0x0021a0c0`, `0x0021d490`, `0x0021dda0`,
`0x0021f570` y `0x0021f5a0`). Nueva sin descompilar: `0x0037b73f`, de la
consola.

### Lo que se sabe

- `sound_cache_idle`: pasa un tick de la caché de los sonidos.
- `sound_permutation_chunks_size`: cuántos bytes suman los primeros trozos
  de muestras de una grabación (30 bits de la palabra en `4` de cada uno).
- `sound_definition_promotion_rule` y `sound_definition_get_custom_playback`
  leen, de lo que comparten los sonidos de un mapa, la promoción de un
  sonido (su byte `0xC`; bloque en `0x48`, de `0x1C` bytes, con reglas de
  `0x10`) y su reproducción propia (byte `0xD`; bloque en `0x30`, de `0x34`).
- `sound_playback_controller_get` da un controlador de reproducción, o el
  de `unknown_0051ebdc` para `NONE`; `sound_playback_controller_add_count`
  le suma algo a su cuenta de 7 bits.
- `looping_sounds_initialize`, `sound_effects_initialize` y
  `sound_voices_initialize` hacen las tablas de los sonidos en bucle (128 de
  `0xD4` bytes), los efectos (16 de `0x14`; el juego la llama «sounds
  effects») y las voces (tantas como diga la configuración, de `0x6C`).
- `sound_effect_delete` suelta un efecto de sonido y le quita un sonido a
  su controlador.
- `sound_console_streams_pause` y `sound_console_streams_resume` paran y
  reanudan los dos flujos de sonido de la consola (`0x0037b73f`).
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las que las llaman.

### Mutaciones

Pendientes; corren aparte, en segundo plano, con las de los hitos 155 y 156.

## 2026-10-04 — Parar los sonidos al leer el estado del juego

Tres funciones de `sound/sound_manager.c`. De 737 funciones se pasa a 740,
el 6,57 % de las del juego (el 3,586 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_reset` | `0x001264c0` | `sound/sound_manager.c` |
| `sound_source_unknown_00127f10` | `0x00127f10` | `sound/sound_manager.c` |
| `sound_voice_release` | `0x0021e050` | `sound/sound_manager.c` |

También el global `unknown_004ed288` y dos funciones apuntadas sin
descompilar: `0x0021f290` y `0x0021a1e0`.

### Lo que se sabe

- `sound_reset` (antes `unknown_001264c0`) suelta los sonidos que suenan y
  pone a cero el reloj del sonido. Es uno de los procedimientos del estado
  del juego (después de leerlo), y `sound_dispose_from_old_map` la llama con
  0. Con el bit 5 de sus marcas algunos sonidos siguen: los de la clase 32
  siempre, los de la clase 38 si además está el bit 6, y los de los tipos 1
  a 4 cuya clase lo diga (su byte `0x4C` a 1); a esos se les pone el tiempo
  de `0x8C` a `NONE`. Sin el bit 5, además, vuelve a activar el sonido si
  estaba desactivado.
- `sound_source_unknown_00127f10` pregunta algo de un sonido a quien el
  sonido dice (la primera función de su tabla, con dos partes del sonido).
  Si responde que no, marca el sonido (bit 5 de sus marcas). Recibe la tabla
  en `edi`: en el hito anterior se había apuntado sin ese argumento, y
  `sound_source_unknown_00127fc0` ya se lo pasa.
- `sound_voice_release`: una voz de sonido tiene uno menos que la usa; sin
  ninguno, suelta su casilla de los globales del sonido y su sitio.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito, solo las pruebas de estas
  funciones y de las que las llaman.

### Mutaciones

- Hito anterior (155): pendiente; corre aparte, en segundo plano.
- Este hito: se comprueba aparte, después de la del 155.

## 2026-10-04 — Lo que sueltan un sonido y un canal

Seis funciones de `sound/sound_manager.c`. De 731 funciones se pasa a 737,
el 6,54 % de las del juego (el 3,564 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_release_chunk` | `0x00126960` | `sound/sound_manager.c` |
| `sound_source_set_sound` | `0x001286b0` | `sound/sound_manager.c` |
| `sound_channel_delete` | `0x0012a5d0` | `sound/sound_manager.c` |
| `sound_playback_controller_release` | `0x00219c80` | `sound/sound_manager.c` |
| `sound_source_unknown_00127fc0` | `0x00127fc0` | `sound/sound_manager.c` |
| `sound_source_get_position` | `0x0012abe0` | `sound/sound_manager.c` |

Las tres primeras de la lista de direcciones (`0x00126960`, `0x0012a5d0` y
`0x00219c80`) estaban apuntadas sin descompilar desde el hito anterior.
Nuevas sin descompilar: `0x002ae4b0`, `0x0021e050` y `0x00127f10`.

### Lo que se sabe

- `sound_source_release_chunk`: un sonido que suena deja de contar en el
  trozo de muestras en el que está (rango de tono, grabación y trozo), si
  contaba (segundo bit de sus marcas).
- `sound_source_set_sound` cambia de qué sonido es una fuente: suelta el
  trozo anterior y pone el sonido, el rango de tono, la grabación y el
  trozo nuevos.
- `sound_channel_delete`: un canal suelta lo que toca: los trozos de sus dos
  grabaciones, su voz de la consola (`0x002ae4b0`, y su bit en el vector de
  `0x60` de los globales) y su voz de sonido (`0x0021e050`), y queda vacío.
- `sound_playback_controller_release` le quita un sonido a un controlador
  de reproducción (una cuenta de 7 bits) y lo borra cuando no le queda
  ninguno.
- `sound_source_unknown_00127fc0` devuelve lo que diga `0x00127f10` para un
  sonido que tiene a quién avisar, cuando no ha llegado su tiempo (`0x8C`) o
  el sonido está desactivado; para el resto, verdadero. No se sabe qué
  significa.
- `sound_source_get_position` da el sitio de un sonido según lo que el
  juego dice de él: su sitio, o su sitio a la altura de un oyente.
- Los nombres son inventados.

### Sin comprobar

- La batería completa no se pasó en este hito: se pasó con las 731
  funciones del anterior (todas bien) y aquí solo las pruebas de las seis
  nuevas y de las dos que las llaman.
- `ninja` (los tamaños de las estructuras) falla en este equipo con el
  clang de Visual Studio: no encuentra `stdlib.h`. Viene de antes.

### Mutaciones

40 líneas rotas a propósito. La comprobación corre aparte, en segundo plano, y su resultado se anota en el hito siguiente.

## 2026-10-04 — Parar un sonido y soltarlo

Cuatro funciones de `sound/sound_manager.c`. De 727 funciones se pasa a 731,
el 6,49 % de las del juego (el 3,541 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_source_stop` | `0x00126360` | `sound/sound_manager.c` |
| `sound_stop_all` | `0x00126400` | `sound/sound_manager.c` |
| `sound_source_delete` | `0x00127320` | `sound/sound_manager.c` |
| `sound_source_free` | `0x00127390` | `sound/sound_manager.c` |

También la estructura `sound_source_callbacks` (a quién se avisa de lo que
le pasa a un sonido; solo se conocen dos de sus funciones) y cuatro
funciones apuntadas sin descompilar: `0x0012a5d0`, `0x00126960`,
`0x00219c80` y `0x0021d4d0`.

### Lo que se sabe

- `sound_source_stop` hace que un sonido que suena empiece a apagarse, igual
  que se apagan todos al soltar un mapa (300 ms), y avisa a quien el sonido
  diga (la función de `0x14` de su tabla). No hace nada si el índice no es
  de un sonido o si el sonido ya se está soltando (su byte `2`).
- `sound_stop_all` suelta de golpe todos los sonidos que suenan, sin
  apagarlos, si el sonido está preparado. Antes llama a `0x0021d4d0`, de los
  efectos de sonido.
- `sound_source_delete` suelta un sonido: su canal (`0x0012a5d0`), lo que
  tiene (`0x00126960`) y su sitio en la tabla, y avisa del motivo a quien
  el sonido diga (la función de `0x10`). `sound_stop_all` da el motivo 10.
- `sound_source_free` suelta el sitio del sonido en la tabla y le quita un
  sonido a su controlador de reproducción (`0x00219c80`); si el byte `3`
  del sonido es `0xFF`, también baja una cuenta de 7 bits del controlador.
- Los nombres son inventados: dicen lo que hacen las funciones.

### Mutaciones

24 líneas rotas a propósito: 23 detectadas. La otra es equivalente: quitar
la comprobación de `NONE` de `sound_source_stop` no cambia nada, porque
`datum_try_and_get` ya la hace.

## 2026-10-03 — Dos funciones pequeñas más del sonido

Dos funciones de `sound/sound_manager.c`. De 725 funciones se pasa a 727,
el 6,45 % de las del juego (el 3,524 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_definition_unknown_00126bd0` | `0x00126bd0` | `sound/sound_manager.c` |
| `sound_set_unknown1f8` | `0x001272c0` | `sound/sound_manager.c` |

También el global `unknown_0047f0e4` y los dos reales de los globales del
sonido en `0x1FC` y `0x204`.

### Lo que se sabe

- `sound_definition_unknown_00126bd0` dice si un sonido es de cierto tipo
  (lo dicen sus bytes quinto y sexto; para el tipo 2, además, hace falta
  `unknown_0047f0e4`) y si el primero de sus rangos de tono tiene
  permutaciones. Qué significa no se sabe.
- `sound_set_unknown1f8` cambia el índice de `0x1F8` de los globales del
  sonido por otro. El que había se guarda en `0x200` y los dos reales se
  ponen a 0; si se vuelve justo al que se guardó, se intercambian y se
  recupera su real. La llama una de las funciones que van por fotograma
  (`0x0018c060`).
- Los nombres dicen lo que hacen, no para qué sirven.

### Mutaciones

13 líneas rotas a propósito: las 13 detectadas.

## 2026-10-03 — Casillas del sonido, una marca de voz y una fracción

Cuatro funciones pequeñas de `sound/sound_manager.c`. De 721 funciones se
pasa a 725, el 6,44 % de las del juego (el 3,518 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_set_voice_flag3` | `0x0012a420` | `sound/sound_manager.c` |
| `sound_slot_new` | `0x0012af10` | `sound/sound_manager.c` |
| `sound_slot_delete` | `0x0012af90` | `sound/sound_manager.c` |
| `sound_interpolation_fraction` | `0x0012aff0` | `sound/sound_manager.c` |

### Lo que se sabe

- `sound_slot_new` toma la primera casilla de los globales del sonido que
  existe (un vector de 64 bits en `0x58`, mirando tantas como dice `0x20A`)
  y que no está tomada (el de `0x70`), la marca y devuelve su número, o
  `NONE`; `sound_slot_delete` la suelta.
- `sound_channel_set_voice_flag3` pone el cuarto bit del cuarto byte de la
  voz de un canal, que dar un trozo a la voz quita.
- `sound_interpolation_fraction` dice dónde está un valor entre un mínimo y
  un máximo, de 0 a 1. Si los dos están a menos de 0,0001 da 0 o 1 según
  si el mínimo pasa del valor y una marca que recibe en `cl`.
- Los nombres son inventados.

### Mutaciones

14 líneas rotas a propósito: las 14 detectadas. Una (dividir con el
inverso) solo se veía con valores que no son redondos, y se añadieron 300.

## 2026-10-03 — Lo que hace el sonido cada fotograma

Una función de `sound/sound_manager.c`, que `main.c` ya llamaba como
`unknown_00125b40`. De 720 funciones se pasa a 721, el 6,40 % de las
del juego (el 3,506 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_update` | `0x00125b40` | `sound/sound_manager.c` |

También el global `sound_lruv_cache` (`0x0050210c`, la caché de las
muestras del sonido) y once funciones sin descompilar a las que llama, que
las pruebas simulan apuntando el orden y lo que reciben.

### Lo que se sabe

- Con el sonido iniciado y un mapa cargado, marca el fotograma y pone en
  marcha sus partes. Si el sonido estaba apagado y el juego no está en
  pausa, lo enciende: reanuda cada canal y apunta la hora.
- Después actualiza sus partes en un orden fijo, entre ellas los canales
  (`sound_channels_update`). Con dos tablas y una estructura en la pila
  (0x1744 bytes en total) prepara los oyentes, uno por cada jugador local,
  y cuenta los fotogramas en que se marcó `0x7E`.
- Siempre pasa un tick de su caché (`lruv_idle`).
- El nombre es inventado.

### Mutaciones

25 líneas rotas a propósito: 24 detectadas. Al principio cuatro se
escapaban por los simuladores: no miraban la marca del fotograma, no
podían descargar el mapa a mitad y no escribían nada en las tablas. Se
arreglaron.

### Sin comprobar

- Un mutante es equivalente: el paso al jugador siguiente nunca recibe
  `NONE`, porque el bucle solo sigue con un jugador.

## 2026-10-03 — Lo fuerte que suena un sonido, en decibelios

Dos funciones de `sound/sound_manager.c`. De 718 funciones se pasa a 720,
el 6,39 % de las del juego (el 3,489 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_definition_decibels` | `0x001251e0` | `sound/sound_manager.c` |
| `sound_decibels_to_gain` | `0x002195f0` | `sound/sound_manager.c` |

### Arreglado

- El arnés no sabía simular una función externa que devuelve un real: lo
  dejaba solo en `eax`. Ahora el simulador da sus 32 bits, y el arnés los
  pone donde la función del juego los devuelve según `symbols/functions.csv`
  (`eax`, `xmm0` o la pila del x87) y, en las de `source/`, en la pila del
  x87, que es donde los deja C. Con esto se pueden probar funciones que
  llaman a otras que devuelven reales, que antes no se podían.

### Lo que se sabe

- `sound_decibels_to_gain` pasa decibelios (de −64 a 0) a una ganancia:
  10 elevado a una vigésima parte de ellos, en el x87 y sin redondear. En C
  hay que impedir que el compilador calcule de antemano, con más
  precisión, los casos de −64 y de 0.
- `sound_definition_decibels` da lo fuerte que suena un sonido en
  decibelios: interpola por un valor entre la ganancia de su volumen menor
  y la de su volumen mayor, la vuelve a pasar a decibelios y le suma los de
  su clase y otros que recibe.
- Los nombres son inventados.

### Mutaciones

15 líneas rotas a propósito: 13 detectadas.

### Sin comprobar

- Dos mutantes son equivalentes: tratar 0 y −0 decibelios igual (los dos
  dan una ganancia de 1), y no redondear la ganancia menor a 32 bits (la
  que sale del x87 ya cabe en ellos: es la suma redondeada a la precisión
  simple, multiplicada por una potencia de 2).

## 2026-10-03 — Lo que hace un canal de sonido cada vez

Tres funciones de `sound/sound_manager.c`; la primera ya se llamaba como
`unknown_0012a450`. De 715 funciones se pasa a 718, el 6,37 % de las
del juego (el 3,481 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_update` | `0x0012a450` | `sound/sound_manager.c` |
| `sound_permutation_next_chunk` | `0x00219290` | `sound/sound_manager.c` |
| `sound_voice_playback_rate` | `0x0021f650` | `sound/sound_manager.c` |

También `exp2l` en la biblioteca de C de las pruebas, que hace en el x87
lo mismo que el juego (`frndint`, `f2xm1` y `fscale`).

### Lo que se sabe

- `sound_channel_update` es lo que se hace con un canal que suena cada vez
  que el sonido recibe tiempo. Mira lo que hace su voz (un número con
  signo de tres bits): si ya no necesita el primer trozo, lo suelta, pasa
  el segundo a primero y pide sus datos; si no, suelta el que tiene.
  Después, si la voz suena, el tiempo del sonido avanza lo que dura el
  tiempo dado por la velocidad de la voz.
- `sound_voice_playback_rate` es la velocidad de una voz: 2 elevado a su
  tono más una cantidad que depende del tipo del sonido, dividido entre
  4096. Se calcula como el juego, en el x87.
- `sound_permutation_next_chunk` da el trozo siguiente de una permutación,
  o `NONE` después del último o sin permutación.
- Una constante de `sound_manager.c` que estaba repetida desde el hito
  anterior se quitó.
- Los nombres son inventados.

### Mutaciones

24 líneas rotas a propósito: 22 detectadas.

### Sin comprobar

- Dos mutantes son equivalentes: poner el estado a 1 en lugar de 0 cuando
  el trozo siguiente no tiene datos da lo mismo, porque luego solo se mira
  si es menor que 2; y usar `log2(e)` con la precisión de un `double` en
  lugar de la del x87 da el mismo producto redondeado a la precisión
  simple del juego, salvo en casos rarísimos que las pruebas no tocan.

## 2026-10-03 — Reanudar un canal de sonido

Una función de `sound/sound_manager.c`, que ya se llamaba como
`unknown_0012a0b0`. De 714 funciones se pasa a 715, el 6,35 % de las
del juego (el 3,464 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_resume` | `0x0012a0b0` | `sound/sound_manager.c` |

También algunos campos más de un canal: un real en `0x10` (lo que el sonido
lleva, en segundos al parecer), el trozo de cada permutación que suena
(`0x18` y `0x1A`) y las dos permutaciones (`0x1C` y `0x20`).

### Lo que se sabe

- Hace seguir el sonido de un canal que `sound_channel_pause` paró. Si el
  canal tiene una permutación que suena, calcula cuántos bytes lleva el
  sonido (`0x00218b00`, con tres bytes de su tipo y el real del canal),
  quita los de los trozos anteriores al que suena (`0x00218ac0`), hace
  empezar la voz desde ahí (`0x0021f5d0`, que toma el número en `edi`) y
  le da el trozo de cada permutación (`0x002ae500`, con la voz en `esi` y
  el trozo en `edi`). Después marca el canal como no parado.
- El nombre es inventado.

### Mutaciones

16 líneas rotas a propósito: las 16 detectadas. Dos (cambiar el orden o el
signo de los bytes del tipo del sonido) solo se veían si esos bytes no son
cero, y las pruebas les dan ahora valores.

## 2026-10-03 — Pausar un canal de sonido

Dos funciones de `sound/sound_manager.c`; la segunda ya se llamaba como
`unknown_0012a060`. De 712 funciones se pasa a 714, el 6,34 % de las
del juego (el 3,455 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channel_pausable` | `0x00129fe0` | `sound/sound_manager.c` |
| `sound_channel_pause` | `0x0012a060` | `sound/sound_manager.c` |

También el número de clases de sonido (54) y la bandera de una clase cuyos
sonidos siguen cuando el juego se pausa (el bit 0 de sus segundas
banderas).

### Lo que se sabe

- `sound_channel_pausable` dice si el sonido de un canal se para cuando el
  juego se pausa: sí si el canal no tiene sonido o si la clase del sonido
  no existe (menor que 0 o de 54 en adelante); si no, lo que diga la
  bandera de su clase.
- `sound_channel_pause` para una voz del sonido de la consola
  (`0x002ae750`, en `0x0051ebe4`, 0x34 bytes por voz) si el canal tiene
  voz, no estaba parado y se puede parar, y lo marca como parado.
- Los nombres son inventados.

### Mutaciones

14 líneas rotas a propósito: las 14 detectadas. Una (el índice de la
fuente de 8 bits) solo se veía con más de 256 fuentes, y las pruebas
tienen ahora 260.

## 2026-10-03 — Los jugadores locales de una partida

Dos funciones de `game/game.c`; la primera ya se llamaba desde
`main_game.c` como `unknown_00138640`. De 710 funciones se pasa a 712, el
6,32 % de las del juego (el 3,449 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_options_controller_compare` | `0x00138600` | `game/game.c` |
| `game_options_setup_local_players` | `0x00138640` | `game/game.c` |

También el global `unknown_004e61cc` (algo de cada uno de los cuatro
mandos), el nombre y otros campos de las opciones de un jugador, y `qsort`
en la biblioteca de C de las pruebas (`tools/difftest_libc`).

### Cambiado

- Smart App Control empezó a bloquear el clang de llvm-mingw (y también el
  LLVM oficial, que no está firmado). Las pruebas se compilan ahora con el
  clang 19 y el ld.lld del componente Clang de Visual Studio Build Tools,
  firmados por Microsoft; toda la batería pasa igual con él.

### Lo que se sabe

- `game_options_setup_local_players` prepara una partida de esta consola
  sola con unos jugadores locales: la consola 0, con identificador de
  ceros, es la de la partida. Cada jugador recibe un mando: el primero de
  los cuatro de `0x004e61cc` que está puesto y libre, o si no el primero
  libre; los mandos se ordenan con `qsort` (por su número, y `NONE` al
  final). Luego cada jugador queda válido, en esta consola, con su
  índice, un identificador `{índice, FF, FF, FF, 0...}`, el nombre vacío
  y, en multijugador, su índice en `0x98` (1 en la campaña).
- `game_options_controller_compare` es el comparador: por el `short` bajo,
  con `NONE` detrás de todo.
- Los nombres son inventados.

### Mutaciones

26 líneas rotas a propósito: 23 detectadas.

### Sin comprobar

- Tres mutantes son equivalentes: ordenar solo los mandos de los jugadores
  (los demás son `NONE`, que ya van al final); no copiar el identificador
  de la consola en cada jugador (es de ceros, como lo deja el `memset`); y
  borrar solo la mitad de `0x5C` (el `memset` de los jugadores ya lo dejó
  a cero).

## 2026-10-03 — Un fotograma del juego

Una función de `game/game.c`, que ya se llamaba desde `main.c` como
`unknown_00137ed0`. De 709 funciones se pasa a 710, el 6,30 % de las del
juego (el 3,434 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_frame` | `0x00137ed0` | `game/game.c` |

También la estructura `display_settings` (`0x00485ac8`, 12 bytes) y los
valores con los que empieza el juego (`display_settings_default`,
`0x0043ed80`), y los globales `unknown_00485b20` y `game_seconds`
(`0x004ba040`, un `double`).

### Cambiado

- `main_frame_vblanks` (`0x00485aca`) no era un global aparte: es el
  `short` en `+2` de `display_settings`, que `0x000128c0` llena entera.
  Ahora es `display_settings.frame_vblanks`, y las pruebas lo dan como
  `display_settings+2`.

### Lo que se sabe

- Es lo que el juego hace en un fotograma de unos segundos, después de sus
  ticks. Si el fotograma duró algo (más de 0; un NaN no), actualiza ocho
  sistemas que van por fotogramas, unos con los segundos y otros sin
  ellos. Si los ajustes de la pantalla no son los del principio, la vuelve
  a preparar con esos. Y guarda los segundos en `0x00485b20`.
- Siempre llama a `0x00176f60` con los segundos y los suma, en coma
  flotante del x87 y en un `double`, a `game_seconds`.
- El nombre es el de Halo 1, que hace algo parecido.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas. Una (que un NaN cuente como
fotograma) solo se veía con un NaN, y se añadió a las pruebas.

## 2026-10-03 — Abrir un archivo esperando

Dos funciones de `cseries/async.c`. De 707 funciones se pasa a 709, el
6,29 % de las del juego (el 3,429 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_reference_get_full_path` | `0x001a1870` | `cseries/async.c` |
| `async_create_file_blocking` | `0x001a0a70` | `cseries/async.c` |

### Lo que se sabe

- `file_reference_get_full_path` da la ruta que abre una referencia de
  archivo: la ruta completa de su sitio (`file_location_get_full_path`),
  partida y vuelta a juntar con su directorio, su nombre y su extensión.
  El directorio de arriba que da `file_path_split` no se usa.
- `async_create_file_blocking` abre un archivo como `async_create_file`,
  con la ruta de la referencia y la prioridad 6, y espera cediendo el
  procesador (`SwitchToThread`) hasta que la tarea dice que ha terminado.
- Los nombres son inventados.

### Mutaciones

14 líneas rotas a propósito: 13 detectadas. Dos no compilaban al
principio (no hay `strcpy`) y se cambiaron por `memcpy`.

### Sin comprobar

- Un mutante es equivalente: no limpiar el búfer de la ruta completa da lo
  mismo, porque `file_location_get_full_path` lo llena hasta el final con
  `strncpy`, que rellena con ceros.

## 2026-10-03 — Un tick del juego

Una función de `game/game.c`, que ya se llamaba como `unknown_00137fe0`. De
706 funciones se pasa a 707, el 6,28 % de las del juego (el 3,421 %
de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_tick` | `0x00137fe0` | `game/game.c` |

También el global `unknown_00510c6c` (un puntero a algo que lleva los
segundos de los ticks y una cuenta atrás) y 31 funciones sin descompilar a
las que llama, que las pruebas simulan apuntando el orden de las llamadas.

### Arreglado

- El arnés calcula lo que una función del juego saca de la pila al volver
  mirando su primer `ret`. `0x002268e0` no tiene uno propio: acaba en una
  llamada de cola (`jmp 0x00227280`) a mitad de su código, y el arnés
  seguía el último salto, que vuelve hacia atrás, y daba con el `ret 4` de
  otra función. Ahora un salto incondicional fuera de una función de
  tamaño conocido lleva a la función que vuelve por ella.

### Lo que se sabe

- Pone la unidad de coma flotante como el resto del juego y prepara en la
  pila una estructura de 0x4048 bytes (por eso llama a la sonda de la
  pila del compilador). Dos funciones sin descompilar la llenan; si la
  segunda dice que hay que actualizar, se actualizan 29 sistemas en un
  orden fijo (uno de ellos es `animation_cache_idle`), la cuenta atrás de
  `0x00510c6c` baja uno sin pasar de 0 (restando sin signo, como el juego)
  y sus segundos suben un tick. Después otra función cierra la estructura
  y, si hubo actualización, el tiempo del juego avanza un tick.
- `0x000683a0` y `0x0014d3a0` reciben la estructura en la pila y la sacan
  ellas mismas al volver (stdcall).
- El nombre es el de Halo 1.

### Mutaciones

17 líneas rotas a propósito: 16 detectadas.

### Sin comprobar

- Un mutante es equivalente: con la cuenta justo en 0, dejarla en 0 con
  `<0` o con `<=0` da lo mismo.

## 2026-10-03 — Un píxel de cada formato en 32 bits

Una función, con la que `bitmaps/bitmaps.c` queda completo. De 705
funciones se pasa a 706, el 6,27 % de las del juego (el 3,411 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitmap_format_to_a8r8g8b8` | `0x00135ca0` | `bitmaps/bitmaps.c` |

También los nombres de los formatos que faltaban (`a1r5g5b5`, y del 18 al
23 los de Halo 2) y el global `global_vector_palette` (`0x00468848`, con el
nombre de Halo 1). Sus 256 colores son del juego y no se copian aquí: lo
que se compile de esto los tiene en negro, y las pruebas les dan valores.

### Lo que se sabe

- Pasa el píxel de un formato a uno de 32 bits (alfa, rojo, verde y azul).
  Las componentes de menos bits se repiten hasta llenar ocho (r5g6b5,
  a1r5g5b5, a4r4g4b4), las de luminancia van a las tres, y p8 se busca en
  la paleta de vectores. El formato 18 da su byte tal cual, v8u8 pone u en
  el rojo y v en el verde, y g8b8 deja el rojo a 0.
- Los formatos de reales (de 32 y de 16 bits) multiplican cada componente
  por 100 antes de llevarla de 0 a 1. Un NaN se queda como NaN, y si es el
  azul se cuela en el bit alto del píxel.
- Los formatos comprimidos y los que no hay dan 0.
- En C las cuentas de bits están escritas de forma legible; se comprobó
  aparte que dan lo mismo que las del juego con los 65.536 valores de 16
  bits.

### Mutaciones

29 líneas rotas a propósito: las 29 detectadas. Una (un NaN que da 0) solo
se veía con un NaN justo en el azul del píxel que se lee, y se pusieron
más NaN en las pruebas.

## 2026-10-03 — El píxel de un bitmap en un punto

Una función de `bitmaps/bitmaps.c`. De 704 funciones se pasa a 705, el
6,26 % de las del juego (el 3,374 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitmap_sample` | `0x001362b0` | `bitmaps/bitmaps.c` |

### Lo que se sabe

- Da el píxel de un bitmap en un punto de 0 a 1 (el más cercano, y el
  bitmap se repite), como uno de 32 bits, con un detalle: 1 o más es el
  mipmap más grande, y menos baja a los pequeños (el índice es el redondeo
  de `(1 - detalle) × mipmaps`). Un bitmap sin píxeles da `NONE`.
- Los lados del mipmap son los de `bitmap_mipmap_dimension`, redondeados
  a un bloque si el bitmap está comprimido. La coordenada se redondea con
  medio píxel menos y se lleva dentro del lado con una máscara si es
  potencia de dos, o con el resto si no.
- Busca dónde empieza el mipmap con la función de su tipo (2D, 3D o cubo)
  y lee el píxel con `bitmap_get_pixel`.
- El nombre es inventado.

### Mutaciones

22 líneas rotas a propósito: 19 detectadas.

### Sin comprobar

- Tres mutantes son equivalentes: con un lado potencia de dos, el resto y
  la máscara dan lo mismo; con un detalle justo de 1 el mipmap es el 0 por
  los dos caminos; y contar también los bitmaps sin mipmaps solo cambia
  algo con un detalle de menos infinito, que da un NaN y un índice
  `0x80000000`, pero del índice solo se usan el byte bajo (al desplazar) y
  los 16 bits bajos (para buscar el mipmap), y son 0. Las pruebas tienen
  ese caso igualmente.

## 2026-10-03 — Leer un píxel de un bitmap

Una función de `bitmaps/bitmaps.c`. De 703 funciones se pasa a 704, el
6,25 % de las del juego (el 3,359 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitmap_get_pixel` | `0x00136140` | `bitmaps/bitmaps.c` |

También el global `bitmap_pixel_out_of_bounds` (`0x0055e72c`). Las pruebas
ponen en lugar del paso de un píxel a 32 bits (`0x00135ca0`, el
`bitmap_format_to_a8r8g8b8` de Halo 1), del índice de un píxel en una
textura con swizzle (`0x000358d0`) y de los tres decodificadores de bloques
DXT (`0x00223ed0`, `0x00224040` y `0x00224080`) unos que dan valores que
dependen de lo que reciben; cada uno toma sus argumentos en registros
distintos.

### Lo que se sabe

- Lee el píxel `(x, y)` de unos píxeles de un ancho como uno de 32 bits.
- En un bitmap comprimido busca el bloque de 4 por 4 que lo tiene (con
  divisiones con signo, como el juego) y se lo pasa al decodificador de su
  formato DXT con la posición dentro del bloque. El bloque tiene que estar
  dentro de un búfer; si no, se usa el primero del búfer y se enciende
  `bitmap_pixel_out_of_bounds`. Un formato comprimido que no es DXT da 0.
- El decodificador deja el píxel donde el juego recibió las banderas: si
  no escribe nada, la función devuelve las banderas.
- En uno sin comprimir, el índice es `y*ancho + x`, o el de la textura con
  swizzle si el bitmap lo tiene.
- El nombre es inventado.

### Mutaciones

24 líneas rotas a propósito: las 24 detectadas. Al principio dos se
escapaban y otras dos solo se «detectaban» porque la prueba fallaba; se
arreglaron los simuladores (números con signo, un bloque fuera de la
memoria y un decodificador que a veces no escribe) y se añadieron píxeles
que empiezan justo al final del búfer.

## 2026-10-03 — Actualizar cada canal de sonido

Una función de `sound/sound_manager.c`, que ya se llamaba como
`unknown_001290d0`. De 702 funciones se pasa a 703, el 6,24 % de las
del juego (el 3,346 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_channels_update` | `0x001290d0` | `sound/sound_manager.c` |

### Lo que se sabe

- Es una de las cosas que hace el sonido cada vez que se le da tiempo:
  llama a `0x0012a450` (sin descompilar) con cada canal, del 0 al número
  de canales que tienen los globales del sonido, y vuelve a mirar ese
  número después de cada uno. Un número negativo no hace nada.
- El nombre es inventado.

### Mutaciones

6 líneas rotas a propósito: las 6 detectadas. Una (el número de canales
sin signo) solo se detectaba porque la prueba fallaba al restar del número
menor; ahora el simulador da la vuelta en 16 bits y la prueba la ve.

## 2026-10-03 — Descargar el escenario y dónde está un punto

Dos funciones de `scenario/scenario.c`, que ya se llamaban desde
`main_game.c` y `sound_manager.c` como `unknown_0011be50` y
`unknown_0011bed0`. De 700 funciones se pasa a 702, el 6,23 % de las del
juego (el 3,345 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `scenario_unload` | `0x0011be50` | `scenario/scenario.c` |
| `scenario_location_from_point` | `0x0011bed0` | `scenario/scenario.c` |

También `struct scenario_location` (la hoja, el cluster y el BSP), como en
Halo 1, y un archivo de pruebas nuevo, `tests/test_scenario.py`.

### Lo que se sabe

- `scenario_unload` descarga las etiquetas del escenario y deja sin nada
  el escenario, su BSP y los globales del juego. Como en Halo 1.
- `scenario_location_from_point` busca en el árbol del BSP cargado
  (`bsp3d_test_point`, desde el nodo 0) la hoja donde está un punto, y su
  cluster en las hojas del BSP (8 bytes cada una). Fuera de las hojas el
  cluster es `NONE`; sin BSP cargado, todo es `NONE`. Como en Halo 1, con
  los argumentos en el orden de Halo 1 (antes estaban al revés en
  `sound_manager.c`).

### Mutaciones

16 líneas rotas a propósito: las 16 detectadas.

## 2026-10-03 — Meter puntos en un rectángulo

Dos funciones de `math/real_math.c`, con las que se llega a 700. De 698
funciones se pasa a 700, el 6,21 % de las del juego (el 3,340 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `real_rectangle2d_enclose_points` | `0x0011f5f0` | `math/real_math.c` |
| `real_rectangle3d_enclose_points` | `0x0011f770` | `math/real_math.c` |

### Lo que se sabe

- Agrandan un rectángulo o una caja lo justo para que quepan unos puntos,
  uno tras otro: cada coordenada menor que el lado de abajo lo baja, y cada
  una mayor que el de arriba lo sube. Un NaN no cambia nada.
- El compilador del juego desenrolló los bucles de cuatro en cuatro; en C
  es un bucle normal.
- Los nombres son los de las herramientas de Blam, a ojo.

### Mutaciones

12 líneas rotas a propósito: las 12 detectadas.

## 2026-10-03 — Cuantizar un punto

Dos funciones de `math/real_math.c`. De 696 funciones se pasa a 698, el
6,20 % de las del juego (el 3,309 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quantize_real_point3d` | `0x0011f4f0` | `math/real_math.c` |
| `dequantize_real_point3d` | `0x0011f580` | `math/real_math.c` |

También el tipo `real_bounds` (el menor y el mayor de un intervalo).

### Lo que se sabe

- `quantize_real_point3d` guarda un punto de una caja como tres enteros de
  unos bits: 0 para el menor de cada eje y todos los bits a 1 para el
  mayor, redondeando al más cercano. Un punto fuera de la caja vale lo que
  el punto de la caja más cercano. Con una caja vacía en un eje sale
  `0x80000000` (0 entre 0).
- `dequantize_real_point3d` hace lo contrario: 0 es el menor, todos los
  bits o más son el mayor, y entre medias la media ponderada de los dos.
- Los nombres son inventados.

### Mutaciones

15 líneas rotas a propósito: 13 detectadas.

### Sin comprobar

- Dos mutantes son equivalentes: si el valor es justo el mayor de la caja,
  da el mayor tanto si se lleva a él como si no; y sumar los dos términos
  de la media al revés da lo mismo.

## 2026-10-03 — Llevar un cuerpo «keyframed» a un sitio

Una función de `physics/havok_component.c`. De 695 funciones se pasa a 696,
el 6,18 % de las del juego (el 3,301 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `havok_component_set_rigid_body_keyframe` | `0x001d0ee0` | `physics/havok_component.c` |

Las pruebas ponen en lugar del cuaternión de una rotación de Havok
(`0x002da2d0`) uno que da los primeros bytes de la rotación cambiados, y
en lugar de `hkKeyFrameUtility::applyHardKeyFrame` (`0x002db880`) uno que
apunta lo que recibe.

### Lo que se sabe

- Hace que un cuerpo rígido «keyframed» se mueva y gire para estar donde
  dice una matriz al cabo de un tick: pasa la matriz a una transformación
  de Havok (desde la del componente si hace falta), saca el cuaternión de
  su rotación y llama a `applyHardKeyFrame` de Havok con la inversa del
  tiempo, que son los ticks por segundo del juego.
- Pasar una matriz a una transformación y llevarla a la de los cuerpos del
  componente son ahora dos pasos que comparten las funciones que ponen un
  cuerpo en un sitio.
- El nombre es inventado.

### Mutaciones

12 líneas rotas a propósito: las 12 detectadas.

## 2026-10-03 — Poner un cuerpo rígido en un sitio

Tres funciones de `physics/havok_component.c`. De 692 funciones se pasa a
695, el 6,17 % de las del juego (el 3,291 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `havok_component_set_rigid_body_matrix` | `0x001d0c20` | `physics/havok_component.c` |
| `havok_component_set_rigid_body_transform` | `0x001d0cf0` | `physics/havok_component.c` |
| `havok_rigid_body_set_transform` | `0x001d8e80` | `physics/havok_component.c` |

También el método `0x4C` de lo que mueve un cuerpo (ponerlo en un sitio),
el mundo de Havok del cuerpo (en `0x08`, parece) y otra marca del
componente (el bit 5). Las pruebas ponen en lugar de la inversa de Havok
(`0x002da3f0`) una que da los bits de la otra transformación cambiados, y
en lugar de `0x001d1260` una que intercambia los cuerpos de Havok de los
dos primeros cuerpos rígidos, para ver que el juego vuelve a buscar el
suyo después.

### Lo que se sabe

- `havok_rigid_body_set_transform` despierta el cuerpo de Havok y lo pone
  en la transformación que se le da con el método `0x4C`, salvo si es fijo
  y está en un mundo. Está lejos de las demás (`0x001d8e80`): puede ser de
  otro archivo.
- `havok_component_set_rigid_body_transform` hace eso con un cuerpo
  rígido del componente; si los cuerpos van desde la transformación del
  componente, antes multiplica por su inversa. Un cuerpo fijo de un
  componente con el bit 5 se pone en su sitio entre dos funciones sin
  descompilar (`0x001d1260` y `0x001d1540`), buscándolo otra vez después
  de la primera.
- `havok_component_set_rigid_body_matrix` pasa una matriz a una
  transformación de Havok, sin mirar la escala, con los cuartos reales a
  0.
- Los nombres son inventados.

### Mutaciones

22 líneas rotas a propósito: las 22 detectadas.

## 2026-10-03 — Fijar las velocidades de un cuerpo rígido

Tres funciones de `physics/havok_component.c`. De 689 funciones se pasa a
692, el 6,14 % de las del juego (el 3,275 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `havok_component_get_rigid_body_point_velocity` | `0x001d0a20` | `physics/havok_component.c` |
| `havok_component_set_rigid_body_linear_velocity` | `0x001d0dd0` | `physics/havok_component.c` |
| `havok_component_set_rigid_body_angular_velocity` | `0x001d0e50` | `physics/havok_component.c` |

También el vector de Havok (`hkVector4`), tres métodos más de lo que mueve
un cuerpo (`0x50`, `0x54` y `0x58`), la isla de simulación del cuerpo de
Havok (`0x44`) y otra marca del componente (el bit 1). Las pruebas ponen
en lugar de `hkEntity_isActive` (`0x0030bc40`) uno que mira la isla como
la de Havok, y en lugar de `hkEntity_activate` (`0x0030bc60`) uno que solo
apunta la llamada; los tres métodos nuevos son mocks que comparan lo que
hay en el vector que reciben.

### Lo que se sabe

- La velocidad de un punto que se mueve con el cuerpo la da el método
  `0x58` de lo que lo mueve, con el punto como vector de cuatro reales y
  el cuarto a 0; un cuerpo fijo da 0.
- Fijar la velocidad lineal o angular despierta antes el cuerpo, si
  duerme y está en el mundo de Havok, y llama al método `0x50` o `0x54`. A
  un cuerpo fijo no se le hace nada, y la angular tampoco se fija si el
  componente tiene el bit 1, que parece decir que sus cuerpos no giran.
- Los nombres son inventados.

### Mutaciones

21 líneas rotas a propósito: las 21 detectadas.

## 2026-10-03 — Dónde está un cuerpo rígido

Dos funciones de `physics/havok_component.c`. De 687 funciones se pasa a
689, el 6,12 % de las del juego (el 3,260 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `havok_component_get_rigid_body_position` | `0x001d0870` | `physics/havok_component.c` |
| `havok_component_get_rigid_body_matrix` | `0x001d08e0` | `physics/havok_component.c` |

También la transformación de Havok (`hkTransform`: tres columnas de
rotación y una traslación, de cuatro reales cada una), la que tiene lo que
mueve un cuerpo (en `0x80`) y la del componente (en `0x30`), con una marca
del componente (el bit 6 de `0x04`). Las pruebas ponen en lugar del
producto de transformaciones de Havok (`0x002da5d0`) uno que mezcla los
bits de las dos.

### Lo que se sabe

- La matriz de un cuerpo rígido es su transformación de Havok, con escala
  1; si el componente tiene la marca, antes se multiplica por la del
  componente.
- La posición, con la marca, sale de esa matriz; sin ella, es la
  traslación de la transformación del cuerpo.
- Los nombres son inventados.

### Mutaciones

16 líneas rotas a propósito: 15 detectadas.

### Sin comprobar

- Un mutante es equivalente: sacar la posición siempre de la matriz da lo
  mismo cuando el componente no tiene la marca, porque entonces la matriz
  no se multiplica por nada.

## 2026-10-03 — La rotación, la masa y el tipo de un cuerpo rígido

Tres funciones de `physics/havok_component.c`. De 684 funciones se pasa a
687, el 6,10 % de las del juego (el 3,249 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `havok_component_get_rigid_body_rotation` | `0x001d0b20` | `physics/havok_component.c` |
| `havok_component_get_rigid_body_mass` | `0x001d0bb0` | `physics/havok_component.c` |
| `havok_component_rigid_body_is_keyframed` | `0x001d0c00` | `physics/havok_component.c` |

También la tabla de métodos de lo que mueve un cuerpo de Havok (solo los
dos que se usan, en `0x18` y `0x24`), la inversa de su masa (`0x2C`) y la
matriz de rotación de Havok (tres columnas de cuatro reales, alineada a
16 bytes). Las pruebas ponen los dos métodos en la memoria de los mocks.

### Arreglado

- El arnés no sabía leer un infinito o un NaN devuelto en la pila del x87:
  calculaba una potencia de 2 enorme y Python fallaba. Ahora los
  reconoce, y del NaN guarda lo que guardaría un real de 32 bits.

### Lo que se sabe

- La rotación la pide al método `0x24` de lo que mueve el cuerpo y copia
  los tres primeros reales de cada columna.
- La masa es la inversa de lo que guarda Havok, pero nunca menor que 1; un
  cuerpo con inversa 0 (que nada mueve) da 1, y un NaN sigue siendo NaN.
- Un cuerpo es «keyframed» (lo mueve el juego, no la física) si el método
  `0x18` dice el tipo 6, el `MOTION_KEYFRAMED` de Havok.
- Los nombres son inventados.

### Mutaciones

17 líneas rotas a propósito: 15 detectadas.

### Sin comprobar

- Dos mutantes son equivalentes: escribir `mass<1.0f` en lugar de
  `1.0f>mass` es la misma comparación, y con `>=` una masa justo de 1 da 1
  por los dos caminos.

## 2026-10-03 — Las velocidades de un objeto

Tres funciones, dos de ellas de un archivo nuevo,
`physics/havok_component.c`. De 681 funciones se pasa a 684, el 6,07 %
de las del juego (el 3,241 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_velocities` | `0x000ba1d0` | `objects/objects.c` |
| `havok_component_get_rigid_body_linear_velocity` | `0x001d09d0` | `physics/havok_component.c` |
| `havok_component_get_rigid_body_angular_velocity` | `0x001d0ad0` | `physics/havok_component.c` |

También el global `global_zero_vector3d` (`0x004687a4`, el nombre de Halo
1), las velocidades de los objetos (`0x88` y `0x94`) y una marca suya en
`0xC1`. Lo que se sabe de Havok (el componente, sus cuerpos rígidos, el
cuerpo rígido de Havok y lo que lo mueve) pasa de `objects.h` a
`physics/havok_component.h`, con estructuras en lugar de desplazamientos.

### Lo que se sabe

- `object_get_velocities` da lo rápido que se mueve y gira el objeto del
  que cuelga todo; cualquiera de las dos puede pedirse con `NULL`. Si
  tiene un componente de Havok con su cuerpo rígido principal, son las de
  Havok, o 0 si el objeto tiene la marca de fijo; si no, las del propio
  objeto. El nombre es el de Halo 1.
- Las dos de Havok leen las velocidades de lo que mueve el cuerpo rígido,
  o dan 0 si el cuerpo es fijo (un byte en `0x40`).
- `object_get_center_of_mass` usa ahora el mismo paso para encontrar el
  cuerpo rígido principal.

### Mutaciones

26 líneas rotas a propósito: las 26 detectadas (una, cambiar el orden de
las velocidades del objeto, la para al compilar la comprobación de su
desplazamiento).

## 2026-10-03 — El centro de masa de un objeto

Una función. De 680 funciones se pasa a 681, el 6,05 % de las del juego
(el 3,226 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_center_of_mass` | `0x000b9ef0` | `objects/objects.c` |

También el global `havok_component_data` (`0x0051e9b8`, la tabla de datos
«havok components»), el campo `havok_component_index` de los objetos (en
`0xB4`) y lo poco que se usa de un componente de Havok y de sus cuerpos
rígidos. Las pruebas de objetos pueden hacer ahora un mundo con
componentes de Havok; sin ellos sale igual que antes.

### Lo que se sabe

- Sube hasta el objeto del que cuelga todo (`object_get_ultimate_parent`)
  y mira su componente de Havok. Si tiene uno y su cuerpo rígido
  principal existe (un índice con signo de 8 bits, entre 0 y el número de
  cuerpos), devuelve el centro de masa que Havok tiene de ese cuerpo.
- Si no, devuelve el origen del objeto pedido, no el del de arriba
  (`object_get_origin`).
- No mira la sal del índice del componente.
- El nombre es inventado.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas (dos, mover campos de las
estructuras, las paran las comprobaciones de tamaño al compilar). Dos se
escapaban al principio: con pocos componentes y pocos cuerpos, un índice
de 8 bits y uno de 16, o uno con signo y otro sin él, daban lo mismo. Las
pruebas tienen ahora 300 componentes a veces y 256 cuerpos.

## 2026-10-03 — Empezar y terminar el dibujo de depuración

Tres funciones de un archivo nuevo, `rasterizer/rasterizer_debug.c`. De 677
funciones se pasa a 680, el 6,04 % de las del juego (el 3,219 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_debug_immediate_end` | `0x0001ee50` | `rasterizer/rasterizer_debug.c` |
| `rasterizer_debug_primitive_begin` | `0x0001ee60` | `rasterizer/rasterizer_debug.c` |
| `rasterizer_debug_primitive_end` | `0x0001ef70` | `rasterizer/rasterizer_debug.c` |

También los globales `rasterizer_debug_immediate_active` (`0x004b6294`),
`rasterizer_debug_primitive_type` (`0x004b6298`) y `d3d_push_buffer_size`
(`0x00408648`, de Direct3D), y los tres primeros campos del dispositivo de
Direct3D. Las pruebas ponen en lugar de las funciones de Direct3D
`D3D_MakeRequestedSpace`, `D3DDevice_SetRenderState_LineWidth`,
`D3D_SetFence` y `D3D_LazySetState` unas que solo apuntan las llamadas; la
última también escribe en el búfer de órdenes, para ver que el juego lo lee
después.

### Lo que se sabe

- Son las líneas y triángulos que se dibujan para ver cómo funciona el
  juego. El juego no llama a `Begin` y `End` de Direct3D: escribe él mismo
  la orden de la gráfica de la consola (NV2A, método `0x17FC`) en el búfer
  de órdenes de Direct3D, y pide sitio cuando no queda.
- Los tipos 1 a 4 son líneas, tira de líneas, triángulos y cuadriláteros.
  Empezar un tipo que no es el que se dibuja termina antes el anterior y
  pone las líneas de 1 de ancho; pedir el mismo no hace nada.
- Al terminar se quitan dos marcas del dispositivo, y si estaba la de
  `0x1000` se pone una «valla» (`D3D_SetFence(1)`), como hace `End`.
- `0x0001ee50` termina lo que se dibuja y apaga la marca que encienden
  `0x0001e930` y `0x0001eb00`, que están por descompilar.
- Los nombres son inventados, a partir de los de Halo 1
  (`rasterizer_xbox_debug.c`).

### Mutaciones

32 líneas rotas a propósito: las 32 detectadas.

## 2026-10-03 — Girar un vector hasta otro

Una función, con la que `matrix_math.c` queda completo. De 676 funciones se
pasa a 677, el 6,01 % de las del juego (el 3,214 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `matrix4x3_rotation_between_vectors` | `0x001417b0` | `math/matrix_math.c` |

Y los globales `global_identity_quaternion` (`0x004687cc`) y
`global_identity4x3` (`0x004687d0`), con los nombres de Halo 1. Las pruebas
ponen en lugar de `valid_real_vector3d_axes3` (`0x00143120`, sin
descompilar) una que mira de verdad los vectores, una que siempre dice que
sí y otra que siempre dice que no.

### Lo que se sabe

- Hace la matriz que gira la dirección de un vector hasta la de otro por el
  camino más corto: saca el cuaternión con el coseno y el seno de medio
  ángulo y el producto vectorial, y gira con él los tres vectores de la
  matriz identidad.
- Si los vectores van en la misma dirección no gira; si van en direcciones
  opuestas da media vuelta alrededor de un vector perpendicular al primero
  (`perpendicular3d`), que no se hace de longitud 1.
- Cada vector de la matriz suma sus términos en un orden distinto (el
  compilador del juego los reordenó), y el código lo copia término a
  término para que salga lo mismo bit a bit.
- Si los vectores no salen de longitud 1 y en ángulo recto, la rotación se
  rehace desde `up` y `forward` con productos vectoriales.
- El nombre es inventado.

### Mutaciones

43 líneas rotas a propósito: las 43 detectadas. Dos primeras versiones no
se detectaban porque no cambiaban nada (sumar dos términos al revés y
multiplicar por 0,5 antes o después de restar dan lo mismo) y se cambiaron
por otras que sí cambian el resultado; otra no compilaba y se rehízo.

## 2026-10-03 — Pedir otro BSP

Una función. De 675 funciones se pasa a 676, el 6,00 % de las del juego
(el 3,169 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_switch_structure_bsp` | `0x0012b850` | `main/main.c` |

### Lo que se sabe

- Pide que se cargue otro BSP del escenario al empezar el fotograma
  siguiente (lo hace `main_switch_structure_bsp_private`), y en la campaña
  avisa al primer jugador de la consola con el texto «cargando»
  (`0x110006A2`).
- Pedir el BSP que **ya está cargado** deshace lo pedido antes y avisa de
  que la carga ha terminado (`0x0F0006A3`); si no se había pedido nada, no
  hace nada.
- Un índice negativo o que no es del escenario no hace nada.
- El nombre es el de Halo 1.

### Mutaciones

13 líneas rotas a propósito: las 13 detectadas.

## 2026-10-03 — Pedir que se guarde la partida

Una función. De 674 funciones se pasa a 675, el 5,99 % de las del juego
(el 3,160 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_save_map` | `0x0012b790` | `main/main.c` |

### Lo que se sabe

- Pide que la partida se guarde al empezar el fotograma siguiente (lo hace
  `main_save_map_private`). En la campaña, además, **suena un sonido** (el
  que dice la estructura de los textos, en `+0x394`, a volumen 1) y se
  muestra a **cada jugador** de la consola el texto de «autoguardado»
  (`0x120006A0`).
- No mira si la estructura de los textos existe antes de leer el sonido: en
  el juego siempre está.
- El nombre es el de Halo 1.

### Mutaciones

10 líneas rotas a propósito: las 10 detectadas.

## 2026-10-03 — Las aristas de una caja

Una función. De 673 funciones se pasa a 674, el 5,98 % de las del juego
(el 3,153 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rectangle3d_build_edges` | `0x0011fa40` | `math/real_math.c` |

### Lo que se sabe

- Da las **12 aristas** de una caja como sus dos extremos (24 puntos): las
  cuatro de abajo, las cuatro que suben y las cuatro de arriba. Saca las
  esquinas con `rectangle3d_build_vertices` y devuelve 12.
- Sirve, probablemente, para dibujar cajas de depuración con líneas. El
  nombre es una suposición.

### Mutaciones

7 líneas rotas a propósito: las 7 detectadas.

## 2026-10-02 — El camino de un cuaternión a otro

Una función. De 672 funciones se pasa a 673, el 5,97 % de las del juego
(el 3,134 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quaternion_slerp` | `0x0011da10` | `math/real_math.c` |

### Lo que se sabe

- Es la **interpolación esférica** entre dos cuaterniones: la parte `t` del
  camino de uno a otro por la esfera. Si están a más de media vuelta, va
  por el lado corto. Si son casi iguales (u opuestos) y el seno del ángulo
  es menor que 0,0001, da el más cercano de los dos según `t` redondeado.
- El juego hace el arcocoseno y los senos en la **x87** y los deja ahí entre
  un paso y otro. Las funciones trascendentes de la x87 (`fpatan`, `fsin`)
  no redondean a la precisión de la palabra de control, así que el ángulo y
  el seno quedan con 64 bits de mantisa, y solo las multiplicaciones y la
  división se redondean a precisión simple. Para dar los mismos bits, el
  código usa `long double` en esos pasos.
- El nombre es una suposición.

### Herramientas

- `acosl`, `sinl` y `fabsl` en `tools/difftest_libc`, con la x87.

### Mutaciones

22 líneas rotas a propósito: 19 detectadas. Una pidió cuaterniones
perpendiculares (producto escalar exactamente 0).

### Sin comprobar

- Dos mutaciones son equivalentes: cambiar el orden de una suma de dos
  términos (es conmutativa) y tratar `t` redondeado a 1 por la otra rama
  (da lo mismo).
- Redondear a real el seno de `t` por el ángulo antes de multiplicarlo no
  ha cambiado ningún resultado en 7.200 casos: el doble redondeo casi nunca
  se nota. El código sigue al juego igualmente.

## 2026-10-02 — Tres vectores perpendiculares a partir de una dirección

Una función. De 671 funciones se pasa a 672, el 5,97 % de las del juego
(el 3,121 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `orthonormal_basis3d` | `0x0011d180` | `math/real_math.c` |

También los globales `global_up3d` (`0x004687b0`) y `global_down3d`
(`0x004687bc`), como en Halo 1.

### Lo que se sabe

- A partir de una dirección y de un vector que es más o menos «arriba», da
  tres vectores de longitud 1 perpendiculares entre sí: **adelante** (la
  dirección, o el eje adelante si no tiene longitud), **izquierda**
  (perpendicular a adelante y a la pista de arriba) y **arriba**.
- Si la pista está en la misma recta que la dirección, se usa el eje arriba;
  y si la dirección es (casi) arriba o abajo, el eje adelante. «Casi» quiere
  decir que cada componente se separa menos de 0,0001.
- El nombre es una suposición.

### Mutaciones

20 líneas rotas a propósito: las 20 detectadas. Una pidió casos en los que
una componente se separa exactamente 0,0001.

## 2026-10-02 — Una orientación después de otra

Una función. De 670 funciones se pasa a 671, el 5,96 % de las del juego
(el 3,102 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `orientation_multiply` | `0x0011dbb0` | `math/real_math.c` |

### Lo que se sabe

- Compone dos orientaciones: primero `b` y luego `a`. Los giros se
  multiplican (`quaternion_multiply`), la posición de `b` se gira con el
  giro de `a`, se escala con la escala de `a` y se le suma la posición de
  `a`, y las escalas se multiplican.
- Con escala 1 no se multiplica nada (la escala del resultado es la de `b`).
- El resultado puede ser una de las dos: entonces esa se copia antes a un
  sitio aparte (si son las dos, se copian al mismo, que es lo mismo).
- Es la pareja de `orientation_inverse`. El nombre es una suposición.

### Mutaciones

19 líneas rotas a propósito: 18 detectadas.

### Sin comprobar

- Una mutación es equivalente: no copiar `b` cuando el resultado es `b`.
  Todo lo que se lee de `b` se lee antes de escribirlo en el resultado.

## 2026-10-02 — Hito 120: deshacer una orientación

Una función. De 669 funciones se pasa a 670, el 5,95 % de las del juego
(el 3,087 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `orientation_inverse` | `0x0011dd80` | `math/real_math.c` |

### Lo que se sabe

- Una **orientación** es un giro (cuaternión), una posición y una escala.
  Esta función da la que la deshace: el giro al revés (cambiando el signo de
  `w`), la posición girada con ese giro, dividida por la escala y cambiada
  de signo, y la escala inversa.
- Una orientación de escala 0 se deshace con escala **10000**, en vez de
  dividir por cero.
- El giro de la posición usa la fórmula `2(u·t)u + (2w² - 1)t + 2w(u×t)`,
  en un orden de sumas concreto que hay que respetar para dar los mismos
  bits.
- Funciona aunque el resultado y la orientación estén en el mismo sitio.
- El nombre es una suposición.

### Mutaciones

18 líneas rotas a propósito: 17 detectadas.

### Sin comprobar

- Una mutación es equivalente: no tratar aparte la escala 1 (1/1 es 1).

## 2026-10-02 — Limitar el ángulo entre dos vectores

Una función. De 668 funciones se pasa a 669, el 5,94 % de las del juego
(el 3,073 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `limit_angle_between_vectors3d` | `0x0011d3b0` | `math/real_math.c` |

### Lo que se sabe

- Recibe dos vectores de longitud 1 y un ángulo (su seno y su coseno). Si
  los vectores no se separan más que el ángulo, devuelve el primero (y
  0). Si no, gira el segundo ese ángulo alrededor de la recta perpendicular
  a los dos, con la **fórmula de Rodrigues**, y devuelve 1.
- Si los dos vectores están en la misma recta, la perpendicular es
  cualquiera perpendicular al segundo (`perpendicular3d`).
- Sirve para que algo gire hacia una dirección sin pasar de un ángulo por
  paso. El nombre es una suposición.

### Mutaciones

21 líneas rotas a propósito: 20 detectadas.

### Sin comprobar

- Una mutación solo cambia algo cuando el producto escalar no es un número;
  como en otras funciones, los NaN no se prueban.

## 2026-10-02 — La distancia de un punto a un segmento

Una función. De 667 funciones se pasa a 668, el 5,93 % de las del juego
(el 3,057 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `point_to_segment_distance_squared3d` | `0x0011e000` | `math/real_math.c` |

### Lo que se sabe

- Devuelve la distancia **al cuadrado** de un punto a un segmento, dado
  como un punto de inicio y un vector. Proyecta el punto sobre la recta,
  recorta la proyección entre el inicio y el final del segmento y mide desde
  ahí.
- Un segmento cuya longitud al cuadrado no pasa de 0,0001 se trata como su
  punto de inicio.
- El nombre es una suposición.

### Mutaciones

15 líneas rotas a propósito: 14 detectadas.

### Sin comprobar

- Una mutación es equivalente: cambiar `0 - t` por `-t` solo cambia el signo
  de un cero, que desaparece al elevar al cuadrado.

## 2026-10-02 — Un real de un rango como un byte

Una función. De 666 funciones se pasa a 667, el 5,92 % de las del juego
(el 3,048 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quantize_real_to_byte` | `0x0011f470` | `math/real_math.c` |

### Lo que se sabe

- Convierte un real de un rango en un byte: 0 para el mínimo, 255 para el
  máximo. Primero calcula `(valor - mínimo) / rango × 255` truncando, y
  luego **baja** mientras el real que representa ese byte quede por encima
  del valor: el resultado es el mayor byte que no se pasa. El 255 representa
  el máximo exacto.
- Con un rango de nada (menos de 0,0001) devuelve 0. Como el byte se queda
  con los 8 bits bajos del entero, un valor fuera del rango da un byte que
  no tiene que ver con él.
- El nombre es una suposición.

### Herramientas

- El arnés compara funciones que devuelven un byte (tipo `byte` en
  `symbols/functions.csv`): solo los 8 bits bajos de `eax`.

### Mutaciones

13 líneas rotas a propósito: 11 detectadas. Una pidió valores iguales al
máximo en rangos cuyo inicio y tamaño no suman exactamente el máximo.

### Sin comprobar

- Dos mutaciones son equivalentes: tratar distinto un rango o un valor que
  no son un número no cambia nada, porque entonces el byte sale 0 y no se
  entra en el bucle.

## 2026-10-02 — El ángulo entre dos vectores y un vector «arriba»

Dos funciones. De 664 funciones se pasa a 666, el 5,91 % de las del
juego (el 3,044 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `angle_between_vectors3d` | `0x0011cf50` | `math/real_math.c` |
| `generate_up_vector3d` | `0x0011d090` | `math/real_math.c` |

También el global `global_forward3d` (`0x004687a8`): el puntero al vector
(1, 0, 0). Junto a él están los de los otros cinco ejes y el vector cero,
como en Halo 1.

### Lo que se sabe

- `angle_between_vectors3d`: el arcocoseno del producto escalar, recortado
  entre -1 y 1. Dos vectores **iguales bit a bit** dan 0 sin calcular nada.
- Usa el `acos` de la biblioteca del juego, que calcula en la x87
  `atan2(sqrt((1+x)(1-x)), x)` con la precisión que tenga la FPU; la libc
  de las pruebas tiene ahora un `acosf` que hace lo mismo.
- `generate_up_vector3d`: un vector «arriba» para algo que mira en una
  dirección: perpendicular a la dirección y al lado (que es horizontal). Si
  la dirección es vertical, el lado es «adelante».
- Los dos nombres son suposiciones.

### Herramientas

- `acosf` en `tools/difftest_libc`.

### Mutaciones

19 líneas rotas a propósito: 18 detectadas. Tres pidieron casos con ceros
con signo y comparar la salida bit a bit; otra, dos vectores que solo
difieren en k.

### Sin comprobar

- Con NaN: el `acos` del juego entra en su manejo de errores (que el arnés no
  tiene preparado), y en `generate_up_vector3d` qué NaN sale depende del
  orden de los operandos de las multiplicaciones, que en C no se controla.
  Por eso una mutación (multiplicar o no la tercera componente por la
  escala) no se puede ver: solo cambia algo con NaN.

## 2026-10-02 — Crear los jugadores y ganar la partida

Dos funciones. De 662 funciones se pasa a 664, el 5,89 % de las del
juego (el 3,031 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_create_players` | `0x00137e40` | `game/game.c` |
| `game_won` | `0x001388e0` | `game/game.c` |

`main_game_start` ya llamaba a la primera con un nombre provisional.

### Lo que se sabe

- `game_create_players`: al empezar una partida, se le dicen a la red las
  **consolas** que están en ella y si esta es una de ellas, y se crea cada
  **jugador** válido de las opciones (hasta 16). Después vienen dos pasos
  más que aún no conocemos. Un indicador global la salta por completo.
- `game_won` (nombre de Halo 1): en la **campaña**, si las opciones tienen
  un cierto valor (`+0x18`), se avisa a cuatro cosas con la **dificultad**,
  se muestra «guardando» y se pasa al **mapa siguiente**; en
  **multijugador**, la partida termina.

### Mutaciones

22 líneas rotas a propósito: las 22 detectadas.

## 2026-10-02 — Archivos dañados y el mensaje de «guardando»

Dos funciones. De 660 funciones se pasa a 662, el 5,88 % de las del
juego (el 3,022 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_files_damaged` | `0x0012bf00` | `main/main.c` |
| `main_print_saving_message` | `0x0012b980` | `main/main.c` |

`main_game.c` ya llamaba a la primera con un nombre provisional, y
`fonts_initialize` la tenía copiada dentro: ahora las dos la llaman.

### Lo que se sabe

- `main_files_damaged` es lo que hace el juego cuando encuentra **archivos
  dañados**: cuando ni siquiera puede abrir el menú principal, y cuando las
  fuentes no están bien. Avisa a varias partes del juego (y apunta cuándo) y,
  en dos estados de otra parte, hace algo más. Uno de los avisos es un campo
  de `main_globals` que antes teníamos como una variable aparte.
- `main_print_saving_message` muestra al primer jugador de la consola el
  texto «guardando» (`0x100006A4`) en la campaña, **una sola vez** hasta que
  se termina de guardar: el indicador `unknown4E` de `main_globals`, que
  `main_unknown4E_private` vuelve a apagar al mostrar «guardado».
- Los dos nombres son suposiciones.

### Mutaciones

14 líneas rotas a propósito: las 14 detectadas.

## 2026-10-02 — Hacia dónde mira un objeto

Una función. De 659 funciones se pasa a 660, el 5,86 % de las del juego
(el 3,015 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_orientation` | `0x000b9fc0` | `objects/objects.c` |

### Lo que se sabe

- Devuelve hacia dónde mira un objeto y qué es «arriba» para él, en el
  mundo. Se puede pedir solo uno de los dos.
- Un objeto suelto los tiene tal cual. Uno unido a otro los guarda respecto
  al **nodo** del otro al que está unido, y aquí se giran con la matriz de
  ese nodo (solo el giro: ni escala ni posición). Es la pareja de
  `object_get_origin`, que hace lo mismo con la posición.
- El nombre es el de Halo 1.

### Mutaciones

13 líneas rotas a propósito: las 13 detectadas.

## 2026-10-02 — Copiar un archivo entero

Dos funciones. De 657 funciones se pasa a 659, el 5,85 % de las del
juego (el 3,001 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_copy_file_work` | `0x001a0c90` | `cseries/async.c` |
| `async_copy_file_blocking` | `0x001a0da0` | `cseries/async.c` |

### Lo que se sabe

- Copiar un archivo entero en otro es una **tarea de cinco pasos** del hilo
  de disco: volver al principio del origen (y apuntar su tamaño), volver al
  principio del destino, y luego leer y escribir **64 KB** cada vez hasta
  que una lectura trae menos; entonces se da por copiado.
- Si un paso falla, la tarea acaba y dice que no se copió. Si la escritura
  falla sin decir cuánto escribió, se cuentan 64 KB igualmente.
- La versión que espera usa un búfer de 64 KB en su propia pila, añade la
  tarea con prioridad 6 y espera a que termine; si alguno de los dos
  archivos no está abierto, no hace nada y devuelve que no se copió.
- Los nombres son suposiciones.

### Mutaciones

28 líneas rotas a propósito: las 28 detectadas.

## 2026-10-02 — Tres funciones pequeñas de los objetos

Tres funciones. De 654 funciones se pasa a 657, el 5,83 % de las del
juego (el 2,994 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `object_get_and_verify_type` | `0x000bae20` | `objects/objects.c` |
| `object_iterator_new` | `0x000bae80` | `objects/objects.c` |
| `object_get_first_unflagged_ancestor` | `0x000baf40` | `objects/objects.c` |

### Lo que se sabe

- `object_get_and_verify_type` es **idéntica** a
  `object_try_and_get_and_verify_type`: en Halo 1 la primera para el juego
  si el objeto no está, y el juego se compiló sin esas comprobaciones, así
  que quedaron dos copias iguales.
- `object_iterator_new` prepara un recorrido de los objetos de unos tipos
  (ninguno quiere decir todos) con unas marcas en su cabecera, desde el
  primero. El recorrido mide 16 bytes, no 12: termina con la firma
  `0x86868686`.
- La tercera sube desde un objeto por aquello a lo que está unido hasta el
  primero que no tiene el bit 26 de sus marcas (o hasta ninguno). Qué es
  ese bit no se sabe aún. Para ello no mira la sal del índice.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas. Una pidió casos con
índices sin sal, que pueden valer 0.

## 2026-10-02 — Matrices, cuaterniones y orientaciones

Cuatro funciones. De 650 funciones se pasa a 654, el 5,81 % de las del
juego (el 2,988 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quaternion_to_axis_angle` | `0x0011d790` | `math/real_math.c` |
| `matrix4x3_from_point_and_quaternion` | `0x001421b0` | `math/matrix_math.c` |
| `matrix4x3_to_orientation` | `0x00142360` | `math/matrix_math.c` |
| `matrix4x3_rotation_between` | `0x001424f0` | `math/matrix_math.c` |

También los globales `real_math_pi` (`0x005476c4`) y `real_math_two_pi`
(`0x005476c8`).

### Lo que se sabe

- `quaternion_to_axis_angle`: el eje de un giro (de longitud 1) y su ángulo,
  de 0 a pi; un giro de más de pi se da como el giro al revés. Un cuaternión
  sin giro da un eje de longitud 0.
- El juego guarda **pi y dos pi** en dos variables que no vienen en el
  ejecutable: se ponen al arrancar.
- `matrix4x3_from_point_and_quaternion` (nombre de Halo 1): una matriz de
  escala 1 que gira como un cuaternión y está en un punto.
- `matrix4x3_to_orientation`: lo contrario, de matriz a giro, posición y
  escala (la «orientación» de 32 bytes que ya conocíamos).
- `matrix4x3_rotation_between`: el giro que lleva de una matriz a otra,
  como un vector: su eje por su ángulo. Se calcula con la inversa de la
  primera.

### Mutaciones

19 líneas rotas a propósito: las 19 detectadas. Una pidió cuaterniones de
media vuelta, cuyo ángulo es pi exacto.

## 2026-10-02 — La memoria del sonido

Dos funciones, en un archivo nuevo. De 648 funciones se pasa a 650, el
5,77 % de las del juego (el 2,977 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_memory_allocate` | `0x00053290` | `sound/sound_memory.c` |
| `sound_memory_free` | `0x000532e0` | `sound/sound_memory.c` |

`XMemAlloc` y `XMemFree` ya las llamaban; ahora llaman a estas.

### Lo que se sabe

- La memoria que piden las bibliotecas de sonido de la consola (los
  asignadores `0x89` y `0x8A`) sale de **uno de dos *pools* de memoria** del
  juego; un indicador global dice cuál se usa.
- Una petición de tamaño 0 recibe `0x200` bytes. Lo demás que dice la
  petición no se mira.
- Devolver «nada» no hace nada.
- Los nombres del archivo y de las funciones son suposiciones.

### Mutaciones

10 líneas rotas a propósito: las 10 detectadas.

## 2026-10-02 — La memoria que piden las bibliotecas de la consola

Cinco funciones. De 643 funciones se pasa a 648, el 5,75 % de las del
juego (el 2,973 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `XMemAlloc` | `0x00012280` | `shell/shell.c` |
| `XMemFree` | `0x000122c0` | `shell/shell.c` |
| `shell_bitmap_get_texture_blocking` | `0x00012310` | `shell/shell.c` |
| `shell_bitmap_get_texture` | `0x00012360` | `shell/shell.c` |
| `xmem_static_allocate` | `0x0012c090` | `shell/shell.c` |

### Lo que se sabe

- Las bibliotecas de la consola (Direct3D, el sonido...) piden memoria con
  `XMemAlloc` y la devuelven con `XMemFree`, diciendo **quién la pide** en
  los bits 16 a 23 de sus atributos. El juego sustituye estas dos funciones
  para repartirla él:
  - quien tiene el número `0x82` recibe, para las peticiones de tipo 1, **un
    búfer de `0xCC` bytes que el juego tiene guardado** (a cero si se pide);
    las demás van a la consola;
  - `0x89` y `0x8A` reciben memoria del **sonido**;
  - el resto, la de la consola, como si el juego no hiciera nada.
- El búfer guardado no se mira para ver si está libre: se da aunque ya esté
  dado. Al devolverlo solo se marca como libre.
- Las otras dos funciones son la manera habitual de pedir la textura de un
  bitmap (si ya la tuvo este fotograma, la misma; si no, se busca y, si no
  está, se lee), esperando por ella o sin esperar.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas. Cuatro pidieron casos
nuevos: atributos con bits altos y un búfer guardado en una dirección que
coincida con la que se devuelve.

## 2026-10-02 — Los perfiles y la variante de las preferencias

Dos funciones. De 641 funciones se pasa a 643, el 5,71 % de las del
juego (el 2,961 % de sus bytes). Con ellas no falta ninguna función entre
las de `global_preferences.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `global_preferences_find_player_profile` | `0x00120e20` | `main/global_preferences.c` |
| `global_preferences_get_game_variant` | `0x00120e70` | `main/global_preferences.c` |

### Lo que se sabe

- Las preferencias guardan **cuatro nombres de perfiles** de jugador y el
  nombre de **una variante de juego**. La primera función busca el perfil de
  uno de esos nombres; la segunda, la variante.
- Si la variante no está, se crea **la variante por defecto**: su nombre es
  un texto del juego (el `0x0E000780` de una lista de textos que nombran los
  globales del juego) y tiene unos valores fijos (2, 25, 480, 16, 5, 10...),
  que de momento no sabemos qué son. Entonces no hay archivo (`NONE`).
- La variante de juego mide `0x130` bytes y lleva su nombre (31 caracteres)
  a 4 bytes del principio; ya tiene sus campos, aunque casi todos sin nombre.

### Mutaciones

30 líneas rotas a propósito: 28 detectadas. Dos pidieron casos nuevos:
etiquetas de índice mayor que 255 y una búsqueda que dice un archivo aunque
no encuentre la variante.

### Sin comprobar

- Dos mutaciones son equivalentes: copiar un carácter más del nombre (el
  cero que se pone después lo tapa) y no poner ese cero (la variante ya
  está a cero).

## 2026-10-02 — Tres funciones pequeñas de `cseries.c`

Tres funciones. De 638 funciones se pasa a 641, el 5,69 % de las del
juego (el 2,945 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `csstrnlen` | `0x0011c9a0` | `cseries/cseries.c` |
| `csnzappendf` | `0x0011c9e0` | `cseries/cseries.c` |
| `system_allocate` | `0x0011ca20` | `cseries/cseries.c` |

Pruebas nuevas en `tests/test_cseries.py`.

### Lo que se sabe

- `csstrnlen`: cuántos caracteres tiene una cadena, sin pasar de un máximo.
- `csnzappendf`: como `csnzprintf`, pero escribe **detrás** de lo que ya
  hay en el búfer, sin pasarse de su tamaño y dejándolo siempre terminado.
  Los dos nombres son los que tienen en Halo 3.
- `system_allocate`: pide memoria a la consola, desde arriba, para leer y
  escribir; si no hay, pregunta el error (sin hacer nada con él) y devuelve
  nada. El nombre es una suposición.

### Mutaciones

13 líneas rotas a propósito: 12 detectadas.

### Sin comprobar

- Una mutación es equivalente: dejar que `vsnprintf` escriba un carácter
  más. El `vsnprintf` del juego (el de Microsoft) no termina la cadena
  cuando corta, así que ese carácter cae justo donde después se pone el
  cero.

## 2026-10-02 — Leer un archivo entero

Dos funciones. De 636 funciones se pasa a 638, el 5,66 % de las del juego
(el 2,941 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_read_whole_file_work` | `0x001a13a0` | `cseries/async.c` |
| `async_read_whole_file` | `0x001a1480` | `cseries/async.c` |

Es la lectura que usa `fonts_initialize`; ya no hace falta suplantarla.

### Lo que se sabe

- Es una tarea del hilo de disco que lee **un archivo entero**, o lo que
  quepa de él, en tres pasos (una vuelta del hilo cada uno): abrirlo (para
  leer, dejando que otros lo lean), saber cuánto ocupa y leerlo de una vez.
- Termina cuando un paso falla o cuando ha leído; entonces cierra el archivo
  y dice si se leyó todo lo pedido. Si se le pide, dice también el tamaño
  del archivo (aunque no quepa entero).
- Al empezar pone a «no leído» y a 0 lo que se le pide que diga, para que
  quien espera no vea lo de antes.
- En el juego, la parte que crea la tarea está copiada dentro de la función;
  aquí se llama a `async_task_add`, que hace lo mismo.

### Mutaciones

36 líneas rotas a propósito: las 36 detectadas. Dos pidieron arreglar la
prueba: lo que se pide que diga tenía que estar en búferes que se comparen.

## 2026-10-02 — Las fuentes que hay

Una función. De 635 funciones se pasa a 636, el 5,65 % de las del juego
(el 2,935 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `fonts_initialize` | `0x00121fc0` | `text/fonts.c` |

`shell.c` ya la llamaba con un nombre provisional; ahora llama a esta.

### Lo que se sabe

- Al arrancar, el juego lee **un archivo que dice qué fuentes hay**: está en
  `z:\fonts\` y su nombre lo da otra función (aún sin descompilar). Es un
  texto con nombres separados por espacios, tabuladores o saltos de línea;
  se leen como mucho `0x7ff` bytes.
- El primer nombre es la fuente 0, el segundo la 1, y así hasta 11. Cada
  fuente nueva se carga **esperando** a que esté; un nombre repetido es la
  misma fuente y no se carga dos veces. Las que el archivo no nombra quedan
  como «no hay» (`-2`).
- Antes, otra función comprueba que lo que dicen los archivos de las fuentes
  está bien; si no, se avisa a varias partes del juego (y se apunta cuándo),
  pero el archivo se lee igual.
- El juego espera a que el archivo se lea y, si no llega a leerse nunca, se
  queda esperando para siempre.
- Solo se hace la primera vez; las siguientes solo se vuelven a preparar los
  caracteres y su caché.

### Mutaciones

45 líneas rotas a propósito: 44 detectadas. Una pidió arreglar la prueba
(la lectura suplantada no borraba el aviso de «terminado», y la espera no se
veía).

### Sin comprobar

- Una mutación es equivalente: cortar el texto leído un byte más allá solo
  cambia algo con `0x7ff` bytes, y entonces las dos versiones cortan en el
  mismo sitio.

## 2026-10-02 — Pintar un texto

Una función. De 634 funciones se pasa a 635, el 5,64 % de las del juego
(el 2,909 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_draw_characters` | `0x0001f070` | `rasterizer/rasterizer_text.c` |

Tres funciones de la consola que aún no están descompiladas reciben el
nombre que tienen en Halo 1, porque hacen lo mismo:
`rasterizer_text_begin` (`0x00035b90`), `rasterizer_text_draw_character`
(`0x000363a0`) y `rasterizer_text_end` (`0x00036560`).

### Lo que se sabe

- Es la que **pinta un texto de verdad**: prepara la consola, recorre el
  texto con `draw_string_draw_characters` dándole la función que pinta cada
  carácter con su sombra, y avisa a la consola de que ha terminado.
- Sin rectángulo, el texto se coloca en la **ventana de los textos** (una
  zona guardada junto a la de la pantalla, medida desde la esquina de esta).
  Lo que se recorta nunca empieza antes de la pantalla ni acaba después.
- No se pinta nada si la caché de caracteres no está lista, si el texto está
  vacío o mientras dos indicadores globales lo impiden.
- Las coordenadas del bitmap de caracteres se pasan en píxeles o divididas
  por su tamaño, según la marca «lineal» del bitmap.
- Como en Halo 1, hay un contador `magic_number` que empieza en 12 y sube con
  cada texto pintado; no se ha visto a nadie leerlo.

### Mutaciones

47 líneas rotas a propósito: 46 detectadas.

### Sin comprobar

- Que `magic_number` empiece en 12: las pruebas siempre fijan su valor. El
  12 se ha leído del ejecutable.

## 2026-10-02 — Un carácter entra en el bitmap de la caché

Una función. De 633 funciones se pasa a 634, el 5,63 % de las del juego (el
2,890 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cache_hardware_format_character` | `0x0001f5c0` | `rasterizer/rasterizer_text.c` |

### Lo que se sabe

- Copia el dibujo de un carácter al bitmap de 256×256 del que se pintan, si
  no está ya. El nombre y los campos de la caché (dónde lee, dónde escribe,
  la posición siguiente y la altura de la fila) son los de Halo 1.
- Los dibujos se colocan **en filas**, de izquierda a derecha y separados un
  píxel; cada fila es tan alta como su dibujo más alto. Al llegar abajo se
  vuelve a empezar por arriba.
- Los 512 sitios se usan **en orden, como un anillo**: se libera siempre el
  más antiguo. Se liberan los dibujos que el nuevo va a pisar (todos los de
  abajo al volver arriba; los de la franja que ocupa la fila al crecer) y,
  si el anillo está lleno, uno más.
- Si se ha liberado algún sitio, antes de escribir se **espera a la consola**,
  por si aún está pintando desde ese trozo del bitmap.
- Alrededor del dibujo se escribe un marco de un píxel de **blanco
  transparente** (`0x0FFF`), para que el filtrado de la textura no mezcle
  dibujos vecinos.
- Después se pide la textura del bitmap a la caché de texturas, como en
  otros sitios del juego.

### Mutaciones

70 líneas rotas a propósito: las 70 detectadas. Dos pidieron casos nuevos:
la caché empezando en la columna o la fila 0, con la textura con margen
delante para ver lo que se escribiría fuera.

## 2026-10-02 — Pintar un carácter y su sombra

Dos funciones. De 631 funciones se pasa a 633, el 5,62 % de las del juego
(el 2,852 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_draw_character` | `0x0001f3a0` | `rasterizer/rasterizer_text.c` |
| `rasterizer_draw_character_with_dropshadow` | `0x0001f490` | `rasterizer/rasterizer_text.c` |

### Lo que se sabe

- `rasterizer_draw_character_with_dropshadow` es **la función que pinta de
  verdad**: la que se le da al recorrido de un texto cuando no se está
  midiendo. Busca el carácter (y si no lo conoce, lo pide y vuelve a
  buscarlo), hace que su dibujo esté en el bitmap de la caché y lo pinta
  desde ahí.
- La **sombra** es el mismo dibujo pintado antes, con el color de la sombra,
  desplazado hacia abajo y a la derecha tanto como la escala (un píxel a
  escala 1). Solo hay sombra si su color tiene algo de opacidad.
- Un carácter se pinta como un **rectángulo de cuatro esquinas**; cada una
  lleva dónde va en pantalla, de qué píxel del bitmap de la caché sale y el
  color. El tamaño en pantalla va por la escala; el del bitmap, no.
- Antes de pintar, las cuatro esquinas se le pueden pasar a una función
  (guardada en cómo se dibujan los textos, en `+0x34`, con un dato suyo en
  `+0x38`): puede cambiarlas, y decir que el carácter no se pinte.
- Los nombres de las dos funciones son los de Halo 1.

### Mutaciones

57 líneas rotas a propósito: las 57 detectadas.

## 2026-10-02 — Hito 100: la caché de los caracteres que se pintan

Dos funciones, en un archivo nuevo. De 629 funciones se pasa a 631, el
5,60 % de las del juego (el 2,844 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `rasterizer_text_cache_initialize` | `0x0001eff0` | `rasterizer/rasterizer_text.c` |
| `rasterizer_text_cache_flush` | `0x0001f2b0` | `rasterizer/rasterizer_text.c` |

También el global `hardware_character_cache` (`0x004b62a0`).

### Lo que se sabe

- Los caracteres no se pintan desde la caché de píxeles de las fuentes, sino
  desde **un solo bitmap de 256×256** (16 bits por píxel, 4 por canal) al
  que se van copiando. Ese bitmap no tiene memoria propia para los píxeles:
  solo existe como textura.
- En él caben **512 caracteres** a la vez. Cada sitio guarda qué carácter
  tiene y dónde están sus píxeles en el bitmap; son los «sitios de dibujo»
  que ya usaba `font_cache.c`, y en el juego van a continuación de esta
  estructura.
- La estructura es la misma que la de Halo 1 (de ahí los nombres), con el
  bitmap y la cantidad de sitios al doble.
- Al empezar, todos los sitios quedan libres y el siguiente carácter irá al
  píxel (1, 1). Si no se puede crear el bitmap, lo que había se queda como
  estaba; si no se puede crear su textura, el bitmap no se devuelve.
- Vaciar la caché deja libres todos los sitios y les quita el suyo a los
  caracteres que lo tenían.

### Mutaciones

27 líneas rotas a propósito: las 27 detectadas.

## 2026-10-02 — Crear un bitmap

Una función. De 628 funciones se pasa a 629, el 5,58 % de las del juego (el
2,832 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bitmap_2d_new` | `0x001358e0` | `bitmaps/bitmaps.c` |

### Lo que se sabe

- Los bitmaps que crea el propio juego (no los que vienen en el mapa) piden
  su memoria a la consola: `0x74` bytes para el bitmap y, aparte, la de sus
  píxeles. Es la pareja de `bitmap_delete`, que devuelve las dos.
- El bitmap empieza por la firma `bitm`, tiene profundidad 1 y lleva la
  marca de «hecho por el juego». Además se le ponen solas tres marcas: si su
  ancho y su alto son potencias de dos, si su formato es comprimido (los tres
  DXT) y si es de paleta (el formato 17).
- Con la marca del bit 11 se crea **sin memoria para los píxeles**: es lo que
  usa la caché de caracteres, cuyo bitmap solo existe como textura.
- Si no hay memoria para el bitmap, no hay bitmap; si no la hay para los
  píxeles, el bitmap se queda sin ellos.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas.

## 2026-10-02 — Las líneas de un texto: `draw_string.c`, completo

Una función. De 627 funciones se pasa a 628, el 5,57 % de las del juego (el
2,824 % de sus bytes). Con ella están las 19 funciones de `draw_string.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_draw_characters` | `0x0013f0e0` | `text/draw_string.c` |

Era la función a la que llamaba la que mide un texto; ya no hace falta
suplantarla.

### Lo que se sabe

- Es la que **reparte un texto en líneas** dentro de un rectángulo. Para
  cada línea: calcula dónde empieza, mide cuánto cabe
  (`draw_string_measure_line`), la coloca a la izquierda, a la derecha o al
  centro, y la dibuja (`draw_string_draw_line`) si su línea base queda por
  encima del borde inferior.
- Una línea empieza en el borde izquierdo del rectángulo más una **sangría**:
  una para la primera línea y otra para las demás.
- Con **tabuladores**, lo que va tras un tabulador empieza en su parada y
  acaba en la siguiente; un tabulador de más no hace nada. Las paradas se
  miden desde el borde izquierdo de la pantalla, no del rectángulo.
- Si lo medido no cabe, sigue en la línea de debajo, desde la misma parada.
  Tras un fin de línea se baja por debajo de la columna que más haya bajado.
- La altura de una línea es lo que sube y lo que baja la fuente, más su
  interlineado, más el interlineado extra que se pida.
- A la derecha, el texto se pega al borde derecho menos el margen de la
  fuente (sin escala); al centro, se reparte lo que sobra.
- Al acabar deja, si se le pide, dónde ha terminado el texto.
- De la fuente se conoce un campo más: el margen con que empieza una línea.

### Herramientas

- Una función suplantada que devuelve una estructura de 8 bytes (`pair`)
  puede dar sus dos mitades.

### Mutaciones

74 líneas rotas a propósito: las 74 detectadas.

## 2026-10-02 — Dibujar una línea de texto

Una función. De 626 funciones se pasa a 627, el 5,57 % de las del juego (el
2,794 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_draw_line` | `0x0013f700` | `text/draw_string.c` |

### Lo que se sabe

- Recorre los caracteres de una línea, desde uno hasta antes de otro, y con
  cada uno llama a la función que se le da (la que lo pinta, o la que mide).
- Cada carácter se **recorta** contra dos rectángulos, que pueden no darse;
  sin ninguno, lo que se dibuja va de -32768 a 32767 en las dos direcciones.
  A la función se le dice qué trozo del carácter se ve: dónde queda, cuánto
  mide y cuánto se ha quitado por la izquierda y por arriba. Si no se ve
  nada, no se la llama.
- El carácter se coloca donde va la línea, más dónde empiezan sus píxeles y
  el ajuste con el anterior; en vertical, lo que sus píxeles suben desde la
  línea. Después la línea avanza lo que diga la fuente, se haya dibujado el
  carácter o no.
- No se dibujan los caracteres hasta el `0x1F`, ni los que la caché no
  tiene.
- Los **dibujos del juego** (los caracteres privados, como los botones del
  mando) se pintan en blanco: del color solo se les deja la opacidad.

### Herramientas

- Una función de pega del arnés (`Mock`) puede decir qué se compara de cada
  llamada (`record`), para cuando recibe direcciones de la pila.

### Mutaciones

82 líneas rotas a propósito: 80 detectadas. Cuatro pidieron casos nuevos
(texto más allá de 640×480 sin rectángulos, y una fuente del recorrido
distinta de la de los textos).

### Sin comprobar

- Dos mutaciones son equivalentes: tratar un «no es un número» de otra
  forma en la comprobación de que queda sitio y en la de que el carácter se
  ve. Los bordes nunca llegan a ser «no es un número» (un rectángulo así no
  los cambia), y el ancho y el alto recortados tampoco.

## 2026-10-02 — Cuánto texto cabe en una línea

Una función. De 625 funciones se pasa a 626, el 5,56 % de las del juego (el
2,765 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_measure_line` | `0x0013ef40` | `text/draw_string.c` |

### Lo que se sabe

- La función va tomando caracteres mientras quepan antes del borde derecho,
  y devuelve **dos cosas**: el ancho de lo que cabe y el índice del carácter
  que va después. Las devuelve en dos registros (`eax` y `edx`), como hace el
  compilador del juego con una estructura de 8 bytes.
- Lo que ocupa un carácter es su avance, más dónde empiezan sus píxeles, más
  el ajuste con el carácter anterior (*kerning*), todo por la escala.
- **El primer carácter cabe siempre**, aunque no quepa: así una línea nunca
  se queda vacía.
- Si el texto se parte donde puede (el bit 0 de las opciones), al llegar a un
  carácter que no cabe la línea vuelve al último sitio donde podía partirse.
  Si no hay tal sitio, la línea acaba ahí mismo, con ese carácter dentro pero
  sin contar su ancho.
- Si el texto solo se parte donde él dice, los caracteres siguen pasando por
  encima del borde y no se cuentan en el ancho.
- De la cabecera de un carácter se conocen ahora tres campos más: el avance
  y dónde están sus píxeles respecto al sitio en que se dibuja (a la derecha
  y hacia arriba).

### Herramientas

- El arnés compara funciones que devuelven una estructura de 8 bytes (tipo
  `pair` en `symbols/functions.csv`), y `source/` se compila devolviéndolas
  en registros, como el juego.

### Mutaciones

51 líneas rotas a propósito: las 51 detectadas.

## 2026-10-02 — Medir un texto

Cuatro funciones. De 621 funciones se pasa a 625, el 5,55 % de las del
juego (el 2,751 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_initialize` | `0x0013e8a0` | `text/draw_string.c` |
| `draw_string_bounds_character` | `0x0013e8e0` | `text/draw_string.c` |
| `draw_string_get_bounds` | `0x0013e9c0` | `text/draw_string.c` |
| `draw_string_get_characters_bounds` | `0x0013ea60` | `text/draw_string.c` |

También el global `global_real_argb_white` (`0x004686cc`, en
`math/real_math.c`) y `draw_string_bounds` (`0x004e7394`).

### Lo que se sabe

- Al empezar, los textos se dibujan con la primera fuente, en blanco, sin
  sombra, sin tabuladores y a la izquierda.
- Lo que se sabe de cómo se dibujan los textos ocupa `0x64` bytes, no
  `0x60`: al final hay dos sangrías, la de la primera línea y la de las
  demás.
- **Medir un texto es recorrerlo como si se dibujara**, pero dándole a la
  función que lo recorre (`0x0013f0e0`, aún sin descompilar) otra cosa que
  hacer con cada carácter: en vez de pintarlo, agrandar un rectángulo hasta
  que quepa. El rectángulo empieza «al revés» (`0x7FFF` arriba y a la
  izquierda, `-0x8000` abajo y a la derecha) y cada carácter lo estira.
- De cada carácter se le dice a esa función, entre otras cosas: por dónde va
  el recorrido, la fuente, el carácter, su color, dónde queda lo que se ve
  de él, cuánto mide y la escala. Son 14 argumentos, y la función los quita
  ella misma de la pila.
- Además del rectángulo, medir da **dónde acaba el texto**: un sitio de un
  píxel de ancho y tan alto como una línea de la fuente (lo que sube y lo
  que baja desde la línea).
- El texto del juego se convierte primero en caracteres de 32 bits (2047
  como mucho), cambiando cada barra `|` y su letra por la orden que es. El
  juego repite ese trozo en cada función que recibe un texto; aquí es una
  sola función en línea.

### Mutaciones

63 líneas rotas a propósito: las 63 detectadas.

## 2026-10-02 — Cargar una fuente: `fonts.c`, completo

Dos funciones. De 619 funciones se pasa a 621, el 5,51 % de las del juego
(el 2,738 % de sus bytes). Con ellas están las 6 funciones de `fonts.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_load_work` | `0x001222d0` | `text/fonts.c` |
| `font_load` | `0x001223a0` | `text/fonts.c` |

### Lo que se sabe

- Una fuente es un archivo de `z:\fonts\`, con el nombre que se le da (31
  caracteres como mucho). Se abre para leer de él a saltos.
- Su **cabecera** son `0x1C4` bytes que están a `0x200` del principio del
  archivo, y se leen tal cual en el sitio de la fuente. Empieza por la firma
  `0xF0000001`.
- Cargarla es una **tarea en dos pasos**, como las demás de disco: en el
  primero se abre el archivo (si no se puede, la fuente deja de estar
  cargándose y la tarea acaba) y en el segundo se lee la cabecera.
- Quien la pide puede esperar a que acabe. Si espera y lo leído no empieza
  por la firma, el juego **olvida las fuentes** que tenía apuntadas en las
  preferencias y las escribe en el disco en el momento: lo mismo que hace la
  caché de caracteres cuando una cabecera no es válida.
- La tarea tiene más prisa cuando se espera por ella que cuando no.

### Mutaciones

42 líneas rotas a propósito: las 42 detectadas.

## 2026-10-02 — Los píxeles de un carácter: `font_cache.c`, completo

Dos funciones. De 617 funciones se pasa a 619, el 5,49 % de las del juego
(el 2,729 % de sus bytes). Con ellas están las 28 funciones de
`font_cache.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_decode_pixels` | `0x00122610` | `text/fonts.c` |
| `font_character_get_pixels` | `0x00140960` | `text/font_cache.c` |

### Lo que se sabe

- En el archivo de una fuente los píxeles de un carácter están **comprimidos
  por tramos**. Cada píxel es un color de 12 bits y una opacidad; en el
  archivo la opacidad tiene 3 bits y al descomprimir pasa a 4 (repitiendo el
  bit bajo). Cada byte es una de cuatro cosas, según sus dos bits altos:
  - `00`: tantos píxeles transparentes como digan los otros 6 bits; con
    cero, un **color nuevo**, que viene en los dos bytes siguientes y cuenta
    él mismo como un píxel;
  - `01`: tantos píxeles opacos;
  - `10`: un píxel con su opacidad y, detrás, de 0 a 5 opacos o
    transparentes, según los tres bits bajos;
  - `11`: dos píxeles, cada uno con su opacidad.
  El color es blanco hasta que se pone otro.
- La función cuenta los píxeles aunque no se le dé dónde escribirlos: sirve
  para saber cuántos son.
- Para dibujar un carácter se piden sus píxeles esperando por ellos, y se
  descomprimen desde su bloque de la caché.

### Mutaciones

54 líneas rotas a propósito: las 54 detectadas.

## 2026-10-02 — Los caracteres que se esperan

Dos funciones. De 615 funciones se pasa a 617, el 5,48 % de las del juego
(el 2,709 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_cache_frame` | `0x001406b0` | `text/font_cache.c` |
| `font_character_precache` | `0x001414d0` | `text/font_cache.c` |

### Lo que se sabe

- Antes de dibujar un texto el juego pregunta por cada carácter si está, con
  sus píxeles. Si no está, lo pide **sin esperar** y lo apunta en una lista
  de «esperados» (256 como mucho), una sola vez por carácter y fuente.
- Cada fotograma la caché envejece un tic, el contador de fotogramas avanza,
  y se vuelve a pedir cada carácter esperado; el que ya está, con sus
  píxeles, se borra de la lista. Así un texto nuevo tarda unos fotogramas en
  tener todas sus letras, sin parar el juego.
- Eran las dos funciones sin nombre que llamaban el bucle principal y
  `draw_string_precache_characters`. La lista era el campo sin nombre de
  `font_cache_globals`.

### Sin comprobar

- Apuntar el carácter cuando la lista de esperados está llena: sin la
  comprobación se escribiría fuera de la lista.

### Mutaciones

32 líneas rotas a propósito: 31 detectadas (cuatro porque la prueba se
detiene: el juego roto pide más veces de las previstas), y la otra es la de
«Sin comprobar».

## 2026-10-02 — Pedir un carácter y sus píxeles

Seis funciones. De 609 funciones se pasa a 615, el 5,46 % de las del juego
(el 2,694 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_request` | `0x001407d0` | `text/font_cache.c` |
| `font_character_pixels_request` | `0x00140870` | `text/font_cache.c` |
| `font_character_get_header` | `0x00140900` | `text/font_cache.c` |
| `font_cache_block_wait` | `0x001412c0` | `text/font_cache.c` |
| `font_character_pixels_new` | `0x00141350` | `text/font_cache.c` |
| `font_cache_read` | `0x00141480` | `text/font_cache.c` |

### Lo que se sabe

- Quien dibuja pide un carácter con dos banderas: **esperar** (bit 0) y
  **darlo a conocer si no se conoce** (bit 1). Sin esperar, cada petición da
  un paso más de la carga y contesta si ya está listo; esperando, da vueltas
  hasta que lo está. En los dos casos se apunta que se usó en este fotograma.
- Los píxeles van aparte: cuando el carácter está listo se le busca sitio en
  la caché (tantos bytes como dice su cabecera), se apunta el bloque y una
  tarea los lee del archivo de la fuente, desde donde dice la cabecera (el
  campo `pixels_offset`, que no tenía nombre). Si no hay sitio, no hay
  píxeles.
- Cada vez que se piden los píxeles de un carácter, su bloque queda marcado
  como recién usado, para que la caché no lo suelte.
- De las banderas con las que se piden los píxeles solo cuenta la primera.
- `font_cache_read` es la lectura del archivo de una fuente que las funciones
  del hito anterior llevaban dentro.

### Mutaciones

56 líneas rotas a propósito: las 56 detectadas.

## 2026-10-02 — Leer un carácter del archivo de su fuente

Seis funciones. De 603 funciones se pasa a 609, el 5,41 % de las del juego
(el 2,666 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_read_location` | `0x00140dc0` | `text/font_cache.c` |
| `font_character_header_validate` | `0x00140e40` | `text/font_cache.c` |
| `font_character_location_wait` | `0x00140f00` | `text/font_cache.c` |
| `font_character_read_header` | `0x00140f50` | `text/font_cache.c` |
| `font_character_header_wait` | `0x00140fc0` | `text/font_cache.c` |
| `font_character_update` | `0x00141020` | `text/font_cache.c` |

### Lo que se sabe

- Un carácter se prepara en cuatro pasos, uno cada vez que se le da una
  vuelta (`font_character_update`), y cada lectura la hace una tarea del hilo
  de las tareas:
  1. se pide leer **dónde está** el carácter: 4 bytes de una tabla del
     archivo de la fuente, en `0x400 + 4 × carácter`;
  2. cuando eso llega, se pide leer **lo que el archivo dice de él**: 16
     bytes en `(sitio + 0x4040) × 16`;
  3. cuando llega, se comprueba y el carácter queda listo.
- Los estados y campos del carácter ya tienen nombre: nuevo, leyendo su
  sitio, sitio leído, leyendo su cabecera y listo.
- Si quien pide el carácter quiere esperar, la lectura se pide con más
  prioridad (6 en vez de 2), a la tarea se le sube la prioridad y el juego se
  queda cediendo el turno hasta que termina.
- La cabecera de un carácter dice su ancho y su alto (128 como mucho) y
  cuántos bytes ocupan sus píxeles en el archivo (de 1 a 4.096). Si no
  cumple, se recorta y además **el juego deja de fiarse de las fuentes del
  disco duro**: apunta en las preferencias que no hay ninguna copiada y las
  escribe en el acto.
- El archivo abierto de una fuente está en su hueco (`font.file`, que era un
  campo sin nombre).

### Cambiado

- Los estados `_font_character_state_unknown0…3` y los campos sin nombre de
  `struct font_character` tienen ahora los nombres de arriba.

### Mutaciones

59 líneas rotas a propósito: las 59 detectadas.

## 2026-10-02 — Un carácter nuevo

Una función. De 602 funciones se pasa a 603, el 5,35 % de las del juego (el
2,644 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_new` | `0x00140b80` | `text/font_cache.c` |

### Lo que se sabe

- Cuando hace falta un carácter que el juego no conoce, se le busca sitio
  entre los 512. Si no queda ninguno, se recorre la lista y se elige, de los
  que no están en uso, **el que lleva más fotogramas sin usarse**; se olvida y
  su sitio pasa al nuevo. Si todos están en uso, no hay carácter.
- El carácter nuevo empieza en el estado 0 (no se sabe nada de él), sin
  píxeles ni hueco entre los dibujos, con el fotograma actual como último
  uso, y se apunta en la tabla hash con su índice.
- Si el carácter ya se conocía, se devuelve el que había.

### Mutaciones

33 líneas rotas a propósito: las 33 detectadas (una porque la prueba se
detiene: el índice roto no existe).

## 2026-10-02 — Crear la caché de los caracteres

Cuatro funciones. De 598 funciones se pasa a 602, el 5,34 % de las del juego
(el 2,633 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_characters_new` | `0x001409d0` | `text/font_cache.c` |
| `font_cache_new` | `0x001410d0` | `text/font_cache.c` |
| `font_cache_block_deleted` | `0x001411e0` | `text/font_cache.c` |
| `font_cache_block_locked` | `0x00141290` | `text/font_cache.c` |

### Lo que se sabe

- El juego sabe de 512 caracteres a la vez, y de 256 «predichos» (los que
  están a punto de dibujarse). La tabla hash que los encuentra tiene 1.024
  listas y guarda 4 bytes con cada clave: el índice del carácter.
- Los píxeles de los caracteres viven en una caché de **30.720 bytes** que se
  pide a la consola, en páginas de 16 bytes, con 512 bloques como mucho. Es
  una caché del mismo tipo que la de texturas (`lruv_cache`), con el mismo
  modo de elegir hueco.
- Un bloque no puede salir de la caché mientras sus píxeles se están leyendo
  del disco. Cuando la caché suelta un bloque, el juego espera a que esa
  lectura termine —contando las tareas pendientes en cada vuelta— y entonces
  deja al carácter sin píxeles y borra el bloque.
- Una lista del juego ocupa su cabecera (0x4C bytes), sus elementos y un bit
  por elemento.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Olvidar un carácter

Dos funciones. De 596 funciones se pasa a 598, el 5,31 % de las del juego
(el 2,620 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_character_delete` | `0x00140cd0` | `text/font_cache.c` |
| `font_character_is_busy` | `0x00140d50` | `text/font_cache.c` |

- `struct font_character` (0x38 bytes): lo que el juego sabe de un carácter
  de una fuente; `struct font_cache_block`; y la tabla `font_glyph_slots`
  (`0x004b62b0`, 512 huecos).
- `font_cache_globals` tiene un campo más: el número de fotograma
  (`0x0054d588`).

### Lo que se sabe

- De cada carácter el juego guarda su clave (fuente y carácter), en qué paso
  de la carga está (cinco estados; el último, listo), el bloque de la caché
  con sus píxeles, el último fotograma en que se usó y su hueco entre los
  dibujos que se están pintando.
- Olvidar un carácter es: dejar libre su hueco entre los dibujos (y quitárselo
  al carácter que lo tuviera apuntado), sacar sus píxeles de la caché, quitarlo
  de la tabla hash y borrarlo de la lista.
- Un carácter no se puede olvidar ahora si se usó en este fotograma o en el
  anterior, si está a medio cargar (estados 1 y 3), o si tiene píxeles en la
  caché que no pueden salir de ella.

### Mutaciones

25 líneas rotas a propósito: 24 detectadas a la primera y la otra con un caso
nuevo (más de 256 caracteres: el índice es de 16 bits).

## 2026-10-02 — Vectores de bits

Cinco funciones. De 591 funciones se pasa a 596, el 5,29 % de las del juego
(el 2,613 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `highest_bit_set` | `0x0013e3a0` | `memory/bit_vector.c` (nuevo) |
| `bits_to_count` | `0x0013e3c0` | `memory/bit_vector.c` |
| `bit_vector_or` | `0x0013e3e0` | `memory/bit_vector.c` |
| `bit_vector_count_bits` | `0x0013e420` | `memory/bit_vector.c` |
| `bit_vector_highest_bit_set` | `0x0013e530` | `memory/bit_vector.c` |

- `tests/test_bit_vector.py`.

### Lo que se sabe

- Un vector de bits son números de 32 bits seguidos. Contar los bits
  encendidos y buscar el más alto solo miran, del último número, los bits que
  son del vector.
- Con un vector de cero bits las dos funciones miran el número que hay
  **antes** del vector: cuentan sus bits, o dan un índice negativo. Es del
  juego y se reproduce.
- `bits_to_count` da cuántos bits hacen falta para distinguir tantas cosas
  (uno como poco); `highest_bit_set`, el bit más alto de un número (0 si no
  tiene ninguno).

### Sin comprobar

- Dos de las máscaras con las que se cuentan los bits: los bits que quitaría
  una máscara más estrecha son siempre 0 en ese paso (dos mutaciones).
- En `bits_to_count`, que la cuenta sea «mayor que 1» o «mayor que 2»: con 2
  sale 1 de las dos formas.

### Mutaciones

38 líneas rotas a propósito: 35 detectadas, y las tres de «Sin comprobar».

## 2026-10-02 — UTF-8, y encontrar un carácter de una fuente

Seis funciones. De 585 funciones se pasa a 591, el 5,25 % de las del juego
(el 2,596 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `utf8_string_to_wchar_string` | `0x00140440` | `text/unicode.c` |
| `utf8_encode_character` | `0x001405a0` | `text/unicode.c` |
| `wchar_string_to_utf8_string` | `0x00140650` | `text/unicode.c` |
| `font_character_find` | `0x00140b20` | `text/font_cache.c` |
| `font_character_key_hash` | `0x00141090` | `text/font_cache.c` |
| `font_character_key_compare` | `0x001410b0` | `text/font_cache.c` |

### Lo que se sabe

- Los textos del juego están guardados en UTF-8 y se pasan a caracteres de
  16 bits al usarlos. Un carácter mal escrito (un byte de continuación
  suelto, un principio sin sus continuaciones) se salta, y se sigue desde el
  byte siguiente. De un carácter de más de 16 bits se guardan los 16 bajos:
  los emoji y demás salen como otro carácter. No se rechazan las formas
  demasiado largas ni los sustitutos.
- Al revés, un carácter se escribe en 1 a 4 bytes; si no cabe entero se corta
  por donde quepa, así que el texto puede acabar con un carácter a medias.
- La clave de un carácter en la tabla hash es el par (hueco de la fuente,
  carácter), y su hash es tres veces el hueco más el carácter. Lo que la
  tabla guarda con la clave es el índice de lo que el juego sabe del
  carácter.

### Cambiado

- `character_is_private_use` pasa de `font_cache.c` a `unicode.c`: está entre
  las funciones de caracteres, antes de las de UTF-8.
- `utf8_string_to_wchar_string` era la función sin nombre que usaba
  `unicode_string_list.c`.
- La tabla de los caracteres de `font_cache_globals` es una `struct
  hash_table`.

### Sin comprobar

- Las máscaras de los bits del primer byte de un carácter de 2, 3 o 4 bytes:
  el bit de más que dejaría pasar una máscara más ancha es siempre 0 en un
  principio válido (tres mutaciones). Lo mismo con el bit 6 de un byte de
  continuación.
- Seguir leyendo continuaciones después de una mala, y avanzar siempre un
  solo byte: el texto que sale es el mismo, porque las continuaciones sueltas
  se saltan.

### Mutaciones

66 líneas rotas a propósito: 58 detectadas a la primera, dos más con casos
nuevos (sin sitio para escribir, nada se escribe tampoco antes del destino),
y las seis de «Sin comprobar».

## 2026-10-02 — La tabla hash

Cinco funciones. De 580 funciones se pasa a 585, el 5,19 % de las del juego
(el 2,572 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `hash_table_new` | `0x0013e1a0` | `memory/hash_table.c` (nuevo) |
| `hash_table_clear` | `0x0013e210` | `memory/hash_table.c` |
| `hash_table_add` | `0x0013e270` | `memory/hash_table.c` |
| `hash_table_find` | `0x0013e2d0` | `memory/hash_table.c` |
| `hash_table_remove` | `0x0013e320` | `memory/hash_table.c` |

- `tests/test_hash_table.py`.

### Lo que se sabe

- Es una tabla de tamaño fijo: una cabecera de 0x3C bytes, las listas (una
  por cada resto del hash) y, detrás, todas las entradas que puede llegar a
  tener, cada una con su clave, su hash, el enlace a la siguiente y los datos
  que se guardan con la clave. Las entradas libres están en otra lista.
- Añadir no mira si la clave ya estaba, y la nueva entrada queda la primera
  de su lista. Si no quedan entradas libres, no se añade.
- Buscar compara primero los hashes y luego las claves, con la función que
  se dio al crear la tabla. Quitar solo compara las claves.
- No es la tabla hash de Halo 1 (que crece): los nombres son propios.
- La usa la caché de caracteres de las fuentes (1.024 listas, 4 bytes de
  datos por clave) y otros tres sitios del juego.

### Mutaciones

50 líneas rotas a propósito: las 50 detectadas.

## 2026-10-02 — La caché de los caracteres: el final

Cuatro funciones. De 576 funciones se pasa a 580, el 5,15 % de las del juego
(el 2,557 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `character_is_private_use` | `0x00140420` | `text/font_cache.c` (nuevo) |
| `font_characters_dispose` | `0x00140aa0` | `text/font_cache.c` |
| `font_cache_dispose` | `0x00141190` | `text/font_cache.c` |
| `font_cache_block_get_address` | `0x00141450` | `text/font_cache.c` |

- `struct font_cache_globals` (`0x0054d570`) y `tests/test_font_cache.py`.

### Lo que se sabe

- El juego guarda aparte los caracteres que tiene listos para dibujar: una
  tabla para encontrarlos, dos listas con lo que sabe de cada uno, y una
  caché (del mismo tipo que la de texturas) con sus píxeles. Las dos
  funciones sin nombre que `shell.c` llamaba al salir eran las que lo
  sueltan todo.
- Los píxeles de un bloque de la caché están en la base de la caché más su
  primera página por el tamaño de página.
- Los caracteres de `0xE000` a `0xF8FF` son los de uso privado de Unicode.

### Mutaciones

30 líneas rotas a propósito: las 30 detectadas.

## 2026-10-02 — Las fuentes cargadas

Tres funciones. De 573 funciones se pasa a 576, el 5,11 % de las del juego
(el 2,548 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `font_get` | `0x001224c0` | `text/fonts.c` (nuevo) |
| `font_get_line_height` | `0x00122540` | `text/fonts.c` |
| `font_get_kerning` | `0x00122570` | `text/fonts.c` |

- `struct font` (0x1D0 bytes) y la tabla `font_slots` (`0x004e2920`): los
  diez huecos para fuentes. Era el bloque sin nombre que `shell.c` borraba al
  salir.
- `tests/test_fonts.py`.

### Lo que se sabe

- Hay diez huecos para fuentes. Una fuente se carga con una tarea del hilo
  de las tareas; quien la pide mientras se carga le sube la prioridad a esa
  tarea y espera a que termine.
- La altura de una línea es la suma de tres medidas de la fuente (lo que sube,
  lo que baja y el interlineado); sin fuente, 10.
- El ajuste entre dos letras (*kerning*) solo existe para caracteres de 8
  bits que no sean el 0: hasta 96 pares ordenados, y un bit por cada carácter
  que empieza algún par, para no buscar en balde.
- Al buscar un par el juego mira el primer hueco de la lista antes de
  comprobar que haya alguno: con cero pares puede dar el ajuste de lo que
  haya en ese hueco. Se reproduce tal cual.

### Sin comprobar

- El hueco número 10 (uno más de los que hay): el juego lee lo que haya
  detrás de la tabla.
- Los caracteres de más de 8 bits: sin la comprobación tampoco encuentran
  par, porque los de los pares son de 8 bits (dos mutaciones).
- Comprobar que quedan pares antes de mirar el primero: da lo mismo, por lo
  dicho arriba.

### Mutaciones

35 líneas rotas a propósito: 25 detectadas a la primera, seis más con casos
nuevos (pares con el carácter 0, un par cuyo bit no está, y algo que parece
un par justo detrás de los pares), y las cuatro de «Sin comprobar».

## 2026-10-02 — El color del texto

Dos funciones. De 571 funciones se pasa a 573, el 5,09 % de las del juego
(el 2,538 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_set_color_exact` | `0x0013eb60` | `text/draw_string.c` |
| `draw_string_set_color` | `0x0013ec70` | `text/draw_string.c` |

### Lo que se sabe

- El juego **no dibuja texto casi blanco**: si las tres partes del color
  (rojo, verde y azul) pasan de 0,68, las escala para que la menor quede en
  0,68. El blanco puro se queda en un gris claro. La opacidad no se toca.
- Después cada parte se recorta a entre 0 y 1. Una parte que no es un número
  se deja como está, y basta una así para que no se escale nada.
- `draw_string_set_color` era la función sin nombre a la que llamaba
  `draw_string_set_font_and_format`.

### Sin comprobar

- Que las partes tengan que pasar de 0,68 o basta con llegar: con una parte
  justo en 0,68 la escala es 1 y no cambia nada.

### Mutaciones

34 líneas rotas a propósito: 32 detectadas a la primera, una más con casos
nuevos (una parte que no es un número y las otras casi blancas), y la otra es
la de «Sin comprobar».

## 2026-10-02 — Cómo se dibuja el texto

Siete funciones. De 564 funciones se pasa a 571, el 5,07 % de las del juego
(el 2,522 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `draw_string_set_tab_stops` | `0x0013eb20` | `text/draw_string.c` |
| `draw_string_set_shadow_color` | `0x0013ed50` | `text/draw_string.c` |
| `draw_string_set_font` | `0x0013ed90` | `text/draw_string.c` |
| `draw_string_set_font_and_format` | `0x0013edb0` | `text/draw_string.c` |
| `draw_string_precache_string` | `0x0013ee20` | `text/draw_string.c` |
| `draw_string_precache_characters` | `0x0013eeb0` | `text/draw_string.c` |
| `draw_string_precache` | `0x0013ef30` | `text/draw_string.c` |

- `struct draw_string_globals` (`0x004e73a0`, 0x60 bytes): cómo se dibuja el
  texto. Sustituye a la variable suelta `draw_string_flags`, que era su
  segundo campo.

### Lo que se sabe

- El juego guarda en un solo sitio cómo dibuja el texto: la fuente, las
  banderas, el estilo, la alineación, el color, si hay sombra y de qué
  color, y hasta 16 tabuladores. Los nombres de las funciones que lo fijan
  son los de Halo 1 (`draw_string_set_font`, `…_set_tab_stops`…).
- Fijar la fuente espera a que esté cargada.
- Antes de dibujar un texto el juego prepara sus caracteres: lo pasa a
  caracteres de 32 bits traduciendo las órdenes de barra y letra (2.047
  como mucho; lo que sobra se corta), lo recorre con el iterador y pide a la
  fuente cada carácter que se dibuja. Dice si pudo con todos; aunque uno
  falle, sigue con los demás.
- Una barra seguida de una letra que no es una orden se queda como barra, y
  la letra se dibuja.

### Sin comprobar

- Con qué estilo, alineación y colores se hace el iterador al preparar los
  caracteres: no cambian qué caracteres salen, y el iterador está en la pila
  de la función (cuatro mutaciones).

### Mutaciones

50 líneas rotas a propósito: 46 detectadas (una porque la prueba se detiene
al leer un color que no hay), y las cuatro de «Sin comprobar».

## 2026-10-02 — Empezar a recorrer un texto

Una función. De 563 funciones se pasa a 564, el 5,01 % de las del juego (el
2,505 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `text_iterator_new` | `0x0013f470` | `text/draw_string.c` |

- Las variables `draw_string_flags` (`0x004e73a4`) y `font_slot_indices`
  (`0x004e28f4`).

### Lo que se sabe

- El iterador guarda la fuente (su número y la fuente misma, que se pide a
  la función que espera a que esté cargada), la alineación, y los colores del
  texto y de su sombra ya como números de 32 bits.
- Cada parte de un color (de 0 a 1) se multiplica por 255 y se **corta** lo
  que queda tras la coma, sin redondear; una parte fuera de rango pisa a las
  demás, porque no se recorta.
- La sombra nunca es más opaca que el texto: su opacidad es la menor de las
  dos. Sin sombra, su color es 0.
- Si las líneas se parten donde se puede lo dice el bit 0 de las banderas de
  cómo se dibuja el texto.
- La función `0x00322b0c`, que aparece ocho veces, es la de la biblioteca de
  C que pasa un real a entero.

### Sin comprobar

- Quedarse con el byte bajo de la opacidad antes de juntarla con las otras
  partes: lo que sobra se pierde al desplazar.

### Mutaciones

29 líneas rotas a propósito: 28 detectadas (una de ellas, que no compilaba,
se repitió bien hecha), y la otra es la de «Sin comprobar».

## 2026-10-02 — Dónde está cada píxel de un bitmap: el 5 % del juego

Seis funciones. De 557 funciones se pasa a 563: **el 5,00 % de las del
juego** (el 2,495 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `real16_to_real` | `0x00135880` | `bitmaps/bitmaps.c` |
| `bitmap_format_get_bits` | `0x001358c0` | `bitmaps/bitmaps.c` |
| `bitmap_delete` | `0x001359d0` | `bitmaps/bitmaps.c` |
| `bitmap_2d_get_pixel_address` | `0x00135a30` | `bitmaps/bitmaps.c` |
| `bitmap_3d_get_pixel_address` | `0x00135af0` | `bitmaps/bitmaps.c` |
| `bitmap_cube_map_get_pixel_address` | `0x00135c00` | `bitmaps/bitmaps.c` |

### Lo que se sabe

- Los píxeles de un bitmap están seguidos: un mipmap detrás de otro y, en
  cada uno, las filas una detrás de otra. En uno de tres dimensiones van
  además las capas seguidas, y en un mapa cúbico las seis caras de cada
  mipmap. La dirección de un píxel es la de los píxeles más los bits que hay
  antes de él, entre 8 redondeando hacia cero.
- De un mipmap al siguiente cada medida es la mitad, sin bajar del mínimo: 1,
  o 4 en un bitmap comprimido. El fondo de uno de tres dimensiones es su byte
  bajo y no baja de 1.
- Los números reales de 16 bits del juego no son los habituales: el signo es
  el bit alto, el **exponente son los 5 bits bajos** y la fracción está en
  medio. No se distingue ningún número, ni el cero.
- Borrar un bitmap es sacarlo de la caché de texturas, soltar su textura y,
  si lo hizo el juego (el bit 8 de sus banderas, que no tenía nombre),
  devolver a la consola la memoria de sus píxeles y la suya.

### Sin comprobar

- En `real16_to_real`, tomar el número con signo o sin él: los bits de más
  se pierden al desplazar.
- Que la mitad sea «mayor o igual» o «mayor» que el mínimo, y el fondo de 2
  o 3 capas: el resultado es el mismo de las dos formas (dos mutaciones).

### Mutaciones

68 líneas rotas a propósito: 62 detectadas a la primera (dos de ellas porque
el número real que sale no cabe en uno de 32 bits y la prueba se detiene),
tres más con casos nuevos (un píxel antes del primero, con 4 bits por píxel),
y las tres de «Sin comprobar».

## 2026-10-02 — Recorrer un texto, y cuánto ocupan los píxeles de un bitmap

Siete funciones. De 550 funciones se pasa a 557, el 4,94 % de las del juego
(el 2,470 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `text_iterator_next` | `0x0013f5a0` | `text/draw_string.c` (nuevo) |
| `character_is_private_picture` | `0x0013f660` | `text/draw_string.c` |
| `bitmap_pixels_size` | `0x00136490` | `bitmaps/bitmaps.c` (nuevo) |
| `bitmap_mipmap_pixel_count` | `0x001364e0` | `bitmaps/bitmaps.c` |
| `bitmap_row_size` | `0x001365a0` | `bitmaps/bitmaps.c` |
| `bitmap_dimensions_mipmap_size` | `0x00136600` | `bitmaps/bitmaps.c` |
| `bitmap_dimensions_size` | `0x001366d0` | `bitmaps/bitmaps.c` |

- `tests/test_draw_string.py`.
- La tabla `bitmap_format_bits_per_pixel` (`0x00453550`): los bits por píxel
  de los 24 formatos.

### Lo que se sabe

- Un texto se dibuja recorriéndolo con un iterador (44 bytes) sobre
  caracteres de 32 bits. Cada paso da una de cinco cosas: un carácter que se
  dibuja, un fin de línea (`\r`), un tabulador, un cambio de alineación
  (`0xE405` a `0xE407`) o el final. El salto de línea (`\n`) se pasa sin
  más. De cada carácter que se dibuja se apunta si la línea puede acabar
  antes de él, cuando el texto se parte en líneas.
- Los caracteres de `0xE000` a `0xE3FF` son dibujos propios del juego (los
  botones del mando y demás), salvo `0xE000`–`0xE002`, `0xE004`, `0xE008` y
  `0xE106`–`0xE131`.
- Un mipmap mide la mitad que el anterior y nunca menos de 1. Un bitmap
  comprimido (el bit 1 de sus banderas, que no tenía nombre) redondea ancho
  y alto a múltiplos de 4. Un mapa cúbico tiene seis caras. Del fondo solo se
  mira el byte bajo.
- Los bytes salen de multiplicar píxeles por bits y dividir entre 8
  redondeando hacia cero; los de una fila se pueden alinear a una potencia de
  dos y son un número de 16 bits.
- `bitmap_row_size` ya se usaba desde la caché de texturas como función sin
  descompilar.

### Sin comprobar

- El límite inferior de los dibujos propios (`0xE000`): los tres primeros
  están excluidos de todas formas, así que no cambia nada (tres mutaciones).
- Que un tamaño reducido sea «mayor que 1» o «mayor que 0», y lo mismo con
  el fondo: con 1 el resultado es 1 de las dos formas (dos mutaciones).
- Dividir entre 8 o desplazar 3 bits los bytes de una fila: el producto
  nunca es negativo (dos mutaciones). En el total de un bitmap sí puede
  serlo, cuando la cuenta de píxeles pasa de 32 bits, y está probado.

### Mutaciones

121 líneas rotas a propósito: 113 detectadas a la primera, una más con
casos nuevos (cuentas de píxeles que desbordan), y las siete de «Sin
comprobar».

## 2026-10-02 — Dónde puede acabar una línea de texto

Seis funciones. De 544 funciones se pasa a 550, el 4,88 % de las del
juego (el 2,440 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `binary_search_4byte` | `0x0013dd70` | `memory/binary_search.c` |
| `usnprintf` | `0x0013fb50` | `text/unicode.c` |
| `unicode_escape_character` | `0x0013fbd0` | `text/unicode.c` |
| `character_can_break_line` | `0x0013fd20` | `text/unicode.c` |
| `character_cannot_start_line` | `0x0013fde0` | `text/unicode.c` |
| `character_cannot_end_line` | `0x00140260` | `text/unicode.c` |

- Las variables `unicode_escape_note_time` (`0x0055e730`) y
  `white_space_ranges` (`0x004536cc`).

### Lo que se sabe

- En los textos del juego una barra «|» seguida de una letra es una orden:
  `|n` fin de línea, `|t` tabulador, `|l`, `|r` y `|c` alinear a la
  izquierda, a la derecha o al centro lo que sigue (son los caracteres
  `0xE405`, `0xE406` y `0xE407`), y `||` la barra misma. Una letra que no es
  ninguna de esas se anota, como mucho una vez por minuto, y en esta versión
  la nota no va a ningún sitio.
- Una línea puede acabar detrás de un carácter si es un espacio (doce rangos
  de Unicode) o si él o el siguiente son de escritura china, japonesa o
  coreana, y además el carácter puede acabar línea y el siguiente puede
  empezarla.
- No pueden empezar línea 80 caracteres: lo que cierra (paréntesis, comillas,
  puntos y comas) y las letras pequeñas del japonés. No pueden acabarla 26:
  lo que abre. Están los 106 en el código del juego como un salto por casos;
  aquí son una lista.
- `usnprintf` es `usnzprintf` sin poner el final: si el texto no cabe, queda
  sin terminar.
- La búsqueda binaria de números de 4 bytes da a la función que compara los
  números mismos, no punteros a ellos.

### Sin comprobar

- El último de los doce rangos de espacios (`0x3000`): es además un carácter
  de escritura asiática, así que la línea puede acabar tras él con el rango
  o sin él.
- `usnprintf` con `%c` y `%s`, como en `usnzprintf`; y lo que devuelve, que
  el juego no usa (aquí no devuelve nada).

### Mutaciones

162 líneas rotas a propósito, entre ellas cada uno de los 106 caracteres:
161 detectadas, y la otra es la primera de «Sin comprobar».

## 2026-10-02 — Crear un perfil y buscarlo por su nombre: `player_profile.c`, completo

Dos funciones. De 542 funciones se pasa a 544, el 4,83 % de las del juego (el
2,403 % de sus bytes). Con ellas están las 11 funciones de
`player_profile.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `player_profile_create` | `0x001a03a0` | `saved games/player_profile.c` |
| `player_profile_find` | `0x001a0560` | `saved games/player_profile.c` |

- `_wcsicmp` en la biblioteca de C de las pruebas (solo distingue las letras
  de ASCII).

### Lo que se sabe

- Un perfil nuevo se crea así: se crea su archivo en el disco duro con el
  nombre; se hace un perfil de invitado; se le quita la marca de «por
  defecto»; se le dan **al azar** cuatro colores (de los 16 primeros), el
  dibujo del emblema (de 64) y su fondo (de 32), con la segunda semilla del
  juego; se le pone el nombre (31 caracteres como mucho); y se escribe. Si
  no se puede escribir, se borra el archivo.
- El primer argumento de `player_profile_create` no se mira.
- Buscar un perfil por su nombre es recorrer los perfiles del disco duro y
  los de serie y comparar sin distinguir mayúsculas de minúsculas. Con el
  nombre vacío el perfil es uno de invitado, y no hay archivo.
- La lista de perfiles se pide con sitio para 4.096, en la pila: la función
  reserva más de 16 KB.

### Sin comprobar

- Nombres con letras que no sean de ASCII: la comparación de la biblioteca
  de las pruebas no es la de la consola.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Escribir el estado de una partida guardada: `saved_game_file_tasks.c`, completo

Una función. De 541 funciones se pasa a 542, el 4,81 % de las del juego (el
2,381 % de sus bytes: la función no está en la lista de Ghidra con la que se
cuentan los bytes, y no suma). Con ella están las 20 funciones de
`saved_game_file_tasks.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_write_work` | `0x002adb10` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El archivo del estado se escribe en cuatro pasos, uno cada vez que el hilo
  de las tareas le da el turno: (0) se abre y se escriben **ceros** donde va
  el principio; (1) se escribe el resto, detrás; (2) se escribe el
  principio; (3) se escribe la firma de los dos. Así un archivo que no se
  llegó a escribir entero no tiene un principio que valga.
- Si no hay principio que escribir, la tarea solo hace sitio: si el archivo
  ya es tan grande como hace falta, ha terminado bien; si no, escribe ceros
  al principio y al final de donde va el resto, y una firma de ceros.
- Los ceros salen del búfer de las tareas, a trozos de lo que quepa.
- Si en el paso 0 falla empezar la firma, el paso no avanza y la tarea vuelve
  a abrir el archivo la vez siguiente, sin cerrar el anterior. Es del juego.
- El paso es uno de los cuatro: el juego salta por una tabla sin comprobarlo.

### Sin comprobar

- En el paso 0, avanzar por lo escrito o por el tamaño del búfer: solo
  difieren en el último trozo, y ahí termina el bucle de las dos formas.
- Un búfer de tareas de tamaño cero con principio que escribir: el juego no
  termina nunca.

### Mutaciones

66 líneas rotas a propósito: 63 detectadas a la primera; dos más tras hacer
más frecuentes los fallos de la consola en las pruebas (800 casos); la otra
es la primera de «Sin comprobar».

## 2026-10-02 — Leer el estado de una partida guardada, paso a paso

Una función. De 540 funciones se pasa a 541, el 4,80 % de las del juego (el
2,381 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_read_work` | `0x002ad7c0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El archivo del estado de una partida guardada tiene un principio, el
  resto y, detrás, la firma de 20 bytes de los dos. El hilo de las tareas lo
  lee en tres pasos, uno cada vez que le da el turno: abre el archivo y lee
  el principio; lee el resto; y lee la firma y la compara con la que la
  consola ha ido haciendo de lo leído. Solo si coinciden la lectura salió
  bien.
- Si no se le da dónde dejar el resto, lo lee a trozos al búfer de las
  tareas (`async_buffer_globals`), tantos bytes como quepan cada vez: sirve
  para comprobar la firma sin quedarse con los datos.
- `async_buffer_globals` apunta además quién lo usó el último y si el búfer
  no es el de siempre (su cuarto campo, que faltaba).
- El progreso que ve el jugador es lo leído entre el total (principio,
  resto y firma); al acabar, bien o mal, es 1.
- Si quien pidió la lectura la cancela (el segundo byte del resultado), el
  paso siguiente cierra todo y termina.
- Los tamaños son números sin signo y el juego los pasa a reales como si
  tuvieran signo, sumando 4.294.967.296 a los negativos. Con la precisión
  simple con la que corre el juego esa suma se redondea, y el resultado no
  es el mismo que convirtiendo el número de una vez: hay que hacerlo igual.

### Sin comprobar

- Que el trozo que se lee no pase de lo que queda, escrito de otra forma
  (`>=` lo que queda más uno): es lo mismo salvo con un resto de 4 GB.

### Mutaciones

54 líneas rotas a propósito: 53 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El índice que un archivo tiene ahora: `saved_game_files.c`, completo

Una función. De 539 funciones se pasa a 540, el 4,79 % de las del juego (el
2,358 % de sus bytes). Con ella están las 48 funciones de
`saved_game_files.c`.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_current_index` | `0x002170e0` | `saved games/saved_game_files.c` |

### Lo que se sabe

- El índice de un archivo deja de valer cuando la memoria de los archivos se
  devuelve y se vuelve a pedir (al cambiar de mapa): la sal es otra. Esta
  función da el índice de ahora a partir del de antes, o −1 si ya no se sabe
  cuál es.
- Con la memoria de siempre basta mirar la sal: si es la del índice, vale.
  El juego compara la sal del índice con el número entero de la memoria, sin
  quitarle los bits de más.
- Con la memoria recién pedida se usa lo que se guardó de cada archivo (hasta
  cuatro): un archivo del disco duro es el de su tipo cuyo directorio es el
  guardado, y se vuelve a guardar con el índice nuevo. Uno de serie conserva
  su sitio y solo cambia de sal.
- Si de un archivo del disco duro no se guardó nada, el juego mira lo
  guardado sin comprobar que haya algo (un puntero nulo). Solo se libra si no
  hay ningún archivo de su tipo. Se reproduce tal cual.
- Al volver a guardar, solo se apunta el índice: lo que se sabía del archivo
  es lo que hubiera en ese hueco.

### Sin comprobar

- El caso del puntero nulo, y el de guardar un quinto archivo (escribe fuera
  de la tabla de los guardados): en el juego es memoria ajena.

### Mutaciones

30 líneas rotas a propósito: las 30 detectadas.

## 2026-10-02 — Buscar los archivos de una unidad

Una función. De 538 funciones se pasa a 539, el 4,78 % de las del juego (el
2,343 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_find_unit_files` | `0x002164c0` | `saved games/saved_game_files.c` |

- `struct saved_game_find_data`: lo que la consola cuenta de cada partida
  guardada (`XGAME_FIND_DATA`, 0x344 bytes).

### Lo que se sabe

- El juego recorre las partidas guardadas de la unidad. De cada una prueba
  los once tipos de archivo, por orden, hasta dar con el que existe en su
  directorio (`profile`, `slayer`…): ese es su tipo. El nombre que le puso
  el jugador y el idioma salen del nombre visible. Lo que sabe de ella va a
  la tabla; con la tabla llena (4.096) se deja de buscar.
- Una partida guardada sin archivo de ningún tipo no es de las del juego: se
  compone un mensaje («unrecognized saved game file found by
  XFindNextSaveGame()…») que en esta versión no se enseña en ningún sitio.
- Era la función sin nombre que llaman el bucle principal y
  `saved_game_files_update`.

### Corregido

- `saved_game_files_count_saved_games` reservaba 0x33C bytes para lo que
  cuenta la consola de una partida guardada, y son 0x344. Las pruebas no lo
  veían porque la consola es suplente; en una consola de verdad se habría
  escrito fuera. Además, como en el juego, tiene en sí misma la función que
  da la raíz de una unidad en vez de llamarla.

### Sin comprobar

- El nombre corto del archivo y el relleno de `saved_game_find_data`: el
  juego no los mira.
- El mensaje, pasado a caracteres de 8 bits: no sale de la función.

### Mutaciones

34 líneas rotas a propósito: 33 detectadas, y la otra es la primera de «Sin
comprobar».

## 2026-10-02 — Crear un archivo

Una función. De 537 funciones se pasa a 538, el 4,78 % de las del juego (el
2,321 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_create` | `0x00215c00` | `saved games/saved_game_files.c` |

- `tools/test_functions.py` admite, detrás del nombre del archivo de pruebas,
  los nombres de las funciones cuyos casos se corren; el resto del archivo se
  recorre (para que los casos sean los mismos) pero no se emula.
  `tools/mutation_check.py` los toma de `FUNCTIONS`. Una pasada de mutación
  de `saved_game_files.c` baja de más de dos minutos a unos doce segundos.

### Lo que se sabe

- Crear un archivo en el disco duro es: esperar a que se hayan buscado los
  archivos de las unidades; pedir a la consola una partida guardada nueva con
  el nombre visible (el del tipo en el idioma de los archivos, y el que puso
  el jugador); crear en su directorio el archivo vacío del tipo; escribir su
  imagen; y apuntarlo en la tabla. Si falla cualquier paso después de crear
  la partida guardada, se borra.
- Por qué no se creó queda en `saved_game_files_last_error`: 1 si ya hay
  4.096 archivos en la tabla o 4.096 partidas guardadas en el disco, 2 si
  al disco no le caben los bloques de un archivo de ese tipo, 4 si le caben
  y falló otra cosa; 0 si se creó. El 1 es el mismo de cuando no queda
  nombre libre, así que `_saved_game_files_error_no_name` pasa a llamarse
  `_saved_game_files_error_too_many`.
- De la ruta que da la consola solo se guardan 19 caracteres.

### Sin comprobar

- La comprobación de que el índice recién hecho no sea −1: no puede serlo
  (un índice del disco duro no tiene el bit de los de serie).
- Lo que hace el juego si no se puede hacer el nombre visible: la función que
  lo hace siempre dice que sí, el compilador lo sabe al compilar `source/`, y
  la suplente de la prueba no puede decir otra cosa.

### Mutaciones

55 líneas rotas a propósito: 53 detectadas a la primera; una más tras hacer
que la suplente del nombre visible dé un nombre distinto cada vez (el juego
borra con el segundo nombre, no con el primero); la otra es la primera de
«Sin comprobar».

## 2026-10-02 — La lista de los archivos de un tipo, y cómo se ordena

Cinco funciones. De 532 funciones se pasa a 537, el 4,77 % de las del juego
(el 2,301 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `qsort_4byte` | `0x0013de30` | `cseries/sort.c` (nuevo) |
| `qsort_2byte` | `0x0013df80` | `cseries/sort.c` |
| `short_sort_4byte` | `0x0013e0e0` | `cseries/sort.c` |
| `short_sort_2byte` | `0x0013e140` | `cseries/sort.c` |
| `saved_game_files_enumerate` | `0x00215900` | `saved games/saved_game_files.c` |

- `tests/test_sort.py`.
- En `tools/mutation_check.py`, una mutación puede llevar un cuarto dato: el
  nombre con el que sale en el resultado. Hace falta cuando el texto que se
  cambia es largo porque la línea está repetida en varias funciones.

### Lo que se sabe

- El juego ordena números de 4 y de 2 bytes con el mismo algoritmo que la
  biblioteca de C de su compilador: se pone un número en su sitio, con los
  que no van después de él antes y los demás después, y se sigue con la
  parte pequeña, dejando la grande apuntada para luego (hasta 30). Con 8
  números o menos se va llevando al final el mayor de los que quedan. Los
  nombres `qsort_4byte` y `qsort_2byte` son los de Halo 1.
- A la función que compara se le pregunta si un número va **después** de
  otro, y recibe además un dato de quien ordena. El primer argumento de pila
  de las dos funciones de ordenar no se mira.
- La lista de archivos de un tipo junta los del disco duro, los de las dos
  unidades de memoria de un mando si se da uno (las unidades 1 + 2 × mando y
  la siguiente) y, si se piden, los que vienen con el juego. Con estos
  últimos la lista se ordena: primero los de las unidades y después los de
  serie, cada grupo por su índice.
- Cada unidad tiene su tabla detrás de la anterior, pero la memoria de los
  archivos solo tiene sitio para una: la cuenta de la tabla de la primera
  unidad de memoria es el mismo número que la sal de los índices, y sus
  archivos caen en el búfer de copia. Es del juego; se reproduce tal cual.
- El mando es uno de los cuatro, ninguno (−1) o `0xFF`: el juego salta por
  una tabla sin comprobar que sea uno de ellos.

### Cambiado

- `saved_game_file_index_after` saca ella sus argumentos de la pila, como
  toda función que se le da a otra: es la que compara al ordenar la lista.

### Sin comprobar

- En las funciones de ordenar, lo que pasa con una parte de un solo número:
  no hay nada que ordenar en ella, así que da igual apuntarla o no. Lo mismo
  la comprobación de que haya al menos dos números.
- En la lista, qué unidad sigue cuando no se da mando: solo se mira el disco
  duro y la cuenta no llega a usarse.
- Un mando que no sea uno de los seis valores: en el juego es un salto a
  donde caiga.

### Mutaciones

105 líneas rotas a propósito (67 en las funciones de ordenar y 38 en la
lista): las 105 detectadas.

## 2026-10-02 — Borrar un archivo y buscarle nombre a uno nuevo

Tres funciones. De 529 funciones se pasa a 532, el 4,72 % de las del juego
(el 2,256 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `usnzprintf` | `0x0013fb70` | `text/unicode.c` |
| `saved_game_file_delete` | `0x00215e60` | `saved games/saved_game_files.c` |
| `saved_game_file_get_unused_name` | `0x00215f40` | `saved games/saved_game_files.c` |

- `_vsnwprintf` en la biblioteca de C de las pruebas.

### Lo que se sabe

- Un archivo nuevo se llama por defecto **«Halo0001»**, «Halo0002»…: el
  juego prueba números desde 1 y pregunta a la consola si ya hay una
  partida guardada con ese nombre visible; el primero que no existe es el
  nombre. Si los 4.096 están cogidos, no hay nombre y queda apuntado un
  error.
- Sin disco duro tampoco hay nombre.
- Borrar un archivo es borrar su partida guardada en la consola (por el
  nombre visible, en el idioma en que se guardó) y quitarlo de la tabla.
  Un archivo de serie solo se quita de lo que el juego sabe: no hay nada
  que borrar en ningún disco.

### Sin comprobar

- En `usnzprintf`, si a la función de la biblioteca se le da el tamaño
  entero o uno menos: el último carácter se pone a cero después.
- `usnzprintf` con `%c` y `%s`: la biblioteca de C de las pruebas toma el
  formato como texto de 8 bits y solo sirve para números. En el juego la
  función llama a la de la consola, que sí los admite.

### Mutaciones

32 líneas rotas a propósito: 31 detectadas, y la otra es la primera de
«Sin comprobar».

## 2026-10-02 — La memoria de los archivos guardados

Siete funciones del principio de `saved_game_files.c`, que empieza antes
de lo que se creía (`0x00215790`). De 522 funciones se pasa a 529, el
4,70 % de las del juego (el 2,238 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_update` | `0x00215790` | `saved games/saved_game_files.c` |
| `saved_game_files_set_string_list` | `0x002157c0` | `saved games/saved_game_files.c` |
| `saved_game_files_allocate_memory` | `0x002157e0` | `saved games/saved_game_files.c` |
| `saved_game_files_free_memory` | `0x00215810` | `saved games/saved_game_files.c` |
| `saved_game_files_new` | `0x00215880` | `saved games/saved_game_files.c` |
| `saved_game_file_information_new` | `0x00215b40` | `saved games/saved_game_files.c` |
| `saved_game_file_get_name` | `0x00215b50` | `saved games/saved_game_files.c` |

### Cambiado

- En la tabla de los que piden memoria al cargar un mapa
  (`main_game_memory_users`), la pareja `unknown_00215810` /
  `unknown_002157e0` es `saved_game_files_free_memory` /
  `saved_game_files_allocate_memory`.

### Corregido

- **Solo hay una tabla de archivos**, no una por unidad como se dijo en el
  hito 63. La memoria de los archivos mide 0x6BF00 bytes, y la cuenta sale
  exacta con: los archivos de serie (0xBEF8), una tabla de 4.096 archivos
  (0x40004), la sal de los índices (4) y el búfer de las copias (0x20000).
  Eso explica también por qué la sal estaba «donde estaría la cuenta de la
  segunda tabla» (hito 51): no hay segunda tabla. Los bits 4-7 del índice
  de un archivo siguen siendo la unidad, pero para elegir tabla solo vale
  la 0.

### Lo que se sabe

- La memoria de los archivos se pide al cargar un mapa en la configuración
  de memoria 1, y se suelta con las demás. Antes de soltarla, el juego
  guarda aparte lo que sabe de hasta cuatro archivos, para poder seguir
  contestando por ellos sin la tabla.
- La sal de los índices cambia cada vez que se pide la memoria (de 0 a
  511): así un índice de archivo anterior ya no vale después.
- El juego escribe la sal sin mirar si le dieron memoria: si no se la
  dieran, escribiría en la dirección 0x4BEFC.
- Al fijar la etiqueta de los textos, los archivos de serie se quedan en
  uno solo: el perfil por defecto.
- El nombre de un archivo de serie está dentro del propio archivo: a 8
  bytes en un perfil y a 4 en una variante de juego.
- `saved_game_files_update` es lo que el bucle principal tiene copiado
  dentro de sí (ya estaba descompilado allí, sin saber que era una función
  aparte).

### Mutaciones

57 líneas rotas a propósito: las 57 detectadas.

## 2026-10-02 — Los primeros pasos de un cambio de nombre

Los dos pasos que la máquina de estados llamaba todavía por su dirección.
De 520 funciones se pasa a 522, el 4,63 % de las del juego (el 2,222 % de
sus bytes). El cambio de nombre de un archivo guardado queda descompilado
de principio a fin.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_rename_create` | `0x002ad020` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_rename_open_copy` | `0x002ad220` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El primer paso pide a la consola una partida guardada **nueva** con el
  nombre visible nuevo («tipo: nombre», en el idioma de la consola). Si ya
  hay una con ese nombre, falla.
- Dentro crea el archivo del tipo y le da ya su tamaño (0x1E0, 0x130 o
  0x15CB8 bytes), antes de escribir nada: así se sabe que cabe.
- De la ruta que devuelve la consola el juego se queda con 19 caracteres:
  es lo que mide el directorio en lo que sabe de cada archivo. Basta
  porque la consola da rutas cortas, del estilo de `u:\0123456789AB\`.
- El paso de la copia abre `auxilary.bin` en la partida vieja para leer en
  secuencia, lo crea en la nueva y le da el mismo tamaño. Que la vieja no
  lo tenga no es un fallo: se salta la copia.
- Si algo falla ahí se cierran los dos archivos, y esta vez bien (cada uno
  comparado con -1 y marcado como cerrado), a diferencia del descuido del
  hito 63.

### Sin comprobar

- Si el directorio se copia con 20 o con 19 caracteres: el último se pone
  a cero después.

### Mutaciones

50 líneas rotas a propósito: 49 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El trabajo de la tarea que escribe un archivo

Una función: lo que hace el hilo de tareas para escribir un archivo de los
jugadores. De 519 funciones se pasa a 520, el 4,62 % de las del juego (el
2,193 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_write_work` | `0x002acde0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El trabajo de una tarea puede decir que no ha terminado (devuelve 0), y
  entonces el hilo vuelve a llamarlo. Así se hace el cambio de nombre: un
  paso por llamada, hasta que la máquina de estados dice que ha acabado.
- Solo entonces se escribe el archivo, con su firma, en la ruta que da la
  descripción.
- Si el cambio de nombre falla, la tarea termina sin escribir nada y con el
  error del resultado a 0: lo único que dice que salió mal es que no se
  marca como bien hecha.

### Mutaciones

15 líneas rotas a propósito: las 15 detectadas.

## 2026-10-02 — Los pasos del cambio de nombre

Dos funciones más de `saved_game_file_tasks.c`. De 517 funciones se pasa a
519, el 4,61 % de las del juego (el 2,185 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_rename_step` | `0x002ad5f0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_get_path` | `0x002ad760` | `saved games/saved_game_file_tasks.c` |

### Cambiado

- En `struct saved_game_file_description`, `unknown9C` es `state` (por dónde
  va el cambio de nombre) y `unknown7C` es `debug`.

### Lo que se sabe

- Cambiar de nombre un archivo no se hace de una vez: es una máquina de
  siete estados, y cada vez que el hilo de tareas le da tiempo hace un paso.
  En orden: crear la partida guardada con el nombre nuevo, escribir su
  imagen, abrir el archivo del estado en la vieja y en la nueva, leer un
  trozo, escribirlo (y volver a leer mientras quede), y borrar la vieja.
- Si la partida vieja no tiene archivo de estado que copiar, se salta la
  copia.
- Si un paso falla se dice por qué (2 en los pasos sueltos, 4 en la copia),
  se cierran los dos archivos y se borra la partida nueva. El estado se
  queda donde estaba.
- Un archivo que no cambia de nombre nace ya en el último estado.
- Queda una rama de depuración: con un indicador puesto, el archivo se
  escribiría en `t:\blam` en vez de en su directorio. Nada de lo
  descompilado pone ese indicador.

### Mutaciones

41 líneas rotas a propósito: las 41 detectadas.

## 2026-10-02 — Cambiar de nombre un archivo guardado

Cuatro funciones más de `saved_game_file_tasks.c`. De 513 funciones se
pasa a 517, el 4,59 % de las del juego (el 2,171 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_copy_read` | `0x002ad380` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_copy_write` | `0x002ad400` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_rename_finish` | `0x002ad480` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_copy_close` | `0x002ad5b0` | `saved games/saved_game_file_tasks.c` |

- Función de la consola que se suple: `XDeleteSaveGame` (`0x00331055`).

### Corregido

- **Los bits del índice de un archivo**, que en los hitos anteriores estaban
  mal o sin aclarar: los bits 0-3 son el **tipo** del archivo y los bits
  4-7 la **unidad** en la que está (0 el disco duro, las demás las
  tarjetas). Las tablas de archivos son una por unidad. Las constantes se
  llaman ahora `SAVED_GAME_FILE_INDEX_TYPE_MASK` y
  `SAVED_GAME_FILE_INDEX_UNIT_*`.
- En `struct saved_game_file_description`, los campos de `+0x84` a `+0x98`
  tienen nombre: los dos archivos de la copia, el búfer, el tamaño y hasta
  dónde se ha leído y escrito.

### Lo que se sabe

- La consola nombra el directorio de una partida guardada a partir de su
  nombre visible, así que cambiar de nombre un archivo es **hacer otro y
  copiarlo**: se copia en trozos de 128 KB, con un búfer que está en la
  memoria de los archivos (`+0x4BF00`).
- Al terminar bien, se borra la partida guardada con el nombre viejo y el
  juego apunta el nuevo, en el idioma de la consola. Al terminar mal, lo
  que se borra es la partida con el nombre nuevo. En los dos casos se
  devuelve el turno de operaciones.
- **Descuido del juego** al cerrar los dos archivos de la copia: el segundo
  se da por abierto si no vale 0 (el primero, si no vale -1), y después de
  cerrarlo se marca como cerrado el primero otra vez. No tiene
  consecuencias porque la descripción no se vuelve a usar.

### Sin comprobar

- Que el nombre visible empiece vacío antes de componerlo: la función que
  lo compone lo vacía también.

### Mutaciones

43 líneas rotas a propósito: 41 detectadas a la primera, una más con un
caso nuevo (un nombre viejo que llena todo su sitio) y la otra es la de
«Sin comprobar».

## 2026-10-02 — Los archivos de los jugadores van firmados

Las dos funciones que leen y escriben un archivo entero de los jugadores.
De 511 funciones se pasa a 513, el 4,55 % de las del juego (el 2,151 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_read_signed` | `0x002adf70` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_write_signed` | `0x002ae090` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- Detrás de los bytes de cada archivo (perfil, variante, lista) van 20
  bytes de firma, que la consola calcula con la clave del juego
  (`XCalculateSignature…`).
- Al leer, el juego calcula la firma de lo leído y la compara con la del
  archivo. **Solo si coinciden da la lectura por buena.** Un archivo
  tocado a mano o corrupto se lee entero pero se da por malo.
- El indicador que acompaña a la lectura y a la escritura dice si la firma
  vale solo para esta consola o para cualquiera.
- Un resultado lleva apuntado hasta dónde se llegó: 3 al leer y 2 al
  escribir, que se cambian por 0 al acabar bien.
- La lectura es la de `file_read` copiada dentro de la función, con una
  diferencia: lo que la consola no escriba en «bytes leídos» si falla es la
  dirección de la referencia del archivo, y eso es lo que se sumaría a la
  posición.

### Mutaciones

45 líneas rotas a propósito: las 45 detectadas.

## 2026-10-02 — Las tareas del archivo del estado de una partida

Las dos funciones que `saved_game_files.c` llamaba todavía por su
dirección. De 509 funciones se pasa a 511, el 4,54 % de las del juego (el
2,134 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_state_read_task_new` | `0x002ada70` | `saved games/saved_game_file_tasks.c` |
| `saved_game_state_write_task_new` | `0x002adec0` | `saved games/saved_game_file_tasks.c` |

### Lo que se sabe

- El estado de una partida guardada (`auxilary.bin`) se lee y se escribe en
  dos partes, la cabecera y el resto, cada una con su búfer y su tamaño.
  Las dos van en los 34 bytes de datos de una tarea genérica de `async.c`,
  de prioridad 2.
- Al escribir, si no se da cabecera, la tarea lo lleva apuntado.
- El progreso empieza en 0 para una lectura y en -1 para una escritura; el
  error empieza en 4 en las dos.
- Con esto está descompilado todo el camino desde que `game_state.c` pide
  guardar o cargar hasta que la tarea queda en la lista. Lo que hace
  después el hilo de tareas (`0x002ad7c0` y `0x002adb10`) no lo está.

### Mutaciones

32 líneas rotas a propósito: las 32 detectadas.

## 2026-10-02 — Las tareas de los archivos de los jugadores

Cómo llegan al hilo de tareas las lecturas y escrituras de los archivos de
los jugadores. De 502 funciones se pasa a 509, el 4,52 % de las del juego
(el 2,123 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_custom_task_work` | `0x001a17b0` | `cseries/async.c` |
| `async_task_add_custom` | `0x001a17d0` | `cseries/async.c` |
| `saved_game_file_read_work` | `0x002accd0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_read_task_new` | `0x002acd60` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_write_task_new` | `0x002acee0` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_new` | `0x002acf40` | `saved games/saved_game_file_tasks.c` |
| `saved_game_file_description_new_renamed` | `0x002acf80` | `saved games/saved_game_file_tasks.c` |

- Archivos nuevos: `saved games/saved_game_file_tasks.c` y
  `tests/test_saved_game_file_tasks.py`.
- `struct async_custom_task` y `struct saved_game_file_description`.

### Cambiado

- Las cuatro funciones que `saved_game_files.c` llamaba por su dirección
  (`unknown_002acd60`, `unknown_002acee0`, `unknown_002acf40`,
  `unknown_002acf80`) tienen nombre y están descompiladas.

### Lo que se sabe

- Además de las tareas fijas de `async.c` (leer, escribir, copiar…), hay
  una genérica: sus parámetros son una función y hasta 35 bytes de datos
  para ella, que se copian. Así cualquier módulo puede mandar trabajo al
  hilo de tareas sin que `async.c` lo conozca.
- Si los datos no caben, esa función no hace nada y devuelve la dirección
  de los datos en lugar de un identificador de tarea: quien la llama solo
  comprueba que no sea -1, así que no se entera.
- La lectura y la escritura de un archivo de los jugadores son tareas de
  esas, de prioridad 2. Sus datos son un búfer, un tamaño, un indicador y
  el resultado, que es también la bandera de «terminado» de la tarea.
- El trabajo de una lectura empieza dando el archivo por no leído, con el
  error 4, y lee del archivo cuya ruta va dentro del propio resultado.
- De un archivo que se escribe se le dice al hilo qué archivo es y lo que
  el juego sabe de él; si además cambia de nombre, el nombre nuevo, el
  directorio que tenía y el turno de operaciones.

### Mutaciones

66 líneas rotas a propósito: las 66 detectadas.

## 2026-10-02 — Los perfiles de los jugadores

Nueve funciones, casi todas de los perfiles. De 493 funciones se pasa a
502, el 4,46 % de las del juego (el 2,099 % de sus bytes). **Se pasa de
las 500 funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `player_appearance_valid` | `0x00153750` | `saved games/player_profile.c` |
| `player_profile_load` | `0x001a0540` | `saved games/player_profile.c` |
| `player_profile_write` | `0x001a0660` | `saved games/player_profile.c` |
| `player_profile_write_async` | `0x001a0680` | `saved games/player_profile.c` |
| `player_color_get` | `0x001a06a0` | `saved games/player_profile.c` |
| `player_profile_get_unknownEC` | `0x001a06f0` | `saved games/player_profile.c` |
| `player_profile_new` | `0x001a07b0` | `saved games/player_profile.c` |
| `player_profile_read` | `0x001a0850` | `saved games/player_profile.c` |
| `async_set_file_position` | `0x001a08d0` | `cseries/async.c` |

- `struct player_appearance` (16 bytes) y más campos de
  `struct player_profile`; `tests/test_player_profile.py`.

### Lo que se sabe

- Un perfil nuevo se llama «Guest». El de serie del hito anterior es el
  mismo con otro nombre.
- El aspecto de un jugador son cuatro colores (de 18, o -1 para ninguno),
  un quinto valor de 0 a 3, un emblema con 64 primeros planos y 32 fondos,
  y cuatro indicadores. Por defecto: colores 10, 0, 0 y 10.
- Al leer un perfil el juego corrige lo que no puede ser: un aspecto
  imposible se pone entero a cero, y el nombre se corta a 31 caracteres.
  **Lo hace aunque la lectura falle**, sobre el perfil que le dieron.
- Lo que se escribe de un perfil empieza en su nombre (`+0x08`), que es lo
  que la función de escritura compara para saber si el archivo cambió de
  nombre.
- Los colores de los jugadores están en la etiqueta de globales
  (`+0x160`); sin mapa cargado, o con un índice que no existe, el color es
  blanco.
- Un perfil guarda cuatro números (`+0xEC`); una función da el mayor de
  los que valen y cuál de los cuatro es. Qué son no se sabe (los niveles
  alcanzados en cada dificultad, quizá).

### Sin comprobar

- Si el indicador de un perfil nuevo se pone con `|=` o con `=`: la memoria
  se acaba de poner a cero.

### Mutaciones

72 líneas rotas a propósito: 71 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los archivos que vienen con el juego

Cinco funciones pequeñas: cuatro de `saved_game_files.c`, al que ya solo
le faltan dos, y la primera de los perfiles. De 488 funciones se pasa a
493, el 4,38 % de las del juego (el 2,078 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_make_directory` | `0x00216440` | `saved games/saved_game_files.c` |
| `saved_game_files_add_default_profile` | `0x00216760` | `saved games/saved_game_files.c` |
| `saved_game_files_add_built_in_variant` | `0x00217080` | `saved games/saved_game_files.c` |
| `cached_saved_game_file_delete` | `0x002172a0` | `saved games/saved_game_files.c` |
| `player_profile_set_defaults` | `0x001a0750` | `saved games/player_profile.c` |

- Archivos nuevos: `saved games/player_profile.h` y `player_profile.c`.
- En la herramienta de pruebas, `clean` en un búfer: una función que dice
  qué parte de lo que queda en él se compara.

### Lo que se sabe

- Los archivos «de serie» son hasta 101, cada uno con su tipo delante. El
  juego los llena al arrancar con un perfil por defecto y con las variantes
  de juego que trae.
- El perfil por defecto se llama «Default» («Player» en japonés,
  «Predeterminada» en español…), en el idioma de los archivos.
- De un perfil se sabe el nombre (`+0x08`, 31 caracteres), un indicador que
  se pone al crearlo (`+0x04`, bit 0) y seis bytes con valores por defecto
  (10, 0, 0, 10 en `+0x118`; 3 en `+0x102`): qué son no se sabe todavía.
- **Rareza del juego:** al añadir una variante de serie copia a la tabla
  0x1E4 bytes de una variable local de la que solo ha rellenado 0x134. El
  resto es lo que hubiera en la pila en ese momento. No afecta a nada,
  porque de una variante solo se leen 0x130 bytes.
- De la lista corta de cuatro archivos se quita uno poniendo el último en
  su sitio.

### Sin comprobar

- Si el nombre de un perfil tiene 31 o 30 caracteres: los nombres por
  defecto son cortos y la memoria ya está a cero.

### Mutaciones

36 líneas rotas a propósito: 35 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Arrancar la lectura y la escritura de un archivo

Las dos funciones que el hito anterior llamaba sin estar descompiladas. De
486 funciones se pasa a 488, el 4,33 % de las del juego (el 2,060 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_read_async` | `0x002162b0` | `saved games/saved_game_files.c` |
| `saved_game_file_write_async` | `0x002173d0` | `saved games/saved_game_files.c` |

- De `struct saved_game_file_result`: `progress` (`+0x108`). De
  `struct saved_game_file_information`: `language` (`+0x3D`).
- `struct built_in_saved_game_file`: un archivo de los que vienen con el
  juego, 4 bytes y los 0x1E0 de un perfil.

### Lo que se sabe

- Los archivos que vienen con el juego (los perfiles de serie) no se leen
  de ningún disco: están en memoria y la lectura es una copia, que termina
  en el acto con el progreso a 1.
- Los de una unidad los lee otro hilo, del archivo que se llama como su
  tipo (`profile`, `slayer`…) dentro de su directorio.
- Un archivo cuyo nombre visible no empieza por el de su tipo en ningún
  idioma (idioma 0xFF) no se lee.
- Lo que se escribe empieza por el nombre del archivo. Si ese nombre ya no
  es el que el juego tenía apuntado, la escritura es además un cambio de
  nombre, y para eso se coge el turno de operaciones.
- El progreso de una operación va de 0 a 1, y vale -1 antes de empezar.

### Mutaciones

47 líneas rotas a propósito: las 47 detectadas.

## 2026-10-02 — Leer y escribir un archivo de los jugadores

Tres funciones más de `saved_game_files.c`. De 483 funciones se pasa a
486, el 4,31 % de las del juego (el 2,045 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_name_unique` | `0x00216120` | `saved games/saved_game_files.c` |
| `saved_game_file_read` | `0x002161d0` | `saved games/saved_game_files.c` |
| `saved_game_file_write` | `0x00216240` | `saved games/saved_game_files.c` |

- `saved_game_files_last_error` (`0x0055c154`), y el campo `error` de
  `struct saved_game_file_result`.
- Función de la consola que se suple: `XCreateSaveGame` (`0x00330df2`).

### Lo que se sabe

- Para saber si un nombre está libre, el juego no recorre sus tablas: le
  pide a la consola que **abra** la partida guardada que se llamaría así
  (con el nombre visible, «tipo: nombre»), y el nombre está libre si la
  consola no la encuentra. La ruta que la consola devolvería se tira.
- Sin nombre, sin la memoria de los archivos preparada o sin disco duro, la
  respuesta es que no está libre.
- Leer y escribir un archivo lo hace otro hilo; estas dos funciones lo
  piden y esperan, atendiendo el sonido. Si sale mal, el código de error de
  la consola queda guardado para quien quiera enseñarlo.

### Sin comprobar

- En `saved_game_file_name_unique`, que el nombre visible empiece vacío:
  la función que lo compone lo vacía también.

### Mutaciones

29 líneas rotas a propósito: 28 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — El nombre visible de un archivo guardado

Dos funciones más de `saved_game_files.c`: la que compone el nombre que la
consola enseña de un archivo y la que lo descompone. De 481 funciones se
pasa a 483, el 4,29 % de las del juego (el 2,032 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_display_name` | `0x00216da0` | `saved games/saved_game_files.c` |
| `saved_game_file_parse_display_name` | `0x00216e50` | `saved games/saved_game_files.c` |

- Globales con nombre: `saved_game_files_string_list_index` (`0x0055c158`),
  `saved_game_files_language` (`0x0055c280`) y
  `saved_game_file_name_separator` (`0x00470054`, que apunta a `": "`).

### Lo que se sabe

- En el panel de la consola un archivo se llama «tipo: nombre», por ejemplo
  «Slayer: mi variante», con el tipo en el idioma en que se guardó. Hasta
  127 caracteres.
- Si no hay idioma, o la etiqueta de textos no tiene el del tipo, el nombre
  visible es solo el que puso el jugador.
- Para recuperar el nombre del jugador a partir del visible, el juego
  prueba los nueve idiomas, empezando por el suyo y dando la vuelta, hasta
  que el nombre empieza por «tipo: » en alguno. Se queda con lo que sigue,
  16 caracteres como mucho, y con qué idioma era.
- Si no empieza así en ninguno, el idioma es 0xFF y el nombre son los 16
  primeros caracteres del nombre visible.
- Como varios idiomas comparten el nombre de un tipo («Slayer» es igual en
  inglés, japonés, coreano y portugués), el idioma que sale es el primero
  que coincide, no necesariamente aquel en que se guardó.

### Sin comprobar

Cuatro escrituras que el juego hace y que no cambian nada, porque el valor
ya estaba puesto: el cero final tras añadir el separador, el cero inicial
y el final del nombre del tipo al descomponer, y el cero tras copiar el
nombre encontrado.

### Mutaciones

41 líneas rotas a propósito: 37 detectadas, y las otras cuatro son las de
«Sin comprobar».

## 2026-10-02 — Los textos del juego por idioma

Cómo encuentra el juego un texto en el idioma de la consola. De 475
funciones se pasa a 481, el 4,27 % de las del juego (el 2,017 % de sus
bytes: **se pasa del 2 % de los bytes**).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `unicode_string_list_has_string` | `0x001a0120` | `text/unicode_string_list.c` |
| `unicode_string_list_get_string` | `0x001a0180` | `text/unicode_string_list.c` |
| `language_texts_load` | `0x001a01e0` | `text/unicode_string_list.c` |
| `language_texts_find` | `0x001a02d0` | `text/unicode_string_list.c` |
| `language_texts_has` | `0x001a0310` | `text/unicode_string_list.c` |
| `language_texts_get` | `0x001a0350` | `text/unicode_string_list.c` |

- Archivos nuevos: `text/unicode_string_list.h`,
  `text/unicode_string_list.c`, `tests/test_unicode_string_list.py`.

### Corregido

- **`hud_get_text` recibía un argumento de menos.** La función del juego
  toma tres cosas: el índice de la etiqueta que tiene el texto (en `eax`),
  el identificador del texto y dónde escribirlo. `main.c` solo le pasaba
  las dos últimas. Las pruebas no lo veían porque la función estaba
  suplida y el registro no se comparaba. Ahora se llama
  `unicode_string_list_get_string`, recibe el índice, y la prueba de
  `main.c` comprueba que es el que debe ser.

### Cambiado

- `unknown_001a01e0`, que `main_game.c` llamaba al cargar un mapa, es
  `language_texts_load`.

### Lo que se sabe

- Los textos de todas las etiquetas de un mapa están juntos, por idioma, en
  el archivo del mapa. La etiqueta de globales dice dónde (una tabla de
  0x1C bytes por idioma, desde `+0x188`), y se leen a memoria al cargar el
  mapa, en sectores enteros de 512 bytes.
- De cada texto se guarda su identificador y dónde empieza; los textos
  están en UTF-8 y se pasan a 16 bits al pedirlos, con 255 caracteres como
  mucho.
- Una etiqueta con textos solo dice, para cada idioma, cuántos textos son
  suyos y a partir de cuál: buscar un texto es recorrer ese tramo.
- El idioma se pregunta a la consola la primera vez que hace falta.
- Si el mismo identificador está dos veces en el tramo, vale el primero.

### Mutaciones

56 líneas rotas a propósito: 55 detectadas a la primera y la otra con un
caso nuevo (un tramo que empieza antes del primer texto).

## 2026-10-02 — Los nombres por defecto de los archivos

El nombre que recibe un archivo de cada tipo cuando el jugador no le pone
ninguno, en cada idioma. De 461 funciones se pasa a 475, el 4,22 % de las
del juego (el 1,996 % de sus bytes).

### Añadido

| Función | Dirección |
| --- | --- |
| `saved_game_file_default_name_profile` | `0x00217620` |
| `saved_game_file_default_name_slayer` | `0x00217680` |
| `saved_game_file_default_name_koth` | `0x002176e0` |
| `saved_game_file_default_name_race` | `0x00217740` |
| `saved_game_file_default_name_oddball` | `0x002177a0` |
| `saved_game_file_default_name_juggernaut` | `0x002177f0` |
| `saved_game_file_default_name_headhunter` | `0x00217840` |
| `saved_game_file_default_name_ctf` | `0x002178a0` |
| `saved_game_file_default_name_assault` | `0x00217900` |
| `saved_game_file_default_name_territories` | `0x00217960` |
| `saved_game_file_default_name_playlist` | `0x002179c0` |
| `saved_game_file_default_name_unknown` | `0x00217a20` |
| `saved_game_file_get_default_name` | `0x00217a80` |
| `saved_game_file_name_length` | `0x00217b70` |

Todas en el archivo nuevo `saved games/saved_game_file_names.c`, con
`tests/test_saved_game_file_names.py`.

- En la herramienta de pruebas, el tipo de resultado `wstring`: como
  `string`, para textos de caracteres de 16 bits.

### Lo que se sabe

- Cada tipo de archivo tiene su función, que es un `switch` del idioma.
  Las doce funciones se han escrito con un script que lee las tablas de
  salto y los textos del ejecutable, no a mano.
- El juego traduce los nombres al japonés, alemán, francés, español,
  italiano, coreano y chino, pero **no todos a todos**: «Slayer» solo
  cambia en alemán («Showdown»), francés, español, italiano y chino;
  «Juggernaut» solo en alemán («Moloch»), francés y chino. El japonés y el
  coreano solo traducen «Profile» y «Playlist».
- El portugués, el noveno idioma de los textos del juego, está en todos los
  `switch` y en todos recibe el nombre en inglés.
- Los modos `race` y `headhunter`, que no salieron en el juego, tienen sus
  nombres traducidos igual que los demás.
- Hay un duodécimo texto («Default», «Player» en japonés) que no es de
  ningún tipo de archivo: su identificador es `0x1B0006E8`.

### Sin comprobar

- Si el nombre se copia con 255 o con 256 caracteres: todos los nombres son
  cortos y después se pone el cero final.

### Mutaciones

176 líneas rotas a propósito: 174 detectadas. De las otras dos, una es la
de «Sin comprobar» y la otra señaló una constante que no se usaba
(`_language_portuguese`): se ha quitado.

## 2026-10-02 — Crear archivos y la imagen de una variante

Tres funciones de archivos que faltaban y la que escribe la imagen de una
variante de juego. De 457 funciones se pasa a 461, el 4,09 % de las del
juego (el 1,966 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_reference_create_from_path` | `0x00136710` | `tag_files/files_xbox.c` |
| `find_files` | `0x00136770` | `tag_files/files_xbox.c` |
| `file_create` | `0x001367d0` | `tag_files/files_xbox.c` |
| `saved_game_file_write_image` | `0x00216f80` | `saved games/saved_game_files.c` |

- `file_reference_set_path`, en línea en `files.h`: el juego la tiene
  copiada donde hace una referencia a partir de una ruta.

### Corregido

- **El módulo de archivos no estaba completo**, como se dijo en el hito 44.
  Las tres primeras funciones de la tabla estaban antes de `file_delete` y
  no se habían visto; aparecieron al seguir una llamada desde
  `saved_game_files.c`. ESTADO.md lo recoge, con las que quedan por mirar.

### Lo que se sabe

- `file_create` hace un archivo vacío (y vacía el que hubiera) o un
  directorio, según la referencia sea de archivo o no.
- `find_files` es el envoltorio de las tres funciones de búsqueda: llena
  una lista de referencias con lo que hay en un directorio (`*.*`), hasta
  un máximo.
- Junto a cada variante de juego guardada se escribe `SaveImage.xbx`, la
  imagen que la consola enseña en su panel: 4.096 bytes que están dentro
  del ejecutable (`0x00449038`). Los perfiles y las listas no llevan
  imagen.
- **Esa imagen no se copia al repositorio**: es un recurso del juego, no
  código. En `source/` queda declarada y vacía; las pruebas le dan el
  mismo contenido al juego y a `source/`.

### Sin comprobar

- En `file_create`, que la ruta completa se ponga a cero antes de
  escribirla entera.
- En `saved_game_file_write_image`, el cero al final de la ruta antes de
  añadirle el nombre: `csstrcat256` lo pone también.

### Mutaciones

50 líneas rotas a propósito: 48 detectadas, y las otras dos son las de
«Sin comprobar».

## 2026-10-02 — Lo que el juego sabe de cada archivo

Cinco funciones más de `saved_game_files.c`: las que leen, cambian, añaden
y quitan lo que el juego guarda de cada archivo de una unidad. De 452
funciones se pasa a 457, el 4,06 % de las del juego (el 1,947 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_name_differs` | `0x00216060` | `saved games/saved_game_files.c` |
| `saved_game_file_get_information` | `0x00216800` | `saved games/saved_game_files.c` |
| `saved_game_file_set_information` | `0x002168b0` | `saved games/saved_game_files.c` |
| `saved_game_file_add_information` | `0x00216990` | `saved games/saved_game_files.c` |
| `saved_game_file_delete_information` | `0x002169e0` | `saved games/saved_game_files.c` |

- `struct saved_game_file_information` (0x40 bytes: una ruta de 0x14, un
  nombre de 18 caracteres de 16 bits, el tipo) y
  `struct saved_game_file_table` (una cuenta y 4.096 de esas).
- `wcsncmp` en la biblioteca de C de las pruebas.

### Cambiado

- **Corrección del hito anterior:** el campo de los bits 4-7 del índice de
  un archivo no es su tipo, sino en cuál de las tablas está. Las
  constantes `SAVED_GAME_FILE_INDEX_TYPE_*` pasan a llamarse
  `SAVED_GAME_FILE_INDEX_TABLE_*`. Qué distingue una tabla de otra no se
  sabe todavía.
- `unknown_00216800` es `saved_game_file_get_information`.

### Lo que se sabe

- Las tablas empiezan en `+0xBEF8` de la memoria de los archivos
  (`0x0051ea14`) y miden 0x40004 bytes cada una.
- **La «sal» de los índices está justo donde estaría la cuenta de la
  segunda tabla** (`+0x4BEFC`). O solo hay una tabla en la práctica, o las
  dos cosas son la misma; no se puede decidir con lo descompilado.
- Si esa memoria no está preparada, el juego guarda aparte lo de hasta
  cuatro archivos (`0x0055c164`), y de ahí lee.
- Cambiar lo de un archivo no se permite mientras hay otra operación en
  curso, salvo que sea sobre ese mismo archivo y la haya pedido **otro**
  hilo.
- Cuando el cambio se hace en esos cuatro archivos guardados aparte, la
  función contesta que no lo hizo aunque lo haya hecho.
- Una tabla llena (4.096) no admite más; pero la comparación es de
  igualdad, así que con una cuenta mayor el juego escribiría fuera.

### Sin comprobar

- Al borrar el último archivo de una tabla, si se entra o no a mover los
  de detrás: no hay ninguno que mover.
- `saved_game_file_set_information` y `saved_game_file_delete_information`
  con un índice de archivo «integrado» (bit 21): el juego no lo mira.

### Mutaciones

52 líneas rotas a propósito: 49 detectadas a la primera, dos más con
casos nuevos (un puesto mayor que 4.095; una tabla con más archivos de los
que caben) y la otra es la primera de «Sin comprobar».

## 2026-10-02 — Los índices de los archivos y el archivo del estado

Siete funciones más de `saved_game_files.c`. De 445 funciones se pasa a
452, el 4,01 % de las del juego (el 1,926 % de sus bytes). **Se pasa del
4 %.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_file_get_directory` | `0x002160b0` | `saved games/saved_game_files.c` |
| `saved_game_file_index_after` | `0x002163d0` | `saved games/saved_game_files.c` |
| `saved_game_file_index_after_delete` | `0x00217300` | `saved games/saved_game_files.c` |
| `saved_game_files_take_operation` | `0x00217370` | `saved games/saved_game_files.c` |
| `saved_game_auxiliary_read` | `0x002174b0` | `saved games/saved_game_files.c` |
| `saved_game_auxiliary_write` | `0x00217520` | `saved games/saved_game_files.c` |
| `saved_game_files_count_saved_games` | `0x00217590` | `saved games/saved_game_files.c` |

- Funciones de la consola que se suplen: `GetCurrentThreadId`
  (`0x002d2234`), `XFindFirstSaveGame` (`0x00331125`), `XFindNextSaveGame`
  (`0x0033122f`) y `XFindClose` (`0x00331276`).

### Cambiado

- `saved_game_auxiliary_read` y `saved_game_auxiliary_write`, que
  `game_state.c` llamaba sin estar descompiladas, ya lo están; sus
  prototipos pasan de `game_state.h` a `saved_game_files.h`.

### Lo que se sabe

- Un archivo se nombra con un número de 32 bits: la unidad (bits 0-3), el
  tipo (4-7), el número del archivo entre los de su tipo (8-20), un
  indicador de que no está en ninguna unidad sino en la memoria del juego
  (21) y una «sal» de 9 bits (22-30) que cambia cuando cambia la lista.
- Al borrar un archivo, los que iban detrás entre los de su tipo bajan un
  puesto y reciben la sal nueva.
- En las listas, los archivos que vienen con el juego van detrás de los de
  las unidades.
- La partida guardada tiene su estado en `auxilary.bin` (sic), en el
  directorio de la partida.
- Antes de preguntar por un archivo el juego espera a que se hayan buscado
  los de las unidades (`0x0055c14c`), atendiendo el sonido mientras tanto.
- Las operaciones con archivos van de una en una: hay un único «turno»
  (`0x0055c274`) con el archivo y el hilo que lo pidió.

### Sin comprobar

- El cero al final de la ruta en `saved_game_file_get_directory`: la ruta
  que se copia ya lo trae.

### Mutaciones

67 líneas rotas a propósito: 66 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los tipos de archivo de los jugadores

Se empieza `saved_game_files.c`: los archivos que los jugadores guardan en
el disco duro y en las tarjetas de memoria (perfiles, variantes de juego y
listas). De 434 funciones se pasa a 445, el 3,95 % de las del juego (el
1,903 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `saved_game_files_get_unit_root` | `0x00216400` | `saved games/saved_game_files.c` |
| `saved_game_file_type_from_variant` | `0x00216a50` | `saved games/saved_game_files.c` |
| `saved_game_file_type_from_listed_game` | `0x00216ab0` | `saved games/saved_game_files.c` |
| `saved_game_file_type_to_listed_game` | `0x00216b00` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_name` | `0x00216b60` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_string_id` | `0x00216bd0` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_blocks` | `0x00216c40` | `saved games/saved_game_files.c` |
| `saved_game_file_size_to_blocks` | `0x00216c70` | `saved games/saved_game_files.c` |
| `saved_game_files_hard_disk_has_blocks` | `0x00216ce0` | `saved games/saved_game_files.c` |
| `saved_game_file_get_type_path` | `0x00216d70` | `saved games/saved_game_files.c` |
| `saved_game_file_type_get_size` | `0x00217480` | `saved games/saved_game_files.c` |

- Archivos nuevos: `saved games/saved_game_files.h`,
  `saved games/saved_game_files.c`, `tests/test_saved_game_files.py`.
- En la herramienta de pruebas, un tipo de resultado nuevo, `string`: la
  función devuelve un puntero a un texto y lo que se compara es el texto,
  que no está en la misma dirección en el juego y en `source/`.
- Funciones de la consola que se suplen: `XGetDiskClusterSize`
  (`0x002d5f65`) y `GetDiskFreeSpaceExA` (`0x002d5838`).

### Lo que se sabe

- Hay once tipos de archivo, y su nombre es el del directorio donde se
  guardan: `profile`, `slayer`, `koth`, `race`, `oddball`, `juggernaut`,
  `headhunter`, `ctf`, `assault`, `territories` y `playlist`. **`race` y
  `headhunter` están en la tabla aunque Halo 2 no salió con esos modos.**
- Un perfil ocupa 0x1E0 bytes, una variante 0x130 y una lista 0x15CB8.
- El espacio se cuenta en bloques de 16 KB, como lo enseña la consola: los
  bytes del archivo más seis clústeres del disco, redondeado hacia arriba.
- Tres numeraciones distintas para los juegos y funciones para pasar de
  una a otra: la del tipo de archivo, la del motor de la variante (campo
  `+0x44`: 1 ctf, 2 slayer, 3 oddball, 4 koth, 7 juggernaut, 8 territories,
  9 assault) y el orden en que salen en la lista (slayer, oddball,
  juggernaut, koth, ctf, assault, territories).
- Esas tres funciones de conversión no comprueban nada: con un valor que
  no sea de los siete juegos, el juego salta a una dirección cualquiera.

### Sin comprobar

- Las tres conversiones con valores que no son de los siete juegos (el
  juego se cae).
- En `saved_game_file_get_type_path`, el cero que se pone al final de la
  ruta: `csstrcat256` lo pone también.

### Mutaciones

81 líneas rotas a propósito: 80 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Atender el sonido y pedir sus muestras

Cinco funciones más del sonido. De 429 funciones se pasa a 434, el 3,85 %
de las del juego (el 1,881 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_idle` | `0x00125d60` | `sound/sound_manager.c` |
| `sound_load_environment` | `0x00125df0` | `sound/sound_manager.c` |
| `sound_source_request_data` | `0x00125e60` | `sound/sound_manager.c` |
| `sound_unknown_00125ef0` | `0x00125ef0` | `sound/sound_manager.c` |
| `sound_predict` | `0x00125f10` | `sound/sound_manager.c` |

- De la etiqueta común de sonidos de un mapa se conocen tres bloques más:
  los rangos de tono (`+0x20`, 0xC bytes), las grabaciones (`+0x28`, 0x10
  bytes) y los trozos de muestras (`+0x40`, 0xC bytes).

### Cambiado

- **`main_loading_idle` se llama ahora `sound_idle`**. El nombre anterior
  era una suposición sacada de quién la llama (las esperas de la carga, de
  las cachés y de las partidas guardadas); al descompilarla resulta que lo
  que hace es atender el sonido. Cambia en ocho archivos de `source/` y en
  siete de `tests/`.
- `unknown_00125df0` es `sound_load_environment`.

### Lo que se sabe

- `sound_idle` solo hace algo dentro de una partida y si ha pasado más de
  un tick (33 ms a 30 por segundo) desde que el sonido empezó o se
  reanudó. No se vuelve a entrar mientras ya se está dentro.
- Un sonido se compone así: sonido → rangos de tono → grabaciones
  (permutaciones) → trozos de muestras. Cada nivel guarda el índice del
  primero del siguiente, y un sonido que suena guarda cuánto se ha
  desplazado en cada nivel (`+0x9C`, `+0x9D`, `+0x9E`).
- Pedir muestras es pedir un trozo a la caché de sonido (`0x00218850`), que
  contesta con bits: con el 0 o el 1 se avisa al sonido de que no las
  tiene, y el 1 es lo que devuelve la función.
- `sound_predict` pide el primer trozo de un sonido antes de que suene.
- El «ambiente» del mapa sale de una etiqueta que nombra la etiqueta de
  globales (`+0x14` de su bloque de sonido); puede no haberla.

### Mutaciones

71 líneas rotas a propósito: 69 detectadas a la primera y las otras dos
con un caso nuevo (un tick de un segundo justo, para distinguir 1000 de
1001 y de 990).

## 2026-10-02 — El sonido al cambiar de mapa

El resto de la fila del sonido en la tabla de sistemas del juego: lo que
hace al cargar un mapa, al soltarlo y al cambiar de BSP; y dos funciones
más, si hay sonido y pararlo o reanudarlo. De 424 funciones se pasa a 429,
el 3,81 % de las del juego (el 1,867 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_initialize_for_new_map` | `0x00125690` | `sound/sound_manager.c` |
| `sound_dispose_from_old_map` | `0x001257b0` | `sound/sound_manager.c` |
| `sound_initialize_for_new_structure_bsp` | `0x00125920` | `sound/sound_manager.c` |
| `sound_available` | `0x00125a60` | `sound/sound_manager.c` |
| `sound_enable` | `0x00125a90` | `sound/sound_manager.c` |

### Cambiado

- `unknown_00125690`, `unknown_001257b0` y `unknown_00125920` de la tabla
  de sistemas tienen nombre: la fila del sonido está completa.
- De un oyente (`struct sound_listener`) se sabe más: la hoja y el cluster
  del BSP donde está, y su posición (`+0x30`).

### Lo que se sabe

- Al cargar un mapa, el sonido apunta a la etiqueta común de sonidos del
  mapa (su índice está en el bloque de sonido de la etiqueta de globales,
  en `+0x20`), apunta la hora y quita los oyentes.
- Al soltar un mapa, los sonidos que suenan se marcan para apagarse en
  300 ms y el juego **espera** esos 300 ms atendiendo el sonido. Si no
  suena ninguno no espera.
- En esa espera la hora de inicio se toma como número con signo y la hora
  actual sin signo: si la consola llevara más de 24,8 días encendida, la
  espera no ocurriría. El final se guarda como real de 32 bits, así que
  pasadas 4,6 horas ya no es exacto al milisegundo.
- Al cambiar de BSP se recalcula dónde están los sonidos que tienen
  posición (los del tipo 1) y los oyentes.
- «Hay sonido» es: está preparado, hay un mapa cargado y otro indicador
  (`+0x79`) que se pone al arrancar.
- `sound_enable(0)` para todos los canales; `sound_enable(1)` los reanuda y
  apunta la hora. Cualquier otro número cuenta como «reanudar» para el
  indicador pero no toca los canales.

### Sin comprobar

- `sound_initialize_for_new_map` cuando la etiqueta de globales no dice
  nada del sonido: el juego lee de la dirección 0x20 y se cae.

### Mutaciones

113 líneas rotas a propósito: 110 detectadas a la primera, dos más con
casos nuevos (un tipo de sonido con el bit 3 puesto; cargar un mapa sin
otro cargado antes) y la otra es la de «Sin comprobar».

Las pruebas encontraron un error antes de las mutaciones: había puesto la
espera de 300 ms fuera de la condición de que suene algún sonido.

## 2026-10-02 — El arranque del sonido

Las dos funciones que preparan y desmontan el sonido al arrancar y cerrar
el juego, y dos auxiliares que usan. De 420 funciones se pasa a 424, el
3,76 % de las del juego (el 1,850 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_initialize` | `0x001253d0` | `sound/sound_manager.c` |
| `sound_dispose` | `0x00125600` | `sound/sound_manager.c` |
| `shell_memory_data_new` | `0x0011cc20` | `shell/shell.c` |
| `bit_vector_fill` | `0x0012afc0` | `cseries/cseries.c` |

- `shell_memory_allocate`, en línea en `shell.h`: el juego la tiene copiada
  dentro de cada función que pide memoria.
- Globales con nombre: `sound_initialize_count`, `sound_channels`,
  `sound_sources`, `looping_sounds`, `sound_voices`, `sound_effects` y la
  tabla `sound_configuration`.

### Cambiado

- `unknown_001253d0` y `unknown_00125600`, que el arranque y la tabla de
  sistemas del juego llamaban sin saber qué eran, son `sound_initialize` y
  `sound_dispose`.
- En `shell_memory_globals`, los campos `unknown8` y `unknownC` son `size`
  y `size_checksum`.

### Lo que se sabe

- El bloque de memoria que el juego pide al arrancar (`shell_memory_globals`)
  es de donde cada sistema toma la suya, uno detrás de otro, en direcciones
  múltiplo de 4. Con cada petición se actualiza una suma de comprobación de
  los tamaños. **No** es la memoria del estado de la partida, aunque
  funciona igual: tiene sus propios contadores.
- El sonido pide 0x20C bytes para sus globales, una tabla «sound sources»
  de 384 elementos de 0xBC bytes y 128 canales de 0x24 bytes.
- Los canales son de tres tipos, y hay 36, 36 y 1 de cada uno (73 de los
  128); el máximo de voces es 51.
- El sonido se prepara una sola vez aunque se pida varias: lleva la cuenta,
  y solo se desmonta cuando se han pedido tantos cierres como arranques.
- Si alguna de las cinco partes del sonido no se puede preparar, el sonido
  se queda sin marcar como preparado, pero la cuenta sube igual.
- Al desmontar solo se ponen a cero los punteros: la memoria no es suya.

### Sin comprobar

- `bit_vector_fill` con una cuenta de bits negativa de más de 31: el juego
  escribiría casi 4 GB.
- Que el byte `+0x7D` de los globales se ponga a cero: el bloque entero se
  acaba de poner a cero.
- El orden entre pedir la memoria de los canales y guardar el puntero de las
  fuentes; y si el primer vector de bits mide 128 bits o 160, porque los 32
  que siguen los borra el segundo.

### Mutaciones

100 líneas rotas a propósito: 94 detectadas a la primera, tres más
haciendo que las funciones suplidas dejen algo escrito en los globales, y
las demás son las de «Sin comprobar».

## 2026-10-02 — Cómo se reproduce un sonido

Las primeras funciones del sonido: las que dicen a qué distancia se oye un
sonido, con qué cono, qué volumen y qué tono. De 409 funciones se pasa a
420, el 3,73 % de las del juego (el 1,821 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `sound_class_unknown_00124f90` | `0x00124f90` | `sound/sound_manager.c` |
| `sound_get_minimum_distance` | `0x00124fc0` | `sound/sound_manager.c` |
| `sound_get_maximum_distance` | `0x00125010` | `sound/sound_manager.c` |
| `sound_unknown_00125060` | `0x00125060` | `sound/sound_manager.c` |
| `sound_get_inner_cone_angle` | `0x001250a0` | `sound/sound_manager.c` |
| `sound_get_outer_cone_angle` | `0x00125120` | `sound/sound_manager.c` |
| `sound_get_outer_cone_gain` | `0x001251a0` | `sound/sound_manager.c` |
| `sound_definition_random_gain` | `0x00125260` | `sound/sound_manager.c` |
| `sound_definition_random_pitch` | `0x001252f0` | `sound/sound_manager.c` |
| `sound_first_local_listener` | `0x00125360` | `sound/sound_manager.c` |
| `sound_class_get` | `0x00221810` | `sound/sound_classes.c` |

- Archivos nuevos: `sound/sound_definitions.h`, `sound/sound_manager.c`,
  `sound/sound_classes.c`, `tests/test_sound.py`.
- En la herramienta de pruebas, un tipo de resultado nuevo, `real:eax`:
  dos de estas funciones devuelven un real como sus 32 bits en `eax`.

### Cambiado

- `struct players_globals` y `MAXIMUM_LOCAL_PLAYERS` pasan de `main.c` a
  `main.h`, porque el sonido también mira los jugadores de la consola.

### Lo que se sabe

- Los nombres de los archivos y de las funciones son suposiciones: Halo 1
  no tiene estas funciones.
- Cada dato de cómo suena un sonido se busca en hasta tres sitios, por este
  orden: lo que diga quien lo reproduce (la «fuente», con un indicador por
  dato), la clase del sonido (solo las distancias, si el sonido tiene el
  indicador 10 u 11) y los parámetros de reproducción del sonido.
- Los parámetros de reproducción no están en la etiqueta del sonido: están
  todos juntos en una etiqueta común del mapa (`0x0051ebd4`), 0x38 bytes
  cada uno, y el sonido solo guarda un índice.
- Las clases de sonido están en otra etiqueta, a la que se llega por la
  etiqueta de globales (bloque en `+0xC0`); cada clase mide 0x5C bytes.
- Una fuente dice los ángulos del cono como una palabra de 16 bits: la
  parte de media vuelta que son (0xFFFF es π).
- El volumen de cada reproducción es la ganancia base más una parte al azar
  de la variación, con la **segunda** semilla del juego (la que no afecta a
  la partida); el tono, al azar entre dos límites enteros.
- El oyente es el del primer jugador de la consola, si está puesto.

### Sin comprobar

- `sound_class_get` cuando la etiqueta de globales no dice nada del sonido:
  el juego lee de la dirección 4 y se cae.
- En el ángulo de una fuente, los casos de 0 y de 0xFFFF aparte, y el
  término que se multiplica por cero: dan lo mismo que la fórmula general.
- En la ganancia al azar, restar y sumar 0,0: solo cambia el signo de un
  cero.

### Mutaciones

91 líneas rotas a propósito: 81 detectadas a la primera, cuatro más con
casos nuevos (índices de etiqueta de más de 255, índice de reproducción
negativo, ángulos donde dividir no da lo mismo que multiplicar por el
inverso) y las otras seis son las de «Sin comprobar».

## 2026-10-02 — Recorrer los archivos de un directorio

`find_files_next`, lo único que faltaba de los archivos: con ella
`files.c` y `files_xbox.c` quedan completos. De 408 funciones se pasa a
409, el 3,63 % de las del juego (el 1,789 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `find_files_next` | `0x00137000` | `tag_files/files_xbox.c` |

- `struct find_files_data` (el `WIN32_FIND_DATA` de la consola), que ahora
  es parte de `struct find_files_state` (0x388 bytes en total).
- Los indicadores de una búsqueda: recursiva (bit 0), decir los directorios
  (bit 1) y decir los archivos aunque se digan los directorios (bit 2).
- Funciones de la consola que se suplen en las pruebas: `FindFirstFileA`
  (`0x00331b9b`) y `FindNextFileA` (`0x00331ca8`).

### Lo que se sabe

- La búsqueda lleva una pila de hasta 16 directorios, cada uno con su
  búsqueda abierta en la consola. Al encontrar un directorio, si es
  recursiva, lo añade a la ruta y baja; cuando un directorio se acaba,
  cierra su búsqueda, quita el último elemento de la ruta y sube.
- `.` y `..` se saltan siempre.
- Sin indicadores solo se dicen los archivos. Con el bit 1 se dicen solo
  los directorios, y con el bit 2 las dos cosas.
- Con cada archivo se puede pedir la fecha en que se escribió por última
  vez.

### Sin comprobar

- Que la ruta completa se ponga a cero antes de empezar: se escribe entera
  antes de usarla.
- Al hacer la referencia de un archivo, el juego mira si tiene el indicador
  de nombre de archivo para quitarle el último elemento; pero la acaba de
  poner a cero, así que nunca lo tiene.

### Mutaciones

60 líneas rotas a propósito: 58 detectadas, y las otras dos son las de
«Sin comprobar».

## 2026-10-02 — El final de `main_game.c` y las partes de una ruta

Lo que quedaba suelto de `main_game.c`, que queda completo, y
`file_path_split` de los archivos. De 402 funciones se pasa a 408, el
3,62 % de las del juego (el 1,762 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_path_split` | `0x00137400` | `tag_files/files.c` |
| `main_game_next_map` | `0x0018e960` | `main/main_game.c` |
| `main_game_campaign_started` | `0x0018f0a0` | `main/main_game.c` |
| `main_game_goto_main_menu` | `0x0018f1c0` | `main/main_game.c` |
| `main_game_texture_allocation_allocate` | `0x0018f260` | `main/main_game.c` |
| `main_game_texture_allocation_deallocate` | `0x0018f350` | `main/main_game.c` |

- `time` (`0x00321aae`) y `rand` (`0x00321f75`), de la biblioteca de C,
  están en la tabla de símbolos para poder suplirlas en las pruebas.

### Cambiado

- `unknown_0018f1c0`, que el bucle principal y la carga de un estado
  llamaban sin saber qué era, es `main_game_goto_main_menu`; y
  `unknown_0018f0a0` es `main_game_campaign_started`.
- Los tamaños y posiciones de la tabla de `0x0054e8e0` (los perfiles de los
  jugadores, parece) pasan de `input_xbox.c` a `input.h`, porque
  `main_game.c` también la lee.

### Lo que se sabe

- Al acabar un mapa de la campaña, el juego pregunta cuál es el siguiente
  (`0x0019c440`) y pide una partida en él con las mismas opciones, una
  semilla nueva (`time ^ rand ^ GetTickCount`) y el nombre del mapa. Si no
  hay siguiente va al menú principal, con el motivo 7.
- En una sesión de red, antes de eso se escribe un informe de 0x33B bytes
  de la partida que acaba y se manda al servidor como
  `application/x-halo2campaign`.
- Ir al menú principal es pedir una partida de la clase 3 en
  `scenarios\ui\mainmenu\mainmenu`; si ya se está en ella solo se avisa a
  la interfaz.
- Al empezar una campaña de un jugador, si está puesto el indicador de
  `0x0054e7cc`, se carga la partida guardada del único perfil que hay (la
  guarda en `+0x1F8`).
- Hay un asignador de memoria (tabla de funciones en `0x00453c40`, objeto en
  `0x0047d8e4`) cuya memoria se la presta la caché de texturas, esperando
  poco: 30 intentos y una décima de segundo.
- `file_path_split` corta una ruta en sitio: pone ceros donde empiezan el
  nombre y la extensión. No mira el primer carácter de la ruta.

### Sin comprobar

- En `file_path_split`: si la ruta tiene que medir más de 0 o más de 1 para
  recorrerla, y la comparación de `*filename!=path` al final: dan lo mismo.
- En `main_game_next_map`: si el nombre del mapa se copia con 0x104 o con
  0x103 bytes, porque después se pone el cero final.
- En el asignador, el caso en que la memoria no llega nunca y se acaba el
  tiempo: depende del reloj del procesador.

### Mutaciones

96 líneas rotas a propósito: 91 detectadas a la primera, una más con un
caso nuevo (esperar a la consola tras 30 intentos) y las otras cuatro son
las de «Sin comprobar».

## 2026-10-02 — Los mandos

`input_xbox.c`, que ya existía en Halo 1: qué mandos hay enchufados, qué
botones se pulsan y cuánto tiempo, los joysticks y la vibración. De 392
funciones se pasa a 402, el 3,57 % de las del juego (el 1,730 % de sus
bytes). **Se pasa de las 400 funciones.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `input_initialize` | `0x001248b0` | `input/input_xbox.c` |
| `input_dispose` | `0x00124920` | `input/input_xbox.c` |
| `input_update` | `0x00124950` | `input/input_xbox.c` |
| `input_get_gamepad_state` | `0x001249a0` | `input/input_xbox.c` |
| `input_get_gamepad_type` | `0x001249d0` | `input/input_xbox.c` |
| `input_get_unit_drive_letter` | `0x001249f0` | `input/input_xbox.c` |
| `input_set_gamepad_rumbler_state` | `0x00124a40` | `input/input_xbox.c` |
| `input_update_devices` | `0x00124aa0` | `input/input_xbox.c` |
| `input_update_gamepads` | `0x00124c00` | `input/input_xbox.c` |
| `input_flush_rumble` | `0x00124ee0` | `input/input_xbox.c` |

- Archivos nuevos: `input/input.h`, `input/input_xbox.c`,
  `tests/test_input.py`.
- Funciones de la consola que se suplen en las pruebas: `XInitDevices`,
  `XGetDeviceChanges`, `XInputOpen`, `XInputClose`, `XInputGetCapabilities`,
  `XInputGetState`, `XInputSetState`.

### Cambiado

- Tres funciones que el arranque y el bucle principal llamaban sin saber
  qué eran (`unknown_001248b0`, `unknown_00124920`, `unknown_00124950`) son
  `input_initialize`, `input_dispose` e `input_update`.
- Los 0x120 bytes que se ponen a cero al empezar una partida
  (`unknown_004e61dc`) son el estado de los cuatro mandos.

### Lo que se sabe

- De cada mando se guarda, por botón, el valor (los ocho botones de la
  consola son analógicos, de 0 a 255), cuántos fotogramas y cuántos
  milisegundos lleva pulsado, y los dos joysticks.
- Un botón analógico no tiene un umbral fijo para contar como pulsado: el
  umbral le sigue. Mientras está pulsado sube hasta 0x20 por debajo del
  valor, y mientras no lo está baja hasta 0x40 por encima. Así un botón que
  tiembla cerca del umbral no se pulsa y se suelta en cada fotograma.
- Los joysticks tienen una zona muerta de 9.000 (de 32.768), y lo que queda
  se estira a todo el rango. Hacia el lado negativo el rango es de 23.768 y
  hacia el positivo de 23.767, porque el valor va de -32.768 a 32.767.
- Un fotograma de mandos nunca cuenta más de 100 ms.
- La vibración se manda solo si la consola ha acabado con la orden
  anterior, y se apaga si no hay partida, si está en pausa o si los mandos
  están «suprimidos». Tampoco vibra el mando de un jugador que no quiere
  (dos indicadores de la tabla de `0x0054e8e0`, que parece el perfil).
- El módulo también dice la letra de unidad del disco duro (`u`) y de las
  tarjetas de memoria.
- Cuando los mandos están suprimidos, el juego recibe un mando con nada
  pulsado.

### Sin comprobar

- Al subir el umbral de un botón, si se compara con `>` o con `>=`: da el
  mismo valor.

### Mutaciones

75 líneas rotas a propósito: 74 detectadas, y la otra es la de «Sin
comprobar».

## 2026-10-02 — Los estados guardados en archivos

Casi todo lo que faltaba de `game_state.c`: cargar el estado del juego desde
un archivo, sea un «core» del disco o una partida guardada del disco duro.
De 382 funciones se pasa a 392, el 3,48 % de las del juego (el 1,672 % de
sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_load_saved_game` | `0x001240b0` | `saved games/game_state.c` |
| `game_state_saved_game_read_header_async` | `0x00124360` | `saved games/game_state.c` |
| `game_state_load_core` | `0x00124380` | `saved games/game_state.c` |
| `game_state_core_file_reference` | `0x00124490` | `saved games/game_state.c` |
| `game_state_core_read` | `0x00124640` | `saved games/game_state.c` |
| `game_state_saved_game_create` | `0x00124770` | `saved games/game_state.c` |
| `game_state_saved_game_read_header` | `0x00124790` | `saved games/game_state.c` |
| `game_state_saved_game_read_data` | `0x00124800` | `saved games/game_state.c` |
| `game_state_saved_game_write` | `0x00124840` | `saved games/game_state.c` |
| `csprintf` | `0x000b66c0` | `cseries/cseries.c` |

- Archivo nuevo: `tests/test_game_state_files.py`.
- Funciones que se suplen en las pruebas: `saved_game_auxiliary_read`
  (`0x002174b0`) y `saved_game_auxiliary_write` (`0x00217520`).

### Lo que se sabe

- Un «core» es un volcado del estado del juego en un archivo del disco:
  `d:\core\<nombre>.bin`, o la ruta entera si el nombre empieza por una
  unidad. Es una herramienta de los desarrolladores que sigue en el juego.
- Cargar un estado tiene siempre los mismos pasos: leer la cabecera, ver
  que es válida, comprobar que es de la partida en curso (mismas opciones,
  mismo mapa), poner el bsp que dice, y solo entonces leer el estado
  entero. Con un «core», si la partida no coincide se empieza antes la suya;
  con una partida guardada, no se hace nada.
- Un estado cargado de un archivo pasa a ser de una partida local, fuera lo
  que fuera al guardarlo.
- La partida guardada tiene su estado en un archivo aparte, `auxilary.bin`
  (así escrito), que se lee y se escribe en otro hilo: el juego espera
  dando vueltas al bucle de carga.
- El estado ocupa 0x3FE000 bytes (unos 4 MB) y su cabecera 0x1288.

### Sin comprobar

- `csprintf` escribe como mucho 1.023 caracteres y pone el cero en el
  último sitio: si el tope fuera 1.024 daría lo mismo.
- `0x001241b0`, que guarda el estado en la partida guardada, sigue sin
  descompilar.

### Mutaciones

46 líneas rotas a propósito: 44 detectadas a la primera, una con un caso
nuevo y otra que es la de «Sin comprobar».

## 2026-10-02 — Los archivos

`files.c` y su parte de la consola, que en Halo 1 es `files_windows.c`:
abrir, leer, escribir y borrar archivos, y componer rutas. De 365 funciones
se pasa a 382, el 3,39 % de las del juego (el 1,633 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `file_delete` | `0x00136860` | `tag_files/files_xbox.c` |
| `file_exists` | `0x001368f0` | `tag_files/files_xbox.c` |
| `file_open` | `0x00136970` | `tag_files/files_xbox.c` |
| `file_close` | `0x00136bb0` | `tag_files/files_xbox.c` |
| `file_set_position` | `0x00136bf0` | `tag_files/files_xbox.c` |
| `file_set_eof` | `0x00136c40` | `tag_files/files_xbox.c` |
| `file_read` | `0x00136ca0` | `tag_files/files_xbox.c` |
| `file_write` | `0x00136d00` | `tag_files/files_xbox.c` |
| `file_read_from_position` | `0x00136d50` | `tag_files/files_xbox.c` |
| `file_write_to_position` | `0x00136d90` | `tag_files/files_xbox.c` |
| `file_get_last_modification_date` | `0x00136df0` | `tag_files/files_xbox.c` |
| `file_get_size` | `0x00136e90` | `tag_files/files_xbox.c` |
| `find_files_end` | `0x00136fc0` | `tag_files/files_xbox.c` |
| `file_location_get_full_path` | `0x001374c0` | `tag_files/files_xbox.c` |
| `find_files_start` | `0x00136f00` | `tag_files/files.c` |
| `file_path_add_name` | `0x00137320` | `tag_files/files.c` |
| `file_path_add_extension` | `0x00137370` | `tag_files/files.c` |

- Archivos nuevos: `tag_files/files.h`, `files.c`, `files_xbox.c`,
  `tests/test_files.py`.
- Funciones de la consola que se suplen en las pruebas: `GetLastError`,
  `RemoveDirectoryA`, `SetFileAttributesA`, `GetFileAttributesA`,
  `GetFileAttributesExA`.

### Lo que se sabe

- La referencia a un archivo es como la de Halo 1 (la firma `filo`, unos
  indicadores, la ruta de 256 caracteres) con dos campos más: el
  identificador del archivo abierto y la posición. Mide 272 bytes.
- Una ruta que no empieza por una unidad (`x:\`) es del disco del juego:
  se le pone `d:\` delante.
- El juego lleva la cuenta de la posición por su lado, y no le pide a la
  consola que se mueva si ya está donde quiere.
- Solo se abren archivos que existen (crear es cosa de otras funciones). Al
  abrir se dice por qué no se pudo: no existe, no hay permiso, no existe la
  ruta, la unidad no vale, está en uso, u otra cosa.
- Tras cada fallo el juego lee el error de la consola y lo borra, sin hacer
  nada con él: es lo que queda de los avisos de la versión de desarrollo.
  En `file_set_eof` lo hace dos veces.
- Una lectura que trae menos de lo pedido deja el error 0x26 (fin de
  archivo).

### Mutaciones

73 líneas rotas a propósito: 70 detectadas a la primera, y las otras 3 con
casos nuevos (rutas con minúsculas en los extremos del abecedario, una letra
sin dos puntos, una ruta que llena la referencia).

## 2026-10-02 — El cambio de partida

El resto del corazón de `main_game.c`: cómo se pide otra partida, cuándo se
hace el cambio y qué pasos tiene. Con esto está entero el camino que va de
«quiero jugar este mapa» a «la partida ha empezado», aunque muchas de las
piezas que llama sigan sin descompilar. De 359 funciones se pasa a 365, el
3,24 % de las del juego (el 1,568 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_game_restart` | `0x0018e700` | `main/main_game.c` |
| `main_game_change` | `0x0018e790` | `main/main_game.c` |
| `main_game_change_update` | `0x0018e810` | `main/main_game.c` |
| `main_game_change_immediate` | `0x0018e8b0` | `main/main_game.c` |
| `main_game_load_map` | `0x0018eaa0` | `main/main_game.c` |
| `main_game_load_failed` | `0x0018efa0` | `main/main_game.c` |

### Cambiado

- `main_game_load_globals` es más grande de lo que se sabía (0x1230 bytes):
  lleva también el cambio pedido (si hay uno, si es «a nada», cuándo se
  pidió y las opciones de la partida nueva).
- `unknown10C` y `unknown10D` ya tienen nombre: `restarting` y
  `change_pending`.

### Lo que se sabe

- Una partida nueva no empieza cuando se pide: `main_game_change` guarda las
  opciones y `main_game_change_update` la empieza al principio de un
  fotograma, si nada dice que espere.
- El cambio tiene estos pasos: soltar la partida y el mapa que haya, borrar
  lo que el bucle principal tuviera pendiente (guardar, revertir, cambiar de
  bsp), repartir la memoria, cargar el mapa y empezar la partida. Si se pide
  «nada», se suelta todo y queda abierta solo la caché de texturas.
- Cargar el mapa depende de si está en el disco duro: si está, se cargan sus
  tags y se abren las cachés; si no, se espera a que se copie (con la caché
  de texturas abierta, para enseñar algo mientras); si no se puede copiar,
  se pide soltarlo todo.
- Si la partida no puede empezar, el juego va al menú principal. Si lo que
  no puede empezar es el menú principal, se queda con la caché de texturas
  abierta y nada más.
- Reempezar la partida en el mismo mapa no vuelve a cargarlo: solo se
  desmontan y se montan otra vez los sistemas del juego.

### Sin comprobar

- Qué son `0x004e61dc` (0x120 bytes que se ponen a cero al empezar una
  partida), `0x0051ea10` (mientras es mayor que 0, el cambio espera) y los
  avisos a la partida en red (`0x00527330`, `0x000593e0`).

### Mutaciones

61 líneas rotas a propósito: 60 detectadas a la primera, y la otra con un
caso nuevo.

## 2026-10-02 — La memoria de la interfaz

El primer usuario de `memory_pool.c`: el fondo del que sale la memoria de
los menús («ui memory pool»), que es uno de los ocho que toman memoria al
cargar un mapa. De 355 funciones se pasa a 359, el 3,19 % de las del juego
(el 1,545 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `user_interface_memory_pool_new` | `0x001a474c` | `interface/user_interface_memory.c` |
| `user_interface_memory_pool_delete` | `0x001a479a` | `interface/user_interface_memory.c` |
| `user_interface_memory_allocate` | `0x001a47b1` | `interface/user_interface_memory.c` |
| `user_interface_memory_free` | `0x001a4826` | `interface/user_interface_memory.c` |

- Archivos nuevos: `interface/user_interface_memory.{c,h}`,
  `tests/test_user_interface_memory.py`.

### Cambiado

- `main_game_state_has_map` (`0x0018eea0`) se llamaba mal: lo que recibe no
  es la etapa de la carga sino el reparto de memoria, y dice si es uno de
  los dos de campaña en sesión de red. Ahora es
  `main_game_memory_configuration_is_session`.
- La tabla de los que toman memoria ya tiene tres de sus ocho pares con
  nombre: interfaz, animaciones y geometría.

### Lo que se sabe

- El fondo de la interfaz tiene 1 MB fuera de la campaña, 96 KB en la
  campaña y 384 KB en una campaña en sesión de red.
- Sus bloques no se mueven (no guarda los punteros de los dueños) y los
  nuevos rellenan huecos.
- Si no hay sitio para un bloque, la interfaz suelta lo que puede
  (`0x00147ebe`) y lo intenta una vez más.
- El otro fondo que se crea al cargar un mapa es el de la voz («voice
  pool»).

### Mutaciones

15 líneas rotas a propósito, las 15 detectadas.

## 2026-10-02 — El fondo de memoria

`memory_pool.c`, otro módulo que ya existía en Halo 1: un trozo de memoria
del que se van tomando bloques de cualquier tamaño, y que se puede volver a
compactar porque cada bloque sabe qué puntero de su dueño hay que cambiar
cuando se mueve. De 344 funciones se pasa a 355, el 3,15 % de las del juego
(el 1,538 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `memory_pool_new` | `0x0018e1f0` | `memory/memory_pool.c` |
| `memory_pool_delete` | `0x0018e230` | `memory/memory_pool.c` |
| `memory_pool_initialize` | `0x0018e250` | `memory/memory_pool.c` |
| `memory_pool_block_allocate` | `0x0018e2b0` | `memory/memory_pool.c` |
| `memory_pool_block_reallocate` | `0x0018e340` | `memory/memory_pool.c` |
| `memory_pool_block_free` | `0x0018e430` | `memory/memory_pool.c` |
| `memory_pool_compact` | `0x0018e470` | `memory/memory_pool.c` |
| `memory_pool_block_link` | `0x0018e4f0` | `memory/memory_pool.c` |
| `memory_pool_block_new_at_end` | `0x0018e560` | `memory/memory_pool.c` |
| `memory_pool_block_new_at_start` | `0x0018e600` | `memory/memory_pool.c` |
| `memory_pool_block_new_in_gap` | `0x0018e690` | `memory/memory_pool.c` |

- Archivos nuevos: `memory/memory_pool.{c,h}`, `tests/test_memory_pool.py`.

### Lo que se sabe

- Respecto a Halo 1, los bloques pierden las firmas de principio y fin
  (16 bytes en vez de 24) y el fondo gana tres opciones: no guardar los
  punteros de los dueños (y entonces no se puede compactar), rellenar los
  huecos (un bloque nuevo va primero entre dos bloques, luego antes del
  primero y solo al final después del último) y llevar la cuenta de quién
  pide cada bloque (16 bytes más por bloque: la firma `head`, dos datos de
  quien lo pide y la hora).
- Todo va a múltiplos de 16 bytes.
- A diferencia de Halo 1, pedir un bloque no compacta el fondo si no cabe:
  devuelve que no, y quien lo pide decide.
- Cambiar el tamaño de un bloque lo hace en el sitio si hay hueco detrás;
  si no, toma otro bloque, copia y suelta el viejo.
- Lo usan la interfaz («ui memory pool»), los objetos y algo más que toma
  memoria al cargar un mapa.

### Sin comprobar

- En `memory_pool_compact`, si un bloque que ya está en su sitio se mueve
  sobre sí mismo o no: queda igual.
- Qué son los dos datos que se apuntan de quien pide un bloque.

### Mutaciones

68 líneas rotas a propósito: 66 detectadas a la primera. De las otras 2,
una dio un caso nuevo (un fondo que no empieza en un múltiplo de 16) y la
otra es la de «Sin comprobar».

## 2026-10-02 — Cargar y descargar un mapa

`main_game.c`: el módulo que une las tres cachés con el resto del juego.
Lleva la cuenta de por dónde va la carga de un mapa, reparte la memoria
entre los que la usan y empieza la partida. De 332 funciones se pasa a 344,
el 3,05 % de las del juego (el 1,497 % de sus bytes). **Se pasa del 3 %.**

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_game_unload` | `0x0018eb80` | `main/main_game.c` |
| `main_game_load_begin` | `0x0018ec20` | `main/main_game.c` |
| `main_game_load_end` | `0x0018ec60` | `main/main_game.c` |
| `main_game_load_fail` | `0x0018ed00` | `main/main_game.c` |
| `main_game_close_caches` | `0x0018edb0` | `main/main_game.c` |
| `main_game_open_texture_cache` | `0x0018ee60` | `main/main_game.c` |
| `main_game_state_has_map` | `0x0018eea0` | `main/main_game.c` |
| `main_game_memory_configuration` | `0x0018eeb0` | `main/main_game.c` |
| `main_game_configure_memory` | `0x0018ef00` | `main/main_game.c` |
| `main_game_start` | `0x0018f000` | `main/main_game.c` |
| `main_game_unknown_0018f100` | `0x0018f100` | `main/main_game.c` |
| `main_game_launch_main_menu` | `0x0018f170` | `main/main_game.c` |

- Archivos nuevos: `main/main_game.{c,h}`, `tests/test_main_game.py`.
- La tabla de `0x00453c00` (`main_game_memory_users`): los ocho que toman
  memoria al cargar un mapa. Seis siguen sin descompilar.
- Arnés: una función sin descompilar a la que el código llama por un
  puntero «stdcall» se marca así en `symbols/functions.csv` y limpia la pila.

### Cambiado

- Tres globales de `main.c` que estaban sin identificar son campos de otras
  cosas, y ahora se leen de ellas: `0x004ed294` es la etapa de la carga
  (`main_game_load_globals.state`), `0x004e6470` y `0x004e64a0` son los
  contadores de «leerlo todo de una vez» de las cachés de texturas y de
  geometría, y `0x004ed39d` va con el estado de la carga.

### Lo que se sabe

- La carga de un mapa tiene etapas: nada, empezada, mapa cargado (con las
  cachés abiertas), cachés cerradas, y «solo la caché de texturas» para lo
  que se enseña cuando no hay mapa.
- El reparto de memoria tiene cinco maneras: 1 para todo lo que no es
  campaña, 2 y 3 para una campaña en una sesión de red (solo y con un
  segundo jugador), 4 y 5 para una campaña normal. Si la partida que empieza
  pide otro reparto que la anterior, los ocho de la tabla devuelven su
  memoria y la vuelven a tomar.
- Al terminar de cargar un mapa se abren, por este orden: la caché de
  animaciones, los textos del idioma de la consola, la de geometría y la de
  texturas (que antes lee los píxeles que vienen con el mapa).
- Los textos de cada idioma están en el tag de globales, 28 bytes por
  idioma a partir de `+0x188`.
- El menú principal es un mapa más: `scenarios\ui\mainmenu\mainmenu`, con
  el modo de juego 3.
- `main_game_unload` no tiene nada previsto para las etapas 1 y 3: el juego
  salta a la dirección 0. No se la llama entonces.

### Sin comprobar

- En `main_game_launch_main_menu`, el cero al final del nombre del mapa:
  `strncpy` ya rellena con ceros.
- Qué son `0x004cf770` (algo de la partida en red) y la tabla de datos de
  `0x00502104`.

### Mutaciones

79 líneas rotas a propósito: 76 detectadas a la primera. De las otras 3, dos
dieron casos nuevos y una es la de «Sin comprobar».

## 2026-10-02 — La caché de geometría

La tercera caché que usa `lruv_cache.c`: la que trae del disco los vértices
y los triángulos de los modelos y de los niveles. Halo 1 no la tiene; los
nombres siguen los de su caché de texturas. De 315 funciones se pasa a 332,
el 2,95 % de las del juego (el 1,454 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `geometry_cache_new` | `0x0012d8b0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_delete` | `0x0012d970` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_allocate_memory` | `0x0012d9f0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_free_memory` | `0x0012dad0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_open` | `0x0012daf0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_close` | `0x0012db40` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_reading_count` | `0x0012dba0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_update_predicted` | `0x0012dc10` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_predict` | `0x0012dcb0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_idle` | `0x0012dd80` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_delete` | `0x0012ddd0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_load` | `0x0012de70` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_flush` | `0x0012e0c0` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_delete_tag_blocks` | `0x0012e150` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_locked` | `0x0012e210` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_block_deleted` | `0x0012e290` | `cache/xbox_geometry_cache.c` |
| `geometry_cache_start_loading` | `0x0012e3a0` | `cache/xbox_geometry_cache.c` |

- Archivos nuevos: `cache/geometry_cache.h`, `cache/xbox_geometry_cache.c`,
  `tests/test_geometry_cache.py`.
- Funciones que se suplen en las pruebas: `geometry_block_attach_data`
  (`0x00223c70`), `geometry_block_busy` (`0x00223d80`),
  `geometry_block_wait` (`0x00223e00`), `unknown_0012e480`.

### Cambiado

- `struct geometry_block_info`: los seis bytes del final son del juego:
  `loaded`, `flags` y `cache_block_index`.
- En la caché de animaciones, el argumento de
  `animation_cache_allocate_memory` no es el número de jugadores sino el
  reparto de memoria (`configuration`), el mismo que recibe la de geometría.

### Lo que se sabe

- La caché tiene 7 MB, o 6,5 MB en dos de los seis repartos de memoria (el 2
  y el 4), en páginas de 4 KB, con hasta 512 bloques y 350 avisos. El juego
  no comprueba que el reparto sea uno de los seis.
- Cada bloque de geometría de un tag lleva al final si está leído, unos
  indicadores y su bloque en la caché. Al leerse, una función apunta en el
  tag dónde quedó la geometría; al echarlo, esos dos campos del tag se
  ponen a cero.
- Un bloque no se puede echar si se usó en este tick, si se está leyendo, o
  si se usó hace menos de 5 ticks y la consola aún dibuja con él.
- En esta versión el juego nunca espera por la geometría: hay un byte
  (`0x00468c4c`) que lo impide, y está puesto. Lo que no está se pide y se
  dibuja cuando llegue. La excepción es cuando el juego está leyendo todo
  de una vez (al cargar un nivel): entonces sí espera, a toda la geometría o
  solo a la de los niveles (los tags `sbsp` y `ltmp`).
- Al subir la prioridad de una lectura ya empezada, la tarea pasa siempre a
  prioridad 7, sea cual sea la nueva.
- Cada fotograma se empiezan lecturas de la geometría avisada hasta tener 5
  en marcha, si no hay más de 25 tareas pendientes.
- `geometry_cache_delete_tag_blocks` saca de la caché toda la geometría de
  un tag (lo usa quien descarga un tag).

### Sin comprobar

- Dos líneas que no cambian nada: poner `loaded` a falso tras echar el
  bloque (ya lo hace la función que lo echa) y la comprobación de que la
  geometría no estaba ya apuntada (no puede estarlo en ese punto).
- `0x0012e480`, a la que se llama cada fotograma tras el trabajo de la
  caché: se suple en las pruebas.

### Mutaciones

104 líneas rotas a propósito: 101 detectadas a la primera. De las otras 3,
una dio un caso nuevo (un bloque al que aún no se le ha dado su geometría)
y dos son las de «Sin comprobar».

## 2026-10-02 — La caché de texturas, segunda parte

Lo que faltaba de `xbox_texture_cache.c`: cómo se piden y se leen los
píxeles de un bitmap, y la espera de la memoria prestada. Con esto el módulo
está completo. De 301 funciones se pasa a 315, el 2,80 % de las del juego
(el 1,374 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `texture_cache_update_predicted` | `0x0012c450` | `cache/xbox_texture_cache.c` |
| `texture_cache_load_preloaded_pixels` | `0x0012c640` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_initialize_preloaded_format` | `0x0012c820` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_preloaded_format` | `0x0012c960` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_level` | `0x0012c990` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_choose_level` | `0x0012cb10` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_load` | `0x0012cb80` | `cache/xbox_texture_cache.c` |
| `texture_cache_usage_is_preloaded` | `0x0012ccb0` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_loaded` | `0x0012ccf0` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_get_hardware_format` | `0x0012ce00` | `cache/xbox_texture_cache.c` |
| `texture_cache_steal_memory` | `0x0012d400` | `cache/xbox_texture_cache.c` |
| `texture_cache_start_loading_bitmap` | `0x0012d5a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_initialize_hardware_format` | `0x0012d6d0` | `cache/xbox_texture_cache.c` |
| `system_ticks_to_seconds` | `0x00223bb0` | `cseries/cseries.c` |

- Las tablas de formatos de Direct3D por formato de bitmap (`0x00450768` y
  `0x004507c8`), reproducidas en el código.
- Funciones que se suplen en las pruebas: `bitmap_mipmap_count`
  (`0x000359e0`) y `bitmap_row_size` (`0x001365a0`).
- El apoyo de las pruebas (`tools/difftest_libc`) sabe dividir números de
  64 bits.

### Cambiado

- `struct bitmap_data`: nombres para `mipmap_count`, `mip_bias`, `usage`,
  `maximum_mip_bias` y la textura que el bitmap lleva dentro.
- En los globales de la caché: `has_preloaded_pixels`, `preloaded_pixels`,
  `blocking_count`, `blocking_usage`, `use_preloaded_pixels`.

### Lo que se sabe

- Al pedir la textura de un bitmap se dan cinco indicadores: esperar a los
  píxeles, leerlos si no están, «exacto» (el primer nivel y nada más), y dos
  de prisa. La prioridad de la lectura es 7 si el juego espera, 5 o 1 según
  la prisa, y 2 si no se dice nada.
- El nivel de detalle se elige con el sesgo: lo que pide quien dibuja, más
  lo que el bitmap cede de suyo (en centésimas de nivel), más el sesgo de la
  caché. Con sesgo 1 o más vale el segundo nivel; con 2 o más, el tercero.
  Un nivel que no sea el primero solo cuenta si ocupa más de 1 KB.
- Si el nivel elegido no está, vale cualquiera que esté. Si el juego tenía
  que esperar y no hay ninguno, se dibuja con una de tres texturas de
  reserva, según el tipo de bitmap, y se apunta el fallo para subir el sesgo.
- No se empiezan lecturas de poca prioridad si hay más de 25 tareas
  pendientes o ya van 10 lecturas en el fotograma. De los bitmaps previstos
  se atienden 5 por fotograma.
- Para los píxeles de un bitmap no se echan bloques usados en los últimos 5
  ticks; si el juego espera, en los últimos 2.
- El mapa puede traer un bloque de píxeles aparte (cabecera `+0x158` y
  `+0x15C`) que se lee entero al cargar; los bitmaps de ciertos usos (1 a
  11, 17 y 18) se dibujan con esos píxeles. Su textura se monta la primera
  vez que se pide, a partir de lo que el mapa dice de ella.
- La textura de Direct3D se monta a mano: formato, dimensiones como
  potencias de dos, número de mipmaps y dirección de los píxeles en 28 bits.
  Los bitmaps de tamaño que no es potencia de dos llevan el tamaño aparte.
- `texture_cache_steal_memory` deja pasar hasta 30 o 90 fotogramas de la
  caché para que los bloques envejezcan, y luego espera a la consola hasta
  0,1 o 1 segundo, midiendo con `rdtsc`. El procesador de la consola da
  733.333.333 ticks por segundo.
- El juego recorre los bitmaps de todos los grupos al cargar los píxeles
  del mapa y no hace nada con ellos: lo que hacía no está en esta versión.

### Sin comprobar

- La espera con reloj de `texture_cache_steal_memory` cuando no hay sitio
  ni dejando pasar fotogramas: depende del reloj del procesador, que en el
  emulador no da lo mismo dos veces.
- El valor exacto de los ticks por segundo: un tick más o menos no cambia
  el resultado con la precisión de un real.
- Tres escrituras que dejan lo que ya había (el bloque recién creado ya
  está a cero) y dos condiciones que no pueden darse de otra manera (los
  niveles van de más a menos detalle; con textura no se dibuja la de
  reserva).
- En `texture_cache_load_preloaded_pixels`, el recorrido de los bitmaps: en
  las pruebas no hay ninguno.

### Mutaciones

111 líneas rotas a propósito: 101 detectadas a la primera. De las otras 10,
cinco dieron casos nuevos (un bitmap previsto que ya está en la caché, la
profundidad con signo, filas de un tamaño múltiplo de 64) y cinco son las de
«Sin comprobar».

## 2026-10-02 — La caché de texturas, primera parte

`xbox_texture_cache.c`, que ya existía en Halo 1: la caché que guarda en
memoria los píxeles de los bitmaps. En este hito está todo menos la carga de
un bitmap: crearla, darle memoria, vaciarla, el trabajo de cada fotograma,
lo que le pregunta `lruv_cache` y la memoria que presta a otros. De 284
funciones se pasa a 301, el 2,67 % de las del juego (el 1,254 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `texture_cache_new` | `0x0012c0d0` | `cache/xbox_texture_cache.c` |
| `texture_cache_delete` | `0x0012c190` | `cache/xbox_texture_cache.c` |
| `texture_cache_open` | `0x0012c1e0` | `cache/xbox_texture_cache.c` |
| `texture_cache_close` | `0x0012c290` | `cache/xbox_texture_cache.c` |
| `texture_cache_update_mip_bias` | `0x0012c2e0` | `cache/xbox_texture_cache.c` |
| `texture_cache_stolen_memory_idle` | `0x0012c400` | `cache/xbox_texture_cache.c` |
| `texture_cache_reset_frame_index` | `0x0012c530` | `cache/xbox_texture_cache.c` |
| `texture_cache_tick` | `0x0012c5b0` | `cache/xbox_texture_cache.c` |
| `texture_cache_idle` | `0x0012c600` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_delete` | `0x0012c770` | `cache/xbox_texture_cache.c` |
| `texture_cache_bitmap_predict` | `0x0012cc10` | `cache/xbox_texture_cache.c` |
| `texture_cache_flush` | `0x0012d0a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_type` | `0x0012d160` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_locked` | `0x0012d1a0` | `cache/xbox_texture_cache.c` |
| `texture_cache_block_deleted` | `0x0012d200` | `cache/xbox_texture_cache.c` |
| `texture_cache_steal_memory_now` | `0x0012d2f0` | `cache/xbox_texture_cache.c` |
| `texture_cache_return_memory` | `0x0012d520` | `cache/xbox_texture_cache.c` |

- Archivos nuevos: `cache/texture_cache.h`, `cache/xbox_texture_cache.c`,
  `tests/test_texture_cache.py`.
- Funciones de Direct3D que se suplen en las pruebas: `D3DDevice_KickOff`
  (`0x003fac30`), `D3DResource_IsBusy` (`0x003fb710`),
  `D3DResource_BlockUntilNotBusy` (`0x003fad50`).

### Cambiado

- `physical_memory_buffer_allocate` y `physical_memory_buffer_free` eran de
  la caché de texturas: ahora son `texture_cache_steal_memory`
  (`0x0012d400`, aún sin descompilar) y `texture_cache_return_memory`, como
  en Halo 1. La copia de mapas y la partida guardada las siguen usando igual.
- `struct bitmap_data`: tienen nombre los campos que la caché usa del final
  del bitmap (`hardware_format`, `base_address`, `frame_index`) y los que
  ocupan el sitio del cuarto al sexto nivel de detalle (`cache_block_indices`
  y `cache_addresses`).
- `struct d3d_device`: el puntero a los registros de la consola (`+0x934`).

### Lo que se sabe

- La caché se queda con toda la memoria física que sobra al cargar un mapa,
  en páginas de 4 KB, y tiene hasta 512 bloques. Un bitmap ocupa hasta tres:
  uno por nivel de detalle.
- El juego quita detalle a los bitmaps según lo llena que esté la caché. Cada 200 ms
  mira cuántas páginas se usaron en los últimos 5 ticks: con el 95 % o más
  (o si se dibujó un bitmap sin sus píxeles) sube el sesgo 0,05; con el 85 %,
  0,02; con el 70 %, 60 % y 50 % o menos lo baja 0,01, 0,02 y 0,05. El sesgo
  va de 0 a 2 niveles.
- Un bloque no se puede echar si es memoria prestada, si está marcado como
  necesario (salvo al vaciar la caché), si se está leyendo o si la consola
  aún dibuja con él. Al echar uno de un bitmap, el juego espera a la lectura
  y a la consola.
- La memoria que la caché presta (a la copia de mapas y a la partida
  guardada) empieza en una página, y justo antes lleva 36 bytes: el bloque,
  una firma (la dirección con `0x02281972`), el tamaño, un dato de quien la
  pide y dos funciones suyas, una para cada fotograma y otra para cuando la
  caché quiere la memoria de vuelta. Al vaciar la caché se les pide a todos.
- Tras mandar trabajo a la consola (`D3DDevice_KickOff`) el juego lee tres
  registros de la tarjeta gráfica y no hace nada con ellos; está tal cual.
- Los bitmaps tienen sitio para seis niveles de detalle en el mapa, pero el
  juego de la consola solo lee tres y guarda cosas suyas en el sitio de los
  otros tres.
- `texture_cache_delete` no libera la lista de bitmaps previstos.

### Sin comprobar

- En lo que impide echar un bloque, la comprobación de que es memoria
  prestada: un bloque prestado tiene además el resto a cero, y con eso ya
  cuenta como «leyéndose».
- Al prestar memoria, el borrado del bloque: `datum_new_at_index` ya lo deja
  a cero.
- `texture_cache_steal_memory` (`0x0012d400`) y la función que empieza a
  leer los bitmaps previstos (`0x0012c450`) siguen sin descompilar; en las
  pruebas de `texture_cache_idle` la segunda se suple.

### Mutaciones

100 líneas rotas a propósito: 95 detectadas a la primera. De las otras 5,
tres dieron casos nuevos (un bitmap en la lista de previstos sin su marca,
uno que está dos veces, memoria prestada con basura) y dos son las de «Sin
comprobar».

## 2026-10-02 — La caché de animaciones

El primer módulo que usa `lruv_cache.c` entero: la caché que trae del disco
los datos de las animaciones cuando hacen falta. Halo 1 no la tiene (allí las
animaciones van dentro de los tags); los nombres siguen los de su caché de
sonidos. De 266 funciones se pasa a 284, el 2,52 % de las del juego (el
1,187 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `animation_cache_new` | `0x00123180` | `cache/xbox_animation_cache.c` |
| `animation_cache_allocate_memory` | `0x00123230` | `cache/xbox_animation_cache.c` |
| `animation_cache_free_memory` | `0x001232e0` | `cache/xbox_animation_cache.c` |
| `animation_cache_delete` | `0x00123310` | `cache/xbox_animation_cache.c` |
| `animation_cache_open` | `0x001233a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_close` | `0x001233f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_idle` | `0x00123430` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_new` | `0x001234a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_request` | `0x00123550` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_read` | `0x001235b0` | `cache/xbox_animation_cache.c` |
| `animation_cache_find_predicted` | `0x00123680` | `cache/xbox_animation_cache.c` |
| `animation_cache_predict` | `0x001236f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_wait` | `0x001237a0` | `cache/xbox_animation_cache.c` |
| `animation_cache_get_data` | `0x001237e0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_locked` | `0x001238b0` | `cache/xbox_animation_cache.c` |
| `animation_cache_block_deleted` | `0x001238f0` | `cache/xbox_animation_cache.c` |
| `animation_cache_reading_count` | `0x00123970` | `cache/xbox_animation_cache.c` |
| `animation_cache_update_predicted` | `0x001239d0` | `cache/xbox_animation_cache.c` |

- Archivos nuevos: `cache/animation_cache.h`, `cache/xbox_animation_cache.c`,
  `tests/test_animation_cache.py`.
- En `memory/lruv_cache.h`, como código en línea (en Halo 1 son funciones;
  aquí el juego las repite en cada sitio): `lruv_idle`, `lruv_block_get`,
  `lruv_block_get_address`, `lruv_block_touch`.
- Global `heap_allocation` (`0x00468758`): el asignador con el que se crean
  las cachés.
- Arnés: un `Buffer` puede llevar punteros a funciones (`functions`) y
  sitios donde la llamada deja punteros a funciones o a globales (`places`),
  que se comparan por aquello a lo que apuntan.

### Cambiado

- `cache_file_read_async` devuelve el número de la tarea de lectura (antes
  estaba como `void`): la caché lo guarda para poder esperar la lectura.
- `shell_initialize` y `shell_dispose` llaman a `animation_cache_new` y
  `animation_cache_delete` (antes `unknown_00123180` y `unknown_00123310`).
- `CACHE_FILE_SECTOR_SIZE` pasa a `cache_files.h`.

### Lo que se sabe

- La caché tiene 4 MB (3 MB con la pantalla partida), en páginas de 2 KB, y
  hasta 512 bloques. La memoria se toma al cargar un mapa y se suelta al
  salir: las dos funciones están en una tabla de ocho pares en `0x00453c00`,
  la de todos los que toman memoria entonces.
- En el tag, los datos de unas animaciones son 16 bytes: un campo sin
  identificar, el tamaño, el desplazamiento en el mapa y el bloque de la
  caché (o ninguno).
- Hay tres urgencias, en una tabla en `0x00468810` (prioridad de la tarea,
  edad con la que nace el bloque, antigüedad mínima de los bloques que se
  pueden echar): ahora mismo (8, 0, 0), pronto (5, 10, 30) y cuando se pueda
  (2, 20, 50).
- Quien va a usar una animación avisa antes (`animation_cache_predict`); la
  caché apunta hasta 128 avisos y cada fotograma empieza a leer algunos, sin
  pasar de 8 lecturas a la vez ni si hay más de 25 tareas pendientes. Primero
  van los avisos urgentes.
- Si se pide una animación que no está (`animation_cache_get_data`), el
  juego se queda esperando la lectura. Para hacerle sitio puede echar hasta
  los bloques que se están leyendo, esperándolos, y lo intenta hasta tres
  veces más, pasando un tick de la caché cada vez.
- `animation_cache_get_data` no mira si consiguió bloque: una animación que
  no quepa en la caché entera no se puede pedir.
- Un bloque que se está leyendo no se echa. Al echar uno, la animación
  vuelve a quedar sin bloque.

### Corregido durante el hito

- `animation_cache_open` vaciaba dos veces cada tabla: `data_make_valid` ya
  lo hace. Lo encontró la comprobación por mutación.

### Sin comprobar

- Tres escrituras que dejan el valor que ya había: los dos `read_done=
  FALSE` repetidos al pedir y al leer un bloque, y si el ajuste del tick en
  `animation_cache_predict` es con `>` o con `>=`.
- Para qué sirve el segundo argumento de `animation_cache_get_data`
  (`0x7000180` cuando lo llama la propia caché): la función que lo recibe no
  lo usa.

### Mutaciones

89 líneas rotas a propósito: 82 detectadas a la primera. De las otras 7,
dos eran el error de `animation_cache_open`, dos dieron casos nuevos (un
aviso de una animación que ya está en la caché, un bloque sin lectura
empezada) y tres son las de «Sin comprobar».

## 2026-10-01 — Reservar un bloque en la caché

La función que faltaba de `lruv_cache.c`, y la más grande del módulo (1.210
bytes): la que busca sitio para un bloque nuevo y decide a cuáles echar. De
265 funciones se pasa a 266, el 2,36 % de las del juego (el 1,123 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `lruv_block_new` | `0x0013d370` | `memory/lruv_cache.c` |

### Cambiado

- Campos de la caché que ya tienen nombre: `hole_algorithm` (antes
  `unknown2C`) y `failed_page_counts` (antes `unknown44`); y los del hueco,
  `previous_block_index` y `first_page_index`.
- `tests/test_lruv_cache.py`: el constructor de cachés acepta el tamaño de la
  tabla, las edades de los bloques, los huecos entre ellos y ticks de 32 bits.

### Lo que se sabe

- La función recorre las páginas de la primera a la última, bloque a bloque
  y hueco a hueco. En cada sitio donde empieza un bloque o un hueco puede
  empezar un tramo; cada tramo crece hasta tener las páginas pedidas y
  entonces se compara con el mejor hasta el momento. Un bloque que no se
  puede echar (lo dice la función del dueño, o se usó hace menos ticks de
  los que se piden) corta todos los tramos que pasan por él.
- Lleva hasta 255 tramos abiertos a la vez, en un anillo de 256 en la pila.
- Hay tres maneras de comparar dos tramos, según un campo de la caché: el
  más viejo (y a igualdad, el de menos páginas); el más viejo con margen
  (`lruv_hole_is_better`); y el de más edad por página.
- Si no hay sitio, se apunta el número de páginas pedido en una tabla por
  edad, y la próxima petición igual o mayor con esa edad falla sin buscar.
  La tabla se borra cuando la caché cambia o pasa un tick.
- Si la tabla de bloques está llena, además de los bloques del tramo se echa
  al bloque que lleva más tiempo sin usarse.
- La tabla de fallos tiene ocho entradas y la edad se limita a ocho, no a
  siete: con edad 8 o más el juego lee y escribe el campo siguiente de la
  caché, que es el puntero a los bloques. Se ha dejado igual.

### Sin comprobar

- Al enlazar el bloque nuevo detrás del último, si el anterior se toma de
  `last_block_index` o del bloque previo: en una lista bien formada son el
  mismo.

### Mutaciones

47 líneas rotas a propósito: 40 detectadas a la primera. Las otras 7 dieron
casos nuevos de prueba (ticks con el bit alto, más de 256 tramos abiertos,
bloques bloqueados con la tabla llena) y ahora se detectan todas salvo la de
«Sin comprobar».

## 2026-10-01 — La caché de bloques de páginas

`lruv_cache.c`, un módulo que ya existía en Halo 1: una caché que reparte
páginas de memoria en bloques y, cuando no cabe uno nuevo, echa a los que
llevan más tiempo sin usarse. El juego la usa para las animaciones y las
texturas. De 258 funciones se pasa a 265, el 2,35 % de las del juego (el
1,082 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `lruv_initialize` | `0x0013d170` | `memory/lruv_cache.c` |
| `lruv_wrap_ticks` | `0x0013d230` | `memory/lruv_cache.c` |
| `lruv_flush` | `0x0013d2b0` | `memory/lruv_cache.c` |
| `lruv_hole_is_better` | `0x0013d320` | `memory/lruv_cache.c` |
| `lruv_block_delete` | `0x0013d830` | `memory/lruv_cache.c` |
| `lruv_resize` | `0x0013d8b0` | `memory/lruv_cache.c` |
| `lruv_compute_pages_in_use` | `0x0013d950` | `memory/lruv_cache.c` |

- Archivos nuevos: `memory/lruv_cache.{c,h}`; `tests/test_lruv_cache.py`.

### Lo que se sabe

- La caché de Halo 2 tiene los campos de la de Halo 1 y varios más: una
  tercera función, un modo de elegir sitio, ocho contadores y un asignador.
  Los bloques pierden el campo de datos del usuario (24 bytes en vez de 28).
- El contador de ticks llega hasta 0x7fffffff; al llegar vuelve a 0x101 y
  los bloques conservan su orden solo hasta 256 ticks atrás.
- Al elegir entre dos huecos gana el que lleva más tiempo sin usarse; si
  la diferencia es de menos de 150 ticks (y nunca se cuentan más de 300),
  gana el de menos páginas.
- El módulo de `0x00123180` es la caché de animaciones («xbox animation
  cache»), y `0x0012c0d0` la de texturas.

### Sin comprobar

- En `lruv_wrap_ticks`, si un bloque de exactamente 256 ticks de antigüedad
  entra por una rama o por la otra: las dos le dan el mismo valor.

### Pendiente

- `lruv_block_new` (`0x0013d370`), la función que reserva un bloque: es
  grande y tiene tres maneras de puntuar los huecos.

## 2026-10-01 — Las preferencias guardadas

El archivo `z:\preferences.dat`, donde el juego recuerda cosas de una vez
para otra: abrirlo, comprobar que lo firmó esta consola, escribirlo cuando
algo cambia. Es el primero de los sistemas que prepara el arranque que se
identifica. De 244 funciones se pasa a 258, el 2,29 % de las del juego (el
1,052 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `global_preferences_initialize` | `0x00121350` | `main/global_preferences.c` |
| `global_preferences_reset` | `0x00121220` | `main/global_preferences.c` |
| `global_preferences_validate` | `0x00121280` | `main/global_preferences.c` |
| `global_preferences_flush` | `0x00121120` | `main/global_preferences.c` |
| `global_preferences_flush_blocking` | `0x00120d80` | `main/global_preferences.c` |
| `global_preferences_set_name` | `0x00120df0` | `main/global_preferences.c` |
| `global_preferences_set_other_name` | `0x00120e40` | `main/global_preferences.c` |
| `global_preferences_set_unknown15C` | `0x00121040` | `main/global_preferences.c` |
| `global_preferences_set_unknown160` | `0x00121060` | `main/global_preferences.c` |
| `global_preferences_get_unknown160` | `0x001210a0` | `main/global_preferences.c` |
| `global_preferences_set_unknown164` | `0x001210c0` | `main/global_preferences.c` |
| `global_preferences_get_unknown164` | `0x00121100` | `main/global_preferences.c` |
| `xbox_signature_end` | `0x00213380` | `main/global_preferences.c` |
| `async_flush_file_blocking` | `0x001a0af0` | `cseries/async.c` |

- Archivos nuevos: `main/global_preferences.{c,h}`;
  `tests/test_global_preferences.py`.

### Cambiado

- Las tres variables sueltas que usaban los archivos de caché (el idioma y
  el build de las copias de mapas, y la marca de «algo cambió») son campos
  de `global_preferences_globals` (`0x00510818`).
- `shell_initialize` y el fotograma llaman ya a estas funciones por su
  nombre.
- La biblioteca C de las pruebas gana `wcsncpy` con caracteres de 16 bits.

### Lo que se sabe

- El archivo son 0x1fc bytes: una firma de 20 bytes hecha por la consola y
  0x1e8 de datos. Si la firma no coincide, la versión no es la 8 o no se
  lee entero, se descarta y se empieza con preferencias vacías.
- Guarda, entre otras cosas, cinco nombres de 31 caracteres, el idioma y el
  build con los que se hicieron las copias de los mapas (por eso al cambiar
  de idioma se vuelven a copiar), y varios ajustes de 0 a 3.
- Se escribe en segundo plano, y solo si lo que hay difiere de lo último
  escrito.
- Un descuido del juego, reproducido tal cual: al validar unas
  preferencias recién leídas, el final de cuatro de los nombres se pone en
  las preferencias que el juego ya tenía, no en las leídas.

### Sin comprobar

- En `global_preferences_initialize`, marcar la tarea como terminada
  después de validar: la lectura ya la había marcado.
- Qué son los ajustes sin nombre (`unknown15C`, `unknown160`...).

## 2026-10-01 — Las opciones de una partida

Las dos funciones que deciden si unas opciones de partida se pueden jugar y
si dos opciones son de la misma partida. Eran lo que faltaba para volver a
un punto de control sin funciones simuladas. De 242 funciones se pasa a 244,
el 2,17 % de las del juego (ya el 1,000 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_options_verify` | `0x00138180` | `game/game.c` |
| `game_options_match` | `0x001384a0` | `game/game.c` |

- `tests/test_game_options.py`: opciones buenas que se estropean al azar de
  una o varias maneras (2.500 casos por función).

### Cambiado

- `struct game_options` tiene ya casi todos sus campos: el mapa, la
  dificultad, si es cooperativo, las reglas del multijugador, la tabla de
  16 consolas, esta consola y los 16 jugadores.
- `game_is_campaign_unknown12C` pasa a llamarse `game_is_coop`: el campo
  que no se entendía es «campaña de dos jugadores».

### Lo que se sabe

- Unas opciones valen si: el modo, la simulación, el ritmo de ticks (1 a
  300) y la dificultad están en rango; hay nombre de mapa; no hay dos
  consolas iguales ni dos jugadores iguales; cada jugador tiene mando de 0
  a 3 e identificador distinto de cero, y está en una consola de la partida
  (o en ninguna, con la marca correspondiente); y el número de jugadores
  cuadra: 1 en campaña, 2 en cooperativo, de 1 a 16 en multijugador.
- En los modos 4 y 5 no se mira cuántos jugadores hay ni si esta consola
  está en la partida.
- Dos opciones son de la misma partida si coinciden el mapa, el modo, el
  ritmo de ticks y los mandos de esta consola, y además la dificultad y el
  cooperativo (campaña) o las reglas enteras (multijugador).

### Sin comprobar

- Rechazar un modo de juego negativo al principio de la validación: quitar
  esa comprobación no cambia nada, porque más abajo se rechaza igualmente.

## 2026-10-01 — Once funciones pequeñas del juego

Funciones cortas de `game.c` que el resto del juego consulta a todas horas:
si hay partida, cómo se simula, si se perdió o terminó, las opciones por
defecto y los ticks de un fotograma. De 231 funciones se pasa a 242, el
2,15 % de las del juego (el 0,963 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_options_new_default` | `0x00138110` | `game/game.c` |
| `game_run_ticks` | `0x00137f70` | `game/game.c` |
| `game_in_progress` | `0x00138800` | `game/game.c` |
| `game_is_campaign_unknown12C` | `0x00138820` | `game/game.c` |
| `game_is_distributed` | `0x00138840` | `game/game.c` |
| `game_is_authoritative` | `0x00138860` | `game/game.c` |
| `game_is_predicted` | `0x00138880` | `game/game.c` |
| `game_unknown_001388a0` | `0x001388a0` | `game/game.c` |
| `game_lost` | `0x00138960` | `game/game.c` |
| `game_finish` | `0x001389c0` | `game/game.c` |
| `game_finished_wait_over` | `0x00138a10` | `game/game.c` |

### Corregido

- El campo de las opciones que llamé `reverted` no es una marca: es el tipo
  de simulación de la partida (`simulation`). Volver a un punto de control
  lo pone a 1, que es «en una sola consola»; por eso parecía una marca.

### Lo que se sabe

- Las opciones por defecto de una partida: campaña, una consola, dificultad
  normal, semilla 0x78a8 y un ritmo de ticks igual a la mitad de la
  frecuencia de la imagen (30 por segundo a 60 Hz, 25 a 50 Hz).
- Cinco segundos después de perder, el juego vuelve al punto de control;
  siete segundos después de terminar, acaba.
- Un fotograma ejecuta varios ticks seguidos. Si entre dos ticks el bucle
  tiene algo que hacer (guardar, cambiar de bsp...), los que faltan no se
  ejecutan y su tiempo se descuenta.
- Tipo de simulación: de 2 a 5 la partida se reparte entre consolas; 4 es
  la consola que predice (cliente). Los nombres `distributed`,
  `authoritative` y `predicted` son suposiciones.

### Sin comprobar

- Dos cambios que no alteran ningún resultado: dividir la frecuencia entre
  dos con división o con desplazamiento (las negativas se sustituyen antes
  por 60), y leer la simulación con signo o sin él.

## 2026-10-01 — El cambio de bsp y las acciones del bucle

Cómo el juego cambia la parte del nivel que tiene cargada (el bsp), y tres
de las acciones que el bucle principal atiende entre fotogramas. De 226
funciones se pasa a 231, el 2,05 % de las del juego (el 0,944 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `scenario_switch_structure_bsp` | `0x0011c1b0` | `scenario/scenario.c` |
| `cache_file_structure_bsp_unload` | `0x00122bc0` | `cache/cache_files.c` |
| `main_save_map_private` | `0x0012bb20` | `main/main.c` |
| `main_switch_structure_bsp_private` | `0x0012bd40` | `main/main.c` |
| `main_unknown4E_private` | `0x0012bdf0` | `main/main.c` |

- `tests/test_scenario_switch.py`.
- `ESTADO.md` (lista de lo hecho y lo que falta) y
  `tools/mutation_check.py`, del paso anterior, se mantienen al día desde
  aquí.

### Lo que se sabe

- Cambiar de bsp, por orden: avisar a cada parte del juego; desmontar lo
  que las partes tienen del bsp viejo; olvidar el bsp viejo; leer el nuevo
  del disco; apuntar las variables globales (bsp, mapa de luces, bsp de
  colisión); y preparar las partes para el nuevo. Si la lectura falla, el
  juego se queda sin bsp y las partes no se preparan.
- El tiempo que tarda la carga no cuenta en el fotograma siguiente.
- Las tres acciones muestran un mensaje al jugador en la campaña. Sus
  textos son `hud_autosave_end` (punto de control hecho),
  `hud_loading_end` (carga terminada) y `hud_saving_end` (guardado
  terminado); los nombres salen de la tabla de cadenas de los mapas.
- Al guardar, el mensaje va a cada jugador de la consola; en las otras
  dos, solo al primero.

### Sin comprobar

- En `scenario_switch_structure_bsp`, poner a cero el mapa de luces cuando
  el bsp no tiene: el juego lo hace, pero ya estaba a cero.
- `main_save_map_private` y `main_switch_structure_bsp_private` con el
  juego sin preparar (`main_game_globals` nulo): el juego original lee de
  la dirección cero, así que no se puede probar.

### Pendiente

- La acción `0x0012bbd0`, que abre pantallas de la interfaz.

## 2026-10-01 — Un fotograma del juego y su reloj

Lo que el bucle principal hace en cada fotograma, y la función que calcula
cuántos segundos cuenta ese fotograma. De 224 funciones se pasa a 226, el
2,01 % de las del juego (el 0,910 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_loop_body` | `0x0012b450` | `main/main.c` |
| `main_time_update` | `0x0012b0e0` | `main/main.c` |

- `tests/test_main.py`.

### Cambiado

- Las variables sueltas del tiempo del bucle son ahora una estructura,
  `main_time_globals` (`0x004e6388`). Los ocho bytes que `game_state_save`
  copiaba sin saber qué eran son el momento del último fotograma: después
  de guardar se igualan al contador actual para que el tiempo que tardó el
  guardado no cuente en el fotograma siguiente.
- `0x00509340` es `application_paused`, como en Halo 1.

### Lo que se sabe

- Un fotograma hace, por orden: lo que el juego le pidió al bucle (volver
  al punto de control, reiniciar, cambiar de bsp, guardar, cargar un
  núcleo); varias partes que corren haya o no partida; guardar las
  preferencias si cambiaron; un paso de la copia de mapas al disco duro; y,
  si el juego no está en pausa, el reloj y las partes de la partida.
- El tiempo se cuenta en refrescos verticales de la imagen, no en
  milisegundos: los segundos de un fotograma son los refrescos pasados
  divididos por la frecuencia (60 si no se conoce), entre 0 y 10.
- Si al juego se le fija un número de refrescos por fotograma, cada
  fotograma dura como mínimo dos.
- El contador de milisegundos de la consola se guarda en 64 bits, sumando
  uno a la parte alta cada vez que da la vuelta (cada 49 días).
- El campo de las tareas que llamé `tasks_added` se iguala en cada
  fotograma al número de tareas pendientes: es una cuenta aproximada de
  las que quedan, no de las creadas.

### Sin comprobar

- Qué son las 33 funciones que el fotograma llama y que no están
  descompiladas: solo se comprueba que se llaman en el mismo orden y con
  los mismos argumentos que en el juego.
- En `main_time_update`, la comparación del último fotograma con el momento
  previsto: cambiar `>` por `>=` da el mismo resultado.

### Pendiente

- Lo que el juego le pide al bucle y aún no está: guardar
  (`0x0012bb20`), cambiar de bsp (`0x0012bd40`), `0x0012bbd0` y
  `0x0012bdf0`.

## 2026-10-01 — El bucle principal y la tabla de sistemas

El bucle principal del juego y, con él, la tabla de las 68 partes del juego
(objetos, sonido, IA...) que se recorre para prepararlas y desmontarlas. De
210 funciones se pasa a 224, el 1,99 % de las del juego (el 0,876 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `main_loop` | `0x0012b690` | `main/main.c` |
| `main_time_initialize` | `0x0012b070` | `main/main.c` |
| `main_revert_map_private` | `0x0012ba30` | `main/main.c` |
| `main_reset_map_private` | `0x0012ba90` | `main/main.c` |
| `main_revert_map_if_possible_private` | `0x0012bad0` | `main/main.c` |
| `main_action_pending` | `0x0012be90` | `main/main.c` |
| `main_cancel_map_copy_and_wait` | `0x0012bf40` | `main/main.c` |
| `game_initialize` | `0x00137c20` | `game/game.c` |
| `game_initialize_for_new_map` | `0x00137ca0` | `game/game.c` |
| `game_options_new` | `0x00137dd0` | `game/game.c` |
| `game_dispose_from_old_map` | `0x00137d00` | `game/game.c` |
| `game_initialize_for_new_structure_bsp` | `0x00137d40` | `game/game.c` |
| `game_dispose_from_old_structure_bsp` | `0x00137da0` | `game/game.c` |
| `no_operation` | `0x00175f40` | `cseries/cseries.c` |

- La tabla `game_systems` (`0x00440dd8`): 68 partes con hasta 9 funciones
  cada una. La prueba comprueba que cada casilla tiene la misma función que
  la del juego.
- `game_dispose`, que el juego no tiene como función aparte: su código está
  dentro de `main_loop`.
- Archivos nuevos: `main/main.{c,h}`; `tests/test_game.py`.

### Cambiado

- `main_reset_map_requested` pasa a ser un campo de `main_globals`
  (`0x00547f28`), que reúne lo que el juego le pide al bucle.
- `tools/difftest.py`: los punteros a funciones del juego que `source/` aún
  no tiene también se comparan por nombre.

### Lo que se sabe

- El bucle (`main_loop`): empieza a contar el tiempo, prepara el juego,
  repite fotogramas hasta que se pide salir, termina la partida y desmonta
  el juego.
- Cada parte del juego puede tener hasta nueve funciones: preparar y
  desmontar; mapa nuevo y mapa viejo; bsp nuevo y bsp viejo; y tres más
  para cuando cambia el bsp o lo que está activo. Se preparan en el orden
  de la tabla y se desmontan en el contrario. En Halo 1 cada función
  llamaba a las partes por su nombre; aquí es una tabla.
- `game_state` es la parte 16 de la tabla. De las 166 funciones distintas
  que hay en ella, solo cinco están descompiladas; el resto están apuntadas
  por su dirección (`unknown_...`).
- Lo que el juego le pide al bucle (reiniciar el mapa, volver al punto de
  control, guardar, cambiar de bsp...) son marcas en `main_globals` que se
  atienden al principio de cada fotograma.

### Sin comprobar

- Qué hace cada una de las tres últimas funciones de una parte: sus nombres
  son suposiciones, por cómo las llama `0x0011c1b0`.

### Pendiente

- El cuerpo de un fotograma (`0x0012b450`), que llama a unas 35 funciones.
- `scenario_switch_structure_bsp` (`0x0011c1b0`), que carga un bsp y usa
  las funciones de bsp de la tabla.

## 2026-10-01 — El arranque del juego

Las primeras funciones del ejecutable: por dónde empieza el juego, qué
prepara antes de su bucle principal y qué desmonta al salir. Con esto hay
un hilo desde el punto de entrada hasta casi todo lo descompilado. De 203
funciones se pasa a 210, el 1,86 % de las del juego (el 0,840 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `shell_initialize` | `0x00012000` | `shell/shell.c` |
| `shell_dispose` | `0x00012080` | `shell/shell.c` |
| `shell_main` | `0x00012190` | `shell/shell.c` |
| `shell_platform_initialize` | `0x00012200` | `shell/shell.c` |
| `shell_get_video_settings` | `0x000123b0` | `shell/shell.c` |
| `shell_test_device` | `0x00012420` | `shell/shell.c` |
| `shell_create_device` | `0x00012490` | `shell/shell.c` |

- Archivos nuevos: `shell/shell.{c,h}`; `tests/test_shell.py`.

### Cambiado

- `tools/difftest.py`: al sustituir una función del juego que termina
  saltando a otra, se mira en esa otra cuántos argumentos quita de la pila.
  Antes se tomaba el primer retorno que hubiera después, que podía ser de
  otra función (pasaba con `cache_files_initialize`).

### Lo que se sabe

- El orden del arranque (`shell_main`): crear y soltar el dispositivo de
  Direct3D como prueba, reservar la memoria física, `shell_initialize`, el
  bucle principal y `shell_dispose`. La función no retorna: acaba en un
  punto de ruptura.
- `shell_initialize` prepara, en este orden: una biblioteca de la consola,
  0x23000 bytes de memoria, la plataforma, el modo de coma flotante
  (precisión simple), el hilo de tareas, dos sistemas aún sin identificar,
  los archivos de caché, otro sin identificar, el estado del juego y el
  rasterizador. Si no hay dispositivo de Direct3D se rinde; si lo hay,
  prepara dos sistemas más (en Halo 1 son la entrada y el sonido).
- Al salir no se detiene el hilo de tareas: solo se cierran sus cuatro
  identificadores.
- El dispositivo se pide a 640×480 con Z de 24 bits. El modo panorámico y
  el progresivo salen de los ajustes de la consola; la frecuencia (50 o
  60 Hz) solo se ajusta en consolas PAL.
- Si el juego se lanza desde el disco de demos (`XDEMOS`), se anota la
  palabra `xdemo` en una variable que leerá otra parte del juego.

### Sin comprobar

- Que `shell_initialize` se rinda cuando `shell_platform_initialize`
  devuelve falso: esa función siempre devuelve verdadero, el compilador de
  `source/` lo sabe y no mira el resultado. En el juego la comprobación
  está, pero nunca se cumple.
- En `shell_create_device`, poner a cero el puntero del dispositivo cuando
  ya es cero: el juego lo hace y no cambia nada.

### Pendiente

- Los doce sistemas que `shell_initialize` y `shell_dispose` preparan y
  desmontan y que aún no están descompilados (`unknown_...`), y el bucle
  principal (`0x0012b690`).

## 2026-10-01 — El estado del juego y los puntos de control

`game_state.c`: la memoria donde el juego guarda todo lo que hay que
recordar de una partida, y cómo hace un punto de control y vuelve a él. Con
ello, el CRC de 32 bits. De 183 funciones se pasa a 203, el 1,80 % de las
del juego (el 0,809 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_initialize` | `0x00123b30` | `saved games/game_state.c` |
| `game_state_dispose` | `0x00123bf0` | `saved games/game_state.c` |
| `game_state_initialize_for_new_map` | `0x00123c20` | `saved games/game_state.c` |
| `game_state_buffer_protect` | `0x00123b00` | `saved games/game_state.c` |
| `game_state_set_structure_bsp_index` | `0x00123d20` | `saved games/game_state.c` |
| `game_state_malloc` | `0x00123d40` | `saved games/game_state.c` |
| `game_state_malloc_aligned` | `0x00123d80` | `saved games/game_state.c` |
| `game_state_save` | `0x00123e20` | `saved games/game_state.c` |
| `game_state_note_death` | `0x00123ed0` | `saved games/game_state.c` |
| `game_state_revert` | `0x00123f60` | `saved games/game_state.c` |
| `game_state_header_valid` | `0x00124520` | `saved games/game_state.c` |
| `game_state_headers_match` | `0x00124590` | `saved games/game_state.c` |
| `game_state_set_revert_time` | `0x00124620` | `saved games/game_state.c` |
| `game_state_allocation_allocate` | `0x00124700` | `saved games/game_state.c` |
| `game_state_allocation_restore` | `0x00124750` | `saved games/game_state.c` |
| `game_state_call_before_load_procs` | `0x00199520` | `saved games/game_state.c` |
| `game_state_call_after_load_procs` | `0x00199540` | `saved games/game_state.c` |
| `crc_build_table` | `0x00163c00` | `memory/crc.c` |
| `crc_checksum_buffer` | `0x00163ba0` | `memory/crc.c` |
| `no_operation1` | `0x00072c70` | `cseries/cseries.c` |

- Archivos nuevos: `saved games/game_state.c`, `memory/crc.{c,h}`,
  `game/game.{c,h}` (solo las variables y estructuras que hacen falta);
  `tests/test_game_state.py`.

### Cambiado

- `tools/difftest.py`:
  - `memory={dirección: Memory(...)}` pone datos en una dirección fija
    antes de la llamada y los compara después: para lo que el juego alcanza
    por un puntero de una variable global.
  - `GlobalRange(tamaño)`: una variable que la prueba no toca, solo para
    que los punteros a ella se comparen bien.
  - Columna `convention` en `symbols/functions.csv`: `thiscall` para una
    función de `source/` que recibe su primer argumento en `ecx`.
  - La memoria física del emulador pasa de 1 MB a 8 MB, y el límite de
    instrucciones de 20 a 200 millones (poner a cero el estado son 4 MB).
- La biblioteca C de las pruebas gana `strchr`.
- Los tamaños de la memoria del estado estaban al revés: la parte normal es
  de 0x3be000 bytes y la del final, de 0x40000.

### Lo que se sabe

- La memoria del estado se reparte al arrancar, sin liberar nunca: cada
  sistema pide su parte con `game_state_malloc`. Los tamaños pedidos van a
  un CRC; un estado guardado con otro reparto tiene otro CRC y se rechaza.
- La cabecera del estado (0x1288 bytes) guarda ese CRC, la dirección de la
  memoria, el mapa, el build, un checksum del mapa, las opciones de la
  partida y el bsp cargado.
- Los dos archivos se alternan: cada punto de control se escribe en el que
  no se escribió la última vez, así siempre queda el anterior.
- Si el jugador muere cinco veces seguidas, cada una a menos de 8 segundos
  del punto de control, el juego pasa al punto de control anterior.
- Para volver a un punto de control se comprueba que sea de la misma
  partida y del mismo mapa. Si no lo es, o no hay ninguno, se pide
  reiniciar el mapa. La lectura se repite hasta que lo leído sea el punto
  de control.
- Antes de guardar se llama a 4 funciones y después a 1; antes de cargar a
  5 y después a 20. Solo se conocen cuatro de ellas; las demás están
  apuntadas por su dirección (`unknown_...`).
- `game_state_allocation` es un objeto que vive dentro del propio estado y
  da memoria del estado a quien pide un asignador (los `data_array`). Como
  al cargar un estado vuelve con la dirección de funciones de cuando se
  guardó, una de las funciones de después de cargar se la repone.

### Sin comprobar

- En `game_state_header_valid`, que el nombre del mapa no esté vacío: el
  juego lo comprueba, pero la comprobación siguiente (que tenga una barra)
  ya lo implica, así que quitarla no cambia ningún resultado.
- Si la búsqueda de la barra es `strchr` o `strrchr`: la función del juego
  (`0x003219f0`) no tiene nombre y aquí solo importa si la encuentra.

### Pendiente

- El resto de `game_state.c`: guardar y cargar el estado en un archivo de
  núcleo y en una partida guardada (`0x001240b0` a `0x00124490`,
  `0x00124640`, `0x00124770` en adelante).
- Las 21 funciones de antes y después de guardar y cargar.

## 2026-10-01 — Los archivos del estado del juego

Los archivos donde el juego guarda su estado para los puntos de control
(`n:\savegame0.bin` y `n:\savegame1.bin`): abrirlos, cerrarlos, escribir y
leer. Es el `game_state_xbox.c` de Halo 1, con los nombres de allí. De 177
funciones se pasa a 183, el 1,62 % de las del juego (el 0,759 % de sus
bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `game_state_create_or_open_file` | `0x00214f80` | `saved games/game_state_xbox.c` |
| `game_state_close_file` | `0x002150f0` | `saved games/game_state_xbox.c` |
| `game_state_write_finish` | `0x00215140` | `saved games/game_state_xbox.c` |
| `game_state_write_reclaim` | `0x00215180` | `saved games/game_state_xbox.c` |
| `game_state_write_to_file` | `0x002151f0` | `saved games/game_state_xbox.c` |
| `game_state_read_from_file` | `0x002152e0` | `saved games/game_state_xbox.c` |

- Archivos nuevos: `saved games/game_state.h` y
  `saved games/game_state_xbox.c`; `tests/test_game_state_xbox.py`.

### Corregido

- `cache_file_buffer_allocate` (`0x00214f10`) no era de los archivos de
  caché: es `game_state_allocate_buffer`, la que reserva la memoria del
  estado del juego. Se ha movido a `game_state_xbox.c`, y
  `cache_file_buffer_globals` pasa a ser `xbox_game_state_globals`, con
  todos sus campos.
- `physical_memory_buffer_allocate` recibe dos funciones, no una: la
  segunda es la que el juego llama para recuperar la memoria.

### Cambiado

- `tools/difftest.py`: al sustituir una función de `source/` que quita sus
  argumentos de la pila (`stdcall`), la pila queda como la dejaría la
  función.

### Lo que se sabe

- Hay dos archivos de estado, de 0x40b000 bytes cada uno; el estado ocupa
  0x3fe000 (0x40000 más 0x3be000).
- Un punto de control no para el juego: se copia el estado a memoria
  prestada y una tarea escribe la copia con prioridad 2. Si no hay memoria
  para la copia, se escribe el estado mismo con prioridad 3 y el juego
  espera, dibujando la pantalla de carga.
- Un archivo solo se puede leer si su última escritura llegó entera. Antes
  de leer o de volver a escribir se espera a la escritura que esté en curso.
- En Halo 1 había un solo archivo y se escribía y leía sin tareas.

### Sin comprobar

- Los dos argumentos de `game_state_write_reclaim` cuando la llaman
  `game_state_write_to_file` y `game_state_read_from_file`: la función no
  los usa, y el compilador de `source/` no los pone. No se comparan.

### Pendiente

- `game_state.c` (`0x00123b30` en adelante): `game_state_initialize`,
  guardar y volver a un punto de control. Necesita el CRC (`0x00163ba0`).

## 2026-10-01 — La copia de mapas al disco duro

El juego copia cada mapa del DVD al disco duro en segundo plano, para
cargarlo más rápido. Queda descompilado todo ese mecanismo: la cola de
peticiones, la tabla de copias fallidas, la elección del sitio y la máquina
de veinte pasos que hace la copia. De 150 funciones se pasa a 177, el
1,57 % de las del juego (el 0,729 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cached_map_file_copy_update` | `0x002141f0` | `cache/cache_files.c` |
| `cached_map_file_request` | `0x00213d20` | `cache/cache_files.c` |
| `cached_map_file_status` | `0x00213f10` | `cache/cache_files.c` |
| `cached_map_file_copy_current_priority` | `0x00213e30` | `cache/cache_files.c` |
| `cached_map_file_copy_pending_priority` | `0x00213e90` | `cache/cache_files.c` |
| `cached_map_file_copy_priority` | `0x00213ef0` | `cache/cache_files.c` |
| `cached_map_file_copy_cancel` | `0x00214990` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_reclaim` | `0x00213fa0` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_acquire` | `0x00213fe0` | `cache/cache_files.c` |
| `cached_map_file_copy_buffer_release` | `0x00214030` | `cache/cache_files.c` |
| `cached_map_file_copy_get_progress` | `0x002148b0` | `cache/cache_files.c` |
| `cached_map_file_get_progress` | `0x00214940` | `cache/cache_files.c` |
| `cached_map_file_failure_find` | `0x00214060` | `cache/cache_files.c` |
| `cached_map_file_failure_add` | `0x00214100` | `cache/cache_files.c` |
| `cached_map_file_failure_remove` | `0x002141b0` | `cache/cache_files.c` |
| `cached_map_file_failure_is_final` | `0x002141d0` | `cache/cache_files.c` |
| `cached_map_file_is_better_place` | `0x00214ac0` | `cache/cache_files.c` |
| `cached_map_file_choose_place` | `0x00214b80` | `cache/cache_files.c` |
| `cached_map_file_shared_index` | `0x00214c70` | `cache/cache_files.c` |
| `cache_file_map_paths_equal` | `0x00214e40` | `cache/cache_files.c` |
| `cache_file_xor_checksum` | `0x00123110` | `cache/cache_files.c` |
| `async_create_file` | `0x001a0b40` | `cseries/async.c` |
| `async_copy_work` | `0x001a1100` | `cseries/async.c` |
| `async_copy_position` | `0x001a1210` | `cseries/async.c` |
| `async_flush_file_work` | `0x001a1680` | `cseries/async.c` |
| `async_flush_file` | `0x001a16a0` | `cseries/async.c` |
| `file_time_get_current` | `0x00136e70` | `cseries/cseries.c` |

- `tests/test_map_copy.py`.

### Corregido

- `cache_file_header_verify` (`0x00122870`) pide la hora a la consola antes
  de comprobar la cabecera, sin usarla. La versión descompilada no lo hacía;
  salió a la luz al probar la máquina de la copia, que compara las llamadas.

### Cambiado

- `tools/difftest.py`:
  - Las funciones de la consola de `DEFAULT_EXTERNALS` (por ahora, la hora)
    se simulan solas si la prueba no las da.
  - Una función que es solo un salto a una función del kernel se puede
    sustituir.
  - `GlobalData` admite `clean`, para dejar fuera de la comparación lo que
    el juego copia de memoria que nunca escribió.
  - Un puntero al final exacto de una variable ya no se confunde con el
    principio de la siguiente.
- En la cabecera de un mapa, los 8 bytes en `0x14c` son la fecha en que se
  hizo la copia (`copy_time`).
- Los 0x840 bytes sin nombre de `cached_map_file_globals` son la tabla de
  copias fallidas, y `cached_map_file_copy_globals` tiene ya todos sus
  campos con nombre.
- `struct cache_file_time` pasa a ser `struct file_time`, en `cseries.h`.

### Lo que se sabe

- Solo se copia un mapa a la vez y solo uno puede esperar: el pedido con
  más prioridad, o el último. Si el que espera tiene tanta o más prioridad
  que el que se copia, la copia en curso se cancela.
- Los pasos: abrir el mapa del disco, leer su cabecera, comprobar que los
  mapas compartidos que usa están en el disco duro y son los mismos con
  los que se hizo, medir el archivo, elegir sitio, borrar la cabecera de
  la copia, copiar los datos por trozos, y escribir la cabecera al final.
  Así una copia a medias nunca tiene cabecera válida.
- Lo copiado se comprueba con un checksum: el o-exclusivo de todas las
  palabras de 32 bits, comparado con el campo `checksum` de la cabecera.
- Los trozos son de 256 KB con prioridad 0 y de 1 MB con las demás. El
  buffer es memoria prestada: si el juego la reclama, la copia la devuelve
  al terminar el trozo y la vuelve a pedir después.
- Un mapa cuya copia falla tres veces no se vuelve a intentar. La tabla
  guarda ocho mapas; si se llena, se pierde el fallo más antiguo.
- El sitio de la copia depende del tipo de escenario: uno fijo para cada
  mapa compartido, el menú y el multijugador, y dos para la campaña, de
  los que se elige el de la copia más antigua. Nunca se pisa el mapa que
  está cargado.
- La última escritura de la cabecera se pide con prioridad 0, no con la de
  la copia como las demás.

### Sin comprobar

- Los modos de `async_create_file` por encima de 5 y los tipos de
  escenario por encima de 4 en `cached_map_file_choose_place`: en el juego
  saltan a una tabla que no tiene esas entradas, así que no hay nada con
  qué comparar.

### Pendiente

- Quién llama a `cached_map_file_copy_update` y a `cached_map_file_request`
  desde fuera (`0x0012b450`, `0x0012bf40`, `0x00163890`).
- `physical_memory_buffer_allocate` y `physical_memory_buffer_free`
  (`0x0012d400`, `0x0012d520`), la memoria prestada.

## 2026-10-01 — Arranque de los archivos de caché

`cache_files_initialize`, lo primero que hace el juego con los mapas al
arrancar, y todo lo que necesita: abrir las seis copias de mapas del disco
duro, comprobarlas y rehacer las que no sirven. De 136 funciones se pasa a
150, el 1,33 % de las del juego (el 0,653 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `cache_files_initialize` | `0x002136c0` | `cache/cache_files.c` |
| `cached_map_files_open` | `0x00213970` | `cache/cache_files.c` |
| `cached_map_file_read_header` | `0x002149c0` | `cache/cache_files.c` |
| `cached_map_file_shared_get` | `0x00214c30` | `cache/cache_files.c` |
| `async_create_file_work` | `0x001a08f0` | `cseries/async.c` |
| `async_write_work` | `0x001a0fb0` | `cseries/async.c` |
| `async_write_position` | `0x001a1050` | `cseries/async.c` |
| `async_set_file_size_work` | `0x001a12c0` | `cseries/async.c` |
| `async_set_file_size` | `0x001a1310` | `cseries/async.c` |
| `async_close_file_work` | `0x001a1530` | `cseries/async.c` |
| `async_close_file` | `0x001a1550` | `cseries/async.c` |
| `async_get_file_size_work` | `0x001a15d0` | `cseries/async.c` |
| `async_get_file_size` | `0x001a15f0` | `cseries/async.c` |
| `csstrcat256` | `0x00122810` | `cseries/cseries.c` |

- `tests/test_cached_map_files.py`, y más casos en `tests/test_async.py`.

### Cambiado

- `tools/difftest.py` llena la pila de un valor que no es cero antes de cada
  llamada, para que se note una variable que solo uno de los dos lados
  inicializa.
- En la cabecera de un mapa, los tres bytes en `0x174` dicen de cuáles de
  los mapas compartidos toma recursos (`uses_shared_map`), en el orden de
  las tres últimas fechas: menú principal, `shared` y
  `single_player_shared`.

### Lo que se sabe

- Al arrancar, el juego borra las copias `cache006.map` a `cache019.map`
  (de otras versiones) y abre las seis que usa. Si el idioma de la consola
  o el número de build cambiaron desde la última vez, las borra todas.
- Una copia cuyo archivo no tiene el tamaño de su sitio se redimensiona y
  se le escribe una cabecera vacía; las copias que van detrás se borran,
  una sola vez.
- La cabecera vacía que se escribe es memoria de la pila sin inicializar con
  solo las cuatro cadenas vaciadas. Es un descuido del juego original y
  está reproducido tal cual.
- De una copia con el tamaño correcto se lee la cabecera. Solo la de
  `single_player_shared` se compara además con el mapa del disco (el campo
  en `0x144`). El código para comparar las fechas de los mapas compartidos
  está ahí, pero con una condición que nunca se cumple.
- Las tareas de archivo son cinco: abrir (puede crear las carpetas de la
  ruta), leer, escribir, cambiar el tamaño y cerrar, más la que mide.

### Sin comprobar

- Que `cached_map_files_open` vacíe la cadena del nombre en las dos
  cabeceras de la pila: quitar esa línea no cambia ningún resultado que las
  pruebas vean. El resto de la función sí se comprobó rompiendo líneas una
  a una.

### Pendiente

- Los que usan `cached_map_file_copy_globals` (`0x0055b4f0`), que copian un
  mapa del disco al disco duro: `0x002150f0` a `0x002152e0`, `0x00214ac0`.
- Las preferencias guardadas (`0x00120d80` a `0x00121280`).

## 2026-10-01 — El hilo que lee el disco

El juego no lee el disco desde el hilo principal: deja tareas en una lista y
otro hilo las va haciendo. Ese módulo queda descompilado entero, con la
lectura de archivos que usa la carga de mapas. De 117 funciones se pasa a
136, el 1,21 % de las del juego (el 0,584 % de sus bytes).

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `async_initialize` | `0x00120a90` | `cseries/async.c` |
| `async_task_allocate` | `0x001208b0` | `cseries/async.c` |
| `async_task_list_insert` | `0x00120900` | `cseries/async.c` |
| `async_task_queue` | `0x001209c0` | `cseries/async.c` |
| `async_task_list_remove` | `0x00120a30` | `cseries/async.c` |
| `async_task_add` | `0x00120ba0` | `cseries/async.c` |
| `async_task_count` | `0x00120bf0` | `cseries/async.c` |
| `async_thread_procedure` | `0x00120c30` | `cseries/async.c` |
| `async_thread_loop` | `0x00120c40` | `cseries/async.c` |
| `async_task_set_priority` | `0x00120ce0` | `cseries/async.c` |
| `async_wait` | `0x00120d50` | `cseries/async.c` |
| `async_read_work` | `0x001a0e70` | `cseries/async.c` |
| `async_read_position` | `0x001a0f10` | `cseries/async.c` |
| `async_read_goes_before` | `0x001a1720` | `cseries/async.c` |
| `csnzprintf` | `0x0011c9c0` | `cseries/cseries.c` |
| `language_from_xbox_language` | `0x0011ca80` | `cseries/cseries.c` |
| `cached_map_file_maximum_size` | `0x00214c90` | `cache/cache_files.c` |
| `cached_map_file_get_path` | `0x00214cc0` | `cache/cache_files.c` |
| `cached_map_files_delete` | `0x002138f0` | `cache/cache_files.c` |

- `tests/test_async.py`, y más casos en `tests/test_map_files.py`.

### Cambiado

- `tools/difftest.py`:
  - `GlobalData`: el valor de una variable global que tiene punteros a sí
    misma. El juego y `source/` no la tienen en la misma dirección, así que
    los punteros se comparan por el sitio al que apuntan.
  - `GlobalPointer`: un argumento que es la dirección de una variable global.
  - `StopCall`: una función simulada puede cortar la llamada, para probar
    funciones que nunca retornan (el bucle del hilo).
  - El tipo de retorno `pointer` en `symbols/functions.csv`.
  - Los punteros a funciones de `source/` se comparan por nombre.
- `vsnprintf` de la biblioteca C de las pruebas corta el texto como la del
  juego: sin cero al final cuando no cabe.
- Las variables sueltas `async_mutex` y `async_task_list` pasan a ser campos
  de `async_globals`.
- Los dos argumentos sin nombre de `cache_file_read_async` y
  `async_read_position` son la categoría y la prioridad de la tarea.

### Lo que se sabe

- Hay 150 tareas como mucho. Cada una tiene prioridad, categoría, diez
  parámetros, una función y un aviso que se pone a 1 al terminar.
- La lista se ordena por prioridad. Entre lecturas de la misma prioridad se
  ordena por archivo y posición a partir de donde acaba la lectura
  anterior, para que el lector del disco no vaya y venga.
- La primera tarea de la lista solo cede su sitio a una de más prioridad:
  es la que el hilo puede tener a medias.
- Una lectura se hace en trozos de 256 KB como mucho; la función de la
  tarea devuelve 0 mientras le quede y el hilo la vuelve a llamar.
- Las copias de los mapas en el disco duro se llaman `cache000.map` a
  `cache005.map`: las dos compartidas en `z:` y las demás en `n:`. Cada
  sitio tiene un tamaño máximo.
- `0x00510834` es un bloque de 0x1e8 bytes que el juego guarda firmado
  (las preferencias): el idioma está en `+4` y el número de build de las
  copias de los mapas en `+0x1d4`. `0x00510819` avisa de que cambió.

### Pendiente

- `cache_files_initialize` (`0x002136c0`) y la función que copia los mapas
  al disco duro (`0x00213970`), que Ghidra no descompila bien.
- `main_loading_idle` (`0x00125d60`), que `async_wait` llama mientras espera.

## 2026-10-01 — Carga de los tags de un mapa

La cadena completa con la que el juego carga un mapa: construir la ruta del
archivo, buscarlo entre los mapas abiertos, reservar memoria, leer el índice
y los datos de los tags y dejar apuntados el escenario y los globals. De 104
funciones se pasa a 117, el 1,04 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `scenario_load` | `0x001228f0` | `cache/cache_files.c` |
| `scenario_tags_unload` | `0x00122af0` | `cache/cache_files.c` |
| `cache_file_read_async` | `0x00213760` | `cache/cache_files.c` |
| `cache_file_read_header` | `0x00213800` | `cache/cache_files.c` |
| `cache_file_block_until_not_busy` | `0x00213890` | `cache/cache_files.c` |
| `cached_map_files_find_map` | `0x00214a60` | `cache/cache_files.c` |
| `cache_file_get_map_name` | `0x00214d40` | `cache/cache_files.c` |
| `cache_file_get_map_path` | `0x00214db0` | `cache/cache_files.c` |
| `cache_file_buffer_allocate` | `0x00214f10` | `cache/cache_files.c` |
| `physical_memory_initialize` | `0x0012b400` | `memory/physical_memory.c` |
| `async_category_busy` | `0x00120b50` | `cseries/async.c` |
| `csstrcasecmp` | `0x0011c920` | `cseries/cseries.c` |
| `csprintf256` | `0x000b66f0` | `cseries/cseries.c` |

- Archivos nuevos: `memory/physical_memory.{c,h}`, `cseries/async.{c,h}`,
  `scenario/scenario.{c,h}` (las variables `global_scenario_index`,
  `global_scenario` y `global_game_globals`).
- `tests/test_scenario_load.py` y `tests/test_map_files.py`.

### Cambiado

- `tools/difftest.py`:
  - Una prueba puede sustituir cualquier función de
    `symbols/functions.csv` por nombre, esté descompilada o no. Así cada
    función se prueba sola, sin depender de las que llama.
  - `source/` se compila con `-fno-inline`, para que una llamada siga
    siendo una llamada y se pueda sustituir.
  - Una variable global puede recibir bytes (la estructura entera), que se
    comparan al terminar.
  - El emulador tiene memoria en `0x80000000`, donde la consola pone su
    memoria física.
- La biblioteca C de las pruebas (`tools/difftest_libc`) gana `strrchr`,
  `strspn`, `strpbrk`, `towlower` y `vsnprintf`.
- En la cabecera de un mapa, el campo de 256 bytes en `0x20` es la ruta
  del archivo. En el disco está vacío: lo rellena el juego al abrirlo.

### Lo que se sabe

- El juego pide al arrancar un bloque de memoria física de `0x00061000` a
  `0x03145000` (unos 49 MB) y lo reparte por los dos extremos, en páginas
  de 4 KB. Hay cinco etapas; cada una parte de lo que dejó la anterior.
- Los tags se leen en dos partes dentro del mismo bloque: el índice al
  principio y los datos al final. En medio queda el hueco del bsp.
- El juego tiene una tabla de seis mapas abiertos, cada uno con su cabecera.
  Los tres primeros son `shared`, `single_player_shared` y `mainmenu`. Los
  dos bits altos de un desplazamiento eligen de cuál se lee.
- La ruta de un mapa es `d:\maps\<nombre>.map`, con el nombre en
  minúsculas y sin extensión. Si el nombre empieza por `t:\` o `w:\` se
  usa tal cual.
- Si una lectura de los tags falla, el juego pone a cero el número de
  compilación guardado (`0x00510a08`, normalmente 9809). Eso le hace
  copiar otra vez los mapas al disco duro.
- `cache_file_block_until_not_busy` espera a las tareas de las categorías
  2 a 6.

### Nombres supuestos

- Todos menos `csstrcasecmp`, `cache_file_get_map_path`,
  `cache_file_block_until_not_busy` y `scenario_tags_unload`, que hacen lo
  mismo que las de Halo 1 con ese nombre. `scenario_load` reúne lo que en
  Halo 1 son `scenario_load` y `scenario_tags_load`.
- La estructura de las tareas (`async_task`) y sus campos.

## 2026-10-01 — Lectura del mapa y fin de las tablas de datos

Cuatro funciones más: de 100 se pasa a 104, el 0,92 % de las del juego.
Son las primeras que llaman a funciones del juego aún sin descompilar.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `data_new` | `0x0016b570` | `memory/data.c` |
| `data_dispose` | `0x0016b5d0` | `memory/data.c` |
| `cache_file_blocking_read` | `0x00122d60` | `cache/cache_files.c` |
| `cache_file_structure_bsp_load` | `0x00122b40` | `cache/cache_files.c` |

- Con `data_new` y `data_dispose` el módulo de tablas de datos
  (`memory/data.c`, 20 funciones) queda completo.
- `tools/mapinfo.py`: lista el contenido de un mapa. Sustituye al programa
  en C.

### Cambiado

- `tools/difftest.py` puede probar funciones que llaman a otras:
  - `Mock`: una función simulada para un puntero a función (por ejemplo,
    las de un asignador de memoria).
  - Funciones externas: una fila de `symbols/functions.csv` sin archivo de
    origen es una función del juego sin descompilar. La prueba pone la
    misma función simulada en el original y en el nuestro.
  - Las llamadas que reciben las funciones simuladas, con sus argumentos,
    forman parte de lo que se compara.
- `build.ninja` solo comprueba los tamaños de las estructuras
  (`tools/verify_definitions.c`). Ya no genera ningún programa.

### Quitado

- `tools/mapinfo/` y `tools/common/` (el `mapinfo` en C). Smart App Control
  bloqueó `mapinfo.exe` al recompilarlo. No queda ningún ejecutable propio.

### Lo que se sabe

- Una tabla de datos guarda el asignador con el que se creó: un objeto con
  dos funciones, reservar y liberar. `data_dispose` libera con él.
- `cache_file_blocking_read` pide la lectura a `0x00213760` (sin
  descompilar) y cede el procesador hasta que termina.
- `cache_file_structure_bsp_load` redondea el tamaño del bloque a sectores
  de 512 bytes, lo lee en la dirección que dice el escenario, comprueba la
  firma `sbsp` y da su dirección a los tags del bsp y de su lightmap.
- `cache_file_globals` tiene en `+4` un puntero a la memoria de los tags.

### Nombres supuestos

- `cache_file_blocking_read`, `cache_file_structure_bsp_load` y
  `cache_file_read_async` (`0x00213760`).

## 2026-10-01 — Cien funciones

Cinco funciones sueltas más: de 95 se pasa a 100, el 0,89 % de las del
juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `bit_vector_test_flag` | `0x000bfe60` | `cseries/cseries.c` |
| `fast_vector_sphere_intersection` | `0x0011e6f0` | `math/real_math.c` |
| `normalize3d_with_default` | `0x001201a0` | `math/real_math.c` |
| `bitmap_group_get_bitmap` | `0x00137550` | `bitmaps/bitmap_group.c` |
| `path_remove_last_element` | `0x001373c0` | `cseries/cseries.c` |

### Lo que se sabe

- `bitmap_group_get_bitmap` no pasa por `tag_get`: lee la dirección del tag
  en la tabla de instancias sin comprobar su tipo ni su identificador.
- `path_remove_last_element` deja la ruta vacía si no tiene ninguna barra
  invertida.

### Nombres supuestos

- Los cinco. `bitmap_group_get_bitmap` es el nombre de Halo 1 para una
  función que allí recibe el grupo y no su índice.

## 2026-10-01 — Acceso a los tags

Las funciones con las que casi todo el juego llega a sus datos. De 90 se
pasa a 95, el 0,84 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `tag_get` | `0x00122c10` | `cache/cache_files.c` |
| `tag_iterator_next` | `0x00122c70` | `cache/cache_files.c` |
| `cache_file_tag_group_compare` | `0x00122cf0` | `cache/cache_files.c` |
| `cache_file_tag_group_get` | `0x00122d00` | `cache/cache_files.c` |
| `binary_search` | `0x0013ddd0` | `memory/binary_search.c` |

- Las variables globales `cache_file_globals` (`0x005476f8`) y
  `global_tag_instances` (`0x004e3b44`).
- `tests/test_tags.py`: unos 3.700 casos, con índices de tags generados al
  azar.

### Cambiado

- `tools/difftest.py`: un argumento puede ser un puntero a una función
  (cada lado recibe la suya), una dirección dentro de un bloque de la misma
  llamada, y una variable global puede ser un campo de una estructura
  (`nombre+desplazamiento`).

### Lo que se sabe

- `tag_get` comprueba el tipo del tag pedido: lo acepta si es del grupo
  pedido o de un grupo que lo tiene como padre o abuelo. Para eso busca el
  grupo del tag, por búsqueda binaria, en la tabla de grupos.
- La comparación de grupos es una resta. Solo ordena bien valores cercanos,
  como son los cuatro caracteres de un grupo; con valores cualesquiera de 32
  bits la resta se desborda y la búsqueda falla.
- `cache_file_globals` es la de Halo 1 con un campo más: la dirección de la
  memoria de los tags, antes de la cabecera.
- Las funciones que el juego pasa a otras funciones retiran sus argumentos
  de la pila ellas mismas.

### Nombres supuestos

- `binary_search`, `cache_file_tag_group_get` y
  `cache_file_tag_group_compare`. `tag_get` y `tag_iterator_next` son los de
  Halo 1.

## 2026-10-01 — Cuaterniones y restos de módulos

De 83 funciones se pasa a 90, el 0,80 % de las del juego.

### Añadido

| Función | Dirección | Archivo |
| --- | --- | --- |
| `quaternion_from_vectors3d` | `0x0011d6a0` | `math/real_math.c` |
| `quaternion_multiply` | `0x0011d820` | `math/real_math.c` |
| `fast_quaternion_interpolate_and_normalize` | `0x0011d950` | `math/real_math.c` |
| `rectangle3d_build_vertices` | `0x0011f9a0` | `math/real_math.c` |
| `matrix4x3_from_plane` | `0x00142390` | `math/matrix_math.c` |
| `matrix3x3_multiply` | `0x00142eb0` | `math/matrix_math.c` |
| `data_connect` | `0x0016b6b0` | `memory/data.c` |

### Cambiado

- `tools/difftest.py`: un argumento real puede ir en un registro SSE
  (`xmm0` a `xmm3`), que es donde lo recibe
  `fast_quaternion_interpolate_and_normalize`.

### Lo que se sabe

- El módulo de matemáticas reales termina hacia `0x00120890`; lo que sigue
  (`0x001208b0` en adelante) es otro módulo, con semáforos y colas, que
  parece la carga de datos en segundo plano.
- `data_connect` reconstruye el vector de bits de una tabla a partir de los
  identificadores de sus elementos. Sirve para tablas cuyos elementos vienen
  ya hechos, como las de un mapa.

### Nombres supuestos

- `quaternion_from_vectors3d`, `quaternion_multiply`,
  `rectangle3d_build_vertices` y `data_connect`.

## 2026-10-01 — Cadenas anchas y más matemáticas

De 66 funciones se pasa a 83, el 0,74 % de las del juego.

### Añadido

- `source/text/unicode.c` y `unicode.h`: cadenas de caracteres de 16 bits.

  | Función | Dirección |
  | --- | --- |
  | `ustrnzcat` | `0x0013fa70` |
  | `ustrnzcpy` | `0x0013fac0` |
  | `ustrnupr` | `0x0013fae0` |
  | `wchar_string_to_ascii_string` | `0x0013fb90` |
  | `ascii_string_to_wchar_string` | `0x0013fc90` |
  | `character_in_ranges` | `0x0013fcd0` |
  | `character_is_east_asian` | `0x0013fd90` |

- En `source/math/real_math.c`:

  | Función | Dirección |
  | --- | --- |
  | `limit3d` | `0x000a75d0` |
  | `quaternion_normalize` | `0x0011d610` |
  | `euler_angles2d_from_vector3d` | `0x0011df30` |
  | `vectors3d_from_euler_angles3d` | `0x0011df60` |
  | `vectors3d_from_euler_angles2d` | `0x0011dfb0` |
  | `project_point2d` | `0x00120790` |
  | `project_point3d` | `0x00120810` |

- `bsp3d_get_plane_from_designator` (`0x0014a300`) en `physics/bsp3d.c`.
- `real_alpha_to_pixel32` (`0x00131fc0`) y
  `real_alpha_and_rgb_color_to_pixel32` (`0x00131f40`) en
  `bitmaps/bitmap_color_conversion.c`.
- `tests/test_unicode.py` y más casos en los demás: unos 12.000 casos nuevos.

### Lo que se sabe

- El módulo de matemáticas reales del juego está entre `0x0011cf50` y
  `0x00120850`: unas 50 funciones, de las que hay 12 hechas. Las que quedan
  son más grandes (hasta 1.385 bytes).
- El código propio del juego está en direcciones múltiplo de 16. Las 2.581
  funciones que no lo están (394.000 bytes) se concentran en zonas que
  parecen librerías enlazadas: `0x220000`–`0x260000` y `0x2b0000` en
  adelante. Havok estará entre ellas.
- `ustrnupr` pasa a mayúsculas también la é y la ó, las únicas letras
  acentuadas que trata.
- Las funciones de proyección usan una tabla de ejes (`0x00440b94`): para
  cada eje dominante de un plano y cada sentido, qué dos ejes forman sus
  puntos 2D.

### Nombres supuestos

- `ustrnzcat`, `ustrnzcpy`, las dos conversiones entre cadenas anchas y
  ASCII, `character_in_ranges`, `character_is_east_asian` y
  `real_alpha_and_rgb_color_to_pixel32`. Los demás son los de Halo 1.

## 2026-10-01 — Primeras funciones de objetos

Las primeras funciones que usan el estado global del juego. De 59 se pasa a
66, el 0,59 % de las del juego.

### Añadido

- `source/objects/objects.c` y `objects.h`: la cabecera de un objeto, lo que
  se conoce de sus datos y siete funciones.

  | Función | Dirección |
  | --- | --- |
  | `object_wake` | `0x000b7360` |
  | `object_get_origin` | `0x000b9dd0` |
  | `object_get_world_matrix` | `0x000ba160` |
  | `object_get_location` | `0x000ba300` |
  | `object_try_and_get_and_verify_type` | `0x000badc0` |
  | `object_iterator_next` | `0x000baeb0` |
  | `object_get_ultimate_parent` | `0x000baf80` |

- `symbols/globals.csv`: el registro de variables globales, con su dirección
  en el ejecutable. La primera es `object_header_data` (`0x004e0300`), la
  tabla de cabeceras de objetos.
- `tests/test_objects.py`: unos 4.000 casos, con mundos de objetos generados
  al azar, algunos enganchados a otros.

### Cambiado

- `tools/difftest.py` y `tools/test_functions.py`: una prueba puede dar
  valor a variables globales para una llamada (un entero, o un bloque de
  memoria al que la variable apunta). El valor se escribe en la variable del
  ejecutable y en la de `source/`.

### Lo que se sabe de los objetos

- La cabecera de un objeto es la de Halo 1: identificador, flags, tipo y la
  dirección de sus datos.
- De los datos de un objeto: el objeto al que está enganchado (`0x14`) y el
  nodo de este (`0x18`), su localización en el nivel (`0x28`), su posición
  (`0x64`), sus vectores adelante (`0x70`) y arriba (`0x7C`), y dónde están
  las matrices de sus nodos (`0x116`).
- Un objeto enganchado a otro tiene su posición relativa al nodo del otro.

### Nombres supuestos

- `object_wake` y el bit de cabecera que activa; `object_get_location` y
  `object_get_world_matrix`.

## 2026-10-01 — Flujo de bits y más vectores

De 51 funciones se pasa a 59, el 0,52 % de las del juego.

### Añadido

- `source/networking/bitstream.c` y `bitstream.h`: el flujo de bits con el
  que el código de red empaqueta sus mensajes.

  | Función | Dirección |
  | --- | --- |
  | `bitstream_write_bits` | `0x001955d0` |
  | `bitstream_write_integer` | `0x00195720` |
  | `bitstream_read_bool` | `0x001957d0` |
  | `bitstream_read_bits` | `0x00195820` |
  | `bitstream_read_integer` | `0x001959c0` |

- En `source/math/real_math.c`: `projection_from_vector3d` (`0x00120850`),
  `fast_vector_intersects_sphere` (`0x0011e5e0`) y
  `fast_quaternion_normalize` (`0x001d8d60`).
- `tests/test_bitstream.py` y más casos en `tests/test_real_math.py`: unos
  3.900 casos nuevos.

### Lo que se sabe

- Halo 1 no tiene flujo de bits: su red es otra. Los nombres de estas
  funciones son supuestos.
- Las funciones que escriben en el flujo no borran los bits que pisan: dan
  por hecho que desde la posición en adelante hay ceros.
- `bitstream_read_bool` lee también en la posición justo después del final
  de los datos.
- `projection_from_vector3d` y `fast_vector_intersects_sphere` son las de
  Halo 1.

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
