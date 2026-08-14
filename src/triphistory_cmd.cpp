/** @file triphistory_cmd.cpp
 * Implementation of the vehicle trip history logic.
 */

#include "stdafx.h"
#include "triphistory.h"

void TripHistory::AddValue(Money mvalue, EconTime::Date dvalue, int8_t occupancy, uint32_t distance)
{
	this->occupancy_sum += distance * static_cast<uint32_t>(occupancy);
	this->trip_length += distance;
	if (dvalue > EconTime::Date{0}) {
		t[0].profit += mvalue;
		t[0].date = dvalue;
		t[0].occupancy = this->trip_length > 0 ? static_cast<int8_t>(this->occupancy_sum / this->trip_length) : 0;
	}
}

void TripHistory::NewRound()
{
	/* Move the entries down. */
	for (int i = TRIP_LENGTH - 1; i > 0; i--) {
		this->t[i] = this->t[i - 1];
	}

	this->t[0].profit = 0;
	this->t[0].date = this->t[1].date;

	this->occupancy_sum = 0;
	this->trip_length = 0;
}

size_t TripHistory::UpdateCalculated()
{
	this->total_profit = 0;
	this->total_change = 0;
	this->avg_daylength = 0;
	this->profit_per_day = 0;
	uint i = 0;

	while (i < TRIP_LENGTH && t[i].date > EconTime::Date{0}) {
		if (i > 0) {
			t[i - 1].profit_change = this->FindPercentChange(t[i - 1].profit, t[i].profit);
			t[i - 1].TBT = (t[i - 1].date - t[i].date).base();

			if (i > 1) t[i - 2].TBT_change = t[i - 2].TBT - t[i - 1].TBT;

			/* Omit the first -100% row. */
			if (i > 1 || t[0].profit_change != -100) this->total_change += t[i - 1].profit_change;
			this->avg_daylength += t[i - 1].TBT;
		}

		this->total_profit += t[i].profit;
		i++;
	}

	if (i == 0) return 0;

	this->avg_daylength /= --i + 1;

	if (t[0].date != t[i].date) {
		this->profit_per_day = this->total_profit / (t[0].date - t[i].date).base();
	}

	return i;
}

int32_t TripHistory::FindPercentChange(Money v1, Money v2)
{
	if (v1 > v2) {
		Money temp = v1 - v2;
		return TripHistoryRound(static_cast<float>(temp) * 100.0f / static_cast<float>(v1));
	}

	if (v2 > v1) {
		Money temp = v1 - v2;
		return TripHistoryRound(static_cast<float>(temp) * 100.0f / static_cast<float>(v2));
	}

	return 0;
}
