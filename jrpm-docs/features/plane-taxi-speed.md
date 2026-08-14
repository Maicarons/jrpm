---
title: 飞机滑行速度可调（Plane Taxi Speed）
---

# 飞机滑行速度可调

源自 **embeddedt/OpenTTD-modded**（第一批移植）。

## 功能

新增游戏设置 **`vehicle.plane_taxi_speed`（飞机滑行速度）**，可独立调节飞机在机场地面滑行时的限速：

- 范围 **1–8**，默认 **4**（= 原版 50 单位的滑行限速）
- 值越大滑行越快（1 → 12.5，8 → 100）
- 与其他飞机速度设置（`plane_speed` 全局倍率）**独立生效**，互不干扰

## 设置位置

游戏内 **设置 → 专家设置 → 车辆（Vehicles）**，或在配置文件 `openttd.cfg`：

```ini
[vehicle]
plane_taxi_speed = 4
```

## 实现说明

- 设置带 `SettingFlag::NoNetwork`（服务器权威，联机时由房主决定）
- 用 **XSLF 扩展特性门控**（`XSLFI_PLANE_TAXI_SPEED`，版本 1）：旧存档加载时该特性不存在 → 设置保持默认值 4，**不破坏任何旧档**
- 滑行限速在飞机状态为 `AS_RUNNING`（在地面滑行）时应用，取 `min(机型限速, taxi限速)`

## 涉及文件

- `src/aircraft_cmd.cpp`（新增 `SPEED_LIMIT_TAXI` 常量 + 滑行限速逻辑）
- `src/table/settings/game_settings.ini`（设置定义）
- `src/settings_type.h`（`VehicleSettings::plane_taxi_speed`）
- `src/sl/extended_ver_sl.h/.cpp`（XSLF 特性注册）
- `src/lang/english.txt`（STR_CONFIG_SETTING_PLANE_TAXI_SPEED*）
