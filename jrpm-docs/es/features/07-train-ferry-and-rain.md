---
title: Ferry de trenes y lluvia
---

# Ferry de trenes y lluvia

> Fuente: pulsexlb/OpenTTD-patches `px-patch` (lote 2026-10, `pxp-2610.1` – `pxp-2610.3`), fusionado en jrpm mediante git merge.

Este lote trae dos grandes novedades: el **ferry de trenes** (barcos que transportan trenes completos) y el **sistema de lluvia** (con oscurecimiento del mundo y capa de gotas).

## Ferry de trenes

Más allá del transporte de vehículos de carretera (RoRo) existente, donde los coches se cargan en trenes, barcos y aviones, ahora los barcos pueden transportar **trenes completos**.

### Relación con RoRo

| | Transporte de vehículos de carretera (VEHC) | Transporte de trenes (RAIL) |
|---|---|---|
| Transportistas | trenes, barcos, aviones | barcos |
| Objeto transportado | vehículos de carretera | trenes (locomotoras y coches) |
| Etiqueta de carga | `VEHC` | `RAIL` |
| Ranura de carga | `NUM_CARGO - 1` | `NUM_CARGO - 2` |

Ambas ranuras de carga están fuera de las 64 ranuras de NewGRF, por lo que nunca chocan entre sí.

### Carga

- Un barco adquiere la capacidad de transportar trenes completos al **reequiparse manualmente** con la carga «Vehículos (tren)»;
- **Carga por vagón**: un tren puede repartirse entre varias bodegas en lugar de tener que caber entero en una, lo que elimina el problema de los trenes largos que superan la capacidad de una bodega;
- El **selector de cartel** de las opciones de transporte ofrece aparte los carteles de tren y de vehículo, de modo que cada tipo de transporte puede declarar su propio destino;
- La barra de estado ya no detalla los vehículos transportados, lo que libera espacio para información de marcha más importante.

### Visualización de capacidades

- La **ventana de información del barco** muestra el peso de carga que admite cada bodega;
- La **ventana de información del tren** muestra la capacidad de cada vagón.

### Correcciones relacionadas

Este lote también corrige varios defectos de carga, descarga y reserva de andén:

- los barcos juzgaban por error el tipo de vía como incompatible al descargar trenes, impidiendo que estos bajaran;
- la reserva de andén no se liberaba al subir un tren, y aparecían errores de estructura y de reserva al bajar;
- reservas de andén obsoletas impedían que los trenes volvieran a descargar en los andenes;
- ciertas orientaciones de andén no aceptaban la descarga de un tren;
- el límite de vagones no se aplicaba a los vehículos reequipados para transporte.

## Sistema de lluvia

Una simulación meteorológica puramente decorativa: solo afecta al aspecto visual y no toca la lógica de juego ni los valores económicos.

### Presentación del tiempo

- **Períodos de lluvia aleatorios**: el estado del tiempo lo governa un generador congruencial determinista sembrado con la semilla de generación del mapa, por lo que todos los jugadores y el servidor ven exactamente el mismo tiempo;
- **Oscurecimiento progresivo del mundo**: el mundo se oscurece gradualmente mientras llueve y se aclara cuando cesa. El oscurecimiento recorre `RAIN_SHADE_LEVELS` niveles y se lee al dibujar las ventanas de vista;
- **Capa de gotas**: una capa de lluvia a pantalla completa que **se adapta al zoom** y lleva una **vibración aleatoria** para que las gotas no parezcan una textura estática.

### Opción y cheat

- **Opción de dificultad** `difficulty.rain`: decide al iniciar una partida si el oscurecimiento del mundo durante la lluvia está activado;
- **Cheat de caja de arena** «Tiempo»: un ciclo de tres estados — automático / lluvia forzada / sol forzado.

### Partidas guardadas

El estado del tiempo (si está lloviendo ahora, la semilla del generador, el inicio del período de lluvia actual) y el cheat de tiempo de la caja de arena se **guardan junto con la partida**; tras cargar, el oscurecimiento salta directamente al tiempo actual en lugar de volver a fundirse.

- Bloque: `WTHR`;
- Indicador de funcionalidad: `XSLFI_WEATHER`;
- Código: `src/weather.cpp`, `src/weather.h`, `src/sl/weather_sl.cpp`.

## Código relacionado

- Ferry de trenes y capacidades: `src/roadveh_transport.cpp`, `src/cargo_type.h` (`CT_RAILVEHICLES`), `src/table/cargo_const.h`
- Efectos visuales de la lluvia: `src/weather.cpp`, `src/blitter/32bpp_anim.cpp`, `src/blitter/40bpp_anim.cpp`, `src/viewport.cpp`
- Guardado del tiempo: `src/sl/weather_sl.cpp`, `src/sl/extended_ver_sl.cpp` (`XSLFI_WEATHER`)
- Ajustes y cheat: `src/table/settings/difficulty_settings.ini` (`difficulty.rain`), `src/cheat_gui.cpp`, `src/cheat_type.h`
