#ifndef CM_OVERLAYS_HPP
#define CM_OVERLAYS_HPP

#include "sprite.h"  // SpriteID

#include <string>
#include <tuple>
#include <vector>

namespace citymania {

typedef std::vector<std::tuple<uint, SpriteID, std::string>> BuildInfoOverlayData;

void UndrawOverlays(int left, int top, int right, int bottom);
void DrawOverlays();
void ShowBuildInfoOverlay(int x, int y, BuildInfoOverlayData data);
void HideBuildInfoOverlay();

} // namespace citymania

#endif
