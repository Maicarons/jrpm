/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file construction_cost_tip.cpp Implementation of the construction cost tooltip. */

#include "stdafx.h"

#include "construction_cost_tip.h"

#include "command_func.h"
#include "landscape.h"
#include "rail_cmd.h"
#include "road_cmd.h"
#include "road_type.h"
#include "settings_type.h"
#include "slope_func.h"
#include "strings_func.h"
#include "terraform_cmd.h"
#include "texteff.hpp"
#include "tile_map.h"
#include "tilehighlight_type.h"
#include "town_type.h"
#include "track_type.h"
#include "transport_type.h"
#include "viewport_func.h"
#include "widgets/rail_widget.h"
#include "widgets/road_widget.h"
#include "widgets/terraform_widget.h"

#include "safeguards.h"

/** Current tooltip effect, if any. */
static TextEffectID _cost_tip_effect = INVALID_TE_ID;
/** Tile the tooltip was last shown for (throttling). */
static TileIndex _cost_tip_last_tile = INVALID_TILE;
/** Tool the tooltip was last shown for (throttling). */
static WidgetID _cost_tip_last_tool = INVALID_WIDGET;
/** Window class of the tool that currently owns the tooltip. */
static WindowClass _cost_tip_owner_class = WC_INVALID;
/** Window number of the tool that currently owns the tooltip. */
static WindowNumber _cost_tip_owner_number = 0;

/**
 * Hide and remove the construction cost tooltip, if any.
 */
void HideConstructionCostTip()
{
	if (_cost_tip_effect != INVALID_TE_ID) {
		RemoveTextEffect(_cost_tip_effect);
		_cost_tip_effect = INVALID_TE_ID;
	}
	_cost_tip_last_tile = INVALID_TILE;
	_cost_tip_last_tool = INVALID_WIDGET;
	_cost_tip_owner_class = WC_INVALID;
	_cost_tip_owner_number = 0;
}

/**
 * Estimate the cost of building the currently selected tool on the given tile.
 * @param ctx  Context of the active build tool.
 * @param tile Tile to build on.
 * @return The estimated cost, or an error cost when the tool is not supported
 *         by the estimator or the build is not possible on the tile.
 */
static CommandCost EstimateBuildCost(const ConstructionCostTipContext &ctx, TileIndex tile)
{
	if (ctx.window_class == WindowClass::BuildToolbar) {
		if (ctx.window_number == TRANSPORT_RAIL) {
			/* Rail toolbar: support the single track pieces, auto-rail and depot. */
			Track track = INVALID_TRACK;
			switch (ctx.selected_tool) {
				case WID_RAT_BUILD_NS: track = (Track)HT_DIR_VL; break;
				case WID_RAT_BUILD_X:  track = (Track)HT_DIR_Y;  break;
				case WID_RAT_BUILD_EW: track = (Track)HT_DIR_HL; break;
				case WID_RAT_BUILD_Y:  track = (Track)HT_DIR_X;  break;
				case WID_RAT_AUTORAIL: track = (Track)TRACK_X; break; /* best-effort estimate */
				default: break;
			}
			if (track == INVALID_TRACK) return CMD_ERROR;

			BuildRailTrackFlags rflags = _settings_client.gui.auto_remove_signals ? BuildRailTrackFlags::AutoRemoveSignals : BuildRailTrackFlags::None;
			return Command<Commands::BuildRail>::Do(DC_QUERY_COST, tile, ctx.railtype, track, rflags);
		}
		if (ctx.window_number == TRANSPORT_ROAD) {
			/* Road toolbar: estimate a single road piece. */
			RoadBits piece;
			switch (ctx.selected_tool) {
				case WID_ROT_ROAD_X: piece = RoadBits::ROAD_X; break;
				case WID_ROT_ROAD_Y: piece = RoadBits::ROAD_Y; break;
				default: return CMD_ERROR;
			}
			return Command<Commands::BuildRoad>::Do(DC_QUERY_COST, tile, piece, ctx.roadtype, DisallowedRoadDirections::DRD_NONE, INVALID_TOWN, BuildRoadFlags::None);
		}
		return CMD_ERROR;
	}

	if (ctx.window_class == WindowClass::ScenarioGenerateLandscape) {
		if (ctx.selected_tool != WID_TT_RAISE_LAND && ctx.selected_tool != WID_TT_LOWER_LAND) return CMD_ERROR;
		bool dir_up = (ctx.selected_tool == WID_TT_RAISE_LAND);
		return Command<Commands::TerraformLand>::Do(DC_QUERY_COST, tile, GetTileSlope(tile), dir_up);
	}

	return CMD_ERROR;
}

/**
 * Update (or hide) the construction cost tooltip for the given tool and tile.
 * @param ctx  Context of the active build tool.
 * @param tile Tile under the cursor, or INVALID_TILE to hide the tooltip.
 */
void UpdateConstructionCostTip(const ConstructionCostTipContext &ctx, TileIndex tile)
{
	/* A tool is not active (or not ours): hide the tooltip when we were the
	 * one showing it, otherwise leave it alone (another toolbar may own it). */
	if (tile == INVALID_TILE || ctx.selected_tool == INVALID_WIDGET) {
		if (ctx.window_class == _cost_tip_owner_class && ctx.window_number == _cost_tip_owner_number) {
			HideConstructionCostTip();
		}
		return;
	}

	/* Nothing changed since the last update. */
	if (tile == _cost_tip_last_tile && ctx.selected_tool == _cost_tip_last_tool) return;

	_cost_tip_last_tile = tile;
	_cost_tip_last_tool = ctx.selected_tool;

	CommandCost cost = EstimateBuildCost(ctx, tile);
	if (cost.Failed()) {
		HideConstructionCostTip();
		return;
	}

	/* Remove the previous effect before creating a new one. */
	if (_cost_tip_effect != INVALID_TE_ID) {
		RemoveTextEffect(_cost_tip_effect);
		_cost_tip_effect = INVALID_TE_ID;
	}

	/* Remember which tool owns the tooltip. */
	_cost_tip_owner_class = ctx.window_class;
	_cost_tip_owner_number = ctx.window_number;

	/* Anchor the tooltip to the tile under the cursor. */
	Point pt = RemapCoords2(TileX(tile) * TILE_SIZE, TileY(tile) * TILE_SIZE);
	_cost_tip_effect = AddTextEffect(STR_CONSTRUCTION_COST_TOOLTIP, pt.x, pt.y, 40, TE_STATIC, cost.GetCost());
}
