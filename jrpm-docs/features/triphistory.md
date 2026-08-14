---
title: 车辆行程历史（Trip History）
---

# 车辆行程历史（Trip History）

源自 **embeddedt/OpenTTD-modded**（第一批移植），已适配 jrpm 的新式字符串与日期 API。

## 功能

每辆车辆都会记住最近 **10 趟行程**的数据，在车辆详情窗口新增的 **History（历史）** 按钮中查看：

| 列 | 含义 |
|---|---|
| Received（到达） | 本趟货物收到的日期 |
| Profit（利润） | 本趟利润（负数显示黄色） |
| % Change | 与上一趟相比的利润变化百分比（绿 + / 红 -） |
| TBT | 与上一趟之间的时间间隔（天） |
| Change | 与上一趟相比的时间变化（天） |
| Occupancy | 本趟平均载客/货率（%） |

窗口底部还有汇总统计：

- **Total income for the last N trips**：N 趟总利润 + 每趟日均利润
- **Average trip length**：平均行程间隔（天）
- **Improvement over last N trips**：利润综合变化百分比

## 记录机制

- **AddValue**：车辆完成一趟运输（`CargoPayment` 析构）时记录利润、日期、占用率、站间距离
- **NewRound**：车辆按时刻表到达第一个目的地时开启新的一趟
- **占用率**：车辆离开站点时采集（复用 jrpm 已有的 `trip_occupancy` 机制）

## 实现说明

- 数据为 **NOSAVE**（仅运行时，不写存档），因此无需 bump 存档版本，旧档完全兼容
- 新增窗口类 `WindowClass::VehicleTripHistory`，窗口 ID 为车辆 ID
- 车辆详情窗口（火车/非火车）标题栏均新增 History 按钮
- 车辆被删除时历史窗口自动关闭

## 涉及文件

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp`（新增）
- `src/vehicle_base.h`（Vehicle 新增 `trip_history` 字段）
- `src/economy.cpp`（CargoPayment 析构记录行程）
- `src/timetable_cmd.cpp`（到达第一站开启新行程）
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h`（History 按钮）
- `src/lang/english.txt`（STR_TRIP_HISTORY_* 字符串）
