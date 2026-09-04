---
title: Train Decoupling (decouple)
---

# Train Decoupling (decouple)

> Source: `jgrpp-decouple` feature branch from pulsexlb/OpenTTD-patches (core of 152 commits), merged into jrpm via git merge.

## Feature Overview

"Train Decoupling" allows trains to **uncouple and couple** wagons or locomotives within stations/during operations, enabling:

- **Decouple**: A train, following a "decouple order," drops off some wagons (or a wagon+loco group) at a designated station, while the remainder continues its journey;
- **Couple**: A train waits at a station and automatically couples with another train (or a parked wagon group);
- **Independent scheduling after decoupling**: Using mechanisms like "conditional order skip," the two trains after decoupling execute different schedules;
- **Path signal transfer**: When decoupling/coupling, path signals (tracing restriction tags) are correctly transferred or deduplicated between trains.

## Core Capabilities

| Capability | Description |
|---|---|
| Decouple/Couple Orders | New order types supporting "load/do not load," "wait for coupling," "decouple," etc. |
| Coupling Length and Speed Limits | Coupling operations limited by train length; speed limits apply during coupling |
| Dual-Head Support | Front and rear dual-head (including NewGRF dual-head locomotives) correctly couple/decouple |
| Coupling Pathfinding | Both YAPF and NPF support coupling path planning; coupling only occurs within stations |
| Reverse Running | Can reverse after coupling, enabling techniques like "decouple then reverse" |
| NewGRF Coupling | Supports NewGRF-defined coupling attributes (0xC6/0xF2 properties, etc.) |
| Save | `SLV_ORDER_DECOUPLE` save version; `num_decouple` count persisted |

## How to Use

1. Use a "decouple" order (`decouple`) at a station to split the train;
2. Set a couple order for the train to wait for coupling;
3. Use "conditional order skip" to make the two trains after decoupling follow different routes;
4. Drag to adjust wagon order within the station, or use "filter by length" and other aids.

## Related Code

- Orders: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_type.h`
- Trains: `src/train_cmd.cpp`, `src/train.h`
- Pathfinding: `src/pathfinder/yapf`, `src/pathfinder/npf`
- Save: `src/sl/saveload_common.h` (`SLV_ORDER_DECOUPLE`)

## Notes

- This feature involves deep changes to train physical order and signal reservation logic. **It is recommended to perform focused regression testing after real machine compilation**: decouple, couple, reverse, crash recovery, autoreplace, etc.;
- Decouple-related strings and GUI have been merged (`STR_DECOUPLE*`, etc.).