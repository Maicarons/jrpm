---
title: Vitesse de roulage des avions reglable
---

# Vitesse de roulage des avions reglable

Provenant de **embeddedt/OpenTTD-modded** (premier lot de portage).

## Fonctionnalite

Nouveau parametre de jeu **`vehicle.plane_taxi_speed` (Vitesse de roulage des avions)**, permettant de regler independamment la limitation de vitesse des avions au sol sur les aeroports :

- Plage **1–8**, defaut **4** (= limitation de roulage originale de 50 unites)
- Plus la valeur est elevee, plus le roulage est rapide (1 → 12.5, 8 → 100)
- **Independant** des autres parametres de vitesse des avions (`plane_speed` multiplicateur global), sans interference

## Emplacement du parametre

Dans le jeu **Parametres → Parametres experts → Vehicules (Vehicles)**, ou dans le fichier de configuration `openttd.cfg` :

```ini
[vehicle]
plane_taxi_speed = 4
```

## Notes d'implementation

- Le parametre est marque `SettingFlag::NoNetwork` (autorite du serveur, decide par l'hote en multijoueur)
- Utilise le **controle de fonctionnalites etendues XSLF** (`XSLFI_PLANE_TAXI_SPEED`, version 1) : lors du chargement d'anciennes archives, cette fonctionnalite n'existe pas → le parametre reste a la valeur par defaut 4, **ne casse aucune ancienne archive**
- La limitation de roulage s'applique lorsque l'avion est a l'etat `AS_RUNNING` (roulage au sol), prend `min(limite du modele, limite de roulage)`

## Fichiers concernes

- `src/aircraft_cmd.cpp` (nouvelle constante `SPEED_LIMIT_TAXI` + logique de limitation de roulage)
- `src/table/settings/game_settings.ini` (definition du parametre)
- `src/settings_type.h` (`VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (enregistrement de la fonctionnalite XSLF)
- `src/lang/english.txt` (`STR_CONFIG_SETTING_PLANE_TAXI_SPEED*`)