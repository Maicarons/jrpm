# 第四批 / 第五批 / CM 服务器命令 —— 移植难度调研报告

> 调研对象：citymania-org/cmclient（vanilla 15.3 分支）源码精读
> 调研时间：2026-08-14
> 结论：三块工作难度差异很大，给出逐项评估与推荐顺序。

---

## 一、第四批：城镇分区（含 growth_tiles 存档）

### 功能构成

| 组件 | 文件 | 规模 | 说明 |
|---|---|---|---|
| 分区模式枚举 | cm_zoning.hpp | 37 行 | 12 种评估模式（CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES） |
| 评估逻辑 | cm_zoning_cmd.cpp | 413 行 | 每种模式一个查询函数（返回调色板 SpriteID），含 TownZone/Tz 半径算法、StationFinder 覆盖检查、未服务建筑/工业检查 |
| 分区工具栏 GUI | cm_zoning_gui.cpp | 204 行 | 内/外两层分区模式选择窗口（`_zoning.inner/outer` 全局状态） |
| growth_tiles 存档 | cm_saveload.cpp/.hpp | 90 行 | Town 扩展字段 `growth_tiles` + `growth_tiles_last_month`（`std::map<TileIndex, uint8_t>`），用 vanilla SaveLoad handler 存 |
| 数据扩展 | extensions/cmext_town.hpp | ~80 行 | `ext::Town`：growth_tiles ×2 + **大量 CM 服务器玩法字段**（CBTownInfo 货物统计、广告/资金追踪、hs/cs/hr 城镇增长计数） |
| 每月轮转 + 触发 | cm_game.cpp / town_cmd.cpp | — | NewMonth 轮转 growth_tiles；房屋建造/拆除/重建处记录状态 |

### 移植难点逐项评估

| 难点 | 严重度 | 说明 |
|---|---|---|
| **存档体系改写** | 🟠 中 | growth_tiles 用 vanilla `DefaultSaveLoadHandler` + `SlSetStructListLength`/`SlObject`，jrpm 是 `sl/` 新体系（SlTableHeader/SlObjectSaveFiltered）。需用 jrpm 的 NSL/SLE + **XSLF 特性门控**重写（新增 `XSLFI_TOWN_GROWTH_TILES`，版本 1），旧档不受影响 |
| **Town 结构扩展** | 🟢 低 | 只取 growth_tiles 两个 map（**跳过** CBTownInfo/广告/资金字段——那些是 CM 服务器 CityBuilder 玩法专属）；jrpm Town 加字段 + town_sl.cpp 存档表挂载 |
| **评估逻辑** | 🟢 低-中 | 大部分是纯查询（GetTileType/StationFinder/Town 缓存/房屋检查），jrpm API 存在；TownZone 半径算法（`squared_town_zone_radius`）需核对 jrpm 字段名 |
| **渲染管线** | 🟠 中-高 | `DrawTileZoning` 需挂进 viewport 绘制管线——**与第三批高亮共用 TileHighlight 渲染机制**。没有高亮管线就得单独搭（建议**先做高亮，再做分区**） |
| **精灵资源** | 🟠 中 | 12 种调色板用 `CM_SPR_PALETTE_ZONING_*` **自定义精灵**，jrpm 没有 → 需用现有调色板精灵替代或新增资源 |
| **事件钩子** | 🟠 中 | growth_tiles 记录依赖 cmclient 的**事件总线**（event::HouseBuilt/HouseCleared/...，cm_main.cpp 的 Emit）。jrpm 无此机制 → 直接在 town_cmd.cpp 房屋建造/拆除处加钩子 + NewMonth 轮转（IntervalTimer） |

### 难度结论：🟠 中高（约 1.5–2 轮专项，每轮 4–6 小时）

- **前提**：强烈建议在第三批**高亮系统**完成后再做（渲染管线共用）
- 若只做"12 种分区着色、不含 growth_tiles 存档"：难度降至 🟡 中（省掉存档扩展，约 1 轮）

---

## 二、第五批：命令重放 + 导出/录制

### 2.1 命令重放（cm_command_log + cm_commands + generated）

| 组件 | 规模 | 说明 |
|---|---|---|
| 命令对象层 | cm_command_type.hpp + generated/cm_gen_commands（2251+1418 行） | 每个 vanilla 命令包装成可编程对象（as_company/with_callback/set_auto），含**命令→位流序列化** |
| 命令日志加载 | cm_command_log.cpp（203 行） | lzma 解压 + BitOStream 位流解析 → `_fake_commands` 队列（tick counter/期望结果/随机种子/CommandPacket） |
| 命令执行 | ExecuteFakeCommands | 按 tick counter 顺序执行：`ExecuteCommand(&cp)`（vanilla 内部 API）+ **随机种子/结果校验**（反作弊），联机时转发给所有客户端 |

**关键差异（决定难度）**：

| cmclient | jrpm | 影响 |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | 无此函数，命令核心是 `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | 需写 CommandPacket → DoCommandPInternal 的 payload 转换层 🟠 |
| CommandPacket 字段（vanilla） | CommandPacket 存在但结构不同（GeneralCommandPacket\<DynBaseCommandContainer\>） | 需适配字段映射 🟠 |
| `GetCommandName` | ✅ 存在（command_func.h:166） | 🟢 |
| `outgoing_queue`（联机转发） | jrpm 网络层不同（OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand） | 联机回放需重写转发逻辑 🟠 |
| 记录器（怎么生成 .cmd 文件） | **cmclient 的记录钩子依赖命令对象层的 post() 拦截** | jrpm 命令是模板化 Post → 需在命令分发点加记录钩子 🔴 核心工作 |
| 文件格式 | 私有（vanilla 命令 ID + 位流） | jrpm 命令 ID 与 vanilla 完全不同 → 重放文件不兼容，**格式需重设计** 🟠 |

### 2.2 导出 / 录制（cm_export.cpp，536 行）

| 功能 | 说明 | 难度 |
|---|---|---|
| ExportOpenttdData | JSON 导出房屋规格/货物规格/调色板/引擎信息（JsonWriter） | 🟢 低-中（独立，读 Spec 结构写 JSON） |
| ViewportExport / ExportFrameSprites | 逐帧导出视口精灵（依赖 viewport 内部绘制向量 TileSpriteToDrawVector/ParentSpriteToSortVector） | 🟠 中-高（jgrpp viewport 管线不同，需对齐向量接口） |

### 难度结论：🟠 中高（约 2 轮专项）

- **命令重放是最大头**：核心工作 = ①命令分发点加**记录钩子**（jrpm 模板化命令系统的适配）②CommandPacket → DoCommandPInternal 执行层 ③新文件格式设计。**建议先移植命令对象层**（第三批蓝图的地基，也是重放的地基）
- 导出：JSON 数据导出可独立先做（低-中）；帧录制依赖 viewport 对齐（与高亮同一批处理）

---

## 三、CM 服务器命令（cm_console_cmds.cpp，289 行）

### 命令清单与逐项难度

| 命令 | 功能 | 对 jrpm 的难度 | 备注 |
|---|---|---|---|
| `cmgamespeed [n]` | 改游戏速度 | 🟢 **极低**（~20 行） | jrpm 已有 `_game_speed` 全局（gfx.cpp:52），只缺命令外壳 |
| `cmstep [n]` | 步进 n tick | ⏭️ **jrpm 已有** | `step` 命令（ConStepGame）功能一致，跳过 |
| `cmexport` | 导出 openttd.json | 🟢 低-中 | 依赖 ExportOpenttdData（见第五批） |
| `cmtreemap <file>` | 高度图种树 | 🟡 中 | 读高度图 + 种树命令，独立 |
| `cmreset_town_growth` | 清空城镇增长记录 | 🟢 低 | 依赖城镇分区字段 |
| `cmload_commands` | 加载命令回放 | 🟠 中高 | 依赖命令重放基建 |
| `cmstart_record` / `cmstop_record` | 帧录制 | 🟠 中高 | 依赖录制基建 |
| `cmgamestats` | 游戏会话统计 | 🟢 低 | 独立 |
| `cmgfxdebug` | 图形调试 | 🟢 低 | 独立 |

### 难度结论：🟢 总体低（单命令 0.5–2 小时）

**推荐顺序**：`cmgamespeed`（10 分钟）→ `cmgamestats`/`cmgfxdebug`（各 ~1h）→ `cmexport`（随第五批导出）→ `cmtreemap`（~2h）→ `cmreset_town_growth`（随第四批）→ `cmload_commands`/`cmstart_record`（随第五批重放/录制）。

---

## 四、总体推荐路线

```
① 高亮系统（第三批核心，~2-3 轮）      ← 蓝图/分区的渲染地基
② 命令对象层（第三批蓝图 + 第五批重放的地基，~1-2 轮）
③ 蓝图（~1 轮）
④ 城镇分区（~1.5 轮，依赖 ① 渲染管线；growth_tiles 用 XSLF 门控）
⑤ 命令重放 + 导出（~2 轮，依赖 ②；JSON 导出可提前独立做）
⑥ CM 服务器命令（零散插入各批之间，gamespeed/step 可立即做）
```

## 五、可立即低成本落地项（不依赖任何大工程）

1. `cmgamespeed` 命令（~20 行，改 `_game_speed`）
2. `step` 已有 → 无需做
3. `cmgamestats` / `cmgfxdebug`（各 ~1h，独立）
4. `cmexport` 的 JSON 数据导出（~2h，独立）
5. `cmtreemap`（~2h，独立）

这 5 项约 **半天工作量**，可作为"零批"先落地，与三/四/五批并行不冲突。
