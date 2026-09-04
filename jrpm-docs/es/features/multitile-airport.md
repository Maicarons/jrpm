---
title: Aeropuertos modulares (multitile-airport)
---

# Aeropuertos modulares (multitile-airport)

> Fuente: Rama de caracteristica `jgrpp-multitile-airport` de pulsexlb/OpenTTD-patches, incorporada a jrpm mediante git merge.

## Introduccion a la funcion

Reestructuracion de aeropuertos modulares de multiples casillas: transforma los aeropuertos de "tipos de aeropuerto fijos" a un sistema de multiples casillas con **disposicion libremente modificable**:

- **Aeropuerto de multiples casillas**: El aeropuerto se compone de multiples casillas funcionales (pista, calle de rodaje, plataforma de estacionamiento, terminal, helipuerto), que se pueden combinar en cualquier disposicion;
- **Modificacion de disposicion de aeropuerto**: Al activar `station.allow_modify_airports`, se pueden anadir/eliminar/ajustar casillas en aeropuertos existentes;
- **Sistema de tipos air**: Nuevos `air.h`/`air_type.h`/`newgrf_airtype.*` -- abstrae los tipos de aviacion (ala fija/helicoptero, etc.) en un sistema de tipos air extensible, NewGRF puede definir nuevos tipos de aviacion y sprites;
- **Despacho aereo PBS**: `pbs_air.*` -- version aerea de ocupacion de pista/calle de rodaje y reserva de senales, soporta rodaje concurrente de multiples aeronaves;
- **Busqueda de rutas YAPF para aviacion**: La planificacion de rutas de los aviones en tierra (rodaje/espera) y en el aire sigue el sistema YAPF.

## Capacidades principales

| Capacidad | Descripcion |
|---|---|
| Modificacion de disposicion de aeropuerto | `station.allow_modify_airports` (**activado por defecto**; al activarlo se pueden modificar aeropuertos existentes) |
| Tipo de aviacion predeterminado | `gui.default_air_type` |
| Sprites de aeropuerto de multiples casillas | openttd.grf reconstruido (sprites de air type), reparacion de sprites transparentes |
| Comportamiento de aeronaves | Ocupacion de pista, giro en rodaje, cola de despegue/aterrizaje, helipuerto, dibujo de naves espaciales |
| Compatibilidad NewGRF | Carga de sprites de airtype, callbacks de NewGRF de aeropuerto |
| Archivo | Version de archivo `SLV_MULTITILE_AIRPORTS` |

## Modo de uso

1. Active `station.allow_modify_airports` en la configuracion del juego;
2. Despues de construir un aeropuerto, use la herramienta de modificacion de aeropuerto para ajustar la disposicion de pista/puestos de estacionamiento/terminal;
3. Seleccione el tipo de aviacion predeterminado en `gui.default_air_type`;
4. Use tipos air personalizados con NewGRF de aviacion.

## Codigo relacionado

- Tipos de aviacion: `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- Despacho aereo: `src/pbs_air.*`
- Comandos de avion/aeropuerto: `src/aircraft_cmd.cpp` (reestructuracion de 3600 lineas), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- Busqueda de rutas: `src/pathfinder/yapf` (parte de aviacion)
- Archivo: `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## Nota

- Esta caracteristica es una reestructuracion a gran escala del sistema de aviacion (reestructuracion de aircraft_cmd.cpp con 3600+ lineas), **se recomienda encarecidamente realizar pruebas de regresion en una compilacion real**: compra/despegue/aterrizaje de aviones, ocupacion de pista, GUI de aeropuerto, carga de archivos;
- Durante la fusion se eliminaron tipos antiguos sin referencia en el area de trabajo (`VehicleAirFlags`, `AirportMovingDataFlag`), se confirmo que ningun otro archivo los referencia;
- Si necesita modificar aeropuertos existentes en archivos guardados, haga una copia de seguridad primero.