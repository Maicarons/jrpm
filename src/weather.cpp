/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file weather.cpp Handling of the (cosmetic) weather state. */

#include "stdafx.h"
#include "weather.h"
#include "openttd.h"
#include "settings_type.h"
#include "date_func.h"
#include "gfx_func.h"

#include "safeguards.h"

/** Weather forced by the sandbox cheat. */
RainForcing _rain_forcing = RainForcing::Auto;

/** Current world shading level; 0 = clear skies, RAIN_SHADE_LEVELS = fully darkened. */
uint8_t _rain_shade_level = 0;

/** Automatic rain state. */
static bool _rain_auto_raining = false;
/** Start tick of the current automatic weather period. */
static StateTicks _rain_period_start{};
/** Deterministic RNG for weather changes; never touches the synced random state. */
static uint32_t _rain_seed = 0;
/** Start tick of the current shading step, for the gradual transitions. */
static StateTicks _rain_shade_step_start{};

/** Approximate length of one automatic weather period, in game ticks. */
static constexpr StateTicksDelta RAIN_PERIOD_TICKS{31 * 74}; ///< Roughly one game month.
/** Number of shading levels between clear skies and full rain darkness. */
static constexpr uint8_t RAIN_SHADE_LEVELS = 8;
/** Game ticks between shading steps; a full transition takes about RAIN_SHADE_LEVELS * 32 ticks. */
static constexpr StateTicksDelta RAIN_SHADE_STEP_TICKS{32};
/** The shading masks: nominator (of 256) per level, index 0 = first shade level, last = full rain darkness. */
static constexpr uint8_t RAIN_SHADE_NOMS[RAIN_SHADE_LEVELS] = { 245, 234, 223, 212, 200, 189, 178, 167 };

uint8_t RainShadeNom()
{
	return _rain_shade_level == 0 ? 256 : RAIN_SHADE_NOMS[_rain_shade_level - 1];
}

uint32_t RainSeed()
{
	return _rain_seed;
}

uint8_t RainShadeProgress()
{
	return _rain_shade_level * 255 / RAIN_SHADE_LEVELS;
}

static uint32_t RainNextRandom()
{
	_rain_seed = _rain_seed * 1103515245u + 12345u;
	return _rain_seed >> 16;
}

/**
 * Advance the automatic weather state. Called once per game tick; purely
 * cosmetic and local, so it must not touch any synchronised game state.
 */
void WeatherTick()
{
	if (_game_mode == GameMode::Menu) return;

	/* Move the world shading one level towards the target of the current weather. */
	if (_state_ticks - _rain_shade_step_start >= RAIN_SHADE_STEP_TICKS) {
		_rain_shade_step_start = _state_ticks;
		uint8_t target = IsRainActive() ? RAIN_SHADE_LEVELS : 0;
		if (_rain_shade_level != target) {
			_rain_shade_level += (_rain_shade_level < target) ? 1 : -1;
			MarkWholeScreenDirty();
		}
	}

	/* The rain overlay animates, so keep the screen redrawn while rain is
	 * visible (including while it fades in or out). */
	if (_rain_shade_level != 0 || (IsRainActive() && _rain_shade_level != RAIN_SHADE_LEVELS)) MarkWholeScreenDirty();

	if (_rain_forcing != RainForcing::Auto) return;
	if (_settings_game.difficulty.rain) {
		/* Seed once from the map seed, keeping all clients consistent. */
		if (_rain_seed == 0) _rain_seed = _settings_game.game_creation.generation_seed | 1;

		if (_state_ticks - _rain_period_start >= RAIN_PERIOD_TICKS) {
			_rain_period_start = _state_ticks;
			/* Rain chance per period: 20% when clear, 50% chance to clear when raining. */
			uint32_t chance = _rain_auto_raining ? 50 : 20;
			if ((RainNextRandom() % 100) < chance) {
				_rain_auto_raining = !_rain_auto_raining;
			}
		}
	} else {
		_rain_auto_raining = false;
	}
}

/**
 * Is it raining right now?
 * @return true iff rain is currently active.
 */
bool IsRainActive()
{
	if (_game_mode == GameMode::Menu) return false;
	switch (_rain_forcing) {
		case RainForcing::Raining: return true;
		case RainForcing::Sunny: return false;
		default: return _settings_game.difficulty.rain && _rain_auto_raining;
	}
}
