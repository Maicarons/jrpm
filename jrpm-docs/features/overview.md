---
title: 功能总览
---

# 功能总览

jrpm = jgrpp 全特性 + pulsexlb（机车换挂 + 模块化机场）+ modded（行程历史 + 滑行速度）+ cmclient 借鉴（联机 UI 增强）+ jrpm 专属功能。

## jrpm 专属功能

| # | 功能 | 文档 | 状态 |
|---|---|---|---|
| 1 | 资源下载：多镜像 + 并行 | [资源下载](./01-resource-download) | ✅ 已实现（并发数可配置） |
| 2 | 服务器上限研究 | [服务器上限研究](./02-server-caps) | 📖 调研结论（结构性限制） |
| 3 | 车辆自动分组 | [车辆自动分组](./03-vehicle-autogroup) | ✅ 已实现 |
| 4 | 整局感知 AI | [整局感知 AI](./05-global-ai) | ✅ 已实现 |

> 说明：原 F4「建造价格 Tooltip」已按需求移除（提交 `96ebfb75`）。

## modded 移植（第一批）

| 功能 | 文档 | 状态 |
|---|---|---|
| 车辆行程历史 | [行程历史](./triphistory) | ✅ 已实现 |
| 飞机滑行速度可调 | [滑行速度](./plane-taxi-speed) | ✅ 已实现 |

## cmclient 借鉴（第二批）

| 功能 | 文档 | 状态 |
|---|---|---|
| 位置书签 / 货运明细 / 观战 | [联机 UI 增强](./ui-enhancements) | ✅ 已实现 |
| 高亮系统 + 蓝图系统 | [第三批规划](./highlight-blueprint-plan) | 📋 规划中（见路线图） |

## 合并特性（来自 pulsexlb）

| 功能 | 文档 | 说明 |
|---|---|---|
| 机车换挂（decouple） | [机车换挂](./decouple) | 列车解挂/挂载完整体系 |
| 模块化机场（multitile-airport） | [模块化机场](./multitile-airport) | 多格机场系统重构 |

## 完整继承

- **jgrpp 全部特性**：信号增强（多重信号/程序化信号/插槽与计数器）、scheduled dispatch、tracerestrict、模板替换、机车/车厢分离购买窗口、现实制动、One-way 道路增强、平交口安全等；
- **OpenTTD 16.0 内核**：NewGRF/脚本/存档生态完整兼容。

## 快速找到设置

| 功能 | 设置 |
|---|---|
| 并行下载并发数 | `network.content_download_parallel`（1-8） |
| 下载镜像列表 | `network.content_mirrors` |
| 内容服务器 | `network.content_server` |
| 机场布局改造 | `station.allow_modify_airports` |
| 默认航空类型 | `gui.default_air_type` |
| AI 整局感知开关 | `game.script.allow_global_ai_access` |
| 飞机滑行速度 | `vehicle.plane_taxi_speed`（1-8，默认 4） |

## 快速找到控制台命令

| 功能 | 命令 |
|---|---|
| 车辆自动分组 | `autogroup train\|road\|ship\|aircraft` |
| 添加整局感知 AI | `start_ai GlobalAI` |
| 位置书签 | `savelocation <1-9>` / `gotolocation <1-9>` |
| 公司货运明细 | `company_cargo <company_id>` |
| 观战公司 | `watch <company_id>` |

