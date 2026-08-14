---
title: 第三批规划：高亮系统 + 蓝图系统
---

# 第三批规划：高亮系统 + 蓝图系统

来自 cmclient 的两个高价值功能已深入研究，本页记录移植方案与工作量评估。

## 一、对象级高亮系统（cm_highlight，2888 行）

**功能**：建造工具激活时，实时高亮将要建造的对象——铁轨整段、车站全区、道路/停车场、信号、桥梁、隧道、码头、机场、工业等 **15 种对象**的精确预览。

**移植依赖（vanilla API → jrpm API）**：

| cmclient 依赖 | jrpm 对应 |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection`（viewport.cpp 内部） | 存在但签名不同（jgrpp 重构过），需逐一对齐 |
| `TileZoning`（城镇分区着色） | jrpm 无 → 需新建 |
| `_fn_mod` 等全局状态 | cmclient 专属 → 需重设计 |
| `ObjectTileHighlight::make_rail/road_stop/...` 各对象构造器 | 依赖 NewGRF 车站/道路 API，jrpm 与 vanilla 有差异 |

**工作量**：约 2–3 轮（每轮 4–6 小时），核心难点在 viewport 绘制管线的对齐。

## 二、蓝图系统（cm_blueprint，660 行）

**功能**：圈选区域 → 记录建造命令序列（铁轨/车站/隧道/桥梁/信号）→ 16 槽位存储 → 一键重放构建 + 旋转。

**移植依赖**：

| cmclient 依赖 | jrpm 对应 |
|---|---|
| `cm_commands.hpp` **命令对象层**（as_company / with_callback / set_auto / no_estimate + 100+ 生成命令类，2251 行） | jrpm 无此抽象 → **必须先移植/重写** |
| `cm_station_gui.hpp` 的 `_station_gui` 状态 | jrpm 车站 GUI 状态结构不同 |
| `sp<Blueprint>` 智能指针 | jrpm 可用 `std::shared_ptr` |
| `BlueprintCopyArea` 的 tile 遍历 | 依赖 `TileIndexDiffC` 等通用 API（jrpm 有） |

**工作量**：约 2–3 轮（含命令对象层）。

## 三、推荐实施顺序

```
第 1 步：命令对象层（cm_command_type 设计，用 jrpm 的 Command<T>::Do/Post 重新实现）
         —— 蓝图与命令回放（load_commands）的共同地基
第 2 步：对象级高亮（viewport 管线对齐，逐个对象移植）
第 3 步：蓝图（复制/旋转/槽位/重放）
第 4 步（可选）：命令回放（lzma 整局回放）+ 城镇分区（growth_tiles，需存档扩展）
```

## 前提说明

这三个功能合计约 **5000+ 行**、跨越命令系统/存档/视口绘制三层核心，且依赖一批 jrpm 没有的中间层（命令对象、事件总线、位流序列化）。作为独立专项推进更稳妥——建议分批执行，每批编译 + 联机验证后提交，避免一次性大改导致难以定位回归。
