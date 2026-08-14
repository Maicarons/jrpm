/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file road_gui.h Functions/types related to the road GUIs. */

#ifndef ROAD_GUI_H
#define ROAD_GUI_H

#include "road.h"
#include "newgrf_roadstop.h"
#include "road_type.h"
#include "tile_type.h"
#include "direction_type.h"
#include "dropdown_type.h"

struct Window *ShowBuildRoadToolbar(RoadType roadtype);
struct Window *ShowBuildRoadScenToolbar(RoadType roadtype);
struct Window *CreateRoadTramToolbarForRoadType(RoadType roadtype, RoadTramType rtt);
void ShowBuildRoadStopPickerAndSelect(StationType station_type, const RoadStopSpec *spec, RoadTramType rtt_preferred);
void ConnectRoadToStructure(TileIndex tile, DiagDirection direction);
DropDownList GetRoadTypeDropDownList(RoadTramTypes rtts, bool for_replacement = false, bool all_option = false);
DropDownList GetScenRoadTypeDropDownList(RoadTramTypes rtts, bool show_cost, bool use_name = false);
void InitializeRoadGUI();

/** Selected road stop class/type/orientation (shared with cm_highlight). */
struct RoadStopPickerSelection {
	RoadStopClassID sel_class = ROADSTOP_CLASS_DFLT; ///< Selected road stop class.
	uint16_t sel_type = 0; ///< Selected road stop type within the class.
	DiagDirection orientation = DiagDirection::Invalid; ///< Selected orientation.
};

extern RoadStopPickerSelection _roadstop_gui;
extern RoadType _cur_roadtype;


#endif /* ROAD_GUI_H */
