---
title: Aeroports modulaires (multitile-airport)
---

# Aeroports modulaires (multitile-airport)

> Source : Branche de fonctionnalite `jgrpp-multitile-airport` de pulseXLB/OpenTTD-patches, fusionnee dans jrpm via git merge.

## Presentation fonctionnelle

Refonte de l'aeroport multi-tuiles (multitile) modulaire : transforme l'aeroport de « type d'aeroport fixe » en un systeme multi-tuiles **a disposition librement modifiable** :

- **Aeroport multi-tuiles** : L'aeroport est compose de plusieurs tuiles fonctionnelles (piste, voie de circulation, parking, terminal, heliport), pouvant etre combinees en n'importe quelle disposition ;
- **Modification de disposition d'aeroport** : Une fois `station.allow_modify_airports` active, vous pouvez ajouter/supprimer/ajuster des emplacements sur un aeroport existant ;
- **Systeme de type air** : Nouveaux `air.h`/`air_type.h`/`newgrf_airtype.*` -- abstraction des types aeriens (aile fixe/helicoptere, etc.) en un systeme de type air extensible, NewGRF peut definir de nouveaux types aeriens et sprites ;
- **Gestion aerienne PBS** : `pbs_air.*` -- version aerienne de l'occupation de piste/voie de circulation et de la reservation de signaux, supportant le roulage concurrent de plusieurs avions ;
- **Pathfinding aerien YAPF** : La planification de chemin des avions au sol (roulage/attente) et dans les airs suit le systeme YAPF.

## Capacites centrales

| Capacite | Description |
|---|---|
| Modification de disposition d'aeroport | `station.allow_modify_airports` (**active par defaut** ; une fois active, les aeroports existants peuvent etre modifies) |
| Type aerien par defaut | `gui.default_air_type` |
| Sprites d'aeroport multi-tuiles | openttd.grf reconstruit (sprites de type air), correction des sprites transparents |
| Comportement des avions | Occupation de piste, virage au roulage, file d'attente decollage/atterrissage, heliport, rendu des vehicules spatiaux |
| Compatibilite NewGRF | Chargement des sprites airtype, callbacks NewGRF d'aeroport |
| Archive | Version d'archive `SLV_MULTITILE_AIRPORTS` |

## Utilisation

1. Activez `station.allow_modify_airports` dans les parametres du jeu ;
2. Apres avoir construit un aeroport, utilisez l'outil de modification d'aeroport pour ajuster la disposition des pistes/places de stationnement/terminaux ;
3. Choisissez le type aerien par defaut dans `gui.default_air_type` ;
4. Utilisez des NewGRF aeriens pour des types air personnalises.

## Code associe

- Types aeriens : `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- Gestion aerienne : `src/pbs_air.*`
- Commandes avion/aeroport : `src/aircraft_cmd.cpp` (3600 lignes de refonte), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- Pathfinding : `src/pathfinder/yapf` (partie aerienne)
- Archive : `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## Remarque

- Cette fonctionnalite est une refonte majeure du systeme aerien (aircraft_cmd.cpp refait 3600+ lignes), **il est recommande de tester en priorite apres compilation reelle** : achat/decollage/atterrissage des avions, occupation des pistes, GUI d'aeroport, chargement d'archive ;
- La fusion a supprime les anciens types sans reference dans l'espace de travail (`VehicleAirFlags`, `AirportMovingDataFlag`), confirme qu'aucun autre fichier n'y fait reference ;
- Pour les aeroports existants dans les archives, sauvegardez d'abord l'archive avant modification.