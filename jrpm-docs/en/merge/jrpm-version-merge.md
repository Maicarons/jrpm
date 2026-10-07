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

---

## Merge log: 2026-10-07 (jgrpp 0.73.3+89 + px-patch 2610.3)

> Merge commits: jgrpp 23 commits (after `jgrpp-0.73.3`, up to `6318727b02`) + px-patch 55 commits (up to `pxp-2610.3`, `4a4d0724b5`).

### What's new upstream

| Source | Content | Notes |
|---|---|---|
| pulsexlb | **Train ferry** | Ships can be refitted to carry whole trains (dedicated `RAIL` cargo, `CT_RAILVEHICLES`); loading is per-wagons, so the whole train no longer has to sit in one cargo hold |
| pulsexlb | **Rain system** | New `weather.cpp` simulation: random rainy periods, gradual world darkening, rain-drop overlay (scales with zoom, randomised jitter); difficulty option `difficulty.rain`; a sandbox cheat forces auto/rain/sun; weather state is saved with the game (`WTHR` chunk / `XSLFI_WEATHER`) |
| pulsexlb | **Same-direction decouple exit** | When decoupling you can let one part wait for the other to leave and then exit in the same direction; includes a fix so a partner that has not fully left the station is no longer treated as blocking |
| pulsexlb | **Capacity display** | The ship info window shows the capacity of each cargo hold; the train info window shows the capacity of each wagon |
| pulsexlb | **RoRo option improvements** | The road-sign picker in transport options now offers train and road-vehicle signs; trains load road vehicles per wagon; the status bar no longer lists carried vehicles |
| pulsexlb | **Coupling pathfinding performance** | Removed the coupling pathfinding stutter: cargo validation is now a pre-check with failure back-off instead of a full scan every throttle tick |
| pulsexlb | **Android build** | New Android build script and CI; APKs are published alongside releases |
| jgrpp | **StringID strong typing** | `StringID` is now a strong type (and no longer used as a template parameter); `Label` only accepts string / byte-array construction |
| jgrpp | **Duplicate company names** | Duplicate company names are now properly rejected; president name generation refactored to avoid gotos |
| jgrpp | **Widget preferred size** | Widgets can compute a preferred size from a given size |
| jgrpp | Assorted refactors and fixes | Order list expressions, NewGRF GEF ID checks, `GetSlopePixelZOnEdge` returning a tuple, a crash when exporting all order lists from non-group vehicle list windows |

### Key conflict handling

- **Save layer**: pulsexlb's new `WTHR` weather chunk still uses the old `SLEG_VAR`/`SLE_` macros, which remain available through the alias set in `sl/saveload_common.h`; only the outdated chunk-type enum `CH_TABLE` had to become the refactored `ChunkType::Table`;
- **Label / StringID strong typing** (the main compile obstacle this round): every 4-character literal label in jrpm's own code was switched to string construction (`CT_RAILVEHICLES{"RAIL"}`); the road and tram type label comparisons in `afterload.cpp` became `RoadTypeLabel{"ROAD"}` form; the empty `AirTypeInfo` string fields in `airport.cpp` now use `STR_NULL`; the `STR_CHEAT_RAIN` switch branches in `cheat_gui.cpp` use `.base()` (matching the existing `STR_CHEAT_CHANGE_COMPANY.base()` style);
- **cheat_gui.cpp cheat table**: jrpm's `VarMemType` field type and the `InflationCheat` sentinel are preserved, and pulsexlb's new rain cheat row is merged in (`SLE_VAR_U8` + `ClickRainCheat`);
- **jrpm_watch_gui.cpp**: `SetStringTip(SPR_GOTO_LOCATION, …)` became `SetSpriteTip` (upstream split the sprite/string tooltip interfaces);
- **Savegame version pin**: `SL_UPSTREAM_VERSION` is still pinned to 368 (upstream `DoubleEndedShips`) and the static_assert guard in `src/saveload/engine_sl.cpp` passes; jrpm's reserved 367/368 are unchanged and the newer upstream versions shift to 369/370;
- **train_cmd.cpp**: the decouple check keeps jrpm's `enable_decouple` gate and desync debug output, while adopting upstream's depot fix that only triggers a reverse when the front of the train passes the waypoint (`if (v->IsMovingFront())`); the same-direction exit flags after `want_decouple` come from upstream;
- **aircraft_cmd.cpp / window.cpp**: includes from both sides are kept (jrpm's pathfinder/pbs/roadveh_transport alongside upstream's checksum/script_event/widgets);
- **order_cmd.cpp**: `DrivingBackwards` adopts upstream's new `TCF_NO_DRIVING_CAB` semantics and jrpm's `DecouplePart` variable is retained; the language string becomes "Driving backwards at reduced speed" accordingly;
- **deploy-docs.yml**: jrpm's VitePress build and GitHub Pages deployment are kept (pulsexlb's Typst variant only serves its own `pxp-docs` directory);
- **README / .gitignore / .ottdrev-vc**: jrpm branding and local entries are preserved, with pulsexlb's Android build ignore entries and the `pxp-2610.3` version record merged in.

### State after the merge

- Build: passes with MinGW ninja (`-j3`), artifact `build/openttd-jrpm.exe`;
- Run: `-D` dedicated server smoke test passes — new map generation → save → load → save again → exit completes with no assertion failure and no `SetupEngines` crash; upstream chunk version gating behaves correctly.
