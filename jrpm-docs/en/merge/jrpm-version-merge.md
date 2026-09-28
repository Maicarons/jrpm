---
title: jrpm Version Merge Overview
---

> Branch: `jrpm` ｜ Version: jrpm-0.1.0 (tagged, 2026-08-14)
> Build output: `openttd-jrpm` (executable name)

## Version Relationship

```
                        jgrpp-0.73.1 (common ancestor)
                        /                 \
        jgrpp branch (63 commits)          pulsexlb px-patch (152 commits)
        ├ tracerestrict and other recent updates  ├ jgrpp-decouple (train decoupling)
        ├ my 5 features (d4c45740)                └ jgrpp-multitile-airport (modular airports)
        └ version renamed jrpm-0.1.0 (425e7207)
                        \                 /
                        jrpm branch (merge 71fe214c + compatibility cb9848b)
```

## Merge Contents

### 1. pulsexlb/OpenTTD-patches (px-patch full 152 commits) → Merged

| Feature | Description | Main Files |
|---|---|---|
| **Train Decoupling (decouple)** | Train decouple/couple: decouple orders, path signal transfer, coupling length/speed limits, dual-head, NewGRF coupling, coupling pathfinding (YAPF/NPF), independent scheduling after decoupling | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **Modular Airports (multitile-airport)** | Multitile airport system overhaul: air type system (air.h/air_type.h/newgrf_airtype.*), PBS air traffic control (pbs_air.*), YAPF air pathfinding, `station.allow_modify_airports` (modify airport layout), `gui.default_air_type`, multitile airport sprites | air.*, pbs_air.*, aircraft_cmd.cpp (3600-line refactor), airport_cmd/gui, station_cmd |

Conflict resolution: Only 2 header file conflicts (aircraft.h / airport.h) — pulsexlb's aviation refactor removed **unreferenced** dead types from the workspace (`VehicleAirFlags` bitset, `AirportMovingDataFlag`), took pulsexlb's deletion side, confirmed no other files reference them.

### 2. Openttd-Cluster (User's Rust Cluster Project) → Selective Reference

| Patch | Handling | Description |
|---|---|---|
| 0006 vanilla-native-server (multi-version client compatibility) | ✅ **Merged** (cb9848b7) | jrpm server simultaneously accepts jrpm / original jgrpp (`jgrpp-`) / pulsexlb (`pxp`) clients; NewGRF version still strictly validated |
| 0001 revision-handshake / 0005 version-metadata | ✅ Concept adopted | jrpm uses an independent tagged revision string `jrpm-0.1.0`, multiplayer handshake isolated from jgrpp/pxp, achieving "new version for easy multiplayer" |
| 0007 parallel-download (HTTP thread pool + Range chunked download, 30KB) | 📝 Reference, not merged | Same topic as this project's F1 "file-level parallelism + multi-mirror" and modifies the same files; 0007's **transport layer thread pool/chunked download** recorded as a future enhancement direction for F1 |
| 0002-0004 snapshot/command/FFI bridge | 📝 Architecture reference | Depends on the entire otc-engine Rust runtime (FFI statically linked), belongs to "long-term overall integration" rather than patch-level merge; jrpm currently maintains a pure C++ single binary |

### 3. jrpm Exclusive (Previous 5 Features, d4c45740) → Already in jrpm Branch

Parallel download (multi-mirror + 4 concurrent sessions), auto-grouping, build tooltip, whole-game-aware AI (ScriptGlobal + GlobalAI), mirror/content server settings.

## Multiplayer Strategy ("New Version for Easy Multiplayer")

- jrpm is a **tagged version**: `IsNetworkCompatibleVersion` requires exact revision string match → **jrpm clients only connect to jrpm servers**, fully isolated from jgrpp 0.73.x / pxp;
- **Server relaxed**: jrpm server additionally accepts `jgrpp-*` and `pxp*` clients (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, NewGRF version must match);
- Therefore: jrpm server host = only accepts jrpm players (default); when needing compatibility with old clients, it can accept jgrpp/pxp players without configuration changes.

## Build and Verification (User's Local Machine Execution)

```bash
# First time (requires CMake + dependencies, see COMPILING.md)
cmake -B build ..
cmake --build build -j
# Output: build/openttd-jrpm.exe
```

Verification priority:
1. `openttd-jrpm -v` shows `jrpm-0.1.0`;
2. Single-player run for 1-2 game years (merge involves major train/airport changes + save version may be bumped due to allow_modify_airports, etc.);
3. After hosting: jrpm client connects ✓; original jgrpp 0.73.x client tries to connect (expected to join, if NewGRF matches);
4. Train decoupling: add decouple/couple orders to trains, verify independent scheduling after decoupling; modular airports: enable `station.allow_modify_airports` and modify airport layout;
5. Previous 5 features regression (parallel download, auto-grouping, build tooltip, GlobalAI).

## Known Risks

- **Not compiled/verified**: The merged+modified code has not been compiled on a local machine (no toolchain); first real compilation may reveal interface changes that were missed (especially the three major overhauls: aircraft/airport/train);
- Save version: px-patch may have bumped SLV (M9 mentioned savegame version gate), jrpm saves may not be readable by jgrpp 0.73.x saves (same jgrpp convention, downward compatible with trunk saves);
- Merge brings in pulsexlb's full history; use `git log --oneline pulsexlb/px-patch` to trace feature ownership.
---

## Merge log: 2026-09-28 (jgrpp-0.73.3 + px-patch 2609.x)

> Merge commits: jgrpp 94 commits (up to `jgrpp-0.73.3`, 48e8d51c6c) + px-patch 113 commits (up to past `pxp-2609.10`, ec1a3d4fb3).

### New upstream content

| Source | Content | Notes |
|---|---|---|
| pulsexlb | **Road vehicle transport (RoRo)** | Trains/ships/aircraft refit to carry road vehicles; dedicated VEHC cargo; see Features → Road Vehicle Transport |
| pulsexlb | **128 cargo types** | `NUM_CARGO` 64→128, CargoTypes now a Uint128 bitset; savegames gated via XSLFI_CARGO_TYPES_128 |
| pulsexlb | **Decouple/couple fix batch** | Coupled claim state persistence (CPLM chunk / XSLFI_COUPLE_CLAIM_STATE), propulsion/label/platform-command fixes and more |
| pulsexlb | **Schedule enhancements** | Post-decouple schedule execution opens a window, schedule menu conflict fix, no-engine train panel |
| jgrpp | **Order drag & drop + double click** | Orders can be dragged; stop location / RV direction need a double click (0.73.3) |
| jgrpp | **Double-ended ships** | NewGRF double-ended ships can reverse without turning (SLV_DOUBLE_ENDED_SHIPS / XSLFI_DOUBLE_ENDED_SHIPS) |
| jgrpp | Routine fixes | Departures column spacing, order tag dropdown, ByteReader overflow, Robin Hood label hashing, etc. |

### Key conflict handling

- **Saveload layer**: pulsexlb is based on the old SLE_ macro system while jrpm/jgrpp 0.73.3 uses VarFileType/VarMemType + the VarTypes struct. All saveload/ and sl/ conflicts were rewritten in the modern system, with **U128 support added** (VarFileType::U128=13, VarMemType::U128, SLE_UINT128, SlSaveLoadConv128 converted to modern enums);
- **jrpm-occupied version numbers**: `SLV_MULTITILE_AIRPORTS`(367)/`SLV_ORDER_DECOUPLE`(368) kept; upstream's `SLV_LABEL_ORIENTATION_UNIFICATION`/`SLV_DOUBLE_ENDED_SHIPS` shift to 369/370 (NSL alias enum comments note the upstream numbers); upstream savegame compat still goes through the XSLFI_UPSTREAM_VERSION sub-chunk;
- **order_gui.cpp**: jgrpp's drag fields (drag_start_pt/drag_start_time_us) merged into jrpm's order-list refactor; drag-drop callbacks use target-mode accessors (NumOrders/OrderAt) so order-list windows are safe; the upstream double-click semantics adopted;
- **train_cmd.cpp**: jrpm's couple-on-contact logic kept (ValidateCoupleCandidate), crash resolution switched to the upstream TrainsCrashed helper; the `enable_decouple` gate kept;
- **Language files**: jrpm-specific airport strings kept, shared strings taken from upstream ({WHITE} prefix dropped); pulsexlb's RV_TRANSPORT error strings translated into Simplified Chinese;
- **Misc**: CargoLabel 4-char literals converted to string construction (CT_VEHICLES); integer grfid switch converted to GrfID byte-string comparison (OpenGFX+ Airports conversion table); console_cmds keeps both autogroup and the RORO_DEBUG_COMMANDS debug block.

### Post-merge status

- Build: MinGW ninja passes, artifact `build/openttd-jrpm.exe`;
- Run: new-game smoke test passed (see build verification).
