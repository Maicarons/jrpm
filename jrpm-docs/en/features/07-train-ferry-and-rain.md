---
title: Train Ferry and Rain
---

# Train Ferry and Rain

> Source: pulsexlb/OpenTTD-patches `px-patch` (2026-10 batch, `pxp-2610.1` – `pxp-2610.3`), merged into jrpm via git merge.

This batch brings two sizeable new features: the **train ferry** (ships carrying whole trains) and the **rain system** (with world darkening and a rain-drop overlay).

## Train ferry

Building on the existing Road Vehicle Transport (RoRo), where cars are loaded onto trains, ships and aircraft, ships can now also carry **whole trains**.

### Relation to RoRo

| | Road vehicle transport (VEHC) | Train transport (RAIL) |
|---|---|---|
| Carriers | trains, ships, aircraft | ships |
| Cargo carried | road vehicles | trains (locomotives and carriages alike) |
| Cargo label | `VEHC` | `RAIL` |
| Cargo slot | `NUM_CARGO - 1` | `NUM_CARGO - 2` |

Both cargo slots sit outside NewGRF's 64 cargo slots, so they never collide.

### Loading

- A ship gains the ability to carry whole trains by **manually refitting** it to the cargo "Vehicles (Train)";
- **Loading is per-wagon**: a train may be distributed across several cargo holds instead of having to fit into a single hold. This removes the problem of long trains exceeding one hold's capacity;
- The **road-sign picker** in the transport options now offers train and road-vehicle signs separately, so each kind of transport can declare its own destination;
- The status bar no longer lists the carried vehicles in detail, freeing the space for more important running information.

### Capacity display

- The **ship info window** shows the cargo weight capacity of each hold;
- The **train info window** shows the capacity of each wagon.

### Related fixes

This batch also fixed a number of loading, unloading and platform-reservation defects:

- Ships misjudging track type as incompatible when unloading trains, leaving trains unable to disembark;
- Platform reservations not being cleared when a train boards, plus structural and reservation errors on disembarking;
- Stale platform reservations preventing trains from unloading to platforms again;
- Some platform orientations failing to accept an unloading train;
- The wagon limit on vehicles refitted as transport carriers not being enforced.

## Rain system

A purely cosmetic weather simulation that affects visuals only — no gameplay logic or economic values are touched.

### Weather presentation

- **Random rainy periods**: the weather state is driven by a deterministic RNG seeded from the map generation seed, so every player and the server see exactly the same weather;
- **Gradual world darkening**: the world gradually darkens while raining and recovers when it clears. The darkening steps through `RAIN_SHADE_LEVELS` levels and is read when drawing viewports;
- **Rain-drop overlay**: a full-screen rain overlay that **scales with zoom** to keep the right visual density and carries **randomised jitter** so drops never look like a static texture.

### Options and cheat

- **Difficulty option** `difficulty.rain`: decides at new-game time whether rainy world darkening is enabled;
- **Sandbox cheat** "Weather": a three-state cycle — auto / force rain / force sun.

### Savegames

The weather state (whether it is currently raining, the RNG seed, the start of the current rainy period) and the sandbox weather cheat are **saved with the game**; after loading, the shading jumps straight to the current weather instead of fading in again.

- Chunk: `WTHR`;
- Feature flag: `XSLFI_WEATHER`;
- Code: `src/weather.cpp`, `src/weather.h`, `src/sl/weather_sl.cpp`.

## Related code

- Train ferry and capacities: `src/roadveh_transport.cpp`, `src/cargo_type.h` (`CT_RAILVEHICLES`), `src/table/cargo_const.h`
- Rain visuals: `src/weather.cpp`, `src/blitter/32bpp_anim.cpp`, `src/blitter/40bpp_anim.cpp`, `src/viewport.cpp`
- Weather savegame: `src/sl/weather_sl.cpp`, `src/sl/extended_ver_sl.cpp` (`XSLFI_WEATHER`)
- Settings and cheat: `src/table/settings/difficulty_settings.ini` (`difficulty.rain`), `src/cheat_gui.cpp`, `src/cheat_type.h`
