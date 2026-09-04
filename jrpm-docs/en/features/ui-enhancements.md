---
title: Multiplayer UI Enhancements (Location Bookmarks / Cargo Details / Spectate)
---

# Multiplayer UI Enhancements

Batch 2 port from **citymania-org/cmclient**, implemented as **console commands** (not relying on cmclient's hotkey/toolbar infrastructure, stable and scriptable).

## Location Bookmarks (Viewport Locations)

Save/restore the main viewport position and zoom (9 slots), useful for quickly navigating between your factories, stations, and opponent areas during multiplayer.

```
savelocation <1-9>    # Save current viewport position + zoom
gotolocation <1-9>    # Jump to a saved location
```

## Company Cargo Details

A statistics window listing **delivered cargo volume + income** per company, togglable between **Total / Last Month** periods.

```
company_cargo <company_id>    # Open cargo details window
```

- The window lists volume and income for each standard cargo, plus a bottom total (identical to cmclient)
- Click the "Cargo" header to toggle between Total and Last Month
- Per-cargo income tracked via `CompanyEconomyEntry::cargo_income` (`XSLFI_COMPANY_CARGO_INCOME` save extension), lossless through save/load

## Spectate Helper (Watch)

Spectators can quickly locate a company's construction area (jumps to the coordinates of that company's last build action).

```
watch <company_id>    # Jump viewport to the company's location
```

## Skipped Items and Rationale

| cmclient Feature | jrpm Handling |
|---|---|
| Player list overlay (cm_client_list_gui) | jgrpp **already has** Online Players window (`NetworkClientList`), no need to reimplement |
| Ground detail tooltips (cm_tooltips) | jrpm's LandInfoWindow **already covers** house/industry/station detail display |

## Involved Files

- `src/jrpm_locations.cpp/.h` (new: location bookmarks + company_cargo + watch console commands)
- `src/jrpm_cargo_table.cpp/.h` (new: cargo details window)
- `src/window_type.h` (new `WindowClass::CompanyCargos`)
- `src/console_cmds.cpp` (command registration)
- `src/lang/english.txt` (STR_JRPM_CARGOS_* strings)