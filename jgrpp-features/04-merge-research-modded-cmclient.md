# OpenTTD-modded 与 CityMania cmclient 合并研究报告

- 调研日期：2026-08-14
- 目标仓库：
  1. https://github.com/embeddedt/OpenTTD-modded（stock_jgr 分支）
  2. https://github.com/citymania-org/cmclient（master 分支）
- 现状仓库：G:\GitHub\OpenTTD-patches（jrpm 分支，基于 JGR patchpack 0.73.1 + pulsexlb 152 提交）

---

## 一、核心结论（先看这里）

| 仓库 | 版本基线 | 与 jrpm 的关系 | 合并方式 | 难度 |
|---|---|---|---|---|
| **OpenTTD-modded** | **jgrpp-0.59.1（2024-05）** | 同宗（同为 JGR patchpack 系），但比 jrpm 旧 14 个版本；**作者已停止维护**（README 2026 声明） | **逐补丁摘取**（cherry-pick），不能全量 merge | 🟡 中 |
| **CityMania cmclient** | **OpenTTD vanilla 15.3（2026）** | **不同分支体系**（vanilla vs jgrpp），无共同维护线 | **架构移植**（vanilla API → jgrpp API），需大量改写 | 🔴 高 |

**两条重要事实**：
1. **modded 已停维护**，其补丁多为 2024 年旧实现，部分已被 jgrpp 0.73.1 上游吸收或改进；
2. **cmclient 是纯 vanilla 客户端**（有 `src/saveload/`、无 jgrpp 的 `src/sl/` 新存档体系、无 tracerestrict、无 INI 设置系统），它的 CityMania 特性是**为 CM 服务器联机设计的增强**（蓝图/命令日志/导出/分区），与我们的 jrpm 是两套不同的 API 体系。

---

## 二、OpenTTD-modded 详细分析

### 2.1 特性清单（README 明确列出 7 项）

| # | 补丁 | 原始出处 | jrpm 现状 |
|---|---|---|---|
| 1 | **巨型机场（Huge airports）** | tt-forums t=56933 | ❌ 无。但 jrpm 有 pulsexlb 的模块化机场（多格自由拼装），**功能目标重叠且更先进** |
| 2 | **工业产量图（Industry production graph）** | OpenTTD PR#7575 | ⚠️ graph_gui.cpp 已有部分痕迹（差异 2300 行需甄别，可能已被 jgrpp 吸收） |
| 3 | **飞机滑行速度可调** | 自研 | ❌ 无（jgrpp 已有飞机速度/航程相关，但无滑行速度设置） |
| 4 | **车辆行程历史（Trip history）** | tt-forums t=33411 | ❌ 无。triphistory.h/cpp/gui 共 745 行，**最独特、最有价值** |
| 5 | **道路车辆寻路微调** | 自研 | ⚠️ jgrpp 0.73.1 道路寻路已有大量演进，需甄别是否被吸收 |
| 6 | **基础设施共享下可扩建他人轨道** | 自研 | ⚠️ jgrpp 有 infrastructure sharing（jgrpp 独有），需核对交互 |
| 7 | **默认鼠标滚动模式** | 上游讨论 #9150 | ✅ 大概率已被上游吸收（jgrpp 0.73.1 距今较新） |

### 2.2 版本关系

- modded 基线 `jgrpp-0.59.1`（.ottdrev-vc 确认 `jgrpp-0.59.1 20240519`）
- jrpm 基线 `jgrpp-0.73.1`（相差 14 个版本，跨越 2024-05 → 2025+ 大量上游重构）
- **全量 git merge 会产生数千行冲突**（0.59.1 的旧 API 与 0.73.1 的新架构），不可行

### 2.3 推荐合并策略

**逐补丁 cherry-pick，优先级排序：**

1. 🥇 **车辆行程历史（triphistory）**——最独特、独立性强（3 文件 745 行），不与现有系统冲突，玩家价值高（查看每辆车的历史行程路线）
2. 🥈 **飞机滑行速度设置**——小而独立（1 个设置项 + 少量逻辑）
3. 🥉 **工业产量图**——先核对 jrpm 现状（graph_gui 2300 行差异需甄别，可能部分已在）
4. ⚠️ **巨型机场**——**建议放弃**：jrpm 已通过 pulsexlb 合入更先进的模块化机场（多格自由拼装），huge airport 是旧式固定大机场，功能重叠且不兼容
5. ⚠️ 道路寻路/基础设施共享/鼠标模式——逐一甄别是否已被 jgrpp 0.73.1 吸收，吸收则跳过

---

## 三、CityMania cmclient 详细分析

### 3.1 特性清单（40+ 模块 / 1.67 万行）

| 类别 | 模块 | 说明 |
|---|---|---|
| **蓝图系统** | cm_blueprint | 复制/粘贴建造布局（CM 服务器核心玩法） |
| **命令系统** | cm_commands / cm_command_log / cm_commands_gui | 自定义命令 + 命令历史日志 |
| **导出** | cm_export | 导出地图/公司数据 |
| **区域分区** | cm_zoning / cm_zoning_cmd / cm_zoning_gui | 城镇分区规划（growth_tiles 存档字段） |
| **高亮** | cm_highlight | 地图高亮（连接性/服务范围） |
| **联机增强** | cm_client_list_gui / cm_identity_gui / cm_watch_gui | 客户端列表、身份密钥、观战 |
| **UI 增强** | cm_cargo_table_gui / cm_town_gui / cm_station_gui / cm_rail_gui / cm_minimap / cm_overlays / cm_tooltips / cm_hotkeys | 各类窗口增强 |
| **服务器专属** | cm_survey / cm_newgrf_revisions / cm_locations / cm_game | CM 服务器协议 |
| **底层** | cm_saveload / cm_settings / cm_main / cm_base64 / cm_bitstream / cm_colour | 支持层 |

### 3.2 关键架构差异（合并障碍）

| 维度 | cmclient（vanilla 15.3） | jrpm（jgrpp 0.73.1） |
|---|---|---|
| 存档体系 | `src/saveload/`（旧式 `SlObject`） | `src/sl/`（新式 NSL/SlObject 重构） |
| 命令系统 | vanilla 旧式 | jgrpp `Commands` 枚举 + `DEF_CMD_TUPLE` |
| 设置系统 | 旧式 settings.cpp | INI 驱动（table/settings/*.ini → settingsgen） |
| 窗口系统 | vanilla | jgrpp NWidget + 窗口类重构 |
| 联机协议 | vanilla | jgrpp 扩展协议 |
| 版本 | OpenTTD 15.3 | jgrpp 0.73.1（基于 OpenTTD ~14.x 时代）+ pulsexlb |

### 3.3 推荐合并策略

**cmclient 不能整体 merge，只能选择性移植，且需要 API 改写：**

1. 🥇 **蓝图系统（cm_blueprint）**——最独特、玩家价值最高，但依赖 vanilla 命令/存档 API，移植工作量最大（估 3-5 天）
2. 🥈 **命令日志（cm_command_log）**——相对独立，改造命令回调即可
3. 🥉 **城镇分区（cm_zoning + cm_saveload 的 growth_tiles 字段）**——需要接入 jgrpp sl/ 存档体系
4. ⚠️ **联机增强（client_list/identity/watch）**——与 CM 服务器强绑定，除非玩 CM 服务器否则价值低
5. ⚠️ **服务器专属模块（survey/export/locations）**——**不建议移植**（纯 CM 服务器配套）

**重要提示**：cmclient 的大量特性（zoning growth_tiles 存档字段、命令日志）需要**同时改存档格式**，会与 jrpm 当前的存档版本体系（SLV 367/368 机制）叠加，需谨慎设计存档兼容层。

---

## 四、综合建议

### 推荐执行顺序（低风险 → 高价值）

**第一阶段（推荐现在做）：modded 的 2-3 个独立补丁**
1. 车辆行程历史 triphistory（最独立、价值高）
2. 飞机滑行速度设置（小而独立）

**第二阶段（可选）：cmclient 的蓝图/命令日志**
- 明确预期：这是架构移植而非 merge，需逐文件改写 vanilla API → jgrpp API
- 建议先做一个（如命令日志）验证流程可行性

**第三阶段（暂缓）：**
- modded 巨型机场（被模块化机场取代）
- cmclient 服务器专属模块（依赖 CM 服务器）

### 联机影响

- triphistory/滑行速度/蓝图/命令日志均为**客户端增强**，存档字段新增需 SLV bump（jrpm 惯例，已具备）
- 涉及存档的移植（zoning）会与联机存档同步相关，需测试多人存档往返

---

## 五、参考命令（已就绪）

```bash
# 两个库已克隆到：
/g/GitHub/_refs/ottd-modded   (jgrpp-0.59.1, stock_jgr)
/g/GitHub/_refs/cmclient      (vanilla 15.3, master)
```

开始移植前，建议先从 modded cherry-pick triphistory 补丁开始：
```bash
cd /g/GitHub/OpenTTD-patches
git remote add modded /g/GitHub/_refs/ottd-modded
git fetch modded
# 定位 triphistory 相关提交后逐个 cherry-pick
```
