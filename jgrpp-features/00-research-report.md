# OpenTTD jgrpp 项目结构与代码组织调研报告

> 调研对象：`G:\GitHub\OpenTTD-patches`（jgrpp 源码工作区）
> 参考资源：`G:\game\openttd-jgrpp`（已编译完整游戏，只读）
> 调研日期：2026-08-14
> 分支：jgrpp（当前 HEAD `5b5c452e1b`，约 16.0 版本）

---

## 1. 项目概览

OpenTTD jgrpp（JGR's Patchpack）是基于 OpenTTD 的知名增强分支，本工作区源码对应版本 **16.0**，含大量 JGR 私有特性（tracerestrict、programmable signals、scheduled dispatch、模板替换、增强信号等）。

| 项目 | 说明 |
|---|---|
| 语言 | C++20（`CMAKE_CXX_STANDARD 20`，无扩展） |
| 构建 | CMake（≥3.17），`src/CMakeLists.txt` 按目录组织 |
| 脚本引擎 | Squirrel（`src/3rdparty/squirrel/`），用于 AI / GameScript / 模板脚本 |
| 网络 | 自研 TCP/UDP 协议层（`src/network/`），HTTP 用 WinHttp（Windows）/ libcurl（其他）/ JS（Emscripten） |
| 存档 | `src/saveload/` + `src/sl/` 自研二进制格式 |
| 线程 | `src/thread.h` + `src/worker_thread.cpp`（WorkerThreadPool 任务池）、`src/timer/` 定时器 |

---

## 2. 源码目录结构与模块划分

顶层：`CMakeLists.txt` 为根构建脚本；`src/` 为全部 C++ 源码；`bin/` 为运行时数据（AI 兼容脚本、语言文件等）；`media/`、`os/`、`cmake/`、`docs/` 分列资源、平台代码、构建脚本与文档。

`src/` 内主要子目录与职责：

| 目录 | 职责 |
|---|---|
| `src/core/` | 基础工具：pool 类型、bitset、容器、线程封装、位运算等 |
| `src/network/` | 联网：服务器/客户端/UDP/HTTP/内容下载/管理员协议（`core/` 子目录为协议基础） |
| `src/script/` | 脚本运行框架：Squirrel 封装、实例、配置；`api/` 为暴露给 AI/GS 的全部 API 类 |
| `src/ai/` | NoAI 框架（AI 实例、扫描器、配置、GUI） |
| `src/game/` | GameScript 框架（GS 实例、配置、GUI） |
| `src/newgrf/` | NewGRF 解码与处理 |
| `src/pathfinder/` | 寻路（YAPF、NPF） |
| `src/saveload/`、`src/sl/` | 存档读写 |
| `src/blitter/`、`src/video/`、`src/fontcache/`、`src/music/`、`src/sound/` | 渲染/音视频后端 |
| `src/lang/` | 本地化字符串源（strgen 生成） |
| `src/table/` | 静态表；`table/settings/*.ini` 为**设置定义源**（settingsgen 生成） |
| `src/3rdparty/` | 第三方库（squirrel、llvm、icu 等） |
| `src/timer/`、`src/os/`、`src/misc/` | 定时器、平台、杂项 |

顶层散列文件按系统命名（如 `rail_gui.cpp`、`group_cmd.cpp`、`vehicle.cpp`、`order_cmd.cpp`、`economy.cpp`），遵循 OpenTTD 惯例：`*_cmd` 命令逻辑、`*_gui` 窗口、`*_base/_type/_func` 数据结构与内联函数。

---

## 3. 构建方式

- **CMake 三步**：`cmake -B build ..` → `cmake --build build` → 产物 `openttd.exe`。仓库自带 `build.sh` / `build-dedicated.sh`。
- **依赖**：`vcpkg.json` 声明（zlib、lzma、lzo、zstd、png、SDL2、freetype、harfbuzz、icu、opus 等）；Windows 用 WinHttp（无需 curl），非 Win 用 libcurl（`CMakeLists.txt:121-127`）。
- **工具链（host tools）**：`strgen`（语言文件）、`settingsgen`（从 `src/table/settings/*.ini` 生成设置代码）、`squirrel_export`（从 `src/script/api/script_*.hpp` 生成 Squirrel 绑定）。
- **关键生成物**：`generated/script/api/<ai|gs>/...sq.hpp`（API 绑定，**由 `file(GLOB script_*.hpp)` 自动发现，新增 API 类无需改注册列表**，只需加 `.hpp` 并把 `.cpp` 加入 `src/script/api/CMakeLists.txt` 源列表）；`generated/rev.cpp`；`generated/ottdres.rc`。
- **设置系统**：现代版已改为 **INI 驱动**——`src/table/settings/*.ini`（含 `[SDTC_VAR]` 段、`cat=SC_*` 分类、`flags`、`post_cb` 等），由 settingsgen 生成 `settings_*.cpp/h`；对应结构体成员在 `src/settings_type.h`（如 `NetworkSettings` 在 575 行起）。

---

## 4. 五大功能系统实现位置

### 4.1 资源下载（内容下载 / BaNaNaSplit）

| 关注点 | 位置 |
|---|---|
| 内容客户端主类 | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler`（兼 `ContentCallback` + `HTTPCallback`） |
| 内容下载 GUI | `src/network/network_content_gui.cpp/.h` |
| HTTP 客户端 | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`，异步事件驱动（非阻塞，主循环轮询） |
| 内容服务器连接串 | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`：环境变量 `OTTD_CONTENT_SERVER_CS`，默认 `content.openttd.org`（TCP 元数据协议） |
| 镜像 URI | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`：环境变量 `OTTD_CONTENT_MIRROR_URI`，默认 `https://binaries.openttd.org/bananas` |
| 下载流程 | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()`（POST 全部 content ID 到镜像，镜像回一个多文件 tar 流，逐文件写盘）→ `AfterDownload()` 做 gunzip + `TarScanner` 解包 |
| 解压 | `GunzipFile()`（zlib）、`TarScanner`/`ExtractTar`（`src/tar_type.h` / `src/fileio.cpp`） |
| 线程设施（可用于并行） | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`；`src/thread.h` 为平台线程封装 |

**现状结论**：① 只有 1 个镜像且只能靠环境变量配置，无游戏内设置；② 下载是**单连接、串行**的（一次 POST 全部文件）；③ 解压在主线程同步执行。→ 多线程/多镜像改造点清晰。

### 4.2 服务器在线玩家 / 公司上限

| 关注点 | 位置 |
|---|---|
| 客户端上限常量 | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| 客户端池 | 同文件 `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`；`ClientID` 为 `uint32_t` |
| 公司 ID 池 | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`；假公司占 253/254/255 |
| 公司掩码 | 同文件 `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>`（16 位，只能跟踪 16 家公司） |
| 服务器接纳判断 | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`；`static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| 客户端设置项 | `src/table/settings/network_settings.ini:231/241` — `network.max_companies`（def 15, max MAX_COMPANIES）、`network.max_clients`（def 25, max MAX_CLIENTS）；结构体在 `src/settings_type.h` `NetworkSettings` |
| **协议位宽（硬约束）** | `src/network/core/network_game_info.cpp` — `companies_max` 与 `clients_max` 均以 **`Send_uint8`/`Recv_uint8`** 发送（251-296、422-432 行） |
| 服务器列表显示 | `src/network/network_gui.cpp:519` 等 |

**现状结论**：
- **客户端上限 = 255 已是协议上限**（uint8 字段 + 游戏协调器/服务器浏览器兼容）。要突破必须把 `network_game_info` 相关字段改成 uint16（client↔server 全链路 + UDP 广播 + 游戏协调器协议），属于对外协议变更。
- **公司上限 = 15**（`CompanyIDTag` 的 End=0xF）。可安全提升到 **252**（End=0xFC）：底层仍是 uint8，存档字节宽度不变（兼容旧档），需同步放宽 `CompanyMask`（uint16→uint32）与界面/循环假设。

### 4.3 车辆分组

| 关注点 | 位置 |
|---|---|
| 分组数据结构 | `src/group.h` — `Group : GroupPool::PoolItem`（name/owner/vehicle_type/flags/livery/statistics/parent/number）；`GroupID`、`DEFAULT_GROUP`、`IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| 分组命令 | `src/group_cmd.cpp` — `CmdCreateGroup`(536)、`CmdDeleteGroup`(585)、`CmdAlterGroup`(646)、`CmdAddVehicleGroup`、`CmdAddSharedVehicleGroup`（把共享订单车辆加入已有组，718 行附近 `AddVehicleToGroup`） |
| 命令注册 | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`；枚举在 `src/command_type.h` `enum class Commands`（492 行起） |
| 分组 GUI | `src/group_gui.cpp/.h`、`src/vehiclelist.cpp` |
| 车辆↔分组 | `src/vehicle_base.h`（`Vehicle::group_id`）、`SetTrainGroupID/UpdateTrainGroupID`（group.h:130-131） |
| 订单/共享调度 | `src/order_base.h`（`OrderList`、`VehicleOrdersID`）、`src/order_cmd.cpp`、`src/order_func.h`、`src/schdispatch.h/.cpp`（scheduled dispatch，绑定到 order list） |
| 分组统计 | `GroupStatistics`（group.h:60-66），`GetGroupNumVehicle` 等（group.h:125-128） |

**现状结论**：已有 `CmdAddSharedVehicleGroup`（把一个车辆的共享订单车辆加入某组）与 `CmdCreateGroupFromList`（从列表建组），但**没有「按共享订单自动建组/归组」的完整逻辑**。新命令 `AutoGroupSharedOrders`（遍历公司全部主车辆 → 按 `OrderList` 聚合 → 自动建组并归组）落地路径清晰。

### 4.4 建造 tooltip（铁轨等建造时鼠标上方价格提示）

| 关注点 | 位置 |
|---|---|
| 铁轨建造 GUI/逻辑 | `src/rail_gui.cpp`（`BuildRailToolbarWindow`）、`src/rail_cmd.cpp`、`src/rail.h/.cpp`；道路 `road_gui.cpp/road_cmd.cpp` |
| 成本估算 | 各 `*_cmd.cpp` 的 `DoCommand` 返回 `CommandCost`；GUI 内可用 `DC_QUERY_COST` 模式询价 |
| 鼠标下 tile | `src/viewport_func.h:36` — `GetTileBelowCursor()`；`_cursor.pos`（屏幕坐标）；`src/viewport.cpp:1056` |
| 现有文本提示机制 | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)`（世界坐标浮动文本）、`UpdateTextEffect`；`src/texteff.cpp` |
| 现有建造提示 UX | 铁轨工具栏 `OnPlaceDrag` 拖拽时有选中区域与成本累计（`_thd` tilehighlight，`src/tilehighlight_func.h`）；状态栏 `statusbar_gui.cpp` 可显示工具成本 |
| 每帧刷新点 | 各工具栏窗口 `OnMouseLoop` / `viewport.cpp` 的 `HandleMouseEvents`（5422/5733 行） |

**现状结论**：无「鼠标上方价格 tooltip」。可用 `AddTextEffect` 锚定鼠标所在 tile 实现（跟随光标所在格），或自绘屏幕坐标 tooltip；成本可用 `DC_QUERY_COST` 对当前工具做单 tile 询价 + 拖拽选区累计。

### 4.5 AI 接口（NoAI 保留 + 整局感知 AI）

| 关注点 | 位置 |
|---|---|
| NoAI 框架 | `src/ai/` — `ai_core.cpp`（AICore 主循环）、`ai_instance.cpp`（AIInstance/Squirrel VM）、`ai_scanner.cpp`（扫描 `ai/` 目录）、`ai_gui.cpp`（选择/配置）、`ai_config.cpp` |
| GameScript 框架 | `src/game/` — `game_core.cpp`、`game_instance.cpp` 等（GS 为“神模式”，权限高于 AI） |
| 脚本 API 类 | `src/script/api/script_*.hpp/.cpp`（`script_company`、`script_map`、`script_vehicle`、`script_industry`、`script_town`、`script_game`、`script_admin` 等，共 60+ 类） |
| API 自动注册 | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` 自动生成 `ai_*.sq.hpp`/`gs_*.sq.hpp` 绑定；`.cpp` 需加入源列表（235 行起） |
| Squirrel 绑定宏 | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`；`ai/ai_controller.sq.hpp` 为 AI 控制器绑定 |
| 实例调度 | `src/script/script_instance.cpp`、`src/script/script_suspend.hpp`（挂起/恢复）、事件 `script_event*` |
| 公司访问控制 | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`；`ScriptCompanyMode`（`IsDeity()`）区分 GS 神模式；公司 API 参数校验 `ResolveCompanyID`、`EnforceCompanyModeValid` |
| 脚本设置 | `src/table/settings/script_settings.ini`（`game.script.*`）；AI 实例配置 `ai_config.cpp` |

**现状结论**：现代 script API 中，竞争对手的部分信息（如 `GetBankBalance`）已不设限，但**没有面向整局感知的聚合 API**（无“枚举全部公司/全局经济/全局地图统计”的统一入口），也没有按开关控制的“全局 AI”接入点。新增一个 `ScriptGlobal` API 类（GLOB 自动注册）+ 一个“允许 AI 全局感知”的 `game.script` 设置即可实现，GS 恒可用（神模式），AI 受开关控制——即“保留 NoAI、新增整局感知 AI、带接入控制”。

---

## 5. 修改风险总览

| 功能 | 主要改动文件 | 风险 |
|---|---|---|
| F1 下载多源/多线程 | `src/table/settings/network_settings.ini`、`src/settings_type.h`、`src/network/core/config.cpp`、`src/network/network_content.h/.cpp` | 中（网络回调状态机需谨慎） |
| F2 上限扩展 | `src/company_type.h`、`src/table/settings/network_settings.ini`、`src/network/core/network_game_info.cpp`（可选 uint16） | 低-中（公司 252 低风险；客户端 >255 需协议变更） |
| F3 车辆自动分组 | `src/group_cmd.h/.cpp`、`src/command_type.h`、`src/console_cmds.cpp`、`src/group_gui.cpp`、`src/lang/english.txt` | 低 |
| F4 建造 tooltip | 新 `src/construction_cost_tip.h/.cpp`、`src/rail_gui.cpp`/`road_gui.cpp` 钩子、`src/lang/english.txt` | 低-中 |
| F5 整局感知 AI | 新 `src/script/api/script_global.hpp/.cpp`、`src/script/api/CMakeLists.txt`、`src/table/settings/script_settings.ini`、示例 AI `bin/ai/GlobalAI/` | 低（框架自动注册） |

> 注：全部改动均基于 `jgrpp` 分支本地工作区，未编译验证（本机无可用构建工具链）；所有补丁可在 `git diff` 审查后 `git apply`。
