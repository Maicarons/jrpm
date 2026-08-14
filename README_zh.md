# OpenTTD-JRPM

OpenTTD-JRPM（简称 jrpm）是基于 [JGR 的 Patchpack](http://github.com/JGRennison/OpenTTD-patches)
的 fork，额外融合了 pulsexlb px-patch 的特性（机车连挂/摘挂 "loco swap"、多格模块化机场），
以及 [OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded) 和
[cmclient](https://github.com/citymania-org/cmclient) 的部分精选功能；此外还加入了 jrpm 自研
特性（多镜像并行内容下载、按共享订单自动分组车辆、整局感知 AI 接口、服务器端多版本客户端兼容）。

**版本 0.1.0** —— 基于 JGR's Patchpack 0.73.1 + pulsexlb px-patch。

📖 **在线文档：** <https://maicarons.github.io/jrpm/>

[English](README.md) | [简体中文](README_zh.md)

## 本版本新增功能

在完整的 JGRPP 特性集（信号增强、定时发车、tracerestrict、模板替换、真实制动、单行道升级、
道口安全等）与 pulsexlb 合并（机车摘挂、模块化多格机场）之上，本版本新增：

**来自 OpenTTD-modded**
- **行程历史（Trip history）**——每辆车都会记住最近 10 次行程（利润、载客率、行程时间）；
  车辆详情窗口的 History 按钮可查看每次行程的利润、变化百分比与汇总统计。
- **可配置飞机滑行速度**——`vehicle.plane_taxi_speed`（1-8，默认 4）可独立调节机场滑行限速。

**来自 cmclient（改造移植）**
- **物体级建造高亮**——车站、轨道、车库、机场、工业在放置/拖拽建造工具时显示实时物体预览
  （不再只是简单的矩形）。
- **蓝图系统**——框选一片区域后执行 `blueprint_copy`，再到别处执行 `blueprint_build` 即可重建
  铁路、车库、隧道、桥梁、车站与信号（16 个内存槽位，支持旋转/保存/加载）。
- **城镇分区**——分区工具条新增 "Town zones (Tz0-Tz4)" 与 "Town growth tiles" 模式；生长地块
  （本月/上月建造或拆除的房屋）持久化到新的 `GRWT` 存档块，旧存档无需改动即可加载。
- **命令记录/重放**——`cmdrecord start [file]` / `cmdrecord stop` 记录所有已执行的命令；
  `cmdreplay <file>` 重放（使用 jrpm 自己的命令序列化，无需额外的命令对象层）。
- **联机 UI 助手**——视口位置书签（`savelocation`/`gotolocation`）、公司货运详情窗口
  （`company_cargo`）、观战公司（`watch`），以及控制台命令 `cmgamespeed`、`cmgamestats`、
  `cmexport`、`cmtreemap`。

**jrpm 自研**
- 多镜像并行内容下载（并发数可配置）。
- 按共享订单自动分组车辆（窗口按钮 + `autogroup`）。
- 整局感知 AI（`AIGlobal` API + GlobalAI 示例）。
- 服务器兼容 jrpm / 原版 jgrpp / pulsexlb 客户端。

## 致谢

jrpm 的存在离不开以下项目的杰出工作，在此向它们致以诚挚的感谢：

- **[OpenTTD](https://github.com/OpenTTD/OpenTTD)**——基础游戏本体（GPL v2）。
- **[JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches)**——本 fork 赖以构建的基石
  （信号/时刻表/tracerestrict 等增强）。
- **[pulsexlb/OpenTTD-patches (px-patch)](https://github.com/pulsexlb/OpenTTD-patches)**——机车
  连挂/摘挂与模块化多格机场。
- **[embeddedt/OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded)**——行程历史与可配置
  飞机滑行速度。
- **[citymania-org/cmclient](https://github.com/citymania-org/cmclient)**——物体高亮、蓝图系统、
  城镇分区/生长地块、命令记录重放与联机 UI 助手。

---

这是对 [OpenTTD](http://www.openttd.org/) 施加了一系列功能与其他修改的独立版本，可与原版游戏
并存安装游玩，而不是一个可加载的 Mod（NewGRF、脚本等）。它主要面向已熟悉原版游戏的玩家；部分
功能与设置面向资深玩家，可能存在一定的学习曲线。
