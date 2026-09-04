---
title: Добавление API скрипта
---

# Добавление API скрипта

На примере реализованного в этом проекте `ScriptGlobal` (API глобального AI) показаны полные шаги добавления нового класса API скрипта (доступного для AI и GameScript) в jrpm.

## Предыстория

- Классы API скриптов находятся в `src/script/api/`: `script_<имя>.hpp` (объявление) + `script_<имя>.cpp` (реализация);
- `.hpp` **автоматически обнаруживается** на этапе сборки через `file(GLOB script_*.hpp)` и генерирует привязки Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **ручная регистрация класса не требуется**;
- `.cpp` необходимо добавить в список исходников `src/script/api/CMakeLists.txt`.

## 1. Заголовочный файл `script_global.hpp`

```cpp
/** @file script_global.hpp Документация. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Документация класса, обязательно указать @api.
 * @api ai game        # Доступно и AI, и GS; "game" только GS; "-ai" исключает AI
 */
class ScriptGlobal : public ScriptObject {
public:
	/** Перечисления экспортируются как константы класса (например, AIGlobal.VT_TRAIN). */
	enum VehicleType {
		VT_TRAIN = ::VehicleType::Train,
		VT_AIRCRAFT = ::VehicleType::Aircraft,
	};

	/** @api ai game */
	static bool IsGlobalAccessAllowed();

	/** @api ai game */
	static SQInteger GetCompanyCount();

	/** @api ai game */
	static std::optional<std::string> GetCompanyName(ScriptCompany::CompanyID company);
};

#endif /* SCRIPT_GLOBAL_HPP */
```

## 2. Реализация `script_global.cpp`

```cpp
#include "../../stdafx.h"
#include "script_global.hpp"
#include "../../company_base.h"
// ...

/* static */ SQInteger ScriptGlobal::GetCompanyCount()
{
	if (!IsGlobalAccessAllowed()) return 0;
	SQInteger count = 0;
	for (const Company *c : Company::Iterate()) count++;
	return count;
}
```

Ключевые моменты:
- Все методы `static`, с префиксом `/* static */` в определении;
- Типы возвращаемых значений: Squirrel-дружественные типы `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*` и т.д.;
- Контроль доступа: проверка `ScriptCompanyMode::IsDeity() || настройка` внутри метода, при невыполнении возврат значения ошибки.

## 3. Класс списка (опционально)

Для возврата списка компаний/объектов определите подкласс `ScriptList`:

```cpp
class ScriptGlobalCompanyList : public ScriptList {
public:
	#ifdef DOXYGEN_API
		ScriptGlobalCompanyList();
	#else
		ScriptGlobalCompanyList(HSQUIRRELVM vm);
	#endif
};
```

```cpp
ScriptGlobalCompanyList::ScriptGlobalCompanyList(HSQUIRRELVM vm)
{
	ScriptList::FillList<Company>(vm, this);
}
```

## 4. Регистрация в сборке

В список исходников `src/script/api/CMakeLists.txt` добавить:

```cmake
script_global.cpp
```

::: tip
`.hpp` не требует регистрации (GLOB автоматически обнаруживает); но **требуется переконфигурация CMake** для обнаружения нового `.hpp` и генерации привязок.
:::

## 5. Использование в скрипте

```js
// AI сторона: префикс класса AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS сторона: префикс GS
GSGlobal.GetCompanyCount();
```

## Контрольный список

- [ ] `script_<name>.hpp` (с аннотацией `@api`)
- [ ] `script_<name>.cpp` (реализация с `/* static */`)
- [ ] `CMakeLists.txt` добавить `.cpp`
- [ ] Переконфигурация + сборка
- [ ] (Опционально) Пример скрипта в `bin/ai/`