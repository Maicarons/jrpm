/** @file jrpm_cargo_table.cpp
 * Company cargo details window: per-cargo delivered amounts (total /
 * current month) for a company.
 *
 * Adapted from citymania-org/cmclient (cm_cargo_table_gui), amount column
 * only (jrpm does not track per-cargo income without a savegame change).
 */

#include "stdafx.h"
#include "company_base.h"
#include "company_func.h"
#include "cargotype.h"
#include "gfx_func.h"
#include "strings_func.h"
#include "window_func.h"
#include "window_gui.h"
#include "window_type.h"
#include "sprite.h"
#include "zoom_func.h"

#include "table/sprites.h"
#include "table/strings.h"
#include "safeguards.h"

static const uint CT_LINESPACE = 3;   ///< Vertical space for a horizontal (sub-)total line.
static const uint CT_ICON_MARGIN = 2; ///< Space between the cargo icon and the text.

/** Widgets of the #CompanyCargosWindow class. */
enum CompanyCargosWidgets : WidgetID {
	WID_CT_CAPTION,        ///< Caption.
	WID_CT_HEADER_CARGO,   ///< Header: cargo names (click to toggle period).
	WID_CT_HEADER_AMOUNT,  ///< Header: amount.
	WID_CT_LIST,           ///< Cargo names list.
	WID_CT_AMOUNT,         ///< Delivered amounts list.
};

/** Which period the amounts show. */
enum CargoPeriod {
	CP_TOTAL,  ///< Total delivered (current quarter economy).
	CP_MONTH,  ///< Last month delivered.
};

struct CompanyCargosWindow : Window {
	CargoPeriod period = CP_TOTAL;

	CompanyCargosWindow(WindowDesc &desc, WindowNumber window_number) : Window(desc)
	{
		this->InitNested(window_number);
		this->owner = static_cast<Owner>(this->window_number);
	}

	std::string GetWidgetString(WidgetID widget, StringID stringid) const override
	{
		if (widget == WID_CT_CAPTION) {
			return GetString(STR_JRPM_CARGOS_CAPTION, static_cast<CompanyID>(this->window_number));
		}
		return this->Window::GetWidgetString(widget, stringid);
	}

	void OnClick([[maybe_unused]] Point pt, WidgetID widget, [[maybe_unused]] int click_count) override
	{
		if (widget != WID_CT_HEADER_CARGO) return;
		this->period = (this->period == CP_TOTAL) ? CP_MONTH : CP_TOTAL;
		this->SetDirty();
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, const Dimension &padding, Dimension &fill, Dimension &resize) override
	{
		Dimension icon_size = this->GetMaxIconSize();
		int line_height = std::max(GetCharacterHeight(FontSize::Normal), static_cast<int>(icon_size.height));
		int icon_space = icon_size.width + ScaleGUITrad(CT_ICON_MARGIN);
		switch (widget) {
			case WID_CT_HEADER_AMOUNT:
			case WID_CT_AMOUNT:
				size.width = std::max<uint>(size.width, static_cast<uint>(ScaleGUITrad(108)));
				break;
			case WID_CT_HEADER_CARGO:
			case WID_CT_LIST:
				for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
					size.width = std::max(GetStringBoundingBox(cs->name).width + icon_space, size.width);
				}
				break;
			default:
				break;
		}
		switch (widget) {
			case WID_CT_HEADER_AMOUNT:
				size.height = GetCharacterHeight(FontSize::Normal);
				break;
			case WID_CT_AMOUNT:
			case WID_CT_LIST:
				size.height = static_cast<uint>(_sorted_standard_cargo_specs.size()) * line_height + CT_LINESPACE + GetCharacterHeight(FontSize::Normal);
				break;
			default:
				break;
		}
		size.width += padding.width;
		size.height += padding.height;
	}

	Dimension GetMaxIconSize() const
	{
		Dimension size{0, 0};
		for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
			Dimension icon_size = GetSpriteSize(cs->GetCargoIcon());
			size.width = std::max(size.width, icon_size.width);
			size.height = std::max(size.height, icon_size.height);
		}
		return size;
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		const Company *c = Company::Get(static_cast<CompanyID>(this->window_number));
		int y = r.top;
		Dimension max_icon_size = this->GetMaxIconSize();
		int line_height = std::max(GetCharacterHeight(FontSize::Normal), static_cast<int>(max_icon_size.height));
		int icon_space = max_icon_size.width + ScaleGUITrad(CT_ICON_MARGIN);
		int text_y_ofs = line_height - GetCharacterHeight(FontSize::Normal);

		switch (widget) {
			case WID_CT_HEADER_CARGO:
				break;
			case WID_CT_HEADER_AMOUNT:
				DrawString(r.left, r.right, y, GetString(this->period == CP_TOTAL ? STR_JRPM_CARGOS_HEADER_TOTAL : STR_JRPM_CARGOS_HEADER_MONTH), TextColour::FromString, SA_RIGHT);
				break;

			case WID_CT_LIST: {
				int rect_x = r.left + WidgetDimensions::scaled.framerect.left;
				for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
					Dimension icon_size = GetSpriteSize(cs->GetCargoIcon());
					DrawSprite(cs->GetCargoIcon(), PAL_NONE,
							r.left + max_icon_size.width - icon_size.width,
							y + (line_height - static_cast<int>(icon_size.height)) / 2);
					DrawString(rect_x + icon_space, r.right, y + text_y_ofs, GetString(STR_JRPM_CARGOS_NAME, cs->name));
					y += line_height;
				}
				break;
			}
			case WID_CT_AMOUNT: {
				uint32_t total = 0;
				for (const CargoSpec *cs : _sorted_standard_cargo_specs) {
					auto &economy = (this->period == CP_MONTH && c->num_valid_stat_ent > 0) ? c->old_economy[0] : c->cur_economy;
					uint32_t amount = economy.delivered_cargo[cs->Index()];
					total += amount;
					DrawString(r.left, r.right, y + text_y_ofs, GetString(STR_JRPM_CARGOS_UNITS, amount), TextColour::FromString, SA_RIGHT);
					y += line_height;
				}
				GfxFillRect(r.left, y + 1, r.right, y + 1, PC_BLACK);
				y += CT_LINESPACE;
				DrawString(r.left, r.right, y, GetString(STR_JRPM_CARGOS_UNITS_TOTAL, total), TextColour::FromString, SA_RIGHT);
				break;
			}
			default:
				break;
		}
	}
};

static constexpr std::initializer_list<NWidgetPart> _nested_cargos_widgets = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, Colours::Grey),
		NWidget(WWT_CAPTION, Colours::Grey, WID_CT_CAPTION),
		NWidget(WWT_SHADEBOX, Colours::Grey),
		NWidget(WWT_STICKYBOX, Colours::Grey),
	EndContainer(),
	NWidget(WWT_PANEL, Colours::Grey), SetResize(1, 1),
		NWidget(NWID_HORIZONTAL), SetPadding(2, 2, 2, 2), SetPIP(0, 9, 0),
			NWidget(WWT_PUSHTXTBTN, Colours::Grey, WID_CT_HEADER_CARGO), SetFill(1, 0), SetPadding(2, 2, 2, 2), SetStringTip(STR_JRPM_CARGOS_HEADER_CARGO, STR_JRPM_CARGOS_HEADER_CARGO_TIP),
			NWidget(WWT_TEXT, Colours::Grey, WID_CT_HEADER_AMOUNT), SetMinimalSize(108, 16), SetFill(1, 0), SetPadding(2, 2, 2, 2),
		EndContainer(),
	EndContainer(),
	NWidget(WWT_PANEL, Colours::Grey), SetResize(1, 1),
		NWidget(NWID_HORIZONTAL), SetPadding(2, 2, 2, 2), SetPIP(0, 9, 0),
			NWidget(WWT_EMPTY, Colours::Grey, WID_CT_LIST), SetFill(1, 0), SetPadding(2, 2, 2, 2), SetResize(1, 1),
			NWidget(WWT_EMPTY, Colours::Grey, WID_CT_AMOUNT), SetMinimalSize(108, 0), SetFill(1, 0), SetPadding(2, 2, 2, 2), SetResize(1, 1),
		EndContainer(),
	EndContainer(),
};

static WindowDesc _cargos_desc(__FILE__, __LINE__,
	WindowPosition::Automatic, "company_cargos", 0, 0,
	WindowClass::CompanyCargos, WindowClass::None,
	WindowDefaultFlag::Construction,
	_nested_cargos_widgets
);

void ShowCompanyCargos(CompanyID company)
{
	if (!Company::IsValidID(company)) return;
	AllocateWindowDescFront<CompanyCargosWindow>(_cargos_desc, company);
}
