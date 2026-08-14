---
title: 新增设置项
---

# 新增设置项

jrpm 的设置系统是 **INI 驱动**：`src/table/settings/*.ini` 是设置定义源，构建期由 settingsgen 生成代码；对应结构体字段在 `src/settings_type.h`。以本项目实现的 `network.content_download_parallel` 为例。

## 1. 结构体字段

`src/settings_type.h`，找到对应设置结构体（网络 → `NetworkSettings`，游戏 → `GameSettings` 相关子结构，脚本 → `ScriptSettings`）：

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< 逗号分隔的镜像 URI
	uint8_t content_download_parallel = 4;                ///< 并行下载文件数
	// ...
};
```

## 2. INI 定义

`src/table/settings/network_settings.ini`，追加 `[SDTC_VAR]` 块（客户端设置用 `SDTC_*`，游戏设置用 `SDT_*`）：

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

常用字段：

| 字段 | 说明 |
|---|---|
| `var` | 设置全名（结构体路径，如 `network.xxx` / `game.script.xxx`） |
| `type` | `SLE_UINT8/16/32/64`、`SLE_INT*`、`SLE_BOOL`、`SLE_STR`（字符串，需 `length`） |
| `flags` | `SettingFlag::NotInSave`（不入档）、`NoNetworkSync`（不同步）、`NetworkOnly`、`GuiZeroIsSpecial` 等 |
| `def/min/max/interval` | 默认值/范围/步进 |
| `str/strhelp` | 设置界面显示字符串 |
| `cat` | 设置分类（`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`） |

## 3. 字符串（设置界面显示）

`src/lang/english.txt`：

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- 布尔设置以 `{STRING2}`（开/关）结尾；
- 数值设置参考同类字符串格式。

## 4. 在代码中使用

```cpp
// 客户端设置
_settings_client.network.content_download_parallel

// 游戏设置
_settings_game.script.allow_global_ai_access
```

## 5. 特殊场景

- **服务端设置**：`network.*` 在多人游戏中由服务器下发（`NetworkOnly` + 服务器 `sync` 机制）；
- **存档关联**：游戏设置（`game.*`）随存档保存（默认）；客户端设置（`network.*`/`gui.*`）默认 `NotInSave`；
- **回调**：`pre_cb`/`post_cb` 可挂钩值变更处理（如 `UpdateClientConfigValues()`）。

## 完成清单

- [ ] `settings_type.h` 字段
- [ ] 对应 `.ini` 的 `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` 块
- [ ] （设置界面显示用）english.txt 字符串
- [ ] 重新 configure + 构建
