/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file rain_overlay.cpp Screen-space scrolling rain overlay. */

#include "stdafx.h"
#include "rain_overlay.h"
#include "weather.h"
#include "blitter/factory.hpp"
#include "gfx_type.h"
#include "palette_func.h"
#include "spritecache.h"

#include <algorithm>
#include <chrono>
#include <deque>

#include "safeguards.h"

/** Side length (in screen pixels) of one rain texture tile. */
static constexpr int RAIN_TILE = 128;
/** Number of pre-generated animation frames per layer. */
static constexpr int RAIN_FRAMES = 6;
/** Number of parallax layers; far to near. */
static constexpr int RAIN_LAYERS = 3;

/** Parameters of one rain layer. */
struct RainLayerParams {
	int speed_x;        ///< Horizontal scroll speed, screen pixels per second.
	int speed_y;        ///< Vertical scroll speed, screen pixels per second (fall direction).
	int streak_count;   ///< Streaks per texture tile.
	int len_min;        ///< Shortest streak length in pixels.
	int len_extra;      ///< Maximum extra length.
	uint8_t base_alpha; ///< Base opacity of the streaks (before the rain fade is applied).
};

static constexpr RainLayerParams RAIN_LAYER_PARAMS[RAIN_LAYERS] = {
	/* Far layer: many thin, short, faint streaks, moving slowly. */
	{  12,  34, 26,  5,  5, 35 },
	/* Middle layer. */
	{  17,  50, 16,  9,  8, 55 },
	/* Near layer: fewer, longer and more opaque streaks, moving fast. */
	{  24,  70,  9, 14, 10, 75 },
};

/** One encoded frame of one layer. */
struct RainFrame {
	const Sprite *sprite;
	UniquePtrSpriteAllocator allocator;
};

/** All encoded frames, in layer-major order, plus what they were built for. */
struct RainOverlayCache {
	std::deque<RainFrame> frames;
	const Blitter *blitter = nullptr;
	uint8_t shade_level = 0xFF; ///< Shade level the current alpha scaling was made for.

	bool Matches(const Blitter *blitter, uint8_t shade_level) const
	{
		return this->blitter == blitter && this->shade_level == shade_level;
	}
};

static RainOverlayCache _rain_cache;

/** Cheap deterministic hash; all overlay randomness flows through this. */
static uint32_t RainHash(uint32_t a, uint32_t b, uint32_t c)
{
	uint32_t h = RainSeed();
	h ^= a * 0x9E3779B9u;
	h ^= b * 0x85EBCA6Bu;
	h ^= c * 0xC2B2AE35u;
	h ^= h >> 15;
	h *= 0x2545F491u;
	h ^= h >> 13;
	return h;
}

/** Milliseconds elapsed on the monotonic clock. */
static uint64_t RainNowMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
}

/**
 * Generate one texture tile: a pattern of slanted rain streaks that wraps
 * around the tile edges, so it can be tiled seamlessly.
 */
static void GenerateRainTile(SpriteLoader::Sprite &sprite, uint layer, uint frame, uint8_t alpha_scale)
{
	const RainLayerParams &params = RAIN_LAYER_PARAMS[layer];

	sprite.width = RAIN_TILE;
	sprite.height = RAIN_TILE;
	sprite.x_offs = 0;
	sprite.y_offs = 0;
	sprite.AllocateData(ZoomLevel::Min, RAIN_TILE * RAIN_TILE);

	Blitter *blitter = BlitterFactory::GetCurrentBlitter();
	bool use_rgba = blitter->GetScreenDepth() >= 32;
	if (use_rgba) {
		sprite.colours.Set(SpriteComponent::RGB);
		sprite.colours.Set(SpriteComponent::Alpha);
	} else {
		sprite.colours = SpriteComponent::Palette;
	}

	/* Texture transparency is index 0 (8bpp) / alpha 0 (32bpp). */
	for (int i = 0; i < RAIN_TILE * RAIN_TILE; i++) {
		sprite.data[i].r = 0;
		sprite.data[i].g = 0;
		sprite.data[i].b = 0;
		sprite.data[i].a = 0;
		sprite.data[i].m = 0;
	}

	/* Slope of the streaks, matching the layer's scroll direction. */
	const int slope_x_num = params.speed_x;
	const int slope_x_den = params.speed_y;

	for (int s = 0; s < params.streak_count; s++) {
		uint32_t rnd = RainHash(layer * 31 + frame, s, 0x5A17);
		int x = rnd % RAIN_TILE;
		int y = (rnd >> 8) % RAIN_TILE;
		int len = params.len_min + (int)((rnd >> 16) % params.len_extra);
		/* Opacity flickers per streak and per frame. */
		uint8_t alpha = params.base_alpha * (60 + rnd % 81) / 160;
		uint8_t shade = 185 + (rnd >> 8) % 40; /* Pale blue-grey variation. */

		for (int i = 0; i < len; i++) {
			int px = (x + i * slope_x_num / slope_x_den + RAIN_TILE) % RAIN_TILE;
			int py = (y + i) % RAIN_TILE;
			auto &p = sprite.data[py * RAIN_TILE + px];
			if (use_rgba) {
				p.r = shade * 9 / 10;
				p.g = shade;
				p.b = std::min(shade + 25, 255);
				p.a = alpha * alpha_scale / 255;
			} else {
				p.m = PC_LIGHT_BLUE.p;
				p.a = 0xFF;
			}
		}
	}
}

/** (Re)build all encoded rain frames for the current blitter and fade level. */
static void BuildRainFrames()
{
	Blitter *blitter = BlitterFactory::GetCurrentBlitter();
	uint8_t alpha_scale = RainShadeProgress();

	_rain_cache.frames.clear();
	_rain_cache.blitter = blitter;
	_rain_cache.shade_level = _rain_shade_level;

	for (uint layer = 0; layer < RAIN_LAYERS; layer++) {
		for (uint frame = 0; frame < RAIN_FRAMES; frame++) {
			_rain_cache.frames.emplace_back();
			RainFrame &f = _rain_cache.frames.back();

			SpriteLoader::SpriteCollection collection;
			SpriteLoader::Sprite &sprite = collection.Root();
			GenerateRainTile(sprite, layer, frame, alpha_scale);

			UniquePtrSpriteAllocator allocator;
			f.sprite = blitter->Encode(SpriteType::Normal, collection, allocator);
			f.allocator.data = std::move(allocator.data);
		}
	}
}

/**
 * Blit one texture tile at the given screen position, clipped to the
 * destination rectangle.
 */
static void BlitRainTile(Blitter *blitter, const Sprite *sprite, const DrawPixelInfo *dpi, int px, int py)
{
	int left = std::max(px, dpi->left);
	int top = std::max(py, dpi->top);
	int right = std::min(px + RAIN_TILE, dpi->left + dpi->width);
	int bottom = std::min(py + RAIN_TILE, dpi->top + dpi->height);
	if (left >= right || top >= bottom) return;

	Blitter::BlitterParams bp;
	bp.sprite = sprite->data;
	bp.sprite_width = sprite->width;
	bp.sprite_height = sprite->height;
	bp.skip_left = left - px;
	bp.skip_top = top - py;
	bp.width = right - left;
	bp.height = bottom - top;
	bp.left = left - dpi->left;
	bp.top = top - dpi->top;
	bp.dst = dpi->dst_ptr;
	bp.pitch = dpi->pitch;
	bp.remap = nullptr;
	bp.brightness_adjust = 0;

	blitter->Draw(&bp, BlitterMode::Normal, ZoomLevel::Min);
}

void DrawRainOverlay(const DrawPixelInfo *dpi)
{
	if (_rain_shade_level == 0) return;
	Blitter *blitter = BlitterFactory::GetCurrentBlitter();

	if (!_rain_cache.Matches(blitter, _rain_shade_level)) BuildRainFrames();

	uint64_t now = RainNowMs();

	for (uint layer = 0; layer < RAIN_LAYERS; layer++) {
		const RainLayerParams &params = RAIN_LAYER_PARAMS[layer];

		/* Smooth scroll offset. */
		int off_x = (int)(now * params.speed_x / 1000) % RAIN_TILE;
		int off_y = (int)(now * params.speed_y / 1000) % RAIN_TILE;

		/* Gust jitter: the whole layer is nudged by a few pixels now and then. */
		uint64_t gust = now / 400;
		off_x += (int)(RainHash(0xA11CE + layer, (uint32_t)gust, 0) % 5) - 2;
		off_y += (int)(RainHash(0xB0B + layer, (uint32_t)gust, 1) % 3) - 1;

		/* The animation frame jumps pseudo-randomly instead of cycling. */
		uint frame = RainHash(0xF0F + layer, (uint32_t)(now / 120), 2) % RAIN_FRAMES;
		const Sprite *sprite = _rain_cache.frames[layer * RAIN_FRAMES + frame].sprite;

		/* Tile positions align to absolute screen coordinates, so the pattern
		 * is continuous across viewports and stable while panning. */
		int first_x = dpi->left - ((dpi->left + off_x) % RAIN_TILE + RAIN_TILE) % RAIN_TILE;
		int first_y = dpi->top - ((dpi->top + off_y) % RAIN_TILE + RAIN_TILE) % RAIN_TILE;

		for (int py = first_y; py < dpi->top + dpi->height; py += RAIN_TILE) {
			for (int px = first_x; px < dpi->left + dpi->width; px += RAIN_TILE) {
				BlitRainTile(blitter, sprite, dpi, px, py);
			}
		}
	}
}
