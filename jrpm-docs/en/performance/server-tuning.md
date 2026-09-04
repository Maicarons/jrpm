---
title: Server Performance Tuning Guide
---

> Based on the merged jrpm branch (including pulsexlb features and jrpm-specific optimizations).
> Parameter defaults taken from `src/table/settings/network_settings.ini`, recommended to validate with actual load testing.

## 1. Performance Optimizations Already Delivered (This Release)

| Optimization | Commit | Description |
|---|---|---|
| Parallel download concurrency configurable | 4716b925 | `network.content_download_parallel` (default 4, range 1-8): number of simultaneous file downloads, replacing hardcoded 4 |
| Multi-mirror + file-level parallel download | d4c45740 (F1) | `network.content_mirrors` comma-separated multi-mirror, retry on failure per mirror, final fallback to legacy protocol |
| Server multi-version client compatibility | cb9848b7 | Server can simultaneously accept jrpm / original jgrpp / pulsexlb clients |

## 2. Key Server Performance Parameters (jgrpp System)

| Setting | Default | Range | Description | Tuning Suggestions |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | Frame sync check (desync detection) every N frames. Higher values save bandwidth, lower values detect desync earlier | For stable multiplayer, reduce to 20-50; lower if frequent desync occurs |
| `network.frame_freq` | 0 | 0-100 | Server sends command frames every N frames (0 = send every frame). Higher values save bandwidth/CPU but increase latency | Normal 0-3; busy servers can go to 5, balance after load testing |
| `network.commands_per_frame` | 2 | 1-65535 | Max client commands processed per frame (anti-spam/flood) | Increase to 4-8 for many players or frequent actions |
| `network.commands_per_frame_server` | 16 | 1-65535 | Server's own command limit per frame | Usually not needed |
| `network.bytes_per_frame` | 8 | 1-65535 | Long-term average receive bytes per frame (bandwidth shaping) | With good bandwidth, increase to 16-32 for faster large map sync |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | Burst byte limit (allows short-term peaks) | Increase with the above, e.g., 512 |
| `network.max_init_time` | 60 | 0-32000 | Client initialization timeout (ticks) | Relax for players on weak connections |
| `network.max_join_time` | 500 | 0-32000 | Client join (map download + sync) timeout (ticks) | Relax to 1000+ for large maps/slow bandwidth |
| `network.max_download_time` | 1000 | 0-32000 | Map download timeout (ticks) | For large maps (4096+), suggest 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | Max client lag tolerance (ticks) | Relax for high-latency players, but increases desync risk |

> Note: `max_*_time` units are game ticks (1/74 sec ≈ 13.5ms); `bytes_per_frame` refers to the average bytes per sync window per frame.

## 3. Recommended Server Starting Configuration (Multiplayer Scenario)

```ini
[network]
max_clients = 32            ; or adjust as needed
max_companies = 15
frame_freq = 3              ; 0=send commands every frame (smoothest); 3=compromise to save bandwidth
sync_freq = 50              ; detect desync earlier
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; client-side content download concurrency
```

## 4. Future Performance Optimization Directions (By Priority)

1. **HTTP transport layer thread pool + Range chunked download** (Openttd-Cluster 0007 concept)
   Current status: F1 already implements "file-level parallelism" (multiple files downloaded simultaneously). 0007 adds thread pool + `CURLOPT_RANGE` chunking at the HTTP layer, which can further accelerate **single large files** (giant scenarios .tar.gz). The two are complementary but modify the same files; recommended to incrementally add after F1 is stable.

2. **Map send optimization**
   Verify whether jrpm enables map compression (zstd/lzma) by default; compare compression algorithm performance on large maps.

3. **Rust server runtime (long-term architecture reference)**
   Openttd-Cluster's otc-engine (Admin/RCON, Prometheus metrics, cluster failover, Web panel, snapshot bridge) depends on a full Rust engineering FFI integration, which is a "next-generation server"-level overhaul, not a patch-level merge; jrpm currently maintains a pure C++ single binary.

4. **Game simulation performance**
   - Monitor `economy`/`linkgraph` parameters (linkgraph_settings.ini) on large maps with many vehicles;
   - For extreme performance, research `settings_game.economy.` class and pathfinder ceiling parameters (`pathfinding` settings).

## 5. Verification Suggestions

- Start server `openttd-jrpm -D -c server.cfg`, test with multiple jrpm clients: join time, frame sync stability (no desync prompts), CPU/memory usage;
- Content download: set `content_download_parallel=8` and compare download time for a batch of NewGRFs;
- Weak network stress test: combine `max_lag_time` and `sync_freq` to find the stability boundary.