#include "stdafx.h"

#include "cm_highlight.hpp"

#include "cm_highlight_type.hpp"
#include "cm_blueprint.hpp"
#include "cm_overlays.hpp"

#include "core/math_func.hpp"
#include "table/bridge_land.h"
#include "command_func.h"
#include "rail_gui.h"
#include "road_gui.h"
#include "station_cmd.h"
#include "station_gui.h"
#include "station_func.h"
#include "station_base.h"

extern void GetStationLayout(uint8_t *layout, uint numtracks, uint plat_len, const struct StationSpec *statspec);

/* cmclient used custom NewGRF sprites for zoning palettes; map them to the
 * built-in recolour palettes jrpm provides. */
#define CM_SPR_PALETTE_ZONING_RED PALETTE_TO_RED
#define CM_SPR_PALETTE_ZONING_ORANGE PALETTE_TO_ORANGE
#define CM_SPR_PALETTE_ZONING_GREEN PALETTE_TO_GREEN
#define CM_SPR_PALETTE_ZONING_LIGHT_BLUE PALETTE_TO_LIGHT_BLUE
#define CM_SPR_PALETTE_ZONING_YELLOW PALETTE_TO_YELLOW
#define CM_SPR_PALETTE_ZONING_WHITE PALETTE_TO_WHITE
#define CM_PALETTE_TINT_BASE PALETTE_TO_RED
/* cmclient station highlight palette tints (cm_station_gui.cpp). */
#define CM_PALETTE_TINT_WHITE   PALETTE_TO_WHITE
#define CM_PALETTE_TINT_CYAN    PALETTE_TO_CYAN
#define CM_PALETTE_TINT_BLUE    PALETTE_TO_BLUE
#define CM_PALETTE_TINT_RED_DEEP PALETTE_TO_RED
#define CM_PALETTE_TINT_YELLOW  PALETTE_TO_YELLOW
/* cmclient's extra select-proc values are now part of jrpm's
 * ViewportDragDropSelectionProcess enum (viewport_type.h). */
#include "house.h"
#include "industry.h"
#include "landscape.h"
#include "newgrf_airporttiles.h"
#include "newgrf_cargo.h"  // SpriteGroupCargo
#include "newgrf_railtype.h"
#include "newgrf_roadtype.h"
#include "newgrf_station.h"
#include "date_type.h"
#include "timer/timer_game_calendar.h"
#include "newgrf_industrytiles.h"
#include "sound_func.h"
#include "newgrf_station.h"
#include "date_type.h"
#include "timer/timer_game_calendar.h"
#include "spritecache.h"
#include "strings_func.h"
#include "town.h"
#include "town_kdtree.h"
#include "tilearea_type.h"
#include "tilehighlight_type.h"
#include "tilehighlight_func.h"
#include "viewport_func.h"
#include "window_gui.h"
#include "window_func.h"
#include "zoom_func.h"
// #include "zoning.h"
#include "table/airporttile_ids.h"
#include "table/animcursors.h"
#include "table/track_land.h"
#include "table/autorail.h"
#include "table/industry_land.h"
#include "debug.h"
#include "station_gui.h"
#include "station_type.h"
#include "table/sprites.h"
#include "table/strings.h"
#include "tile_type.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>


extern const Station *_viewport_highlight_station;
extern TileHighlightData _thd;
extern bool IsInsideSelectedRectangle(int x, int y);
extern RailType _cur_railtype;
extern RoadType _cur_roadtype;
extern AirportClassID _selected_airport_class; ///< the currently visible airport class
extern int _selected_airport_index;
extern uint8_t _selected_airport_layout;
extern DiagDirection _build_depot_direction; ///< Currently selected depot direction
extern DiagDirection _road_depot_orientation;
extern uint32_t _realtime_tick;
/* Defined in industry_gui.cpp (cmclient port). */
extern uint32_t _cm_funding_layout;
extern IndustryType _cm_funding_type;
extern void SetSelectionTilesDirty();

extern StationPickerSelection _station_gui; ///< Settings of the station picker.
extern RoadStopPickerSelection _roadstop_gui;


template <>
struct std::hash<citymania::ObjectTileHighlight> {
    std::size_t operator()(const citymania::ObjectTileHighlight &oh) const {
        std::size_t h = std::hash<SpriteID>()(oh.palette);
        h ^= hash<citymania::ObjectTileHighlight::Type>()(oh.type);
        switch (oh.type) {
            case citymania::ObjectTileHighlight::Type::RAIL_DEPOT:
                h ^= std::hash<DiagDirection>()(oh.u.rail.depot.ddir);
                break;
            case citymania::ObjectTileHighlight::Type::RAIL_TRACK:
                h ^= std::hash<Track>()(oh.u.rail.track);
                break;
            case citymania::ObjectTileHighlight::Type::RAIL_STATION:
                h ^= hash<Axis>()(oh.u.rail.station.axis);
                h ^= oh.u.rail.station.section;
                break;
            case citymania::ObjectTileHighlight::Type::RAIL_SIGNAL:
                h ^= hash<uint>()(oh.u.rail.signal.pos);
                h ^= hash<SignalType>()(oh.u.rail.signal.type);
                h ^= hash<SignalVariant>()(oh.u.rail.signal.variant);
                break;
            case citymania::ObjectTileHighlight::Type::RAIL_BRIDGE_HEAD:
                h ^= hash<DiagDirection>()(oh.u.rail.bridge_head.ddir);
                h ^= hash<uint32_t>()(oh.u.rail.bridge_head.other_end);
                h ^= hash<BridgeType>()(oh.u.rail.bridge_head.type);
                break;
            case citymania::ObjectTileHighlight::Type::RAIL_TUNNEL_HEAD:
                h ^= hash<DiagDirection>()(oh.u.rail.tunnel_head.ddir);
                break;
            case citymania::ObjectTileHighlight::Type::ROAD_STOP:
                h ^= hash<DiagDirection>()(oh.u.road.stop.ddir);
                h ^= hash<RoadType>()(oh.u.road.stop.roadtype);
                h ^= hash<bool>()(oh.u.road.stop.is_truck);
                h ^= hash<uint16_t>()(oh.u.road.stop.spec_class.base());
                h ^= hash<uint16_t>()(oh.u.road.stop.spec_index);
                break;
            case citymania::ObjectTileHighlight::Type::ROAD_DEPOT:
                h ^= hash<DiagDirection>()(oh.u.road.depot.ddir);
                h ^= hash<RoadType>()(oh.u.road.depot.roadtype);
                break;
            case citymania::ObjectTileHighlight::Type::DOCK_SLOPE:
                h ^= hash<DiagDirection>()(oh.u.dock_slope.ddir);
                break;
            case citymania::ObjectTileHighlight::Type::DOCK_FLAT:
                h ^= hash<Axis>()(oh.u.dock_flat.axis);
                break;
            case citymania::ObjectTileHighlight::Type::AIRPORT_TILE:
                h ^= hash<StationGfx>()(oh.u.airport_tile.gfx);
                break;
            case citymania::ObjectTileHighlight::Type::INDUSTRY_TILE:
                h ^= hash<IndustryGfx>()(oh.u.industry_tile.gfx);
                h ^= hash<IndustryType>()(oh.u.industry_tile.ind_type);
                h ^= oh.u.industry_tile.ind_layout;
                h ^= hash<TileIndexDiff>()(oh.u.industry_tile.tile_diff);
                break;
            case citymania::ObjectTileHighlight::Type::NUMBERED_RECT:
                h ^= hash<uint32_t>()(oh.u.numbered_rect.number);
                break;
            case citymania::ObjectTileHighlight::Type::BORDER:
                h ^= hash<citymania::ZoningBorder>()(oh.u.border);
                break;
            case citymania::ObjectTileHighlight::Type::POINT:
            case citymania::ObjectTileHighlight::Type::RECT:
            case citymania::ObjectTileHighlight::Type::END:
            case citymania::ObjectTileHighlight::Type::TINT:
            case citymania::ObjectTileHighlight::Type::STRUCT_TINT:
                break;
        }
        return h;
    }
};

namespace citymania {

/** Compute the tile area into which the given station can be joined/extended
 *  (cmclient port from cm_station_gui.cpp). */
static TileArea GetStationJoinArea(StationID station_id)
{
    auto station = Station::GetIfValid(station_id);
    if (station == nullptr) return {};
    auto &r = station->rect;
    auto d = (int)_settings_game.station.station_spread - 1;
    TileArea ta(
        TileXY(std::max<int>(r.right - d, 0), std::max<int>(r.bottom - d, 0)),
        TileXY(std::min<int>(r.left + d, (int)Map::SizeX() - 1),
               std::min<int>(r.top + d, (int)Map::SizeY() - 1)));
    return ta;
}

/* Cached GUI info (highlight map + overlay data + cost) for the currently
 * active object tool. Filled by citymania::UpdateTileSelection() and
 * consumed by citymania::UpdateActiveTool(). Mirrors cmclient's
 * placement-driven ToolGUIInfo but bypasses the Tool class machinery
 * (which is not ported to jrpm). */
ToolGUIInfo _cm_gui_info;
bool _cm_gui_active = false;

TileArea ClampToVisibleMap(const TileArea &area) {
    if (area.tile >= Map::Size()) return {};
    auto x = TileX(area.tile);
    auto y = TileY(area.tile);
    uint16_t w = area.w;
    uint16_t h = area.h;
    uint16_t border = 0;
    if (_settings_game.construction.freeform_edges) {
        if (x == 0) {
            x = 1;
            if (w == 0) return {};
            w -= 1;
        } else if (x >= Map::SizeX() - 1)
            return {};

        if (y == 0) {
            y = 1;
            if (h == 0) return {};
            h -= 1;
        } else if (y >= Map::SizeY() - 1)
            return {};

        border = 1;
    }
    return TileArea{
        TileXY(x, y),
        std::min<uint16_t>(w, Map::SizeX() - border - x),
        std::min<uint16_t>(h, Map::SizeY() - border - y)
    };
}


extern HighLightStyle (*GetPartOfAutoLine)(int px, int py, const Point &selstart, const Point &selend, HighLightStyle dir);

struct TileZoning {
    uint8_t town_zone : 3;
    uint8_t industry_fund_result : 2;
    uint8_t advertisement_zone : 2;
    // IndustryType industry_fund_type;
    uint8_t industry_fund_update;
};

static std::unique_ptr<TileZoning[]> _mz = nullptr;
static IndustryType _industry_forbidden_tiles = IT_INVALID;

extern bool _fn_mod;

std::set<std::pair<uint32_t, const Town*>, std::greater<std::pair<uint32_t, const Town*>>> _town_cache;
// struct {
//     int w;
//     int h;
//     int catchment;
// } _station_select;

const uint8_t _tileh_to_sprite[32] = {
    0, 1, 2, 3, 4, 5, 6,  7, 8, 9, 10, 11, 12, 13, 14, 0,
    0, 0, 0, 0, 0, 0, 0, 16, 0, 0,  0, 17,  0, 15, 18, 0,
};

// Copied from rail_cmd.cpp
static const TileIndexDiffC _trackdelta[] = {
    { -1,  0 }, {  0,  1 }, { -1,  0 }, {  0,  1 }, {  1,  0 }, {  0,  1 },
    {  0,  0 },
    {  0,  0 },
    {  1,  0 }, {  0, -1 }, {  0, -1 }, {  1,  0 }, {  0, -1 }, { -1,  0 },
    {  0,  0 },
    {  0,  0 }
};

ObjectTileHighlight ObjectTileHighlight::make_rail_depot(SpriteID palette, DiagDirection ddir) {
    auto oh = ObjectTileHighlight(Type::RAIL_DEPOT, palette);
    oh.u.rail.depot.ddir = ddir;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_rail_track(SpriteID palette, Track track) {
    auto oh = ObjectTileHighlight(Type::RAIL_TRACK, palette);
    oh.u.rail.track = track;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_rail_station(SpriteID palette, Axis axis, uint8_t section, StationClassID spec_class, uint16_t spec_index, TileArea whole_area) {
    auto oh = ObjectTileHighlight(Type::RAIL_STATION, palette);
    oh.u.rail.station.axis = axis;
    oh.u.rail.station.section = section;
    oh.u.rail.station.spec_class = spec_class;
    oh.u.rail.station.spec_index = spec_index;
    oh.u.rail.station.base_tile = whole_area.tile.base();
    oh.u.rail.station.w = whole_area.w;
    oh.u.rail.station.h = whole_area.h;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_rail_signal(SpriteID palette, uint pos, SignalType type, SignalVariant variant) {
    auto oh = ObjectTileHighlight(Type::RAIL_SIGNAL, palette);
    oh.u.rail.signal.pos = pos;
    oh.u.rail.signal.type = type;
    oh.u.rail.signal.variant = variant;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_rail_bridge_head(SpriteID palette, DiagDirection ddir, BridgeType type) {
    auto oh = ObjectTileHighlight(Type::RAIL_BRIDGE_HEAD, palette);
    oh.u.rail.bridge_head.ddir = ddir;
    oh.u.rail.bridge_head.type = type;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_rail_tunnel_head(SpriteID palette, DiagDirection ddir) {
    auto oh = ObjectTileHighlight(Type::RAIL_TUNNEL_HEAD, palette);
    oh.u.rail.tunnel_head.ddir = ddir;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_road_stop(SpriteID palette, RoadType roadtype, DiagDirection ddir, bool is_truck, RoadStopClassID spec_class, uint16_t spec_index) {
    auto oh = ObjectTileHighlight(Type::ROAD_STOP, palette);
    oh.u.road.stop.roadtype = roadtype;
    oh.u.road.stop.ddir = ddir;
    oh.u.road.stop.is_truck = is_truck;
    oh.u.road.stop.spec_class = spec_class;
    oh.u.road.stop.spec_index = spec_index;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_road_depot(SpriteID palette, RoadType roadtype, DiagDirection ddir) {
    auto oh = ObjectTileHighlight(Type::ROAD_DEPOT, palette);
    oh.u.road.depot.roadtype = roadtype;
    oh.u.road.depot.ddir = ddir;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_dock_slope(SpriteID palette, DiagDirection ddir) {
    auto oh = ObjectTileHighlight(Type::DOCK_SLOPE, palette);
    oh.u.dock_slope.ddir = ddir;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_dock_flat(SpriteID palette, Axis axis) {
    auto oh = ObjectTileHighlight(Type::DOCK_FLAT, palette);
    oh.u.dock_flat.axis = axis;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_airport_tile(SpriteID palette, StationGfx gfx) {
    auto oh = ObjectTileHighlight(Type::AIRPORT_TILE, palette);
    oh.u.airport_tile.gfx = gfx;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_industry_tile(SpriteID palette, IndustryType ind_type, uint8_t ind_layout, TileIndexDiff tile_diff, IndustryGfx gfx) {
    auto oh = ObjectTileHighlight(Type::INDUSTRY_TILE, palette);
    oh.u.industry_tile.tile_diff = tile_diff;
    oh.u.industry_tile.gfx = gfx;
    oh.u.industry_tile.ind_type = ind_type;
    oh.u.industry_tile.ind_layout = ind_layout;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_point(SpriteID palette) {
    return ObjectTileHighlight(Type::POINT, palette);
}

ObjectTileHighlight ObjectTileHighlight::make_rect(SpriteID palette) {
    auto oh = ObjectTileHighlight(Type::RECT, palette);
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_numbered_rect(SpriteID palette, uint32_t number) {
    auto oh = ObjectTileHighlight(Type::NUMBERED_RECT, palette);
    oh.u.numbered_rect.number = number;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_border(SpriteID palette, ZoningBorder border) {
    auto oh = ObjectTileHighlight(Type::BORDER, palette);
    oh.u.border = border;
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_tint(SpriteID palette) {
    auto oh = ObjectTileHighlight(Type::TINT, palette);
    return oh;
}

ObjectTileHighlight ObjectTileHighlight::make_struct_tint(SpriteID palette) {
    auto oh = ObjectTileHighlight(Type::STRUCT_TINT, palette);
    return oh;
}

bool ObjectTileHighlight::operator==(const ObjectTileHighlight &oh) const {
    if (this->type != oh.type) return false;
    if (this->palette != oh.palette) return false;
    switch (this->type) {
        case ObjectTileHighlight::Type::RAIL_DEPOT:
            return this->u.rail.depot.ddir == oh.u.rail.depot.ddir;
        case ObjectTileHighlight::Type::RAIL_TRACK:
            return this->u.rail.track == oh.u.rail.track;
        case ObjectTileHighlight::Type::RAIL_STATION:
            return this->u.rail.station.axis == oh.u.rail.station.axis
                && this->u.rail.station.section == oh.u.rail.station.section
                && this->u.rail.station.spec_class == oh.u.rail.station.spec_class
                && this->u.rail.station.spec_index == oh.u.rail.station.spec_index
                && this->u.rail.station.base_tile == oh.u.rail.station.base_tile
                && this->u.rail.station.w == oh.u.rail.station.w
                && this->u.rail.station.h == oh.u.rail.station.h;
        case ObjectTileHighlight::Type::RAIL_SIGNAL:
            return this->u.rail.signal.pos == oh.u.rail.signal.pos
                && this->u.rail.signal.type == oh.u.rail.signal.type
                && this->u.rail.signal.variant == oh.u.rail.signal.variant;
        case ObjectTileHighlight::Type::RAIL_BRIDGE_HEAD:
            return this->u.rail.bridge_head.ddir == oh.u.rail.bridge_head.ddir
                && this->u.rail.bridge_head.other_end == oh.u.rail.bridge_head.other_end
                && this->u.rail.bridge_head.type == oh.u.rail.bridge_head.type;
        case ObjectTileHighlight::Type::RAIL_TUNNEL_HEAD:
            return this->u.rail.tunnel_head.ddir == oh.u.rail.tunnel_head.ddir;
        case ObjectTileHighlight::Type::ROAD_STOP:
            return this->u.road.stop.ddir == oh.u.road.stop.ddir
                && this->u.road.stop.roadtype == oh.u.road.stop.roadtype
                && this->u.road.stop.is_truck == oh.u.road.stop.is_truck
                && this->u.road.stop.spec_class == oh.u.road.stop.spec_class
                && this->u.road.stop.spec_index == oh.u.road.stop.spec_index;
        case ObjectTileHighlight::Type::ROAD_DEPOT:
            return this->u.road.depot.ddir == oh.u.road.depot.ddir
                && this->u.road.depot.roadtype == oh.u.road.depot.roadtype;
        case ObjectTileHighlight::Type::AIRPORT_TILE:
            return this->u.airport_tile.gfx == oh.u.airport_tile.gfx;
        case ObjectTileHighlight::Type::INDUSTRY_TILE:
            return this->u.industry_tile.gfx == oh.u.industry_tile.gfx
                && this->u.industry_tile.ind_type == oh.u.industry_tile.ind_type
                && this->u.industry_tile.ind_layout == oh.u.industry_tile.ind_layout
                && this->u.industry_tile.tile_diff == oh.u.industry_tile.tile_diff;
        case ObjectTileHighlight::Type::NUMBERED_RECT:
            return this->u.numbered_rect.number == oh.u.numbered_rect.number;
        case Type::BORDER:
            return this->u.border == oh.u.border;
        case ObjectTileHighlight::Type::DOCK_SLOPE:
            return this->u.dock_slope.ddir == oh.u.dock_slope.ddir;
        case ObjectTileHighlight::Type::DOCK_FLAT:
            return this->u.dock_flat.axis == oh.u.dock_flat.axis;
        case Type::POINT:
        case Type::RECT:
        case Type::END:
        case Type::TINT:
        case Type::STRUCT_TINT:
            return true;
    }
    return true;
}

bool ObjectTileHighlight::SetTileHighlight(TileHighlight &th, const TileInfo *ti) const {
    switch (this->type) {
        case ObjectTileHighlight::Type::RAIL_DEPOT:
        // case ObjectTileHighlight::Type::RAIL_TRACK:  Depot track shouldn't remove foundation
        case ObjectTileHighlight::Type::RAIL_STATION:
        case ObjectTileHighlight::Type::RAIL_SIGNAL:
        case ObjectTileHighlight::Type::RAIL_BRIDGE_HEAD:
        case ObjectTileHighlight::Type::RAIL_TUNNEL_HEAD:
        case ObjectTileHighlight::Type::ROAD_STOP:
        case ObjectTileHighlight::Type::ROAD_DEPOT:
        case ObjectTileHighlight::Type::AIRPORT_TILE:
        case ObjectTileHighlight::Type::INDUSTRY_TILE:
        case ObjectTileHighlight::Type::DOCK_SLOPE:
        case ObjectTileHighlight::Type::DOCK_FLAT:
            th.set_structure(this->palette);
            return true;
        case ObjectTileHighlight::Type::TINT:
            th.tint_all(this->palette);
            return true;
        case ObjectTileHighlight::Type::STRUCT_TINT:
            th.tint_structure_prio(this->palette);
            return true;

        default:
            break;
    }
    return false;
}

bool ObjectHighlight::operator==(const ObjectHighlight& oh) const {
    if (this->type != oh.type) return false;
    return (this->tile == oh.tile
            && this->end_tile == oh.end_tile
            && this->trackdir == oh.trackdir
            && this->tile2 == oh.tile2
            && this->end_tile2 == oh.end_tile2
            && this->trackdir2 == oh.trackdir2
            && this->axis == oh.axis
            && this->ddir == oh.ddir
            && this->roadtype == oh.roadtype
            && this->is_truck == oh.is_truck
            && this->airport_type == oh.airport_type
            && this->airport_layout == oh.airport_layout
            && this->blueprint == oh.blueprint);
}


bool ObjectHighlight::operator!=(const ObjectHighlight& oh) const {
    return !(*this == oh);
}


ObjectHighlight ObjectHighlight::make_rail_depot(TileIndex tile, DiagDirection ddir) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::RAIL_DEPOT};
    oh.tile = tile;
    oh.ddir = ddir;
    return oh;
}

ObjectHighlight ObjectHighlight::make_rail_station(TileIndex start_tile, uint16_t w, uint16_t h, Axis axis, StationClassID station_class, uint16_t station_type) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::RAIL_STATION};
    oh.tile = start_tile;
    oh.w = w;
    oh.h = h;
    oh.axis = axis;
    oh.rail_station_class = station_class;
    oh.rail_station_type = station_type;
    return oh;
}

ObjectHighlight ObjectHighlight::make_road_stop(TileIndex start_tile, uint16_t w, uint16_t h, RoadType roadtype, DiagDirection orientation, bool is_truck, RoadStopClassID spec_class, uint16_t spec_index) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::ROAD_STOP};
    oh.tile = start_tile;
    oh.w = w;
    oh.h = h;
    oh.ddir = orientation;
    oh.roadtype = roadtype;
    oh.is_truck = is_truck;
    oh.road_stop_spec_class = spec_class;
    oh.road_stop_spec_index = spec_index;
    return oh;
}

ObjectHighlight ObjectHighlight::make_road_depot(TileIndex tile, RoadType roadtype, DiagDirection orientation) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::ROAD_DEPOT};
    oh.tile = tile;
    oh.ddir = orientation;
    oh.roadtype = roadtype;
    return oh;
}

ObjectHighlight ObjectHighlight::make_airport(TileIndex start_tile, int airport_type, uint8_t airport_layout) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::AIRPORT};
    oh.tile = start_tile;
    oh.airport_type = airport_type;
    oh.airport_layout = airport_layout;
    return oh;
}

ObjectHighlight ObjectHighlight::make_blueprint(TileIndex tile, sp<Blueprint> blueprint) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::BLUEPRINT};
    oh.tile = tile;
    oh.blueprint = blueprint;
    return oh;
}

ObjectHighlight ObjectHighlight::make_polyrail(TileIndex start_tile, TileIndex end_tile, Trackdir trackdir,
                                               TileIndex start_tile2, TileIndex end_tile2, Trackdir trackdir2) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::POLYRAIL};
    oh.tile = start_tile;
    oh.end_tile = end_tile;
    oh.trackdir = trackdir;
    oh.tile2 = start_tile2;
    oh.end_tile2 = end_tile2;
    oh.trackdir2 = trackdir2;
    return oh;
}

ObjectHighlight ObjectHighlight::make_industry(TileIndex tile, IndustryType ind_type, uint32_t ind_layout) {
    auto oh = ObjectHighlight{ObjectHighlight::Type::INDUSTRY};
    oh.tile = tile;
    oh.ind_type = ind_type;
    oh.ind_layout = ind_layout;
    return oh;
}

ObjectHighlight ObjectHighlight::make_dock(TileIndex tile, DiagDirection orientation) {
	auto oh = ObjectHighlight{ObjectHighlight::Type::DOCK};
    oh.tile = tile;
    oh.ddir = orientation;
	return oh;
}

/**
 * Try to add an additional rail-track at the entrance of a depot
 * @param tile  Tile to use for adding the rail-track
 * @param dir   Direction to check for already present tracks
 * @param track Track to add
 * @see CcRailDepot()
 */
void ObjectHighlight::PlaceExtraDepotRail(TileIndex tile, DiagDirection dir, Track track)
{
    if (GetRailTileType(tile) != RailTileType::Normal) return;
    if ((GetTrackBits(tile) & DiagdirReachesTracks(dir)) == 0) return;

    this->AddTile(tile, ObjectTileHighlight::make_rail_track(PALETTE_TO_WHITE, track));
}

/** Additional pieces of track to add at the entrance of a depot. */
static const Track _place_depot_extra_track[12] = {
    TRACK_LEFT,  TRACK_UPPER, TRACK_UPPER, TRACK_RIGHT, // First additional track for directions 0..3
    TRACK_X,     TRACK_Y,     TRACK_X,     TRACK_Y,     // Second additional track
    TRACK_LOWER, TRACK_LEFT,  TRACK_RIGHT, TRACK_LOWER, // Third additional track
};

/** Direction to check for existing track pieces. */
static const DiagDirection _place_depot_extra_dir[12] = {
    DiagDirection::SE, DiagDirection::SW, DiagDirection::SE, DiagDirection::SW,
    DiagDirection::SW, DiagDirection::NW, DiagDirection::NE, DiagDirection::SE,
    DiagDirection::NW, DiagDirection::NE, DiagDirection::NW, DiagDirection::NE,
};

void ObjectHighlight::AddTile(TileIndex tile, ObjectTileHighlight &&oh) {
    if (tile >= Map::Size()) return;
    if (_settings_game.construction.freeform_edges) {
        auto x = TileX(tile);
        auto y = TileY(tile);
        if (x == 0 || x >= Map::SizeX() - 1) return;
        if (y == 0 || y >= Map::SizeY() - 1) return;
    }

    this->tiles.insert(std::make_pair(tile, std::move(oh)));
}

uint16_t GetPreviewStationCallback(CallbackID callback, uint32_t param1, uint32_t param2, const StationSpec *statspec, TileIndex tile, TileArea area, StationGfx gfx, Axis axis);
uint16_t GetPurchaseStationCallback(CallbackID callback, uint32_t param1, uint32_t param2, const StationSpec *statspec, TileIndex tile, TileArea area);

std::map<std::tuple<const StationSpec *, Axis, TileIndex, int16_t, int16_t>, std::vector<uint8_t>> _station_layout_cache;
std::vector<uint8_t> &GetPreviewStationLayout(const StationSpec *statspec, Axis axis, TileArea area) {
    static std::vector<uint8_t> _empty_layout;
    if (area.w == 0 || area.h == 0) return _empty_layout;

    std::tuple<const StationSpec *, Axis, TileIndex, int16_t, int16_t> key{statspec, axis, area.tile, area.w, area.h};
    auto it = _station_layout_cache.find(key);
    if (it != _station_layout_cache.end()) return it->second;
    uint8_t numtracks = area.w;
    uint8_t plat_len = area.h;
    if (axis == Axis::X) std::swap(numtracks, plat_len);

    it = _station_layout_cache.insert(it, {key, std::vector<uint8_t>(area.w * area.h)});
    auto &res_layout = it->second;

    /* jrpm: use the built-in station layout generator (vanilla cmclient used
     * RailStationTileLayout + IterateStation, which jrpm does not provide). */
    GetStationLayout(res_layout.data(), numtracks, plat_len, statspec);

    return res_layout;
}

void ObjectHighlight::UpdateTiles() {
    this->tiles.clear();
    this->sprites.clear();
    this->cost = CMD_ERROR;
    switch (this->type) {
        case Type::NONE:
            break;

        case Type::RAIL_DEPOT: {
            auto dir = this->ddir;

            this->cost = CMD_ERROR;
            auto palette = (cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);

            this->tiles.insert(std::make_pair(this->tile, ObjectTileHighlight::make_rail_depot(palette, dir)));
            auto tile = AddTileIndexDiffCWrap(this->tile, TileIndexDiffCByDiagDir(dir));
            if (tile == INVALID_TILE) break;
            if (IsTileType(tile, TileType::Railway) && IsCompatibleRail(GetRailType(tile), _cur_railtype)) {
                this->PlaceExtraDepotRail(tile, _place_depot_extra_dir[static_cast<uint8_t>(dir)], _place_depot_extra_track[static_cast<uint8_t>(dir)]);
                this->PlaceExtraDepotRail(tile, _place_depot_extra_dir[static_cast<uint8_t>(dir) + 4 & 3], _place_depot_extra_track[static_cast<uint8_t>(dir) + 4 & 3]);
                this->PlaceExtraDepotRail(tile, _place_depot_extra_dir[static_cast<uint8_t>(dir) + 8 & 3], _place_depot_extra_track[static_cast<uint8_t>(dir) + 8 & 3]);
            }
            break;
        }
        case Type::RAIL_STATION: {
            auto ta = OrthogonalTileArea(this->tile, this->w, this->h);
            auto numtracks = ta.w;
            auto plat_len = ta.h;
            if (this->axis == Axis::X) std::swap(numtracks, plat_len);

            this->cost = CMD_ERROR;
            auto palette = (this->cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);

            ta = ClampToVisibleMap(ta);
            /* Note: since ta is clamped to map preview may not be accurate, but it's even worse with wrapping. */
            const StationSpec *statspec = StationClass::Get(this->rail_station_class)->GetSpec(this->rail_station_type);
            auto layout = GetPreviewStationLayout(statspec, this->axis, ta);
            auto it = layout.begin();
            for (auto tile : ta) {
                this->AddTile(tile, ObjectTileHighlight::make_rail_station(
                    palette,
                    this->axis,
                    *it++,
                    this->rail_station_class,
                    this->rail_station_type,
                    ta
                ));
            }

            break;
        }
        case Type::ROAD_STOP: {
            auto ta = OrthogonalTileArea(this->tile, this->w, this->h);
            this->cost = CMD_ERROR;
            auto palette = (this->cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);
            ta = ClampToVisibleMap(ta);
            for (TileIndex tile : ta) {
                this->AddTile(tile, ObjectTileHighlight::make_road_stop(palette, this->roadtype, this->ddir, this->is_truck, this->road_stop_spec_class, this->road_stop_spec_index));
            }
            break;
        }

        case Type::ROAD_DEPOT: {
            this->cost = CMD_ERROR;
            auto palette = (this->cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);
            this->AddTile(this->tile, ObjectTileHighlight::make_road_depot(palette, this->roadtype, this->ddir));
            break;
        }

        case Type::AIRPORT: {
            this->cost = CMD_ERROR;
            auto palette = (this->cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);

                        const AirportSpec *as = AirportSpec::Get(this->airport_type);
            if (!as->IsAvailable() || this->airport_layout >= as->layouts.size()) break;
            const AirportTileLayout &layout = as->layouts[this->airport_layout];
            uint16_t w = layout.size_x;
            uint16_t h = layout.size_y;
            auto ta = ClampToVisibleMap(TileArea{this->tile, w, h});
            for (uint16_t ly = 0; ly < h; ly++) {
                for (uint16_t lx = 0; lx < w; lx++) {
                    uint pos = ly * w + lx;
                    if (pos >= layout.tiles.size() || layout.tiles[pos].type == ATT_INVALID) continue;
                    TileIndex iter = this->tile + TileDiffXY(lx, ly);
                    if (!ta.Contains(iter)) continue;
                    this->AddTile(iter, ObjectTileHighlight::make_airport_tile(palette, static_cast<StationGfx>(layout.tiles[pos].gfx[0])));
                }
            }
            break;
        }
        case Type::BLUEPRINT:
            if (this->blueprint && this->tile != INVALID_TILE)
                this->tiles = this->blueprint->GetTiles(this->tile);
            break;
        case Type::POLYRAIL: {

            auto point1 = this->tile;
            auto point2 = INVALID_TILE;
            switch (trackdir) {
                case TRACKDIR_X_NE:
                    point1 += ToTileIndexDiff({1, 0});
                    point2 = point1 + ToTileIndexDiff({0, 1});
                    break;
                case TRACKDIR_Y_NW:
                    point1 += ToTileIndexDiff({0, 1});
                    point2 = point1 + ToTileIndexDiff({1, 0});
                    break;
                case TRACKDIR_Y_SE:
                    point2 = point1 + ToTileIndexDiff({1, 0});
                    break;
                case TRACKDIR_X_SW:
                    point2 = point1 + ToTileIndexDiff({0, 1});
                    break;
                case TRACKDIR_RIGHT_N:
                case TRACKDIR_LEFT_N:
                    point1 += ToTileIndexDiff({1, 1});
                    break;
                case TRACKDIR_UPPER_W:
                case TRACKDIR_LOWER_W:
                    point1 += ToTileIndexDiff({0, 1});
                    break;
                case TRACKDIR_UPPER_E:
                case TRACKDIR_LOWER_E:
                    point1 += ToTileIndexDiff({1, 0});
                    break;
                // TRACKDIR_RIGHT/LEFT_S - ok
                default:
                    break;
            }
            this->AddTile(point1, ObjectTileHighlight::make_point(PALETTE_TO_WHITE));
            if (point2 != INVALID_TILE)
                this->AddTile(point2, ObjectTileHighlight::make_point(PALETTE_TO_WHITE));
            auto z = TileHeight(point1);

            auto add_track = [this, z](TileIndex tile, TileIndex end_tile, Trackdir trackdir, SpriteID palette, TileIndex point1, TileIndex point2) {
                if (trackdir == INVALID_TRACKDIR) return;

                while(tile <= Map::Size()) {
                    this->sprites.emplace_back(
                        RemapCoords(TileX(tile) * TILE_SIZE, TileY(tile) * TILE_SIZE, z * TILE_HEIGHT + 7 /* z_offset */),
                        SPR_AUTORAIL_BASE + _autorail_slope_sprite_offsets[0][TrackdirToTrack(trackdir)],
                        palette
                    );
                    // this->AddTile(tile, std::move(ObjectTileHighlight::make_rail_track(palette, TrackdirToTrack(trackdir)).set_z(z)));
                    if (point1 != INVALID_TILE) {
                        point1 += ToTileIndexDiff(_trackdelta[trackdir]);
                        this->AddTile(point1, ObjectTileHighlight::make_point(PALETTE_TO_WHITE));
                    }
                    if (point2 != INVALID_TILE) {
                        point2 += ToTileIndexDiff(_trackdelta[trackdir]);
                        this->AddTile(point2, ObjectTileHighlight::make_point(PALETTE_TO_WHITE));
                    }

                    if (tile == end_tile) break;

                    tile += ToTileIndexDiff(_trackdelta[trackdir]);
                    /* toggle railbit for the non-diagonal tracks */
                    if (!IsDiagonalTrackdir(trackdir)) ToggleBit(trackdir, 0);
                }
                if (!IsDiagonalTrackdir(trackdir) && point1 != INVALID_TILE) {
                    ToggleBit(trackdir, 0);
                    point1 += ToTileIndexDiff(_trackdelta[trackdir]);
                    this->AddTile(point1, ObjectTileHighlight::make_point(PALETTE_TO_WHITE));
                }
            };
            add_track(this->tile, this->end_tile, this->trackdir, PALETTE_TO_YELLOW, point1, point2);
            add_track(this->tile2, this->end_tile2, this->trackdir2, PALETTE_SEL_TILE_BLUE, INVALID_TILE, INVALID_TILE);
            break;
        }
        case Type::INDUSTRY: {
            this->cost = CMD_ERROR;
            if (this->cost.Succeeded()) {
                const IndustrySpec *indspec = GetIndustrySpec(this->ind_type);
                if (indspec == nullptr) break;
                if (this->ind_layout >= indspec->layouts.size()) break;

                const IndustryTileLayout &layout = indspec->layouts[this->ind_layout];
                for (const IndustryTileLayoutTile &it : layout) {
                    if (it.gfx == GFX_WATERTILE_SPECIALCHECK) continue;
                    auto tile_diff = ToTileIndexDiff(it.ti);
                    TileIndex cur_tile = this->tile + tile_diff;
                    // WaterClass wc = (IsWaterTile(cur_tile) ? GetWaterClass(cur_tile) : WATER_CLASS_INVALID);
                    this->AddTile(
                        cur_tile,
                        ObjectTileHighlight::make_industry_tile(
                            PALETTE_TO_WHITE,
                            this->ind_type,
                            this->ind_layout,
                            tile_diff,
                            it.gfx
                        )
                    );
                }
            } else {
                this->AddTile(this->tile, ObjectTileHighlight::make_rect(CM_SPR_PALETTE_ZONING_RED));
            }
            break;
        }
        case Type::DOCK: {
            this->cost = CMD_ERROR;
            auto palette = (cost.Succeeded() ? PALETTE_TO_WHITE : PALETTE_TO_RED);
            this->AddTile(this->tile, ObjectTileHighlight::make_dock_slope(palette, this->ddir));
            if (this->ddir != DiagDirection::Invalid) {
                TileIndex tile_to = TileAddByDiagDir(this->tile, this->ddir);
                this->AddTile(tile_to, ObjectTileHighlight::make_dock_flat(palette, DiagDirToAxis(this->ddir)));
            }
            break;
        }
        default:
            NOT_REACHED();
    }
}

void ObjectHighlight::MarkDirty() {
    for (const auto &kv: this->tiles) {
        /* Defensive: tiles may reference tile indices left over from a
         * previous map (e.g. on return to the menu screen the previous
         * game's high-light tiles would otherwise index _m beyond the new
         * 64x64 menu map and crash). */
        if (kv.first < Map::Size()) MarkTileDirtyByTile(kv.first);
    }
    for (const auto &s: this->sprites) {
        auto sprite = GetSprite(GB(s.sprite_id, 0, SPRITE_WIDTH), SpriteType::Normal, LowZoomMask(ZoomLevel::Normal));
        if (sprite == nullptr) continue;
        auto left = s.pt.x + sprite->x_offs;
        auto top = s.pt.y + sprite->y_offs;
        MarkAllViewportsDirty(
            left,
            top,
            left + UnScaleByZoom(sprite->width, ZoomLevel::Normal),
            top + UnScaleByZoom(sprite->height, ZoomLevel::Normal)
        );
    }
    if (this->type == ObjectHighlight::Type::BLUEPRINT && this->blueprint) {  // TODO why && blueprint check is needed?
        for (auto tile : this->blueprint->source_tiles) {
            if (tile < Map::Size()) MarkTileDirtyByTile(tile);
        }
    }
}


template <typename F>
uint8_t Get(uint32_t x, uint32_t y, F getter) {
    if (x >= Map::SizeX() || y >= Map::SizeY()) return 0;
    return getter(TileXY(x, y));
}

template <typename F>
std::pair<ZoningBorder, uint8_t> CalcTileBorders(TileIndex tile, F getter) {
    auto x = TileX(tile), y = TileY(tile);
    ZoningBorder res = ZoningBorder::NONE;
    auto z = getter(tile);
    if (z == 0)
        return std::make_pair(res, 0);
    auto tr = Get(x - 1, y, getter);
    auto tl = Get(x, y - 1, getter);
    auto bl = Get(x + 1, y, getter);
    auto br = Get(x, y + 1, getter);
    if (tr < z) res |= ZoningBorder::TOP_RIGHT;
    if (tl < z) res |= ZoningBorder::TOP_LEFT;
    if (bl < z) res |= ZoningBorder::BOTTOM_LEFT;
    if (br < z) res |= ZoningBorder::BOTTOM_RIGHT;
    if (tr == z && tl == z && Get(x - 1, y - 1, getter) < z) res |= ZoningBorder::TOP_CORNER;
    if (tr == z && br == z && Get(x - 1, y + 1, getter) < z) res |= ZoningBorder::RIGHT_CORNER;
    if (br == z && bl == z && Get(x + 1, y + 1, getter) < z) res |= ZoningBorder::BOTTOM_CORNER;
    if (tl == z && bl == z && Get(x + 1, y - 1, getter) < z) res |= ZoningBorder::LEFT_CORNER;
    return std::make_pair(res, z);
}

const HighlightMap::MapType &HighlightMap::GetMap() const {
    return this->map;
}

void HighlightMap::Add(TileIndex tile, ObjectTileHighlight oth) {
    this->map[tile].push_back(oth);
}

bool HighlightMap::Contains(TileIndex tile) const {
    return this->map.find(tile) != this->map.end();
}

std::optional<std::reference_wrapper<const std::vector<ObjectTileHighlight>>>
        HighlightMap::GetForTile(TileIndex tile) const {
    auto it = this->map.find(tile);
    if (it == this->map.end()) return std::nullopt;
    return it->second;
}

HighlightMap::MapTypeKeys HighlightMap::GetAllTiles() const {
    return std::views::keys(this->map);
}

std::vector<TileIndex> HighlightMap::UpdateWithMap(const HighlightMap &update) {
    std::vector<TileIndex> tiles_changed;
    for (auto it = this->map.begin(); it != this->map.end();) {
        tiles_changed.push_back(it->first);
        it = (update.Contains(it->first) ? std::next(it) : this->map.erase(it));
    }
    for (auto &[t, l] : update.GetMap()) {
        auto it = this->map.find(t);
        if (it != this->map.end() && it->second == l)
            continue;
        this->map.insert_or_assign(it, t, l);
        tiles_changed.push_back(t);
    }
    return tiles_changed;
}

void HighlightMap::AddTileArea(const TileArea &area, SpriteID palette) {
    if (area.w == 0 || area.h == 0) return;

    auto sx = TileX(area.tile), sy = TileY(area.tile);
    auto ex = sx + area.w - 1, ey = sy + area.h - 1;

    for (auto y = sy; y <= ey; y++) {
        for (auto x = sx; x <= ex; x++) {
            this->Add(TileXY(x, y), ObjectTileHighlight::make_tint(palette));
        }
    }
}

void HighlightMap::AddTileAreaWithBorder(const TileArea &area, SpriteID palette) {
    if (area.w == 0 || area.h == 0) return;

    this->AddTileArea(area, palette);

    auto sx = TileX(area.tile), sy = TileY(area.tile);
    auto ex = sx + area.w - 1, ey = sy + area.h - 1;

    if (area.w == 1 && area.h == 1) {
        this->Add(area.tile, ObjectTileHighlight::make_border(palette, ZoningBorder::FULL));
        return;
    }
    // NOTE: Doesn't handle one-tile width/height separately but relies on border overlapping
    this->Add(TileXY(sx, sy), ObjectTileHighlight::make_border(palette, ZoningBorder::TOP_LEFT | ZoningBorder::TOP_RIGHT));
    for (auto x = sx + 1; x < ex; x++)
        this->Add(TileXY(x, sy), ObjectTileHighlight::make_border(palette, ZoningBorder::TOP_LEFT));
    this->Add(TileXY(ex, sy), ObjectTileHighlight::make_border(palette, ZoningBorder::TOP_LEFT | ZoningBorder::BOTTOM_LEFT));
    for (auto y = sy + 1; y < ey; y++) {
        this->Add(TileXY(sx, y), ObjectTileHighlight::make_border(palette, ZoningBorder::TOP_RIGHT));
        for (auto x = sx + 1; x < ex; x++) {
            this->Add(TileXY(x, y), ObjectTileHighlight::make_border(palette, ZoningBorder::NONE));
        }
        this->Add(TileXY(ex, y), ObjectTileHighlight::make_border(palette, ZoningBorder::BOTTOM_LEFT));
    }
    this->Add(TileXY(sx, ey), ObjectTileHighlight::make_border(palette, ZoningBorder::TOP_RIGHT | ZoningBorder::BOTTOM_RIGHT));
    for (auto x = sx + 1; x < ex; x++)
        this->Add(TileXY(x, ey), ObjectTileHighlight::make_border(palette, ZoningBorder::BOTTOM_RIGHT));
    this->Add(TileXY(ex, ey), ObjectTileHighlight::make_border(palette, ZoningBorder::BOTTOM_LEFT | ZoningBorder::BOTTOM_RIGHT));
}

void HighlightMap::AddTilesBorder(const std::set<TileIndex> &tiles, SpriteID palette) {
    for (auto t : tiles) {
        auto b = CalcTileBorders(t, [&tiles](TileIndex t) {
            return tiles.find(t) == tiles.end() ? 0 : 1;
        });
        if (b.first != ZoningBorder::NONE)
            this->Add(t, ObjectTileHighlight::make_border(palette, b.first));
    }
}

SpriteID MixTints(SpriteID bottom, SpriteID top) {
    /* jrpm does not ship the custom tint sprite table cmclient used;
     * stacking falls back to the bottom tint. */
    if (top == PAL_NONE) return bottom;
    if (bottom == PAL_NONE) return top;
    return bottom;
}

SpriteID GetTintBySelectionColour(SpriteID colour, bool deep=false) {
    switch(colour) {
        case CM_SPR_PALETTE_ZONING_RED: return (deep ? PALETTE_TO_RED : PALETTE_TO_RED);
        case CM_SPR_PALETTE_ZONING_ORANGE: return (deep ? PALETTE_TO_ORANGE : PALETTE_TO_ORANGE);
        case CM_SPR_PALETTE_ZONING_GREEN: return PALETTE_TO_GREEN;
        case CM_SPR_PALETTE_ZONING_LIGHT_BLUE: return PALETTE_TO_LIGHT_BLUE;
        case CM_SPR_PALETTE_ZONING_YELLOW: return PALETTE_TO_YELLOW;
        // case SPR_PALETTE_ZONING__: return PALETTE_TINT_YELLOW_WHITE;
        case CM_SPR_PALETTE_ZONING_WHITE: return PALETTE_TO_WHITE;
        default: return PAL_NONE;
    }
}

SpriteID GetSelectionColourByTint(SpriteID colour) {
    switch(colour) {
        case PALETTE_TO_RED:
            return CM_SPR_PALETTE_ZONING_RED;
        case PALETTE_TO_ORANGE:
            return CM_SPR_PALETTE_ZONING_ORANGE;
        case PALETTE_TO_GREEN:
            return CM_SPR_PALETTE_ZONING_GREEN;
        case PALETTE_TO_LIGHT_BLUE:
            return CM_SPR_PALETTE_ZONING_LIGHT_BLUE;
        case PALETTE_TO_YELLOW:
            return CM_SPR_PALETTE_ZONING_YELLOW;
        // returnase SPR_PALETTE_ZONING__: return PALETTE_TINT_YELLOW_WHITE;
        case PALETTE_TO_WHITE:
            return CM_SPR_PALETTE_ZONING_WHITE;
        default: return PAL_NONE;
    }
}

void TileHighlight::set_old_selection(SpriteID sprite) {
    this->selection = sprite;
    this->tint_ground(GetTintBySelectionColour(sprite));
}

void TileHighlight::tint_ground(SpriteID colour) {
    this->ground_pal = MixTints(this->ground_pal, colour);
}

void TileHighlight::tint_structure(SpriteID colour) {
    this->structure_pal = MixTints(this->structure_pal, colour);
}

void DrawTrainDepotSprite(SpriteID palette, const TileInfo *ti, RailType railtype, DiagDirection ddir)
{
    const DrawTileSprites *dts = &_depot_gfx_table[ddir];
    const RailTypeInfo *rti = GetRailTypeInfo(railtype);
    SpriteID image = rti->UsesOverlay() ? SPR_FLAT_GRASS_TILE : dts->ground.sprite;
    uint32_t offset = rti->GetRailtypeSpriteOffset();

    if (image != SPR_FLAT_GRASS_TILE) image += offset;
    // PaletteID palette = COMPANY_SPRITE_COLOUR(_local_company);

    // DrawSprite(image, PAL_NONE, x, y);

    switch (ddir) {
        case DiagDirection::SW: DrawAutorailSelection(ti, HT_DIR_X, GetSelectionColourByTint(palette)); break;
        case DiagDirection::SE: DrawAutorailSelection(ti, HT_DIR_Y, GetSelectionColourByTint(palette)); break;
        default: break;
    }
    // if (rti->UsesOverlay()) {
    //     SpriteID ground = GetCustomRailSprite(rti, INVALID_TILE, RailSpriteType::Ground);

    //     switch (ddir) {
    //         case DiagDirection::SW: DrawSprite(ground + RTO_X, PALETTE_TINT_WHITE, x, y); break;
    //         case DiagDirection::SE: DrawSprite(ground + RTO_Y, PALETTE_TINT_WHITE, x, y); break;
    //         default: break;
    //     }
    // }
    int depot_sprite = GetCustomRailSprite(rti, INVALID_TILE, RailSpriteType::Depot);
    if (depot_sprite != 0) offset = depot_sprite - SPR_RAIL_DEPOT_SE_1;

    DrawRailTileSeq(ti, dts, TransparencyOption::Invalid, offset, 0, palette);
}

void AddGroundAsSortableSprite(const TileInfo *ti, SpriteID image, PaletteID pal /*, const SubSprite *sub = nullptr, int extra_offs_x = 0, int extra_offs_y = 0 */) {
    AddSortableSpriteToDraw(image, pal, *ti, {{}, {1, 1, BB_HEIGHT_UNDER_BRIDGE}, {}});
}

struct PreviewStationScopeResolver : public StationScopeResolver {
    TileArea area;
    StationGfx gfx;
    Axis axis;
    bool purchase;  // Running in purchase mode (fake vars)

    PreviewStationScopeResolver(ResolverObject &ro, const StationSpec *statspec, TileIndex tile, TileArea area, StationGfx gfx, Axis axis, bool purchase)
        : StationScopeResolver(ro, statspec, nullptr, tile, INVALID_RAILTYPE), area{area}, gfx{gfx}, axis{axis}, purchase{purchase} {}

    uint32_t GetRandomBits() const override { return 574740206;  /* It's random, I promise ;) */ };
    uint32_t GetRandomTriggers() const override { return 0; };

    TileIndex FindRailStationEnd(TileIndex tile, TileIndexDiff delta, bool check_type, bool check_axis) const
    {
        for (;;) {
            TileIndex new_tile = TileAdd(tile, delta);
            if (!this->area.Contains(new_tile)) {
                // Only run checks for tiles outside preview area
                // TODO check station index
                // if (!IsTileType(new_tile, TileType::Station) || GetStationIndex(new_tile) != sid) break;
                if (!IsTileType(new_tile, TileType::Station)) break;
                if (!HasStationRail(new_tile)) break;
                if (check_type && GetStationSpec(new_tile) != this->statspec) break;
                if (check_axis && GetRailStationAxis(new_tile) != this->axis) break;
            }

            tile = new_tile;
        }
        return tile;
    }

    uint32_t GetPlatformInfoHelper(bool check_type, bool check_axis, bool centred) const {
        int tx = TileX(this->tile);
        int ty = TileY(this->tile);
        int sx = TileX(this->FindRailStationEnd(this->tile, TileDiffXY(-1,  0), check_type, check_axis));
        int sy = TileY(this->FindRailStationEnd(this->tile, TileDiffXY( 0, -1), check_type, check_axis));
        int ex = TileX(this->FindRailStationEnd(this->tile, TileDiffXY( 1,  0), check_type, check_axis)) + 1;
        int ey = TileY(this->FindRailStationEnd(this->tile, TileDiffXY( 0,  1), check_type, check_axis)) + 1;

        tx -= sx; ex -= sx;
        ty -= sy; ey -= sy;

        // Debug(misc, 0, "GetPlatformInfoHelper ofs = {},{}  t = {},{}  e = {},{}",
        //     TileX(this->tile) - TileX(this->area.tile),
        //     TileY(this->tile) - TileY(this->area.tile),
        //     tx, ty,
        //     ex, ey
        // );

        return GetPlatformInfo(this->gfx, ex, ey, tx, ty, centred);
    }

    uint32_t GetVariable(uint16_t variable, uint32_t parameter, GetVariableExtra &extra) const override {
        // Debug(misc, 0, "Var {:x}({}) requested", variable, parameter);

        if (this->purchase) {
            // Don't try to be smart with faking wars, we actually need the dumb way.
            return StationScopeResolver::GetVariable(variable, parameter, extra);
        }

        switch (variable) {
            case 0x40: return this->GetPlatformInfoHelper(false, false, false);
            case 0x41: return this->GetPlatformInfoHelper(true,  false, false);
            case 0x42: return GetTerrainType(tile) | (GetReverseRailTypeTranslation(_cur_railtype, this->statspec->grf_prop.grffile) << 8); // use current railtype but real tile type
            case 0x43: return GetCompanyInfo(_current_company);  // Station owner - current company
            case 0x44: return 4;  // PBS status - no reservation
            // case 0x45: return 0;  TODO rail continuation info
            case 0x46: return this->GetPlatformInfoHelper(false, false, true);
            case 0x47: return this->GetPlatformInfoHelper(true,  false, true);
            case 0x49: return this->GetPlatformInfoHelper(false, true, false);

            case 0x67: { // Land info of nearby tile
                auto tile = this->tile;
                if (parameter != 0) tile = GetNearbyTile(parameter, tile, true, this->axis); // only perform if it is required

                Slope tileh = GetTileSlope(tile);
                bool swap = (this->axis == Axis::Y && HasBit(tileh, CORNER_W) != HasBit(tileh, CORNER_E));

                return GetNearbyTileInformation(tile, this->ro.grffile->grf_version >= 8, 0) ^ (swap ? SLOPE_EW : 0);
            }

            case 0x68: { // Station info of nearby tiles
                TileIndex tile = GetNearbyTile(parameter, this->tile, true, this->axis);
                // auto diff = TileIndexToTileIndexDiffC(tile, this->tile);
                // Debug(misc, 0, "Var68 tile={},{} area={},{} contains={}", diff.x, diff.y, this->area.w, this->area.h, this->area.Contains(tile));
                if (!this->area.Contains(tile)) return 0xFFFFFFFF;

                // Restore gfx from offset by quirying station layout
                auto layout = GetPreviewStationLayout(this->statspec, this->axis, this->area);
                auto ofs = TileIndexToTileIndexDiffC(tile, this->area.tile);
                auto gfx = layout[ofs.x + ofs.y * this->area.w];

                bool perpendicular = false;
                bool same_station = true;
                uint32_t res = GB(gfx, 1, 2) << 12 | !!perpendicular << 11 | !!same_station << 10;
                auto local_id = ClampTo<uint8_t>(statspec->grf_prop.local_id);
                // Debug(misc, 0, "Var68 gfx={} local_id={} ofs={},{} layout={} this->gfx={}", gfx, local_id, ofs.x, ofs.y, ofs.y * this->area.w + ofs.x, this->gfx);
                res |= 1 << 8 | local_id;
                return res;
            }

            case 0xFA: return ClampTo<uint16_t>(CalTime::CurDate() - CalTime::DAYS_TILL_ORIGINAL_BASE_YEAR); // Build date, clamped to a 16 bit value
        }

        extra.available = false;
        return UINT_MAX;
    }
};

struct StationPreivewResolverObject : public StationResolverObject {
    PreviewStationScopeResolver preview_station_scope;
    TileIndex tile;
    TileIndexDiffC offset;  // TODO remove?

    StationPreivewResolverObject(const StationSpec *statspec, TileIndex tile, TileArea area, StationGfx gfx, Axis axis, bool purchase,
            CallbackID callback = CBID_NO_CALLBACK, uint32_t callback_param1 = 0, uint32_t callback_param2 = 0)
        : StationResolverObject(statspec, nullptr, tile, INVALID_RAILTYPE, callback, callback_param1, callback_param2),
            preview_station_scope{*this, statspec, tile, area, gfx, axis, purchase},
            tile{tile}, offset{} {

        CargoType ctype = (purchase ? CargoGRFFileProps::SG_PURCHASE : CargoGRFFileProps::SG_DEFAULT_NA);
        this->root_spritegroup = statspec->grf_prop.GetSpriteGroup(ctype);
        if (!purchase && this->root_spritegroup == nullptr) {
            CargoType ctype = CargoGRFFileProps::SG_DEFAULT;
            this->root_spritegroup = statspec->grf_prop.GetSpriteGroup(ctype);
        }
        this->preview_station_scope.cargo_type = this->station_scope.cargo_type = ctype;
    }

    ScopeResolver *GetScope(VarSpriteGroupScope scope = VSG_SCOPE_SELF, VarSpriteGroupScopeOffset relative = 0) override
    {
        switch (scope) {
            case VSG_SCOPE_SELF:
                return &this->preview_station_scope;

            case VSG_SCOPE_PARENT: {
                if (!this->town_scope.has_value()) {
                    auto t = ClosestTownFromTile(this->tile, UINT_MAX);
                    this->town_scope.emplace(*this, t, true);
                }
                return &*this->town_scope;
            }

            default:
                return ResolverObject::GetScope(scope, relative);
        }
    }

    const SpriteGroup *ResolveReal(const RealSpriteGroup &group) const override
    {
        if (!this->preview_station_scope.purchase && !group.loaded.empty()) {
            return group.loaded[0];
        }
        return group.loading[0];
    }
};

uint16_t GetPreviewStationCallback(CallbackID callback, uint32_t param1, uint32_t param2, const StationSpec *statspec, TileIndex tile, TileArea area, StationGfx gfx, Axis axis)
{
    StationPreivewResolverObject object(statspec, tile, area, gfx, axis, false, callback, param1, param2);
    return object.ResolveCallback();
}

uint16_t GetPurchaseStationCallback(CallbackID callback, uint32_t param1, uint32_t param2, const StationSpec *statspec, TileIndex tile, TileArea area)
{
    StationPreivewResolverObject object(statspec, tile, area, 0, Axis::Invalid, true, callback, param1, param2);
    return object.ResolveCallback();
}

SpriteID GetCustomPreviewStationRelocation(const StationSpec *statspec, uint32_t var10, TileIndex tile, TileArea area, StationGfx gfx, Axis axis)
{
    StationPreivewResolverObject object(statspec, tile, area, gfx, axis, false, CBID_NO_CALLBACK, var10);
    const auto *group = object.Resolve<ResultSpriteGroup>();
    if (group == nullptr || group->num_sprites == 0) return 0;
    return group->sprite - SPR_RAIL_PLATFORM_Y_FRONT;
}

void DrawTrainStationSprite(SpriteID palette, const TileInfo *ti, RailType railtype, Axis axis, uint8_t section, StationClassID spec_class, uint16_t spec_index, TileArea area) {
    int32_t total_offset = 0;
    StationGfx gfx = (section & ~1) + (axis == Axis::X ? 0 : 1);
    const StationSpec *statspec = StationClass::Get(spec_class)->GetSpec(spec_index);
    const NewGRFSpriteLayout *layout = nullptr;
    const DrawTileSprites *t = nullptr;
    const RailTypeInfo *rti = nullptr;
    BaseStation *st = nullptr;

    // Debug(misc, 0, "DrawTrainStationSprite {} {} {}", spec_class, spec_index, statspec == nullptr);

    if (statspec != nullptr) {
        uint tile_layout = gfx;
        if (statspec->callback_mask.Test(StationCallbackMask::DrawTileLayout)) {
            uint16_t callback = GetPreviewStationCallback(CBID_STATION_DRAW_TILE_LAYOUT, 0, 0, statspec, ti->tile, area, gfx, axis);
            if (callback != CALLBACK_FAILED) tile_layout = (callback & ~1) + static_cast<uint8_t>(axis);
        }

        // Debug(misc, 0, "DrawTrainStationSprite layout={}", tile_layout);

        /* Ensure the chosen tile layout is valid for this custom station */
        if (!statspec->renderdata.empty()) {
            layout = &statspec->renderdata[tile_layout < statspec->renderdata.size() ? tile_layout : static_cast<uint8_t>(axis)];
            if (!layout->NeedsPreprocessing()) {
                t = layout;
                layout = nullptr;
            }
        }
    }

    // Debug(misc, 0, "DrawTrainStationSprite get default? {} {} {}", layout == nullptr, t == nullptr, t == nullptr || t->seq == nullptr);

    if (layout == nullptr && (t == nullptr || t->GetSequence().empty())) t = GetStationTileLayout(StationType::Rail, gfx);

    if (railtype != INVALID_RAILTYPE) {
        rti = GetRailTypeInfo(railtype);
        total_offset = rti->GetRailtypeSpriteOffset();
    }

    uint32_t ground_relocation = 0;
    uint32_t relocation = 0;
    DrawTileSpriteSpan tmp_layout;
    if (layout != nullptr) {
        /* Sprite layout which needs preprocessing */
        bool separate_ground = statspec->flags.Test(StationSpecFlag::SeparateGround);
        auto processor = SpriteLayoutProcessor(*layout, total_offset, rti->fallback_railtype, 0, 0, separate_ground);
        for (uint8_t var10 : processor.Var10Values()) {
            uint32_t var10_relocation = GetCustomStationRelocation(statspec, st, ti->tile, railtype, var10);
            processor.ProcessRegisters(var10, var10_relocation);
        }
        tmp_layout = processor.GetLayout();
        t = &tmp_layout;
        total_offset = 0;
    } else if (statspec != nullptr) {
        /* Simple sprite layout */
        ground_relocation = relocation = GetCustomStationRelocation(statspec, st, ti->tile, railtype, 0);
        if (statspec->flags.Test(StationSpecFlag::SeparateGround)) {
            ground_relocation = GetCustomStationRelocation(statspec, st, ti->tile, railtype, 1);
        }
        if (rti != nullptr) {
            ground_relocation += rti->fallback_railtype;
        }
    }

    // Debug(misc, 0, "DrawTrainStationSprite ground {}", t->ground.sprite);
    // const DrawTileSeqStruct *dtss;
    // foreach_draw_tile_seq(dtss, t->seq) Debug(misc, 0, "  seq {} {}", GB(dtss->image.sprite, 0, SPRITE_WIDTH), HasBit(dtss->image.sprite, CUSTOM_BIT));

    SpriteID image = t->ground.sprite;
    // PaletteID pal  = t->ground.pal;
    RailTrackOffset overlay_offset;
    if (rti != nullptr && rti->UsesOverlay() && SplitGroundSpriteForOverlay(ti, &image, &overlay_offset)) {
        SpriteID ground = GetCustomRailSprite(rti, ti->tile, RailSpriteType::Ground);
        AddGroundAsSortableSprite(ti, image, palette);
        AddGroundAsSortableSprite(ti, ground + overlay_offset, palette);
    } else {
        image += HasBit(image, SPRITE_MODIFIER_CUSTOM_SPRITE) ? ground_relocation : total_offset;
        // if (HasBit(pal, SPRITE_MODIFIER_CUSTOM_SPRITE)) pal += ground_relocation;
        AddGroundAsSortableSprite(ti, image, palette);
    }

    // DrawAutorailSelection(ti, (axis == Axis::X ? HT_DIR_X : HT_DIR_Y), GetSelectionColourByTint(palette));

    /* Default waypoint has no railtype specific sprites */
    // DrawRailTileSeq(ti, t, TransparencyOption::Invalid, (st == STATION_WAYPOINT ? 0 : total_offset), 0, PALETTE_TINT_WHITE);
    DrawRailTileSeq(ti, t, TransparencyOption::Invalid, total_offset, relocation, palette);
}

void DrawRoadStop(SpriteID palette, const TileInfo *ti, RoadType roadtype, DiagDirection orientation, bool is_truck, RoadStopClassID spec_class, uint16_t spec_index) {
    // TODO this is based on preview drawing code, not map one, is it right?
    int32_t total_offset = 0;
    const RoadTypeInfo* rti = GetRoadTypeInfo(roadtype);
    const RoadStopSpec *spec = RoadStopClass::Get(spec_class)->GetSpec(spec_index);
    uint view = (uint)orientation;
    StationType type = (is_truck ? StationType::Truck : StationType::Bus);

    const DrawTileSprites *dts = nullptr;
    DrawTileSpriteSpan dtsspan;
    if (spec != nullptr) {
        RoadStopResolverObject object(spec, nullptr, INVALID_TILE, roadtype, type, view);
        const auto *group = object.Resolve<TileLayoutSpriteGroup>();
        if (group == nullptr) return;
        auto processor = group->ProcessRegisters(nullptr);
        dtsspan = processor.GetLayout();
        dts = &dtsspan;
    } else {
        dts = GetStationTileLayout(type, view);
    }

    SpriteID image = dts->ground.sprite;
    if (GB(image, 0, SPRITE_WIDTH) != 0) {
        AddGroundAsSortableSprite(ti, image, palette);
    }

    if (view >= 4) {
        /* Drive-through stop */
        uint sprite_offset = 5 - view;

        /* Road underlay takes precedence over tram */
        if (!spec || spec->draw_mode.Test(RoadStopDrawMode::Overlay)) {
            if (rti->UsesOverlay()) {
                SpriteID ground = GetCustomRoadSprite(rti, INVALID_TILE, RoadSpriteType::Ground);
                DrawSprite(ground + sprite_offset, PAL_NONE, ti->x, ti->y);

                SpriteID overlay = GetCustomRoadSprite(rti, INVALID_TILE, RoadSpriteType::Overlay);
                // if (overlay) DrawSprite(overlay + sprite_offset, PAL_NONE, x, y);
                if (overlay) AddGroundAsSortableSprite(ti, overlay + sprite_offset, palette);
            } else if (RoadTypeIsTram(roadtype)) {
                // DrawSprite(SPR_TRAMWAY_TRAM + sprite_offset, PAL_NONE, x, y);
                AddGroundAsSortableSprite(ti, SPR_TRAMWAY_TRAM + sprite_offset, palette);
            }
        }
    } else {
        /* Bay stop */
        bool draw_mode_road = (spec != nullptr ? spec->draw_mode.Test(RoadStopDrawMode::Road) : RoadTypeIsRoad(roadtype));
        if (draw_mode_road && rti->UsesOverlay()) {
            SpriteID ground = GetCustomRoadSprite(rti, INVALID_TILE, RoadSpriteType::Roadstop);
            // DrawSprite(, PAL_NONE, x, y);
            AddGroundAsSortableSprite(ti, ground + view, palette);
        }
    }

    DrawRailTileSeq(ti, dts, TransparencyOption::Invalid, total_offset, 0, palette);
}

void DrawDockSlope(SpriteID palette, const TileInfo *ti, DiagDirection ddir) {
    uint image = (uint)ddir;
    const DrawTileSprites *t = GetStationTileLayout(StationType::Dock, image);
    DrawRailTileSeq(ti, t, TransparencyOption::Invalid, 0, 0, palette);
}

void DrawDockFlat(SpriteID palette, const TileInfo *ti, Axis axis) {
    uint image = GFX_DOCK_BASE_WATER_PART + (uint)axis;
    const DrawTileSprites *t = GetStationTileLayout(StationType::Dock, image);
    DrawRailTileSeq(ti, t, TransparencyOption::Invalid, 0, 0, palette);
}


struct DrawRoadTileStruct {
    uint16_t image;
    uint8_t subcoord_x;
    uint8_t subcoord_y;
};

#include "table/road_land.h"

// copied from road_gui.cpp
static uint GetRoadSpriteOffset(Slope slope, RoadBits bits)
{
    if (slope != SLOPE_FLAT) {
        switch (slope) {
            case SLOPE_NE: return 11;
            case SLOPE_SE: return 12;
            case SLOPE_SW: return 13;
            case SLOPE_NW: return 14;
            default: NOT_REACHED();
        }
    } else {
        static const uint offsets[] = {
            0, 18, 17, 7,
            16, 0, 10, 5,
            15, 8, 1, 4,
            9, 3, 6, 2
        };
        return offsets[bits.base()];
    }
}


void DrawRoadDepot(SpriteID palette, const TileInfo *ti, RoadType roadtype, DiagDirection orientation) {
    if (orientation >= DiagDirection::End) {
        /* Clamp invalid/Auto sentinel so _road_depot[] stays in bounds. */
        orientation = DiagDirection::NE;
    }
    const RoadTypeInfo* rti = GetRoadTypeInfo(roadtype);
    int relocation = GetCustomRoadSprite(rti, INVALID_TILE, RoadSpriteType::Depot);
    bool default_gfx = relocation == 0;
    if (default_gfx) {
        if (rti->flags.Test(RoadTypeFlag::Catenary)) {
            if (false && RoadTypeIsTram(roadtype) && !rti->UsesOverlay()) {
                /* Sprites with track only work for default tram */
                relocation = SPR_TRAMWAY_DEPOT_WITH_TRACK - SPR_ROAD_DEPOT;
                default_gfx = false;
            } else {
                /* Sprites without track are always better, if provided */
                relocation = SPR_TRAMWAY_DEPOT_NO_TRACK - SPR_ROAD_DEPOT;
            }
        }
    } else {
        relocation -= SPR_ROAD_DEPOT;
    }

    const DrawTileSprites *dts = &_road_depot[orientation];
    AddGroundAsSortableSprite(ti, dts->ground.sprite, palette);

    if (default_gfx) {
        uint offset = GetRoadSpriteOffset(SLOPE_FLAT, DiagDirToRoadBits(orientation));
        if (rti->UsesOverlay()) {
            SpriteID ground = GetCustomRoadSprite(rti, INVALID_TILE, RoadSpriteType::Overlay);
            if (ground != 0) AddGroundAsSortableSprite(ti, ground + offset, palette);
        } else if (RoadTypeIsTram(roadtype)) {
            AddGroundAsSortableSprite(ti, SPR_TRAMWAY_OVERLAY + offset, palette);
        }
    }

    DrawRailTileSeq(ti, dts, TransparencyOption::Invalid, relocation, 0, palette);
}

#include "table/station_land.h"

void DrawAirportTile(SpriteID palette, const TileInfo *ti, StationGfx gfx) {
    int32_t total_offset = 0;
    const DrawTileSprites *t = nullptr;
    gfx = GetTranslatedAirportTileID(gfx);
    if (gfx >= NEW_AIRPORTTILE_OFFSET) {
        const AirportTileSpec *ats = AirportTileSpec::Get(gfx);
        if (ats->grf_prop.spritegroups[0] != nullptr /* && DrawNewAirportTile(ti, Station::GetByTile(ti->tile), gfx, ats) */) {
            return;
        }
        /* No sprite group (or no valid one) found, meaning no graphics associated.
         * Use the substitute one instead */
        assert(ats->grf_prop.subst_id != INVALID_AIRPORTTILE);
        gfx = ats->grf_prop.subst_id;
    }
    switch (gfx) {
        case APT_RADAR_GRASS_FENCE_SW:
            t = &_station_display_datas_airport_radar_grass_fence_sw[0];
            break;
        case APT_GRASS_FENCE_NE_FLAG:
            t = &_station_display_datas_airport_flag_grass_fence_ne[0];
            break;
        case APT_RADAR_FENCE_SW:
            t = &_station_display_datas_airport_radar_fence_sw[0];
            break;
        case APT_RADAR_FENCE_NE:
            t = &_station_display_datas_airport_radar_fence_ne[0];
            break;
        case APT_GRASS_FENCE_NE_FLAG_2:
            t = &_station_display_datas_airport_flag_grass_fence_ne_2[0];
            break;
    }
    if (t == nullptr || t->GetSequence().empty()) t = GetStationTileLayout(StationType::Airport, gfx);
    if (t) {
        AddGroundAsSortableSprite(ti, t->ground.sprite, palette);
        DrawRailTileSeq(ti, t, TransparencyOption::Invalid, total_offset, 0, palette);
    }
}

bool is_same_industry(TileIndex tile, Industry* ind) {
    const IndustrySpec *indspec = GetIndustrySpec(ind->type);
    const IndustryTileLayout &layout = indspec->layouts[ind->selected_layout - 1];

    auto diff = TileIndexToTileIndexDiffC(tile, ind->location.tile);
    for (const IndustryTileLayoutTile &it : layout) {
        if (it.ti.x == diff.x && it.ti.y == diff.y) return true;
    }
    return false;
}

uint32_t GetNearbyIndustryTileInformation(uint8_t parameter, TileIndex tile, [[maybe_unused]] Industry* ind, bool signed_offsets, bool grf_version8)
{
    if (parameter != 0) tile = GetNearbyTile(parameter, tile, signed_offsets); // only perform if it is required

    //auto same = is_same_industry(tile, ind);
    auto same = true;
    auto res = GetNearbyTileInformation(tile, grf_version8, 0) | (same ? 1 : 0) << 8;
    if (same) res = (res & 0xFFFFFFU) | ((uint32_t)TileType::Industry << 24);
    return res;
}


struct IndustryTilePreviewScopeResolver : public IndustryTileScopeResolver {
    IndustryTilePreviewScopeResolver(ResolverObject &ro, Industry *industry, TileIndex tile)
        : IndustryTileScopeResolver{ro, industry, tile} {}

    uint32_t GetRandomBits() const override { return 0; };
    uint32_t GetRandomTriggers() const override { return 0; };

    uint32_t GetVariable(uint16_t variable, uint32_t parameter, GetVariableExtra &extra) const override {
        // Debug(misc, 0, "TILE VAR {:X} requested", variable);
        switch (variable) {
            /* Construction state of the tile: a value between 0 and 3 */
            case 0x40: return INDUSTRY_COMPLETED;

            /* Animation frame. Like house variable 46 but can contain anything 0..FF. */
            case 0x44: return 0;

            /* Land info of nearby tiles */
            case 0x60:
                return citymania::GetNearbyIndustryTileInformation(parameter, this->tile,
                    this->industry, true, this->ro.grffile->grf_version >= 8);

            /* Animation stage of nearby tiles */
            case 0x61: {
                TileIndex tile = GetNearbyTile(parameter, this->tile);
                if (is_same_industry(tile, this->industry)) return 0;
                return UINT_MAX;
            }

            /* Get industry tile ID at offset */
            case 0x62:
                if (is_same_industry(GetNearbyTile(parameter, this->tile), this->industry)) {
                    const IndustrySpec *indspec = GetIndustrySpec(this->industry->type);
                    return indspec->grf_prop.local_id;  // our local id
                };
                return 0xFFFF;  // empty tile

            default:
                return IndustryTileScopeResolver::GetVariable(variable, parameter, extra);
        }
    }
};

struct IndustriesPreviewScopeResolver : public IndustriesScopeResolver {
    IndustriesPreviewScopeResolver(ResolverObject &ro, TileIndex tile, Industry *industry, IndustryType type, uint32_t random_bits = 0)
        : IndustriesScopeResolver{ro, tile, industry, type, random_bits} {}

    uint32_t GetRandomBits() const override { return 0; };
    uint32_t GetRandomTriggers() const override { return 0; };
    uint32_t GetVariable(uint16_t variable, uint32_t parameter, GetVariableExtra &extra) const override {
        // Debug(misc, 0, "UNDUSTRY VAR {:X} requested", variable);
        return IndustriesScopeResolver::GetVariable(variable, parameter, extra);
    }
};

static const GRFFile *GetIndTileGrffile(IndustryGfx gfx)
{
    const IndustryTileSpec *its = GetIndustryTileSpec(gfx);
    return (its != nullptr) ? its->grf_prop.grffile : nullptr;
}

struct IndustryTilePreviewResolverObject : public ResolverObject {
    IndustryTilePreviewScopeResolver indtile_scope; ///< Scope resolver for the industry tile.
    IndustriesPreviewScopeResolver ind_scope;       ///< Scope resolver for the industry owning the tile.
    IndustryGfx gfx;

    IndustryTilePreviewResolverObject(IndustryGfx gfx, TileIndex tile, Industry *indus,
            CallbackID callback = CBID_NO_CALLBACK, uint32_t callback_param1 = 0, uint32_t callback_param2 = 0)
        :ResolverObject(GetIndTileGrffile(gfx), callback, callback_param1, callback_param2),
         indtile_scope(*this, indus, tile),
         ind_scope(*this, tile, indus, indus->type),
         gfx(gfx)
    {
        this->root_spritegroup = GetIndustryTileSpec(gfx)->grf_prop.spritegroups[0];
    }

    ScopeResolver *GetScope(VarSpriteGroupScope scope = VSG_SCOPE_SELF, VarSpriteGroupScopeOffset relative = 0) override {
        // Debug(misc, 0, "Scope requested {} {}", (int)scope, (int)relative);
        switch (scope) {
            case VSG_SCOPE_SELF: return &indtile_scope;
            case VSG_SCOPE_PARENT: return &ind_scope;
            default: return ResolverObject::GetScope(scope, relative);
        }
    }

    GrfSpecFeature GetFeature() const override { return GrfSpecFeature::IndustryTiles; }
    uint32_t GetDebugID() const override { return GetIndustryTileSpec(gfx)->grf_prop.local_id; }
};


void DrawIndustryTile(SpriteID palette, const TileInfo *ti, IndustryType ind_type, uint8_t ind_layout, IndustryGfx gfx, TileIndexDiff tile_diff) {
    const IndustryTileSpec *indts = GetIndustryTileSpec(gfx);
    if (gfx >= NEW_INDUSTRYTILEOFFSET) {
        /* jrpm industries are pool items without a default constructor, so the
         * cmclient newgrf preview path is not portable. Fall back to the
         * substitute tile so the highlight still shows the covered area. */
        if (indts->grf_prop.subst_id != INVALID_INDUSTRYTILE) {
            gfx = indts->grf_prop.subst_id;
            indts = GetIndustryTileSpec(gfx);
        }
    }

    const DrawBuildingsTileStruct *dits = &_industry_draw_tile_data[gfx << 2 | INDUSTRY_COMPLETED];

    SpriteID image = dits->ground.sprite;

    /* DrawFoundation() modifies ti->z and ti->tileh */
    // if (ti->tileh != SLOPE_FLAT) DrawFoundation(ti, Foundation::Leveled);

    /* If the ground sprite is the default flat water sprite, draw also canal/river borders.
     * Do not do this if the tile's WaterClass is 'land'. */
    // if (image == SPR_FLAT_WATER_TILE && IsTileOnWater(ti->tile)) {
    //     DrawWaterClassGround(ti);
    // } else {
        DrawGroundSprite(image, GroundSpritePaletteTransform(image, palette, PALETTE_RECOLOUR_START));
    // }

    /* Add industry on top of the ground? */
    image = dits->building.sprite;
    if (image != 0) {
        AddSortableSpriteToDraw(image, palette, *ti, *dits, false);
    }
}

enum SignalOffsets {  // from rail_cmd.cpp
    SIGNAL_TO_SOUTHWEST,
    SIGNAL_TO_NORTHEAST,
    SIGNAL_TO_SOUTHEAST,
    SIGNAL_TO_NORTHWEST,
    SIGNAL_TO_EAST,
    SIGNAL_TO_WEST,
    SIGNAL_TO_SOUTH,
    SIGNAL_TO_NORTH,
};

/**
 * copied from rail_cmd.cpp
 * Get surface height in point (x,y)
 * On tiles with halftile foundations move (x,y) to a safe point wrt. track
 */
static uint GetSaveSlopeZ(uint x, uint y, Track track)
{
    switch (track) {
        case TRACK_UPPER: x &= ~0xF; y &= ~0xF; break;
        case TRACK_LOWER: x |=  0xF; y |=  0xF; break;
        case TRACK_LEFT:  x |=  0xF; y &= ~0xF; break;
        case TRACK_RIGHT: x &= ~0xF; y |=  0xF; break;
        default: break;
    }
    return GetSlopePixelZ(x, y);
}

void DrawSignal(SpriteID palette, const TileInfo *ti, RailType railtype, uint pos, SignalType type, SignalVariant variant) {
    // reference: DraawSingleSignal in rail_cmd.cpp
    bool side;
    switch (to_underlying(_settings_game.construction.train_signal_side)) {
        case 0:  side = false;                                 break; // left
        case 2:  side = true;                                  break; // right
        default: side = to_underlying(_settings_game.vehicle.road_side) != 0; break; // driving side
    }
    static const Point SignalPositions[2][12] = {
        { // Signals on the left side
        /*  LEFT      LEFT      RIGHT     RIGHT     UPPER     UPPER */
            { 8,  5}, {14,  1}, { 1, 14}, { 9, 11}, { 1,  0}, { 3, 10},
        /*  LOWER     LOWER     X         X         Y         Y     */
            {11,  4}, {14, 14}, {11,  3}, { 4, 13}, { 3,  4}, {11, 13}
        }, { // Signals on the right side
        /*  LEFT      LEFT      RIGHT     RIGHT     UPPER     UPPER */
            {14,  1}, {12, 10}, { 4,  6}, { 1, 14}, {10,  4}, { 0,  1},
        /*  LOWER     LOWER     X         X         Y         Y     */
            {14, 14}, { 5, 12}, {11, 13}, { 4,  3}, {13,  4}, { 3, 11}
        }
    };

    uint x = TileX(ti->tile) * TILE_SIZE + SignalPositions[side][pos].x;
    uint y = TileY(ti->tile) * TILE_SIZE + SignalPositions[side][pos].y;

    static const Track pos_track[] = {
        TRACK_LEFT, TRACK_LEFT, TRACK_RIGHT, TRACK_RIGHT,
        TRACK_UPPER, TRACK_UPPER, TRACK_LOWER, TRACK_LOWER,
        TRACK_X, TRACK_X, TRACK_Y, TRACK_Y,
    };
    static const SignalOffsets pos_offset[] = {
        SIGNAL_TO_NORTH, SIGNAL_TO_SOUTH, SIGNAL_TO_NORTH, SIGNAL_TO_SOUTH,
        SIGNAL_TO_WEST, SIGNAL_TO_EAST, SIGNAL_TO_WEST, SIGNAL_TO_EAST,
        SIGNAL_TO_SOUTHWEST, SIGNAL_TO_NORTHEAST, SIGNAL_TO_SOUTHEAST, SIGNAL_TO_NORTHWEST,
    };

    auto track = pos_track[pos];
    auto image = pos_offset[pos];
    static const SignalState condition = SignalState::Green;

    auto rti = GetRailTypeInfo(railtype);
    SpriteID sprite = GetCustomSignalSprite(rti, ti->tile, type, variant, to_underlying(condition), {CSSC_GUI}, 0).sprite.sprite;
    if (sprite != 0) {
        sprite += image;
    } else {
        /* Normal electric signals are stored in a different sprite block than all other signals. */
        sprite = (type == SignalType::Block && variant == SignalVariant::Electric) ? SPR_ORIGINAL_SIGNALS_BASE : SPR_SIGNALS_BASE - 16;
        sprite += to_underlying(type) * 16 + to_underlying(variant) * 64 + image * 2 + to_underlying(condition) + (to_underlying(type) > 3 ? 64 : 0);
    }

    AddSortableSpriteToDraw(sprite, palette, x, y, GetSaveSlopeZ(x, y, track), {{}, {1, 1, BB_HEIGHT_UNDER_BRIDGE}, {}});
}

// copied from tunnelbridge_cmd.cpp
static inline std::span<const PalSpriteID> GetBridgeSpriteTable(int index, BridgePieces table)
{
    const BridgeSpec *bridge = GetBridgeSpec(index);
    assert(table < NUM_BRIDGE_PIECES);
    if (table < bridge->sprite_table.size() && !bridge->sprite_table[table].empty()) return bridge->sprite_table[table];

    return _bridge_sprite_table[index][table];
}

void DrawBridgeHead(SpriteID palette, const TileInfo *ti, RailType railtype, DiagDirection ddir, BridgeType type) {
    auto rti = GetRailTypeInfo(railtype);
    int base_offset = rti->bridge_offset;
    const PalSpriteID *psid;

    /* HACK Wizardry to convert the bridge ramp direction into a sprite offset */
    base_offset += (6 - to_underlying(ddir)) % 4;

    /* Table number BRIDGE_PIECE_HEAD always refers to the bridge heads for any bridge type */
    if (ti->tileh == SLOPE_FLAT) base_offset += 4; // sloped bridge head
    psid = &GetBridgeSpriteTable(type, BRIDGE_PIECE_HEAD)[base_offset];

    AddSortableSpriteToDraw(psid->sprite, palette, ti->x, ti->y, ti->z, {{}, {16, 16, (uint8_t)(ti->tileh == SLOPE_FLAT ? 0 : 8)}, {}});
    // DrawAutorailSelection(ti, (ddir == DiagDirection::SW || ddir == DiagDirection::NE ? HT_DIR_X : HT_DIR_Y), PAL_NONE);
}

void DrawTunnelHead(SpriteID palette, const TileInfo *ti, RailType railtype, DiagDirection ddir) {
    auto rti = GetRailTypeInfo(railtype);

    SpriteID image;
    SpriteID railtype_overlay = 0;

    image = rti->base_sprites.tunnel;
    if (rti->UsesOverlay()) {
        /* Check if the railtype has custom tunnel portals. */
        railtype_overlay = GetCustomRailSprite(rti, ti->tile, RailSpriteType::TunnelPortal);
        if (railtype_overlay != 0) image = SPR_RAILTYPE_TUNNEL_BASE; // Draw blank grass tunnel base.
    }

    image += to_underlying(ddir) * 2;
    AddSortableSpriteToDraw(image, palette, ti->x, ti->y, ti->z, {{}, {16, 16, 0}, {}});
}

void DrawSelectionPoint(SpriteID palette, const TileInfo *ti) {
    int z = 0;
    FoundationPart foundation_part = FOUNDATION_PART_NORMAL;
    if (ti->tileh & SLOPE_N) {
        z += TILE_HEIGHT;
        if (RemoveHalftileSlope(ti->tileh) == SLOPE_STEEP_N) z += TILE_HEIGHT;
    }
    if (IsHalftileSlope(ti->tileh)) {
        Corner halftile_corner = GetHalftileSlopeCorner(ti->tileh);
        if ((halftile_corner == CORNER_W) || (halftile_corner == CORNER_E)) z += TILE_HEIGHT;
        if (halftile_corner != CORNER_S) {
            foundation_part = FOUNDATION_PART_HALFTILE;
            if (IsSteepSlope(ti->tileh)) z -= TILE_HEIGHT;
        }
    }
    DrawSelectionSprite(SPR_DOT, palette, ti, z, static_cast<FoundationPart>(foundation_part));
}

void DrawBorderSprites(const TileInfo *ti, ZoningBorder border, SpriteID color) {
    auto b = (uint8_t)border & 15;
    auto tile_sprite = SPR_DOT + _tileh_to_sprite[ti->tileh] * 19;
    if (b) {
        DrawSelectionSprite(tile_sprite + b - 1, color, ti, 7, FOUNDATION_PART_NORMAL);
    }
    if (border & ZoningBorder::TOP_CORNER)
        DrawSelectionSprite(tile_sprite + 15, color, ti, 7, FOUNDATION_PART_NORMAL);
    if (border & ZoningBorder::RIGHT_CORNER)
        DrawSelectionSprite(tile_sprite + 16, color, ti, 7, FOUNDATION_PART_NORMAL);
    if (border & ZoningBorder::BOTTOM_CORNER)
        DrawSelectionSprite(tile_sprite + 17, color, ti, 7, FOUNDATION_PART_NORMAL);
    if (border & ZoningBorder::LEFT_CORNER)
        DrawSelectionSprite(tile_sprite + 18, color, ti, 7, FOUNDATION_PART_NORMAL);
}

TileHighlight ObjectHighlight::GetTileHighlight(const TileInfo *ti) {
    TileHighlight th;
    auto range = this->tiles.equal_range(ti->tile);
    for (auto t = range.first; t != range.second; t++) {
        t->second.SetTileHighlight(th, ti);
    }
    return th;
}

void ObjectHighlight::AddToHighlightMap(HighlightMap &hlmap, SpriteID palette) {
    // TODO remove the need to convert (maybe replace HighlightMap with multimap?)
    for (auto &[tile, oth] : this->tiles) {
        auto othp = oth;
        othp.palette = palette;
        hlmap.Add(tile, othp);
    }
}

std::optional<TileArea> ObjectHighlight::GetArea() {
    switch(this->type) {
        case Type::NONE:
        case Type::BLUEPRINT:
        case Type::POLYRAIL:
        case Type::INDUSTRY:
            return std::nullopt;
        case Type::RAIL_DEPOT:
        case Type::ROAD_DEPOT:
            return TileArea{this->tile, 1, 1};
        case Type::RAIL_STATION:
        case Type::ROAD_STOP:
            return TileArea{this->tile, this->w, this->h};
        case Type::AIRPORT: {
            const AirportSpec *as = AirportSpec::Get(this->airport_type);
            if (!as->IsAvailable() || this->airport_layout >= as->layouts.size()) return std::nullopt;
            return TileArea{this->tile, as->layouts[this->airport_layout].size_x, as->layouts[this->airport_layout].size_y};
        }
        case Type::DOCK: {
            if (this->ddir == DiagDirection::Invalid) return std::nullopt;
            return TileArea{this->tile, TileAddByDiagDir(this->tile, this->ddir)};
        }
        default:
            NOT_REACHED();
    }
}

static void DrawObjectTileHighlight(const TileInfo *ti, const ObjectTileHighlight &oth) {
    switch (oth.type) {
        case ObjectTileHighlight::Type::RAIL_DEPOT:
            DrawTrainDepotSprite(oth.palette, ti, _cur_railtype, oth.u.rail.depot.ddir);
            break;
        case ObjectTileHighlight::Type::RAIL_TRACK: {
            DrawAutorailSelection(ti, (HighLightStyle)oth.u.rail.track, GetSelectionColourByTint(oth.palette));
            break;
        }
        case ObjectTileHighlight::Type::RAIL_STATION: {
            TileArea area{TileIndex(oth.u.rail.station.base_tile), oth.u.rail.station.w, oth.u.rail.station.h};
            DrawTrainStationSprite(
                oth.palette,
                ti,
                _cur_railtype,
                oth.u.rail.station.axis,
                oth.u.rail.station.section,
                oth.u.rail.station.spec_class,
                oth.u.rail.station.spec_index,
                area
            );
            break;
        }
        case ObjectTileHighlight::Type::RAIL_SIGNAL:
            DrawSignal(oth.palette, ti, _cur_railtype, oth.u.rail.signal.pos, oth.u.rail.signal.type, oth.u.rail.signal.variant);
            break;
        case ObjectTileHighlight::Type::RAIL_BRIDGE_HEAD:
            DrawBridgeHead(oth.palette, ti, _cur_railtype, oth.u.rail.bridge_head.ddir, oth.u.rail.bridge_head.type);
            break;
        case ObjectTileHighlight::Type::RAIL_TUNNEL_HEAD:
            DrawTunnelHead(oth.palette, ti, _cur_railtype, oth.u.rail.tunnel_head.ddir);
            break;
        case ObjectTileHighlight::Type::ROAD_STOP:
            DrawRoadStop(oth.palette, ti, oth.u.road.stop.roadtype, oth.u.road.stop.ddir, oth.u.road.stop.is_truck, oth.u.road.stop.spec_class, oth.u.road.stop.spec_index);
            break;
        case ObjectTileHighlight::Type::ROAD_DEPOT:
            DrawRoadDepot(oth.palette, ti, oth.u.road.depot.roadtype, oth.u.road.depot.ddir);
            break;
        case ObjectTileHighlight::Type::DOCK_SLOPE:
            DrawDockSlope(oth.palette, ti, oth.u.dock_slope.ddir);
            break;
        case ObjectTileHighlight::Type::DOCK_FLAT:
            DrawDockFlat(oth.palette, ti, oth.u.dock_flat.axis);
            break;
        case ObjectTileHighlight::Type::AIRPORT_TILE:
            DrawAirportTile(oth.palette, ti, oth.u.airport_tile.gfx);
            break;
        case ObjectTileHighlight::Type::INDUSTRY_TILE:
            DrawIndustryTile(oth.palette, ti, oth.u.industry_tile.ind_type, oth.u.industry_tile.ind_layout, oth.u.industry_tile.gfx, oth.u.industry_tile.tile_diff);
            break;
        case ObjectTileHighlight::Type::POINT:
            DrawSelectionPoint(oth.palette, ti);
            break;
        case ObjectTileHighlight::Type::RECT:
            DrawTileSelectionRect(ti, oth.palette);
            break;
        case ObjectTileHighlight::Type::NUMBERED_RECT: {
            // TODO NUMBERED_RECT should not be used atm anyway
            // DrawTileSelectionRect(ti, oth.palette);
            // auto string_id = oth.u.numbered_rect.number ? CM_STR_LAYOUT_NUM : CM_STR_LAYOUT_RANDOM;
            // SetDParam(0, oth.u.numbered_rect.number);
            // std::string buffer = GetString(string_id);
            // auto bb = GetStringBoundingBox(buffer);
            // ViewportSign sign;
            // sign.width_normal = WidgetDimensions::scaled.fullbevel.left + Align(bb.width, 2) + WidgetDimensions::scaled.fullbevel.right;
            // Point pt = RemapCoords2(TileX(ti->tile) * TILE_SIZE + TILE_SIZE / 2, TileY(ti->tile) * TILE_SIZE + TILE_SIZE / 2);
            // sign.center = pt.x;
            // sign.top = pt.y - bb.height / 2;

            // ViewportAddString(_cur_dpi, ZOOM_LVL_OUT_8X, &sign,
            //                   string_id, STR_NULL, STR_NULL, oth.u.numbered_rect.number, 0, COLOUR_WHITE);
            break;
        }
        case ObjectTileHighlight::Type::BORDER: {
            DrawBorderSprites(ti, oth.u.border, oth.palette);
            break;
        }
        default:
            break;
    }
}

void ObjectHighlight::Draw(const TileInfo *ti) {
    auto range = this->tiles.equal_range(ti->tile);
    for (auto t = range.first; t != range.second; t++) {
        DrawObjectTileHighlight(ti, t->second);
    }
    // fprintf(stderr, "TILEH DRAW %d %d %d\n", ti->tile, (int)i, (int)this->tiles.size());
}

bool Intersects(Point tl, Point br, int left, int top, int right, int bottom) {
    return (
        right >= tl.x &&
        left <= br.x &&
        bottom >= tl.y &&
        top <= br.y
    );
}

bool Intersects(const Rect &rect, int left, int top, int right, int bottom) {
    return (
        right >= rect.left &&
        left <= rect.right &&
        bottom >= rect.top &&
        top <= rect.bottom
    );
}


void ObjectHighlight::DrawSelectionOverlay([[maybe_unused]] DrawPixelInfo *dpi) {
    for (auto &s : this->sprites) {
        DrawSprite(s.sprite_id, s.palette_id, s.pt.x, s.pt.y);
    }
    // for (auto &[tile, oth] : this->tiles) {
    //     switch (oth.type) {
    //         case ObjectTileHighlight::Type::RAIL_TRACK: {
    //             if (oth.z != -1) {
    //                 auto h = oth.z * TILE_HEIGHT + 7 /* z_offset */;
    //                 auto tx = TileX(tile) * TILE_SIZE, ty = TileY(tile) * TILE_SIZE;
    //                 auto tl = RemapCoords(tx + TILE_SIZE / 2, ty - TILE_SIZE / 2, h);
    //                 auto br = RemapCoords(tx + TILE_SIZE / 2, ty + 3 * TILE_SIZE / 2, h);
    //                 if (Intersects(tl, br, dpi.left, dpi.top, dpi.left + dpi.width, dpi.top + dpi.height)) {
    //                     auto sprite = SPR_AUTORAIL_BASE + _autorail_slope_sprite_offsets[0][oth.u.rail.track];
    //                     auto p = RemapCoords(tx, ty, h);
    //                     DrawSpriteViewport(sprite, oth.palette, p.x, p.y);
    //                 }
    //             }
    //             break;
    //         }
    //         default:
    //             break;
    //     }
    // }
}

void ObjectHighlight::DrawOverlay([[maybe_unused]] DrawPixelInfo *dpi) {
    if (!this->cost.Succeeded()) return;
}

static uint8_t _industry_highlight_hash = 0;

void UpdateIndustryHighlight() {
    _industry_highlight_hash++;
}

/* Simplified stand-in: jrpm has no public CanBuildIndustryOnTile; the
 * highlight only needs a coarse "can fund here" hint. */
static bool CanBuildIndustryOnTile(IndustryType type, TileIndex tile)
{
    return IsTileType(tile, TileType::Clear);
}

bool CanBuildIndustryOnTileCached(IndustryType type, TileIndex tile) {
    // if (_mz[tile].industry_fund_type != type || !_mz[tile].industry_fund_result) {
    if (_mz[tile.base()].industry_fund_update != _industry_highlight_hash || !_mz[tile.base()].industry_fund_result) {
        bool res = CanBuildIndustryOnTile(type, tile);
        // _mz[tile].industry_fund_type = type;
        _mz[tile.base()].industry_fund_update = _industry_highlight_hash;
        _mz[tile.base()].industry_fund_result = res ? 2 : 1;
        return res;
    }
    return (_mz[tile.base()].industry_fund_result == 2);
}

SpriteID GetIndustryZoningPalette(TileIndex tile) {
    if (!IsTileType(tile, TileType::Industry)) return PAL_NONE;
    Industry *ind = Industry::GetByTile(tile);
    auto n_produced = 0;
    auto n_serviced = 0;
    for (uint8_t pc_idx = 0; pc_idx < ind->produced_cargo_count; pc_idx++) {
        auto &pc = ind->produced[pc_idx];
        if (pc.history[LAST_MONTH].production == 0 && pc.history[THIS_MONTH].production == 0) continue;
        n_produced++;
        if (pc.history[LAST_MONTH].transported > 0 || pc.history[THIS_MONTH].transported > 0)
            n_serviced++;
    }
    if (n_serviced < n_produced)
        return (n_serviced == 0 ? PALETTE_TO_RED : PALETTE_TO_ORANGE);
    return PAL_NONE;
}

static void SetStationSelectionHighlight(const TileInfo *ti, TileHighlight &th) {
    bool draw_selection = ((_thd.drawstyle & HT_DRAG_MASK) == HT_RECT && _thd.outersize.x > 0);
    const Station *highlight_station = _viewport_highlight_station;

    if (draw_selection) {
        // const SpriteID pal[] = {SPR_PALETTE_ZONING_RED, SPR_PALETTE_ZONING_YELLOW, SPR_PALETTE_ZONING_LIGHT_BLUE, SPR_PALETTE_ZONING_GREEN};
        // auto color = pal[(int)_station_building_status];
        // if (_thd.redsq != INVALID_TILE) color = SPR_PALETTE_ZONING_RED;
        if (_thd.redsq != INVALID_TILE) {
            auto b = CalcTileBorders(ti->tile, [](TileIndex t) {
                auto x = TileX(t) * TILE_SIZE, y = TileY(t) * TILE_SIZE;
                return IsInsideSelectedRectangle(x, y);
            });
            if (b.first != ZoningBorder::NONE)
                th.add_border(b.first, CM_SPR_PALETTE_ZONING_RED);
        }
        if (IsInsideSelectedRectangle(TileX(ti->tile) * TILE_SIZE, TileY(ti->tile) * TILE_SIZE)) {
            if (_thd.redsq != INVALID_TILE) {
                th.tint_ground(PALETTE_TO_RED);
                th.set_structure(PALETTE_TO_RED);
            } else {
                th.hide_structure();
            }
            return;
        }
    }

    auto coverage_getter = [draw_selection, highlight_station](TileIndex t) {
        auto x = TileX(t) * TILE_SIZE, y = TileY(t) * TILE_SIZE;
        if (highlight_station && IsTileType(t, TileType::Station) && GetStationIndex(t) == highlight_station->index) return 2;
        if (_settings_client.gui.station_show_coverage && highlight_station && highlight_station->TileIsInCatchment(t)) return 1;
        if (draw_selection && _settings_client.gui.station_show_coverage && IsInsideBS(x, _thd.pos.x + _thd.offs.x, _thd.size.x + _thd.outersize.x) &&
                        IsInsideBS(y, _thd.pos.y + _thd.offs.y, _thd.size.y + _thd.outersize.y)) return 1;
        return 0;
    };
    auto b = CalcTileBorders(ti->tile, coverage_getter);
    if (b.second) {
        const SpriteID pal[] = {PAL_NONE, CM_SPR_PALETTE_ZONING_WHITE, PAL_NONE};
        th.add_border(b.first, pal[b.second]);
        const SpriteID pal2[] = {PAL_NONE, PALETTE_TO_WHITE, PALETTE_TO_BLUE};
        th.tint_all(pal2[b.second]);
    }
}

void CalcCBAcceptanceBorders(TileHighlight &th, TileIndex tile, SpriteID border_pal, SpriteID ground_pal) {
    int tx = TileX(tile), ty = TileY(tile);
    uint16_t radius = 5;
    bool in_zone = false;
    ZoningBorder border = ZoningBorder::NONE;
    _town_kdtree.FindContained(
        (uint16_t)std::max<int>(0, tx - radius),
        (uint16_t)std::max<int>(0, ty - radius),
        (uint16_t)std::min<int>(tx + radius + 1, Map::SizeX()),
        (uint16_t)std::min<int>(ty + radius + 1, Map::SizeY()),
        [tx, ty, radius, &in_zone, &border] (TownID tid) {
            Town *town = Town::GetIfValid(tid);
            if (!town || town->larger_town)
                return;

            int dx = TileX(town->xy) - tx;
            int dy = TileY(town->xy) - ty;
            in_zone = in_zone || (std::max(abs(dx), abs(dy)) <= radius);
            if (dx == radius) border |= ZoningBorder::TOP_RIGHT;
            if (dx == -radius) border |= ZoningBorder::BOTTOM_LEFT;
            if (dy == radius) border |= ZoningBorder::TOP_LEFT;
            if (dy == -radius) border |= ZoningBorder::BOTTOM_RIGHT;
        }
    );
    th.add_border(border, border_pal);
    if (in_zone) th.tint_all(ground_pal);
}


void AddTownCBLimitBorder(TileIndex tile, const Town *town, ZoningBorder &border, bool &in_zone) {
    auto sq = town->cache.squared_town_zone_radius[0] + 30;
    auto x = CalcTileBorders(tile, [town, sq] (TileIndex tile) {
        return DistanceSquare(tile, town->xy) <= sq ? 1 : 0;
    });
    border |= x.first;
    in_zone = in_zone || x.second;
}

void CalcCBTownLimitBorder(TileHighlight &th, TileIndex tile, SpriteID border_pal, SpriteID ground_pal) {
    auto n = Town::GetNumItems();
    uint32_t sq = 0;
    uint i = 0;
    ZoningBorder border = ZoningBorder::NONE;
    bool in_zone = false;
    for (auto &p : _town_cache) {
        sq = p.second->cache.squared_town_zone_radius[0] + 30;
        if (4 * sq * n < Map::Size() * i) break;
        AddTownCBLimitBorder(tile, p.second, border, in_zone);
        i++;
    }
    uint radius = IntSqrt(sq);
    int tx = TileX(tile), ty = TileY(tile);
    _town_kdtree.FindContained(
        (uint16_t)std::max<int>(0, tx - radius),
        (uint16_t)std::max<int>(0, ty - radius),
        (uint16_t)std::min<int>(tx + radius + 1, Map::SizeX()),
        (uint16_t)std::min<int>(ty + radius + 1, Map::SizeY()),
        [tile, &in_zone, &border] (TownID tid) {
            Town *town = Town::GetIfValid(tid);
            if (!town || town->larger_town)
                return;
            AddTownCBLimitBorder(tile, town, border, in_zone);
        }
    );
    th.add_border(border, border_pal);
    if (in_zone) th.tint_all(ground_pal);
}

Zoning _zoning = {EvaluationMode::CHECKNOTHING, EvaluationMode::CHECKNOTHING};



ObjectHighlight _cm_active_object;
/* Holds the last built & UpdateTiles()ed active object (cmclient stores this
 * in _thd.cm); used by UpdateActiveTool to populate _at.tiles for rendering. */
ObjectHighlight _cm_prev_object;

TileHighlight GetTileHighlight(const TileInfo *ti, TileType tile_type) {
    TileHighlight th;

    th = TileHighlight{};
    if (ti->tile == INVALID_TILE || tile_type == TileType::Void) return th;
    /* Town zoning modes are ported with batch 4 (cm_zoning); currently unused. */
    return th;
}

void DrawTileZoning(const TileInfo *ti, const TileHighlight &th, TileType tile_type) {
    if (ti->tile == INVALID_TILE || tile_type == TileType::Void) return;
    for (uint i = 0; i < th.border_count; i++)
        DrawBorderSprites(ti, th.border[i], th.border_colour[i]);
    if (th.sprite) {
        DrawSelectionSprite(th.sprite, PAL_NONE, ti, 0, FOUNDATION_PART_NORMAL);
    }
    if (th.selection) {
        DrawBorderSprites(ti, ZoningBorder::FULL, th.selection);
        // DrawSelectionSprite(SPR_SELECT_TILE + _tileh_to_sprite[ti->tileh],
        //                     th.selection, ti, 0, FOUNDATION_PART_NORMAL);
    }
}

bool DrawTileSelection(const TileInfo *ti, [[maybe_unused]] const TileHighlightType &tht) {
    if (ti->tile == INVALID_TILE || IsTileType(ti->tile, TileType::Void)) return false;

    auto hl = _at.tiles.GetForTile(ti->tile);
    if (hl.has_value()) {
        for (auto &oth : hl.value().get()) {
            DrawObjectTileHighlight(ti, oth);
        }
        return true;
    }


    if (_thd.drawstyle == HT_BLUEPRINT_PLACE) return true;

    if (false) {
        return true;
    }

    return false;
}

void DrawSelectionOverlay(DrawPixelInfo *dpi) {
    /* cmclient overlay drawing requires its extended tile-highlight state; skipped. */
}


TileIndex _autodetection_tile = INVALID_TILE;
DiagDirDiff _autodetection_rotation = DiagDirDiff::Same;

static DiagDirDiff GetAutodetectionRotation() {
    auto pt = GetTileBelowCursor();
    auto tile = TileVirtXY(pt.x, pt.y);

    if (tile != _autodetection_tile) {
        _autodetection_tile = tile;
        _autodetection_rotation = DiagDirDiff::Same;
    }

    return _autodetection_rotation;
}

void RotateAutodetection() {
    auto rotation = GetAutodetectionRotation();
    if (rotation == DiagDirDiff::Left90) rotation = DiagDirDiff::Same;
    else rotation = static_cast<DiagDirDiff>((to_underlying(rotation) + 1) & 3);
    _autodetection_rotation = rotation;
    ::UpdateTileSelection();
}

void ResetRotateAutodetection() {
    _autodetection_tile = INVALID_TILE;
    _autodetection_rotation = DiagDirDiff::Same;
}

DiagDirection AddAutodetectionRotation(DiagDirection ddir) {
    if (ddir >= DiagDirection::End) return (DiagDirection)(((to_underlying(ddir) + to_underlying(GetAutodetectionRotation())) % 2) + to_underlying(DiagDirection::End));
    return ChangeDiagDir(ddir, GetAutodetectionRotation());
}

HighLightStyle UpdateTileSelection(HighLightStyle new_drawstyle) {
    _cm_active_object = ObjectHighlight(ObjectHighlight::Type::NONE);
    auto pt = GetTileBelowCursor();
    auto tile = (pt.x == -1 ? INVALID_TILE : TileVirtXY(pt.x, pt.y));
    bool force_new = false;
    // fprintf(stderr, "UPDATE %d %d %d %d\n", tile, _thd.size.x, _thd.size.y, (int)((_thd.place_mode & HT_DRAG_MASK) == HT_RECT));
    if (_thd.place_mode == HT_BLUEPRINT_PLACE) {
        UpdateBlueprintTileSelection(tile);
        new_drawstyle = HT_BLUEPRINT_PLACE;
    } else if (pt.x == -1) {
    } else if (_thd.redsq != INVALID_TILE) {
    } else if (_thd.select_proc == CM_DDSP_FUND_INDUSTRY) {
        _cm_active_object = ObjectHighlight::make_industry(tile, _cm_funding_type, _cm_funding_layout);
        force_new = true;
        new_drawstyle = HT_RECT;
    } else if (_thd.select_proc == CM_DDSP_BUILD_ROAD_DEPOT) {
        auto dir = _road_depot_orientation;
        if (dir >= DiagDirection::End) {
            /* Auto mode / invalid sentinel: clamp to a real direction
             * before the highlight preview indexes _road_depot[]. */
            dir = DiagDirection::NE;
        }
        _cm_active_object = ObjectHighlight::make_road_depot(tile, _cur_roadtype, dir);
        new_drawstyle = HT_RECT;
    } else if (_thd.select_proc == CM_DDSP_BUILD_RAIL_DEPOT) {
        auto dir = _build_depot_direction;
        if (dir >= DiagDirection::End) {
            dir = DiagDirection::NE;
        }
        _cm_active_object = ObjectHighlight::make_rail_depot(tile, dir);
    // } else if (((_thd.place_mode & HT_DRAG_MASK) == HT_RECT || ((_thd.place_mode & HT_DRAG_MASK) == HT_SPECIAL && (_thd.next_drawstyle & HT_DRAG_MASK) == HT_RECT)) && _thd.new_outersize.x > 0 && !_thd.redsq != INVALID_TILE) {  // station
    } else if (_thd.select_proc == CM_DDSP_BUILD_AIRPORT) {
        auto tile = TileXY(_thd.new_pos.x / TILE_SIZE, _thd.new_pos.y / TILE_SIZE);
        if (_selected_airport_index != -1) {
            auto ac = AirportClass::Get(_selected_airport_class);
            auto as = (ac != nullptr ? ac->GetSpec(_selected_airport_index) : nullptr);
            if (as != nullptr) {
                _cm_active_object = ObjectHighlight::make_airport(tile, as->GetIndex(), _selected_airport_layout);
                new_drawstyle = HT_RECT;
            }
        }
    } else if (_thd.select_proc == DDSP_BUILD_STATION || _thd.select_proc == DDSP_BUILD_BUSSTOP
               || _thd.select_proc == DDSP_BUILD_TRUCKSTOP) {  // station / road stop (cmclient port)
        if (_thd.size.x >= (int)TILE_SIZE && _thd.size.y >= (int)TILE_SIZE) {
            const auto start_tile = TileXY(_thd.new_pos.x / TILE_SIZE, _thd.new_pos.y / TILE_SIZE);
            const auto w = _thd.new_size.x / TILE_SIZE;
            const auto h = _thd.new_size.y / TILE_SIZE;
            const TileArea ta(start_tile, w, h);

            /* Object preview sprite (station building or road stop). */
            if (_thd.select_proc == DDSP_BUILD_STATION) {
                _cm_active_object = ObjectHighlight::make_rail_station(
                    start_tile, w, h, _station_gui.axis, _station_gui.sel_class, _station_gui.sel_type);
            } else { /* BUSSTOP / TRUCKSTOP */
                auto ddir = _roadstop_gui.orientation;
                if (pt.x != -1 && ddir == DiagDirection::Invalid) ddir = DiagDirection::NE;
                _cm_active_object = ObjectHighlight::make_road_stop(
                    start_tile, w, h, _cur_roadtype, ddir,
                    _thd.select_proc == DDSP_BUILD_TRUCKSTOP,
                    _roadstop_gui.sel_class, _roadstop_gui.sel_type);
            }

            /* Extended preview: coverage area + cost overlay (cmclient port of
             * cm_station_gui.cpp::PlacementAction::PrepareGUIInfo). */
            if (_thd.select_proc == DDSP_BUILD_STATION) {
                BuildInfoOverlayData overlay;
                HighlightMap hlmap;
                CommandCost cc;
                int rad = (int)CA_UNMODIFIED + _settings_game.station.catchment_increase;
                if (_settings_game.station.modified_catchment) rad = _settings_game.station.catchment_increase;
                TileArea rad_area = ta;
                rad_area.Expand(rad);
                rad_area = ClampToVisibleMap(rad_area);

                /* Cost via DoCommandFlag::QueryCost. */
                cc = Command<Commands::BuildRailStation>::Do(
                    CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildRailStation>()) | DoCommandFlag::QueryCost,
                    start_tile, _cur_railtype, _station_gui.axis,
                    (uint8_t)ta.w, (uint8_t)ta.h,
                    _station_gui.sel_class, _station_gui.sel_type,
                    StationID::Invalid(), false);

                /* Coverage tint (catchment area). */
                for (auto t : rad_area) {
                    hlmap.Add(t, ObjectTileHighlight::make_tint(CM_PALETTE_TINT_WHITE));
                }
                {
                    std::set<TileIndex> cov_set;
                    for (auto t : rad_area) cov_set.insert(t);
                    hlmap.AddTilesBorder(cov_set, CM_PALETTE_TINT_WHITE);
                }
                /* Station building sprite preview on top of the white frame. */
                _cm_active_object.AddToHighlightMap(hlmap, CM_PALETTE_TINT_WHITE);

                /* Cost overlay line. */
                if (cc.GetCost() != 0) {
                    auto err = cc.GetErrorMessage();
                    if (cc.Succeeded()) {
                        overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_COST_OK, cc.GetCost()));
                    } else if (err == STR_ERROR_NOT_ENOUGH_CASH_REQUIRES_CURRENCY) {
                        overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_COST_NO_MONEY, cc.GetCost()));
                    } else {
                        overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_ERROR_UNKNOWN));
                    }
                }

                /* Supplies (cmclient port). */
                {
                    auto production = GetProductionAroundTiles(ta.tile, (int)ta.w, (int)ta.h, rad);
                    bool has_header = false;
                    for (CargoType i : EnumRange(NUM_CARGO)) {
                        if (production[i] == 0) continue;
                        const CargoSpec *cs = CargoSpec::Get(i);
                        if (cs == nullptr) continue;
                        if (!has_header) {
                            overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_STATION_SUPPLIES));
                            has_header = true;
                        }
                        overlay.emplace_back(1, cs->GetCargoIcon(), GetString(CM_STR_BUILD_INFO_OVERLAY_STATION_CARGO, i, production[i] >> 8));
                    }
                }

                /* Accepts (cmclient port). */
                {
                    CargoTypes always_accepted;
                    CargoArray cargoes = GetAcceptanceAroundTiles(ta.tile, (int)ta.w, (int)ta.h, rad).first;
                    std::vector<std::pair<uint, std::string>> cargostr;
                    for (CargoType i : EnumRange(NUM_CARGO)) {
                        if (cargoes[i] > 0) {
                            if (cargoes[i] < 8) {
                                cargostr.emplace_back(2, GetString(CM_STR_BULID_INFO_OVERLAY_ACCEPTS_CARGO_PARTIAL, CargoTypes{}.Set(i), cargoes[i]));
                            } else if (always_accepted.Test(i)) {
                                cargostr.emplace_back(1, GetString(CM_STR_BULID_INFO_OVERLAY_ACCEPTS_CARGO_FULL, CargoTypes{}.Set(i), cargoes[i]));
                            } else {
                                cargostr.emplace_back(0, GetString(CM_STR_BULID_INFO_OVERLAY_ACCEPTS_CARGO, CargoTypes{}.Set(i)));
                            }
                        }
                    }
                    std::stable_sort(cargostr.begin(), cargostr.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
                    std::string cargolist;
                    for (auto &[_, s] : cargostr) {
                        if (!cargolist.empty()) cargolist += ", ";
                        cargolist += s;
                    }
                    if (cargolist.empty()) cargolist = GetString(STR_JUST_NOTHING);
                    overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BULID_INFO_OVERLAY_ACCEPTS, cargolist));
                }

                /* Town + Size (cmclient port). */
                {
                    Town *t = ClosestTownFromTile(ta.tile, _settings_game.economy.dist_local_authority);
                    if (t != nullptr) {
                        auto town_allowed = CheckIfAuthorityAllowsNewStation(ta.tile, {}).Succeeded();
                        auto rating = t->ratings[_current_company];
                        auto dist = DistanceManhattan(t->xy, ta.tile);
                        StringID zone_id;
                        if (dist <= 10) {
                            zone_id = CM_STR_BULID_INFO_OVERLAY_TOWN_S_ADS;
                        } else if (dist <= 15) {
                            zone_id = CM_STR_BULID_INFO_OVERLAY_TOWN_M_ADS;
                        } else if (dist <= 20) {
                            zone_id = CM_STR_BULID_INFO_OVERLAY_TOWN_L_ADS;
                        } else {
                            zone_id = CM_STR_BULID_INFO_OVERLAY_TOWN_NO_ADS;
                        }
                        overlay.emplace_back(0, PAL_NONE, GetString(
                            town_allowed ? CM_STR_BULID_INFO_OVERLAY_TOWN_ALLOWS : CM_STR_BULID_INFO_OVERLAY_TOWN_DENIES,
                            t->index, rating, zone_id));
                    } else {
                        overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BULID_INFO_OVERLAY_TOWN_NONE));
                    }
                    overlay.emplace_back(0, PAL_NONE, GetString(CM_STR_BULID_INFO_OVERLAY_STATION_SIZE, ta.w, ta.h));
                }
                _cm_gui_info = {std::move(hlmap), std::move(overlay), cc};
                _cm_gui_active = true;
            } else {
                _cm_gui_info = {};
                _cm_gui_active = false;
            }
            new_drawstyle = HT_RECT;
        }
        force_new = true;
    }
    if (force_new || _cm_prev_object != _cm_active_object) {
        _cm_prev_object.MarkDirty();
        _cm_prev_object = _cm_active_object;
        _cm_prev_object.UpdateTiles();
        _cm_prev_object.MarkDirty();
    }
    return new_drawstyle;
}

void AllocateZoningMap(uint map_size) {
    _mz = std::make_unique<TileZoning[]>(map_size);
}

uint8_t GetTownZone(Town *town, TileIndex tile) {
    auto dist = DistanceSquare(tile, town->xy);
    if (dist > town->cache.squared_town_zone_radius[(uint8_t)HouseZone::TownEdge])
        return 0;

    uint8_t z = 1;
    for (uint8_t i = (uint8_t)HouseZone::TownOutskirt; i < (uint8_t)HouseZone::TownEnd; i++)
        if (dist < town->cache.squared_town_zone_radius[i])
            z = (uint8_t)i + 1;
        else if (town->cache.squared_town_zone_radius[i] != 0)
            break;
    return z;
}

uint8_t GetAnyTownZone(TileIndex tile) {
    uint8_t next_zone = (uint8_t)HouseZone::TownEdge;
    uint8_t z = 0;

    for (Town *town : Town::Iterate()) {
        uint dist = DistanceSquare(tile, town->xy);
        // town code uses <= for checking town borders (tz0) but < for other zones
        while (next_zone < (uint8_t)HouseZone::TownEnd
            && (town->cache.squared_town_zone_radius[next_zone] == 0
                || dist <= town->cache.squared_town_zone_radius[next_zone] - (next_zone == (uint8_t)HouseZone::TownEdge ? 0 : 1))
        ) {
            if (town->cache.squared_town_zone_radius[next_zone] != 0)  z = (uint8_t)next_zone + 1;
            next_zone++;
        }
    }
    return z;
}

void UpdateTownZoning(Town *town, uint32_t prev_edge) {
    auto edge = town->cache.squared_town_zone_radius[(uint8_t)HouseZone::TownEdge];
    if (prev_edge && edge == prev_edge)
        return;

    auto area = OrthogonalTileArea(town->xy, 1, 1);
    bool recalc;
    if (edge < prev_edge) {
        area.Expand(IntSqrt(prev_edge));
        recalc = true;
    } else {
        area.Expand(IntSqrt(edge));
        recalc = false;
    }
    // TODO mark dirty only if zoning is on
    for(TileIndex tile : area) {
        uint8_t group = GetTownZone(town, tile);

        if (_mz[tile.base()].town_zone != group)
            _mz[tile.base()].industry_fund_result = 0;

        if (_mz[tile.base()].town_zone > group) {
            if (recalc) {
                _mz[tile.base()].town_zone = GetAnyTownZone(tile);
                if (_zoning.outer == citymania::EvaluationMode::CHECKTOWNZONES)
                    MarkTileDirtyByTile(tile);
            }
        } else if (_mz[tile.base()].town_zone < group) {
            _mz[tile.base()].town_zone = group;
            if (_zoning.outer == citymania::EvaluationMode::CHECKTOWNZONES)
                MarkTileDirtyByTile(tile);
        }
    }
}

void UpdateAdvertisementZoning(TileIndex center, uint radius, uint8_t zone) {
    uint16_t x1, y1, x2, y2;
    x1 = (uint16_t)std::max<int>(0, TileX(center) - radius);
    x2 = (uint16_t)std::min<int>(TileX(center) + radius + 1, Map::SizeX());
    y1 = (uint16_t)std::max<int>(0, TileY(center) - radius);
    y2 = (uint16_t)std::min<int>(TileY(center) + radius + 1, Map::SizeY());
    for (uint16_t y = y1; y < y2; y++) {
        for (uint16_t x = x1; x < x2; x++) {
            auto tile = TileXY(x, y);
            if (DistanceManhattan(tile, center) > radius) continue;
            _mz[tile.base()].advertisement_zone = std::max(_mz[tile.base()].advertisement_zone, zone);
        }
    }
}

void UpdateZoningTownHouses(const Town *town, uint32_t old_houses) {
    if (!town->larger_town)
        return;
    _town_cache.erase(std::make_pair(old_houses, town));
    _town_cache.insert(std::make_pair(town->cache.num_houses, town));
}

void InitializeZoningMap() {
    _town_cache.clear();
    for (Town *t : Town::Iterate()) {
        UpdateTownZoning(t, 0);
        UpdateAdvertisementZoning(t->xy, 10, 3);
        UpdateAdvertisementZoning(t->xy, 15, 2);
        UpdateAdvertisementZoning(t->xy, 20, 1);
        UpdateZoningTownHouses(t, 0);
        if (!t->larger_town)
            _town_cache.insert(std::make_pair(t->cache.num_houses, t));
    }
}

std::pair<ZoningBorder, uint8_t> GetTownZoneBorder(TileIndex tile) {
    return CalcTileBorders(tile, [](TileIndex t) { return _mz[t.base()].town_zone; });
}

ZoningBorder GetAnyStationCatchmentBorder(TileIndex tile) {
    ZoningBorder border = ZoningBorder::NONE;
    StationFinder morestations(TileArea(tile, 1, 1));
    for (Station *st: morestations.GetStations()) {
        border |= CalcTileBorders(tile, [st](TileIndex t) {return st->TileIsInCatchment(t) ? 1 : 0; }).first;
    }
    return border;
}

void SetIndustryForbiddenTilesHighlight(IndustryType type) {
    if (false &&
            _industry_forbidden_tiles != type) {
        MarkWholeScreenDirty();
    }
    _industry_forbidden_tiles = type;
    UpdateIndustryHighlight();
}


PaletteID GetTreeShadePal(TileIndex tile) {
    /* Tree shading was driven by a cmclient GUI setting; not ported. */
    return PAL_NONE;
}

ActiveTool _at;

static void ResetVanillaHighlight() {
    if (_thd.window_class != WindowClass::Invalid) {
        /* Undo clicking on button and drag & drop */
        Window *w = _thd.GetCallbackWnd();
        /* Call the abort function, but set the window class to something
         * that will never be used to avoid infinite loops. Setting it to
         * the 'next' window class must not be done because recursion into
         * this function might in some cases reset the newly set object to
         * place or not properly reset the original selection. */
        _thd.window_class = WindowClass::Invalid;
        if (w != nullptr) {
            w->OnPlaceObjectAbort();
            CloseWindowById(WindowClass::ToolTips, 0);
        }
    }

    /* Mark the old selection dirty, in case the selection shape or colour changes */
    if ((_thd.drawstyle & HT_DRAG_MASK) != HT_NONE) SetSelectionTilesDirty();

    SetTileSelectSize(1, 1);

    _thd.redsq = INVALID_TILE;
}

void SetActiveTool(up<Tool> &&tool) {
    ResetVanillaHighlight();
    ResetActiveTool();
    _at.tool = std::move(tool);
}

void ResetActiveTool() {
    for (auto t : _at.tiles.GetAllTiles()) {
        MarkTileDirtyByTile(t);
    }
    _at.tool = nullptr;
    _at.tiles = {};
}

const up<Tool> &GetActiveTool() {
    return _at.tool;
}


void UpdateActiveTool() {
    Point pt = GetTileBelowCursor();
    auto tile = pt.x == -1 ? INVALID_TILE : TileVirtXY(pt.x, pt.y);

    ToolGUIInfo info;
    if (_at.tool != nullptr) {
        _at.tool->Update(pt, tile);
        info = _at.tool->GetGUIInfo();
    } else if (_cm_gui_active) {
        /* jrpm: station placement preview (cmclient port of
         * PlacementAction::PrepareGUIInfo) -- the active object hint
         * plus the catchment coverage tint plus the cost overlay all
         * travel through this single ToolGUIInfo blob. */
        info = _cm_gui_info;
    } else if (_cm_prev_object.type != ObjectHighlight::Type::NONE) {
        /* jrpm: no Tool classes are ported; feed the active object
         * highlight (built & UpdateTiles()ed by UpdateTileSelection)
         * into _at.tiles so DrawTileSelection can render it. */
        HighlightMap hlmap;
        _cm_prev_object.AddToHighlightMap(hlmap, PAL_NONE);
        info = {std::move(hlmap), {}, {}};
    }
    auto [hlmap, overlay_data, cc] = info;
    auto tiles_changed = _at.tiles.UpdateWithMap(hlmap);
    for (auto t : tiles_changed)
        MarkTileDirtyByTile(t);

    if (cc.GetExpensesType() != ExpensesType::Invalid || cc.GetErrorMessage() != INVALID_STRING_ID) {
        /* Add CommandCost info to the build overlay. */
        auto err = cc.GetErrorMessage();
        if (cc.Succeeded()) {
            auto money = cc.GetCost();
            if (money != 0) {
                overlay_data.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_COST_OK, money));
            }
        } else if (err == STR_ERROR_NOT_ENOUGH_CASH_REQUIRES_CURRENCY) {
            overlay_data.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_COST_NO_MONEY, cc.GetCost()));
        } else {
            EncodedString error = std::move(cc.GetEncodedMessage());
            if (error.empty()) error = GetEncodedStringIfValid(err);

            if (!error.empty()) overlay_data.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_ERROR, error.GetDecodedString()));
            auto extra_msg = cc.GetExtraErrorMessage();
            if (extra_msg != INVALID_STRING_ID) {
                overlay_data.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_ERROR, extra_msg));
            }

            if (extra_msg == INVALID_STRING_ID && error.empty()) {
                overlay_data.emplace_back(0, PAL_NONE, GetString(CM_STR_BUILD_INFO_OVERLAY_ERROR_UNKNOWN));
            }
        }
    }

    /* Show the build info overlay next to the cursor. */
    if (overlay_data.size() > 0) {
        auto w = FindWindowFromPt(_cursor.pos.x, _cursor.pos.y);
        if (w == nullptr) { HideBuildInfoOverlay(); return; }
        auto vp = IsPtInWindowViewport(w, _cursor.pos.x, _cursor.pos.y);
        if (vp == nullptr) { HideBuildInfoOverlay(); return; }
        Point pto = RemapCoords2(TileX(tile) * TILE_SIZE, TileY(tile) * TILE_SIZE);
        pto.x = UnScaleByZoom(pto.x - vp->virtual_left, vp->zoom) + vp->left;
        pto.y = UnScaleByZoom(pto.y - vp->virtual_top, vp->zoom) + vp->top;
        ShowBuildInfoOverlay(pto.x, pto.y, overlay_data);
    } else {
        HideBuildInfoOverlay();
    }
}

bool _prev_left_button_down = false;
// const Window *_click_window = nullptr;
bool _keep_mouse_click = false;
// const Window *_keep_mouse_window;

bool HandleMouseMove() {
    bool changed = _left_button_down != _prev_left_button_down;
    _prev_left_button_down = _left_button_down;
    // Window *w = FindWindowFromPt(_cursor.pos.x, _cursor.pos.y);
    bool released = !_left_button_down && changed && _keep_mouse_click;
    if (!_left_button_down) _keep_mouse_click = false;

    if (_at.tool == nullptr) return false;
    // Viewport *vp = IsPtInWindowViewport(w, );

    auto pt = GetTileBelowCursor();
    if (pt.x == -1) return false;
    auto tile = pt.x == -1 ? INVALID_TILE : TileVirtXY(pt.x, pt.y);
    _at.tool->Update(pt, tile);
    _at.tool->HandleMouseMove();
    if (_left_button_down) {
        if (changed && _at.tool->HandleMousePress()) {
            _keep_mouse_click = true;
        }
        if (_keep_mouse_click) return true;
    }
    if (released) {
        _at.tool->HandleMouseRelease();
    }
    return false;
}

bool HandleMouseClick(Viewport *vp, bool double_click) {
    if (_at.tool == nullptr) return false;
    auto pt = GetTileBelowCursor();
    auto tile = pt.x == -1 ? INVALID_TILE : TileVirtXY(pt.x, pt.y);
    _at.tool->Update(pt, tile);
    return _at.tool->HandleMouseClick(vp, pt, tile, double_click);
}

bool HandlePlacePushButton(Window *w, WidgetID widget, up<Tool> tool) {
    if (w->IsWidgetDisabled(widget)) return false;

    if (_settings_client.sound.click_beep) SndPlayFx(SND_15_BEEP);
    w->SetDirty();

    if (w->IsWidgetLowered(widget)) {
        ResetObjectToPlace();
        return false;
    }

    auto icon = tool->GetCursor();
    if ((icon & ANIMCURSOR_FLAG) != 0) {
        SetAnimatedMouseCursor(_animcursors[icon & ~ANIMCURSOR_FLAG]);
    } else {
        SetMouseCursor(icon, PAL_NONE);
    }
    citymania::SetActiveTool(std::move(tool));
    _thd.window_class = w->window_class;
    _thd.window_number = w->window_number;
    w->LowerWidget(widget);

    return true;
}

}  // namespace citymania
