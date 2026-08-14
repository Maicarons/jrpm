/** @file triphistory.h
 * Vehicle trip history: records the profit, date, occupancy and length of
 * the last few trips of every vehicle.
 *
 * Ported from embeddedt/OpenTTD-modded to jrpm (new-style string/date API).
 */

#ifndef TRIPHISTORY_H
#define TRIPHISTORY_H

#include <cstdint>
#include "money_type.h"
#include "date_type.h"

/** Number of trips kept per vehicle. */
static constexpr uint TRIP_LENGTH = 10;

/** Round a float to the nearest integer. */
static inline int TripHistoryRound(float x)
{
	return static_cast<int>(x > 0.0f ? x + 0.5f : x - 0.5f);
}

/** A single recorded trip. */
struct TripHistoryEntry {
	Money profit = 0;            ///< Profit of this trip.
	EconTime::Date date;         ///< Date the cargo of this trip was received (0 = empty slot).
	int32_t profit_change = 0;   ///< Calculated: % profit change vs the previous trip.
	int32_t TBT = 0;             ///< Calculated: time (days) between this and the previous trip.
	int32_t TBT_change = 0;      ///< Calculated: change in trip length vs the previous trip.
	int8_t occupancy = -1;       ///< Calculated: average occupancy (in percent) during the trip.

	TripHistoryEntry() : date(EconTime::Date{0}) {}
};

/** Per-vehicle trip history data. */
struct TripHistory {
	TripHistoryEntry t[TRIP_LENGTH]; ///< The recorded trips, t[0] is the most recent.

	Money total_profit = 0;       ///< Calculated summary.
	int32_t avg_daylength = 0;    ///< Calculated summary.
	int32_t total_change = 0;     ///< Calculated summary.
	Money profit_per_day = 0;     ///< Calculated summary.
	uint32_t occupancy_sum = 0;   ///< Weighted occupancy accumulator of the current trip (runtime).
	uint32_t trip_length = 0;     ///< Distance covered by the current trip (runtime).

	void NewRound();
	void AddValue(Money mvalue, EconTime::Date dvalue, int8_t occupancy, uint32_t distance);

	/** Recalculate the summary values for the GUI. @return number of valid rows. */
	size_t UpdateCalculated();

	int32_t FindPercentChange(Money v1, Money v2);
};

#endif /* TRIPHISTORY_H */
