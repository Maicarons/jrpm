/** @file jrpm_locations.cpp
 * Viewport location bookmarks: save and restore up to 9 viewport
 * positions (position + zoom) of the main window.
 *
 * Adapted from citymania-org/cmclient (cm_locations) as console commands.
 */

#include "stdafx.h"
#include "company_base.h"
#include "company_func.h"
#include "console_func.h"
#include "console_internal.h"
#include "core/string_consumer.hpp"
#include "zoom_func.h"
#include "jrpm_cargo_table.h"
#include "viewport_func.h"
#include "window_func.h"
#include "window_gui.h"
#include "window_type.h"
#include "zoom_type.h"

#include "safeguards.h"

static constexpr uint NUM_LOCATIONS = 9;

struct ViewportLocation {
	ZoomLevel zoom = ZoomLevel::Normal;
	int32_t scrollpos_x = 0;
	int32_t scrollpos_y = 0;
};

static ViewportLocation _locations[NUM_LOCATIONS];

static bool ConSaveLocation(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Save the current viewport position to a slot. Usage: 'savelocation <slot 1-9>'.");
		return true;
	}

	auto result = ParseInteger(argv[0], 0);
	if (!result.has_value() || *result == 0 || *result > NUM_LOCATIONS) {
		IConsolePrint(CC_ERROR, "Invalid slot. Please use a value between 1 and 9.");
		return false;
	}
	uint slot = static_cast<uint>(*result) - 1;

	Window *w = FindWindowById(WindowClass::MainWindow, 0);
	if (w == nullptr || w->viewport == nullptr) return true;

	ViewportData *vp = w->viewport;
	_locations[slot].zoom = vp->zoom;
	_locations[slot].scrollpos_x = vp->scrollpos_x;
	_locations[slot].scrollpos_y = vp->scrollpos_y;

	IConsolePrint(CC_DEFAULT, "Viewport location saved to slot {}.", *result);
	return true;
}

static bool ConGotoLocation(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Jump to a saved viewport position. Usage: 'gotolocation <slot 1-9>'.");
		return true;
	}

	auto result = ParseInteger(argv[0], 0);
	if (!result.has_value() || *result == 0 || *result > NUM_LOCATIONS) {
		IConsolePrint(CC_ERROR, "Invalid slot. Please use a value between 1 and 9.");
		return false;
	}
	uint slot = static_cast<uint>(*result) - 1;

	const ViewportLocation &loc = _locations[slot];
	if (loc.scrollpos_x == 0 && loc.scrollpos_y == 0) {
		IConsolePrint(CC_ERROR, "Slot {} is empty.", *result);
		return false;
	}

	Window *w = FindWindowById(WindowClass::MainWindow, 0);
	if (w == nullptr || w->viewport == nullptr) return true;

	ViewportData *vp = w->viewport;
	vp->zoom = loc.zoom;
	vp->follow_vehicle = VehicleID::Invalid();
	vp->dest_scrollpos_x = loc.scrollpos_x;
	vp->dest_scrollpos_y = loc.scrollpos_y;
	vp->virtual_width = ScaleByZoom(vp->width, vp->zoom);
	vp->virtual_height = ScaleByZoom(vp->height, vp->zoom);
	w->SetDirty();

	IConsolePrint(CC_DEFAULT, "Jumped to viewport location {}.", *result);
	return true;
}

static bool ConCompanyCargo(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Show the cargo details of a company. Usage: 'company_cargo <company_id>'.");
		return true;
	}

	auto result = ParseInteger(argv[0], 0);
	if (!result.has_value() || *result >= MAX_COMPANIES) {
		IConsolePrint(CC_ERROR, "Invalid company id.");
		return false;
	}
	ShowCompanyCargos(static_cast<CompanyID>(*result));
	return true;
}

static bool ConWatchCompany(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Center the viewport on a company and watch it. Usage: 'watch <company_id>' or 'watch' for the company list window.");
		return true;
	}

	auto result = ParseInteger(argv[0], 0);
	if (!result.has_value() || *result >= MAX_COMPANIES) {
		IConsolePrint(CC_ERROR, "Invalid company id.");
		return false;
	}

	CompanyID cid = static_cast<CompanyID>(*result);
	const Company *c = Company::GetIfValid(cid);
	if (c == nullptr) {
		IConsolePrint(CC_ERROR, "Company {} does not exist.", *result);
		return false;
	}

	ScrollMainWindowToTile(c->last_build_coordinate, true);
	IConsolePrint(CC_DEFAULT, "Watching company {}.", *result);
	return true;
}

void RegisterJRPMConsoleCommands()
{
	IConsole::CmdRegister("savelocation", ConSaveLocation);
	IConsole::CmdRegister("gotolocation", ConGotoLocation);
	IConsole::CmdRegister("company_cargo", ConCompanyCargo);
	IConsole::CmdRegister("watch", ConWatchCompany);
}
