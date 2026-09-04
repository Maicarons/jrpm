---
title: Исследование структуры проекта и организации кода
---

> Объект исследования: `G:\GitHub\OpenTTD-patches` (рабочая область исходного кода jgrpp)
> Справочные ресурсы: `G:\game\openttd-jgrpp` (скомпилированная полная игра, только чтение)
> Дата исследования: 2026-08-14
> Ветка: jgrpp (текущий HEAD `5b5c452e1b`, примерно версия 16.0)

---

## 1. Обзор проекта

OpenTTD jgrpp (JGR's Patchpack) — известное расширенное ответвление OpenTTD. Исходный код рабочей области соответствует версии **16.0** и содержит множество частных функций JGR (tracerestrict, программируемые сигналы, scheduled dispatch, шаблонная замена, улучшенные сигналы и т.д.).

| Компонент | Описание |
|---|---|
| Язык | C++20 (`CMAKE_CXX_STANDARD 20`, без расширений) |
| Сборка | CMake (≥3.17), `src/CMakeLists.txt` организован по каталогам |
| Скриптовый движок | Squirrel (`src/3rdparty/squirrel/`), используется для AI / GameScript / скриптов шаблонов |
| Сеть | Собственный протокольный слой TCP/UDP (`src/network/`), HTTP через WinHttp (Windows) / libcurl (остальные) / JS (Emscripten) |
| Сохранение | `src/saveload/` + `src/sl/` собственный бинарный формат |
| Потоки | `src/thread.h` + `src/worker_thread.cpp` (пул задач WorkerThreadPool), `src/timer/` таймеры |

---

## 2. Структура каталогов исходного кода и разделение модулей

Верхний уровень: `CMakeLists.txt` — корневой скрипт сборки; `src/` — весь исходный код C++; `bin/` — данные времени выполнения (совместимые скрипты AI, языковые файлы и т.д.); `media/`, `os/`, `cmake/`, `docs/` — ресурсы, платформенный код, скрипты сборки и документация.

Основные подкаталоги в `src/` и их назначение:

| Каталог | Назначение |
|---|---|
| `src/core/` | Базовые инструменты: типы pool, bitset, контейнеры, обёртки потоков, битовые операции и т.д. |
| `src/network/` | Сеть: сервер/клиент/UDP/HTTP/загрузка контента/протокол администратора (`core/` — основа протокола) |
| `src/script/` | Фреймворк выполнения скриптов: обёртка Squirrel, экземпляры, конфигурация; `api/` — все классы API, доступные AI/GS |
| `src/ai/` | Фреймворк NoAI (экземпляры AI, сканер, конфигурация, GUI) |
| `src/game/` | Фреймворк GameScript (экземпляры GS, конфигурация, GUI) |
| `src/newgrf/` | Декодирование и обработка NewGRF |
| `src/pathfinder/` | Навигация (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | Чтение/запись сохранений |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | Бэкенды рендеринга/аудио/видео |
| `src/lang/` | Исходные файлы локализации (генерируются strgen) |
| `src/table/` | Статические таблицы; `table/settings/*.ini` — **источники определения настроек** (генерируются settingsgen) |
| `src/3rdparty/` | Сторонние библиотеки (squirrel, llvm, icu и т.д.) |
| `src/timer/`, `src/os/`, `src/misc/` | Таймеры, платформа, разное |

Файлы верхнего уровня названы по системе (например, `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), следуя конвенции OpenTTD: `*_cmd` — логика команд, `*_gui` — окна, `*_base/_type/_func` — структуры данных и встроенные функции.

---

## 3. Способ сборки

- **Три шага CMake**: `cmake -B build ..` → `cmake --build build` → результат `openttd.exe`. Репозиторий включает `build.sh` / `build-dedicated.sh`.
- **Зависимости**: объявлены в `vcpkg.json` (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus и т.д.); Windows использует WinHttp (без curl), не-Win — libcurl (`CMakeLists.txt:121-127`).
- **Инструменты (host tools)**: `strgen` (языковые файлы), `settingsgen` (генерация кода настроек из `src/table/settings/*.ini`), `squirrel_export` (генерация привязок Squirrel из `src/script/api/script_*.hpp`).
- **Ключевые генерируемые файлы**: `generated/script/api/<ai|gs>/...sq.hpp` (привязки API, **автоматически обнаруживаются через `file(GLOB script_*.hpp)`, новые классы API не требуют изменения списка регистрации** — достаточно добавить `.hpp` и включить `.cpp` в `src/script/api/CMakeLists.txt`); `generated/rev.cpp`; `generated/ottdres.rc`.
- **Система настроек**: современная версия перешла на **INI-драйв** — `src/table/settings/*.ini` (содержит секции `[SDTC_VAR]`, `cat=SC_*`, `flags`, `post_cb` и т.д.), из которых settingsgen генерирует `settings_*.cpp/h`; соответствующие поля структур находятся в `src/settings_type.h` (например, `NetworkSettings` начиная со строки 575).

---

## 4. Расположение реализации пяти функциональных систем

### 4.1 Загрузка ресурсов (загрузка контента / BaNaNaSplit)

| Область | Расположение |
|---|---|
| Главный класс клиента контента | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler` (также `ContentCallback` + `HTTPCallback`) |
| GUI загрузки контента | `src/network/network_content_gui.cpp/.h` |
| HTTP клиент | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, асинхронный событийно-ориентированный (неблокирующий, основной цикл опроса) |
| Строка подключения сервера контента | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`: переменная окружения `OTTD_CONTENT_SERVER_CS`, по умолчанию `content.openttd.org` (TCP протокол метаданных) |
| URI зеркала | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`: переменная окружения `OTTD_CONTENT_MIRROR_URI`, по умолчанию `https://binaries.openttd.org/bananas` |
| Процесс загрузки | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (POST всех ID контента на зеркало, зеркало возвращает многофайловый tar-поток, запись файлов по одному) → `AfterDownload()` выполняет gunzip + `TarScanner` распаковка |
| Распаковка | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| Потоковые средства (для параллелизации) | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` — обёртка платформенных потоков |

**Текущий вывод**: ① только 1 зеркало, настраиваемое только через переменную окружения, без внутриигровых настроек; ② загрузка **односоединительная, последовательная** (один POST всех файлов); ③ распаковка выполняется синхронно в главном потоке. → Точки модификации для многопоточности/многозеркальности чётко определены.

### 4.2 Лимиты онлайн-игроков / компаний на сервере

| Область | Расположение |
|---|---|
| Константа лимита клиентов | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| Пул клиентов | Тот же файл `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` — `uint32_t` |
| Пул ID компаний | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; фиктивные компании занимают 253/254/255 |
| Маска компаний | Тот же файл `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 бит, может отслеживать только 16 компаний) |
| Проверка при подключении | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| Настройки клиента | `src/table/settings/network_settings.ini:231/241` — `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); структура в `src/settings_type.h` `NetworkSettings` |
| **Разрядность протокола (жёсткое ограничение)** | `src/network/core/network_game_info.cpp` — `companies_max` и `clients_max` передаются через **`Send_uint8`/`Recv_uint8`** (строки 251-296, 422-432) |
| Отображение списка серверов | `src/network/network_gui.cpp:519` и т.д. |

**Текущий вывод**:
- **Лимит клиентов = 255 уже является протокольным пределом** (поле uint8 + совместимость с Game Coordinator/браузером серверов). Для его превышения необходимо изменить связанные поля `network_game_info` на uint16 (полный путь client↔сервер + UDP широковещание + протокол Game Coordinator), что является внешним изменением протокола.
- **Лимит компаний = 15** (End=0xF у `CompanyIDTag`). Можно безопасно поднять до **252** (End=0xFC): основа остаётся uint8, ширина байта сохранения не меняется (совместимость со старыми сохранениями), требуется синхронное расширение `CompanyMask` (uint16→uint32) и интерфейса/циклов.

### 4.3 Группировка транспортных средств

| Область | Расположение |
|---|---|
| Структура данных группы | `src/group.h` — `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| Команды групп | `src/group_cmd.cpp` — `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (добавление ТС с общими приказами в существующую группу, около строки 718 `AddVehicleToGroup`) |
| Регистрация команд | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; перечисление в `src/command_type.h` `enum class Commands` (начиная со строки 492) |
| GUI групп | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| ТС↔группа | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| Приказы/общее расписание | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, привязан к списку приказов) |
| Статистика групп | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` и т.д. (group.h:125-128) |

**Текущий вывод**: уже есть `CmdAddSharedVehicleGroup` (добавление ТС с общими приказами одного ТС в группу) и `CmdCreateGroupFromList` (создание группы из списка), но **нет полной логики «автоматического создания групп/группировки по общим приказам»**. Новая команда `AutoGroupSharedOrders` (обход всех основных ТС компании → агрегация по `OrderList` → автоматическое создание группы и группировка) имеет чёткий путь реализации.

### 4.4 Подсказка стоимости строительства (подсказка цены при наведении на рельсы и т.д.)

| Область | Расположение |
|---|---|
| GUI/логика строительства путей | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; дороги `road_gui.cpp/road_cmd.cpp` |
| Оценка стоимости | `DoCommand` в каждом `*_cmd.cpp` возвращает `CommandCost`; в GUI доступен режим `DC_QUERY_COST` для запроса цены |
| Тайл под курсором | `src/viewport_func.h:36` — `GetTileBelowCursor()`; `_cursor.pos` (экранные координаты); `src/viewport.cpp:1056` |
| Существующий механизм текстовых подсказок | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)` (плавающий текст в мировых координатах), `UpdateTextEffect`; `src/texteff.cpp` |
| Существующий UX подсказок строительства | Панель путей `OnPlaceDrag` при перетаскивании показывает выделенную область и накопленную стоимость (`_thd` tilehighlight, `src/tilehighlight_func.h`); строка состояния `statusbar_gui.cpp` может отображать стоимость инструмента |
| Точка обновления каждого кадра | `OnMouseLoop` в окнах панелей / `HandleMouseEvents` в `viewport.cpp` (строки 5422/5733) |

**Текущий вывод**: нет «подсказки цены при наведении мыши». Можно использовать `AddTextEffect` с привязкой к тайлу под курсором (следует за тайлом под курсором) или нарисовать собственную подсказку в экранных координатах; стоимость можно получить через `DC_QUERY_COST` для одиночного тайла с текущим инструментом + накопление для выделенной области.

### 4.5 Интерфейс AI (сохранение NoAI + глобальный AI)

| Область | Расположение |
|---|---|
| Фреймворк NoAI | `src/ai/` — `ai_core.cpp` (главный цикл AICore), `ai_instance.cpp` (AIInstance/Squirrel VM), `ai_scanner.cpp` (сканирование каталога `ai/`), `ai_gui.cpp` (выбор/конфигурация), `ai_config.cpp` |
| Фреймворк GameScript | `src/game/` — `game_core.cpp`, `game_instance.cpp` и т.д. (GS — «режим божества», права выше, чем у AI) |
| Классы API скриптов | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin` и т.д., всего 60+ классов) |
| Автоматическая регистрация API | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` автоматически генерирует привязки `ai_*.sq.hpp`/`gs_*.sq.hpp`; `.cpp` нужно добавить в список исходников (начиная со строки 235) |
| Макросы привязки Squirrel | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` — привязка контроллера AI |
| Диспетчеризация экземпляров | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (приостановка/возобновление), события `script_event*` |
| Контроль доступа компаний | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) различает режим божества GS; проверка параметров API компании `ResolveCompanyID`, `EnforceCompanyModeValid` |
| Настройки скриптов | `src/table/settings/script_settings.ini` (`game.script.*`); конфигурация экземпляров AI `ai_config.cpp` |

**Текущий вывод**: в современном script API некоторая информация о конкурентах (например, `GetBankBalance`) уже не ограничена, но **нет агрегированного API для глобального восприятия** (нет единого входа для «перечисления всех компаний / глобальной экономики / глобальной статистики карты»), нет точки доступа «глобального AI» с управлением через переключатель. Добавление нового класса API `ScriptGlobal` (автоматическая регистрация GLOB) + настройка `game.script` «разрешить глобальное восприятие AI» — это реализуемо, GS всегда доступно (режим божества), AI управляется переключателем — т.е. «сохранение NoAI, добавление глобального AI с контролем доступа».

---

## 5. Общий обзор рисков изменений

| Функция | Основные изменяемые файлы | Риск |
|---|---|---|
| F1 Многозеркальная/многопоточная загрузка | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | Средний (конечный автомат сетевых колбэков требует осторожности) |
| F2 Расширение лимитов | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (опционально uint16) | Низкий–средний (компании 252 низкий риск; клиенты >255 требуют изменения протокола) |
| F3 Автоматическая группировка ТС | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | Низкий |
| F4 Подсказка строительства | Новые `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` хуки, `src/lang/english.txt` | Низкий–средний |
| F5 Глобальный AI | Новые `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, пример AI `bin/ai/GlobalAI/` | Низкий (автоматическая регистрация фреймворка) |

> Примечание: все изменения основаны на локальной рабочей области ветки `jgrpp`, не компилировались (нет доступной цепочки инструментов сборки на данной машине); все патчи доступны для `git diff` и `git apply`.