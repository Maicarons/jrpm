---
title: 版本历史
---

# 版本历史

## jrpm-0.1.0（2026-08-14）cmclient 移植完成

在首个 0.1.0 的基础上完成 cmclient 五批功能移植（modded + cmclient 全系）：

### 新增功能（cmclient / modded 移植）

- **modded 第一批**：车辆行程历史（`b89f93f9`）+ 飞机滑行速度可调（`b89f93f9`）；
- **cmclient 第二批**：位置书签 / 货运明细 / 观战（`089480b3`）；
- **cmclient 零批**：`cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` 控制台命令（`1fd94d12`）；
- **cmclient 第三批 ① 高亮**：对象级建造预览（车站/轨道/仓库/机场），挂接 viewport 绘制管线与建造工具（6 提交，`d97aa38a` → `1957bf45`）；
- **cmclient 第三批 ③ 蓝图**：圈选复制/旋转/16 槽位/重建（`8e08ca6b`）；
- **cmclient 第四批 ④ 城镇分区**：Tz 分区 + growth_tiles 着色 + GRWT 存档块（`9e3f95a2`）；
- **cmclient 第五批 ⑤ 命令重放**：`cmdrecord` / `cmdreplay`（`f113acce28`）；
- **控制台参数约定修复**：全部 jrpm 命令改用 argv[1] 起始参数（argv[0] 为命令名）。

### 架构结论

- **② 命令对象层未移植**：高亮/蓝图/重放分别用 `CMD_ERROR`、命令闭包、jrpm 原生命令序列化绕过，无需 cmclient 的 2251 行生成代码。

### 存档兼容

- growth_tiles 数据存于独立 `GRWT` chunk，旧存档（不含该块）加载完全兼容；
- 命令记录文件（`.jrcm`）为 jrpm 私有格式，跨版本不保证兼容。

## jrpm-0.1.0（2026-08-14）

首个 jrpm 版本，基于 jgrpp 0.73.1 + pulsexlb px-patch 合并，并移植 modded / cmclient 特性。

### 变更内容

- **合并 pulsexlb px-patch（152 提交）**：
  - 机车换挂（decouple）：解挂/挂载订单、路签转移、耦合长度/车速限制、双车头、NewGRF 耦合、耦合寻路、解挂后独立调度；
  - 模块化机场（multitile-airport）：air 类型体系、PBS 空中调度、YAPF 航空寻路、机场布局改造（`allow_modify_airports`）；
  - 存档版本新增 `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`。
- **版本改名**：`openttd-jrpm` / `jrpm-0.1.0`（可执行文件名与修订串）。
- **服务器多版本兼容**：jrpm 服务器接受 jrpm / 原版 jgrpp / pulsexlb 客户端。
- **jrpm 专属功能**：
  - 资源下载多镜像 + 文件级并行（并发数可配置）；
  - 车辆按共享订单自动分组（窗口按钮 + `autogroup` 命令）；
  - 整局感知 AI（`AIGlobal` API + GlobalAI 示例，设置开关控制）；
  - 建造价格 Tooltip（已按需求移除，提交 `96ebfb75`）。
- **modded 移植（第一批）**：
  - 车辆行程历史（最近 10 趟利润/占用率/时长，车辆详情窗口 History 按钮）；
  - 飞机滑行速度可调（`vehicle.plane_taxi_speed`，XSLF 特性门控，旧档兼容）。
- **cmclient 借鉴（第二批）**：
  - 位置书签（`savelocation` / `gotolocation`，9 槽位）；
  - 公司货运明细窗口（`company_cargo`）；
  - 观战辅助（`watch <company_id>`）。

### 存档兼容

- 继承 jgrpp 惯例：可加载 trunk 存档（到最近并入的版本）；
- jrpm 存档（含多格机场/解挂订单数据）**不保证**与旧 jgrpp 存档互读；
- 存档版本号 `SAVEGAME_VERSION` 与 pulsexlb 一致（`SLV_CUSTOM_SUBSIDY_DURATION`）；
- 新增特性均用 XSLF 扩展特性门控或 NOSAVE，不破坏旧档。

## 上游版本

- **jgrpp 0.73.1**：本项目基座（OpenTTD 16.0 内核 + JGR 全部特性）。
- **pulsexlb px-patch 2608.3**：机车换挂与模块化机场的来源。
- **embeddedt/OpenTTD-modded（0.59.1 时代）**：行程历史与滑行速度来源。
- **citymania-org/cmclient（vanilla 15.3）**：位置书签/货运明细/观战借鉴来源。

## 路线图

- [x] 真机构建验证与首编错误修复
- [x] 行程历史 + 滑行速度（modded 第一批）
- [x] 位置书签 + 货运明细 + 观战（cmclient 第二批）
- [ ] 高亮系统 + 蓝图系统（cmclient 第三批，见[路线图](../features/highlight-blueprint-plan)）
- [ ] HTTP 传输层线程池 + Range 分块下载（单大文件加速）
- [ ] 地图发送压缩对比优化
- [ ] 每货物收入统计（需存档格式扩展）
- [ ] （远期）Rust 服务器运行时集成（参考 Openttd-Cluster）
