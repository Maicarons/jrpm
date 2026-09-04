---
title: Version History
---

# Version History

## jrpm-0.1.0 (2026-08-14) cmclient Port Complete

Based on the initial 0.1.0, completed the port of five batches of cmclient features (modded + cmclient full series):

### New Features (cmclient / modded Port)

- **modded Batch 1**: Vehicle trip history (`b89f93f9`) + Adjustable plane taxi speed (`b89f93f9`);
- **cmclient Batch 2**: Location bookmarks / Cargo details / Spectate (`089480b3`);
- **cmclient Batch 0**: `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` console commands (`1fd94d12`);
- **cmclient Batch 3 ① Highlight**: Object-level build preview (stations/tracks/depots/airports), hooked into viewport drawing pipeline and build tools (6 commits, `d97aa38a` → `1957bf45`);
- **cmclient Batch 3 ③ Blueprint**: Area selection copy/rotate/16 slots/reconstruct (`8e08ca6b`);
- **cmclient Batch 4 ④ Town Zoning**: Tz zoning + growth_tiles coloring + GRWT save chunk (`9e3f95a2`);
- **cmclient Batch 5 ⑤ Command Replay**: `cmdrecord` / `cmdreplay` (`f113acce28`);
- **Console parameter convention fix**: All jrpm commands now use argv[1] as the starting parameter (argv[0] is the command name).

### Architecture Conclusion

- **② Command object layer not ported**: Highlight/blueprint/replay bypass cmclient's 2251-line generated code by using `CMD_ERROR`, command closures, and jrpm native command serialization respectively.

### Save Compatibility

- growth_tiles data stored in a separate `GRWT` chunk; old saves (without this chunk) load fully compatible;
- Command record files (`.jrcm`) are jrpm's private format, cross-version compatibility is not guaranteed.

## jrpm-0.1.0 (2026-08-14)

First jrpm version, based on jgrpp 0.73.1 + pulsexlb px-patch merge, with ported modded / cmclient features.

### Changes

- **Merged pulsexlb px-patch (152 commits)**:
  - Train decoupling (decouple): decouple/couple orders, path signal transfer, coupling length/speed limits, dual-head, NewGRF coupling, coupling pathfinding, independent scheduling after decoupling;
  - Modular airports (multitile-airport): air type system, PBS air traffic control, YAPF air pathfinding, airport layout modification (`allow_modify_airports`);
  - Save versions added `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`.
- **Version rename**: `openttd-jrpm` / `jrpm-0.1.0` (executable name and revision string).
- **Server multi-version compatibility**: jrpm server accepts jrpm / original jgrpp / pulsexlb clients.
- **jrpm exclusive features**:
  - Resource download multi-mirror + file-level parallel (configurable concurrency);
  - Vehicle auto-grouping by shared orders (window button + `autogroup` command);
  - Whole-game-aware AI (`AIGlobal` API + GlobalAI example, controlled by setting toggle);
  - Build cost tooltip (removed per request, commit `96ebfb75`).
- **modded port (Batch 1)**:
  - Vehicle trip history (last 10 trips profit/occupancy/duration, History button in vehicle details window);
  - Adjustable plane taxi speed (`vehicle.plane_taxi_speed`, XSLF feature-gated, old save compatible).
- **cmclient reference (Batch 2)**:
  - Location bookmarks (`savelocation` / `gotolocation`, 9 slots);
  - Company cargo details window (`company_cargo`);
  - Spectate helper (`watch <company_id>`).

### Save Compatibility

- Inherits jgrpp convention: can load trunk saves (up to the most recently merged version);
- jrpm saves (containing multitile airport/decouple order data) **not guaranteed** to be readable by old jgrpp saves;
- Save version `SAVEGAME_VERSION` is consistent with pulsexlb (`SLV_CUSTOM_SUBSIDY_DURATION`);
- All new features use XSLF extended feature gating or NOSAVE, do not break old saves.

## Upstream Versions

- **jgrpp 0.73.1**: The base of this project (OpenTTD 16.0 kernel + all JGR features).
- **pulsexlb px-patch 2608.3**: Source of train decoupling and modular airports.
- **embeddedt/OpenTTD-modded (0.59.1 era)**: Source of trip history and taxi speed.
- **citymania-org/cmclient (vanilla 15.3)**: Source of location bookmarks/cargo details/spectate reference.

## Roadmap

- [x] Real machine build verification and first build error fixes
- [x] Trip history + Taxi speed (modded Batch 1)
- [x] Location bookmarks + Cargo details + Spectate (cmclient Batch 2)
- [ ] Highlight system + Blueprint system (cmclient Batch 3, see [Roadmap](../features/highlight-blueprint-plan))
- [ ] HTTP transport layer thread pool + Range chunked download (single large file acceleration)
- [ ] Map send compression comparison optimization
- [ ] Per-cargo income statistics (requires save format extension)
- [ ] (Long-term) Rust server runtime integration (reference Openttd-Cluster)