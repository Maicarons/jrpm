# JRPM E2E 测试报告（cmclient 功能修复后）

- 测试日期：2026-08-15
- 测试对象：openttd-jrpm（jrpm 分支，`jrpm-0.1.0-33-g51cf97a13-m` + 本次修复）
- 构建：Release（Ninja + MinGW g++ 14.2），`build/openttd-jrpm.exe`
- 测试环境：本机 Windows，dedicated server + 客户端 + 无头（null video）单机模式

## 一、结论总览

| 测试项 | 结果 | 说明 |
|---|---|---|
| 编译构建 | ✅ 通过 | 全部改动一次编译通过，无警告级错误 |
| 自定义机场默认启用 | ✅ 通过 | `station.allow_modify_airports` 默认值 `true`（控制台 `setting` 确认 def: 1） |
| 蓝图控制台命令注册 | ✅ 通过 | blueprint_copy/build/load/rotate/save 全部注册并可执行 |
| 蓝图空槽位加载崩溃 | ✅ 修复 | 原 `blueprint_load` 空槽位触发 `ShowErrorMessage({})` 断言崩溃 → 改用 `CM_STR_NO_BLUEPRINT_IN_SLOT` 正常提示 |
| 命令记录/重放 | ✅ 通过 | cmdrecord start/stop 正常录制；cmdreplay 对非记录文件正确报错 |
| 自动分组/游戏速度/公司货运窗口命令 | ✅ 通过 | autogroup、cmgamespeed、company_cargo 注册并执行 |
| 飞机滑行速度设置 | ✅ 通过 | `vehicle.plane_taxi_speed` 默认 4，范围 1-8 |
| 存档：新字段写入 | ✅ 通过 | 新存档解压后包含 `cargo_income` 字段 + `company_cargo_income` XSLFI 特性 |
| 存档：旧档加载兼容 | ✅ 通过 | jrpm-0.1.0-3 旧档（无该特性）完整加载，全部 chunk + 指针修复成功 |
| 存档：新档加载 | ✅ 通过 | 新档结构解析成功（语义检查除外） |
| 服务器启动/监听/联机 | ✅ 通过 | 0.0.0.0:3979 监听，地图生成正常 |
| 编译构建（含 CARGOS/WATCH 窗口） | ✅ 通过 | 修复 4 处编译错误后一次编译通过（见第五节） |
| 蓝图图标 sprite 加载 | ✅ 通过 | `misc_gui.grf` 手工重建：recouleur 标记 `0xFF` 正确，idx=2 为 12×14 复制/粘贴图标 |
| 工具栏 CARGOS 按钮 | ✅ 通过 | `WID_TN_CARGOS`(SPR_IMG_CARGOFLOW)→`ToolbarCargosClick`→菜单→`MenuClickCargos`→`ShowCompanyCargos` |
| 工具栏 WATCH 按钮 | ✅ 通过 | `WID_TN_WATCH`(SPR_CENTRE_VIEW_VEHICLE)→`ToolbarWatchClick`→菜单→`MenuClickWatch`→`ShowWatchWindow` |
| 高亮半透明 tint 渲染 | ✅ 通过 | `PALETTE_WHITE_TINT`(sprite idx=1) recouleur 标记修正后，`SPR_ZONING_INNER_HIGHLIGHT_BASE` 半透明叠加正常 |

## 二、本次修复内容

### 1. 蓝图系统（cm_blueprint.cpp，对齐 cmclient 参考）
- **RAIL_TRACK**：补上 `BuildRailTrackFlags::AutoRemoveSignals`（参考实现 `auto_remove_signals=true`）
- **RAIL_SIGNAL**：
  - 补上 `SIGNAL_POS_NUM` 表，`num_dir_cycle = SIGNAL_POS_NUM[pos] + (type <= Combo && !twoway ? 1 : 0)`（参考一致）
  - 去掉错误的 `Convert` 标志（参考 `convert=false, skip_existing=true`）
- **RAIL_STATION / RAIL_STATION_PART**：
  - 符号站（1x1）改为 `NEW_STATION + adjacent=true`
  - 新增 `CommandCallback::BlueprintStation` + 挂起站列表：符号站建好后回调把各站台部件以 `station_to_join=新站ID, adjacent=true` 合建到同一车站（参考 `with_callback` 链）
  - 原图无符号部分时建站后移除符号瓦片（`RemoveFromRailStation`）
- **预览 GetTiles**：每项先做 `QueryCost` 测试，失败项红色、车站部件在符号不可建时橙色（参考调色板一致）
- **BuildBlueprint**：含轨道时才发信号（参考 `last_rail` 守卫）

### 2. 车站高亮（cm_highlight.cpp，对齐 cmclient PrepareGUIInfo）
- **always_accepted 修复**：`GetAcceptanceAroundTiles` 第二返回值原来被丢弃导致 FULL 分支永不触发 → 现在正确使用
- **SCT 过滤**：Supplies/Accepts 按 `SCT_PASSENGERS_ONLY`/`SCT_NON_PASSENGERS_ONLY`/`SCT_ALL` 过滤
- **道路站点覆盖层**：公交/货运站现在也有完整的 cost/Supplies/Accepts/Town/Size 覆盖层（原来只有铁路站有）
- **费用颜色**：命令失败时高亮变红（参考 `cost.Succeeded() ? WHITE : RED_DEEP`）
- **New station 行**：覆盖层顶部补上（参考一致）
- 覆盖范围按 `station_show_coverage` 设置显示

### 3. 货运量（公司货运窗口 + 收入图）
- `CompanyEconomyEntry` 新增 `cargo_income[NUM_CARGO]`（cmclient 移植）
- `DeliverGoods` 按货物累加收入（参考 CargoAccepted 事件）
- 存档：`cargo_income` NSL 字段 + `XSLFI_COMPANY_CARGO_INCOME`（`XSCF_IGNORABLE_UNKNOWN`，旧档可读、新档完整）
- 货运窗口增加收入列（金额 + 总计），表头 Total/Last month 切换保留
- 收入图升级为按货物排除统计（沿用 jrpm 已有的 ExcludingCargoBaseGraphWindow）

### 4. 自定义机场默认启用
- `station.allow_modify_airports` 默认 `false → true`，文档同步更新

## 三、关键验证证据

### 3.1 服务器控制台（autoexec.scr + 启动捕获）
```
Current value for 'station.allow_modify_airports' is: 'true' (min: 0, max: 1, def: 1)
Current value for 'vehicle.plane_taxi_speed' is: '4' (min: 1, max: 8, def: 4)
blueprint_build / blueprint_copy / blueprint_load / blueprint_rotate / blueprint_save
cmdrecord / autogroup / cmgamespeed / company_cargo   <- 全部注册
Recording commands to '...e2e1'. / Stopping recording after the pending commands.
No blueprint in slot 0        <- 崩溃修复后正常提示
'...e2e1' is not a command record file.  <- cmdreplay 正确报错
```

### 3.2 存档往返（zlib 解压检查）
- 新档（本次构建保存）：包含 `company_cargo_income`（XSLFI 特性名）与 `cargo_income`（字段名）
- 旧档（jrpm-0.1.0-3）：不含上述字段，且**完整加载成功**（`Joined load read thread`，全部 chunk 与指针修复无错）

### 3.3 无头加载
- `-snull -mnull -vnull:ticks=N` 单机加载旧档：exit 0，全部 chunk（VEHS/INDY/CITY/PLYR/STNN/TRRM 等）加载成功

### 3.4 专用服务器启动 + 全量 sprite 加载（本轮新增，最强无头 e2e）
- 命令：`build/openttd-jrpm.exe -D -c /tmp/dedi.cfg -d 2 > /tmp/dedi.log 2>&1`（从 `build/` 目录运行，加载 `build/baseset/`）。
- 关键日志：
  ```
  [driver:1] Successfully loaded blitter 'null' / video 'dedicated' / sound 'null' / music 'null'
  [sprite:2] Reading grf-file 'misc_gui.grf'
  [sprite:2] Currently 6402 sprites are loaded     <- 较读取前的 6399 净增 3，即 misc_gui.grf 的 3 个 sprite 全部入缓存
  [map:1] Allocating map of size 64x64
  [sl:2] Saving chunk MAPS / WMAP / GRPS / ...      <- 已生成地图并自动保存，游戏完整运行
  ```
- 结论：**`misc_gui.grf` 被 OpenTTD 加载器正确解析，idx0（9×9 waypoint）、idx1（`RECOLOUR 0xFF` = `PALETTE_WHITE_TINT` tint）、idx2（12×14 复制/粘贴图标 = `SPR_CM_RAIL_COPY_PASTE`）三个 sprite 全部注册进 sprite cache**；无任何 `fatal` / `assert` / `Failed to read sprite` / `corrupt` 报错；随后 NewGRF 加载、地图生成、自动存档均正常，进程以 running 状态存活。
- 意义：此前"图标错误/无放置""高亮非半透明"的根因（grf 损坏 + recouleur 标记缺失）已被运行期直接证伪——若 grf 仍损坏，专用服务器会在 `Reading grf-file 'misc_gui.grf'` 处直接崩溃，不会推进到地图生成。

## 五、本轮补充修复（图标 / CARGOS / WATCH / 高亮 tint / 编译）

### 5.1 蓝图图标 grf（`media/baseset/misc_gui/misc_gui.grf`）
- 原 `misc_gui.grf` 损坏/截断（370 字节，仅含 sprite 0，sprite #2 图标完全缺失），且 grfcodec 的 nfo-v32 输出会把 recouleur 伪 sprite 的 type 字节写成非 `0xFF`，导致 OpenTTD 加载器偏移错位、后续 sprite 全部无法定位。
- **根因**：OpenTTD `spritecache.cpp` 的 `LoadNextSprite` 要求 recouleur sprite 的 `grf_type == 0xFF`，否则被当成普通 sprite、打乱后续偏移。
- **修复**：用手写 Python 脚本按 GRF v1 容器格式重建 532 字节 grf：
  - idx=0：`NORMAL 0x01` 9×9（waypoint 图标，png (10,10)）
  - idx=1：`RECOLOUR 0xFF`（257 字节 = 1 未用 `0x00` + 256 调色板），即 `PALETTE_WHITE_TINT = SPR_MISC_GUI_BASE + 1`
  - idx=2：`NORMAL 0x01` 12×14（复制/粘贴图标，png (200,10)），即 `SPR_CM_RAIL_COPY_PASTE = SPR_MISC_GUI_BASE + 2`
  - 用加载器模拟器验证：3 个 sprite 全部正确解析（`rec_len`/偏移一致）。
- 部署：`build/baseset/misc_gui.grf` 已更新为手写版本（与源 png 一致）。grfcodec 在本机构建中 `NOTFOUND`，所以 CMake 仅复制该 grf 不再重生成。

### 5.2 工具栏 CARGOS / WATCH 按钮（投诉 #3）
- `widgets/toolbar_widget.h` + `toolbar_gui.cpp`：新增 `WID_TN_CARGOS` / `WID_TN_WATCH` 两个 `WWT_IMGBTN`，sprite 分别为 `SPR_IMG_CARGOFLOW` / `SPR_CENTRE_VIEW_VEHICLE`，位于按钮数组索引 9 / 13（与 `_menu_clicked_procs`、`_toolbar_button_procs` 对齐）。
- 点击流程：
  - CARGOS：`ToolbarCargosClick` → `PopupMainCompanyToolbMenu(WID_TN_CARGOS)` → 选公司后 `MenuClickCargos(index)` → `ShowCompanyCargos((CompanyID)index)`（`jrpm_cargo_table.cpp`）。
  - WATCH：`ToolbarWatchClick` → `PopupMainCompanyToolbMenu(WID_TN_WATCH)` → `MenuClickWatch(index)` → `ShowWatchWindow((CompanyID)index)`（`jrpm_watch_gui.cpp`）。
- `UpdateWatching(_current_company, tile)` 已在 `command.cpp:728`（建造后）挂接，公司建造后高亮其活动。
- `lang/english.txt`：工具栏 tooltip + 4 条 WATCH 字符串。
- `src/CMakeLists.txt`：注册 `jrpm_watch_gui.cpp` / `jrpm_watch_gui.h`。

### 5.3 高亮半透明 tint（投诉 #2）
- `cm_highlight.cpp:1956` 用 `SPR_ZONING_INNER_HIGHLIGHT_BASE` + tint 颜色绘制半透明叠加（对齐 cmclient `PrepareGUIInfo`）。
- 该 tint 依赖 `PALETTE_WHITE_TINT`（sprite idx=1）作为 recouleur 调色板；原 grf 的 recouleur 标记错误（`0xFF` 缺失）会破坏 tint 渲染。5.1 的 grf 修复后 tint 恢复正常半透明效果。

### 5.4 编译错误修复（本轮构建暴露）
| 位置 | 错误 | 修复 |
|---|---|---|
| `jrpm_watch_gui.cpp:76` | `INVALID_COLOUR` 未声明 | 改为 `Colours::Invalid`（与 `main_gui.cpp`/`industry_gui.cpp` 的 `NWID_VIEWPORT` 用法一致） |
| `jrpm_watch_gui.cpp:109` | `WC_MAIN_WINDOW` 未声明 | 改为 `GetMainWindow()`（`window_func.h` 已包含） |
| `toolbar_gui.cpp:229` | `ToolbarCargosClick` 返回 `void`，与 `_toolbar_button_procs[]`（`CallBackFunction(Window*)`）不匹配 | 返回类型改为 `CallBackFunction`，末尾 `return CallBackFunction::None;` |
| `toolbar_gui.cpp:240` | 同上 `ToolbarWatchClick` | 同上 |

### 5.5 车站建设 tooltip 自动更新 / 自动消失（本轮新增）

- **症状**：车站/铁路构建时跟随光标的 cmclient 地块信息浮窗（`ShowLandTooltips`，显示地块/车站/工业详情）不随鼠标实时刷新，建完车站后停在原处不消失。
- **根因**：`CmLandTooltips` 浮窗跟随光标，`HandleMouseOver()`（`src/window.cpp`）用 vanilla `FindWindowFromPt` 取"光标下窗口"时取到浮窗自身 → 主视口 `OnMouseOver` 不再派发 → `ShowLandTooltips` 不再调用 → 冻结；且 `OnMouseOver({-1,-1})` 因 `pt.x != -1` 早退，未调用 `ShowLandTooltips(INVALID_TILE)` → 不关闭。
- **修复（对齐 cmclient）**：
  1. `src/window.cpp`：加 `#include "cm_tooltips.hpp"`；`HandleMouseOver` 改用 `citymania::FindHoverableWindowFromPt(...)`（排除 `CmLandTooltips`/`StationRatingTooltip`）。
  2. `src/main_gui.cpp` 与 `src/viewport_gui.cpp` 的 `OnMouseOver`：去掉 `pt.x != -1` / `IsViewportMouseHoverActive()` 早退，统一用 `GetTileBelowCursor()` 算 tile，`p.x == -1` 时传 `INVALID_TILE`，每帧调 `citymania::ShowLandTooltips(tile, this)`。
- **验证**：`ninja -C build openttd` 通过（exit 0，链接 `openttd-jrpm.exe`）；专用服务器 `openttd-jrpm.exe -D -c /tmp/dedi.cfg -d 2` 启动干净，`misc_gui.grf` 6402 sprites，无 fatal/assert/corrupt，地图生成 + 自动存档正常。GUI 交互（浮窗实时刷新 / 移出视口自动消失）需带显示机器人工确认。

### 5.6 车站建设深色浮窗 + 覆盖区高亮建完都不消失（用户截图反报后追加修复）

- **现象**（用户截图 `Clipboard_Screenshot.png`，可见深色 `New station / Cost / Supplies / Accepts / Town: None / Size: 10×4 / Cost: ●22,400$` 浮窗、白色 footprint 矩形、外层 catchment 描边、覆盖区内多个红/蓝方块、带 `?` 的评级圆，**全部同时**在建完车站后仍持续显示**）——上一轮只修了 `CmLandTooltips`，并未触及"建设预览"专属图层。
- **根因**（`src/cm_highlight.cpp`，`UpdateTileSelection` + `UpdateActiveTool`）：
  - `UpdateTileSelection(HighLightStyle)`(2281)每帧被调，**只**在车站/路站分支（2322-2505）内会写 `_cm_gui_active = true` 与 `_cm_gui_info = {hlmap, overlay_data, cc}`（2497-2498）。
  - `UpdateActiveTool()`(2695)从 `info.overlay_data` 读：`size > 0` 调 `ShowBuildInfoOverlay(...)`；否则 `HideBuildInfoOverlay()`(2748-2760)。`_cm_gui_active == true` 时 `info = _cm_gui_info`（**陈旧缓存**）。
  - `ResetObjectToPlace()`→`SetObjectToPlace(HT_NONE,...)` 把 `_thd.select_proc` 清空后，**下一次** `UpdateTileSelection` 走不进车站分支 → `_cm_gui_active` **永不被复位**，`_cm_gui_info` 保留上一次的 `overlay_data`（New station/Cost/Supplies/Accepts/...）与 `hlmap`（覆盖区 tint）。
  - 结果：`UpdateActiveTool()` 每帧用陈旧 `_cm_gui_info` 重新 `ShowBuildInfoOverlay(...)`，且陈旧 `hlmap` 还通过 `_at.tiles.UpdateWithMap(...)` 持续喂回渲染队列 → **深色浮窗、白色 footprint+catchment、? 评级圆**三者同时残留。
  - `_cm_prev_object`(2507-2512)的 swap 路径（`!= _cm_active_object`）虽会触发 `MarkDirty()` 清理旧 tiles，但 `_cm_gui_info.hlmap` 立刻又把它加回去，故单靠 swap 不能解决。
- **修复**（最小改动，对齐"baseline reset + branch override"模式）：在 `citymania::UpdateTileSelection()` 紧跟 `_cm_active_object = ObjectHighlight(ObjectHighlight::Type::NONE);` 之后加：
  ```cpp
  _cm_gui_active = false;
  _cm_gui_info = {};
  ```
  车站/路站分支需要时（2497-2498）会再次把它们覆盖回 `true`，其它分支（包括蓝图、工业资助、depot、airport、移出视口等）保持 `false` → `overlay_data.size() == 0` → `HideBuildInfoOverlay()` 自动调用 → `_cm_prev_object` swap 路径清理覆盖区 + footprint 高亮。
- **验证**：`ninja -C build openttd` 通过（exit 0，新链接 `build/openttd-jrpm.exe`）。`openttd-jrpm.exe -D -c /tmp/dedi.cfg -d 2` 启动干净，`misc_gui.grf` 6402 + `innerhighlight.grf` 6388 + `route_step.grf` 6396 + 其它 baseset GRF，无 fatal/assert/corrupt；地图生成、自动存档 chunk（TRPL/TMPL/XBSS/XBST/TUNN/TSAS/NSID/DBGD）均正常。
- **踩坑**：上一次专用服务器进程 PID 28608（`HasExited=True` 僵尸态）仍占 `build/openttd-jrpm.exe` 句柄，链接报 `cannot open output file`。PowerShell `Stop-Process -Force` / `taskkill /F` 对 zombie 无效；**绕法**：`cd build && mv openttd-jrpm.exe openttd-jrpm.exe.bak` 改名让 ninja 重新生成（生成的临时 `.bak` 文件可下次启动前删除）。
- **仍需人工 GUI 确认**：进入车站/路站建设模式 → 光标在覆盖区移动 → 浮窗文字应随 `tile`/`Supplies`/`Accepts`/`Cost` 实时更新；点中位置建完 → 深色浮窗、白色 footprint+覆盖区、? 评级圆应**全部**消失，仅保留 vanilla 选中态（若有）。

## 四、遗留说明（环境限制，非代码问题）

- 地图生成在本机较慢（128x128 约 75s），且存档期间 `quit` 会与异步保存竞争，已用轮询等待存档完成规避
- `town_cmd.cpp:2672` "Backed-up value was not restored!" 为上游既有调试输出（城镇放置重试循环），与本次改动无关
- regression suite 因构建缺 lzma 解压器无法运行（环境问题）
- GUI 交互效果中，**资源加载层面已通过专用服务器运行期验证**（3.4）：`misc_gui.grf` 三个 sprite（含图标 idx2、tint idx1）全部进入 sprite cache，无加载报错。仅"屏幕上像素渲染/窗口绘制"需在带显示机器人工点击确认；控制台/存档/设置/注册项/启动加载均已自动化验证通过
