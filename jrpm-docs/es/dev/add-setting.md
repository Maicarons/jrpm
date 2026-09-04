---
title: Anadir nueva configuracion
---

# Anadir nueva configuracion

El sistema de configuracion de jrpm es **controlado por INI**: `src/table/settings/*.ini` son las fuentes de definicion de configuracion, en tiempo de construccion settingsgen genera el codigo; los campos de estructura correspondientes estan en `src/settings_type.h`. Usando `network.content_download_parallel` implementado en este proyecto como ejemplo.

## 1. Campo de estructura

`src/settings_type.h`, encontrar la estructura de configuracion correspondiente (red → `NetworkSettings`, juego → subestructura de `GameSettings`, script → `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< URI de espejos separados por comas
	uint8_t content_download_parallel = 4;                ///< Numero de archivos de descarga paralela
	// ...
};
```

## 2. Definicion INI

`src/table/settings/network_settings.ini`, anadir bloque `[SDTC_VAR]` (configuracion de cliente usa `SDTC_*`, configuracion de juego usa `SDT_*`):

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

Campos comunes:

| Campo | Descripcion |
|---|---|
| `var` | Nombre completo de la configuracion (ruta de estructura, ej. `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (cadena, necesita `length`) |
| `flags` | `SettingFlag::NotInSave` (no se guarda en archivo), `NoNetworkSync` (no se sincroniza), `NetworkOnly`, `GuiZeroIsSpecial`, etc. |
| `def/min/max/interval` | Valor predeterminado/rango/paso |
| `str/strhelp` | Cadena de visualizacion en la interfaz de configuracion |
| `cat` | Categoria de configuracion (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. Cadena de texto (visualizacion en interfaz de configuracion)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- Las configuraciones booleanas terminan con `{STRING2}` (activado/desactivado);
- Las configuraciones numericas siguen el formato de cadenas similares existentes.

## 4. Uso en codigo

```cpp
// Configuracion de cliente
_settings_client.network.content_download_parallel

// Configuracion de juego
_settings_game.script.allow_global_ai_access
```

## 5. Escenarios especiales

- **Configuracion de servidor**: `network.*` en multijugador es enviada por el servidor (`NetworkOnly` + mecanismo de `sync` del servidor);
- **Relacion con archivo**: Las configuraciones de juego (`game.*`) se guardan con el archivo (predeterminado); las configuraciones de cliente (`network.*`/`gui.*`) tienen `NotInSave` por predeterminado;
- **Callback**: `pre_cb`/`post_cb` pueden enlazar manejo de cambios de valor (ej. `UpdateClientConfigValues()`).

## Lista de verificacion

- [ ] Campo en `settings_type.h`
- [ ] Bloque `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` en el `.ini` correspondiente
- [ ] Cadena en english.txt (para visualizacion en interfaz de configuracion)
- [ ] Reconfigurar + construir