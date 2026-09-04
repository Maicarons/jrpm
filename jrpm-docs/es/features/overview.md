---
title: Vision general de funciones
---

# Vision general de funciones

jrpm = todas las caracteristicas de jgrpp + pulsexlb (desenganche de locomotoras + aeropuertos modulares) + modded (historial de viajes + velocidad de taxi) + referencia de cmclient (mejoras de UI multijugador) + funciones exclusivas de jrpm.

## Funciones exclusivas de jrpm

| # | Funcion | Documentacion | Estado |
|---|---|---|---|
| 1 | Descarga de recursos: multiples espejos + paralelo | [Descarga de recursos](./01-resource-download) | ✅ Implementado (concurrencia configurable) |
| 2 | Estudio de limites del servidor | [Estudio de limites del servidor](./02-server-caps) | 📖 Conclusion de investigacion (limitaciones estructurales) |
| 3 | Agrupacion automatica de vehiculos | [Agrupacion automatica de vehiculos](./03-vehicle-autogroup) | ✅ Implementado |
| 4 | IA de percepcion global | [IA de percepcion global](./05-global-ai) | ✅ Implementado |

> Nota: La funcion F4 original "Tooltip de precio de construccion" se elimino segun requerimiento (commit `96ebfb75`).

## Portabilidad de modded (primera tanda)

| Funcion | Documentacion | Estado |
|---|---|---|
| Historial de viajes de vehiculos | [Historial de viajes](./triphistory) | ✅ Implementado |
| Velocidad de taxi de aviones ajustable | [Velocidad de taxi](./plane-taxi-speed) | ✅ Implementado |

## Referencia de cmclient (segunda a quinta tanda)

| Funcion | Documentacion | Estado |
|---|---|---|
| Marcadores de posicion / Detalles de carga / Espectador | [Mejoras de UI multijugador](./ui-enhancements) | ✅ Implementado (segunda tanda) |
| Sistema de resaltado a nivel de objeto | [Resaltado + Planos](./highlight-blueprint-plan) | ✅ Implementado (tercera tanda, 6 commits) |
| Sistema de planos (copiar/rotar/ranuras/reconstruir) | [Resaltado + Planos](./highlight-blueprint-plan) | ✅ Implementado (tercera tanda) |
| Zonificacion urbana + archivo growth_tiles | [Zonificacion urbana](./town-zoning) | ✅ Implementado (cuarta tanda) |
| Grabacion y reproduccion de comandos | [Reproduccion de comandos](./command-replay) | ✅ Implementado (quinta tanda) |

> **② Capa de objetos de comando** (2251 lineas de codigo generado de cmclient): Segun la experiencia de portabilidad, se puede **omitir por completo** -- el resaltado usa `CMD_ERROR` para estimacion de costos, los planos usan cierres de comando, la reproduccion usa serializacion nativa de comandos de jrpm, por lo tanto no se porto.

## Caracteristicas combinadas (de pulsexlb)

| Funcion | Documentacion | Descripcion |
|---|---|---|
| Desenganche de locomotoras (decouple) | [Desenganche de locomotoras](./decouple) | Sistema completo de desenganche/enganche de trenes |
| Aeropuertos modulares (multitile-airport) | [Aeropuertos modulares](./multitile-airport) | Reestructuracion del sistema de aeropuertos de multiples casillas |

## Herencia completa

- **Todas las caracteristicas de jgrpp**: Mejoras de senales (senales multiples/senales programables/ranuras y contadores), scheduled dispatch, tracerestrict, reemplazo de plantillas, ventana de compra separada de locomotoras/vagones, frenado realista, mejoras de carreteras de un solo sentido, seguridad en pasos a nivel, etc.;
- **Kernel OpenTTD 16.0**: Compatibilidad completa con el ecosistema NewGRF/scripts/archivos.

## Encuentre rapidamente la configuracion

| Funcion | Configuracion |
|---|---|
| Concurrencia de descarga paralela | `network.content_download_parallel` (1-8) |
| Lista de espejos de descarga | `network.content_mirrors` |
| Servidor de contenido | `network.content_server` |
| Modificacion de disposicion de aeropuerto | `station.allow_modify_airports` |
| Tipo de aviacion predeterminado | `gui.default_air_type` |
| Interruptor de percepcion global de IA | `game.script.allow_global_ai_access` |
| Velocidad de taxi de aviones | `vehicle.plane_taxi_speed` (1-8, predeterminado 4) |

## Encuentre rapidamente los comandos de consola

| Funcion | Comando |
|---|---|
| Agrupacion automatica de vehiculos | `autogroup train\|road\|ship\|aircraft` |
| Anadir IA de percepcion global | `start_ai GlobalAI` |
| Marcadores de posicion | `savelocation <1-9>` / `gotolocation <1-9>` |
| Detalles de carga de la compania | `company_cargo <company_id>` |
| Espectador de compania | `watch <company_id>` |
| Copiar/rotar/guardar/cargar/reconstruir planos | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| Grabacion/reproduccion de comandos | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| Velocidad/estadisticas/exportacion/arboles del juego | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>` |