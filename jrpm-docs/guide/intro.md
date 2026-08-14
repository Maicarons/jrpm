---
title: 项目介绍
---

# OpenTTD-JRPM 项目介绍

## 是什么

**OpenTTD-JRPM（jrpm）** 是基于 [JGR 的 Patchpack](https://github.com/JGRennison/OpenTTD-patches)（jgrpp）的二次开发分支，融合了 [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches) 的「机车换挂」与「模块化机场」特性，并加入了 jrpm 专属功能。当前版本 **jrpm-0.1.0**。

它是一个可独立安装、独立联机的全新版本：jrpm 客户端/服务器使用独立的版本标识（`jrpm-0.1.0`）握手，与原版 jgrpp、pulsexlb 版本完全隔离，避免版本混乱。

## 特性一览

### 来自 pulsexlb（152 个提交，git merge 并入）
- **机车换挂（decouple）**：列车解挂/挂载完整体系——解挂订单、路签转移、耦合长度与车速限制、双车头支持、NewGRF 耦合、耦合寻路（YAPF/NPF）、解挂后两列车独立调度；
- **模块化机场（multitile-airport）**：多格机场系统重构——air 类型体系（`air.h`/`air_type.h`/`newgrf_airtype.*`）、PBS 空中调度（`pbs_air.*`）、YAPF 航空寻路、`station.allow_modify_airports` 改造机场布局、`gui.default_air_type` 默认航空类型。

### jrpm 专属
| 功能 | 说明 | 入口 |
|---|---|---|
| 资源下载：多镜像 + 并行 | 逗号分隔多镜像源、文件级并行下载（并发数可配置）、失败自动切换镜像 | 设置 → `network.content_mirrors` / `network.content_download_parallel` |
| 车辆自动分组 | 按共享订单/调度一键自动建组归组，组名自动取线路名 | 分组窗口按钮 / 控制台 `autogroup` |
| 建造价格 Tooltip | 建造铁轨/道路/地形时鼠标上方实时显示估算成本 | 铁轨/道路/地形工具栏 |
| 整局感知 AI | 保留 NoAI，新增 `AIGlobal` 全局 API + 示例 AI GlobalAI，接入受游戏设置控制 | `game.script.allow_global_ai_access` |
| 服务器多版本兼容 | jrpm 服务器同时接受 jrpm / 原版 jgrpp / pulsexlb 客户端 | 服务器加入检查自动生效 |

### 完整继承
- jgrpp 全部特性（信号增强、scheduled dispatch、tracerestrict、模板替换等）；
- 上游 OpenTTD 全部功能与 NewGRF/脚本生态兼容。

## 版本关系

```
                 jgrpp-0.73.1 (共同祖先)
                 /                 \
 jgrpp 分支 (63 提交)        pulsexlb px-patch (152 提交)
 ├ jgrpp 近期更新             ├ 机车换挂（decouple 分支）
 ├ jrpm 专属功能              └ 模块化机场（multitile-airport 分支）
 └ 版本改名 jrpm-0.1.0
                 \                 /
                  jrpm 分支 (git merge)
```

## 分支与提交

- 分支：`jrpm`（开发主线）
- 关键提交：
  - `d4c45740` jrpm 专属 5 功能
  - `71fe214c` merge pulsexlb（换挂 + 模块化机场）
  - `425e7207` 版本改名 openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` 服务器多版本客户端兼容
  - `4716b925` 并行下载并发数可配置

## 许可

与 OpenTTD 一致：**GPL-2.0**。
