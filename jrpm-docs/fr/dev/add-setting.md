---
title: Ajouter un parametre
---

# Ajouter un parametre

Le systeme de parametres de jrpm est **pilote par INI** : `src/table/settings/*.ini` est la source de definition des parametres, le code est genere par settingsgen en phase de construction ; les champs de structure correspondants se trouvent dans `src/settings_type.h`. Prenons l'exemple de `network.content_download_parallel` implemente dans ce projet.

## 1. Champ de structure

`src/settings_type.h`, trouver la structure de parametres correspondante (reseau → `NetworkSettings`, jeu → sous-structures `GameSettings` associees, script → `ScriptSettings`) :

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< URI des miroirs separes par des virgules
	uint8_t content_download_parallel = 4;                ///< Nombre de telechargements paralleles
	// ...
};
```

## 2. Definition INI

`src/table/settings/network_settings.ini`, ajouter un bloc `[SDTC_VAR]` (parametres client utilisent `SDTC_*`, parametres de jeu utilisent `SDT_*`) :

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

Champs courants :

| Champ | Description |
|---|---|
| `var` | Nom complet du parametre (chemin de la structure, ex. `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (chaine, necessite `length`) |
| `flags` | `SettingFlag::NotInSave` (pas dans l'archive), `NoNetworkSync` (pas synchronise), `NetworkOnly`, `GuiZeroIsSpecial`, etc. |
| `def/min/max/interval` | Valeur par defaut/plage/pas |
| `str/strhelp` | Chaine d'affichage dans l'interface des parametres |
| `cat` | Categorie du parametre (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. Chaine (affichage dans l'interface des parametres)

`src/lang/english.txt` :

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- Les parametres booléens se terminent par `{STRING2}` (on/off) ;
- Les parametres numeriques se referent au format des chaines similaires.

## 4. Utilisation dans le code

```cpp
// Parametre client
_settings_client.network.content_download_parallel

// Parametre de jeu
_settings_game.script.allow_global_ai_access
```

## 5. Scenarios speciaux

- **Parametres serveur** : `network.*` en multijoueur sont envoyes par le serveur (`NetworkOnly` + mecanisme de synchronisation serveur) ;
- **Association d'archive** : Les parametres de jeu (`game.*`) sont sauvegardes avec l'archive (par defaut) ; les parametres client (`network.*`/`gui.*`) sont par defaut `NotInSave` ;
- **Callback** : `pre_cb`/`post_cb` peuvent etre accroches pour le traitement des changements de valeur (ex. `UpdateClientConfigValues()`).

## Liste de verification

- [ ] Champ dans `settings_type.h`
- [ ] Bloc `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` dans le `.ini` correspondant
- [ ] Chaine english.txt (pour l'affichage dans l'interface des parametres)
- [ ] Reconfigurer + construire