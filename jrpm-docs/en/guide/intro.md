---
title: Project Introduction
---

# OpenTTD-JRPM Project Introduction

## What is it

**OpenTTD-JRPM (jrpm)** is a secondary development branch based on [JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches) (jgrpp), incorporating the "train decoupling" and "modular airport" features from [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches), along with jrpm-specific functionality. Current version **jrpm-0.1.0**.

It is a fully standalone installable, standalone multiplayer version: jrpm clients/servers use an independent version identifier (`jrpm-0.1.0`) for handshake, fully isolated from original jgrpp and pulsexlb versions to avoid version confusion.

## Feature Overview

### From pulsexlb (152 commits, merged via git merge)
- **Train Decoupling (decouple)**: Complete train decouple/couple system -- decouple orders, path signal transfer, coupling length and speed limits, dual-head support, NewGRF coupling, coupling pathfinding (YAPF/NPF), and independent scheduling after decoupling;
- **Modular Airports (multitile-airport)**: Multitile airport system overhaul -- air type system (`air.h`/`air_type.h`/`newgrf_airtype.*`), PBS air traffic control (`pbs_air.*`), YAPF air pathfinding, `station.allow_modify_airports` for airport layout modification, `gui.default_air_type` for default air type.

### jrpm Exclusive
| Feature | Description | Entry Point |
|---|---|---|
| Resource Download: Multi-Mirror + Parallel | Comma-separated multi-mirror sources, file-level parallel download (configurable concurrency), automatic mirror fallback on failure | Settings → `network.content_mirrors` / `network.content_download_parallel` |
| Vehicle Auto-Grouping | One-click auto-create and assign groups by shared orders/schedules, group names automatically derived from route names | Group window button / Console `autogroup` |
| Build Cost Tooltip | Real-time estimated cost displayed at mouse cursor when building rails/roads/terrain | Rail/road/terrain toolbar |
| Whole-Game-Aware AI | Retains NoAI, adds new `AIGlobal` global API + example AI GlobalAI, access controlled by game setting | `game.script.allow_global_ai_access` |
| Server Multi-Version Compatibility | jrpm server accepts jrpm / original jgrpp / pulsexlb clients simultaneously | Server-side check activates automatically |

### Complete Inheritance
- All jgrpp features (signal enhancements, scheduled dispatch, tracerestrict, template replacement, etc.);
- All upstream OpenTTD functionality and NewGRF/script ecosystem compatibility.

## Version Relationship

```
                 jgrpp-0.73.1 (common ancestor)
                 /                 \
 jgrpp branch (63 commits)        pulsexlb px-patch (152 commits)
 ├ jgrpp recent updates            ├ train decoupling (decouple branch)
 ├ jrpm exclusive features         └ modular airports (multitile-airport branch)
 └ version renamed jrpm-0.1.0
                 \                 /
                  jrpm branch (git merge)
```

## Branches and Commits

- Branch: `jrpm` (development mainline)
- Key commits:
  - `d4c45740` jrpm exclusive 5 features
  - `71fe214c` merge pulsexlb (decoupling + modular airports)
  - `425e7207` version renamed openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` server multi-version client compatibility
  - `4716b925` parallel download concurrency configurable

## License

Same as OpenTTD: **GPL-2.0**.