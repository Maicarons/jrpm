/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 */

/**
 * @file info.nut Information about the GlobalAI example AI.
 *
 * GlobalAI demonstrates the whole-game perception API (AIGlobal) that is
 * added by this patch pack. It reads data about every company and the whole
 * map, which normal AIs cannot do.
 *
 * The AI's access to the Global API is controlled by the game setting
 * "game.script.allow_global_ai_access" (disabled by default).
 */

class GlobalAIInfo extends AIInfo {
	function GetAuthor()      { return "MaiBot"; }
	function GetName()        { return "GlobalAI"; }
	function GetDescription() { return "Example AI demonstrating the whole-game Global API."; }
	function GetVersion()     { return 1; }
	function GetDate()        { return "2026-08-14"; }
	function CreateInstance() { return "GlobalAI"; }
	function GetShortName()   { return "GLAI"; }
	function GetAPIVersion()  { return "1.11"; }
	function GetMinVersionToLoad() { return 1; }
	function GetSettings()    { return []; }
}

RegisterAI(GlobalAIInfo());
