---
title: 服务器上限研究
---

## 调研结论：两个上限都是结构性的

### 在线玩家上限 = 255（协议位宽硬约束）

| 位置 | 内容 |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` 以 **`Send_uint8` / `Recv_uint8`** 在 `SerializeNetworkGameInfo`（251-296 行）与反序列化（422-432 行）中传输 |
| `src/table/settings/network_settings.ini:241` | `network.max_clients`（SLE_UINT8，max = MAX_CLIENTS，默认 25） |
| `src/network/network_server.cpp:360` | 接纳判断 `_network_clients_connected < MAX_CLIENTS` |

**结论**：255 已是 uint8 协议字段的上限。要突破必须把 `clients_max` 等相关字段改为 uint16——这会改变 client↔server 线上协议、UDP 广播与**游戏协调器（Game Coordinator）**协议，属于对外协议变更（fork 内两端同升可玩，但公共服务器列表兼容性受影响）。

### 公司数量上限 = 15（瓦片 owner 存储格式硬约束）

| 位置 | 内容 |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner`：`SB(_m[tile].m1, 0, 5, owner.base())` —— **瓦片 owner 只存 5 位**（`_m[].m1` 的低 5 bit） |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`、`OWNER_NONE{0x10}`、`OWNER_WATER{0x11}`、`OWNER_DEITY{0x12}`、`OWNER_END{0x13}` 与公司 ID 共用同一字节空间 |

**结论**：`Owner` 与 `CompanyID` 是同一类型，瓦片所有权字段仅 5 位（值 0-31），其中 15-18 被特殊 owner 占用。因此真实公司上限 15 是**地图数组格式**决定的——突破需扩大瓦片 owner 存储（`m1` 全 8 位或改 `_m` 结构）并迁移 OWNER_* 常量，涉及**全量存档转换**与地图内存增长，属于大型格式重构（OpenTTD 上游多年未做即是此原因）。

## 可选方案

| 方案 | 改动 | 兼容性 | 建议 |
|---|---|---|---|
| A. 保持现状 | 无 | 存档/协议/协调器全兼容 | ✅ 推荐：255 客户端 / 15 公司对多人联机已远超原版 |
| B. 客户端 → uint16 协议 | `network_game_info.cpp` 序列化/反序列化改 `Send_uint16/Recv_uint16`；`MAX_CLIENTS` 提到 4095；`network_settings.ini` 的 `max_clients` 改 SLE_UINT16；`settings_type.h` 字段改 uint16；`console_cmds.cpp:1056` 显示宽度 | fork 内两端可玩；公共协调器/旧客户端不兼容 | 可选，若确需 >255 客户端 |
| C. 公司 → 瓦片格式重构 | 瓦片 owner 字段加宽 + OWNER_* 常量迁移 + 全量存档转换 | 存档格式变更（一次性转换） | 不建议短期实施 |

## 已交付内容

- 本设计文档（含精确文件/行号）；
- 可选补丁 `option-clients-uint16.diff`（方案 B 的完整改动，**未应用**，供 `git apply` 审查）。

> 结论先行：**两个上限的“扩展”本质上是格式/协议升级而非配置项调整**；在保持存档与公共网络兼容的前提下，当前 255/15 即为上限。若接受兼容性代价，方案 B/C 的改法如上。
