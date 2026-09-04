---
title: Velocidade de Taxi de Aeronaves Ajustavel (Plane Taxi Speed)
---

# Velocidade de Taxi de Aeronaves Ajustavel

Originado do **embeddedt/OpenTTD-modded** (primeiro lote de portabilidade).

## Funcionalidade

Nova configuracao do jogo **`vehicle.plane_taxi_speed` (Velocidade de taxi de aeronaves)**, que permite ajustar independentemente o limite de velocidade das aeronaves ao taxiar no solo do aeroporto:

- Faixa **1-8**, padrao **4** (= limite de taxi original de 50 unidades)
- Quanto maior o valor, mais rapido o taxi (1 -> 12,5, 8 -> 100)
- Funciona **independentemente** de outras configuracoes de velocidade de aeronaves (`plane_speed` multiplicador global), sem interferencia mutua

## Local da Configuracao

No jogo: **Configuracoes -> Configuracoes Avancadas -> Veiculos (Vehicles)**, ou no arquivo de configuracao `openttd.cfg`:

```ini
[vehicle]
plane_taxi_speed = 4
```

## Notas de Implementacao

- Configuracao com `SettingFlag::NoNetwork` (autoridade do servidor, definida pelo host em jogos online)
- Usa **controle de recurso XSLF** (`XSLFI_PLANE_TAXI_SPEED`, versao 1): ao carregar arquivos antigos, esse recurso nao existe -> configuracao mantem o valor padrao 4, **sem quebrar nenhum arquivo antigo**
- O limite de taxi e aplicado quando a aeronave esta no estado `AS_RUNNING` (taxiando no solo), usando `min(limite do modelo, limite de taxi)`

## Arquivos Envolvidos

- `src/aircraft_cmd.cpp` (nova constante `SPEED_LIMIT_TAXI` + logica de limite de taxi)
- `src/table/settings/game_settings.ini` (definicao da configuracao)
- `src/settings_type.h` (campo `VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (registro de recurso XSLF)
- `src/lang/english.txt` (strings STR_CONFIG_SETTING_PLANE_TAXI_SPEED*)