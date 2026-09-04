---
title: Добавление настройки
---

# Добавление настройки

Система настроек jrpm — **INI-драйв**: `src/table/settings/*.ini` — источники определения настроек, на этапе сборки settingsgen генерирует код; соответствующие поля структур находятся в `src/settings_type.h`. На примере реализованной в этом проекте настройки `network.content_download_parallel`.

## 1. Поле структуры

`src/settings_type.h`, найти соответствующую структуру настроек (сеть → `NetworkSettings`, игра → подструктуры `GameSettings`, скрипты → `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< URI зеркал через запятую
	uint8_t content_download_parallel = 4;                ///< количество параллельных загрузок
	// ...
};
```

## 2. INI-определение

`src/table/settings/network_settings.ini`, добавить блок `[SDTC_VAR]` (клиентские настройки используют `SDTC_*`, игровые — `SDT_*`):

```ini
[SDTC_VAR]
var      = network.content_download_parallel
type     = SLE_UINT8
flags    = SettingFlag::NotInSave, SettingFlag::NoNetworkSync
def      = 4
min      = 1
max      = 8
cat      = SC_BASIC
```

Часто используемые поля:

| Поле | Описание |
|---|---|
| `var` | Полное имя настройки (путь структуры, например `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (строка, требуется `length`) |
| `flags` | `SettingFlag::NotInSave` (не в сохранении), `NoNetworkSync` (не синхронизируется), `NetworkOnly`, `GuiZeroIsSpecial` и т.д. |
| `def/min/max/interval` | Значение по умолчанию / диапазон / шаг |
| `str/strhelp` | Отображаемые строки в интерфейсе настроек |
| `cat` | Категория настройки (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. Строки (отображение в интерфейсе настроек)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- Булевы настройки заканчиваются на `{STRING2}` (вкл/выкл);
- Числовые настройки следуют формату аналогичных строк.

## 4. Использование в коде

```cpp
// Клиентская настройка
_settings_client.network.content_download_parallel

// Игровая настройка
_settings_game.script.allow_global_ai_access
```

## 5. Особые случаи

- **Серверные настройки**: `network.*` в многопользовательской игре отправляются сервером (`NetworkOnly` + механизм `sync` сервера);
- **Связь с сохранением**: игровые настройки (`game.*`) сохраняются с файлом сохранения (по умолчанию); клиентские настройки (`network.*`/`gui.*`) по умолчанию `NotInSave`;
- **Колбэки**: `pre_cb`/`post_cb` могут подключать обработку изменений значений (например, `UpdateClientConfigValues()`).

## Контрольный список

- [ ] Поле в `settings_type.h`
- [ ] Блок `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` в соответствующем `.ini`
- [ ] (Для отображения в интерфейсе) строка в english.txt
- [ ] Переконфигурация + сборка