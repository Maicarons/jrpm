---
title: Transporte de veículos rodoviários (RoRo)
---

# Transporte de veículos rodoviários (RoRo: Road-vehicle on Road-vehicle)

> Fonte: pulsexlb/OpenTTD-patches `px-patch` (lote de setembro de 2026), incorporado ao jrpm via git merge.

## Visão geral

O "transporte de veículos rodoviários" permite que trens, navios e aviões **transportem veículos rodoviários diretamente**:

- Os veículos rodoviários não precisam mais dirigir por conta própria: podem "pegar carona" — um transportador os leva a uma estação distante e eles voltam a dirigir na estrada;
- O transportador (trem/navio/avião) ganha a capacidade de levar veículos rodoviários ao ser **reconvertido para a carga "Vehicles (Road)"** (carga dedicada `VEHC`, que ocupa espaço de carga);
- No lado do veículo rodoviário, as opções de ordem "**Aguardar transporte**" e "**Desembarcar aqui**" combinam com as opções "carregar veículos rodoviários" / "descarregar veículos rodoviários" do transportador.

## Como usar

### Lado do transportador (trem/navio/avião)

1. Reconverta o trem num depósito **manualmente** para a carga "Vehicles (Road)" — tornar-se transportador só é possível por reconversão manual; a reconversão por ordem não se aplica;
2. Ative "**Carregar veículos rodoviários**" numa ordem de estação: o trem embarca os veículos que esperam nessa estação;
3. Opções de emparelhamento opcionais:
   - "Aguardar carga" (partir só quando carregado);
   - "Correspondência de destino": embarcar apenas veículos cuja estação de desembarque declarada seja o próximo destino do transportador;
   - "Desembarcar aqui todos os veículos": desembarcar todos, ignorando a estação declarada por cada um.

### Lado do veículo rodoviário

1. Defina "**Aguardar transporte**" numa ordem de estação: o veículo para ali e aguarda um transportador;
2. Defina "**Desembarcar aqui**": o veículo desce do transportador nesta estação;
3. As duas opções são mutuamente exclusivas (uma por ordem);
4. Ao desembarcar, o veículo faz uma passagem de busca de caminho para escolher a melhor plataforma.

## Detalhes e regras

- **Slot de carga dedicado**: o transporte de veículos rodoviários usa o slot de carga 128 (`NUM_CARGO - 1`), fora das 64 slots que um NewGRF pode definir; o total de tipos de carga foi ampliado de 64 para **128**;
- **Detecção de transportador dedicado**: quando todas as partes do veículo estão reconvertidas para "Vehicles (Road)", os botões de ordem mostram por padrão o transporte de veículos rodoviários; veículos com carga normal mostram carga normal;
- **Configuração de partes transportadoras**: `vehicle.rv_transport_carrier_parts` decide quais partes podem levar veículos rodoviários;
- **Carregamento entre empresas**: opcionalmente permitir carregar/descarregar veículos de outras empresas com liquidação automática de taxas;
- **Aviso "transportado por muito tempo"**: um aviso único quando um veículo foi levado por muito tempo;
- **Listas de ordens criadas pelo jogador**: listas compartilhadas/independentes também suportam as marcas de transporte de veículos rodoviários.

## Compatibilidade com jogos salvos

- O estado de espera/transporte e as marcas de ordem são persistidos;
- Jogos salvos antigos (sem a marca XSLFI_CARGO_TYPES_128) são lidos com 64 slots de carga e permanecem compatíveis.

## Comandos de console de depuração (desativados por padrão)

A família de comandos `rvtransport` só é compilada com a opção CMake `RORO_DEBUG_COMMANDS=ON`, para testes de regressão.

## Código relacionado

- Núcleo: `src/roadveh_transport.h`, `src/cargo_type.h` (carga VEHC)
- Ordens: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_base.h` (OrderExtraInfo)
- Carregamento de trens: `src/train_cmd.cpp`, `src/station_cmd.cpp`
- Extensão de carga: `src/sl/station_sl.cpp`, `src/sl/company_sl.cpp` (XSLFI_CARGO_TYPES_128)
