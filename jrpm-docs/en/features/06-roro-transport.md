---
title: Road Vehicle Transport (RoRo)
---

# Road Vehicle Transport (RoRo: Road-vehicle on Road-vehicle)

> Source: pulsexlb/OpenTTD-patches `px-patch` (September 2026 batch), merged into jrpm via git merge.

## Overview

"Road vehicle transport" lets trains, ships and aircraft **carry road vehicles directly**:

- Road vehicles no longer have to drive everywhere on their own: they can "hitch a ride", being carried to a distant station and then driving off on the road network again;
- A carrier (train/ship/aircraft) gains the ability to carry road vehicles by being **refitted to the "Vehicles (Road)" cargo** (dedicated cargo label `VEHC`, which uses up cargo space);
- On the road-vehicle side, the order flags "**Wait to be transported**" and "**Unload here**" pair up with the carrier's "load road vehicles" / "unload road vehicles" order flags.

## How to use

### Carrier side (train/ship/aircraft)

1. Refit the train in a depot **manually** to the "Vehicles (Road)" cargo — refitting into a carrier can only be done manually; order-list refitting does not apply;
2. Enable "**Load road vehicles**" on a station order: the train picks up waiting road vehicles at that station;
3. Optional pairings:
   - "Wait for load" (depart only when loaded);
   - "Destination match": only load road vehicles whose declared unload station equals the carrier's next stop;
   - "Unload all road vehicles here": unload everything, ignoring each vehicle's declared station.

### Road vehicle side

1. Set "**Wait to be transported**" on a station order: the vehicle stops there and waits for a carrier;
2. Set "**Unload here**": the vehicle drives off the carrier at this station;
3. The two options are mutually exclusive (one per order);
4. When unloaded, the road vehicle runs a pathfinder pass to pick the best platform to leave from.

## Details and rules

- **Dedicated cargo slot**: road vehicle transport uses cargo slot 128 (`NUM_CARGO - 1`), outside the 64 slots NewGRFs can define; the total number of cargo types is extended from 64 to **128**;
- **Dedicated-carrier detection**: when every part of a vehicle is refitted to "Vehicles (Road)", its order buttons default to road vehicle transport; vehicles carrying any normal cargo default to normal cargo;
- **Carrier parts setting**: `vehicle.rv_transport_carrier_parts` decides which parts may carry road vehicles (any part / oversized only / bulk+oversized+vehicles cargo);
- **Cross-company loading**: optionally allow loading/unloading other companies' vehicles with automatic fee settlement;
- **"Carried too long" warning**: a one-time warning when a road vehicle has been carried for too long;
- **Player-created order lists**: shared/standalone order lists support road vehicle transport flags as well.

> Ships can additionally carry **whole trains** (dedicated `RAIL` cargo), added in the 2026-10 batch. See "Train Ferry and Rain".

## Savegame compatibility

- Waiting/carried state and order flags are persisted;
- Old savegames (without the XSLFI_CARGO_TYPES_128 flag) are read with 64 cargo slots and stay compatible.

## Console debug commands (off by default)

The `rvtransport` command family (list/wait/orderflag/attach/detach/selftest etc.) is only compiled with the CMake option `RORO_DEBUG_COMMANDS=ON`, for regression testing.

## Related code

- Core: `src/roadveh_transport.h`, `src/cargo_type.h` (VEHC cargo)
- Orders: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_base.h` (OrderExtraInfo)
- Train loading: `src/train_cmd.cpp`, `src/station_cmd.cpp`
- Cargo extension: `src/sl/station_sl.cpp`, `src/sl/company_sl.cpp` (XSLFI_CARGO_TYPES_128)
