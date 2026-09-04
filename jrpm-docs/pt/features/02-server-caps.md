---
title: Pesquisa de Limites do Servidor
---

## Conclusao da Pesquisa: Ambos os Limites sao Estruturais

### Limite de Jogadores Online = 255 (Restricao Rigida de Largura de Bits do Protocolo)

| Local | Conteudo |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` sao transmitidos como **`Send_uint8` / `Recv_uint8`** em `SerializeNetworkGameInfo` (linhas 251-296) e desserializacao (linhas 422-432) |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, padrao 25) |
| `src/network/network_server.cpp:360` | Verificacao de aceitacao `_network_clients_connected < MAX_CLIENTS` |

**Conclusao**: 255 ja e o limite do campo de protocolo uint8. Para ultrapassar, seria necessario alterar `clients_max` e campos relacionados para uint16 -- isso mudaria o protocolo online entre cliente e servidor, a transmissao UDP e o protocolo do **Game Coordinator**, sendo uma alteracao de protocolo externo (jogavel dentro do fork com ambos os lados atualizados, mas a compatibilidade com a lista de servidores publicos seria afetada).

### Limite de Empresas = 15 (Restricao Rigida do Formato de Armazenamento de Propriedade de Tiles)

| Local | Conteudo |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` -> `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner`: `SB(_m[tile].m1, 0, 5, owner.base())` -- **propriedade do tile usa apenas 5 bits** (bits baixos de `_m[].m1`) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`, `OWNER_NONE{0x10}`, `OWNER_WATER{0x11}`, `OWNER_DEITY{0x12}`, `OWNER_END{0x13}` compartilham o mesmo espaco de bytes com IDs de empresa |

**Conclusao**: `Owner` e `CompanyID` sao do mesmo tipo, o campo de propriedade do tile tem apenas 5 bits (valores 0-31), dos quais 15-18 sao ocupados por owners especiais. Portanto, o limite real de empresas de 15 e determinado pelo **formato do array de mapa** -- para ultrapassar, seria necessario expandir o armazenamento de propriedade do tile (usar todos os 8 bits de `m1` ou modificar a estrutura `_m`) e migrar as constantes `OWNER_*`, envolvendo **conversao completa de arquivos salvos** e aumento de memoria do mapa, sendo uma grande reforma de formato (razao pela qual o upstream do OpenTTD nao fez isso por anos).

## Opcoes Disponiveis

| Opcao | Alteracao | Compatibilidade | Sugestao |
|---|---|---|---|
| A. Manter atual | Nenhuma | Total compatibilidade de arquivos/protocolo/coordenador |  Recomendado: 255 clientes / 15 empresas ja excede em muito o original para multijogador |
| B. Cliente -> protocolo uint16 | `network_game_info.cpp` serializacao/desserializacao alterar para `Send_uint16/Recv_uint16`; `MAX_CLIENTS` aumentar para 4095; `network_settings.ini` `max_clients` alterar para SLE_UINT16; `settings_type.h` campo alterar para uint16; `console_cmds.cpp:1056` largura de exibicao | Jogavel dentro do fork; Game Coordinator publico/clientes antigos incompativeis | Opcional, se realmente necessario >255 clientes |
| C. Empresa -> reforma de formato de tile | Alargar campo de propriedade do tile + migrar constantes OWNER_* + conversao completa de arquivos salvos | Alteracao de formato de arquivo (conversao unica) | Nao recomendado para implementacao a curto prazo |

## Conteudo Entregue

- Este documento de design (com arquivos/linhas exatas);
- Patch opcional `option-clients-uint16.diff` (alteracoes completas da opcao B, **nao aplicado**, para revisao com `git apply`).

> Conclusao antecipada: **"Expandir" ambos os limites e essencialmente uma atualizacao de formato/protocolo, nao um ajuste de configuracao**; mantendo a compatibilidade de arquivos salvos e rede publica, os limites atuais de 255/15 sao definitivos. Se o custo de compatibilidade for aceitavel, as opcoes B/C podem ser implementadas como acima.