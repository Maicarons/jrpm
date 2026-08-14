#ifndef CM_BLUEPRINT_HPP
#define CM_BLUEPRINT_HPP

#include "cm_highlight.hpp"
#include "cm_highlight_type.hpp"

#include <functional>

namespace citymania {

void BlueprintCopyArea(TileIndex start, TileIndex end);
void ResetActiveBlueprint();
void SetBlueprintHighlight(const TileInfo *ti, TileHighlight &th);


void UpdateBlueprintTileSelection(TileIndex tile);
void BuildActiveBlueprint(TileIndex start);
void RotateActiveBlueprint();
void SaveBlueprint(uint slot);
bool LoadBlueprint(uint slot);

void CommandExecuted(bool res, TileIndex tile, uint32_t p1, uint32_t p2, uint32_t cmd);

/** Register the blueprint console commands. */
void RegisterJRPMBlueprintCommands();

}  // namespace citymania

#endif
