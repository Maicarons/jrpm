---
title: Добавление игровой команды
---

# Добавление игровой команды

На примере реализованной в этом проекте команды `Commands::AutoGroupSharedOrders` (автоматическая группировка ТС) показаны полные шаги добавления новой команды в jrpm.

## 1. Объявление в перечислении команд

`src/command_type.h`, `enum class Commands` вставка в середину (смещает ID последующих):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. Объявление обработчика команды и регистрация

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` одновременно объявляет функцию-обработчик и регистрирует `CommandTraits`):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: без параметра тайла (с тайлом используйте `DEF_CMD_TUPLE`);
- `CmdDataT<...>`: список типов параметров, обработчик распаковывает по порядку.

## 3. Реализация обработчика

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... бизнес-логика ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

Ключевые моменты:
- Возврат `CMD_ERROR` означает ошибку (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` управляет разделением «реального выполнения» и «тестового режима»;
- Вложенные команды через `Command<Commands::X>::Do(flags, ...)`;
- Вызов из GUI/сети через `Command<Commands::X>::Post(err_string, args...)` (автоматическая сетевая синхронизация).

## 4. Добавление точки входа

### Кнопка в окне (опционально)

1. `src/widgets/<system>_widget.h` добавить `WID_XXX` в перечисление;
2. Добавить кнопку в NWidget раскладку окна;
3. В `OnClick` добавить `case`:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### Консольная команда (опционально)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// В IConsoleStdLibRegister():
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### Строки (опционально)

`src/lang/english.txt` добавить (например, `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. Контрольный список

- [ ] Перечисление в `command_type.h`
- [ ] `DEF_CMD_TUPLE*` в `<system>_cmd.h`
- [ ] Обработчик в `<system>_cmd.cpp`
- [ ] (Опционально) Элементы управления / консоль / строки
- [ ] Сеть и сохранение адаптируются автоматически (`CommandTraits` генерируется автоматически)