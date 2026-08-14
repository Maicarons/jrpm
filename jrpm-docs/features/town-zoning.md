---
title: 城镇分区（Town Zoning）
---

# 城镇分区（Town Zoning）

cmclient 城镇分区移植（第四批，提交 `9e3f95a2`）。jrpm 基座（pulsexlb）本身已带一套 zoning 系统（工具栏 + 绘制管线 + 菜单），本批次在它之上叠加了 cmclient 独有的评估模式与 **growth_tiles 存档**。

## 新增模式（zoning 工具栏下拉）

| 模式 | 说明 | 着色 |
|---|---|---|
| **Town zones (Tz)** | 城镇同心分区，复用 `squared_town_zone_radius` | Tz0 边缘=浅蓝 / Tz1=红 / Tz2=黄 / Tz3=绿 / Tz4 市中心=白 |
| **Town growth tiles** | 本月/上月房屋建造与拆除轨迹 | 新房=绿 / 拆除=浅蓝 / 重建=白 / 跳过增长=橙 / 跳过建房=黄 / 服务器拆房=红 |

## growth_tiles 数据层（`cm_town_growth.cpp/.h`）

- 两个滚动月 map：`TileIndex → TownGrowthTileState`（当前月 / 上月）
- 事件钩子：
  - `BuildTownHouse` → `NEW_HOUSE`（若上月为拆除则升级 `RH_REBUILT`）
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → 每月轮转（上月=本月，本月清空）
- **存档持久化**：新的 `GRWT` savegame chunk（`misc_sl.cpp`），序列化为 `{tile, state}` 对列表；旧存档不含此块，加载完全兼容（已验证存档→读档往返）。

## 打开方式

工具栏菜单 → 地图（Zoning）→ 打开 zoning 工具栏，内外圈下拉分别选择评估模式。

## 裁剪说明

cmclient 的 CityBuilder 服务器专属模式（CB 接受区 / CB 城镇上限）与 `ext::Town` 扩展字段（广告、资金、货物统计）为服务器玩法专属，未移植。
