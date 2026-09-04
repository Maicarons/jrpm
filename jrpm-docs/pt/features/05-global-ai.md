---
title: Interface de IA de Percepcao Global
---

## Situacao Atual (Conclusao da Pesquisa)

- Framework NoAI: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) -- o sistema de IA original **mantido como estava**, este recurso nao o altera;
- Sistema de API de script: `src/script/api/script_*.hpp/.cpp` (60+ classes), vinculacao Squirrel gerada **automaticamente** em tempo de construcao:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` descobre automaticamente novas classes de API (gera `ai_*.sq.hpp` / `gs_*.sq.hpp`), **novas classes nao precisam de registro manual**;
  - `.cpp` precisa ser adicionado a lista de fontes do CMake; comentario de classe `@api ai game` controla exposicao para AI/GS;
- Dados da empresa: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- Modo divindade GS: `ScriptCompanyMode::IsDeity()`;
- Limitacao atual: AI por padrao so pode acessar dados de sua propria empresa, sem API de agregacao global, sem alternador de acesso.

**Conclusao**: Adicionar uma classe de API `ScriptGlobal` (registro automatico) + alternador `game.script.allow_global_ai_access` e suficiente para implementar "manter NoAI, adicionar IA de percepcao global, com controle de acesso".

## Implementacao deste Recurso

### 1. API `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

Metodos estaticos expostos para AI e GS (`@api ai game`):

| Metodo | Retorno | Fonte de Dados |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| alternador de configuracao` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()` | int | `MapSizeX/Y` |
| `GetDate()` | int | `EconTime::CurDate` |
| `GetYear()` | int | `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` contagem por owner |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- ID de empresa reutiliza `ScriptCompany::CompanyID` (COMPANY_SELF resolve para a empresa atual);
- **Controle de acesso**: Todos os metodos verificam `IsGlobalAccessAllowed()` na primeira linha, retornando -1/nullopt/false se nao atendido (GS sempre disponivel, AI controlado pelo alternador);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` enumera todas as empresas.

### 2. Alternador de Acesso (Configuracao)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, padrao false, `SC_EXPERT`);
- Campo de estrutura `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- String `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. IA de Exemplo: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- Demonstracao: Le e registra dados financeiros/veiculos/estacoes/classificacao e informacoes do mapa de todas as empresas; explica o controle de acesso (avisa quando o alternador nao esta ativado);
- Esta IA e a implementacao de referencia para "percepcao global do jogo + controle de acesso", e a logica de controle da IA pode ser expandida em seu metodo `Start()`.

### Arquivos Envolvidos

- Novo `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (adicionar .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- Novo `bin/ai/GlobalAI/info.nut`, `main.nut`

## Pontos de Verificacao

1. A nova IA aparece na interface de configuracao de IA (scan de `ai/GlobalAI`);
2. Quando `game.script.allow_global_ai_access` nao esta ativado, o registro da IA indica sem permissao; quando ativado, exibe dados completos de cada empresa;
3. GameScript pode usar `GSGlobal` sem o alternador;
4. Em tempo de construcao, `ai_global.sq.hpp` / `gs_global.sq.hpp` sao gerados automaticamente sem erros (depende de `file(GLOB)`, necessario reconfigurar CMake).

## Direcoes de Expansao

- Adicionar mais dados agregados como economia/emprestimos/infraestrutura (`GetCompanyInfrastructure`);
- Adicionar assinatura de eventos (falencia/aquisicao/criacao de nova empresa);
- Transformar a logica de decisao da IA em parametros configuraveis (`GetSettings()`).