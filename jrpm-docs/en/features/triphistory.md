---
title: Vehicle Trip History
---

# Vehicle Trip History

Originated from **embeddedt/OpenTTD-modded** (Batch 1 port), adapted to jrpm's new string and date APIs.

## Feature

Every vehicle remembers data from its last **10 trips**, viewable via the new **History** button in the vehicle details window:

| Column | Meaning |
|---|---|
| Received | Date when cargo was received for this trip |
| Profit | Profit for this trip (negative values shown in yellow) |
| % Change | Profit change percentage compared to the previous trip (green + / red -) |
| TBT | Time between trips (days) |
| Change | Time change compared to the previous trip (days) |
| Occupancy | Average load/passenger rate for this trip (%) |

The bottom of the window also shows summary statistics:

- **Total income for the last N trips**: Total profit for N trips + average daily profit per trip
- **Average trip length**: Average trip interval (days)
- **Improvement over last N trips**: Overall profit change percentage

## Recording Mechanism

- **AddValue**: When a vehicle completes a transport run (`CargoPayment` destructor), records profit, date, occupancy, and station-to-station distance
- **NewRound**: When a vehicle reaches the first destination according to its timetable, starts a new trip
- **Occupancy**: Captured when the vehicle leaves a station (reuses jrpm's existing `trip_occupancy` mechanism)

## Implementation Notes

- Data is **NOSAVE** (runtime only, not written to saves), so no save version bump is needed, and old saves are fully compatible
- New window class `WindowClass::VehicleTripHistory`, window ID is the vehicle ID
- The vehicle details window (train/non-train) title bar gets a new History button
- History windows are automatically closed when the vehicle is deleted

## Involved Files

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (new)
- `src/vehicle_base.h` (Vehicle adds `trip_history` field)
- `src/economy.cpp` (CargoPayment destructor records trip)
- `src/timetable_cmd.cpp` (reaching first station starts a new trip)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (History button)
- `src/lang/english.txt` (STR_TRIP_HISTORY_* strings)