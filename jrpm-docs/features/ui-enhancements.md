---
title: 联机 UI 增强（位置书签 / 货运明细 / 观战）
---

# 联机 UI 增强

第二批移植自 **citymania-org/cmclient**，以**控制台命令**形式落地（不依赖 cmclient 的热键/工具栏基础设施，稳定且可脚本化）。

## 位置书签（Viewport Locations）

保存/恢复主视口位置与缩放（9 个槽位），适合联机时快速往返于自己的工厂、车站与对手区域。

```
savelocation <1-9>    # 保存当前视口位置+缩放
gotolocation <1-9>    # 跳转到已保存的位置
```

## 公司货运明细（Company Cargo Details）

按货物列出公司**已送达运量**的统计窗口，可切换**总计 / 上个月**两个周期。

```
company_cargo <company_id>    # 打开货运明细窗口
```

- 窗口列出每种标准货物的运量 + 底部总计
- 点击表头"Cargo"切换总计/上月周期
- 说明：jrpm 的 `CompanyEconomyEntry` 不含**每货物收入**字段（cmclient 为此扩展了存档格式），为避免破坏存档兼容，本版本只提供运量列；每货物收入需后续存档扩展

## 观战辅助（Watch）

旁观者快速定位到某公司的建设区域（跳转到该公司最后一次建造的坐标）。

```
watch <company_id>    # 视口跳转到公司所在位置
```

## 已跳过项与理由

| cmclient 功能 | jrpm 处理 |
|---|---|
| 玩家列表悬浮层（cm_client_list_gui） | jgrpp **已有** Online Players 窗口（`NetworkClientList`），不重复实现 |
| 地面详情提示（cm_tooltips） | jrpm 的 LandInfoWindow **已覆盖**房屋/工业/车站详情展示 |

## 涉及文件

- `src/jrpm_locations.cpp/.h`（新增：位置书签 + company_cargo + watch 控制台命令）
- `src/jrpm_cargo_table.cpp/.h`（新增：货运明细窗口）
- `src/window_type.h`（新增 `WindowClass::CompanyCargos`）
- `src/console_cmds.cpp`（命令注册）
- `src/lang/english.txt`（STR_JRPM_CARGOS_* 字符串）
