<div align="center">

<img src="media/jrpm/icon.png" alt="OpenTTD-JRPM logo" width="168" />

# 🚄 OpenTTD-JRPM

**JGR Patchpack 精品 fork —— 融合 pulsexlb、OpenTTD-modded、cmclient 三大上游特性，并加入 jrpm 自研能力。**

[![文档](https://img.shields.io/badge/📖_文档-在线-00b7ff?style=for-the-badge&logo=readthedocs&logoColor=white)](https://maicarons.github.io/jrpm/)
[![版本](https://img.shields.io/github/v/release/Maicarons/jrpm?style=for-the-badge&color=1f6feb&logo=github&logoColor=white&label=版本)](https://github.com/Maicarons/jrpm/releases)
[![许可](https://img.shields.io/badge/许可-GPL--2.0-4c1?style=for-the-badge&logo=gnu)](COPYING.md)
[![星标](https://img.shields.io/github/stars/Maicarons/jrpm?style=for-the-badge&color=ffd33d&logo=github)](https://github.com/Maicarons/jrpm/stargazers)
[![平台](https://img.shields.io/badge/平台-Windows%20%7C%20Linux%20%7C%20macOS-2ea44f?style=for-the-badge)](#-关于)

[English](README.md) · [简体中文](README_zh.md)

</div>

---

## ✨ 关于

OpenTTD-JRPM（简称 jrpm）是基于 [JGR 的 Patchpack](http://github.com/JGRennison/OpenTTD-patches)
的 fork，额外融合了 **pulsexlb px-patch** 的特性（机车连挂/摘挂 "loco swap"、多格模块化机场），
以及 [OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded) 和
[cmclient](https://github.com/citymania-org/cmclient) 的部分精选功能；此外还加入了 jrpm 自研
特性（多镜像并行内容下载、按共享订单自动分组车辆、整局感知 AI 接口、服务器端多版本客户端兼容）。

> **版本 0.1.0** —— 基于 JGR's Patchpack 0.73.1 + pulsexlb px-patch。

---

## 📦 本版本新增功能

在完整的 JGRPP 特性集（信号增强、定时发车、tracerestrict、模板替换、真实制动、单行道升级、
道口安全等）与 pulsexlb 合并（机车摘挂、模块化多格机场）之上，本版本新增：

<table>
<tr>
<th width="34%" align="center">🚂 来自 OpenTTD-modded</th>
<th width="33%" align="center">🎨 来自 cmclient（改造移植）</th>
<th width="33%" align="center">⚡ jrpm 自研</th>
</tr>
<tr valign="top">
<td>
<ul>
<li><b>行程历史（Trip history）</b>——每辆车都会记住最近 10 次行程（利润、载客率、行程时间）。History 按钮可查看每次行程的利润、变化百分比与汇总统计。</li>
<li><b>可配置飞机滑行速度</b>——<code>vehicle.plane_taxi_speed</code>（1–8，默认 4）独立调节机场滑行限速。</li>
</ul>
</td>
<td>
<ul>
<li><b>物体级建造高亮</b>——车站、轨道、车库、机场、工业实时物体预览。</li>
<li><b>蓝图系统</b>——<code>blueprint_copy</code> / <code>blueprint_build</code>，16 个内存槽位，支持旋转/保存/加载。</li>
<li><b>城镇分区</b>——新增 "Town zones (Tz0–Tz4)" 与 "Town growth tiles" 模式；生长地块持久化到新的 <code>GRWT</code> 存档块。</li>
<li><b>命令记录/重放</b>——<code>cmdrecord</code> + <code>cmdreplay</code>，使用 jrpm 自己的命令序列化。</li>
<li><b>联机 UI 助手</b>——视口书签、公司货运窗口、观战公司、控制台帮手。</li>
</ul>
</td>
<td>
<ul>
<li><b>多镜像并行内容下载</b>——可配置并发数。</li>
<li><b>按共享订单自动分组车辆</b>——窗口按钮或 <code>autogroup</code>。</li>
<li><b>整局感知 AI</b>——<code>AIGlobal</code> API + GlobalAI 示例。</li>
<li><b>多版本客户端兼容</b>——服务器同时接受 jrpm / 原版 jgrpp / pulsexlb 客户端。</li>
</ul>
</td>
</tr>
</table>

---

## 🙏 致谢

jrpm 的存在离不开以下项目的杰出工作，在此向它们致以诚挚的感谢：

| 项目 | 借鉴内容 |
|---|---|
| **[OpenTTD](https://github.com/OpenTTD/OpenTTD)** | 基础游戏本体（GPL v2）。 |
| **[JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches)** | 本 fork 赖以构建的基石 —— 信号、时刻表、tracerestrict 等。 |
| **[pulsexlb/OpenTTD-patches (px-patch)](https://github.com/pulsexlb/OpenTTD-patches)** | 机车连挂/摘挂与模块化多格机场。 |
| **[embeddedt/OpenTTD-modded](https://github.com/embeddedt/OpenTTD-modded)** | 行程历史与可配置飞机滑行速度。 |
| **[citymania-org/cmclient](https://github.com/citymania-org/cmclient)** | 物体高亮、蓝图系统、城镇分区、命令记录重放与联机 UI 助手。 |

---

## 📜 许可与说明

这是对 [OpenTTD](http://www.openttd.org/) 施加了一系列功能与其他修改的独立版本，可与原版游戏
并存安装游玩，而不是一个可加载的 Mod（NewGRF、脚本等）。它主要面向已熟悉原版游戏的玩家；部分
功能与设置面向资深玩家，可能存在一定的学习曲线。

以 **GNU 通用公共许可证 v2.0** 发布 —— 详见 [COPYING.md](COPYING.md)。

<div align="center">

<sub>🚄 由 jrpm 维护者精心打造 · <a href="https://maicarons.github.io/jrpm/">阅读完整文档 →</a></sub>

</div>