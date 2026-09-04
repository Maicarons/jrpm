---
title: Feature Overview
---

# Feature Overview

jrpm = jgrpp full features + pulsexlb (train decoupling + modular airports) + modded (trip history + taxi speed) + cmclient reference (multiplayer UI enhancements) + jrpm exclusive features.

## jrpm Exclusive Features

| # | Feature | Documentation | Status |
|---|---|---|---|
| 1 | Resource Download: Multi-Mirror + Parallel | [Resource Download](./01-resource-download) | ✅ Implemented (configurable concurrency) |
| 2 | Server Capacity Research | [Server Capacity Research](./02-server-caps) | 📖 Research findings (structural limits) |
| 3 | Vehicle Auto-Grouping | [Vehicle Auto-Grouping](./03-vehicle-autogroup) | ✅ Implemented |
| 4 | Whole-Game-Aware AI | [Whole-Game-Aware AI](./05-global-ai) | ✅ Implemented |

> Note: The former F4 "Build Cost Tooltip" has been removed per request (commit `96ebfb75`).

## modded Port (Batch 1)

| Feature | Documentation | Status |
|---|---|---|
| Vehicle Trip History | [Trip History](./triphistory) | ✅ Implemented |
| Adjustable Plane Taxi Speed | [Taxi Speed](./plane-taxi-speed) | ✅ Implemented |

## cmclient Reference (Batches 2-5)

| Feature | Documentation | Status |
|---|---|---|
| Location Bookmarks / Cargo Details / Spectate | [Multiplayer UI Enhancements](./ui-enhancements) | ✅ Implemented (Batch 2) |
| Object-Level Highlight System | [Highlight + Blueprint](./highlight-blueprint-plan) | ✅ Implemented (Batch 3, 6 commits) |
| Blueprint System (Copy/Rotate/Slots/Rebuild) | [Highlight + Blueprint](./highlight-blueprint-plan) | ✅ Implemented (Batch 3) |
| Town Zoning + growth_tiles Save | [Town Zoning](./town-zoning) | ✅ Implemented (Batch 4) |
| Command Recording & Replay | [Command Replay](./command-replay) | ✅ Implemented (Batch 5) |

> **② Command Object Layer** (cmclient's 2251-line generated code): Proven through porting practice to be **entirely bypassable** — highlighting uses `CMD_ERROR` for cost estimation, blueprints use command closures, replay uses jrpm native command serialization, therefore not ported.

## Merged Features (from pulsexlb)

| Feature | Documentation | Description |
|---|---|---|
| Train Decoupling (decouple) | [Train Decoupling](./decouple) | Complete train decouple/couple system |
| Modular Airports (multitile-airport) | [Modular Airports](./multitile-airport) | Multitile airport system overhaul |

## Complete Inheritance

- **All jgrpp features**: Signal enhancements (multi-signal, programmable signals, slots and counters), scheduled dispatch, tracerestrict, template replacement, locomotive/wagon separate purchase window, realistic braking, one-way road enhancements, level crossing safety, etc.;
- **OpenTTD 16.0 kernel**: Full compatibility with NewGRF/script/save ecosystem.

## Quick Settings Reference

| Feature | Setting |
|---|---|
| Parallel download concurrency | `network.content_download_parallel` (1-8) |
| Download mirror list | `network.content_mirrors` |
| Content server | `network.content_server` |
| Airport layout modification | `station.allow_modify_airports` |
| Default air type | `gui.default_air_type` |
| AI whole-game access toggle | `game.script.allow_global_ai_access` |
| Plane taxi speed | `vehicle.plane_taxi_speed` (1-8, default 4) |

## Quick Console Commands Reference

| Feature | Command |
|---|---|
| Vehicle auto-grouping | `autogroup train\|road\|ship\|aircraft` |
| Add whole-game-aware AI | `start_ai GlobalAI` |
| Location bookmarks | `savelocation <1-9>` / `gotolocation <1-9>` |
| Company cargo details | `company_cargo <company_id>` |
| Spectate company | `watch <company_id>` |
| Blueprint copy/rotate/save/load/build | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| Command record/replay | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| Game speed/stats/export/tree map | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>` |