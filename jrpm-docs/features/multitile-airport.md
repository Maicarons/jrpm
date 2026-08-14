---
title: 模块化机场（multitile-airport）
---

# 模块化机场（multitile-airport）

> 来源：pulsexlb/OpenTTD-patches 的 `jgrpp-multitile-airport` 特性分支，经 git merge 并入 jrpm。

## 功能简介

多格（multitile）模块化机场重构：把机场从「固定式机场类型」改为**可自由改造布局**的多格系统：

- **多格机场**：机场由多个功能格组成（跑道、滑行道、停机坪、航站楼、直升机坪），可组合成任意布局；
- **机场布局改造**：开启 `station.allow_modify_airports` 后，可在已有机场上增删/调整格位；
- **air 类型体系**：新增 `air.h`/`air_type.h`/`newgrf_airtype.*`——把航空类型（固定翼/直升机等）抽象为可扩展的 air type 体系，NewGRF 可定义新航空类型与精灵；
- **PBS 空中调度**：`pbs_air.*`——跑道/滑行道占用与信号预约的空中版本，支持多机并发滑行；
- **YAPF 航空寻路**：飞机在地面（滑行/等待）与空中的路径规划走 YAPF 体系。

## 核心能力

| 能力 | 说明 |
|---|---|
| 机场布局改造 | `station.allow_modify_airports`（默认关闭；开启后可改已有机场） |
| 默认航空类型 | `gui.default_air_type` |
| 多格机场精灵 | 重新构建的 openttd.grf（air type sprites）、透明精灵修复 |
| 飞机行为 | 跑道占用、滑行转向、起飞/降落排队、直升机坪、航天器绘制 |
| NewGRF 兼容 | airtype 精灵加载、机场 NewGRF 回调 |
| 存档 | `SLV_MULTITILE_AIRPORTS` 存档版本 |

## 使用方式

1. 游戏设置开启 `station.allow_modify_airports`；
2. 建造机场后，使用机场改造工具调整跑道/停机位/航站楼布局；
3. 在 `gui.default_air_type` 选择默认航空类型；
4. 配合航空 NewGRF 使用自定义 air type。

## 相关代码

- 航空类型：`src/air.h`、`src/air_type.h`、`src/newgrf_airtype.*`
- 空中调度：`src/pbs_air.*`
- 飞机/机场命令：`src/aircraft_cmd.cpp`（3600 行重构）、`src/airport_cmd.cpp`、`src/airport_gui.cpp`
- 寻路：`src/pathfinder/yapf`（航空部分）
- 存档：`src/sl/saveload_common.h`（`SLV_MULTITILE_AIRPORTS`）

## 注意

- 该特性是航空系统的大规模重构（aircraft_cmd.cpp 重构 3600+ 行），**建议真机编译后重点回归**：飞机购买/起飞/降落、跑道占用、机场 GUI、存档加载；
- 合并时删除了工作区无引用的旧类型（`VehicleAirFlags`、`AirportMovingDataFlag`），确认无其他文件引用；
- 存档中已有机场如需改造，先备份存档。
