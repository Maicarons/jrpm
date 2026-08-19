#include "stdafx.h"
#include "settings_type.h"

#include "cm_blueprint.hpp"


#include "console_func.h"
#include "console_internal.h"
#include "rail_gui.h"
#include "cm_highlight_type.hpp"
#include "command_func.h"
#include "core/string_consumer.hpp"
#include "tilehighlight_func.h"
#include "command_type.h"
#include "rail_cmd.h"
#include "tunnelbridge_cmd.h"
#include "station_cmd.h"
#include "error.h"
#include "debug.h"
#include "direction_type.h"

#include "rail_map.h"
#include "station_cmd.h"
#include "station_map.h"
#include "station_base.h"
#include "strings_func.h"
#include "table/strings.h"
#include "newgrf_station.h"
#include "tilearea_type.h"
#include "tile_map.h"
#include "tunnelbridge_map.h"
#include "network/network.h"

#include <algorithm>
#include <map>
#include <vector>

extern TileHighlightData _thd;
extern RailType _cur_railtype;

extern StationPickerSelection _station_gui; ///< Settings of the station picker.

/* Pending station sign builds: the 1x1 sign tile is posted first (with
 * CommandCallback::BlueprintStation); when it completes, the station parts
 * of the same blueprint item are posted joined to the newly created station,
 * and the sign tile is removed if the original blueprint had no sign part.
 * Mirrors cmclient's BuildBlueprint with_callback chain. */
namespace citymania {
struct PendingBlueprintStation {
	sp<Blueprint> blueprint;
	TileIndex start;
	TileIndex sign_tile;
	bool sign_part;
	StationID sid;
};
static std::vector<PendingBlueprintStation> _pending_blueprint_stations;
}  // namespace citymania

/** Command callback for a blueprint station sign (BuildRailStation). */
void CcBlueprintStation(const CommandCost &result, TileIndex tile, RailType railtype, Axis axis, uint8_t numtracks, uint8_t plat_len, StationClassID spec_class, uint16_t spec_index, StationID station_to_join, bool adjacent)
{
	using namespace citymania;

	/* Find the pending blueprint station that owns this sign tile. */
	auto it = std::find_if(_pending_blueprint_stations.begin(), _pending_blueprint_stations.end(), [tile](const PendingBlueprintStation &p) {
		return p.sign_tile == tile;
	});
	if (it == _pending_blueprint_stations.end()) return;

	PendingBlueprintStation pending = *it;
	_pending_blueprint_stations.erase(it);

	if (!result.Succeeded()) return;

	/* The newly created station is the one which owns the sign tile. */
	StationID station_id = GetStationIndex(tile);

	/* Post all station parts of this blueprint joined to the new station. */
	for (const auto &item : pending.blueprint->items) {
		if (item.type != Blueprint::Item::Type::RAIL_STATION_PART) continue;
		if (item.u.rail.station_part.id != pending.sid) continue;
		Command<Commands::BuildRailStation>::Post(STR_ERROR_CAN_T_BUILD_RAILROAD_STATION, CommandCallback::None,
			AddTileIndexDiffCWrap(pending.start, item.tdiff), _cur_railtype, item.u.rail.station_part.axis,
			item.u.rail.station_part.numtracks, item.u.rail.station_part.plat_len,
			_station_gui.sel_class, _station_gui.sel_type, station_id, true);
	}

	/* If the original station had no sign part, remove the sign tile. */
	if (!pending.sign_part) {
		Command<Commands::RemoveFromRailStation>::Post(tile, (TileIndex)0, false);
	}
}

namespace citymania {
extern ObjectHighlight _cm_active_object;

/* Local replacement for cmclient's IterateStation (cm_station_gui). */
template <typename Tfunc>
static void IterateStation(TileIndex tile, Axis axis, uint numtracks, uint plat_len, Tfunc &&func)
{
	uint w = axis == Axis::X ? plat_len : numtracks;
	uint h = axis == Axis::X ? numtracks : plat_len;
	uint platform = 0, position = 0;
	for (uint y = 0; y < h; y++) {
		for (uint x = 0; x < w; x++) {
			func(tile + TileDiffXY(x, y), platform, position);
		}
		platform++;
		position = 0;
	}
}

/* A blueprint item compiles to a jrpm command closure. */
using BlueprintCmdClosure = std::function<void(TileIndex start)>;

/* Signal position -> number of direction cycles (cmclient cm_blueprint.cpp). */
static const uint SIGNAL_POS_NUM[] = {
	1, 0, 1, 0,
	0, 1, 0, 1,
	0, 1, 0, 1,
};

std::pair<TileIndex, sp<Blueprint>> _active_blueprint = std::make_pair(INVALID_TILE, nullptr);
const size_t MAX_BLUEPRINT_SLOTS = 16;

sp<Blueprint> _blueprint_slots[16] = {};

TileIndexDiffC operator+(const TileIndexDiffC &a, const TileIndexDiffC &b) {
    return TileIndexDiffC{(int16_t)(a.x + b.x), (int16_t)(a.y + b.y)};
}

bool operator==(const TileIndexDiffC &a, const TileIndexDiffC &b) {
    return a.x == b.x && a.y == b.y;
}

bool operator!=(const TileIndexDiffC &a, const TileIndexDiffC &b) {
    return a.x != b.x || a.y != b.y;
}

void Blueprint::Add(TileIndex source_tile, Blueprint::Item item) {
    this->items.push_back(item);
    switch (item.type) {
        case Item::Type::RAIL_TRACK: {
            auto tdir = item.u.rail.track.start_dir;
            for (auto i = 0; i < item.u.rail.track.length; i++) {
                this->source_tiles.insert(source_tile);
                source_tile = TileAddByDiagDir(source_tile, TrackdirToExitdir(tdir));
                tdir = NextTrackdir(tdir);
            }
            break;
        }
        case Item::Type::RAIL_STATION_PART:
            IterateStation(source_tile, item.u.rail.station_part.axis, item.u.rail.station_part.numtracks, item.u.rail.station_part.plat_len,
                [&](TileIndex tile, int, int) {
                    this->source_tiles.insert(tile);
                }
            );
            break;
        case Item::Type::RAIL_BRIDGE:
            this->source_tiles.insert(TileAddXY(source_tile, item.u.rail.bridge.other_end.x, item.u.rail.bridge.other_end.y));
            this->source_tiles.insert(source_tile);
            break;
        case Item::Type::RAIL_TUNNEL:
            this->source_tiles.insert(TileAddXY(source_tile, item.u.rail.tunnel.other_end.x, item.u.rail.tunnel.other_end.y));
            this->source_tiles.insert(source_tile);
            break;
        case Item::Type::RAIL_DEPOT:
        case Item::Type::RAIL_STATION:
            this->source_tiles.insert(source_tile);
            break;
        case Item::Type::RAIL_SIGNAL: // tile is added for track anyway
            break;
        default:
            NOT_REACHED();
    }
}

BlueprintCmdClosure GetBlueprintCommand(TileIndex start, const Blueprint::Item &item) {
	static const Track SIGNAL_POS_TRACK[] = {
		TRACK_LEFT, TRACK_LEFT, TRACK_RIGHT, TRACK_RIGHT,
		TRACK_UPPER, TRACK_UPPER, TRACK_LOWER, TRACK_LOWER,
		TRACK_X, TRACK_X, TRACK_Y, TRACK_Y,
	};

	switch (item.type) {
		case Blueprint::Item::Type::RAIL_TRACK:
			return [item, start](TileIndex) {
				auto start_tile = AddTileIndexDiffCWrap(start, item.tdiff);
				auto end_tile = start_tile;
				auto tdir = item.u.rail.track.start_dir;
				for (auto i = 1; i < item.u.rail.track.length; i++) {
					auto new_tile = AddTileIndexDiffCWrap(end_tile, TileIndexDiffCByDiagDir(TrackdirToExitdir(tdir)));
					if (new_tile == INVALID_TILE) break;
					end_tile = new_tile;
					tdir = NextTrackdir(tdir);
				}
				Command<Commands::BuildRailLong>::Post(end_tile, start_tile, _cur_railtype, TrackdirToTrack(item.u.rail.track.start_dir), BuildRailTrackFlags::AutoRemoveSignals, false);
			};
		case Blueprint::Item::Type::RAIL_DEPOT:
			return [item, start](TileIndex) {
				Command<Commands::BuildRailDepot>::Post(AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, item.u.rail.depot.ddir);
			};
		case Blueprint::Item::Type::RAIL_TUNNEL:
			return [item, start](TileIndex) {
				Command<Commands::BuildTunnel>::Post(AddTileIndexDiffCWrap(start, item.tdiff), TransportType::Rail, to_underlying(_cur_railtype));
			};
		case Blueprint::Item::Type::RAIL_BRIDGE:
			return [item, start](TileIndex) {
				Command<Commands::BuildBridge>::Post(AddTileIndexDiffCWrap(start, item.tdiff), AddTileIndexDiffCWrap(start, item.u.rail.bridge.other_end), TransportType::Rail, item.u.rail.bridge.type, to_underlying(_cur_railtype), BuildBridgeFlags::None);
			};
		case Blueprint::Item::Type::RAIL_STATION:
			return [item, start](TileIndex) {
				Command<Commands::BuildRailStation>::Post(STR_ERROR_CAN_T_BUILD_RAILROAD_STATION, CommandCallback::BlueprintStation, AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, Axis::X, 1, 1, _station_gui.sel_class, _station_gui.sel_type, NEW_STATION, true);
			};
		case Blueprint::Item::Type::RAIL_STATION_PART:
			return [item, start](TileIndex) {
				Command<Commands::BuildRailStation>::Post(STR_ERROR_CAN_T_BUILD_RAILROAD_STATION, CommandCallback::None, AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, item.u.rail.station_part.axis, item.u.rail.station_part.numtracks, item.u.rail.station_part.plat_len, _station_gui.sel_class, _station_gui.sel_type, NEW_STATION, true);
			};
		case Blueprint::Item::Type::RAIL_SIGNAL:
			return [item, start](TileIndex) {
				Command<Commands::BuildSignal>::Post(STR_ERROR_CAN_T_BUILD_SIGNALS_HERE, CommandCallback::None,
					AddTileIndexDiffCWrap(start, item.tdiff), SIGNAL_POS_TRACK[item.u.rail.signal.pos],
					item.u.rail.signal.type, item.u.rail.signal.variant, 0, 0,
					BuildSignalFlags::SkipExisting, SCG_BLOCK,
					SIGNAL_POS_NUM[item.u.rail.signal.pos] + (item.u.rail.signal.type <= SignalType::Combo && !item.u.rail.signal.twoway ? 1 : 0),
					0);
			};
		default:
			NOT_REACHED();
	}
	NOT_REACHED();
}

/* Test whether the command for a blueprint item would succeed at the given
 * location, without executing it (cmclient parity: GetBlueprintCommand().test()). */
static bool TestBlueprintCommand(TileIndex start, const Blueprint::Item &item) {
	static const Track SIGNAL_POS_TRACK[] = {
		TRACK_LEFT, TRACK_LEFT, TRACK_RIGHT, TRACK_RIGHT,
		TRACK_UPPER, TRACK_UPPER, TRACK_LOWER, TRACK_LOWER,
		TRACK_X, TRACK_X, TRACK_Y, TRACK_Y,
	};

	switch (item.type) {
		case Blueprint::Item::Type::RAIL_TRACK: {
			auto start_tile = AddTileIndexDiffCWrap(start, item.tdiff);
			auto end_tile = start_tile;
			auto tdir = item.u.rail.track.start_dir;
			for (auto i = 1; i < item.u.rail.track.length; i++) {
				auto new_tile = AddTileIndexDiffCWrap(end_tile, TileIndexDiffCByDiagDir(TrackdirToExitdir(tdir)));
				if (new_tile == INVALID_TILE) break;
				end_tile = new_tile;
				tdir = NextTrackdir(tdir);
			}
			return Command<Commands::BuildRailLong>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildRailLong>()) | DoCommandFlag::QueryCost, end_tile, start_tile, _cur_railtype, TrackdirToTrack(item.u.rail.track.start_dir), BuildRailTrackFlags::AutoRemoveSignals, false).Succeeded();
		}
		case Blueprint::Item::Type::RAIL_DEPOT:
			return Command<Commands::BuildRailDepot>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildRailDepot>()) | DoCommandFlag::QueryCost, AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, item.u.rail.depot.ddir).Succeeded();
		case Blueprint::Item::Type::RAIL_TUNNEL:
			return Command<Commands::BuildTunnel>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildTunnel>()) | DoCommandFlag::QueryCost, AddTileIndexDiffCWrap(start, item.tdiff), TransportType::Rail, to_underlying(_cur_railtype)).Succeeded();
		case Blueprint::Item::Type::RAIL_BRIDGE:
			return Command<Commands::BuildBridge>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildBridge>()) | DoCommandFlag::QueryCost, AddTileIndexDiffCWrap(start, item.tdiff), AddTileIndexDiffCWrap(start, item.u.rail.bridge.other_end), TransportType::Rail, item.u.rail.bridge.type, to_underlying(_cur_railtype), BuildBridgeFlags::None).Succeeded();
		case Blueprint::Item::Type::RAIL_STATION:
			return Command<Commands::BuildRailStation>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildRailStation>()) | DoCommandFlag::QueryCost, AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, Axis::X, 1, 1, _station_gui.sel_class, _station_gui.sel_type, NEW_STATION, true).Succeeded();
		case Blueprint::Item::Type::RAIL_STATION_PART:
			return Command<Commands::BuildRailStation>::Do(CommandFlagsToDCFlags(GetCommandFlags<Commands::BuildRailStation>()) | DoCommandFlag::QueryCost, AddTileIndexDiffCWrap(start, item.tdiff), _cur_railtype, item.u.rail.station_part.axis, item.u.rail.station_part.numtracks, item.u.rail.station_part.plat_len, _station_gui.sel_class, _station_gui.sel_type, NEW_STATION, true).Succeeded();
		case Blueprint::Item::Type::RAIL_SIGNAL:
			return true; /* Signals are placed on freshly built track; never flagged in the preview. */
		default:
			NOT_REACHED();
	}
	NOT_REACHED();
}

std::multimap<TileIndex, ObjectTileHighlight> Blueprint::GetTiles(TileIndex tile) {
    std::multimap<TileIndex, ObjectTileHighlight> res;
    if (tile == INVALID_TILE) return res;
    auto add_tile = [&res](TileIndex tile, const ObjectTileHighlight &ohl) {
        if (tile >= Map::Size()) return;
        res.emplace(tile, ohl);
    };

    /* Station parts whose sign (RAIL_STATION item) can be built are white;
     * those whose sign cannot be built are orange (cmclient parity). */
    std::set<StationID> can_build_station_sign;
    for (auto &item : this->items) {
        if (item.type != Item::Type::RAIL_STATION) continue;
        if (TestBlueprintCommand(tile, item)) can_build_station_sign.insert(item.u.rail.station.id);
    }

    for (auto &o: this->items) {
        auto otile = AddTileIndexDiffCWrap(tile, o.tdiff);
        auto palette = CM_PALETTE_TINT_WHITE;
        if (o.type != Item::Type::RAIL_SIGNAL && !TestBlueprintCommand(tile, o)) {
            palette = CM_PALETTE_TINT_RED_DEEP;
        }

        switch(o.type) {
            case Item::Type::RAIL_TRACK: {
                auto end_tile = otile;
                auto tdir = o.u.rail.track.start_dir;
                for (auto i = 0; i < o.u.rail.track.length; i++) {
                    add_tile(end_tile, ObjectTileHighlight::make_rail_track(palette, TrackdirToTrack(tdir)));
                    auto new_tile = AddTileIndexDiffCWrap(end_tile, TileIndexDiffCByDiagDir(TrackdirToExitdir(tdir)));
                    if (new_tile == INVALID_TILE) break;
                    end_tile = new_tile;
                    tdir = NextTrackdir(tdir);
                }
                break;
            }
            case Item::Type::RAIL_BRIDGE:
                add_tile(otile, ObjectTileHighlight::make_rail_bridge_head(palette, o.u.rail.bridge.ddir, o.u.rail.bridge.type));
                add_tile(AddTileIndexDiffCWrap(tile, o.u.rail.bridge.other_end), ObjectTileHighlight::make_rail_bridge_head(palette, ReverseDiagDir(o.u.rail.bridge.ddir), o.u.rail.bridge.type));
                break;
            case Item::Type::RAIL_TUNNEL:
                add_tile(otile, ObjectTileHighlight::make_rail_tunnel_head(palette, o.u.rail.tunnel.ddir));
                add_tile(AddTileIndexDiffCWrap(tile, o.u.rail.tunnel.other_end), ObjectTileHighlight::make_rail_tunnel_head(palette, ReverseDiagDir(o.u.rail.tunnel.ddir)));
                break;
            case Item::Type::RAIL_DEPOT:
                add_tile(otile, ObjectTileHighlight::make_rail_depot(palette, o.u.rail.depot.ddir));
                break;
            case Item::Type::RAIL_STATION_PART: {
                uint8_t layout_idx = 0;
                TileArea area{tile, o.u.rail.station_part.numtracks, o.u.rail.station_part.plat_len};
                if (o.u.rail.station_part.axis == Axis::X) std::swap(area.w, area.h);

                if (palette == CM_PALETTE_TINT_WHITE && can_build_station_sign.find(o.u.rail.station_part.id) == can_build_station_sign.end()) {
                    palette = CM_PALETTE_TINT_ORANGE;
                }
                IterateStation(otile, o.u.rail.station_part.axis, o.u.rail.station_part.numtracks, o.u.rail.station_part.plat_len,
                    [&](TileIndex tile, int, int) {
                        add_tile(tile, ObjectTileHighlight::make_rail_station(palette, o.u.rail.station_part.axis, layout_idx++, STAT_CLASS_DFLT, 0, area));
                    }
                );
                break;
            }
            case Item::Type::RAIL_SIGNAL:
                add_tile(otile, ObjectTileHighlight::make_rail_signal(CM_PALETTE_TINT_WHITE, o.u.rail.signal.pos, o.u.rail.signal.type, o.u.rail.signal.variant));
                if (o.u.rail.signal.twoway)
                    add_tile(otile, ObjectTileHighlight::make_rail_signal(CM_PALETTE_TINT_WHITE, o.u.rail.signal.pos | 1, o.u.rail.signal.type, o.u.rail.signal.variant));
                break;
            case Item::Type::RAIL_STATION:
                break;
            default:
                NOT_REACHED();
        }
    }
    return res;
}

sp<Blueprint> Blueprint::Rotate() {
    static const Trackdir ROTATE_TRACKDIR[] = {
        TRACKDIR_Y_SE, TRACKDIR_X_SW,
        TRACKDIR_RIGHT_S, TRACKDIR_LEFT_S,
        TRACKDIR_UPPER_W, TRACKDIR_LOWER_W,
        TRACKDIR_RVREV_SE, TRACKDIR_RVREV_SW,
        TRACKDIR_Y_NW, TRACKDIR_X_NE,
        TRACKDIR_RIGHT_N, TRACKDIR_LEFT_N,
        TRACKDIR_UPPER_E, TRACKDIR_LOWER_E,
        TRACKDIR_RVREV_NW, TRACKDIR_RVREV_NE
    };
    static const uint ROTATE_SIGNAL_POS[] = {
        // 5, 4, 7, 6,
        // 2, 3, 0, 1,
        // 11, 10, 8, 9
        5, 4, 7, 6,
        2, 3, 0, 1,
        10, 11, 9, 8
    };
    auto res = std::make_shared<Blueprint>();

    auto rotate = [](TileIndexDiffC td) -> TileIndexDiffC {
        return {static_cast<int16_t>(td.y), static_cast<int16_t>(-td.x)};
    };

    auto rotate_dir = [](DiagDirection ddir) -> DiagDirection {
        return ChangeDiagDir(ddir, DiagDirDiff::Right90);
    };

    for (auto &o: this->items) {
        auto odiff = rotate(o.tdiff);
        Blueprint::Item bi(o.type, odiff);
        switch(o.type) {
            case Item::Type::RAIL_TRACK:
                bi.u.rail.track.length = o.u.rail.track.length;
                bi.u.rail.track.start_dir = ROTATE_TRACKDIR[o.u.rail.track.start_dir];
                break;
            case Item::Type::RAIL_BRIDGE:
                bi.u.rail.bridge.type = o.u.rail.bridge.type;
                bi.u.rail.bridge.ddir = rotate_dir(o.u.rail.bridge.ddir);
                bi.u.rail.bridge.other_end = rotate(o.u.rail.bridge.other_end);
                break;
            case Item::Type::RAIL_TUNNEL:
                bi.u.rail.tunnel.ddir = rotate_dir(o.u.rail.tunnel.ddir);
                bi.u.rail.tunnel.other_end = rotate(o.u.rail.tunnel.other_end);
                break;
            case Item::Type::RAIL_DEPOT:
                bi.u.rail.depot.ddir = rotate_dir(o.u.rail.depot.ddir);
                break;
            case Item::Type::RAIL_STATION:
                bi.u.rail.station.id = o.u.rail.station.id;
                bi.u.rail.station.has_part = o.u.rail.station.has_part;
                break;
            case Item::Type::RAIL_STATION_PART: {
                auto ediff = rotate({
                    (int16_t)(o.tdiff.x + (o.u.rail.station_part.axis == Axis::X ? o.u.rail.station_part.plat_len : o.u.rail.station_part.numtracks) - 1),
                    (int16_t)(o.tdiff.y + (o.u.rail.station_part.axis == Axis::X ? o.u.rail.station_part.numtracks : o.u.rail.station_part.plat_len) - 1),
                });
                bi.tdiff = {std::min(odiff.x, ediff.x), std::min(odiff.y, ediff.y)};
                bi.u.rail.station_part.id = o.u.rail.station_part.id;
                bi.u.rail.station_part.axis = OtherAxis(o.u.rail.station_part.axis);
                bi.u.rail.station_part.numtracks = o.u.rail.station_part.numtracks;
                bi.u.rail.station_part.plat_len = o.u.rail.station_part.plat_len;
                break;
            }
            case Item::Type::RAIL_SIGNAL:
                bi.u.rail.signal.pos = ROTATE_SIGNAL_POS[o.u.rail.signal.pos]; // TODO rotate
                bi.u.rail.signal.type = o.u.rail.signal.type;
                bi.u.rail.signal.variant = o.u.rail.signal.variant;
                break;
            default:
                NOT_REACHED();
        }
        res->items.push_back(bi);
    }
    res->source_tiles = this->source_tiles;
    return res;
}


static void BlueprintAddSignals(sp<Blueprint> &blueprint, TileIndex tile, TileIndexDiffC tdiff) {
    // reference: DrawSignals @ rail_cmd.cpp

    auto add = [&](Track track, uint x, uint pos) {
        auto a = IsSignalPresent(tile, x);
        auto b = IsSignalPresent(tile, x ^ 1);
        if (!a && !b) return;
        if (!a) pos = pos | 1;
        Blueprint::Item bi(Blueprint::Item::Type::RAIL_SIGNAL, tdiff);
        bi.u.rail.signal.pos = pos;
        bi.u.rail.signal.type = GetSignalType(tile, track);
        bi.u.rail.signal.variant = GetSignalVariant(tile, track);
        bi.u.rail.signal.twoway = a && b;
        blueprint->Add(tile, bi);
    };
    auto rails = GetTrackBits(tile);
    if (!(rails & TRACK_BIT_Y)) {
        if (!(rails & TRACK_BIT_X)) {
            if (rails & TRACK_BIT_LEFT) add(TRACK_LEFT, 2, 0);
            if (rails & TRACK_BIT_RIGHT) add(TRACK_RIGHT, 0, 2);
            if (rails & TRACK_BIT_UPPER) add(TRACK_UPPER, 3, 4);
            if (rails & TRACK_BIT_LOWER) add(TRACK_LOWER, 1, 6);
        } else {
            add(TRACK_X, 3, 8);
        }
    } else {
        add(TRACK_Y, 3, 10);
    }
}

static void BlueprintAddTracks(sp<Blueprint> &blueprint, TileIndex tile, TileIndexDiffC tdiff, TileArea &area,
                               std::map<TileIndex, Track> &track_tiles) {
    // tilearea is iterated by x and y so chose direction to go in uniterated area
    static const Trackdir _track_iterate_dir[TRACK_END] = { TRACKDIR_X_SW, TRACKDIR_Y_SE, TRACKDIR_UPPER_E, TRACKDIR_LOWER_E, TRACKDIR_LEFT_S, TRACKDIR_RIGHT_S};

    for (Track track = TRACK_BEGIN; track < TRACK_END; track++) {
        TileIndex c_tile = tile;
        TileIndex end_tile = INVALID_TILE;
        uint16_t length = 0;
        Track c_track = track;
        Trackdir c_tdir = _track_iterate_dir[track];
        while (IsPlainRailTile(c_tile) && HasBit(GetTrackBits(c_tile), c_track)) {
            if (HasBit(track_tiles[c_tile], c_track)) break;
            length++;
            SetBit(track_tiles[c_tile], c_track);
            end_tile = c_tile;
            c_tile = TileAddByDiagDir(c_tile, TrackdirToExitdir(c_tdir));
            if (!area.Contains(c_tile)) break;
            c_tdir = NextTrackdir(c_tdir);
            c_track = TrackdirToTrack(c_tdir);
        }
        if (end_tile == INVALID_TILE) continue;
        Blueprint::Item bi(Blueprint::Item::Type::RAIL_TRACK, tdiff);
        bi.u.rail.track.length = length;
        bi.u.rail.track.start_dir = _track_iterate_dir[track];
        blueprint->Add(tile, bi);
    }
}

void BlueprintCopyArea(TileIndex start, TileIndex end) {
    // if (start > end) Swap(start, end);
    TileArea ta{start, end};
    start = ta.tile;

    ResetActiveBlueprint();

    auto blueprint = std::make_shared<Blueprint>();

    std::map<TileIndex, Track> track_tiles;
    std::multimap<StationID, TileIndex> station_tiles;
    std::set<StationID> stations;

    for (TileIndex tile : ta) {
        TileIndexDiffC td = TileIndexToTileIndexDiffC(tile, start);
        switch (GetTileType(tile)) {
            case TileType::Station:
                if (IsRailStation(tile)) {
                    stations.insert(GetStationIndex(tile));
                    if (stations.size() > 10) return;
                }
                break;

            case TileType::Railway:
                switch (GetRailTileType(tile)) {
                    case RailTileType::Depot: {
                        Blueprint::Item bi(Blueprint::Item::Type::RAIL_DEPOT, td);
                        bi.u.rail.depot.ddir = GetRailDepotDirection(tile);
                        blueprint->Add(tile, bi);
                        break;
                    }
                    case RailTileType::Signals:
                        BlueprintAddSignals(blueprint, tile, td);
                        [[fallthrough]];
                    case RailTileType::Normal:
                        BlueprintAddTracks(blueprint, tile, td, ta, track_tiles);
                        break;
                    default:
                        NOT_REACHED();
                }
                break;
            case TileType::TunnelBridge: {
                if (GetTunnelBridgeTransportType(tile) != TransportType::Rail) break;
                auto other = GetOtherTunnelBridgeEnd(tile);
                if (!ta.Contains(other)) break;
                if (other < tile) break;
                Blueprint::Item bi(Blueprint::Item::Type::RAIL_TUNNEL, td);
                if (!IsTunnel(tile)) {
                    bi.type = Blueprint::Item::Type::RAIL_BRIDGE;
                    bi.u.rail.bridge.type = GetBridgeType(tile);
                    bi.u.rail.bridge.ddir = GetTunnelBridgeDirection(tile);
                    bi.u.rail.bridge.other_end = TileIndexToTileIndexDiffC(other, start);
                } else {
                    bi.u.rail.tunnel.ddir = GetTunnelBridgeDirection(tile);
                    bi.u.rail.tunnel.other_end = TileIndexToTileIndexDiffC(other, start);
                }
                blueprint->Add(tile, bi);
                break;
            }
            default:
                break;
        }
        if (blueprint->items.size() > 200) return;
    }

    for (auto sid : stations) {
        auto st = Station::Get(sid);

        if (!ta.Contains(st->xy)) continue;

        TileArea sta(TileXY(st->rect.left, st->rect.top), TileXY(st->rect.right, st->rect.bottom));
        bool in_area = true;
        bool sign_part = false;
        std::vector<TileIndex> tiles;
        for (TileIndex tile : sta) {
            if (!IsTileType(tile, TileType::Station) || GetStationIndex(tile) != sid || !IsRailStation(tile)) continue;
            if (!ta.Contains(tile)) {
                in_area = false;
                break;
            }
            tiles.push_back(tile);
            if (tile == st->xy) sign_part = true;
        }

        if (!in_area) continue;

        Blueprint::Item bi(Blueprint::Item::Type::RAIL_STATION, TileIndexToTileIndexDiffC(st->xy, start));
        bi.u.rail.station.id = sid;
        bi.u.rail.station.has_part = sign_part;
        blueprint->Add(st->xy, bi);

        std::set<TileIndex> added_tiles;
        for (auto tile : tiles) {
            if (added_tiles.find(tile) != added_tiles.end())
                continue;

            auto axis = GetRailStationAxis(tile);
            auto platform_delta = (axis == Axis::X ? TileDiffXY(1, 0) : TileDiffXY(0, 1));
            auto track_delta = (axis == Axis::Y ? TileDiffXY(1, 0) : TileDiffXY(0, 1));
            auto plat_len = 0;

            auto can_add_tile = [=](TileIndex tile) {
                return (IsValidTile(tile)
                    && IsTileType(tile, TileType::Station)
                    && GetStationIndex(tile) == sid
                    && IsRailStation(tile)
                    && GetRailStationAxis(tile) == axis);
            };

            auto cur_tile = tile;
            do {
                plat_len++;
                cur_tile = TileAdd(cur_tile, platform_delta);
            } while (can_add_tile(cur_tile));

            auto numtracks = 0;
            cur_tile = tile;
            bool can_add = false;
            do {
                numtracks++;
                cur_tile = TileAdd(cur_tile, track_delta);
                can_add = true;
                auto plat_tile = cur_tile;
                for (auto i = 0; i < plat_len; i++) {
                    if (!can_add_tile(plat_tile)) {
                        can_add = false;
                        break;
                    }
                    plat_tile = TileAdd(plat_tile, platform_delta);
                }
            } while (can_add);

            cur_tile = tile;
            for (auto i = 0; i < numtracks; i++) {
                auto plat_tile = cur_tile;
                for (auto j = 0; j < plat_len; j++) {
                    added_tiles.insert(plat_tile);
                    plat_tile = TileAdd(plat_tile, platform_delta);
                }
                cur_tile = TileAdd(cur_tile, track_delta);
            }

            Blueprint::Item bi(Blueprint::Item::Type::RAIL_STATION_PART, TileIndexToTileIndexDiffC(tile, start));
            bi.u.rail.station_part.id = sid;
            bi.u.rail.station_part.axis = axis;
            bi.u.rail.station_part.numtracks = numtracks;
            bi.u.rail.station_part.plat_len = plat_len;
            blueprint->Add(tile, bi);
        }
    }

    _active_blueprint = std::make_pair(start, blueprint);
}

void UpdateBlueprintTileSelection(TileIndex tile) {
    if (_active_blueprint.second == nullptr) {
        _cm_active_object = ObjectHighlight{};
        return;
    }
    _cm_active_object = ObjectHighlight::make_blueprint(tile, _active_blueprint.second);
}

void ResetActiveBlueprint() {
    _active_blueprint = std::make_pair(INVALID_TILE, nullptr);
}

void SetBlueprintHighlight(const TileInfo *ti, TileHighlight &th) {
    if (_active_blueprint.first == INVALID_TILE || !_active_blueprint.second)
        return;

    if (_active_blueprint.second->HasSourceTile(ti->tile)) {
        th.tint_all(CM_PALETTE_TINT_BLUE);
    }
}

void BuildBlueprint(sp<Blueprint> &blueprint, TileIndex start) {
	bool has_rails = false;

	/* First pass: tracks, depots, tunnels, bridges and stations. */
	for (auto &item : blueprint->items) {
		switch (item.type) {
			case Blueprint::Item::Type::RAIL_TRACK:
				has_rails = true;
				[[fallthrough]];
			case Blueprint::Item::Type::RAIL_DEPOT:
			case Blueprint::Item::Type::RAIL_TUNNEL:
			case Blueprint::Item::Type::RAIL_BRIDGE:
				GetBlueprintCommand(start, item)(start);
				break;
			case Blueprint::Item::Type::RAIL_STATION: {
				/* Register the pending station build: the sign (1x1) is
				 * posted with CommandCallback::BlueprintStation, which posts
				 * the joined parts once the sign has been built. */
				TileIndex sign_tile = AddTileIndexDiffCWrap(start, item.tdiff);
				_pending_blueprint_stations.push_back({blueprint, start, sign_tile, item.u.rail.station.has_part, item.u.rail.station.id});
				GetBlueprintCommand(start, item)(start);
				break;
			}
			default:
				break;
		}
	}

	/* Second pass: signals go on top of the freshly built track.
	 * Signals are only posted if the blueprint contains rails (cmclient parity). */
	if (has_rails) {
		for (auto &item : blueprint->items) {
			if (item.type == Blueprint::Item::Type::RAIL_SIGNAL) {
				GetBlueprintCommand(start, item)(start);
			}
		}
	}
}
void RotateActiveBlueprint() {
    if (!_active_blueprint.second) return;
    _active_blueprint.second = _active_blueprint.second->Rotate();
}

void BuildActiveBlueprint(TileIndex start) {
    if (!_active_blueprint.second) return;
    BuildBlueprint(_active_blueprint.second, start);
}

void SaveBlueprint(uint slot) {
    if (slot >= MAX_BLUEPRINT_SLOTS) return;
    _blueprint_slots[slot] = _active_blueprint.second;
    _active_blueprint = {INVALID_TILE, nullptr};
}

bool LoadBlueprint(uint slot) {
    if (slot >= MAX_BLUEPRINT_SLOTS) return false;
    _active_blueprint = {INVALID_TILE, _blueprint_slots[slot]};
    if (_active_blueprint.second == nullptr) {
        ShowErrorMessage(GetEncodedString(CM_STR_NO_BLUEPRINT_IN_SLOT, slot), {}, WarningLevel::Error);
    }
    return _active_blueprint.second != nullptr;
}


/* ---- Blueprint console commands ---- */

static bool ConBlueprintCopy(std::span<std::string_view> argv)
{
	if (!_settings_game.jrpm_features.enable_blueprint) {
		IConsolePrint(CC_ERROR, "The blueprint feature is disabled in the settings (jrpm feature toggles).");
		return true;
	}
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Copy the rail layout in the current tile selection to the blueprint buffer. Usage: 'blueprint_copy'.");
		return true;
	}
	BlueprintCopyArea(TileVirtXY(_thd.selstart.x, _thd.selstart.y), TileVirtXY(_thd.selend.x, _thd.selend.y));
	IConsolePrint(CC_DEFAULT, "Blueprint copied ({} items).", _active_blueprint.second ? _active_blueprint.second->items.size() : 0);
	return true;
}

static bool ConBlueprintBuild(std::span<std::string_view> argv)
{
	if (!_settings_game.jrpm_features.enable_blueprint) {
		IConsolePrint(CC_ERROR, "The blueprint feature is disabled in the settings (jrpm feature toggles).");
		return true;
	}
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Build the active blueprint at the selected tile. Usage: 'blueprint_build'.");
		return true;
	}
	TileIndex tile = TileVirtXY(_thd.selstart.x, _thd.selstart.y);
	BuildActiveBlueprint(tile);
	return true;
}

static bool ConBlueprintSave(std::span<std::string_view> argv)
{
	if (!_settings_game.jrpm_features.enable_blueprint) {
		IConsolePrint(CC_ERROR, "The blueprint feature is disabled in the settings (jrpm feature toggles).");
		return true;
	}
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Save the active blueprint to a slot. Usage: 'blueprint_save <0-15>'.");
		return true;
	}
	auto r = ParseInteger(argv[1]);
	if (!r.has_value() || *r >= MAX_BLUEPRINT_SLOTS) {
		IConsolePrint(CC_ERROR, "Invalid slot.");
		return false;
	}
	SaveBlueprint(*r);
	/* Leave blueprint placement mode (if active). */
	ResetObjectToPlace();
	return true;
}

static bool ConBlueprintLoad(std::span<std::string_view> argv)
{
	if (!_settings_game.jrpm_features.enable_blueprint) {
		IConsolePrint(CC_ERROR, "The blueprint feature is disabled in the settings (jrpm feature toggles).");
		return true;
	}
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Load a blueprint from a slot. Usage: 'blueprint_load <0-15>'.");
		return true;
	}
	auto r = ParseInteger(argv[1]);
	if (!r.has_value() || *r >= MAX_BLUEPRINT_SLOTS) {
		IConsolePrint(CC_ERROR, "Invalid slot.");
		return false;
	}
	if (LoadBlueprint(*r)) {
		/* Enter blueprint placement mode so the loaded blueprint can be placed immediately. */
		ActivateRailBlueprintPlaceMode();
	}
	return true;
}

static bool ConBlueprintRotate(std::span<std::string_view> argv)
{
	if (!_settings_game.jrpm_features.enable_blueprint) {
		IConsolePrint(CC_ERROR, "The blueprint feature is disabled in the settings (jrpm feature toggles).");
		return true;
	}
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Rotate the active blueprint 90 degrees. Usage: 'blueprint_rotate'.");
		return true;
	}
	RotateActiveBlueprint();
	return true;
}

void RegisterJRPMBlueprintCommands()
{
	IConsole::CmdRegister("blueprint_copy", ConBlueprintCopy);
	IConsole::CmdRegister("blueprint_build", ConBlueprintBuild);
	IConsole::CmdRegister("blueprint_save", ConBlueprintSave);
	IConsole::CmdRegister("blueprint_load", ConBlueprintLoad);
	IConsole::CmdRegister("blueprint_rotate", ConBlueprintRotate);
}

}  // namespace citymania
