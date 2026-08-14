---
title: 建造价格 Tooltip
---

## 现状（调研结论）

- 建造工具栏：`src/rail_gui.cpp` `BuildRailToolbarWindow`（`WindowClass::BuildToolbar` + `TRANSPORT_RAIL`）、`road_gui.cpp` `BuildRoadToolbarWindow`（`TRANSPORT_ROAD`）、`terraform_gui.cpp` `TerraformToolbarWindow`（`WindowClass::ScenarioGenerateLandscape`）；
- 选中工具记录：三个工具栏均有 `last_user_action` / `last_started_action`（WidgetID）；活跃工具判断用 `_thd.GetCallbackWnd() == this`；
- 鼠标下 tile：`GetTileBelowCursor()`（`src/viewport_func.h`，返回 `INVALID_TILE` 表示不在视口）；
- 现有文本提示机制：`src/texteff.hpp` `AddTextEffect(msg, x, y, duration, mode, param...)`（屏幕坐标浮动文本，`TE_STATIC` 不上升）、`RemoveTextEffect`；成本动画 `ShowCostOrIncomeAnimation` 同机制（command.cpp:482）；
- 成本估算：命令系统支持 `DC_QUERY_COST` 模式（不执行、只算钱）；
- **现状无任何鼠标跟随价格提示**（原版只在状态栏等处间接显示）。

## 本功能实现

### 新模块 `src/construction_cost_tip.h/.cpp`

- `ConstructionCostTipContext`：当前工具栏（window_class/window_number/selected_tool/railtype/roadtype）；
- `UpdateConstructionCostTip(ctx, tile)`：
  1. 节流：同一 tile + 同一工具不重复更新；
  2. 估算：`EstimateBuildCost()` 用 `DC_QUERY_COST` 对悬停 tile 询价——
     - 铁轨：4 个单轨按钮（NS/X/EW/Y 按游戏自身 `HT_DIR_*`→`Track` 映射）+ 自动铁轨（`TRACK_X` 估算）+ 道路单块（ROAD_X/ROAD_Y）；
     - 地形：`WID_TT_RAISE/LOWER_LAND` → `TerraformLand`（用该 tile 当前坡度）；
  3. 成功 → 用 `AddTextEffect` 在鼠标所在 tile 的屏幕位置显示 `STR_CONSTRUCTION_COST_TOOLTIP`（`Estimated cost: {CURRENCY_LONG}`）；失败（不可建/不支持的工具）→ 隐藏；
  4. 「显示者」追踪：非活跃工具栏发送 INVALID_TILE 只隐藏自己显示的内容，避免多工具栏竞争残留；
- `HideConstructionCostTip()`：移除特效。

### 三个工具栏接入

- 各工具栏新增 `OnMouseLoop()`：`_thd.GetCallbackWnd() == this` 时 `UpdateConstructionCostTip(ctx, GetTileBelowCursor())`，否则传 `INVALID_TILE`；
- 各工具栏 `Close()` 开头调用 `HideConstructionCostTip()`（防止窗口关闭后残留）。

### 涉及文件

- 新增 `src/construction_cost_tip.h/.cpp`（`src/CMakeLists.txt` 已注册）
- `src/rail_gui.cpp`、`src/road_gui.cpp`、`src/terraform_gui.cpp`（OnMouseLoop + Close 钩子）
- `src/lang/english.txt`：`STR_CONSTRUCTION_COST_TOOLTIP`

## 验证要点

1. 打开铁轨工具栏选「NS 轨道」→ 鼠标移到可建铁轨的格子上方，出现「Estimated cost: £x」且跟随鼠标所在格；
2. 移到水中/已有轨道等不可建处 → 提示消失；
3. 切换到道路/地形工具栏同样生效；关闭工具栏提示消失；
4. 单 tile 询价频率由节流控制（仅 tile/工具变化时查询），性能可接受。
