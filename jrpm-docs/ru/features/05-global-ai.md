---
title: API глобального AI
---

## Текущее состояние (выводы исследования)

- Фреймворк NoAI: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) — оригинальная система AI **сохранена без изменений**, данная функция её не затрагивает;
- Система API скриптов: `src/script/api/script_*.hpp/.cpp` (60+ классов), привязка Squirrel **автоматически генерируется** инструментами сборки:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` автоматически обнаруживает новые классы API (генерирует `ai_*.sq.hpp` / `gs_*.sq.hpp`), **новые классы не требуют ручной регистрации**;
  - `.cpp` необходимо добавить в список исходников CMake; комментарий класса `@api ai game` управляет доступностью для AI/GS;
- Данные компании: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- Режим «божества» GS: `ScriptCompanyMode::IsDeity()`;
- Текущие ограничения: AI по умолчанию может надёжно получать данные только своей компании, нет агрегированного API для всей игры, нет переключателя доступа.

**Вывод**: добавление нового класса API `ScriptGlobal` (автоматическая регистрация) + переключатель `game.script.allow_global_ai_access` позволяет реализовать «сохранение NoAI, добавление глобального AI с контролем доступа».

## Реализация данной функции

### 1. API `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

Статические методы, доступные AI и GS (`@api ai game`):

| Метод | Возвращает | Источник данных |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| настройка` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` по owner |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- ID компании использует `ScriptCompany::CompanyID` (COMPANY_SELF разрешается в текущую компанию);
- **Контроль доступа**: все методы проверяют `IsGlobalAccessAllowed()` в первой строке, при невыполнении возвращают -1/nullopt/false (GS всегда доступно, AI управляется настройкой);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` перечисляет все компании.

### 2. Переключатель доступа (настройка)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, по умолчанию false, `SC_EXPERT`);
- Поле структуры `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- Строка `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. Пример AI: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- Демонстрация: чтение финансов/ТС/станций/рейтингов и информации о карте всех компаний с выводом в лог; пояснение контроля доступа (подсказка, если переключатель не включён);
- Этот AI является эталонной реализацией «восприятия всей игры + контроля доступа», логика управления AI может быть расширена в его `Start()`.

### Затронутые файлы

- Новые `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (добавить .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- Новые `bin/ai/GlobalAI/info.nut`, `main.nut`

## Ключевые моменты проверки

1. Новый AI виден в окне конфигурации AI (сканирование `ai/GlobalAI`);
2. Без включения `game.script.allow_global_ai_access` AI в логе сообщает об отсутствии прав; после включения выводит полные данные всех компаний;
3. GameScript может использовать `GSGlobal` без переключателя;
4. На этапе сборки автоматически генерируются `ai_global.sq.hpp` / `gs_global.sq.hpp` без ошибок (зависит от `file(GLOB)`, требуется переконфигурация CMake).

## Направления расширения

- Добавление экономических/кредитных/инфраструктурных (`GetCompanyInfrastructure`) данных;
- Добавление подписки на события (банкротство/поглощение/новая компания);
- Превращение логики принятия решений AI в настраиваемые параметры (`GetSettings()`).