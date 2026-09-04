---
title: "Batch 3 Plan: Highlight System + Blueprint System"
---

# Batch 3 Plan: Highlight System + Blueprint System

Two high-value features from cmclient have been studied in depth. This page documents the porting plan and workload estimation.

## 1. Object-Level Highlight System (cm_highlight, 2888 lines)

**Feature**: When a build tool is active, displays a real-time highlight of what will be built — precise preview for **15 object types** including entire rail segments, full station areas, roads/parking lots, signals, bridges, tunnels, docks, airports, industries, etc.

**Porting Dependencies (vanilla API → jrpm API)**:

| cmclient Dependency | jrpm Equivalent |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (inside viewport.cpp) | Exists but with different signatures (jgrpp refactored them), need to align one by one |
| `TileZoning` (town zone coloring) | jrpm does not have it → needs to be created |
| `_fn_mod` and other global state | cmclient-specific → needs redesign |
| `ObjectTileHighlight::make_rail/road_stop/...` object constructors | Depends on NewGRF station/road APIs, differences between jrpm and vanilla |

**Workload**: Approximately 2–3 sessions (4–6 hours each), the core difficulty is aligning the viewport drawing pipeline.

## 2. Blueprint System (cm_blueprint, 660 lines)

**Feature**: Select area → record build command sequence (rails/stations/tunnels/bridges/signals) → 16-slot storage → one-click replay + rotation.

**Porting Dependencies**:

| cmclient Dependency | jrpm Equivalent |
|---|---|
| `cm_commands.hpp` **command object layer** (as_company / with_callback / set_auto / no_estimate + 100+ generated command classes, 2251 lines) | jrpm lacks this abstraction → **must port/rewrite first** |
| `cm_station_gui.hpp` `_station_gui` state | jrpm station GUI state structure is different |
| `sp<Blueprint>` smart pointer | jrpm can use `std::shared_ptr` |
| `BlueprintCopyArea` tile iteration | Depends on generic APIs like `TileIndexDiffC` (jrpm has them) |

**Workload**: Approximately 2–3 sessions (including the command object layer).

## 3. Recommended Implementation Order

```
Step 1: Command object layer (cm_command_type design, reimplemented using jrpm's Command<T>::Do/Post)
         —— Common foundation for blueprints and command replay (load_commands)
Step 2: Object-level highlighting (viewport pipeline alignment, port objects one by one)
Step 3: Blueprints (copy/rotate/slots/replay)
Step 4 (optional): Command replay (lzma full-game replay) + Town zoning (growth_tiles, needs save extension)
```

## Prerequisites

These three features total approximately **5000+ lines**, spanning three core layers: command system, save, and viewport rendering. They also depend on a set of intermediate layers (command objects, event bus, bitstream serialization) that jrpm does not have. It is safer to advance as a dedicated project — it is recommended to proceed in batches, with each batch compiled and multiplayer-verified before committing, to avoid hard-to-trace regressions from a single large change.