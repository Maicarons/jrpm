/*
 * Town growth tile tracking - implementation (cmclient port, batch 4).
 */

#include "stdafx.h"

#include "cm_town_growth.h"

#include "safeguards.h"

TownsGrowthTilesIndex _town_growth_tiles;
TownsGrowthTilesIndex _town_growth_tiles_last_month;

void SetTownGrowthTile(TileIndex tile, TownGrowthTileState state)
{
	_town_growth_tiles[tile] = state;
}

TownGrowthTileState GetTownGrowthTile(TileIndex tile)
{
	auto a = _town_growth_tiles.find(tile);
	auto b = _town_growth_tiles_last_month.find(tile);
	auto as = (a == _town_growth_tiles.end() ? TownGrowthTileState::NONE : (*a).second);
	auto bs = (b == _town_growth_tiles_last_month.end() ? TownGrowthTileState::NONE : (*b).second);
	return std::max(as, bs);
}

void RotateTownGrowthTiles()
{
	_town_growth_tiles_last_month.swap(_town_growth_tiles);
	_town_growth_tiles.clear();
}
