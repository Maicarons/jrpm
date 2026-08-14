#include "stdafx.h"

#include "cm_tooltips.hpp"

#include "company_base.h"
#include "debug.h"
#include "house.h"
#include "industry.h"
#include "station_base.h"
#include "station_map.h"
#include "string_func.h"
#include "strings_func.h"
#include "tile_type.h"
#include "town_map.h"
#include "town.h"
#include "viewport_func.h"
#include "window_func.h"
#include "zoom_func.h"
#include "widgets/misc_widget.h"

#include "safeguards.h"

namespace citymania {

static const NWidgetPart _nested_land_tooltips_widgets[] = {
    NWidget(WWT_PANEL, Colours::Grey, WID_LI_BACKGROUND), SetMinimalSize(64, 32), EndContainer(),
};

static WindowDesc _land_tooltips_desc(__FILE__, __LINE__,
    WindowPosition::Manual, "land_tooltips", 0, 0,
    WindowClass::CmLandTooltips, WindowClass::None,
    {},
    _nested_land_tooltips_widgets
);


struct LandTooltipsWindow : public Window
{
    TileType tiletype;
    uint16_t objIndex;

    LandTooltipsWindow(Window *parent, uint param) : Window(_land_tooltips_desc)
    {
        this->parent = parent;
        this->tiletype = (TileType)(param & 0xFFFF);
        this->objIndex = (uint16_t)((param >> 16) & 0xFFFF);
        this->InitNested();
        this->flags.Reset(WindowFlag::WhiteBorder);

    }

    virtual ~LandTooltipsWindow() {}

    Point OnInitialPosition(int16_t sm_width, int16_t sm_height, int /* window_number */) override
    {
        int scr_top = GetMainViewTop() + 2;
        int scr_bot = GetMainViewBottom() - 2;
        Point pt;
        pt.y = Clamp(_cursor.pos.y + _cursor.total_size.y + _cursor.total_offs.y + 5, scr_top, scr_bot);
        if (pt.y + sm_height > scr_bot) pt.y = std::min(_cursor.pos.y + _cursor.total_offs.y - 5, scr_bot) - sm_height;
        pt.x = sm_width >= _screen.width ? 0 : Clamp(_cursor.pos.x - (sm_width >> 1), 0, _screen.width - sm_width);
        return pt;
    }

    void UpdateWidgetSize(WidgetID widget, Dimension &size, [[maybe_unused]] const Dimension &padding, [[maybe_unused]] Dimension &fill, [[maybe_unused]] Dimension &resize) override
    {
        if (widget != WID_LI_BACKGROUND) return;

        uint icon_size = ScaleGUITrad(10);
        uint line_height = std::max((uint)GetCharacterHeight(FontSize::Normal), icon_size) + WidgetDimensions::scaled.hsep_normal;
        uint text_height = GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.hsep_normal;
        uint icons_width = icon_size * 3 + WidgetDimensions::scaled.vsep_normal;
        size.width = ScaleGUITrad(194);
        size.height = GetCharacterHeight(FontSize::Normal);
        switch(this->tiletype) {
            case TileType::House: {
                const HouseSpec *hs = HouseSpec::Get((HouseID)this->objIndex);
                if(hs == NULL) break;
                size.width = std::max(GetStringBoundingBox(GetString(CM_STR_LAND_TOOLTIPS_HOUSE_NAME, hs->building_name)).width, size.width);
                size.height += text_height;
                size.width = std::max(size.width, GetStringBoundingBox(GetString(CM_STR_LAND_TOOLTIPS_HOUSE_POPULATION, hs->population)).width);
                break;
            }
            case TileType::Industry: {
                const Industry *ind = Industry::GetIfValid((IndustryID)this->objIndex);
                if(ind == NULL) break;

                size.width = std::max(GetStringBoundingBox(GetString(CM_STR_LAND_TOOLTIPS_INDUSTRY_NAME, ind->index)).width, size.width);

                for (uint8_t i = 0; i < ind->produced_cargo_count; i++) {
                    const auto &p = ind->produced[i];
                    const CargoSpec *cs = CargoSpec::Get(p.cargo);
                    if(cs == nullptr) continue;
                    size.height += line_height;
                    auto str = GetString(
                        CM_STR_LAND_TOOLTIPS_INDUSTRY_CARGO,
                        cs->name,
                        cs->Index(),
                        p.history[LAST_MONTH].production,
                        ToPercent8(p.history[LAST_MONTH].PctTransported())
                    );
                    size.width = std::max(GetStringBoundingBox(str).width + icons_width, size.width);
                }
                break;
            }
            case TileType::Station: {
                const Station *st = Station::GetIfValid((StationID)this->objIndex);
                if(st == NULL) break;

                size.width = std::max(GetStringBoundingBox(GetString(CM_STR_LAND_TOOLTIPS_STATION_NAME, st->index)).width, size.width);

                for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
                    auto cargoid = cs->Index();
                    auto &ge = st->goods[cargoid];
                    if (ge.HasRating()) {
                        size.height += line_height;
                        auto str = GetString(
                            CM_STR_LAND_TOOLTIPS_STATION_CARGO,
                            cs->name,
                            cargoid,
                            ge.CargoTotalCount(),
                            ToPercent8(ge.rating)
                        );
                        size.width = std::max(GetStringBoundingBox(str).width + icons_width, size.width);
                    }
                }
                break;
            }
            default:
                break;
        }
        size.width  += WidgetDimensions::scaled.framerect.Horizontal() + WidgetDimensions::scaled.fullbevel.Horizontal();
        size.height += WidgetDimensions::scaled.framerect.Vertical() + WidgetDimensions::scaled.fullbevel.Vertical();
    }

    void DrawWidget(const Rect &r, int /* widget */) const override
    {
        uint icon_size = ScaleGUITrad(10);
        uint line_height = std::max((uint)GetCharacterHeight(FontSize::Normal), icon_size) + WidgetDimensions::scaled.hsep_normal;
        uint text_height = GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.hsep_normal;
        uint icons_width = icon_size * 3 + WidgetDimensions::scaled.vsep_normal;
        uint text_ofs = (line_height - GetCharacterHeight(FontSize::Normal)) >> 1;
        uint icon_ofs = (line_height - icon_size) >> 1;

        GfxFillRect(r.left, r.top, r.right, r.top + WidgetDimensions::scaled.bevel.top - 1, PC_BLACK);
        GfxFillRect(r.left, r.bottom - WidgetDimensions::scaled.bevel.bottom + 1, r.right, r.bottom, PC_BLACK);
        GfxFillRect(r.left, r.top, r.left + WidgetDimensions::scaled.bevel.left - 1,  r.bottom, PC_BLACK);
        GfxFillRect(r.right - WidgetDimensions::scaled.bevel.right + 1, r.top, r.right, r.bottom, PC_BLACK);

        auto ir = r.Shrink(WidgetDimensions::scaled.framerect).Shrink(WidgetDimensions::scaled.fullbevel);
        switch(this->tiletype) {
            case TileType::House: {
                const HouseSpec *hs = HouseSpec::Get((HouseID)this->objIndex);
                if(hs == nullptr) break;

                DrawString(ir, GetString(CM_STR_LAND_TOOLTIPS_HOUSE_NAME, hs->building_name), TextColour::Black, SA_CENTER);
                ir.top += text_height;
                DrawString(ir, GetString(CM_STR_LAND_TOOLTIPS_HOUSE_POPULATION, hs->population), TextColour::Black, SA_CENTER);
                break;
            }
            case TileType::Industry: {
                const Industry *ind = Industry::GetIfValid((IndustryID)this->objIndex);
                if(ind == nullptr) break;

                DrawString(ir, GetString(CM_STR_LAND_TOOLTIPS_INDUSTRY_NAME, ind->index), TextColour::Black, SA_CENTER);
                ir.top += text_height;

                for (uint8_t i = 0; i < ind->produced_cargo_count; i++) {
                    const auto &p = ind->produced[i];
                    const CargoSpec *cs = CargoSpec::Get(p.cargo);
                    if(cs == nullptr) continue;
                    auto str = GetString(
                        CM_STR_LAND_TOOLTIPS_INDUSTRY_CARGO,
                        cs->name,
                        cs->Index(),
                        p.history[LAST_MONTH].production,
                        ToPercent8(p.history[LAST_MONTH].PctTransported())
                    );
                    this->DrawSpriteIcons(cs->GetCargoIcon(), ir.left, ir.top + icon_ofs);
                    DrawString(ir.left + icons_width, ir.right, ir.top + text_ofs, str);
                    ir.top += line_height;
                }
                break;
            }
            case TileType::Station: {
                const Station *st = Station::GetIfValid((StationID)this->objIndex);
                if(st == nullptr) break;

                DrawString(ir, GetString(CM_STR_LAND_TOOLTIPS_STATION_NAME, st->index), TextColour::Black, SA_CENTER);
                ir.top += text_height;

                for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
                    auto cargoid = cs->Index();
                    auto &ge = st->goods[cargoid];
                    if (ge.HasRating()) {
                        auto str = GetString(
                            CM_STR_LAND_TOOLTIPS_STATION_CARGO,
                            cs->name,
                            cargoid,
                            ge.CargoTotalCount(),
                            ToPercent8(ge.rating)
                        );

                        this->DrawSpriteIcons(cs->GetCargoIcon(), ir.left, ir.top + icon_ofs);
                        DrawString(ir.left + icons_width, ir.right, ir.top + text_ofs, str);
                        ir.top += line_height;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    void DrawSpriteIcons(SpriteID sprite, int left, int top) const
    {
        uint step = ScaleGUITrad(10);
        for(int i = 0; i < 3; i++) {
            DrawSprite(sprite, PAL_NONE, left + i * step, top);
        }
    }
};

void ShowLandTooltips(TileIndex tile, Window *parent) {
    static TileIndex last_tooltip_tile = INVALID_TILE;
    if (tile == last_tooltip_tile) return;
    last_tooltip_tile = tile;

    if (tile == INVALID_TILE) {
        CloseWindowById(WindowClass::CmLandTooltips, 0);
        return;
    }

    uint param = 0;
    switch (GetTileType(tile)) {
        case TileType::House: {
            if (true) {
                const HouseID house = GetHouseType(tile);
                param = ((house & 0xFFFF) << 16) | to_underlying(TileType::House);
            }
            break;
        }
        case TileType::Industry: {
            if (true) {
                const Industry *ind = Industry::GetByTile(tile);
                // if(ind->produced_cargo[0] == CT_INVALID && ind->produced_cargo[1] == CT_INVALID) return;
                param = ((ind->index.base() & 0xFFFF) << 16) | to_underlying(TileType::Industry);
            }
            break;
        }
        case TileType::Station: {
            if (true) {
                if (IsRailWaypoint(tile) || HasTileWaterGround(tile)) break;
                const Station *st = Station::GetByTile(tile);
                param |= ((st->index.base() & 0xFFFF) << 16) | to_underlying(TileType::Station);
                break;
            }
        }
        default:
            break;
    }
    CloseWindowById(WindowClass::CmLandTooltips, 0);

    if (param == 0) return;
    new LandTooltipsWindow(parent, param);
}

/* copied from window.cpp */
// static bool MayBeShown(const Window *w)
// {
//     /* If we're not modal, everything is okay. */
//     if (!HasModalProgress()) return true;

//     switch (w->window_class) {
//         case WC_MAIN_WINDOW:    ///< The background, i.e. the game.
//         case WC_MODAL_PROGRESS: ///< The actual progress window.
//         case WC_CONFIRM_POPUP_QUERY: ///< The abort window.
//             return true;

//         default:
//             return false;
//     }
// }

Window *FindHoverableWindowFromPt(int x, int y)
{
    for (Window *w : Window::IterateFromFront()) {
        if (MayBeShown(w) && IsInsideBS(x, w->left, w->width) && IsInsideBS(y, w->top, w->height)
                && w->window_class != WindowClass::CmLandTooltips
                && w->window_class != WindowClass::StationRatingTooltip) {
            return w;
        }
    }

    return nullptr;
}

} // namespace citymania
