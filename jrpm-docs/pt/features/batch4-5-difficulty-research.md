# Quarto Lote / Quinto Lote / Comandos do Servidor CM -- Relatorio de Dificuldade de Portabilidade

> Objeto de pesquisa: Leitura aprofundada do codigo fonte do citymania-org/cmclient (branch vanilla 15.3)
> Data da pesquisa: 2026-08-14
> Conclusao: A dificuldade dos tres blocos de trabalho varia muito; abaixo, a avaliacao item por item e a ordem recomendada.

---

## I. Quarto Lote: Zoneamento Urbano (incluindo arquivo growth_tiles)

### Componentes Funcionais

| Componente | Arquivo | Tamanho | Descricao |
|---|---|---|---|
| Enumeracao de modos de zoneamento | cm_zoning.hpp | 37 linhas | 12 modos de avaliacao (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| Logica de avaliacao | cm_zoning_cmd.cpp | 413 linhas | Uma funcao de consulta por modo (retorna SpriteID de paleta), incluindo algoritmo de raio TownZone/Tz, verificacao de cobertura StationFinder, verificacao de edificios/industrias nao atendidas |
| GUI da barra de ferramentas de zoneamento | cm_zoning_gui.cpp | 204 linhas | Janela de selecao de modo de zoneamento interna/externa (estado global `_zoning.inner/outer`) |
| Arquivo growth_tiles | cm_saveload.cpp/.hpp | 90 linhas | Campos de extensao Town `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), salvos com manipulador vanilla SaveLoad |
| Extensao de dados | extensions/cmext_town.hpp | ~80 linhas | `ext::Town`: growth_tiles x2 + **muitos campos de modo de jogo do servidor CM** (CBTownInfo estatisticas de carga, rastreamento de publicidade/fundos, contadores de crescimento urbano hs/cs/hr) |
| Rotacao mensal + acionamento | cm_game.cpp / town_cmd.cpp | -- | NewMonth rotaciona growth_tiles; registro de estado em construcao/demolicao/reconstrucao de edificios |

### Avaliacao Item por Item dos Pontos Dificeis da Portabilidade

| Ponto Dificil | Gravidade | Descricao |
|---|---|---|
| **Reescrita do sistema de arquivo** |  Medio | growth_tiles usa `DefaultSaveLoadHandler` vanilla + `SlSetStructListLength`/`SlObject`, jrpm usa o novo sistema `sl/` (SlTableHeader/SlObjectSaveFiltered). Necessario reescrever com NSL/SLE + **controle de recurso XSLF** do jrpm (novo `XSLFI_TOWN_GROWTH_TILES`, versao 1), arquivos antigos nao sao afetados |
| **Extensao da estrutura Town** |  Baixo | Apenas pegar os dois mapas growth_tiles (**pular** CBTownInfo/publicidade/fundos -- sao exclusivos do modo CityBuilder do servidor CM); adicionar campos ao Town do jrpm + montar tabela de arquivo town_sl.cpp |
| **Logica de avaliacao** |  Baixo-Medio | Maioria e consulta pura (GetTileType/StationFinder/cache Town/verificacao de edificios), API jrpm existe; algoritmo de raio TownZone (`squared_town_zone_radius`) precisa verificar nomes de campos no jrpm |
| **Pipeline de renderizacao** |  Medio-Alto | `DrawTileZoning` precisa ser inserido no pipeline de renderizacao viewport -- **compartilha mecanismo de renderizacao TileHighlight com o destaque do terceiro lote**. Sem o pipeline de destaque, seria necessario montar um separadamente (recomendado **fazer destaque primeiro, depois zoneamento**) |
| **Recursos de sprite** |  Medio | 12 paletas usam **sprites personalizados** `CM_SPR_PALETTE_ZONING_*`, jrpm nao tem -> necessario substituir por sprites de paleta existentes ou adicionar novos recursos |
| **Ganchos de eventos** |  Medio | O registro growth_tiles depende do **barramento de eventos** do cmclient (event::HouseBuilt/HouseCleared/..., Emit de cm_main.cpp). jrpm nao tem esse mecanismo -> adicionar ganchos diretamente em town_cmd.cpp nos pontos de construcao/demolicao de edificios + rotacao NewMonth (IntervalTimer) |

### Conclusao de Dificuldade:  Medio-Alto (aproximadamente 1.5-2 rodadas dedicadas, cada rodada 4-6 horas)

- **Pre-requisito**: Fortemente recomendado concluir o **sistema de destaque** do terceiro lote primeiro (pipeline de renderizacao compartilhado)
- Se fizer apenas "12 tipos de coloracao de zoneamento, sem arquivo growth_tiles": dificuldade reduz para  Medio (elimina extensao de arquivo, aproximadamente 1 rodada)

---

## II. Quinto Lote: Reproducao de Comandos + Exportacao/Gravacao

### 2.1 Reproducao de Comandos (cm_command_log + cm_commands + generated)

| Componente | Tamanho | Descricao |
|---|---|---|
| Camada de objeto de comando | cm_command_type.hpp + generated/cm_gen_commands (2251+1418 linhas) | Cada comando vanilla encapsulado em objeto programavel (as_company/with_callback/set_auto), incluindo **serializacao comando->fluxo de bits** |
| Carregamento de log de comandos | cm_command_log.cpp (203 linhas) | Descompressao lzma + analise de fluxo de bits BitOStream -> fila `_fake_commands` (contador de tick/resultado esperado/semente aleatoria/CommandPacket) |
| Execucao de comandos | ExecuteFakeCommands | Executa em ordem de tick counter: `ExecuteCommand(&cp)` (API interna vanilla) + **verificacao de semente aleatoria/resultado** (anti-cheat), encaminha para todos os clientes em rede |

**Diferencas Chave (determinam a dificuldade)**:

| cmclient | jrpm | Impacto |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | Nao existe, o nucleo do comando e `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | Necessario escrever camada de conversao CommandPacket -> payload de DoCommandPInternal  |
| Campos de CommandPacket (vanilla) | CommandPacket existe mas estrutura e diferente (`GeneralCommandPacket<DynBaseCommandContainer>`) | Necessario adaptar mapeamento de campos  |
| `GetCommandName` |  Existe (command_func.h:166) |  |
| `outgoing_queue` (encaminhamento em rede) | Camada de rede jrpm e diferente (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | Reproducao em rede requer reescrita da logica de encaminhamento  |
| Gravador (como gerar arquivo .cmd) | **O gancho de gravacao do cmclient depende da interceptacao de post() da camada de objeto de comando** | Comandos jrpm sao Post templateizado -> necessario adicionar gancho de gravacao no ponto de despacho de comando   Trabalho central |
| Formato de arquivo | Privado (ID de comando vanilla + fluxo de bits) | IDs de comando jrpm sao completamente diferentes do vanilla -> arquivos de reproducao incompativeis, **formato precisa ser redesenhado**  |

### 2.2 Exportacao / Gravacao (cm_export.cpp, 536 linhas)

| Funcionalidade | Descricao | Dificuldade |
|---|---|---|
| ExportOpenttdData | Exportacao JSON de especificacoes de edificios/especificacoes de carga/paletas/informacoes de motores (JsonWriter) |  Baixo-Medio (independente, le estrutura Spec e escreve JSON) |
| ViewportExport / ExportFrameSprites | Exportacao quadro a quadro de sprites da viewport (depende dos vetores internos de renderizacao viewport TileSpriteToDrawVector/ParentSpriteToSortVector) |  Medio-Alto (pipeline viewport jgrpp e diferente, necessario alinhar interfaces de vetor) |

### Conclusao de Dificuldade:  Medio-Alto (aproximadamente 2 rodadas dedicadas)

- **Reproducao de comandos e o maior**: Trabalho central = 1 adicionar **gancho de gravacao** no ponto de despacho de comando (adaptacao ao sistema de comando templateizado jrpm) 2 camada de execucao CommandPacket -> DoCommandPInternal 3 design de novo formato de arquivo. **Recomendado portar a camada de objeto de comando primeiro** (fundacao do blueprint do terceiro lote, tambem base da reproducao)
- Exportacao: Exportacao JSON de dados pode ser feita independentemente primeiro (baixo-medio); gravacao de quadros depende do alinhamento viewport (mesmo lote do destaque)

---

## III. Comandos do Servidor CM (cm_console_cmds.cpp, 289 linhas)

### Lista de Comandos e Dificuldade Item por Item

| Comando | Funcionalidade | Dificuldade para jrpm | Notas |
|---|---|---|---|
| `cmgamespeed [n]` | Alterar velocidade do jogo |  **Muito baixa** (~20 linhas) | jrpm ja tem `_game_speed` global (gfx.cpp:52), so falta o invólucro do comando |
| `cmstep [n]` | Avancar n ticks |  **jrpm ja tem** | Comando `step` (ConStepGame) tem funcionalidade identica, pular |
| `cmexport` | Exportar openttd.json |  Baixo-Medio | Depende de ExportOpenttdData (veja quinto lote) |
| `cmtreemap <file>` | Plantar arvores por heightmap |  Medio | Ler heightmap + comando de plantar arvores, independente |
| `cmreset_town_growth` | Limpar registros de crescimento urbano |  Baixo | Depende de campos de zoneamento urbano |
| `cmload_commands` | Carregar reproducao de comandos |  Medio-Alto | Depende da infraestrutura de reproducao de comandos |
| `cmstart_record` / `cmstop_record` | Gravar quadros |  Medio-Alto | Depende da infraestrutura de gravacao |
| `cmgamestats` | Estatisticas da sessao de jogo |  Baixo | Independente |
| `cmgfxdebug` | Depuracao grafica |  Baixo | Independente |

### Conclusao de Dificuldade:  Geralmente baixa (0.5-2 horas por comando)

**Ordem recomendada**: `cmgamespeed` (10 minutos) -> `cmgamestats`/`cmgfxdebug` (~1h cada) -> `cmexport` (junto com exportacao do quinto lote) -> `cmtreemap` (~2h) -> `cmreset_town_growth` (junto com quarto lote) -> `cmload_commands`/`cmstart_record` (junto com reproducao/gravacao do quinto lote).

---

## IV. Rota Geral Recomendada

```
1 Sistema de destaque (nucleo do terceiro lote, ~2-3 rodadas)      <- Base de renderizacao para blueprint/zoneamento
2 Camada de objeto de comando (base do blueprint do terceiro lote + reproducao do quinto lote, ~1-2 rodadas)
3 Blueprint (~1 rodada)
4 Zoneamento urbano (~1.5 rodadas, depende do pipeline de renderizacao 1; growth_tiles com controle XSLF)
5 Reproducao de comandos + exportacao (~2 rodadas, depende de 2; exportacao JSON pode ser feita independentemente antes)
6 Comandos do servidor CM (inseridos entre os lotes, gamespeed/step podem ser feitos imediatamente)
```

## V. Itens de Baixo Custo Implementaveis Imediatamente (sem depender de grandes projetos)

1. Comando `cmgamespeed` (~20 linhas, altera `_game_speed`)
2. `step` ja existe -> nao necessario fazer
3. `cmgamestats` / `cmgfxdebug` (~1h cada, independente)
4. Exportacao de dados JSON de `cmexport` (~2h, independente)
5. `cmtreemap` (~2h, independente)

Estes 5 itens totalizam aproximadamente **meio dia de trabalho**, podem ser implementados como "lote zero" antes dos lotes tres/quatro/cinco, sem conflito.