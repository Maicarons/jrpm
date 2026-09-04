---
title: Historique des versions
---

# Historique des versions

## jrpm-0.1.0 (2026-08-14) Portage cmclient termine

Base sur le premier 0.1.0, portage des cinq lots de fonctionnalites cmclient (modded + cmclient complet) :

### Nouvelles fonctionnalites (portage cmclient / modded)

- **Premier lot modded** : Historique des trajets des vehicules (`b89f93f9`) + Vitesse de roulage des avions reglable (`b89f93f9`) ;
- **Deuxieme lot cmclient** : Signets de position / Details de fret / Observation (`089480b3`) ;
- **Lot zero cmclient** : Commandes console `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` (`1fd94d12`) ;
- **Troisieme lot cmclient 1 - Surbrillance** : Apercu de construction par objet (gares/voies/entrepots/aeroports), integration dans le pipeline de rendu viewport et les outils de construction (6 commits, `d97aa38a` → `1957bf45`) ;
- **Troisieme lot cmclient 3 - Plans** : Selection copie/rotation/16 emplacements/reconstruction (`8e08ca6b`) ;
- **Quatrieme lot cmclient 4 - Zonage urbain** : Zonage Tz + coloration growth_tiles + bloc d'archive GRWT (`9e3f95a2`) ;
- **Cinquieme lot cmclient 5 - Rejeu de commandes** : `cmdrecord` / `cmdreplay` (`f113acce28`) ;
- **Correction des conventions de parametres console** : Toutes les commandes jrpm utilisent desormais argv[1] comme premier parametre (argv[0] etant le nom de la commande).

### Conclusion architecturale

- **La couche d'objet de commande 2 n'a pas ete portee** : Surbrillance/Plans/Rejeu utilisent respectivement `CMD_ERROR`, des fermetures de commande et la serialisation native de commandes jrpm pour contourner le besoin, sans les 2251 lignes de code genere de cmclient.

### Compatibilite des archives

- Les donnees growth_tiles sont stockees dans un bloc `GRWT` independant, les anciennes archives (sans ce bloc) sont entierement compatibles ;
- Les fichiers d'enregistrement de commandes (`.jrcm`) sont un format prive jrpm, la compatibilite entre versions n'est pas garantie.

## jrpm-0.1.0 (2026-08-14)

Premiere version jrpm, basee sur jgrpp 0.73.1 + pulseXLB px-patch fusionne, avec portage de fonctionnalites modded / cmclient.

### Changements

- **Fusion de pulseXLB px-patch (152 commits)** :
  - Decouplage de locomotives : ordres d'attelage/decouplage, transfert de consignes, limite de longueur/vitesse d'attelage, double locomotive, attelage NewGRF, pathfinding d'attelage, ordonnancement independant apres decouplage ;
  - Aeroports modulaires (multitile-airport) : systeme de type air, gestion aerienne PBS, pathfinding aerien YAPF, modification de disposition d'aeroport (`allow_modify_airports`) ;
  - Nouveaux versions d'archive `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`.
- **Renommage de version** : `openttd-jrpm` / `jrpm-0.1.0` (nom de l'executable et chaine de revision).
- **Compatibilite multi-versions du serveur** : Le serveur jrpm accepte les clients jrpm / jgrpp original / pulseXLB.
- **Fonctionnalites propres a jrpm** :
  - Telechargement de ressources multi-miroirs + parallele au niveau fichier (nombre de connexions configurable) ;
  - Groupement automatique de vehicules par ordres partages (bouton de fenetre + commande `autogroup`) ;
  - IA consciente de la partie (API `AIGlobal` + exemple GlobalAI, controle par parametre) ;
  - Infobulle de cout de construction (retiree sur demande, commit `96ebfb75`).
- **Portage modded (premier lot)** :
  - Historique des trajets des vehicules (10 derniers trajets : profit/taux d'occupation/duree, bouton History dans la fenetre de details du vehicule) ;
  - Vitesse de roulage des avions reglable (`vehicle.plane_taxi_speed`, controle par fonctionnalite XSLF, compatible avec les anciennes archives).
- **Inspiration cmclient (deuxieme lot)** :
  - Signets de position (`savelocation` / `gotolocation`, 9 emplacements) ;
  - Fenetre de details de fret de l'entreprise (`company_cargo`) ;
  - Aide a l'observation (`watch <company_id>`).

### Compatibilite des archives

- Herite des conventions jgrpp : peut charger les archives trunk (jusqu'a la version fusionnee la plus recente) ;
- Les archives jrpm (contenant les donnees d'aeroports multi-tuiles/ordres de decouplage) **ne garantissent pas** la lecture croisee avec les anciennes archives jgrpp ;
- Le numero de version d'archive `SAVEGAME_VERSION` est coherent avec pulseXLB (`SLV_CUSTOM_SUBSIDY_DURATION`) ;
- Les nouvelles fonctionnalites utilisent toutes le controle de fonctionnalites etendues XSLF ou NOSAVE, sans casser les anciennes archives.

## Versions amont

- **jgrpp 0.73.1** : Base de ce projet (noyau OpenTTD 16.0 + toutes les fonctionnalites JGR).
- **pulseXLB px-patch 2608.3** : Source du decouplage de locomotives et des aeroports modulaires.
- **embeddedt/OpenTTD-modded (epoque 0.59.1)** : Source de l'historique des trajets et de la vitesse de roulage.
- **citymania-org/cmclient (vanilla 15.3)** : Source d'inspiration pour les signets de position/details de fret/observation.

## Feuille de route

- [x] Verification de construction reelle et correction des premieres erreurs de compilation
- [x] Historique des trajets + Vitesse de roulage (premier lot modded)
- [x] Signets de position + Details de fret + Observation (deuxieme lot cmclient)
- [ ] Systeme de surbrillance + Systeme de plans (troisieme lot cmclient, voir [feuille de route](../features/highlight-blueprint-plan))
- [ ] Pool de threads HTTP + Telechargement par plage Range (acceleration des gros fichiers uniques)
- [ ] Optimisation de la compression de l'envoi de carte
- [ ] Statistiques de revenus par marchandise (necessite extension du format d'archive)
- [ ] (Long terme) Integration d'un runtime serveur Rust (reference Openttd-Cluster)