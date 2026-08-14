---
title: 版本历史
---

# 版本历史

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
