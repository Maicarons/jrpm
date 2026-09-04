---
title: Pesquisa de Estrutura do Projeto e Organizacao do Codigo
---

> Objeto de pesquisa: `G:\GitHub\OpenTTD-patches` (area de trabalho do codigo fonte jgrpp)
> Recursos de referencia: `G:\game\openttd-jgrpp` (jogo completo compilado, somente leitura)
> Data da pesquisa: 2026-08-14
> Branch: jgrpp (HEAD atual `5b5c452e1b`, aproximadamente versao 16.0)

---

## 1. Visao Geral do Projeto

OpenTTD jgrpp (JGR's Patchpack) e um conhecido branch de aprimoramento baseado no OpenTTD. O codigo fonte desta area de trabalho corresponde a versao **16.0**, contendo muitos recursos proprietarios do JGR (tracerestrict, programmable signals, scheduled dispatch, substituicao de modelos, sinais aprimorados, etc.).

| Item | Descricao |
|---|---|
| Linguagem | C++20 (`CMAKE_CXX_STANDARD 20`, sem extensoes) |
| Construcao | CMake (>=3.17), `src/CMakeLists.txt` organizado por diretorio |
| Motor de script | Squirrel (`src/3rdparty/squirrel/`), usado para AI / GameScript / scripts de template |
| Rede | Camada de protocolo TCP/UDP propria (`src/network/`), HTTP usa WinHttp (Windows) / libcurl (outros) / JS (Emscripten) |
| Arquivo | `src/saveload/` + `src/sl/` formato binario proprio |
| Threads | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool), `src/timer/` temporizadores |

---

## 2. Estrutura de Diretorios do Codigo Fonte e Divisao de Modulos

Nivel superior: `CMakeLists.txt` e o script de construcao raiz; `src/` contem todo o codigo fonte C++; `bin/` contem dados de runtime (scripts de compatibilidade AI, arquivos de idioma, etc.); `media/`, `os/`, `cmake/`, `docs/` contem recursos, codigo de plataforma, scripts de construcao e documentacao.

Subdiretorios principais dentro de `src/` e suas responsabilidades:

| Diretorio | Responsabilidade |
|---|---|
| `src/core/` | Ferramentas basicas: tipos pool, bitset, contêineres, encapsulamento de threads, operacoes bitwise, etc. |
| `src/network/` | Rede: servidor/cliente/UDP/HTTP/download de conteudo/protocolo de administracao (subdiretorio `core/` e base do protocolo) |
| `src/script/` | Framework de execucao de script: encapsulamento Squirrel, instancias, configuracao; `api/` contem todas as classes de API expostas para AI/GS |
| `src/ai/` | Framework NoAI (instancias de AI, scanner, configuracao, GUI) |
| `src/game/` | Framework GameScript (instancias de GS, configuracao, GUI) |
| `src/newgrf/` | Decodificacao e processamento NewGRF |
| `src/pathfinder/` | Navegacao (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | Leitura e escrita de arquivos salvos |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | Backends de renderizacao/audio/video |
| `src/lang/` | Fontes de strings de localizacao (geradas por strgen) |
| `src/table/` | Tabelas estaticas; `table/settings/*.ini` sao as **fontes de definicao de configuracoes** (geradas por settingsgen) |
| `src/3rdparty/` | Bibliotecas de terceiros (squirrel, llvm, icu, etc.) |
| `src/timer/`, `src/os/`, `src/misc/` | Temporizadores, plataforma, diversos |

Arquivos soltos no nivel superior sao nomeados por sistema (ex: `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), seguindo a convencao do OpenTTD: `*_cmd` logica de comando, `*_gui` janelas, `*_base/_type/_func` estruturas de dados e funcoes inline.

---

## 3. Metodo de Construcao

- **Tres passos CMake**: `cmake -B build ..` -> `cmake --build build` -> artefato `openttd.exe`. O repositorio inclui `build.sh` / `build-dedicated.sh`.
- **Dependencias**: `vcpkg.json` declara (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus, etc.); Windows usa WinHttp (sem curl), nao-Windows usa libcurl (`CMakeLists.txt:121-127`).
- **Ferramentas de host (host tools)**: `strgen` (arquivos de idioma), `settingsgen` (gera codigo de configuracao a partir de `src/table/settings/*.ini`), `squirrel_export` (gera vinculacoes Squirrel a partir de `src/script/api/script_*.hpp`).
- **Artefatos chave gerados**: `generated/script/api/<ai|gs>/...sq.hpp` (vinculacoes de API, **descobertas automaticamente por `file(GLOB script_*.hpp)`, novas classes de API nao precisam modificar lista de registro**, apenas adicionar `.hpp` e incluir `.cpp` na lista de fontes de `src/script/api/CMakeLists.txt`); `generated/rev.cpp`; `generated/ottdres.rc`.
- **Sistema de configuracoes**: A versao moderna foi alterada para **orientada a INI** -- `src/table/settings/*.ini` (com secoes `[SDTC_VAR]`, `cat=SC_*` categorias, `flags`, `post_cb`, etc.), gerando `settings_*.cpp/h` via settingsgen; membros de estrutura correspondentes em `src/settings_type.h` (ex: `NetworkSettings` a partir da linha 575).

---

## 4. Localizacao da Implementacao dos Cinco Principais Sistemas Funcionais

### 4.1 Download de Recursos (Download de Conteudo / BaNaNaSplit)

| Ponto de Interesse | Local |
|---|---|
| Classe principal do cliente de conteudo | `src/network/network_content.h/.cpp` -- `ClientNetworkContentSocketHandler` (tambem `ContentCallback` + `HTTPCallback`) |
| GUI de download de conteudo | `src/network/network_content_gui.cpp/.h` |
| Cliente HTTP | `src/network/core/http.h/.cpp` -- `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, assincrono orientado a eventos (nao bloqueante, polling no loop principal) |
| String de conexao do servidor de conteudo | `src/network/core/config.cpp` -- `NetworkContentServerConnectionString()`: variavel de ambiente `OTTD_CONTENT_SERVER_CS`, padrao `content.openttd.org` (protocolo TCP de metadados) |
| URI de espelho | `src/network/core/config.cpp` -- `NetworkContentMirrorUriString()`: variavel de ambiente `OTTD_CONTENT_MIRROR_URI`, padrao `https://binaries.openttd.org/bananas` |
| Fluxo de download | `DownloadSelectedContent()` -> `DownloadSelectedContentHTTP()` (POST de todos os IDs de conteudo para o espelho, espelho retorna um fluxo tar multifile, escreve arquivo por arquivo) -> `AfterDownload()` faz gunzip + `TarScanner` descompacta |
| Descompressao | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| Infraestrutura de threads (utilizavel para paralelismo) | `src/worker_thread.h/.cpp` -- `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` encapsulamento de threads de plataforma |

**Conclusao do estado atual**: 1 Apenas 1 espelho e configuravel apenas por variavel de ambiente, sem configuracao no jogo; 2 Download e **conexao unica, serial** (POST de todos os arquivos de uma vez); 3 Descompressao executada de forma síncrona na thread principal. -> Pontos de modificacao claros para multithread/multiespelho.

### 4.2 Limite de Jogadores Online / Empresas no Servidor

| Ponto de Interesse | Local |
|---|---|
| Constante de limite de clientes | `src/network/network_type.h:21` -- `static const uint MAX_CLIENTS = 255;` |
| Pool de clientes | Mesmo arquivo `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` e `uint32_t` |
| Pool de IDs de empresa | `src/company_type.h` -- `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` -> `MAX_COMPANIES = CompanyID::End().base() = 15`; empresas ficticias ocupam 253/254/255 |
| Mascara de empresa | Mesmo arquivo `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 bits, so pode rastrear 16 empresas) |
| Verificacao de aceitacao do servidor | `src/network/network_server.cpp:360` -- `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| Item de configuracao de cliente | `src/table/settings/network_settings.ini:231/241` -- `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); estrutura em `src/settings_type.h` `NetworkSettings` |
| **Largura de bits do protocolo (restricao rigida)** | `src/network/core/network_game_info.cpp` -- `companies_max` e `clients_max` sao enviados como **`Send_uint8`/`Recv_uint8`** (linhas 251-296, 422-432) |
| Exibicao da lista de servidores | `src/network/network_gui.cpp:519` etc. |

**Conclusao do estado atual**:
- **Limite de clientes = 255 ja e o limite do protocolo** (campo uint8 + compatibilidade com Game Coordinator/navegador de servidores). Para ultrapassar, seria necessario alterar campos relacionados em `network_game_info` para uint16 (cadeia completa cliente-servidor + transmissao UDP + protocolo do Game Coordinator), uma alteracao de protocolo externo.
- **Limite de empresas = 15** (End=0xF em `CompanyIDTag`). Pode ser aumentado com seguranca para **252** (End=0xFC): subjacente ainda e uint8, largura de bytes do arquivo inalterada (compativel com arquivos antigos), necessario tambem ajustar `CompanyMask` (uint16->uint32) e suposicoes de interface/laco.

### 4.3 Agrupamento de Veiculos

| Ponto de Interesse | Local |
|---|---|
| Estrutura de dados de grupo | `src/group.h` -- `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| Comandos de grupo | `src/group_cmd.cpp` -- `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (adiciona veiculos de ordens compartilhadas a grupo existente, `AddVehicleToGroup` proximo a linha 718) |
| Registro de comando | `src/group_cmd.h:27-35` -- `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; enumeracao em `src/command_type.h` `enum class Commands` (a partir da linha 492) |
| GUI de grupo | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| Veiculo <-> grupo | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| Ordem/programacao compartilhada | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, vinculado a order list) |
| Estatisticas de grupo | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` etc. (group.h:125-128) |

**Conclusao do estado atual**: Ja existem `CmdAddSharedVehicleGroup` (adiciona veiculos de ordens compartilhadas de um veiculo a um grupo) e `CmdCreateGroupFromList` (cria grupo a partir de lista), mas **faltava a logica completa de "criar grupo automaticamente por ordens compartilhadas e adicionar veiculos"**. O novo comando `AutoGroupSharedOrders` (percorre todos os veiculos principais da empresa -> agrega por `OrderList` -> cria grupo automaticamente e adiciona veiculos) tem caminho de implementacao claro.

### 4.4 Dica de Construcao (dica de preco ao passar o mouse sobre trilhos, etc.)

| Ponto de Interesse | Local |
|---|---|
| GUI/logica de construcao de trilhos | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; estradas `road_gui.cpp/road_cmd.cpp` |
| Estimativa de custo | `DoCommand` em cada `*_cmd.cpp` retorna `CommandCost`; GUI pode usar modo `DC_QUERY_COST` para consultar preco |
| Tile sob o mouse | `src/viewport_func.h:36` -- `GetTileBelowCursor()`; `_cursor.pos` (coordenadas de tela); `src/viewport.cpp:1056` |
| Mecanismo de dica de texto existente | `src/texteff.hpp` -- `AddTextEffect(msg, x, y, duration, mode, ...)` (texto flutuante em coordenadas mundiais), `UpdateTextEffect`; `src/texteff.cpp` |
| UX de dica de construcao existente | Barra de ferramentas de trilhos `OnPlaceDrag` tem area selecionada e custo acumulado durante arrasto (`_thd` tilehighlight, `src/tilehighlight_func.h`); barra de status `statusbar_gui.cpp` pode exibir custo da ferramenta |
| Ponto de atualizacao por quadro | `OnMouseLoop` de cada janela de ferramentas / `HandleMouseEvents` em viewport.cpp (linhas 5422/5733) |

**Conclusao do estado atual**: Nao existe "dica de preco ao passar o mouse". Pode ser implementado com `AddTextEffect` ancorado no tile sob o mouse (seguindo o cursor), ou desenhando dica em coordenadas de tela; custo pode usar `DC_QUERY_COST` para consulta de tile unico da ferramenta atual + acumulado de selecao por arrasto.

### 4.5 Interface de IA (NoAI mantido + IA de percepcao global)

| Ponto de Interesse | Local |
|---|---|
| Framework NoAI | `src/ai/` -- `ai_core.cpp` (loop principal AICore), `ai_instance.cpp` (AIInstance/VM Squirrel), `ai_scanner.cpp` (scan do diretorio `ai/`), `ai_gui.cpp` (selecao/configuracao), `ai_config.cpp` |
| Framework GameScript | `src/game/` -- `game_core.cpp`, `game_instance.cpp` etc. (GS e "modo divindade", permissoes maiores que AI) |
| Classes de API de script | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin`, etc., total de 60+ classes) |
| Registro automatico de API | `src/script/api/CMakeLists.txt` -- `file(GLOB script_*.hpp)` gera automaticamente `ai_*.sq.hpp`/`gs_*.sq.hpp`; `.cpp` precisa ser adicionado a lista de fontes (a partir da linha 235) |
| Macro de vinculacao Squirrel | `src/script/squirrel_class.hpp` -- `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` e a vinculacao do controlador AI |
| Agendamento de instancia | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (suspensao/retomada), eventos `script_event*` |
| Controle de acesso da empresa | `src/script/api/script_object.hpp:318` -- `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) diferencia modo divindade GS; validacao de parametros de API de empresa `ResolveCompanyID`, `EnforceCompanyModeValid` |
| Configuracao de script | `src/table/settings/script_settings.ini` (`game.script.*`); configuracao de instancia AI `ai_config.cpp` |

**Conclusao do estado atual**: Na API de script moderna, algumas informacoes de concorrentes (como `GetBankBalance`) ja nao sao restritas, mas **nao existe API de agregacao voltada para percepcao global** (sem entrada unificada para "enumerar todas as empresas/economia global/estatisticas globais do mapa"), nem ponto de acesso de "IA global" controlado por alternador. Adicionar uma nova classe de API `ScriptGlobal` (registro automatico GLOB) + uma configuracao `game.script` de "permitir percepcao global de IA" e suficiente para implementar, GS sempre disponivel (modo divindade), AI controlada pelo alternador -- ou seja, "manter NoAI, adicionar IA de percepcao global, com controle de acesso".

---

## 5. Visao Geral dos Riscos de Modificacao

| Funcionalidade | Principais Arquivos Modificados | Risco |
|---|---|---|
| F1 Download multi-fonte/multithread | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | Medio (maquina de estados de callback de rede requer cuidado) |
| F2 Extensao de limites | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (opcional uint16) | Baixo-Medio (empresa 252 baixo risco; cliente >255 requer alteracao de protocolo) |
| F3 Agrupamento automatico de veiculos | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | Baixo |
| F4 Dica de construcao | Novo `src/construction_cost_tip.h/.cpp`, ganchos em `src/rail_gui.cpp`/`road_gui.cpp`, `src/lang/english.txt` | Baixo-Medio |
| F5 IA de percepcao global | Novo `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, IA de exemplo `bin/ai/GlobalAI/` | Baixo (registro automatico do framework) |

> Nota: Todas as alteracoes sao baseadas na area de trabalho local do branch `jgrpp`, nao compiladas/verificadas (esta maquina nao possui cadeia de ferramentas de construcao disponivel); todos os patches podem ser revisados com `git diff` e aplicados com `git apply`.