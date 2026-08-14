---
title: 新增游戏命令
---

# 新增游戏命令

以本项目实现的 `Commands::AutoGroupSharedOrders`（车辆自动分组）为模板，介绍在 jrpm 新增一条命令的完整步骤。

## 1. 在命令枚举中声明

`src/command_type.h`，`enum class Commands` 中段插入（会顺移后续 ID）：

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. 声明命令处理器与注册

`src/group_cmd.h`（`DEF_CMD_TUPLE_NT` 会同时声明处理器函数并注册 `CommandTraits`）：

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`：无 tile 参数（带 tile 用 `DEF_CMD_TUPLE`）；
- `CmdDataT<...>`：参数类型列表，处理器按顺序解包。

## 3. 实现处理器

`src/group_cmd.cpp`：

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... 业务逻辑 ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

要点：
- 返回 `CMD_ERROR` 表示失败（`CommandCost(INVALID_STRING_ID)`）；
- `flags.Test(DoCommandFlag::Execute)` 门控「真正执行」与「测试模式」；
- 嵌套命令用 `Command<Commands::X>::Do(flags, ...)`；
- GUI/网络调用用 `Command<Commands::X>::Post(err_string, args...)`（自动走网络同步）。

## 4. 添加入口

### 窗口按钮（可选）

1. `src/widgets/<system>_widget.h` 枚举加 `WID_XXX`；
2. 窗口 NWidget 布局加按钮；
3. `OnClick` 加 `case`：
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### 控制台命令（可选）

`src/console_cmds.cpp`：

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// IConsoleStdLibRegister() 中：
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### 字符串（可选）

`src/lang/english.txt` 添加（如 `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`）。

## 5. 完成清单

- [ ] `command_type.h` 枚举
- [ ] `<system>_cmd.h` 的 `DEF_CMD_TUPLE*`
- [ ] `<system>_cmd.cpp` 处理器
- [ ] （可选）控件/控制台/字符串
- [ ] 网络与存档自动适配（`CommandTraits` 自动生成）
