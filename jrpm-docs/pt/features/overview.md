---
title: Visao Geral dos Recursos
---

# Visao Geral dos Recursos

jrpm = jgrpp (todos os recursos) + pulsexlb (desacoplamento de locomotivas + aeroportos modulares) + modded (historico de viagens + velocidade de taxi) + inspirado em cmclient (melhorias de UI online) + recursos exclusivos do jrpm.

## Recursos Exclusivos do jrpm

| # | Recurso | Documentacao | Status |
|---|---|---|---|
| 1 | Download de recursos: Multiplos espelhos + paralelo | [Download de recursos](./01-resource-download) |  Implementado (concorrencia configuravel) |
| 2 | Pesquisa de limites do servidor | [Pesquisa de limites do servidor](./02-server-caps) |  Conclusao de pesquisa (limitacoes estruturais) |
| 3 | Agrupamento automatico de veiculos | [Agrupamento automatico de veiculos](./03-vehicle-autogroup) |  Implementado |
| 4 | IA de percepcao global | [IA de percepcao global](./05-global-ai) |  Implementado |

> Nota: O antigo F4 "Dica de custo de construcao" foi removido conforme solicitado (commit `96ebfb75`).

## Portabilidade modded (Primeiro lote)

| Recurso | Documentacao | Status |
|---|---|---|
| Historico de viagens de veiculos | [Historico de viagens](./triphistory) |  Implementado |
| Velocidade de taxi de aeronaves ajustavel | [Velocidade de taxi](./plane-taxi-speed) |  Implementado |

## Inspirado no cmclient (Segundo ao Quinto lote)

| Recurso | Documentacao | Status |
|---|---|---|
| Marcadores de posicao / Detalhes de carga / Observacao | [Melhorias de UI online](./ui-enhancements) |  Implementado (segundo lote) |
| Sistema de destaque em nivel de objeto | [Destaque + Blueprint](./highlight-blueprint-plan) |  Implementado (terceiro lote, 6 commits) |
| Sistema de blueprint (copiar/rotacionar/slots/reconstruir) | [Destaque + Blueprint](./highlight-blueprint-plan) |  Implementado (terceiro lote) |
| Zoneamento urbano + arquivo growth_tiles | [Zoneamento urbano](./town-zoning) |  Implementado (quarto lote) |
| Gravacao e reproducao de comandos | [Reproducao de comandos](./command-replay) |  Implementado (quinto lote) |

> **2 Camada de objeto de comando** (2251 linhas de codigo gerado do cmclient): A pratica de portabilidade comprovou que pode ser **completamente ignorada** -- destaque usa `CMD_ERROR` para estimativa de custo, blueprint usa fechamentos de comando, reproducao usa serializacao nativa de comandos jrpm, portanto nao foi portada.

## Recursos Mesclados (do pulsexlb)

| Recurso | Documentacao | Descricao |
|---|---|---|
| Desacoplamento de locomotivas (decouple) | [Desacoplamento](./decouple) | Sistema completo de desengate/engate de trens |
| Aeroportos modulares (multitile-airport) | [Aeroportos modulares](./multitile-airport) | Reformulacao do sistema de aeroportos multibloco |

## Heranca Completa

- **Todos os recursos do jgrpp**: Sinais aprimorados (sinais multiplos/sinais programaveis/slots e contadores), scheduled dispatch, tracerestrict, substituicao de modelos, janela de compra separada de locomotivas/vagoes, freio realista, melhorias em estradas de mao unica, seguranca em passagens de nivel, etc.;
- **Kernel OpenTTD 16.0**: Compatibilidade total com ecossistema NewGRF/scripts/arquivos salvos.

## Encontre Configuracoes Rapidamente

| Recurso | Configuracao |
|---|---|
| Concorrencia de download paralelo | `network.content_download_parallel` (1-8) |
| Lista de espelhos de download | `network.content_mirrors` |
| Servidor de conteudo | `network.content_server` |
| Modificacao de layout de aeroporto | `station.allow_modify_airports` |
| Tipo de aeronave padrao | `gui.default_air_type` |
| Alternar percepcao global de IA | `game.script.allow_global_ai_access` |
| Velocidade de taxi de aeronaves | `vehicle.plane_taxi_speed` (1-8, padrao 4) |

## Encontre Comandos de Console Rapidamente

| Recurso | Comando |
|---|---|
| Agrupamento automatico de veiculos | `autogroup train\|road\|ship\|aircraft` |
| Adicionar IA de percepcao global | `start_ai GlobalAI` |
| Marcadores de posicao | `savelocation <1-9>` / `gotolocation <1-9>` |
| Detalhes de carga da empresa | `company_cargo <company_id>` |
| Observar empresa | `watch <company_id>` |
| Copiar/rotacionar/salvar/carregar/reconstruir blueprint | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| Gravar/reproduzir comandos | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| Velocidade/estatisticas/exportar/plantar arvores | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>` |