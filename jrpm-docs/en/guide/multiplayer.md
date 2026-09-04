---
title: Multiplayer Guide
---

# Multiplayer Guide

## Version Compatibility Strategy

jrpm is a **tagged version** (`jrpm-0.1.0`), requiring the revision string to **match exactly** for multiplayer handshake:

| Scenario | Behavior |
|---|---|
| jrpm client ↔ jrpm server | ✅ Normal connection (version matches) |
| jrpm client ↔ original jgrpp / pulsexlb server | ❌ Rejected (version isolation) |
| Original jgrpp client → jrpm server | ✅ Allowed (server-side relaxes acceptance of `jgrpp-*` revision) |
| pulsexlb client → jrpm server | ✅ Allowed (accepts `pxp` revision) |

::: warning NewGRF Version
Regardless of the client's version, **NewGRF version must exactly match the server** (`_openttd_newgrf_version` is strictly validated) — this is the baseline for deterministic simulation.
:::

## Setting Up a Server

```bash
# Dedicated server (no GUI)
openttd-jrpm -D -c server.cfg
```

Recommended `server.cfg` configuration (see [Server Performance Tuning](../performance/server-tuning) for details):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## Joining a Server

- In-game: "Multiplayer → Join Internet Server/Add Server", or
- Command line: `openttd-jrpm -n <host>:<port>`

## Content Download (NewGRF/Scenarios)

jrpm supports **multi-mirror + parallel download**:

```ini
[network]
content_server = content.openttd.org        ; metadata server
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; parallel download file count (1-8)
```

- Mirror list is comma-separated, used in order; failed mirrors automatically switch to the next;
- Environment variables `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` take priority over settings;
- Falls back to the legacy TCP download protocol when all mirrors fail.

## Server Administration

- Console commands: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (vehicle auto-grouping), etc.;
- RCON: Set `rcon_password` on the server to send console commands remotely.