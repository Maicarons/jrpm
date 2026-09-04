---
title: Автоматическая группировка транспортных средств
---

## Текущее состояние (выводы исследования)

- Структура данных групп: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- Существующие команды (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (переименование/установка родителя)
  - `CmdAddVehicleGroup` (добавление одного ТС в группу)
  - `CmdAddSharedVehicleGroup` (добавление ТС с **общими приказами определённого ТС** в **существующую** группу)
  - `CmdCreateGroupFromList` (создание группы из списка ТС, поддерживает тип списка `VL_SHARED_ORDERS`; автоматическое именование)
- Помощник автоименования: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, по начальным/конечным городам маршрута `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) — jgrpp уже имеет базу для «именования по маршруту»;
- Механизм регистрации команд: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (макрос одновременно объявляет обработчик и регистрирует `CommandTraits`);
- ТС↔группа: `vehicle_base.h` `Vehicle::group_id`; цепочка общих приказов `FirstShared()/NextShared()`; `OrderList *orders`.

**Вывод**: отсутствует единая команда для «обхода всех ТС компании, автоматического создания групп по общим приказам и помещения ТС в эти группы».

## Реализация данной функции

### Новая команда `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Процесс (`src/group_cmd.cpp`):
1. Обход всех основных ТС указанного типа для данной компании (`Vehicle::IterateTypeFrontOnly(type)`), пропуск не принадлежащих компании / без приказов / уже в нестандартной группе;
2. Подсчёт каждой цепочки общих приказов; при ≥2 ТС с одинаковым списком приказов:
   - Генерация имени группы через `GenerateAutoNameForVehicleGroup(v)` (например, «Город A ↔ Город B»);
   - Создание `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, вложенный вызов `Command<Commands::CreateGroupFromList>::Do(flags, ...)` для создания группы и перемещения всех общих ТС в неё;
   - После выполнения `group_id` ТС в группе уже не является группой по умолчанию, цикл автоматически пропускает их (один список приказов — одна группа);
3. `GroupChangeDeferredUpdateScope` для единой отложенной статистики групп.

### Три точки входа

| Вход | Расположение | Описание |
|---|---|---|
| Кнопка в окне групп | `src/group_gui.cpp` новый `WID_GL_AUTOGROUP_SHARED` (панель инструментов рядом с LIVERY), OnClick отправляет команду | Спрайт повторно использует `SPR_GROUP_CREATE_TRAIN + vtype` |
| Консольная команда | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (без параметров = все четыре типа) | `IConsole::CmdRegister` |
| Скрипт/другое | Любой код через `Command<Commands::AutoGroupSharedOrders>::Post(...)` | Команда проходит сетевую синхронизацию, безопасна на многопользовательском сервере |

### Затронутые файлы

- `src/command_type.h`: новое значение в перечислении `Commands` — `AutoGroupSharedOrders` (**вставка в середине перечисления смещает ID последующих команд, что нормально для форка, но несовместимо между разными версиями**)
- `src/group_cmd.h` / `group_cmd.cpp`: объявление и реализация команды
- `src/widgets/group_widget.h`: новый ID элемента управления
- `src/group_gui.cpp`: кнопка на панели инструментов (NWidget + OnPaint спрайт + OnClick)
- `src/console_cmds.cpp`: консольная команда
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Ключевые моменты проверки

1. Два или более ТС с одинаковым списком приказов → после нажатия кнопки/`autogroup` создаётся группа с «именем маршрута» и все ТС помещаются в неё;
2. Уже сгруппированные ТС не перемещаются повторно; разные списки приказов образуют отдельные группы;
3. Статистика группы (количество/прибыль) обновляется корректно (зависит от `GroupChangeDeferredUpdateScope`).