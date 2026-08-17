/** @file triphistory_gui.cpp
 * Window showing the trip history of a vehicle (last 10 trips).
 *
 * Ported from embeddedt/OpenTTD-modded to jrpm (new-style string API).
 */

#include "stdafx.h"
#include "triphistory.h"
#include "strings_func.h"
#include "error.h"
#include "window_func.h"
#include "gfx_func.h"
#include "date_func.h"
#include "vehicle_base.h"
#include "vehicle_gui.h"
#include "widgets/vehicle_widget.h"
#include "table/strings.h"

/* Names of the widgets. Keep them in the same order as in the widget array. */
enum VehicleTripHistoryWidgets : WidgetID {
	VTH_CAPTION,
	VTH_LABEL_RECEIVED,
	VTH_LABEL_PROFIT,
	VTH_LABEL_PERCHANGE,
	VTH_LABEL_TBT,
	VTH_LABEL_DAYCHANGE,
	VTH_LABEL_OCCUPANCY,
	VTH_MATRIX_RECEIVED,
	VTH_MATRIX_PROFIT,
	VTH_MATRIX_PERCHANGE,
	VTH_MATRIX_TBT,
	VTH_MATRIX_DAYCHANGE,
	VTH_MATRIX_OCCUPANCY,
	VTH_SUMMARY,
};

static constexpr std::initializer_list<NWidgetPart> _vehicle_trip_history_widgets = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, Colours::Grey),
		NWidget(WWT_CAPTION, Colours::Grey, VTH_CAPTION), SetStringTip(STR_TRIP_HISTORY_CAPTION, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, Colours::Grey),
		NWidget(WWT_STICKYBOX, Colours::Grey),
	EndContainer(),
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_RECEIVED), SetMinimalSize(110, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_RECEIVED_LABEL, STR_TRIP_HISTORY_RECEIVED_LABEL_TIP),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_PROFIT), SetMinimalSize(110, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_PROFIT_LABEL, STR_TRIP_HISTORY_PROFIT_LABEL_TIP),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_PERCHANGE), SetMinimalSize(50, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_PERCHANGE_LABEL, STR_TRIP_HISTORY_PERCHANGE_LABEL_TIP),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_TBT), SetMinimalSize(70, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_TBT_LABEL, STR_TRIP_HISTORY_TBT_LABEL_TIP),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_DAYCHANGE), SetMinimalSize(50, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_DAYCHANGE_LABEL, STR_TRIP_HISTORY_DAYCHANGE_LABEL_TIP),
		NWidget(WWT_TEXTBTN, Colours::Grey, VTH_LABEL_OCCUPANCY), SetMinimalSize(65, 0), SetMinimalTextLines(1, 2), SetResize(1, 0), SetFill(1, 0),
			SetStringTip(STR_TRIP_HISTORY_OCCUPANCY_LABEL, STR_TRIP_HISTORY_OCCUPANCY_LABEL_TIP),
	EndContainer(),
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_RECEIVED), SetMinimalSize(110, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_PROFIT), SetMinimalSize(110, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_PERCHANGE), SetMinimalSize(50, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_TBT), SetMinimalSize(70, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_DAYCHANGE), SetMinimalSize(50, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
		NWidget(WWT_MATRIX, Colours::Grey, VTH_MATRIX_OCCUPANCY), SetMinimalSize(65, 0), SetMatrixDataTip(1, 10), SetResize(1, 1), SetFill(1, 0),
	EndContainer(),
	NWidget(WWT_PANEL, Colours::Grey, VTH_SUMMARY), SetMinimalTextLines(3, 2), SetResize(1, 0), SetFill(1, 0), EndContainer(),
};

struct VehicleTripHistoryWindow : Window {
private:
	uint8_t valid_rows = 0; ///< Number of rows in the trip history.

public:
	VehicleTripHistoryWindow(WindowDesc &desc, WindowNumber window_number) : Window(desc)
	{
		const Vehicle *v = Vehicle::Get(window_number);
		this->CreateNestedTree();
		this->FinishInitNested(window_number);
		this->owner = v->owner;
		this->OnInvalidateData();
	}

	void OnInvalidateData([[maybe_unused]] int data = 0, [[maybe_unused]] bool gui_scope = true) override
	{
		Vehicle *v = Vehicle::Get(this->window_number);
		this->valid_rows = static_cast<uint8_t>(v->trip_history.UpdateCalculated());
		this->SetDirty();
	}

	std::string GetWidgetString(WidgetID widget, StringID stringid) const override
	{
		if (widget == VTH_CAPTION) return GetString(STR_TRIP_HISTORY_CAPTION, Vehicle::Get(this->window_number)->index);
		return this->Window::GetWidgetString(widget, stringid);
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, const Dimension &padding, Dimension &fill, Dimension &resize) override
	{
		switch (widget) {
			case VTH_SUMMARY: {
				std::string str = GetString(STR_TRIP_HISTORY_TOTALINCOME, UINT64_MAX >> 2, 100, 100);
				Dimension text_dim = GetStringBoundingBox(str);
				size.width = text_dim.width + WidgetDimensions::scaled.framerect.Horizontal();
				break;
			}

			case VTH_LABEL_RECEIVED:
			case VTH_MATRIX_RECEIVED: {
				Dimension text_dim = GetStringBoundingBox(GetString(STR_TRIP_HISTORY_DATE, CalTime::CurDate()));
				size.width = text_dim.width + WidgetDimensions::scaled.matrix.Horizontal();
				break;
			}
			case VTH_MATRIX_PROFIT:
			case VTH_MATRIX_PERCHANGE:
			case VTH_MATRIX_TBT:
			case VTH_MATRIX_DAYCHANGE:
			case VTH_MATRIX_OCCUPANCY:
				resize.height = GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.matrix.Vertical();
				size.height = 10 * resize.height;
				break;
		}
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		const Vehicle *v = Vehicle::Get(this->window_number);
		int y = WidgetDimensions::scaled.framerect.top;
		int line_height = GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.matrix.Vertical();

		switch (widget) {
			case VTH_MATRIX_RECEIVED:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i].date > EconTime::Date{0}) {
						DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
								GetString(STR_TRIP_HISTORY_DATE, v->trip_history.t[i].date), TextColour::Black, SA_RIGHT);
					}
				}
				break;
			case VTH_MATRIX_PROFIT:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i].date > EconTime::Date{0}) {
						if (v->trip_history.t[i].profit > 0) {
							DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
									GetString(STR_TRIP_HISTORY_PROFIT, v->trip_history.t[i].profit), TextColour::Black, SA_RIGHT);
						} else {
							DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
									GetString(STR_TRIP_HISTORY_VIRTUAL_PROFIT, -v->trip_history.t[i].profit), TextColour::Black, SA_RIGHT);
						}
					}
				}
				break;
			case VTH_MATRIX_PERCHANGE:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i + 1].date > EconTime::Date{0}) {
						DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
								GetString(v->trip_history.t[i].profit_change >= 0 ? STR_TRIP_HISTORY_PROFITCHANGEPOS : STR_TRIP_HISTORY_PROFITCHANGENEG, v->trip_history.t[i].profit_change),
								TextColour::Black, SA_RIGHT);
					}
				}
				break;
			case VTH_MATRIX_TBT:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i].date > EconTime::Date{0}) {
						DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
								GetString(STR_TRIP_HISTORY_TBT, v->trip_history.t[i].TBT), TextColour::Black, SA_RIGHT);
					}
				}
				break;
			case VTH_MATRIX_DAYCHANGE:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i + 1].date > EconTime::Date{0}) {
						DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
								GetString(v->trip_history.t[i].TBT_change > 0 ? STR_TRIP_HISTORY_TBTCHANGEPOS : STR_TRIP_HISTORY_TBTCHANGENEG, v->trip_history.t[i].TBT_change),
								TextColour::Black, SA_RIGHT);
					}
				}
				break;
			case VTH_MATRIX_OCCUPANCY:
				for (int i = 0; i <= this->valid_rows; i++, y += line_height) {
					if (v->trip_history.t[i + 1].date > EconTime::Date{0} && v->trip_history.t[i].occupancy >= 0) {
						DrawString(r.left + WidgetDimensions::scaled.matrix.left, r.right - WidgetDimensions::scaled.matrix.right, r.top + y,
								GetString(STR_TRIP_HISTORY_OCCUPANCY, v->trip_history.t[i].occupancy), TextColour::Black, SA_RIGHT);
					}
				}
				break;
			case VTH_SUMMARY:
				DrawString(r.left + WidgetDimensions::scaled.framerect.left, r.right - WidgetDimensions::scaled.framerect.right, r.top + WidgetDimensions::scaled.framerect.top,
						GetString(STR_TRIP_HISTORY_TOTALINCOME, this->valid_rows + 1, v->trip_history.total_profit, v->trip_history.profit_per_day), TextColour::Black);
				DrawString(r.left + WidgetDimensions::scaled.framerect.left, r.right - WidgetDimensions::scaled.framerect.right, r.top + GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.framerect.top,
						GetString(STR_TRIP_HISTORY_DAYAVERAGE, v->trip_history.avg_daylength), TextColour::Black);
				DrawString(r.left + WidgetDimensions::scaled.framerect.left, r.right - WidgetDimensions::scaled.framerect.right, r.top + 2 * GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.framerect.top,
						GetString(STR_TRIP_HISTORY_DAYAVERAGE_IMPROVEMENT, this->valid_rows + 1, v->trip_history.total_change), TextColour::Black);
				break;
		}
	}
};

static WindowDesc _vehicle_trip_history_desc(__FILE__, __LINE__,
	WindowPosition::Automatic, "trip_history", 450, 191,
	WindowClass::VehicleTripHistory, WindowClass::VehicleDetails,
	{},
	_vehicle_trip_history_widgets
);

void ShowTripHistoryWindow(const Vehicle *v)
{
	if (!_settings_game.jrpm_features.enable_trip_history) {
		ShowErrorMessage(GetEncodedString(STR_JRPM_FEATURE_DISABLED, STR_CONFIG_SETTING_JRPM_ENABLE_TRIP_HISTORY), {}, WarningLevel::Error);
		return;
	}
	if (!BringWindowToFrontById(WindowClass::VehicleTripHistory, v->index)) {
		AllocateWindowDescFront<VehicleTripHistoryWindow>(_vehicle_trip_history_desc, v->index);
	}
}
