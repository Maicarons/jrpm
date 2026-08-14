/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file config.cpp Configuration of the connection strings for network stuff using environment variables. */

#include "../../stdafx.h"

#include <cstdlib>
#include <ranges>
#include <string>
#include <vector>
#include "../../settings_type.h"
#include "../../string_func.h"

#include "../../safeguards.h"

/**
 * Get the connection string for the game coordinator from the environment variable OTTD_COORDINATOR_CS,
 * or when it has not been set a hard coded default DNS hostname of the production server.
 * @return The game coordinator's connection string.
 */
std::string_view NetworkCoordinatorConnectionString()
{
	return GetEnv("OTTD_COORDINATOR_CS").value_or("coordinator.openttd.org");
}

/**
 * Get the connection string for the STUN server from the environment variable OTTD_STUN_CS,
 * or when it has not been set a hard coded default DNS hostname of the production server.
 * @return The STUN server's connection string.
 */
std::string_view NetworkStunConnectionString()
{
	return GetEnv("OTTD_STUN_CS").value_or("stun.openttd.org");
}

/**
 * Get the connection string for the content server from the environment variable OTTD_CONTENT_SERVER_CS,
 * the setting network.content_server, or a hard coded default DNS hostname of the production server.
 * @return The content server's connection string.
 */
std::string_view NetworkContentServerConnectionString()
{
	if (auto env = GetEnv("OTTD_CONTENT_SERVER_CS"); env.has_value()) return *env;
	if (!_settings_client.network.content_server.empty()) return _settings_client.network.content_server;
	return "content.openttd.org";
}

/**
 * Get the URI string for the content mirror from the environment variable OTTD_CONTENT_MIRROR_URI,
 * the setting network.content_mirrors, or a hard coded URI of the production server.
 * @return The first content mirror's URI string.
 */
std::string_view NetworkContentMirrorUriString()
{
	auto mirrors = NetworkContentMirrorUris();
	return mirrors.front();
}

/**
 * Get the list of content mirror URIs from the environment variable OTTD_CONTENT_MIRROR_URI
 * or the setting network.content_mirrors (comma separated), falling back to the production server.
 *
 * Mirrors are used in order: when a download from one mirror fails, the next mirror is tried.
 * @return The list of content mirror URIs.
 */
std::vector<std::string> NetworkContentMirrorUris()
{
	std::string_view list = "https://binaries.openttd.org/bananas";
	if (auto env = GetEnv("OTTD_CONTENT_MIRROR_URI"); env.has_value()) {
		list = *env;
	} else if (!_settings_client.network.content_mirrors.empty()) {
		list = _settings_client.network.content_mirrors;
	}

	std::vector<std::string> mirrors;
	for (const auto part : StrMakeValidView(list) | std::views::split(',')) {
		std::string_view sv(part.data(), part.size());
		size_t begin = sv.find_first_not_of(" \t\r\n");
		if (begin == std::string_view::npos) continue;
		size_t end = sv.find_last_not_of(" \t\r\n");
		std::string uri(sv.substr(begin, end - begin + 1));
		if (!uri.empty()) mirrors.push_back(std::move(uri));
	}
	if (mirrors.empty()) mirrors.emplace_back("https://binaries.openttd.org/bananas");
	return mirrors;
}

/**
 * Get the URI string for the survey from the environment variable OTTD_SURVEY_URI,
 * or when it has not been set a hard coded URI of the production server.
 * @return The survey's URI string.
 */
std::string_view NetworkSurveyUriString()
{
	return GetEnv("OTTD_SURVEY_URI").value_or("https://survey-participate.openttd.org/");
}
