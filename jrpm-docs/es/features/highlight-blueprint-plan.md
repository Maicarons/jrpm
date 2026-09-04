---
title: "Planificacion de la tercera tanda: sistema de resaltado + sistema de planos"
---

# Planificacion de la tercera tanda: sistema de resaltado + sistema de planos

Dos funciones de alto valor de cmclient han sido investigadas en profundidad. Esta pagina registra el plan de portabilidad y la estimacion de esfuerzo.

## I. Sistema de resaltado a nivel de objeto (cm_highlight, 2888 lineas)

**Funcion**: Cuando la herramienta de construccion esta activa, resalta en tiempo real el objeto que se va a construir -- vista previa precisa de **15 tipos de objetos**: tramos de via, area completa de estacion, carreteras/estacionamientos, senales, puentes, tuneles, muelles, aeropuertos, industrias, etc.

**Dependencias de portabilidad (API vanilla → API jrpm)**:

| Dependencia de cmclient | Correspondencia en jrpm |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (dentro de viewport.cpp) | Existe pero con firma diferente (jgrpp lo reestructuro), necesita alineacion uno a uno |
| `TileZoning` (coloracion de zonificacion urbana) | jrpm no tiene → necesita crearse |
| `_fn_mod` y otros estados globales | Especifico de cmclient → necesita rediseno |
| Constructores de objetos `ObjectTileHighlight::make_rail/road_stop/...` | Dependen de la API de NewGRF de estaciones/carreteras, jrpm tiene diferencias con vanilla |

**Esfuerzo**: Aproximadamente 2-3 rondas (4-6 horas cada una), la dificultad principal esta en la alineacion del pipeline de dibujo del viewport.

## II. Sistema de planos (cm_blueprint, 660 lineas)

**Funcion**: Seleccionar area → grabar secuencia de comandos de construccion (via/estacion/tunel/puente/senal) → 16 ranuras de almacenamiento → reconstruccion con un clic + rotacion.

**Dependencias de portabilidad**:

| Dependencia de cmclient | Correspondencia en jrpm |
|---|---|
| `cm_commands.hpp` **Capa de objetos de comando** (as_company / with_callback / set_auto / no_estimate + 100+ clases de comando generadas, 2251 lineas) | jrpm no tiene esta abstraccion → **debe portarse/reescribirse primero** |
| Estado `_station_gui` de `cm_station_gui.hpp` | La estructura de estado de la GUI de estaciones de jrpm es diferente |
| Puntero inteligente `sp<Blueprint>` | jrpm puede usar `std::shared_ptr` |
| Recorrido de tiles de `BlueprintCopyArea` | Depende de API genericas como `TileIndexDiffC` (jrpm las tiene) |

**Esfuerzo**: Aproximadamente 2-3 rondas (incluyendo la capa de objetos de comando).

## III. Orden de implementacion recomendado

```
Paso 1: Capa de objetos de comando (diseno de cm_command_type, reimplementar con Command<T>::Do/Post de jrpm)
         -- base comun para planos y reproduccion de comandos (load_commands)
Paso 2: Resaltado a nivel de objeto (alineacion del pipeline de viewport, portar objeto por objeto)
Paso 3: Planos (copiar/rotar/ranuras/reproduccion)
Paso 4 (opcional): Reproduccion de comandos (reproduccion completa de la partida con lzma) + Zonificacion urbana (growth_tiles, necesita extension de archivo)
```

## Nota previa

Estas tres funciones suman aproximadamente **5000+ lineas**, abarcan tres capas principales (sistema de comandos/archivo/dibujo de viewport) y dependen de varias capas intermedias que jrpm no tiene (objetos de comando, bus de eventos, serializacion de flujo de bits). Es mas seguro avanzar como proyecto especializado independiente -- se recomienda ejecutar en lotes, compilar y verificar en multijugador antes de cada commit, evitando grandes cambios de una sola vez que dificulten la localizacion de regresiones.