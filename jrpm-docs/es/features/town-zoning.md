---
title: Zonificacion urbana (Town Zoning)
---

# Zonificacion urbana (Town Zoning)

Portabilidad de la zonificacion urbana de cmclient (cuarta tanda, commit `9e3f95a2`). La base de jrpm (pulsexlb) ya tiene su propio sistema de zonificacion (barra de herramientas + pipeline de dibujo + menu), esta tanda anade encima los modos de evaluacion exclusivos de cmclient y el **archivo de growth_tiles**.

## Nuevos modos (desplegable de la barra de herramientas de zonificacion)

| Modo | Descripcion | Coloracion |
|---|---|---|
| **Town zones (Tz)** | Zonas concentricas urbanas, reutiliza `squared_town_zone_radius` | Borde Tz0=azul claro / Tz1=rojo / Tz2=amarillo / Tz3=verde / Tz4 centro=blanco |
| **Town growth tiles** | Trayectoria de construccion y demolicion de casas de este mes/mes anterior | Casa nueva=verde / Demolicion=azul claro / Reconstruccion=blanco / Saltar crecimiento=naranja / Saltar construccion=amarillo / Demolicion del servidor=rojo |

## Capa de datos de growth_tiles (`cm_town_growth.cpp/.h`)

- Dos mapas mensuales rotativos: `TileIndex → TownGrowthTileState` (mes actual / mes anterior)
- Ganchos de eventos:
  - `BuildTownHouse` → `NEW_HOUSE` (si el mes anterior era demolicion, se actualiza a `RH_REBUILT`)
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → rotacion mensual (mes anterior = mes actual, mes actual se vacia)
- **Persistencia en archivo**: Nuevo chunk `GRWT` de savegame (`misc_sl.cpp`), serializado como lista de pares `{tile, state}`; los archivos antiguos sin este chunk se cargan con total compatibilidad (verificado con ciclo guardar→cargar).

## Como abrirlo

Menu de la barra de herramientas → Mapa (Zoning) → abrir barra de herramientas de zonificacion, los desplegables interior y exterior seleccionan el modo de evaluacion.

## Nota de recorte

Los modos exclusivos del servidor CityBuilder de cmclient (zona de aceptacion CB / limite de ciudades CB) y los campos de extension `ext::Town` (publicidad, fondos, estadisticas de carga) son exclusivos del modo de juego del servidor y no se han portado.