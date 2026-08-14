/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file script_global.hpp Whole-game perception API for AIs and GameScripts. */

#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"
#include "script_list.hpp"
#include "../../economy_type.h"
#include "../../vehicle_type.h"

/**
 * Class that provides a whole-game perception API.
 *
 * Unlike the regular AI API, which is limited to what the AI's own company
 * can observe, this class exposes data about every company and the whole map
 * (company finances, vehicle counts, performance ratings, map size, ...).
 *
 * For AIs this access is controlled by the game setting
 * \c game.script.allow_global_ai_access (disabled by default). GameScripts
 * always have access, as they run in "deity mode".
 *
 * @api ai game
 */
class ScriptGlobal : public ScriptObject {
public:
	/** Vehicle types, matching ScriptVehicle::VehicleType values. */
	enum VehicleType {
		VT_TRAIN    = ::VehicleType::Train,    ///< Trains.
		VT_ROAD     = ::VehicleType::Road,     ///< Road vehicles.
		VT_SHIP     = ::VehicleType::Ship,     ///< Ships.
		VT_AIRCRAFT = ::VehicleType::Aircraft, ///< Aircraft.
	};

	/**
	 * Check whether the current script is allowed to use this API.
	 * @return True when the API can be used (GameScript, or the game setting allows it).
	 * @api ai game
	 */
	static bool IsGlobalAccessAllowed();

	/**
	 * Get the number of companies currently in the game.
	 * @return The number of companies, or 0 when the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetCompanyCount();

	/**
	 * Get the width of the map, in tiles.
	 * @return The width of the map, or 0 when the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetMapSizeX();

	/**
	 * Get the height of the map, in tiles.
	 * @return The height of the map, or 0 when the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetMapSizeY();

	/**
	 * Get the current game date.
	 * @return The current date, or 0 when the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetDate();

	/**
	 * Get the current calendar year.
	 * @return The current year, or 0 when the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetYear();

	/**
	 * Get the name of a company.
	 * @param company The company to get the name of.
	 * @return The name of the company, or nullopt when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static std::optional<std::string> GetCompanyName(ScriptCompany::CompanyID company);

	/**
	 * Get the bank balance of a company.
	 * @param company The company to get the balance of.
	 * @return The bank balance of the company, or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static Money GetCompanyBankBalance(ScriptCompany::CompanyID company);

	/**
	 * Get the current loan of a company.
	 * @param company The company to get the loan of.
	 * @return The current loan, or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static Money GetCompanyLoan(ScriptCompany::CompanyID company);

	/**
	 * Get the estimated value of a company.
	 * @param company The company to get the value of.
	 * @return The value of the company, or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static Money GetCompanyValue(ScriptCompany::CompanyID company);

	/**
	 * Get the performance rating of a company.
	 * @param company The company to get the rating of.
	 * @return The performance rating (0..1000), or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetCompanyPerformanceRating(ScriptCompany::CompanyID company);

	/**
	 * Get the number of vehicles of the given type owned by a company.
	 * @param company      The company to count vehicles of.
	 * @param vehicle_type The type of vehicle to count.
	 * @return The number of vehicles, or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetCompanyVehicleCount(ScriptCompany::CompanyID company, VehicleType vehicle_type);

	/**
	 * Get the number of stations owned by a company.
	 * @param company The company to count stations of.
	 * @return The number of stations, or -1 when the company does not exist
	 *         or the API is not accessible.
	 * @api ai game
	 */
	static SQInteger GetCompanyStationCount(ScriptCompany::CompanyID company);

	/**
	 * Check whether a company is currently in bankruptcy.
	 * @param company The company to check.
	 * @return True when the company is bankrupt, or false when the company does
	 *         not exist or the API is not accessible.
	 * @api ai game
	 */
	static bool IsCompanyBankrupt(ScriptCompany::CompanyID company);
};

/**
 * Creates a list of all companies currently in the game.
 * @api ai game
 * @ingroup ScriptList
 */
class ScriptGlobalCompanyList : public ScriptList {
public:
#ifdef DOXYGEN_API
	/**
	 * Create a list of all companies currently in the game.
	 */
	ScriptGlobalCompanyList();
#else
	/**
	 * The constructor wrapper from Squirrel.
	 */
	ScriptGlobalCompanyList(HSQUIRRELVM vm);
#endif /* DOXYGEN_API */
};

#endif /* SCRIPT_GLOBAL_HPP */
