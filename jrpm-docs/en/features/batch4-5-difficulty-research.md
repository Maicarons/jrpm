# Batch 4 / Batch 5 / CM Server Commands — Port Difficulty Research Report

> Research target: citymania-org/cmclient (vanilla 15.3 branch) source code deep reading
> Research date: 2026-08-14
> Conclusion: The three work packages differ greatly in difficulty; individual assessments and recommended order are provided below.

---

## 1. Batch 4: Town Zoning (including growth_tiles save)

### Feature Composition

| Component | File | Size | Description |
|---|---|---|---|
| Zoning mode enum | cm_zoning.hpp | 37 lines | 12 evaluation modes (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| Evaluation logic | cm_zoning_cmd.cpp | 413 lines | One query function per mode (returns palette SpriteID), includes TownZone/Tz radius algorithm, StationFinder coverage check, unserviced building/industry check |
| Zoning toolbar GUI | cm_zoning_gui.cpp | 204 lines | Inner/outer two-layer zone mode selection window (`_zoning.inner/outer` global state) |
| growth_tiles save | cm_saveload.cpp/.hpp | 90 lines | Town extension fields `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), saved using vanilla SaveLoad handler |
| Data extensions | extensions/cmext_town.hpp | ~80 lines | `ext::Town`: growth_tiles ×2 + **many CM server gameplay fields** (CBTownInfo cargo stats, ad/fund tracking, hs/cs/hr town growth counters) |
| Monthly rotation + triggers | cm_game.cpp / town_cmd.cpp | — | NewMonth rotation of growth_tiles; record state at house build/demolish/rebuild sites |

### Port Difficulty Assessment by Item

| Difficulty | Severity | Description |
|---|---|---|
| **Save system rewrite** | 🟠 Medium | growth_tiles uses vanilla `DefaultSaveLoadHandler` + `SlSetStructListLength`/`SlObject`; jrpm uses the `sl/` new system (SlTableHeader/SlObjectSaveFiltered). Must be rewritten using jrpm's NSL/SLE + **XSLF feature gating** (new `XSLFI_TOWN_GROWTH_TILES`, version 1); old saves unaffected |
| **Town structure extension** | 🟢 Low | Only take the two growth_tiles maps (**skip** CBTownInfo/ad/fund fields — those are CM server CityBuilder gameplay-specific); add fields to jrpm Town + mount save table in town_sl.cpp |
| **Evaluation logic** | 🟢 Low-Medium | Mostly pure queries (GetTileType/StationFinder/Town cache/house checks), jrpm APIs exist; TownZone radius algorithm (`squared_town_zone_radius`) needs to match jrpm field names |
| **Rendering pipeline** | 🟠 Medium-High | `DrawTileZoning` needs to hook into the viewport drawing pipeline — **shares the TileHighlight rendering mechanism with Batch 3 highlighting**. Without the highlight pipeline, it must be built separately (recommended: **do highlighting first, then zoning**) |
| **Sprite resources** | 🟠 Medium | 12 palette modes use `CM_SPR_PALETTE_ZONING_*` **custom sprites**; jrpm does not have them → must use existing palette sprites or add new resources |
| **Event hooks** | 🟠 Medium | growth_tiles recording depends on cmclient's **event bus** (event::HouseBuilt/HouseCleared/..., emitted in cm_main.cpp). jrpm lacks this mechanism → add hooks directly in town_cmd.cpp at house build/demolish points + NewMonth rotation (IntervalTimer) |

### Difficulty Conclusion: 🟠 Medium-High (approximately 1.5–2 dedicated sessions, 4–6 hours each)

- **Prerequisite**: Strongly recommended to complete the **highlight system** from Batch 3 first (shared rendering pipeline)
- If only doing "12 zone colorings, without growth_tiles save": difficulty drops to 🟡 Medium (save extension omitted, about 1 session)

---

## 2. Batch 5: Command Replay + Export/Recording

### 2.1 Command Replay (cm_command_log + cm_commands + generated)

| Component | Size | Description |
|---|---|---|
| Command object layer | cm_command_type.hpp + generated/cm_gen_commands (2251+1418 lines) | Each vanilla command wrapped as a programmable object (as_company/with_callback/set_auto), includes **command-to-bitstream serialization** |
| Command log loading | cm_command_log.cpp (203 lines) | lzma decompression + BitOStream bitstream parsing → `_fake_commands` queue (tick counter/expected result/random seed/CommandPacket) |
| Command execution | ExecuteFakeCommands | Executes in tick counter order: `ExecuteCommand(&cp)` (vanilla internal API) + **random seed/result verification** (anti-cheat), forwards to all clients in multiplayer |

**Key Differences (Determining Difficulty)**:

| cmclient | jrpm | Impact |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | No such function; command core is `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | Need to write a CommandPacket → DoCommandPInternal payload conversion layer 🟠 |
| CommandPacket fields (vanilla) | CommandPacket exists but different structure (GeneralCommandPacket\<DynBaseCommandContainer\>) | Need to adapt field mapping 🟠 |
| `GetCommandName` | ✅ Exists (command_func.h:166) | 🟢 |
| `outgoing_queue` (multiplayer forwarding) | jrpm network layer is different (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | Multiplayer replay needs rewrite of forwarding logic 🟠 |
| Recorder (how to generate .cmd file) | **cmclient's recording hook depends on the command object layer's post() interception** | jrpm commands are templated Post → need to add recording hook at command dispatch point 🔴 Core work |
| File format | Private (vanilla command ID + bitstream) | jrpm command IDs are completely different from vanilla → replay files are incompatible, **format must be redesigned** 🟠 |

### 2.2 Export / Recording (cm_export.cpp, 536 lines)

| Feature | Description | Difficulty |
|---|---|---|
| ExportOpenttdData | JSON export of house specs/cargo specs/palette/engine info (JsonWriter) | 🟢 Low-Medium (standalone, read Spec structures, write JSON) |
| ViewportExport / ExportFrameSprites | Frame-by-frame export of viewport sprites (depends on viewport internal drawing vectors TileSpriteToDrawVector/ParentSpriteToSortVector) | 🟠 Medium-High (jgrpp viewport pipeline is different, need to align vector interfaces) |

### Difficulty Conclusion: 🟠 Medium-High (approximately 2 dedicated sessions)

- **Command replay is the biggest part**: Core work = ① Add **recording hook** at command dispatch point (adapting jrpm's templated command system) ② CommandPacket → DoCommandPInternal execution layer ③ New file format design. **Recommend porting the command object layer first** (foundation for Batch 3 blueprints and also for replay)
- Export: JSON data export can be done independently first (Low-Medium); frame recording depends on viewport alignment (same batch as highlighting)

---

## 3. CM Server Commands (cm_console_cmds.cpp, 289 lines)

### Command List and Per-Command Difficulty

| Command | Feature | Difficulty for jrpm | Notes |
|---|---|---|---|
| `cmgamespeed [n]` | Change game speed | 🟢 **Very Low** (~20 lines) | jrpm already has `_game_speed` global (gfx.cpp:52), only needs a command wrapper |
| `cmstep [n]` | Step n ticks | ⏭️ **jrpm already has it** | `step` command (ConStepGame) has the same functionality, skip |
| `cmexport` | Export openttd.json | 🟢 Low-Medium | Depends on ExportOpenttdData (see Batch 5) |
| `cmtreemap <file>` | Tree planting from heightmap | 🟡 Medium | Read heightmap + tree planting command, standalone |
| `cmreset_town_growth` | Clear town growth records | 🟢 Low | Depends on town zoning fields |
| `cmload_commands` | Load command replay | 🟠 Medium-High | Depends on command replay infrastructure |
| `cmstart_record` / `cmstop_record` | Frame recording | 🟠 Medium-High | Depends on recording infrastructure |
| `cmgamestats` | Game session statistics | 🟢 Low | Standalone |
| `cmgfxdebug` | Graphics debugging | 🟢 Low | Standalone |

### Difficulty Conclusion: 🟢 Overall Low (0.5–2 hours per command)

**Recommended order**: `cmgamespeed` (10 min) → `cmgamestats`/`cmgfxdebug` (~1h each) → `cmexport` (with Batch 5 export) → `cmtreemap` (~2h) → `cmreset_town_growth` (with Batch 4) → `cmload_commands`/`cmstart_record` (with Batch 5 replay/recording).

---

## 4. Overall Recommended Route

```
① Highlight system (Batch 3 core, ~2-3 sessions)      ← Rendering foundation for blueprints/zoning
② Command object layer (foundation for Batch 3 blueprints + Batch 5 replay, ~1-2 sessions)
③ Blueprints (~1 session)
④ Town zoning (~1.5 sessions, depends on ① rendering pipeline; growth_tiles uses XSLF gating)
⑤ Command replay + export (~2 sessions, depends on ②; JSON export can be done independently early)
⑥ CM server commands (scattered between batches; gamespeed/step can be done immediately)
```

## 5. Immediately Low-Cost Items (No Major Dependencies)

1. `cmgamespeed` command (~20 lines, change `_game_speed`)
2. `step` already exists → no need to implement
3. `cmgamestats` / `cmgfxdebug` (~1h each, standalone)
4. `cmexport` JSON data export (~2h, standalone)
5. `cmtreemap` (~2h, standalone)

These 5 items take about **half a day of work** and can be delivered as "Batch 0", running in parallel with Batches 3/4/5 without conflict.