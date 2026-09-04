---
title: Whole-Game-Aware AI Interface
---

## Current Status (Research Findings)

- NoAI framework: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) — the original AI system is **preserved as-is**, this feature does not touch it;
- Script API system: `src/script/api/script_*.hpp/.cpp` (60+ classes), Squirrel bindings are **auto-generated** by build tools:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` automatically discovers new API classes (generates `ai_*.sq.hpp` / `gs_*.sq.hpp`), **no manual registration needed for new classes**;
  - `.cpp` must be added to the CMake source list; class annotation `@api ai game` controls exposure to AI/GS;
- Company data: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- GS deity mode: `ScriptCompanyMode::IsDeity()`;
- Current limitation: AI can only reliably access its own company data by default; there is no whole-game aggregation API or access toggle.

**Conclusion**: Adding a `ScriptGlobal` API class (auto-registered) + `game.script.allow_global_ai_access` toggle is sufficient to achieve "retain NoAI, add whole-game-aware AI, with access control."

## This Feature Implementation

### 1. `ScriptGlobal` API (`src/script/api/script_global.hpp/.cpp`)

Exposed to AI and GS (`@api ai game`) static methods:

| Method | Return | Data Source |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| setting toggle` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` count by owner |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- Company ID reuses `ScriptCompany::CompanyID` (COMPANY_SELF resolves to the current company);
- **Access control**: All methods check `IsGlobalAccessAllowed()` first, returning -1/nullopt/false if not satisfied (GS always available, AI controlled by toggle);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` enumerates all companies.

### 2. Access Toggle (Setting)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, default false, `SC_EXPERT`);
- Struct field `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- String `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. Example AI: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- Demonstrates: reads all company finances/vehicles/stations/ratings and map information, logs them; explains access control (prompts when toggle is off);
- This AI serves as a reference implementation for "whole-game perception + access control"; subsequent AI control logic can be extended within its `Start()`.

### Involved Files

- New `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (add .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- New `bin/ai/GlobalAI/info.nut`, `main.nut`

## Verification Points

1. The new AI is visible in the AI configuration screen (scans `ai/GlobalAI`);
2. When `game.script.allow_global_ai_access` is off, the AI logs no permission; when on, it outputs complete data for all companies;
3. GameScript can access `GSGlobal` without needing the toggle;
4. Build-time auto-generation of `ai_global.sq.hpp` / `gs_global.sq.hpp` completes without errors (depends on `file(GLOB)`, requires reconfiguring CMake).

## Extension Directions

- Add more aggregated data such as economy/loans/infrastructure (`GetCompanyInfrastructure`);
- Add event subscriptions (company bankruptcy/acquisition/new company creation);
- Make AI decision logic configurable via parameters (`GetSettings()`).