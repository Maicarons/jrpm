# JRPM 服务端客户端联调 E2E 测试报告

- 测试日期：2026-08-14
- 测试对象：openttd-jrpm（jrpm 分支，版本 jrpm-0.1.0）
- 测试环境：本机真机（Windows），dedicated 服务器 + 客户端双进程
- 构建产物：`build/openttd-jrpm.exe`（Release）

## 一、测试结论总览

| 测试项 | 结果 | 备注 |
|---|---|---|
| 基础联机流程 | ✅ 通过 | 服务器启动 → 地图生成 → 监听 → 客户端加入 → 建公司 → TCP 保持 |
| 新增功能 F1 资源下载 | ✅ 代码层通过 | 多镜像设置、并行下载并发数设置可配置且正常加载 |
| 新增功能 F3 车辆自动分组 | ✅ 通过 | autogroup 控制台命令注册与执行正常（无车辆时静默跳过） |
| 新增功能 F4 建造 tooltip | ✅ 代码层通过 | 模块、字符串、钩子完整；GUI 效果需人工确认 |
| 新增功能 F5 整局感知 AI | ✅ 通过 | GlobalAI 实机加载运行，全部 API 返回正确数据（修复 2 个 bug） |
| 服务器性能参数 | ✅ 通过 | 6 项参数可配置，服务器正常运行 |
| 多版本兼容 | ✅ 服务器侧通过 | jrpm 服务器接受 jrpm/jgrpp-/pxp 修订（代码路径验证） |

## 二、E2E 联机实测记录

### 1. 服务器启动
```
命令: openttd-jrpm.exe -D -c G:/tmp/ottd-srv3/openttd.cfg -g 42
结果: Map generated, starting game → TCP 0.0.0.0:3979 LISTENING
```

### 2. 客户端加入（jrpm 客户端）
```
命令: openttd-jrpm.exe -n 127.0.0.1:3979 -c G:/tmp/ottd-cli3/openttd.cfg
服务器日志:
  *** Game paused (connecting clients)
  *** JRPM Test Client has joined the game (Client #2)
  [server] Client #2 (127.0.0.1) joined as JRPM Test Client, pubkey: ...
  *** JRPM Test Client has started a new company (#1)
  *** Game unpaused (connecting clients)
TCP: 127.0.0.1:3979 ↔ 127.0.0.1:xxxxx ESTABLISHED（双向）
```

### 3. 进程存活验证
```
服务器: OpenTTD Dedicated Server — Responding=True
客户端: OpenTTD jrpm-0.1.0-1-g053937a1a-m — Responding=True
```

## 三、功能验证详情

### F3 车辆自动分组
- 控制台命令 `autogroup` 已注册（`IConsole::CmdRegister("autogroup", ConAutoGroup)`）
- 支持 `train|road|ship|aircraft` 四种车型
- 命令实现 `CmdAutoGroupSharedOrders`：按共享订单链自动建组，跳过已分组车辆，组名自动生成
- 通过 autoexec.scr 实机执行验证：命令被识别、无错误（服务器模式无本地公司时静默跳过，符合预期）

### F5 整局感知 AI（GlobalAI）
- AI 扫描识别正常（服务器搜索路径含 GlobalAI）
- **修复 Bug 1**：`AILog.Info(msg, AILog.INFO)` → `AILog.Info(msg)`（`AILog.INFO` 常量不存在，脚本运行时 `index INFO does not exist`）
- **修复 Bug 2**：访问被拒时 `Start()` 直接 return → 脚本被判定死亡；改为 sleep 循环保持存活
- 开启 `game.script.allow_global_ai_access` 后实机运行输出：
```
[script:4] [0] [I] Whole-game perception demo:
[script:4] [0] [I] Companies in game: 1
[script:4] [0] [I]   GlobalAI Demo: balance=100000 loan=100000 value=0 rating=0 vehicles(t/r/s/a)=0/0/0/0 stations=0
[script:4] [0] [I] Map size: 256x256, year: 1950, date: 712244
```
- 验证的 API：IsGlobalAccessAllowed / CompanyList / GetCompanyName / GetCompanyBankBalance / GetCompanyLoan / GetCompanyValue / GetCompanyPerformanceRating / GetCompanyVehicleCount / GetCompanyStationCount / GetMapSizeX / GetMapSizeY / GetYear / GetDate —— **全部正常返回**

### F1 资源下载（代码层验证）
- 设置 `network.content_mirrors`（逗号分隔多镜像）已注册，`NetworkContentMirrorUris()` 正确解析
- 设置 `network.content_download_parallel`（默认 4，范围 1-8）已注册，下载会话上限 `std::max<size_t>(1, _settings_client.network.content_download_parallel)`
- 配置 `content_mirrors = ...`、`content_download_parallel = 6` 后服务器正常启动无错误

### F4 建造 tooltip（代码层验证）
- 模块 `construction_cost_tip.cpp/h` 已编译进产物
- `UpdateConstructionCostTip` / `HideConstructionCostTip` 完整
- 字符串 `STR_CONSTRUCTION_COST_TOOLTIP`（Estimated cost: {CURRENCY_LONG}）已定义
- 三个工具栏（rail/road/terraform）OnMouseLoop 钩子已接入（真机 GUI 效果需人工点击确认）

### 服务器性能参数
- 6 项参数全部可配置且服务器正常加载：sync_freq=20 / frame_freq=1 / commands_per_frame=16 / bytes_per_frame=4096 / max_join_time=60 / max_download_time=120

### 多版本兼容（服务器侧）
- `IsNetworkCompatibleVersion()` + `IsJgrppNativeNetworkRevision()`（jgrpp- 前缀）+ `IsPxpNetworkRevision()`（pxp 前缀）
- 服务器加入检查：`revision_ok = 兼容(jrpm) || jgrpp- || pxp`，NewGRF 版本严格校验
- 说明：原版 jgrpp 0.73.1 客户端窗口可启动但需 GUI 手动输入服务器地址加入（老客户端行为），自动化未覆盖

## 四、E2E 发现并修复的 Bug 汇总

| Bug | 位置 | 影响 | 状态 |
|---|---|---|---|
| NOT_REACHED 崩溃（opntitle 旧档错位） | window.cpp:1250 | 启动即崩 | 已修复（d8218458，移除旧 opntitle） |
| 客户端无法加入（client_name 空） | 配置文件路径/private.cfg | 连接被拒 | 已修复（d8218458） |
| AILog.INFO 不存在 | GlobalAI/main.nut | AI 脚本运行时报错 | 已修复（e5ac2dbb） |
| AI Start() 无 Sleep 判死 | GlobalAI/main.nut | AI 加载后崩溃 | 已修复（e5ac2dbb） |

## 五、遗留事项

1. F4 建造 tooltip 的 GUI 交互效果（鼠标悬停显示价格）需人工进游戏点击确认
2. F3 自动分组在**有车辆**的真实场景（建 2+ 辆共享订单的车后点按钮）需人工确认
3. F1 真实内容下载（连接 BaNaNaSplit/镜像拉取文件）需 GUI 操作触发，未自动化
4. jgrpp 0.73.1 客户端手动加入 jrpm 服务器需人工验证

## 六、测试配置参考

服务器 `G:/tmp/ottd-srv3/openttd.cfg`：
```
[version]
ini_version = 8
[misc]
personal_dir = G:/tmp/ottd-srv3
[network]
server_port = 3979
content_mirrors = https://binaries.openttd.org/bananas
content_download_parallel = 6
sync_freq = 20
...
```
