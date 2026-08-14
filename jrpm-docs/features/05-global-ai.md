---
title: 整局感知 AI 接口
---

## 现状（调研结论）

- NoAI 框架：`src/ai/`（ai_core/ai_instance/ai_scanner/ai_gui/ai_config）——原版 AI 体系**原样保留**，本功能不触碰；
- 脚本 API 体系：`src/script/api/script_*.hpp/.cpp`（60+ 类），Squirrel 绑定由构建期工具**自动生成**：
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` 自动发现新 API 类（生成 `ai_*.sq.hpp` / `gs_*.sq.hpp`），**新增类无需手工注册**；
  - `.cpp` 需加入 CMake 源列表；类注释 `@api ai game` 控制暴露给 AI/GS；
- 公司数据：`company_base.h` `Company`（money/current_loan/old_economy[quarter]（company_value、performance_history）/group_all[type].num_vehicle/months_of_bankruptcy）、`GetAvailableMoney()`；
- GS 神模式：`ScriptCompanyMode::IsDeity()`；
- 现有限制：AI 默认只能可靠访问自己公司数据，无整局聚合 API、无接入开关。

**结论**：新增一个 `ScriptGlobal` API 类（自动注册）+ `game.script.allow_global_ai_access` 开关即可实现「保留 NoAI、新增整局感知 AI、带接入控制」。

## 本功能实现

### 1. `ScriptGlobal` API（`src/script/api/script_global.hpp/.cpp`）

暴露给 AI 与 GS（`@api ai game`）的静态方法：

| 方法 | 返回 | 数据源 |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| 设置开关` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`、`GetDate()`、`GetYear()` | int | `MapSizeX/Y`、`EconTime::CurDate`、`CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle`（VT_TRAIN/ROAD/SHIP/AIRCRAFT） |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` 按 owner 计数 |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- 公司 ID 复用 `ScriptCompany::CompanyID`（COMPANY_SELF 解析到当前公司）；
- **接入控制**：所有方法首行校验 `IsGlobalAccessAllowed()`，不满足返回 -1/nullopt/false（GS 恒可用，AI 受开关控制）；
- `ScriptGlobalCompanyList : ScriptList`：`ScriptList::FillList<Company>` 枚举全部公司。

### 2. 接入开关（设置）

- `game.script.allow_global_ai_access`（`src/table/settings/script_settings.ini` `[SDT_BOOL]`，默认 false，`SC_EXPERT`）；
- 结构体字段 `ScriptSettings::allow_global_ai_access`（`src/settings_type.h`）；
- 字符串 `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]`（`src/lang/english.txt`）。

### 3. 示例 AI：`bin/ai/GlobalAI/`

- `info.nut`（GlobalAIInfo） + `main.nut`（GlobalAI : AIController）；
- 演示：读取全部公司财务/车辆/车站/评级与地图信息并打日志；说明接入控制（未开开关时提示）；
- 该 AI 即「感知整局游戏 + 接入控制」的参考实现，后续 AI 控制逻辑可在其 `Start()` 内扩展。

### 涉及文件

- 新增 `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt`（加 .cpp）
- `src/table/settings/script_settings.ini`、`src/settings_type.h`、`src/lang/english.txt`
- 新增 `bin/ai/GlobalAI/info.nut`、`main.nut`

## 验证要点

1. 新增 AI 在 AI 配置界面可见（扫描 `ai/GlobalAI`）；
2. 未开启 `game.script.allow_global_ai_access` 时，AI 日志提示无权限；开启后输出各公司完整数据；
3. GameScript 无需开关即可用 `GSGlobal` 访问；
4. 构建期自动生成 `ai_global.sq.hpp` / `gs_global.sq.hpp` 无报错（依赖 `file(GLOB)`，需重新 configure CMake）。

## 扩展方向

- 增加经济/贷款/基建（`GetCompanyInfrastructure`）等更多聚合数据；
- 增加事件（公司破产/收购/新公司建立）订阅；
- 把 AI 决策逻辑做成可配置参数（`GetSettings()`）。
