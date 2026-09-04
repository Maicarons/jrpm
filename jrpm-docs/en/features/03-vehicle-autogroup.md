---
title: Vehicle Auto-Grouping
---

## Current Status (Research Findings)

- Group data structure: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- Existing commands (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (rename/set parent group)
  - `CmdAddVehicleGroup` (single vehicle into group)
  - `CmdAddSharedVehicleGroup` (adds **vehicles sharing orders with a given vehicle** to an **existing** group)
  - `CmdCreateGroupFromList` (create group from a vehicle list, supports `VL_SHARED_ORDERS` list type; auto-naming)
- Auto-naming helper: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, by route start/end town `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) — jgrpp already has the "name by route" infrastructure;
- Command registration mechanism: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (macro that simultaneously declares the handler and registers `CommandTraits`);
- Vehicle↔Group: `vehicle_base.h` `Vehicle::group_id`; shared order chain `FirstShared()/NextShared()`; `OrderList *orders`.

**Conclusion**: Missing a single command that "iterates all company vehicles, auto-creates groups by shared orders, and assigns vehicles to them."

## This Feature Implementation

### New Command `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Flow (`src/group_cmd.cpp`):
1. Iterate all primary vehicles of the specified type for the company (`Vehicle::IterateTypeFrontOnly(type)`), skip vehicles not owned by this company, without orders, or already in a custom group;
2. Count each shared order chain; when ≥2 vehicles share the same order list:
   - Use `GenerateAutoNameForVehicleGroup(v)` to generate a group name (e.g., "Town A ↔ Town B");
   - Construct `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, nest a call to `Command<Commands::CreateGroupFromList>::Do(flags, ...)` to create the group and move all shared vehicles into it;
   - After completion, vehicles in that group have a `group_id` different from the default group, so the loop automatically skips them (only one group per order list);
3. `GroupChangeDeferredUpdateScope` defers group statistics updates uniformly.

### Three Entry Points

| Entry Point | Location | Description |
|---|---|---|
| Group window button | `src/group_gui.cpp` new `WID_GL_AUTOGROUP_SHARED` (toolbar near LIVERY), OnClick sends command | Reuses `SPR_GROUP_CREATE_TRAIN + vtype` sprite |
| Console command | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (no argument = execute for all four types) | `IConsole::CmdRegister` |
| Script/Other | Any code can call `Command<Commands::AutoGroupSharedOrders>::Post(...)` | Command goes through network sync, safe for multiplayer servers |

### Involved Files

- `src/command_type.h`: `Commands` enum adds `AutoGroupSharedOrders` (**Note: inserting in the middle shifts subsequent command IDs, causing inconsistency between old and new version multiplayer — this is normal within a fork**)
- `src/group_cmd.h` / `group_cmd.cpp`: command declaration and implementation
- `src/widgets/group_widget.h`: new widget ID
- `src/group_gui.cpp`: toolbar button (NWidget + OnPaint sprite + OnClick)
- `src/console_cmds.cpp`: console command
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Verification Points

1. Two or more vehicles sharing the same order list → clicking the button / `autogroup` creates a "route name" group with all vehicles assigned;
2. Already grouped vehicles are not moved again; different order lists each form their own group;
3. Group vehicle statistics (count/profit) are correctly refreshed (depends on `GroupChangeDeferredUpdateScope`).