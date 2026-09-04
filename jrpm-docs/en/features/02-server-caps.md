---
title: Server Capacity Research
---

## Research Conclusion: Both Limits Are Structural

### Online Player Limit = 255 (Protocol Bit-Width Hard Constraint)

| Location | Content |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` transmitted as **`Send_uint8` / `Recv_uint8`** in `SerializeNetworkGameInfo` (lines 251-296) and deserialization (lines 422-432) |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, default 25) |
| `src/network/network_server.cpp:360` | Acceptance check `_network_clients_connected < MAX_CLIENTS` |

**Conclusion**: 255 is already the protocol field limit for uint8. To break through, `clients_max` and related fields must be changed to uint16 — this alters client↔server wire protocol, UDP broadcast, and the **Game Coordinator** protocol, which is an external protocol change (playable within a fork if both ends upgrade, but public server list compatibility is affected).

### Company Limit = 15 (Tile Owner Storage Format Hard Constraint)

| Location | Content |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner` : `SB(_m[tile].m1, 0, 5, owner.base())` —— **tile owner only stores 5 bits** (low 5 bits of `_m[].m1`) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`, `OWNER_NONE{0x10}`, `OWNER_WATER{0x11}`, `OWNER_DEITY{0x12}`, `OWNER_END{0x13}` share the same byte space as company IDs |

**Conclusion**: `Owner` and `CompanyID` are the same type, and the tile ownership field is only 5 bits (values 0-31), of which 15-18 are occupied by special owners. Therefore the real company limit of 15 is determined by the **map array format** — breaking through requires widening the tile owner storage (`m1` full 8 bits or changing the `_m` structure) and migrating OWNER_* constants, involving **full save conversion** and map memory growth, which is a major format refactoring (the reason upstream OpenTTD has not done this for years).

## Options

| Option | Changes | Compatibility | Suggestion |
|---|---|---|---|
| A. Keep current | None | Save/Protocol/Coordinator fully compatible | ✅ Recommended: 255 clients / 15 companies already far exceeds the original for multiplayer |
| B. Client → uint16 protocol | `network_game_info.cpp` serialization/deserialization change to `Send_uint16/Recv_uint16`; `MAX_CLIENTS` raised to 4095; `network_settings.ini` `max_clients` change to SLE_UINT16; `settings_type.h` field change to uint16; `console_cmds.cpp:1056` display width | Playable within fork; public coordinator/old clients incompatible | Optional, if >255 clients is truly needed |
| C. Company → tile format refactor | Widen tile owner field + migrate OWNER_* constants + full save conversion | Save format change (one-time conversion) | Not recommended for short-term implementation |

## Delivered Content

- This design document (with exact file/line numbers);
- Optional patch `option-clients-uint16.diff` (complete changes for Option B, **not applied**, available for `git apply` review).

> Conclusion first: **"Extending" both limits is essentially a format/protocol upgrade, not a configuration adjustment**; under the premise of maintaining save and public network compatibility, the current 255/15 is the upper limit. If compatibility cost is acceptable, Options B/C are as described above.