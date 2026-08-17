#ifndef CITYMANIA_HIGHLIGHT_HPP
#define CITYMANIA_HIGHLIGHT_HPP

#include "cm_highlight_type.hpp"

#include "command_func.h"

#include "core/enum_type.hpp"
#include "gfx_type.h"
#include "industry_type.h"
#include "tile_cmd.h"
#include "tile_type.h"
#include "tilehighlight_type.h"
#include "town_type.h"

#include "table/sprites.h"

namespace citymania {

/* cmclient used custom NewGRF sprites for zoning palettes; map them to
 * JGRPP's built-in semi-transparent inner-highlight tint sprites
 * (innerhighlight.grf, loaded at SPR_ZONING_INNER_HIGHLIGHT_BASE). */
#define CM_SPR_PALETTE_ZONING_RED        SPR_ZONING_INNER_HIGHLIGHT_RED
#define CM_SPR_PALETTE_ZONING_ORANGE     SPR_ZONING_INNER_HIGHLIGHT_ORANGE
#define CM_SPR_PALETTE_ZONING_GREEN      SPR_ZONING_INNER_HIGHLIGHT_GREEN
#define CM_SPR_PALETTE_ZONING_LIGHT_BLUE SPR_ZONING_INNER_HIGHLIGHT_LIGHT_BLUE
#define CM_SPR_PALETTE_ZONING_YELLOW     SPR_ZONING_INNER_HIGHLIGHT_YELLOW
#define CM_SPR_PALETTE_ZONING_WHITE      SPR_ZONING_INNER_HIGHLIGHT_WHITE
#define CM_SPR_PALETTE_ZONING_BLACK      SPR_ZONING_INNER_HIGHLIGHT_BLACK
#define CM_SPR_PALETTE_ZONING_PURPLE     SPR_ZONING_INNER_HIGHLIGHT_PURPLE
#define CM_PALETTE_TINT_BASE SPR_ZONING_INNER_HIGHLIGHT_BASE
/* cmclient station highlight palette tints (cm_station_gui.cpp). */
#define CM_PALETTE_TINT_WHITE    SPR_ZONING_INNER_HIGHLIGHT_WHITE
#define CM_PALETTE_TINT_CYAN     SPR_ZONING_INNER_HIGHLIGHT_LIGHT_BLUE
#define CM_PALETTE_TINT_BLUE     SPR_ZONING_INNER_HIGHLIGHT_LIGHT_BLUE
#define CM_PALETTE_TINT_RED_DEEP SPR_ZONING_INNER_HIGHLIGHT_RED
#define CM_PALETTE_TINT_RED      SPR_ZONING_INNER_HIGHLIGHT_RED
#define CM_PALETTE_TINT_ORANGE   SPR_ZONING_INNER_HIGHLIGHT_ORANGE
#define CM_PALETTE_TINT_YELLOW   SPR_ZONING_INNER_HIGHLIGHT_YELLOW
#define CM_PALETTE_TINT_GREEN    SPR_ZONING_INNER_HIGHLIGHT_GREEN

// enum class AdvertisementZone: uint8_t {
//     NONE = 0,
//     LARGE = 1,
//     MEDIUM = 2,
//     SMALL = 3,
// };
SpriteID MixTints(SpriteID bottom, SpriteID top);

TileHighlight GetTileHighlight(const TileInfo *ti, TileType tile_type);
void DrawTileZoning(const TileInfo *ti, const TileHighlight &th, TileType tile_type);
bool DrawTileSelection(const TileInfo *ti, const TileHighlightType &tht);
void DrawSelectionOverlay(DrawPixelInfo *dpi);

void AllocateZoningMap(uint map_size);
void InitializeZoningMap();

void UpdateTownZoning(Town *town, uint32_t prev_edge);
void UpdateZoningTownHouses(const Town *town, uint32_t old_houses);
HighLightStyle UpdateTileSelection(HighLightStyle new_drawstyle);

std::pair<ZoningBorder, uint8_t> GetTownZoneBorder(TileIndex tile);
ZoningBorder GetAnyStationCatchmentBorder(TileIndex tlie);
// std::pair<ZoningBorder, uint8_t> GetTownAdvertisementBorder(TileIndex tile);
//
SpriteID GetTownTileZoningPalette(TileIndex tile);
SpriteID GetIndustryTileZoningPalette(TileIndex tile, Industry *ind);
void UpdateIndustryHighlight();
void SetIndustryForbiddenTilesHighlight(IndustryType type);


PaletteID GetTreeShadePal(TileIndex tile);

void RotateAutodetection();
void ResetRotateAutodetection();

void ResetActiveTool();
void SetActiveTool(up<Tool> &&tool);
void UpdateActiveTool();
const up<Tool> &GetActiveTool();


bool HandlePlacePushButton(Window *w, WidgetID widget, up<Tool> tool);
bool HandleMouseMove();
bool HandleMouseClick(Viewport *vp, bool double_click);

}  // namespace citymania

#endif
