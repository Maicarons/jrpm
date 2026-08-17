/** @file jrpm_watch_gui.cpp
 * Watch another company's building activity (cmclient "Watch company's actions" port).
 *
 * Company-only mode: shows a viewport that follows the watched company's
 * last build coordinate, plus a row of company buttons to pick which company
 * to watch. Activity of each company is briefly highlighted after it builds.
 */

#include "stdafx.h"

#include "jrpm_watch_gui.h"

#include "company_base.h"
#include "company_gui.h"
#include "company_type.h"
#include "core/geometry_func.hpp"
#include "gfx_func.h"
#include "landscape.h"
#include "map_func.h"
#include "strings_func.h"
#include "viewport_func.h"
#include "window_func.h"
#include "window_gui.h"
#include "window_type.h"
#include "zoom_func.h"

#include "table/sprites.h"
#include "table/strings.h"

#include "safeguards.h"

static const uint MAX_ACTIVITY = 30; ///< Activity counter ceiling for highlighting.

/** Widgets of the #WatchCompanyWindow class. */
enum WatchCompanyWidgets : WidgetID {
	WID_WC_CAPTION,   ///< Caption.
	WID_WC_LOCATION,  ///< Center the main view on this watch window's location.
	WID_WC_WATCH,     ///< Viewport showing where the watched company is building.
	WID_WC_COMPANY_BEGIN, ///< First company button (one per company).
	WID_WC_COMPANY_LAST = static_cast<WatchCompanyWidgets>(static_cast<int>(WID_WC_COMPANY_BEGIN) + MAX_COMPANIES - 1),
};

/** Build the row of company buttons shown at the bottom of the window. */
static std::unique_ptr<NWidgetBase> MakeCompanyButtons()
{
	auto hor = std::make_unique<NWidgetHorizontal>();

	Dimension company_sprite_size = GetSpriteSize(SPR_COMPANY_ICON);
	company_sprite_size.width  += WidgetDimensions::scaled.matrix.Horizontal();
	company_sprite_size.height += WidgetDimensions::scaled.matrix.Vertical() + 1;

	for (int cid = 0; cid < MAX_COMPANIES; cid++) {
		auto panel = std::make_unique<NWidgetBackground>(WWT_PANEL, Colours::Grey, WID_WC_COMPANY_BEGIN + cid);
		panel->SetMinimalSizeAbsolute(company_sprite_size.width, company_sprite_size.height);
		panel->SetResize(1, 0);
		panel->SetFill(1, 1);
		panel->SetToolTip(STR_JRPM_WATCH_CLICK_TO_WATCH);
		hor->Add(std::move(panel));
	}

	return hor;
}

static const NWidgetPart _nested_watch_company_widgets[] = {
	/* Title bar with close box, caption, location, shade, defsize and sticky boxes. */
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, Colours::Grey),
		NWidget(WWT_CAPTION, Colours::Grey, WID_WC_CAPTION),
		NWidget(WWT_PUSHIMGBTN, Colours::Grey, WID_WC_LOCATION), SetMinimalSize(12, 14), SetStringTip(SPR_GOTO_LOCATION, STR_JRPM_WATCH_LOCATION_TOOLTIP),
		NWidget(WWT_SHADEBOX, Colours::Grey),
		NWidget(WWT_DEFSIZEBOX, Colours::Grey),
		NWidget(WWT_STICKYBOX, Colours::Grey),
	EndContainer(),
	/* Watch viewport. */
	NWidget(WWT_PANEL, Colours::Grey),
		NWidget(NWID_VIEWPORT, Colours::Invalid, WID_WC_WATCH), SetPadding(2, 2, 2, 2), SetResize(1, 1), SetFill(1, 1),
	EndContainer(),
	/* Bottom row with company buttons and resize box. */
	NWidget(NWID_HORIZONTAL),
		NWidgetFunction(MakeCompanyButtons),
		NWidget(WWT_RESIZEBOX, Colours::Grey),
	EndContainer(),
};

static WindowDesc _watch_company_desc(
	__FILE__, __LINE__,
	WindowPosition::Automatic, "jrpm_watch_company", 0, 0,
	WindowClass::WatchCompany, WindowClass::None,
	WindowDefaultFlag::NoFocus,
	_nested_watch_company_widgets
);

/** Window that follows a company's building activity. */
struct WatchCompanyWindow : Window {
	CompanyID watched_company = CompanyID::Invalid();       ///< Currently watched company.
	std::array<uint8_t, MAX_COMPANIES> company_activity{};   ///< Activity counter per company.

	WatchCompanyWindow(WindowDesc &desc, int window_number, CompanyID company_to_watch = CompanyID::Invalid()) : Window(desc)
	{
		this->watched_company = company_to_watch;
		this->InitNested(window_number);
		this->owner = static_cast<Owner>(this->watched_company);

		/* Initialise the viewport. */
		NWidgetViewport *nvp = this->GetWidget<NWidgetViewport>(WID_WC_WATCH);
		nvp->InitializeViewport(this, 0, ScaleZoomGUI(ZoomLevel::Normal));

		/* Center on the same place as the main window. */
		const Window *w = GetMainWindow();
		Point pt;
		pt.x = w->viewport->scrollpos_x + w->viewport->virtual_width / 2;
		pt.y = w->viewport->scrollpos_y + w->viewport->virtual_height / 2;
		this->viewport->scrollpos_x = pt.x - this->viewport->virtual_width / 2;
		this->viewport->scrollpos_y = pt.y - this->viewport->virtual_height / 2;
		this->viewport->dest_scrollpos_x = this->viewport->scrollpos_x;
		this->viewport->dest_scrollpos_y = this->viewport->scrollpos_y;

		if (auto c = Company::GetIfValid(this->watched_company); c != nullptr) {
			this->ScrollToTile(c->last_build_coordinate);
		}
		this->InvalidateData();
	}

	std::string GetWidgetString(WidgetID widget, StringID stringid) const override
	{
		if (widget == WID_WC_CAPTION) {
			if (Company::IsValidID(this->watched_company)) {
				return GetString(STR_JRPM_WATCH_CAPTION_COMPANY, this->watched_company);
			}
			return GetString(STR_JRPM_WATCH_CAPTION_NONE);
		}
		return this->Window::GetWidgetString(widget, stringid);
	}

	void OnPaint() override
	{
		this->DrawWidgets();
	}

	void DrawWidget(const Rect &r, int widget) const override
	{
		if (!IsInsideMM(widget, static_cast<int>(WID_WC_COMPANY_BEGIN), static_cast<int>(WID_WC_COMPANY_LAST) + 1)) return;
		if (this->IsWidgetDisabled(widget)) return;

		CompanyID cid = static_cast<CompanyID>(widget - static_cast<int>(WID_WC_COMPANY_BEGIN));
		if (!Company::IsValidID(cid)) return;

		/* Activity blot: highlight recently active companies in red. */
		if (this->company_activity[cid.base()] > 0) {
			Dimension blot = GetSpriteSize(SPR_BLOT);
			DrawSprite(SPR_BLOT, PALETTE_TO_RED, r.left + 1, r.top + 1);
			(void)blot; // size only queried to keep layout in sync with the icon
		}

		int offset = (cid == this->watched_company) ? 1 : 0;
		Dimension sprite_size = GetSpriteSize(SPR_COMPANY_ICON);
		DrawCompanyIcon(cid, (r.left + r.right - sprite_size.width) / 2 + offset, (r.top + r.bottom - sprite_size.height) / 2 + offset);
	}

	void OnResize() override
	{
		if (this->viewport != nullptr) {
			NWidgetViewport *nvp = this->GetWidget<NWidgetViewport>(WID_WC_WATCH);
			nvp->UpdateViewportCoordinates(this);
		}
	}

	void OnScroll(Point delta) override
	{
		const Viewport *vp = IsPtInWindowViewport(this, _cursor.pos.x, _cursor.pos.y);
		if (vp == nullptr) return;

		this->viewport->scrollpos_x += ScaleByZoom(delta.x, vp->zoom);
		this->viewport->scrollpos_y += ScaleByZoom(delta.y, vp->zoom);
		this->viewport->dest_scrollpos_x = this->viewport->scrollpos_x;
		this->viewport->dest_scrollpos_y = this->viewport->scrollpos_y;
	}

	void OnMouseWheel(int wheel, WidgetID widget) override
	{
		if (widget != WID_WC_WATCH) return;
		ZoomInOrOutToCursorWindow(wheel < 0, this);
	}

	void OnClick([[maybe_unused]] Point pt, WidgetID widget, [[maybe_unused]] int click_count) override
	{
		if (IsInsideMM(widget, static_cast<int>(WID_WC_COMPANY_BEGIN), static_cast<int>(WID_WC_COMPANY_LAST) + 1)) {
			if (!this->IsWidgetDisabled(widget)) {
				CompanyID cid = static_cast<CompanyID>(widget - static_cast<int>(WID_WC_COMPANY_BEGIN));
				if (this->watched_company == cid) {
					this->watched_company = CompanyID::Invalid();
				} else {
					this->watched_company = cid;
					if (auto c = Company::GetIfValid(cid); c != nullptr) {
						this->ScrollToTile(c->last_build_coordinate);
					}
				}
				this->owner = static_cast<Owner>(this->watched_company);
				this->SetDirty();
			}
			return;
		}

		if (widget == WID_WC_LOCATION) {
			int x = this->viewport->scrollpos_x + this->viewport->virtual_width / 2;
			int y = this->viewport->scrollpos_y + this->viewport->virtual_height / 2;
			Point p = InverseRemapCoords(x, y);
			ScrollMainWindowToTile(TileXY(p.x, p.y));
		}
	}

	void OnInvalidateData(int, bool) override
	{
		for (int c = 0; c < MAX_COMPANIES; c++) {
			this->SetWidgetDisabledState(WID_WC_COMPANY_BEGIN + c, !Company::IsValidID(static_cast<CompanyID>(c)));
		}

		if (this->watched_company != CompanyID::Invalid() && !Company::IsValidID(this->watched_company)) {
			this->watched_company = CompanyID::Invalid();
		}
		if (Company::IsValidID(this->watched_company)) {
			this->LowerWidget(WID_WC_COMPANY_BEGIN + this->watched_company.base());
		}
	}

	/** Scroll the watch viewport to a tile (only if non-zero). */
	void ScrollToTile(TileIndex tile)
	{
		if (tile != 0) {
			ScrollWindowTo(TileX(tile) * TILE_SIZE + TILE_SIZE / 2, TileY(tile) * TILE_SIZE + TILE_SIZE / 2, 0, this);
		}
	}

	/** Called by UpdateWatching when a company builds something. */
	void OnDoCommand(CompanyID company, TileIndex tile)
	{
		if (this->watched_company == company) {
			this->ScrollToTile(tile);
		}
		if (tile != 0) {
			this->company_activity[company.base()] = MAX_ACTIVITY;
			this->SetDirty();
		}
	}

	void OnRealtimeTick([[maybe_unused]] uint delta_ms) override
	{
		bool set_dirty = false;
		for (int c = 0; c < MAX_COMPANIES; c++) {
			if (this->company_activity[c] > 0) {
				this->company_activity[c]--;
				if (this->company_activity[c] == 0) set_dirty = true;
			}
		}
		if (set_dirty) this->SetDirty();
	}
};

void ShowWatchWindow(CompanyID company_to_watch)
{
	int i = 0;
	while (FindWindowById(WindowClass::WatchCompany, i) != nullptr) i++;
	new WatchCompanyWindow(_watch_company_desc, i, company_to_watch);
}

void UpdateWatching(CompanyID company, TileIndex tile)
{
	WatchCompanyWindow *wc;
	for (int w = 0; ; w++) {
		wc = dynamic_cast<WatchCompanyWindow *>(FindWindowById(WindowClass::WatchCompany, w));
		if (wc != nullptr) wc->OnDoCommand(company, tile);
		else break;
	}
}
