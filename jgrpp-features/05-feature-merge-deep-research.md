# jrpm 特性合并深度研究报告（供拍板）

- 调研日期：2026-08-14
- 调研对象：CityMania cmclient（vanilla 15.3）+ OpenTTD-modded 三补丁
- 目标：评估各特性移植进 jrpm（jgrpp-0.73.1 + pulsexlb）的**玩法价值 / 技术难度 / 联机影响**，供拍板

---

## 零、一句话结论

- **cmclient 有 6 大独特玩法功能**，全部有价值，但因为是 vanilla API 体系，每个都要做 API 适配（可做，非不可能）
- **modded 三补丁**：工业产量图 **jrpm 已有**（不用做）；车辆行程历史、飞机滑行速度 **jrpm 没有**，值得做且难度低
- **推荐分三批做**：先易后难、每批可独立联机验证

---

## 一、cmclient 核心特性详解（按玩法价值排序）

### 🥇 1. 蓝图系统（Blueprint）—— 最强玩法功能

**玩法**：圈选一块区域 → 自动记录区内所有建造命令（铁轨/车站/隧道/桥梁/信号）→ 保存到 **16 个槽位** → 在任意位置一键重放整块建筑（支持旋转）。  
**技术**：`cm_blueprint.cpp` 660 行 + 命令拦截记录（`CommandExecuted` 钩子）+ 对象级高亮预览（`ObjectHighlight` 15 种对象）。  
**价值**：★★★★★ 轨道交通玩家福音——复制一段车站+轨道布局反复使用，省去大量重复操作。  
**难度**：🟠 中高。依赖 vanilla 命令钩子（`CommandCallback`），jgrpp 命令系统已重构（`Commands` 枚举 + `DEF_CMD_TUPLE`），需改写命令记录/重放层；16 槽位可放存档或配置文件。

### 🥇 2. 城镇分区（Zoning）—— 规划神器

**玩法**：12 种地图着色模式，一眼看清：

- 城镇意见（可建性红绿）、站点覆盖范围、活跃/非活跃站（50 天服务判断）
- 未服务的建筑/工业、城镇分区 Tz 层级、货物接受区
- **城镇增长瓦片**（新盖房/被拆房高亮，growth_tiles 存档字段）  
  **技术**：`cm_zoning*` + `cm_game` 的 `growth_tiles` 追踪 + `cm_saveload` 存档（每城 2 组瓦片索引）。  
  **价值**：★★★★★ 城市/物流规划核心工具，尤其是**城镇增长追踪**是其他 mod 没有的独特功能。  
  **难度**：🟠 中高。zoning 绘制较独立（tile 着色层），但 growth_tiles 需要**改存档格式**（新增 town 字段）→ 接入 jrpm `sl/` 新体系 + SLV bump。

### 🥈 3. 命令重放（Command Replay / cm_command_log）

**玩法**：**整局回放**——服务器记录所有玩家命令（lzma 压缩），可加载 replay 文件按 tick 重放整场游戏（`ExecuteFakeCommands`）；同时有命令延迟统计（`get_average_command_lag`）。  
**技术**：`cm_command_log.cpp` + `cm_commands.cpp` 命令队列/回调管理 + 回放帧驱动。  
**价值**：★★★★ 联机录像/复盘功能，比赛服刚需；命令延迟显示对高延迟玩家有用。  
**难度**：🟠 中。回放核心较独立（FakeCommand 队列注入），但命令序列化依赖 vanilla `CommandPacket` 格式，jgrpp 命令参数已扩展（需做序列化适配）。

### 🥈 4. 地图高亮（Highlight / ObjectHighlight）

**玩法**：建造时精确预览——15 种对象（铁轨/信号/车站/桥梁/隧道/码头/机场/工业/蓝图区）在放置前就以半透明色块显示**实际占用区域**，比原版的高亮方块精确得多（例如铁轨显示整段、车站显示整个站台区）。  
**技术**：`cm_highlight*` 对象级高亮渲染 + `TileHighlight` 扩展（cm_new 字段）。  
**价值**：★★★★ 建造体验大提升，与蓝图联动。  
**难度**：🟡 中。高亮渲染独立，但 jgrpp 的 tilehighlight 系统与 vanilla 有差异，需适配。

### 🥉 5. 导出与录制（Export / Frame Recording）

**玩法**：

- `ExportOpenttdData`：把建筑/货物/调色板/引擎规格导出 JSON（给外部工具用）
- `ExportFrameSprites` / `ViewportExportJson`：**逐帧导出视口精灵**（tick 级 JSON + 精灵图），可做游戏录像渲染  
  **技术**：`cm_export.cpp` + 视口绘制钩子（ViewportExportDrawBegin/End）。  
  **价值**：★★★ 内容创作者/工具链用途（AI 训练数据、地图渲染、数据可视化）。  
  **难度**：🟡 中。视口绘制钩子需要接触 jgrpp 渲染管线（viewport 精灵排序），有一定侵入性。

### 🥉 6. 联机与 UI 增强（多项小功能）

| 功能                             | 说明                | 价值  | 难度   |
| ------------------------------ | ----------------- | --- | ---- |
| **玩家列表悬浮层**（ClientListOverlay） | 游戏内常驻显示玩家状态，无需开窗口 | ★★★ | 🟢 低 |
| **观战公司窗口**（WatchCompany）       | 观战时看指定公司的动态       | ★★★ | 🟢 低 |
| **身份密钥管理**（Identity）           | 多身份/密钥切换（联机鉴权）    | ★★  | 🟢 低 |
| **货运明细表**（CargosWindow）        | 每货物收入/成本明细窗口      | ★★★ | 🟢 低 |
| **小地图增强**（minimap）             | 工业链断裂显示、建筑/所有者图例  | ★★★ | 🟡 中 |
| **信息叠加层**（BuildInfoOverlay）    | 视口内叠加建造信息         | ★★  | 🟡 中 |
| **地面详情提示**（LandTooltips）       | 悬停显示建筑/工业/车站详情    | ★★★ | 🟢 低 |
| **热键统计**（Hotkey stats）         | 热键使用统计            | ★   | 🟢 低 |
| **位置书签热键**（Locations）          | 9 个位置快捷跳转/保存      | ★★★ | 🟢 低 |

### ❌ 不建议做（cmclient 服务器专属）

- `cm_survey`（遥测调查）、`cm_newgrf_revisions`、`cm_main` 的服务器协议部分、`cm_command_type` 的自定义命令类型——这些是 **CityMania 服务器配套**，不玩 CM 服务器无意义。

---

## 二、modded 三补丁详解

### 1. 车辆行程历史（Trip History）—— ✅ 推荐做

**玩法**：每辆车显示最近 **10 趟**行程：收入、日期、收入变化%、**平均日收入（TBT）**、占用率。窗口化展示（`triphistory_gui`）。  
**触发**：车辆完成一趟时 `AddValue(route_profit, date, occupancy, distance)`（economy.cpp 1442 行）。  
**价值**：★★★★ 运营分析利器——直观看出哪条线路、哪辆车在赚钱/亏损，比原版"总收入"细化得多。  
**难度**：🟢 低。745 行独立代码；主要工作：① 车辆类加 `TripHistory` 字段 ② 存档适配 jrpm `sl/` 新体系（新增 2 个字段）③ GUI 窗口类适配（jgrpp NWidget 差异小）。  
**联机影响**：字段存进存档 → SLV bump（jrpm 惯例，低风险）。

### 2. 飞机滑行速度可调（Plane Taxi Speed）—— ✅ 推荐做

**玩法**：设置 `vehicle.plane_taxi_speed`（相对速度，/4 换算），让飞机在地面滑行快/慢，配合已有 `plane_speed` 可微调整个机场节奏。  
**技术**：`GetSpeedLimitWhileTaxiing` 里 `(SPEED_LIMIT_TAXI/4) * plane_taxi_speed`（aircraft_cmd.cpp 679 行）+ 1 个设置项。  
**价值**：★★★ 真实感/游戏节奏调节（机场繁忙时滑行太慢可提速）。  
**难度**：🟢 极低。1 个设置项（jrpm INI 体系注册）+ aircraft_cmd 2 处改动。  
**联机影响**：设置项 `NoNetwork` 属性 → 服务器权威，无存档影响。

### 3. 工业产量图（Industry Production Graph）—— ❌ 已存在

**甄别结果**：jrpm 的 `graph_gui.cpp:2162` **已有** `IndustryProductionGraphWindow`（jgrpp 0.73.1 已从上游吸收 PR#7575），**无需移植**。

---

## 三、总工作量与分批建议

| 批次            | 内容                                                     | 预估工作量    | 联机影响                                   |
| ------------- | ------------------------------------------------------ | -------- | -------------------------------------- |
| **第一批（推荐先做）** | modded：行程历史 + 滑行速度                                     | 🟢 1 天   | 存档 SLV bump（行程历史）；设置项（滑行）              |
| **第二批**       | cmclient：地面详情提示 + 玩家列表 + 观战公司 + 位置书签 + 货运明细（5 个低难度 UI） | 🟢 1-2 天 | 无（纯客户端 UI）                             |
| **第三批**       | cmclient：高亮系统 + 蓝图系统（联动）                               | 🟠 3-5 天 | 蓝图槽位可放本地配置；高亮纯客户端                      |
| **第四批**       | cmclient：城镇分区（含 growth_tiles 存档）                       | 🟠 3 天   | **存档格式变更**（town 字段）+ SLV bump，需仔细测存档往返 |
| **第五批（可选）**   | cmclient：命令重放 + 导出/录制                                  | 🟠 3-4 天 | 重放依赖命令序列化适配；导出触及渲染管线                   |

**建议**：第一批做完即可联机验证（延续现有测试流程）；第三批蓝图价值最高但建议放在高亮之后（两者联动）。

---

## 四、关键风险提示

1. **cmclient 是 vanilla 15.3**——它的命令/存档/窗口 API 与 jrpm（jgrpp 0.73.1）差异较大，**每个模块移植都需要 API 改写**，不能复制粘贴。最耗时的不是"写代码"而是"找对应 API"。
2. **存档字段是硬约束**——行程历史、城镇增长瓦片都要写进存档，必须走 jrpm `sl/` 新体系 + SLV bump，与 pulsexlb 已有的 SLV_ORDER_DECOUPLE/SLV_MULTITILE_AIRPORTS 机制一致，技术成熟。
3. **联机一致性**——所有客户端功能（蓝图/高亮/UI）都不影响联机协议，安全；但存档类改动（行程/分区）会影响多人存档，需做服务器+客户端往返测试（现有 E2E 流程可复用）。
4. **CM 服务器专属模块不做**（survey/协议/自定义命令类型）——除非未来要接 CityMania 服务器。

---

## 五、待拍板决策项

请确认以下 3 个问题：

1. **第一批**（行程历史 + 滑行速度）是否现在开始做？
2. **第三批**蓝图系统：是否做（价值高、工作量中高）？如果做，槽位存储用**存档**还是**本地配置文件**？
3. **第四批**城镇分区：growth_tiles 需要改存档格式，接受 SLV bump 吗？（会影响与旧 jrpm 存档的兼容，但 jrpm 本来就是新版本系）

---

## 六、CM 服务器专属模块深度剖析（参考价值评估）

之前标注"不建议做"的模块，实际精读后发现 **3 个高价值技术参考点 + 2 个玩法命令**，值得借鉴。

### 🥇 A. 命令对象封装层（cm_command_type.hpp）—— 最有价值的参考

**这不是玩法功能，而是一套「可编程命令基础设施」**：

```cpp
class Command {
    CompanyID company;       // 可指定以某公司身份执行
    StringID error;          // 自定义错误
    CommandCallback callback; // 执行完成回调
    bool no_estimate_flag;   // 跳过费用估算
    bool automatic_flag;     // 自动命令（不计入 APM）
    virtual bool _post(...) = 0;  // 发命令
    virtual CommandCost _do(...) = 0; // 直接执行
    virtual Commands get_command() = 0;
    // 链式 API：
    Command &as_company(CompanyID c);  // 以公司 c 执行
    Command &with_callback(fn);        // 完成后回调
    Command &set_auto();               // 自动命令
    Command &no_estimate();            // 跳过估算
};
```

**核心价值**：

1. **命令可以排队/组合/延迟执行**——蓝图重放、命令回放的基石（`BuildBlueprint` 里 `last_rail->post()` 按顺序发命令）
2. **回调链**——命令完成后再执行后续逻辑（车站拆分段重建时先建站→回调再续建）
3. **以任意公司身份执行**——联机录像回放不同玩家的操作
4. 生成器 `generated/cm_gen_commands`（2251 行）自动把**每个 vanilla 命令**包装成 `CmdXxx` 类（BuildRailStation、BuildRailWaypoint 等 100+ 命令）

**对 jrpm 的参考价值**：★★★★★

- jrpm 的 `Commands` 枚举 + `DEF_CMD_TUPLE` 体系更强（模板化），但**没有**这层"可编程命令对象 + 回调链"抽象
- 如果做蓝图/命令重放，**建议借鉴这个设计**（用 jrpm 的 `Command<T>::Do/Post` 重新实现一个类似的 Command 对象层），而不是直接搬 vanilla 代码

### 🥈 B. 事件分发系统（cm_event.hpp）—— 数据追踪的地基

14 种游戏事件 + 带优先级槽位的 TypeDispatcher：

```cpp
event::TownBuilt{Town*};           event::HouseBuilt{Town*, TileIndex, HouseID};
event::TownGrowthSucceeded{Town*, TileIndex, prev_houses};
event::HouseRebuilt / HouseCleared / HouseDestroyed / HouseCompleted;
event::CargoDeliveredToIndustry{Industry*, CargoType, amount, Station*};
event::CargoAccepted{Company*, CargoType, amount, Station*, Money profit, Source};
event::CompanyMoneyChanged{Company*, delta};  // 资金/贷款/余额变化
// 优先级槽位：GOAL=10 / CONTROLLER=20 / GAME=30 / RECORDER=50
```

**核心价值**：

1. 轻量级发布-订阅，`std::type_index` 按事件类型分发
2. **Slot 优先级**设计（谁先谁后处理事件）比 jgrpp 现有的 `TimerGameTick` 监听更灵活
3. 城镇增长追踪、货物流向统计、公司财务监控都建在这个事件层上

**对 jrpm 的参考价值**：★★★★

- jrpm 有 `TimerGameTick::Elapsed` 等 tick 回调，但**没有这种细粒度游戏事件总线**
- 做城镇分区（growth_tiles）或统计功能时，这个事件层可以**借鉴设计**（或在 jgrpp 现有机制上实现同等粒度）

### 🥉 C. 位流序列化（cm_bitstream + cm_base64）—— 紧凑编码方案

```cpp
class BitOStream {
    void WriteBytes(uint32_t value, int amount);  // 按位写入
    void WriteBytes64(uint64_t value, int amount);
    void WriteMoney(Money value);                 // Money 专用编码
};
```

**核心价值**：按位压缩的命令/事件序列化——联机命令日志、蓝图数据、回放文件用它做 lzma 压缩前的紧凑编码。

**对 jrpm 的参考价值**：★★★（工具类，需要时直接借鉴思路）

### 🎮 D. 玩法命令（cm_console_cmds）—— 可直接借鉴

| 命令                         | 功能           | 参考价值                 |
| -------------------------- | ------------ | -------------------- |
| `gamespeed <n>`            | 游戏速度调节       | ★★★ 实用（jgrpp 有类似？需查） |
| `step <n>`                 | 步进 N 帧（逐帧调试） | ★★★ 调试神器             |
| `export`                   | 导出数据         | ★★                   |
| `treemap`                  | 树木地图         | ★★                   |
| `reset_town_growth`        | 重置城镇增长记录     | ★★（配合分区）             |
| `load_commands`            | 加载命令日志回放     | ★★★★ 回放入口            |
| `start_record/stop_record` | 开始/停止录制      | ★★★                  |
| `gamestats`                | 游戏会话统计       | ★★                   |
| `gfxdebug`                 | 图形调试         | ★★                   |

### 结论：CM 服务器专属模块的参考策略

| 模块                                          | 结论                                        |
| ------------------------------------------- | ----------------------------------------- |
| **cm_command_type（命令对象层）**                  | ✅ **强烈建议借鉴设计**——做蓝图/回放的前提，用 jrpm 命令体系重新实现 |
| **cm_event（事件总线）**                          | ✅ 建议借鉴——城镇增长/统计功能的地基                      |
| **cm_bitstream/base64**                     | 🔧 工具类，需要时移植                              |
| **gamespeed/step 命令**                       | ✅ 小而实用，可顺手加                               |
| **cm_survey / cm_newgrf_revisions / 服务器协议** | ❌ 纯 CM 服务器配套，无移植价值                        |

