---
title: Aeroportos Modulares (multitile-airport)
---

# Aeroportos Modulares (multitile-airport)

> Fonte: Branch de recurso `jgrpp-multitile-airport` do pulsexlb/OpenTTD-patches, incorporado ao jrpm via git merge.

## Introducao a Funcionalidade

Reformulacao de aeroportos multibloco (multitile) modulares: transforma aeroportos de "tipos de aeroporto fixos" em um sistema multibloco com **layout livremente modificavel**:

- **Aeroportos multibloco**: Aeroportos compostos por varios blocos funcionais (pistas, taxiways, pátios, terminais, heliportos), que podem ser combinados em qualquer layout;
- **Modificacao de layout de aeroporto**: Apos ativar `station.allow_modify_airports`, e possivel adicionar/remover/ajustar blocos em aeroportos existentes;
- **Sistema de tipos air**: Novos `air.h`/`air_type.h`/`newgrf_airtype.*` -- abstrai tipos de aeronave (asa fixa/helicoptero, etc.) em um sistema de tipos air extensivel, onde NewGRF pode definir novos tipos de aeronave e sprites;
- **Despacho aereo PBS**: `pbs_air.*` -- versao aerea de ocupacao de pistas/taxiways e reserva de sinais, suportando taxi concorrente de multiplas aeronaves;
- **Navegacao aerea YAPF**: O planejamento de rota de aeronaves no solo (taxi/espera) e no ar usa o sistema YAPF.

## Capacidades Principais

| Capacidade | Descricao |
|---|---|
| Modificacao de layout de aeroporto | `station.allow_modify_airports` (**ativado por padrao**; quando ativado, pode modificar aeroportos existentes) |
| Tipo de aeronave padrao | `gui.default_air_type` |
| Sprites de aeroporto multibloco | openttd.grf reconstruido (sprites de tipo air), correcao de sprites transparentes |
| Comportamento de aeronaves | Ocupacao de pista, taxi, fila de decolagem/pouso, heliporto, renderizacao de aeronaves |
| Compatibilidade NewGRF | Carregamento de sprites airtype, callbacks de aeroporto NewGRF |
| Arquivo | Versao de arquivo `SLV_MULTITILE_AIRPORTS` |

## Como Usar

1. Ative `station.allow_modify_airports` nas configuracoes do jogo;
2. Apos construir um aeroporto, use a ferramenta de modificacao de aeroporto para ajustar o layout de pistas/vagas/terminais;
3. Escolha o tipo de aeronave padrao em `gui.default_air_type`;
4. Use tipos air personalizados com NewGRF de aviacao.

## Codigo Relacionado

- Tipos de aeronave: `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- Despacho aereo: `src/pbs_air.*`
- Comandos de aeronave/aeroporto: `src/aircraft_cmd.cpp` (3600 linhas refatoradas), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- Navegacao: `src/pathfinder/yapf` (parte aerea)
- Arquivo: `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## Atencao

- Este recurso e uma reformulacao em larga escala do sistema de aviacao (aircraft_cmd.cpp refatorado em 3600+ linhas), **recomenda-se testar exaustivamente apos compilacao real**: compra de aeronaves/decolagem/pouso, ocupacao de pista, GUI de aeroporto, carregamento de arquivo;
- Tipos antigos sem referencias na area de trabalho foram removidos durante a mesclagem (`VehicleAirFlags`, `AirportMovingDataFlag`), confirmado que nenhum outro arquivo os referencia;
- Para aeroportos existentes em arquivos salvos que precisam de modificacao, faca backup do arquivo primeiro.