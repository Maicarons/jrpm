---
title: Melhorias de UI Online (Marcadores de Posicao / Detalhes de Carga / Observacao)
---

# Melhorias de UI Online

Segundo lote portado do **citymania-org/cmclient**, implementado como **comandos de console** (sem depender da infraestrutura de teclas de atalho/barra de ferramentas do cmclient, estavel e scriptavel).

## Marcadores de Posicao (Viewport Locations)

Salva e restaura a posicao e zoom da viewport principal (9 slots), util para navegar rapidamente entre suas fabricas, estacoes e areas de oponentes durante o jogo online.

```
savelocation <1-9>    # Salva a posicao e zoom atuais da viewport
gotolocation <1-9>    # Vai para a posicao salva
```

## Detalhes de Carga da Empresa (Company Cargo Details)

Janela de estatisticas que lista o **volume transportado + receita** da empresa por tipo de carga, com alternancia entre **Total / Ultimo mes**.

```
company_cargo <company_id>    # Abre a janela de detalhes de carga
```

- A janela lista o volume e receita de cada tipo de carga padrao, com total na parte inferior (identico ao cmclient)
- Clique no cabecalho "Cargo" para alternar entre Total/Ultimo mes
- A receita por carga e rastreada via `CompanyEconomyEntry::cargo_income` (extensao de arquivo `XSLFI_COMPANY_CARGO_INCOME`), sem perdas ao salvar/carregar

## Auxilio de Observacao (Watch)

Permite que observadores localizem rapidamente a area de construcao de uma empresa (vai para as coordenadas da ultima construcao dessa empresa).

```
watch <company_id>    # A viewport vai para a localizacao da empresa
```

## Itens Ignorados e Justificativa

| Recurso cmclient | Tratamento no jrpm |
|---|---|
| Sobreposicao de lista de jogadores (cm_client_list_gui) | jgrpp **ja possui** janela de Jogadores Online (`NetworkClientList`), sem necessidade de reimplementacao |
| Dicas de detalhes do solo (cm_tooltips) | O LandInfoWindow do jrpm **ja cobre** a exibicao de detalhes de edificios/industrias/estacoes |

## Arquivos Envolvidos

- `src/jrpm_locations.cpp/.h` (novos: marcadores de posicao + company_cargo + comando de console watch)
- `src/jrpm_cargo_table.cpp/.h` (novos: janela de detalhes de carga)
- `src/window_type.h` (novo `WindowClass::CompanyCargos`)
- `src/console_cmds.cpp` (registro de comandos)
- `src/lang/english.txt` (strings STR_JRPM_CARGOS_*)