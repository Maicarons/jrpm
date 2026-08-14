/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file script_global.cpp Implementation of ScriptGlobal and ScriptGlobalCompanyList. */

#include "../../stdafx.h"

#include "script_global.hpp"
#include "script_companymode.hpp"
#include "../../company_base.h"
#include "../../company_func.h"
#include "../../date_func.h"
#include "../../economy_base.h"
#include "../../group.h"
#include "../../map_func.h"
#include "../../settings_type.h"
#include "../../station_base.h"
#include "../../string_func.h"
#include "../../strings_func.h"
#include "table/strings.h"

#include "../../safeguards.h"

/**
 * Check whether the current script is allowed to use the Global API.
 * @return True when the API can be used.
 */
static bool IsGlobalAccessAllowed()
{
	return ScriptCompanyMode::IsDeity() || _settings_game.script.allow_global_ai_access;
}

/**
 * Resolve a script company ID to an internal company ID.
 * @param company The script company ID.
 * @return The internal company ID, or CompanyID::Invalid() when it cannot be resolved.
 */
static ::CompanyID ResolveCompany(ScriptCompany::CompanyID company)
{
	if (company == ScriptCompany::COMPANY_SELF) {
		if (!::Company::IsValidID(_current_company)) return ::CompanyID::Invalid();
		return _current_company;
	}
	if (company == ScriptCompany::COMPANY_SPECTATOR || company == ScriptCompany::COMPANY_INVALID) {
		return ::CompanyID::Invalid();
	}
	if (company < ScriptCompany::COMPANY_FIRST || company >= ScriptCompany::COMPANY_LAST) {
		return ::CompanyID::Invalid();
	}
	return static_cast<::CompanyID>(company);
}

/* static */ bool ScriptGlobal::IsGlobalAccessAllowed()
{
	return ::IsGlobalAccessAllowed();
}

/* static */ SQInteger ScriptGlobal::GetCompanyCount()
{
	if (!::IsGlobalAccessAllowed()) return 0;

	SQInteger count = 0;
	for (const Company *c : Company::Iterate()) {
		(void)c;
		count++;
	}
	return count;
}

/* static */ SQInteger ScriptGlobal::GetMapSizeX()
{
	if (!::IsGlobalAccessAllowed()) return 0;
	return (SQInteger)::Map::SizeX();
}

/* static */ SQInteger ScriptGlobal::GetMapSizeY()
{
	if (!::IsGlobalAccessAllowed()) return 0;
	return (SQInteger)::Map::SizeY();
}

/* static */ SQInteger ScriptGlobal::GetDate()
{
	if (!::IsGlobalAccessAllowed()) return 0;
	return (SQInteger)EconTime::CurDate().base();
}

/* static */ SQInteger ScriptGlobal::GetYear()
{
	if (!::IsGlobalAccessAllowed()) return 0;
	return (SQInteger)CalTime::CurYear().base();
}

/* static */ std::optional<std::string> ScriptGlobal::GetCompanyName(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return std::nullopt;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return std::nullopt;

	return ::StrMakeValid(::GetString(STR_COMPANY_NAME, cid), {});
}

/* static */ Money ScriptGlobal::GetCompanyBankBalance(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;
	/* If we return INT64_MAX as usual, overflows may occur in the script. So return a smaller value. */
	if (_settings_game.difficulty.infinite_money) return INT32_MAX;

	return GetAvailableMoney(cid);
}

/* static */ Money ScriptGlobal::GetCompanyLoan(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;

	return ::Company::Get(cid)->current_loan;
}

/* static */ Money ScriptGlobal::GetCompanyValue(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;

	return ::Company::Get(cid)->old_economy[0].company_value;
}

/* static */ SQInteger ScriptGlobal::GetCompanyPerformanceRating(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;

	return (SQInteger)::Company::Get(cid)->old_economy[0].performance_history;
}

/* static */ SQInteger ScriptGlobal::GetCompanyVehicleCount(ScriptCompany::CompanyID company, VehicleType vehicle_type)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	if (vehicle_type < VT_TRAIN || vehicle_type > VT_AIRCRAFT) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;

	return (SQInteger)::Company::Get(cid)->group_all[static_cast<::VehicleType>(vehicle_type)].num_vehicle;
}

/* static */ SQInteger ScriptGlobal::GetCompanyStationCount(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return -1;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return -1;

	SQInteger count = 0;
	for (const Station *s : Station::Iterate()) {
		if (s->owner == cid) count++;
	}
	return count;
}

/* static */ bool ScriptGlobal::IsCompanyBankrupt(ScriptCompany::CompanyID company)
{
	if (!::IsGlobalAccessAllowed()) return false;

	::CompanyID cid = ResolveCompany(company);
	if (!::Company::IsValidID(cid)) return false;

	return ::Company::Get(cid)->months_of_bankruptcy != 0;
}

ScriptGlobalCompanyList::ScriptGlobalCompanyList(HSQUIRRELVM vm)
{
	if (!::IsGlobalAccessAllowed()) return;

	ScriptList::FillList<Company>(vm, this);
}
