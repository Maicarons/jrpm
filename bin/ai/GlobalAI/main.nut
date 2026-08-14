/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 */

/**
 * @file main.nut Entry point of the GlobalAI example AI.
 *
 * This AI does not build anything; it demonstrates how an AI can use the
 * whole-game perception API (AIGlobal) to observe every company and the
 * whole map, and how the access is controlled.
 *
 * Enable "game.script.allow_global_ai_access" in the game settings for this
 * AI to gain access; GameScripts always have access.
 */

class GlobalAI extends AIController {
	function Start()
	{
		AICompany.SetName("GlobalAI Demo");

		if (!AIGlobal.IsGlobalAccessAllowed()) {
			AILog.Info("Global API not allowed. Enable the game setting 'game.script.allow_global_ai_access' to let this AI see the whole game.", AILog.INFO);
			return;
		}

		AILog.Info("Whole-game perception demo:", AILog.INFO);

		local companies = AIGlobalCompanyList();
		AILog.Info("Companies in game: " + companies.Count(), AILog.INFO);

		foreach (company_id, value in companies) {
			local name    = AIGlobal.GetCompanyName(company_id);
			local balance = AIGlobal.GetCompanyBankBalance(company_id);
			local loan    = AIGlobal.GetCompanyLoan(company_id);
			local worth   = AIGlobal.GetCompanyValue(company_id);
			local rating  = AIGlobal.GetCompanyPerformanceRating(company_id);
			local trains  = AIGlobal.GetCompanyVehicleCount(company_id, AIGlobal.VT_TRAIN);
			local road    = AIGlobal.GetCompanyVehicleCount(company_id, AIGlobal.VT_ROAD);
			local ships   = AIGlobal.GetCompanyVehicleCount(company_id, AIGlobal.VT_SHIP);
			local planes  = AIGlobal.GetCompanyVehicleCount(company_id, AIGlobal.VT_AIRCRAFT);
			local stations = AIGlobal.GetCompanyStationCount(company_id);

			AILog.Info("  " + name + ": balance=" + balance
				+ " loan=" + loan + " value=" + worth + " rating=" + rating
				+ " vehicles(t/r/s/a)=" + trains + "/" + road + "/" + ships + "/" + planes
				+ " stations=" + stations, AILog.INFO);
		}

		AILog.Info("Map size: " + AIGlobal.GetMapSizeX() + "x" + AIGlobal.GetMapSizeY()
			+ ", year: " + AIGlobal.GetYear() + ", date: " + AIGlobal.GetDate(), AILog.INFO);

		/*
		 * This is where an AI's own control logic would plug in: for example,
		 * only expand when the own company has the lowest performance rating,
		 * or target cargo types that no other company transports.
		 */
		while (true) {
			AIController.Sleep(100);
		}
	}
}
