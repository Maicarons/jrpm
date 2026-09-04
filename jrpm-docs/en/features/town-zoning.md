---
title: Town Zoning
---

# Town Zoning

cmclient town zoning port (Batch 4, commit `9e3f95a2`). The jrpm base (pulsexlb) already has its own zoning system (toolbar + drawing pipeline + menu). This batch adds cmclient's unique evaluation modes and **growth_tiles save** on top of it.

## New Modes (zoning toolbar dropdown)

| Mode | Description | Coloring |
|---|---|---|
| **Town zones (Tz)** | Town concentric zoning, reuses `squared_town_zone_radius` | Tz0 edge=light blue / Tz1=red / Tz2=yellow / Tz3=green / Tz4 city center=white |
| **Town growth tiles** | Current month/previous month house construction and demolition traces | New house=green / Demolished=light blue / Rebuilt=white / Skipped growth=orange / Skipped build=yellow / Server demolished=red |

## growth_tiles Data Layer (`cm_town_growth.cpp/.h`)

- Two rolling month maps: `TileIndex → TownGrowthTileState` (current month / previous month)
- Event hooks:
  - `BuildTownHouse` → `NEW_HOUSE` (upgraded to `RH_REBUILT` if previous month was demolished)
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → monthly rotation (previous month = current month, current month cleared)
- **Save persistence**: New `GRWT` savegame chunk (`misc_sl.cpp`), serialized as a list of `{tile, state}` pairs; old saves without this chunk load fully compatible (verified save→load round-trip).

## How to Open

Toolbar menu → Map (Zoning) → Open zoning toolbar, inner and outer ring dropdowns select evaluation modes.

## Trim Notes

cmclient's CityBuilder server-specific modes (CB acceptance zone / CB town limit) and `ext::Town` extension fields (advertising, funds, cargo statistics) are server gameplay-specific and were not ported.