/** @file jrpm_console_commands.cpp
 * Batch 0 console commands adapted from citymania-org/cmclient.
 *
 * - cmgamespeed [n] : set the game speed (100 = 1x, 0 = infinite)
 * - cmgamestats     : print the total number of vehicles
 * - cmexport        : export house/cargo/engine data as JSON (openttd.json)
 * - cmtreemap <f>   : load a heightmap-like image and plant trees from it
 */

#include "stdafx.h"
#include <fstream>
#include "cargotype.h"
#include "core/tinystring_type.hpp"
#include "core/enum_type.hpp"
#include "core/string_consumer.hpp"
#include "heightmap.h"
#include "strings_func.h"
#include "command_func.h"
#include "command_type.h"
#include "console_func.h"
#include "console_internal.h"
#include "engine_base.h"
#include "gfx_func.h"
#include "house.h"
#include "map_func.h"
#include "tree_cmd.h"
#include "vehicle_base.h"

#include "safeguards.h"

static bool ConGameSpeed(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Changes game speed. Usage: 'cmgamespeed [n]' (100 is 1x, 0 is infinite).");
		return true;
	}
	if (argv.size() > 2) return false;

	uint new_speed = 100;
	if (argv.size() > 1) {
		auto t = ParseInteger(argv[1]);
		if (!t.has_value()) {
			IConsolePrint(CC_ERROR, "Invalid number '{}'", argv[1]);
			return true;
		}
		new_speed = *t;
	}
	extern uint16_t _game_speed;
	_game_speed = new_speed;
	IConsolePrint(CC_DEFAULT, "Game speed set to {} (100 is 1x).", new_speed);
	return true;
}

static bool ConGameStats(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Prints total number of vehicles. Usage: 'cmgamestats'.");
		return true;
	}

	uint num_trains = 0, num_rvs = 0, num_ships = 0, num_aircraft = 0;

	for (Vehicle *v : Vehicle::Iterate()) {
		if (!v->IsPrimaryVehicle()) continue;
		switch (v->type) {
			default: break;
			case VehicleType::Train: num_trains++; break;
			case VehicleType::Road: num_rvs++; break;
			case VehicleType::Ship: num_ships++; break;
			case VehicleType::Aircraft: num_aircraft++; break;
		}
	}

	IConsolePrint(CC_INFO, "Number of trains: {}", num_trains);
	IConsolePrint(CC_INFO, "Number of road vehicles: {}", num_rvs);
	IConsolePrint(CC_INFO, "Number of ships: {}", num_ships);
	IConsolePrint(CC_INFO, "Number of aircraft: {}", num_aircraft);
	IConsolePrint(CC_INFO, "Total number of vehicles: {}", num_trains + num_rvs + num_ships + num_aircraft);

	return true;
}

/* ---------------------------------------------------------------------- */
/* cmexport: minimal JSON data export (house specs, cargo specs, engines)  */
/* ---------------------------------------------------------------------- */

class JsonWriter {
protected:
	int i = 0;
	bool no_comma = true;
	bool js = false;

public:
	std::ofstream f;

	JsonWriter(const std::string &fname, bool js = false)
	{
		this->js = js;
		f.open(fname.c_str());
		if (this->js) f << "OPENTTD = {";
		no_comma = true;
	}

	~JsonWriter()
	{
		this->ident(false);
		if (this->js) f << "}" << std::endl;
		f.close();
	}

	void ident(bool comma = true)
	{
		if (comma && !no_comma) f << ",";
		no_comma = false;
		f << std::endl;
		for (int j = 0; j < i; j++) f << "  ";
	}

	void key(const char *k)
	{
		const char *kn;
		for (kn = k + strlen(k); kn >= k && *kn != '>' && *kn != '.'; kn--) {}
		kn++;
		this->ident();
		f << "\"" << kn << "\": ";
	}

	void value(bool val) { f << (val ? "true" : "false"); }
	void value(unsigned int val) { f << val; }
	void value(uint64_t val) { f << val; }
	void value(Money val) { f << val; }
	void value(int val) { f << val; }
	void value(const char *v) { f << "\"" << v << "\""; }
	void value(const std::string &s) { f << "\"" << s << "\""; }
	void value(const TinyString &s) { f << "\"" << s.c_str() << "\""; }

	template<typename T>
	void kv(const char *k, T v)
	{
		key(k);
		if constexpr (std::is_enum_v<T>) {
			value(static_cast<uint64_t>(::to_underlying(v)));
		} else {
			value(v);
		}
	}

	void ks(const char *k, StringID s)
	{
		key(k);
		value(GetString(s));
	}

	void begin_dict_with_key(const char *k)
	{
		key(k);
		f << "{";
		no_comma = true;
		i++;
	}

	void begin_dict()
	{
		this->ident();
		f << "{";
		no_comma = true;
		i++;
	}

	void end_dict()
	{
		i--;
		this->ident(false);
		f << "}";
	}

	void begin_list_with_key(const char *k)
	{
		key(k);
		f << "[";
		no_comma = true;
		i++;
	}

	void end_list()
	{
		i--;
		this->ident(false);
		f << "]";
	}
};

static void WriteHouseSpecInfo(JsonWriter &j)
{
	j.begin_list_with_key("house_specs");
	for (uint i = 0; i < NUM_HOUSES; i++) {
		const HouseSpec *hs = HouseSpec::Get(i);
		j.begin_dict();
		j.kv("min_year", hs->min_year.base());
		j.kv("max_year", hs->max_year.base());
		j.kv("population", hs->population);
		j.kv("removal_cost", hs->removal_cost);
		j.kv("name", GetString(hs->building_name));
		j.kv("mail_generation", hs->mail_generation);
		j.kv("enabled", hs->enabled);
		j.end_dict();
	}
	j.end_list();
}

static void WriteCargoSpecInfo(JsonWriter &j)
{
	j.begin_list_with_key("cargo_specs");
	for (const CargoSpec *cs : CargoSpec::Iterate()) {
		j.begin_dict();
		j.kv("id", cs->bitnum);
		j.kv("initial_payment", cs->initial_payment);
		j.kv("transit_periods_1", cs->transit_periods[0]);
		j.kv("transit_periods_2", cs->transit_periods[1]);
		j.kv("weight", cs->weight);
		j.kv("multiplier", cs->multiplier);
		j.kv("is_freight", cs->is_freight);
		j.kv("sprite", cs->sprite);
		j.ks("name", cs->name);
		j.ks("name_single", cs->name_single);
		j.ks("units_volume", cs->units_volume);
		j.ks("quantifier", cs->quantifier);
		j.ks("abbrev", cs->abbrev);
		j.kv("label", cs->label.base());
		j.end_dict();
	}
	j.end_list();
}

static void WriteEngineInfo(JsonWriter &j)
{
	j.begin_list_with_key("engines");
	for (const Engine *e : Engine::Iterate()) {
		if (e->type != VehicleType::Train) continue;
		j.begin_dict();
		j.kv("index", e->index.base());
		j.kv("name", e->name);
		j.kv("cost", e->GetCost());
		j.kv("running_cost", e->GetRunningCost());
		{
			const RailVehicleInfo *rvi = &e->VehInfo<RailVehicleInfo>();
			j.begin_dict_with_key("rail");
			j.kv("image_index", rvi->image_index);
			j.kv("railveh_type", rvi->railveh_type);
			j.kv("max_speed", rvi->max_speed);
			j.kv("power", rvi->power);
			j.kv("weight", rvi->weight);
			j.kv("running_cost", rvi->running_cost);
			j.kv("running_cost_class", rvi->running_cost_class);
			j.kv("engclass", rvi->engclass);
			j.kv("tractive_effort", rvi->tractive_effort);
			j.kv("air_drag", rvi->air_drag);
			j.kv("capacity", rvi->capacity);
			j.end_dict();
		}
		j.end_dict();
	}
	j.end_list();
}

static void ExportOpenttdData(const std::string &filename)
{
	JsonWriter j(filename, true);
	WriteHouseSpecInfo(j);
	WriteCargoSpecInfo(j);
	WriteEngineInfo(j);
}

static bool ConExport(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Exports various game data in json format to openttd.json file. Usage: 'cmexport'.");
		return true;
	}

	ExportOpenttdData("openttd.json");
	IConsolePrint(CC_DEFAULT, "Data successfully saved to openttd.json");
	return true;
}

/* ---------------------------------------------------------------------- */
/* cmtreemap: plant trees according to a heightmap-like image              */
/* ---------------------------------------------------------------------- */

static bool ConTreeMap(std::span<std::string_view> argv)
{
	if (argv.empty()) {
		IConsolePrint(CC_HELP, "Loads a heightmap-like file and plants trees according to it, values 0-256 are scaled to 0-4 trees.");
		IConsolePrint(CC_HELP, "Usage: 'cmtreemap <file>'");
		IConsolePrint(CC_HELP, "Default lookup path is in scenario/heightmap in your openttd directory");
		return true;
	}

	if (argv.size() != 2) return false;

	std::string_view filename = argv[1];

	if (_game_mode != GameMode::Editor) {
		IConsolePrint(CC_ERROR, "This command is only available in scenario editor.");
		return true;
	}

	if (filename.size() < 4) {
		IConsolePrint(CC_ERROR, "Unknown treemap extension, should be .bmp or .png.");
		return true;
	}

	auto ext = filename.substr(filename.length() - 4, 4);
	DetailedFileType dft;
	if (ext == ".bmp") dft = DetailedFileType::HeightmapBmp;
#ifdef WITH_PNG
	else if (ext == ".png") dft = DetailedFileType::HeightmapPng;
#endif
	else {
		IConsolePrint(CC_ERROR, "Unknown treemap extension {}, should be .bmp or .png.", ext);
		return true;
	}

	uint x, y;
	std::vector<uint8_t> map;

	if (!ReadHeightMap(dft, filename, &x, &y, &map)) {
		IConsolePrint(CC_ERROR, "Failed to load '{}'.", filename);
		return true;
	}

	uint planted = 0;
	for (TileIndex tile{0}; tile < Map::Size(); tile += 1) {
		int mx = static_cast<int>(x) - x * static_cast<int>(TileX(tile)) / static_cast<int>(Map::SizeX()) - 1;
		int my = y * TileY(tile) / Map::SizeY();
		if (mx < 0 || mx >= static_cast<int>(x) || my < 0 || my >= static_cast<int>(y)) continue;
		auto t = map[mx + my * x];
		auto tree_count = std::min(t / 51, 4);
		for (auto i = 0; i < tree_count; i++) {
			Command<Commands::PlantTree>::Post(tile, tile, TreeTypes{TreeType::TREE_INVALID}, 1, false);
			planted++;
		}
	}

	IConsolePrint(CC_DEFAULT, "Treemap '{}' applied: {} trees planted.", filename, planted);
	return true;
}

void RegisterJRPMBatch0Commands()
{
	IConsole::CmdRegister("cmgamespeed", ConGameSpeed);
	IConsole::CmdRegister("cmgamestats", ConGameStats);
	IConsole::CmdRegister("cmexport", ConExport);
	IConsole::CmdRegister("cmtreemap", ConTreeMap);
}
