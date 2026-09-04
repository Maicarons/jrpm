---
title: Velocidad de taxi de aviones ajustable (Plane Taxi Speed)
---

# Velocidad de taxi de aviones ajustable

Originado de **embeddedt/OpenTTD-modded** (primera tanda de portabilidad).

## Funcion

Nueva configuracion del juego **`vehicle.plane_taxi_speed` (Velocidad de taxi de aviones)**, que permite ajustar de forma independiente el limite de velocidad de los aviones al rodar en tierra en el aeropuerto:

- Rango **1–8**, predeterminado **4** (= limite de taxi de 50 unidades de la version original)
- Cuanto mayor es el valor, mas rapido rueda (1 → 12.5, 8 → 100)
- Es **independiente** de otros ajustes de velocidad de aviones (`plane_speed` multiplicador global), no interfieren entre si

## Ubicacion de la configuracion

En el juego: **Configuracion → Configuracion de experto → Vehiculos (Vehicles)**, o en el archivo de configuracion `openttd.cfg`:

```ini
[vehicle]
plane_taxi_speed = 4
```

## Notas de implementacion

- La configuracion tiene `SettingFlag::NoNetwork` (autoridad del servidor, en multijugador la decide el anfitrion)
- Usa **control de caracteristica extendida XSLF** (`XSLFI_PLANE_TAXI_SPEED`, version 1): al cargar archivos antiguos, esta caracteristica no existe → la configuracion mantiene el valor predeterminado 4, **no rompe ningun archivo antiguo**
- El limite de taxi se aplica cuando el avion esta en estado `AS_RUNNING` (rodando en tierra), tomando `min(limite del modelo, limite de taxi)`

## Archivos involucrados

- `src/aircraft_cmd.cpp` (nueva constante `SPEED_LIMIT_TAXI` + logica de limite de taxi)
- `src/table/settings/game_settings.ini` (definicion de la configuracion)
- `src/settings_type.h` (`VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (registro de caracteristica XSLF)
- `src/lang/english.txt` (STR_CONFIG_SETTING_PLANE_TAXI_SPEED*)