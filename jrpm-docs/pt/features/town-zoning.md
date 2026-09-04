---
title: Zoneamento Urbano (Town Zoning)
---

# Zoneamento Urbano (Town Zoning)

Portabilidade do zoneamento urbano cmclient (quarto lote, commit `9e3f95a2`). A base jrpm (pulsexlb) ja possui um sistema de zoning (barra de ferramentas + pipeline de renderizacao + menu), e este lote adiciona, sobre ele, o modo de avaliacao exclusivo do cmclient e o **arquivo growth_tiles**.

## Novos Modos (menu suspenso da barra de ferramentas de zoning)

| Modo | Descricao | Coloracao |
|---|---|---|
| **Town zones (Tz)** | Zonas concentricas urbanas, reutiliza `squared_town_zone_radius` | Borda Tz0=azul claro / Tz1=vermelho / Tz2=amarelo / Tz3=verde / Tz4 centro=branco |
| **Town growth tiles** | Trajetoria de construcao e demolicao de edificios do mes atual/anterior | Nova casa=verde / Demolicao=azul claro / Reconstrucao=branco / Pular crescimento=laranja / Pular construcao=amarelo / Demolicao pelo servidor=vermelho |

## Camada de Dados growth_tiles (`cm_town_growth.cpp/.h`)

- Dois mapas mensais rotativos: `TileIndex -> TownGrowthTileState` (mes atual / mes anterior)
- Ganchos de eventos:
  - `BuildTownHouse` -> `NEW_HOUSE` (se era demolicao no mes anterior, atualiza para `RH_REBUILT`)
  - `ClearTownHouse` -> `RH_REMOVED`
  - `TownsMonthlyLoop` -> rotacao mensal (mes anterior = mes atual, mes atual e limpo)
- **Persistencia em arquivo**: Novo chunk `GRWT` no savegame (`misc_sl.cpp`), serializado como lista de pares `{tile, state}`; arquivos antigos sem este chunk carregam com compatibilidade total (verificado em ciclo de salvamento/leitura).

## Como Abrir

Menu da barra de ferramentas -> Mapa (Zoning) -> abrir barra de ferramentas de zoning, selecionar modo de avaliacao nos menus suspensos interno e externo.

## Nota de Adaptacao

O modo exclusivo do servidor CityBuilder do cmclient (zona de aceitacao CB / limite urbano CB) e os campos de extensao `ext::Town` (publicidade, fundos, estatisticas de carga) sao especificos do modo de jogo do servidor e nao foram portados.