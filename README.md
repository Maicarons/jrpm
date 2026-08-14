# OpenTTD-JRPM

OpenTTD-JRPM (jrpm) is a fork of [JGR's Patchpack](http://github.com/JGRennison/OpenTTD-patches),
additionally merging the pulsexlb px-patch features (train coupling/decoupling "loco swap" and
multi-tile modular airports), selected features from [OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded)
and [cmclient](https://github.com/citymania-org/cmclient), plus jrpm-specific additions (parallel
content download with multiple mirrors, automatic vehicle grouping by shared orders, a whole-game
perception AI API, and server-side multi-version client compatibility).

**Version 0.1.0** — based on JGR's Patchpack 0.73.1 + pulsexlb px-patch.

📖 **Documentation:** <https://maicarons.github.io/jrpm/>

[English](README.md) | [简体中文](README_zh.md)

## New features in this version

Beyond the full JGRPP feature set (signal enhancements, scheduled dispatch, tracerestrict,
template replacement, realistic braking, one-way road upgrades, level-crossing safety, ...) and
the pulsexlb merge (loco decouple / modular multi-tile airports), this version adds:

**From OpenTTD-modded**
- **Trip history** - every vehicle remembers its last 10 trips (profit, occupancy, trip time); a
  History button on the vehicle details window shows per-trip profit, % change and summary stats.
- **Configurable plane taxi speed** - `vehicle.plane_taxi_speed` (1-8, default 4) tunes the airport
  taxi speed limit independently.

**From cmclient (adapted)**
- **Object-level build highlights** - stations, tracks, depots, airports and industries render a
  live object preview (not just a rectangle) while placing/dragging build tools.
- **Blueprint system** - select an area, `blueprint_copy`, then `blueprint_build` elsewhere to
  rebuild rails, depots, tunnels, bridges, stations and signals (16 in-memory slots, rotate/save/load).
- **Town zoning** - the zoning toolbar gains "Town zones (Tz0-Tz4)" and "Town growth tiles" modes;
  growth tiles (houses built/removed this and last month) persist in a new `GRWT` savegame chunk
  that older saves load without changes.
- **Command record/replay** - `cmdrecord start [file]` / `cmdrecord stop` records every executed
  command; `cmdreplay <file>` replays them (uses jrpm's own command serialisation - no extra
  command-object layer needed).
- **Multiplayer UI helpers** - viewport location bookmarks (`savelocation`/`gotolocation`),
  per-company cargo details window (`company_cargo`), watch a company (`watch`), plus console
  commands `cmgamespeed`, `cmgamestats`, `cmexport`, `cmtreemap`.

**jrpm-specific**
- Parallel content download with multiple mirrors (configurable concurrency).
- Automatic vehicle grouping by shared orders (window button + `autogroup`).
- Whole-game perception AI (`AIGlobal` API + GlobalAI example).
- Server accepts jrpm / stock jgrpp / pulsexlb clients.

## Credits

jrpm would not exist without the work of these projects - many thanks to all of them:

- **[OpenTTD](https://github.com/OpenTTD/OpenTTD)** - the base game (GPL v2).
- **[JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches)** - the foundation this fork
  is built on (signal/schedule/tracerestrict enhancements, ...).
- **[pulsexlb/OpenTTD-patches (px-patch)](https://github.com/pulsexlb/OpenTTD-patches)** - loco
  coupling/decoupling and modular multi-tile airports.
- **[embeddedt/OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded)** - trip history and
  configurable plane taxi speed.
- **[citymania-org/cmclient](https://github.com/citymania-org/cmclient)** - object highlights,
  blueprint system, town zoning / growth tiles, command record-replay and multiplayer UI helpers.

---

This is a collection of features and other modifications applied to [OpenTTD](http://www.openttd.org/).
It's a separate version of the game which can be installed and played alongside the standard game,
not a loadable mod (NewGRF, script, or so on). It is mainly intended for players who are already
familiar with the standard game; some features and settings target very experienced players and may
have a steep learning curve.
