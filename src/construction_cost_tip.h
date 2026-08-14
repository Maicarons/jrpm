/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file construction_cost_tip.h Shows a cost tooltip near the cursor while building. */

#ifndef CONSTRUCTION_COST_TIP_H
#define CONSTRUCTION_COST_TIP_H

#include "rail_type.h"
#include "road_type.h"
#include "tile_type.h"
#include "window_type.h"

/**
 * Context describing the currently active build tool, as passed in by the
 * construction toolbar windows (rail / road / terraform).
 */
struct ConstructionCostTipContext {
	WindowClass window_class = WindowClass::Invalid;         ///< Toolbar window class.
	WindowNumber window_number = 0;                ///< Toolbar window number (e.g. TRANSPORT_RAIL / TRANSPORT_ROAD).
	WidgetID selected_tool = INVALID_WIDGET;       ///< Widget of the currently selected build tool.
	RailType railtype = INVALID_RAILTYPE;          ///< Rail type of the rail toolbar.
	RoadType roadtype = INVALID_ROADTYPE;          ///< Road type of the road toolbar.
};

/**
 * Update (or hide) the construction cost tooltip for the given tool and tile.
 *
 * The tooltip is anchored to the tile under the cursor and displays the
 * estimated cost of building the selected tool there. It is only shown when
 * the estimate succeeds (i.e. the build is possible on that tile).
 *
 * @param ctx  Context of the active build tool.
 * @param tile Tile under the cursor, or INVALID_TILE to hide the tooltip.
 */
void UpdateConstructionCostTip(const ConstructionCostTipContext &ctx, TileIndex tile);

/** Remove and hide the construction cost tooltip, if any. */
void HideConstructionCostTip();

/**
 * Get the tile under the mouse cursor, or INVALID_TILE when the cursor is
 * not over the map.
 * @return The tile under the cursor.
 */
TileIndex GetTileUnderCursor();

#endif /* CONSTRUCTION_COST_TIP_H */
