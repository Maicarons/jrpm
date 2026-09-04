---
title: Adicionar Item de Configuracao
---

# Adicionar Item de Configuracao

O sistema de configuracoes do jrpm e **orientado a INI**: `src/table/settings/*.ini` sao as fontes de definicao de configuracoes, e em tempo de construcao o settingsgen gera o codigo; os campos de estrutura correspondentes estao em `src/settings_type.h`. Usando o `network.content_download_parallel` implementado neste projeto como exemplo.

## 1. Campo na Estrutura

`src/settings_type.h`, encontre a estrutura de configuracao correspondente (rede -> `NetworkSettings`, jogo -> subestruturas relacionadas de `GameSettings`, script -> `ScriptSettings`):

```cpp
struct NetworkSettings {
    // ...
    std::string content_mirrors;                          ///< URIs de espelho separados por virgula
    uint8_t content_download_parallel = 4;                ///< Numero de arquivos para download paralelo
    // ...
};
```

## 2. Definicao INI

`src/table/settings/network_settings.ini`, adicione bloco `[SDTC_VAR]` (configuracoes de cliente usam `SDTC_*`, configuracoes de jogo usam `SDT_*`):

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

Campos comuns:

| Campo | Descricao |
|---|---|
| `var` | Nome completo da configuracao (caminho da estrutura, ex: `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (string, requer `length`) |
| `flags` | `SettingFlag::NotInSave` (nao salvar no arquivo), `NoNetworkSync` (nao sincronizar), `NetworkOnly`, `GuiZeroIsSpecial`, etc. |
| `def/min/max/interval` | Valor padrao/faixa/passo |
| `str/strhelp` | Strings de exibicao na interface de configuracao |
| `cat` | Categoria da configuracao (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. Strings (exibicao na interface de configuracao)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Permitir que IAs acessem dados globais do jogo: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Permitir que IAs usem a API Global...
```

- Configuracoes booleanas terminam com `{STRING2}` (ligado/desligado);
- Configuracoes numericas consultem formato de strings similares.

## 4. Usar no Codigo

```cpp
// Configuracao de cliente
_settings_client.network.content_download_parallel

// Configuracao de jogo
_settings_game.script.allow_global_ai_access
```

## 5. Cenarios Especiais

- **Configuracao de servidor**: `network.*` em jogos multijogador e enviada pelo servidor (`NetworkOnly` + mecanismo de `sync` do servidor);
- **Associacao com arquivo**: Configuracoes de jogo (`game.*`) sao salvas com o arquivo (padrao); configuracoes de cliente (`network.*`/`gui.*`) tem `NotInSave` como padrao;
- **Callback**: `pre_cb`/`post_cb` podem ser vinculados a manipulacao de mudanca de valor (ex: `UpdateClientConfigValues()`).

## Lista de Verificacao

- [ ] Campo em `settings_type.h`
- [ ] Bloco `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` no `.ini` correspondente
- [ ] Strings em english.txt (para exibicao na interface de configuracao)
- [ ] Reconfigurar + construir