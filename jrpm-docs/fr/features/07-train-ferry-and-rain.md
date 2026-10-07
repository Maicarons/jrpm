---
title: Ferry de trains et pluie
---

# Ferry de trains et pluie

> Source : pulsexlb/OpenTTD-patches `px-patch` (lot 2026-10, `pxp-2610.1` – `pxp-2610.3`), fusionné dans jrpm via git merge.

Ce lot apporte deux grandes nouveautés : le **ferry de trains** (navires transportant des trains entiers) et le **système de pluie** (avec assombrissement du monde et voile de gouttes).

## Ferry de trains

Au-delà du transport de véhicules routiers (RoRo) existant, où les voitures sont chargées sur les trains, navires et avions, les navires peuvent désormais transporter des **trains entiers**.

### Relation avec le RoRo

| | Transport de véhicules routiers (VEHC) | Transport de trains (RAIL) |
|---|---|---|
| Transporteurs | trains, navires, avions | navires |
| Objet transporté | véhicules routiers | trains (locomotives et voitures) |
| Étiquette de fret | `VEHC` | `RAIL` |
| Emplacement de fret | `NUM_CARGO - 1` | `NUM_CARGO - 2` |

Les deux emplacements de fret se trouvent hors des 64 emplacements NewGRF et n'entrent donc jamais en conflit.

### Chargement

- Un navire acquiert la capacité de transporter des trains entiers en étant **réaffecté manuellement** au fret « Véhicules (train) » ;
- **Chargement par voiture** : un train peut être réparti sur plusieurs calandres au lieu de devoir tenir dans une seule, ce qui supprime le problème des trains longs dépassant la capacité d'une calandre ;
- Le **sélecteur de panneau** des options de transport propose séparément les panneaux de train et de véhicule routier, chaque mode de transport pouvant ainsi déclarer sa destination ;
- La barre d'état ne liste plus le détail des véhicules transportés, ce qui libère de la place pour des informations de marche plus importantes.

### Affichage des capacités

- La **fenêtre d'information du navire** affiche la charge que chaque calandre peut recevoir ;
- La **fenêtre d'information du train** affiche la capacité de chaque voiture.

### Corrections associées

Ce lot corrige également plusieurs défauts de chargement, de déchargement et de réservation de quai :

- les navires jugeaient à tort le type de voie incompatible au déchargement des trains, empêchant ceux-ci de descendre ;
- la réservation de quai n'était pas libérée au chargement d'un train, et des erreurs de structure et de réservation survenaient au déchargement ;
- des réservations de quai périmées empêchaient les trains de se décharger à nouveau ;
- certaines orientations de quai refusaient le déchargement d'un train ;
- la limite de voitures n'était pas appliquée aux véhicules réaffectés au transport.

## Système de pluie

Une simulation météo purement décorative : elle n'affecte que le rendu visuel, sans toucher à la logique de jeu ni aux valeurs économiques.

### Présentation de la météo

- **Périodes de pluie aléatoires** : l'état de la météo est piloté par un générateur pseudo-aléatoire déterministe initialisé avec la graine de génération de carte, de sorte que tous les joueurs et le serveur voient exactement la même météo ;
- **Assombrissement progressif du monde** : le monde s'assombrit progressivement sous la pluie et s'éclaircit quand elle cesse. L'assombrissement passe par `RAIN_SHADE_LEVELS` niveaux et est lu au rendu des fenêtres de vue ;
- **Voile de gouttes** : une superposition de pluie plein écran qui **s'adapte au zoom** et présente une **gigue aléatoire** pour que les gouttes ne ressemblent pas à une texture figée.

### Option et triche

- **Option de difficulté** `difficulty.rain` : détermine à la création d'une partie si l'assombrissement du monde sous la pluie est activé ;
- **Triche bac à sable** « Météo » : un cycle à trois états — automatique / pluie forcée / soleil forcé.

### Fichiers de sauvegarde

L'état de la météo (s'il pleut actuellement, la graine du générateur, le début de la période de pluie en cours) et la triche météo du bac à sable sont **sauvegardés avec la partie** ; après chargement, l'assombrissement passe directement à la météo courante au lieu de réapparaître en fondu.

- Bloc : `WTHR` ;
- Indicateur de fonctionnalité : `XSLFI_WEATHER` ;
- Code : `src/weather.cpp`, `src/weather.h`, `src/sl/weather_sl.cpp`.

## Code associé

- Ferry de trains et capacités : `src/roadveh_transport.cpp`, `src/cargo_type.h` (`CT_RAILVEHICLES`), `src/table/cargo_const.h`
- Effets visuels de la pluie : `src/weather.cpp`, `src/blitter/32bpp_anim.cpp`, `src/blitter/40bpp_anim.cpp`, `src/viewport.cpp`
- Sauvegarde de la météo : `src/sl/weather_sl.cpp`, `src/sl/extended_ver_sl.cpp` (`XSLFI_WEATHER`)
- Réglages et triche : `src/table/settings/difficulty_settings.ini` (`difficulty.rain`), `src/cheat_gui.cpp`, `src/cheat_type.h`
