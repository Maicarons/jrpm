/*
 * Zoning toolbar - cmc-style two-column toggle button list.
 *
 * Replaced the previous JGR dropdown style with the cmclient toggle-button
 * list so it visually and behaviourally matches cmclient.
 */

#include "stdafx.h"
#include "openttd.h"
#include "dropdown_func.h"
#include "widget_type.h"
#include "window_func.h"
#include "gui.h"
#include "viewport_func.h"
#include "sound_func.h"
#include "table/sprites.h"
#include "table/strings.h"
#include "strings_func.h"
#include "gfx_func.h"
#include "core/geometry_func.hpp"
#include "zoning.h"
#include "debug_settings.h"

#include <initializer_list>

/* jrpm's ZoningEvaluationMode skips CHECKNOTHING (ZEM_NOTHING), so the
 * cmc-style button indexes offset by 1 from ZEM_NOTHING. */
static const ZoningEvaluationMode ZONES[] = {
	ZEM_AUTHORITY,
	ZEM_CAN_BUILD,
	ZEM_STA_CATCH,
	ZEM_STA_CATCH_WIN,
	ZEM_BUL_UNSER,
	ZEM_IND_UNSER,
	ZEM_TRACERESTRICT,
	ZEM_2x2_GRID,
	ZEM_3x3_GRID,
	ZEM_ONE_WAY_ROAD,
	ZEM_TOWN_ZONES,
	ZEM_TOWN_GROWTH_TILES,
};

static const StringID ZONE_STRINGS[] = {
	STR_ZONING_AUTHORITY,
	STR_ZONING_CAN_BUILD,
	STR_ZONING_STA_CATCH,
	STR_ZONING_STA_CATCH_OPEN,
	STR_ZONING_BUL_UNSER,
	STR_ZONING_IND_UNSER,
	STR_ZONING_TRACERESTRICT,
	STR_ZONING_2x2_GRID,
	STR_ZONING_3x3_GRID,
	STR_ZONING_ONE_WAY_ROAD,
	STR_ZONING_TOWN_ZONES,
	STR_ZONING_TOWN_GROWTH_TILES,
};

static const int ZONES_COUNT = lengthof(ZONES);

enum ZoningToolbarWidgets : WidgetID {
	ZTW_CAPTION,
	ZTW_OUTER_FIRST,
	ZTW_INNER_FIRST = ZTW_OUTER_FIRST + ZONES_COUNT,
	ZTW_INNER_END = ZTW_INNER_FIRST + ZONES_COUNT,
};

struct ZoningWindow : public Window {
	uint maxwidth = 0;
	uint maxheight = 0;

	ZoningWindow(WindowDesc &desc, int window_number)
			: Window(desc)
	{
		Dimension dim;
		for (int i = 0; i < ZONES_COUNT; i++) {
			dim = GetStringBoundingBox(ZONE_STRINGS[i]);
			this->maxwidth = std::max(this->maxwidth, dim.width);
			this->maxheight = std::max(this->maxheight, dim.height);
		}

		this->InitNested(window_number);
		this->InvalidateData();
		if (_zoning.outer != ZEM_NOTHING) {
			for (int i = 0; i < ZONES_COUNT; i++) {
				if (ZONES[i] == _zoning.outer) {
					this->LowerWidget(ZTW_OUTER_FIRST + i);
					break;
				}
			}
		}
		if (_zoning.inner != ZEM_NOTHING) {
			for (int i = 0; i < ZONES_COUNT; i++) {
				if (ZONES[i] == _zoning.inner) {
					this->LowerWidget(ZTW_INNER_FIRST + i);
					break;
				}
			}
		}
	}

	void OnPaint() override
	{
		this->DrawWidgets();
	}

	void RaiseColumn(bool outer)
	{
		WidgetID start = outer ? ZTW_OUTER_FIRST : ZTW_INNER_FIRST;
		WidgetID end = outer ? ZTW_INNER_FIRST : ZTW_INNER_END;
		for (WidgetID i = start; i < end; i++) {
			if (this->IsWidgetLowered(i)) {
				this->ToggleWidgetLoweredState(i);
				break;
			}
		}
	}

	int FindZoneIndex(ZoningEvaluationMode mode) const
	{
		for (int i = 0; i < ZONES_COUNT; i++) if (ZONES[i] == mode) return i;
		return -1;
	}

	void OnClick(Point /* pt */, WidgetID widget, int /* click_count */) override
	{
		bool outer = true;
		bool deselect = false;
		if (widget >= ZTW_OUTER_FIRST && widget < ZTW_INNER_FIRST) {
			int idx = widget - ZTW_OUTER_FIRST;
			deselect = _zoning.outer == ZONES[idx];
			_zoning.outer = deselect ? ZEM_NOTHING : ZONES[idx];
		} else if (widget >= ZTW_INNER_FIRST && widget < ZTW_INNER_END) {
			outer = false;
			int idx = widget - ZTW_INNER_FIRST;
			deselect = _zoning.inner == ZONES[idx];
			_zoning.inner = deselect ? ZEM_NOTHING : ZONES[idx];
		} else return;

		this->RaiseColumn(outer);
		if (!deselect) this->ToggleWidgetLoweredState(widget);
		this->InvalidateData();
		PostZoningModeChange();
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		StringID strid = STR_EMPTY;
		if (widget >= ZTW_OUTER_FIRST && widget < ZTW_INNER_FIRST) {
			strid = ZONE_STRINGS[widget - ZTW_OUTER_FIRST];
		} else if (widget >= ZTW_INNER_FIRST && widget < ZTW_INNER_END) {
			strid = ZONE_STRINGS[widget - ZTW_INNER_FIRST];
		} else return;

		bool rtl = _current_text_dir == TD_RTL;
		uint8_t clk_dif = this->IsWidgetLowered(widget) ? 1 : 0;
		int x = r.left + WidgetDimensions::scaled.framerect.left;
		int y = r.top;

		DrawString(rtl ? r.left : x + clk_dif + 1, (rtl ? r.right + clk_dif : r.right), y + 1 + clk_dif, strid, TextColour::FromString, SA_LEFT);
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, const Dimension &padding, Dimension & /* fill */, Dimension & /* resize */) override
	{
		if (widget >= ZTW_OUTER_FIRST && widget < ZTW_INNER_END) {
			size.width = this->maxwidth + padding.width + 8;
			size.height = this->maxheight + 2;
		}
	}
};

static std::unique_ptr<NWidgetBase> MakeZoningButtons()
{
	auto hor = std::make_unique<NWidgetHorizontal>(NWidContainerFlag::EqualSize);
	hor->SetPadding(1, 1, 1, 1);

	for (int i = 0; i < 2; i++) {
		auto vert = std::make_unique<NWidgetVertical>();

		WidgetID offset = (i == 0) ? ZTW_OUTER_FIRST : ZTW_INNER_FIRST;
		Colours colour = (i == 0) ? Colours::Orange : Colours::Yellow;

		for (int j = 0; j < ZONES_COUNT; j++) {
			auto leaf = std::make_unique<NWidgetBackground>(WWT_PANEL, colour, offset + j);
			leaf->SetFill(1, 0);
			leaf->SetPadding(0, 0, 0, 0);
			vert->Add(std::move(leaf));
		}
		hor->Add(std::move(vert));
	}
	return hor;
}

static constexpr NWidgetPart _nested_zoning_widgets[] = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, Colours::Grey),
		NWidget(WWT_CAPTION, Colours::Grey, ZTW_CAPTION), SetStringTip(STR_ZONING_TOOLBAR, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, Colours::Grey),
		NWidget(WWT_STICKYBOX, Colours::Grey),
	EndContainer(),
	NWidget(WWT_PANEL, Colours::Grey),
		NWidgetFunction(MakeZoningButtons),
	EndContainer()
};

static WindowDesc _zoning_desc (__FILE__, __LINE__,
	WindowPosition::Automatic, "zoning_gui", 0, 0,
	WindowClass::ZoningToolbar, WindowClass::None,
	{},
	_nested_zoning_widgets
);

void ShowZoningToolbar()
{
	AllocateWindowDescFront<ZoningWindow>(_zoning_desc, 0);
}
