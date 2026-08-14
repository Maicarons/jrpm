---
title: 版本历史
---

# 版本历史

## jrpm-0.1.0（2026-08-14）

首个 jrpm 版本，基于 jgrpp 0.73.1 + pulsexlb px-patch 合并。

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
  - 建造价格 Tooltip（铁轨/道路/地形）；
  - 整局感知 AI（`AIGlobal` API + GlobalAI 示例，设置开关控制）。

### 存档兼容

- 继承 jgrpp 惯例：可加载 trunk 存档（到最近并入的版本）；
- jrpm 存档（含多格机场/解挂订单数据）**不保证**与旧 jgrpp 存档互读；
- 存档版本号 `SAVEGAME_VERSION` 与 pulsexlb 一致（`SLV_CUSTOM_SUBSIDY_DURATION`）。

## 上游版本

- **jgrpp 0.73.1**：本项目基座（OpenTTD 16.0 内核 + JGR 全部特性）。
- **pulsexlb px-patch 2608.3**：机车换挂与模块化机场的来源。

## 路线图

- [ ] 真机构建验证与首编错误修复
- [ ] HTTP 传输层线程池 + Range 分块下载（单大文件加速）
- [ ] 地图发送压缩对比优化
- [ ] （远期）Rust 服务器运行时集成（参考 Openttd-Cluster）
