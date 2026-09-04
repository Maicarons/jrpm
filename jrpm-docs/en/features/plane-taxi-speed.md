---
title: Adjustable Plane Taxi Speed
---

# Adjustable Plane Taxi Speed

Originated from **embeddedt/OpenTTD-modded** (Batch 1 port).

## Feature

New game setting **`vehicle.plane_taxi_speed` (Plane Taxi Speed)**, independently adjusts the speed limit for aircraft taxiing on the ground:

- Range **1–8**, default **4** (= original taxi speed limit of 50 units)
- Higher values mean faster taxiing (1 → 12.5, 8 → 100)
- Works **independently** from other aircraft speed settings (`plane_speed` global multiplier), no interference

## Setting Location

In-game **Settings → Expert Settings → Vehicles**, or in the configuration file `openttd.cfg`:

```ini
[vehicle]
plane_taxi_speed = 4
```

## Implementation Notes

- Setting has `SettingFlag::NoNetwork` (server-authoritative, decided by the host in multiplayer)
- Uses **XSLF extended feature gating** (`XSLFI_PLANE_TAXI_SPEED`, version 1): when loading old saves, this feature does not exist → the setting stays at default value 4, **does not break any old saves**
- Taxi speed limit applies when the aircraft is in `AS_RUNNING` state (taxiing on ground), taking `min(aircraft speed limit, taxi speed limit)`

## Involved Files

- `src/aircraft_cmd.cpp` (new `SPEED_LIMIT_TAXI` constant + taxi speed limit logic)
- `src/table/settings/game_settings.ini` (setting definition)
- `src/settings_type.h` (`VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (XSLF feature registration)
- `src/lang/english.txt` (STR_CONFIG_SETTING_PLANE_TAXI_SPEED*)