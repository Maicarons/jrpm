---
title: Modular Airports (multitile-airport)
---

# Modular Airports (multitile-airport)

> Source: `jgrpp-multitile-airport` feature branch from pulsexlb/OpenTTD-patches, merged into jrpm via git merge.

## Feature Overview

Multitile modular airport overhaul: transforms airports from "fixed airport types" into a **freely modifiable layout** multitile system:

- **Multitile airports**: Airports consist of multiple functional tiles (runways, taxiways, aprons, terminals, helipads), combinable into any layout;
- **Airport layout modification**: After enabling `station.allow_modify_airports`, tiles can be added/removed/adjusted on existing airports;
- **Air type system**: New `air.h`/`air_type.h`/`newgrf_airtype.*` — abstracts aviation types (fixed-wing, helicopter, etc.) into an extensible air type system; NewGRF can define new aviation types and sprites;
- **PBS air traffic control**: `pbs_air.*` — an airborne version of runway/taxiway occupancy and signal reservation, supporting concurrent multi-aircraft taxiing;
- **YAPF air pathfinding**: Aircraft path planning on the ground (taxiing/waiting) and in the air uses the YAPF system.

## Core Capabilities

| Capability | Description |
|---|---|
| Airport layout modification | `station.allow_modify_airports` (**default on**; when enabled, existing airports can be modified) |
| Default air type | `gui.default_air_type` |
| Multitile airport sprites | Rebuilt openttd.grf (air type sprites), transparent sprite fixes |
| Aircraft behavior | Runway occupancy, taxi turns, takeoff/landing queues, helipads, spacecraft drawing |
| NewGRF compatibility | Airtype sprite loading, airport NewGRF callbacks |
| Save | `SLV_MULTITILE_AIRPORTS` save version |

## How to Use

1. Enable `station.allow_modify_airports` in game settings;
2. After building an airport, use the airport modification tool to adjust runway/stand/terminal layout;
3. Select the default air type in `gui.default_air_type`;
4. Use custom air types with aviation NewGRFs.

## Related Code

- Air types: `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- Air traffic control: `src/pbs_air.*`
- Aircraft/airport commands: `src/aircraft_cmd.cpp` (3600-line refactor), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- Pathfinding: `src/pathfinder/yapf` (aviation section)
- Save: `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## Notes

- This feature is a large-scale overhaul of the aviation system (aircraft_cmd.cpp refactored 3600+ lines). **It is recommended to perform focused regression testing after real machine compilation**: aircraft purchase/takeoff/landing, runway occupancy, airport GUI, save loading;
- During merge, unreferenced old types in the workspace were removed (`VehicleAirFlags`, `AirportMovingDataFlag`), confirmed no other files reference them;
- For existing airports in saves, back up the save before modifying.