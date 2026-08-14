/*
 * Town growth tile tracking (cmclient port, batch 4).
 *
 * Records per-tile house construction/demolition events for the
 * "town growth tiles" zoning mode. Data is kept in two rolling monthly
 * maps and persisted in a dedicated GRWT savegame chunk (gated by the
 * XSLFI_TOWN_GROWTH_TILES extended feature so old saves load unchanged).
 */

#ifndef CM_TOWN_GROWTH_H
#define CM_TOWN_GROWTH_H

#include "tile_type.h"

#include <map>

/** State of a growth tile. */
enum class TownGrowthTileState : uint8_t {
	NONE = 0,      ///< No event recorded.
	RH_REMOVED,    ///< House was removed.
	NEW_HOUSE,     ///< New house was built.
	RH_REBUILT,    ///< House was removed then rebuilt.
	CS,            ///< Skipped growth cycle (CityBuilder server mode).
	HS,            ///< Skipped house build (CityBuilder server mode).
	HR,            ///< House removed by server (CityBuilder server mode).
};

/** Map of tile -> growth state. */
typedef std::map<TileIndex, TownGrowthTileState> TownsGrowthTilesIndex;

/** Growth tiles recorded during the current month. */
extern TownsGrowthTilesIndex _town_growth_tiles;
/** Growth tiles recorded during the previous month. */
extern TownsGrowthTilesIndex _town_growth_tiles_last_month;

/** Record a growth event on a tile (current month map). */
void SetTownGrowthTile(TileIndex tile, TownGrowthTileState state);
/** Query the effective state of a tile (max of current and last month). */
TownGrowthTileState GetTownGrowthTile(TileIndex tile);
/** Roll the monthly maps over (call from TownsMonthlyLoop). */
void RotateTownGrowthTiles();

#endif /* CM_TOWN_GROWTH_H */
