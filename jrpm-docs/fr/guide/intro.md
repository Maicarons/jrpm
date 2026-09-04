---
title: Presentation du projet
---

# Presentation du projet OpenTTD-JRPM

## Qu'est-ce que c'est

**OpenTTD-JRPM (jrpm)** est une branche de developpement secondaire basee sur le [Patchpack de JGR](https://github.com/JGRennison/OpenTTD-patches) (jgrpp), fusionnant les fonctionnalites de « decouplage de locomotives » et d'« aeroports modulaires » de [pulseXLB/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches), tout en ajoutant des fonctionnalites propres a jrpm. Version actuelle : **jrpm-0.1.0**.

Il s'agit d'une toute nouvelle version installable et jouable en multijoueur de maniere independante : le client/serveur jrpm utilise un identifiant de version distinct (`jrpm-0.1.0`) pour la poignee de main, completement isole des versions originales jgrpp et pulseXLB, evitant toute confusion de version.

## Apercu des fonctionnalites

### Provenant de pulseXLB (152 commits, fusionnes via git merge)
- **Decouplage de locomotives (decouple)** : Systeme complet d'attelage/decouplage de trains -- ordres de decouplage, transfert de consignes, limite de longueur/vitesse d'attelage, support de double locomotive, attelage NewGRF, pathfinding d'attelage (YAPF/NPF), deux trains independants apres decouplage ;
- **Aeroports modulaires (multitile-airport)** : Refonte du systeme d'aeroport multi-tuiles -- systeme de type air (`air.h`/`air_type.h`/`newgrf_airtype.*`), gestion aerienne PBS (`pbs_air.*`), pathfinding aerien YAPF, `station.allow_modify_airports` pour modifier la disposition des aeroports, `gui.default_air_type` pour le type aerien par defaut.

### Fonctionnalites propres a jrpm
| Fonctionnalite | Description | Acces |
|---|---|---|
| Telechargement de ressources : multi-miroirs + parallele | Multiples miroirs separes par des virgules, telechargement parallele de fichiers (nombre de connexions configurable), bascule automatique en cas d'echec | Parametres → `network.content_mirrors` / `network.content_download_parallel` |
| Groupement automatique de vehicules | Cree et regroupe automatiquement les vehicules par ordres partages/affectations en un clic, nom de groupe reprenant le nom de la ligne | Bouton de fenetre de groupe / console `autogroup` |
| Infobulle de cout de construction | Affiche en temps reel le cout estime lors de la construction de rails/routes/terrains | Barres d'outils rails/routes/terrains |
| IA consciente de la partie | Conserve NoAI, ajoute l'API globale `AIGlobal` + IA d'exemple GlobalAI, controlee par les parametres de jeu | `game.script.allow_global_ai_access` |
| Compatibilite multi-versions du serveur | Le serveur jrpm accepte simultanement les clients jrpm / jgrpp original / pulseXLB | Verification automatique a l'ajout du serveur |

### Heritage complet
- Toutes les fonctionnalites de jgrpp (signaux ameliores, scheduled dispatch, tracerestrict, remplacement de modeles, etc.) ;
- Toutes les fonctionnalites d'OpenTTD amont et compatibilite avec l'ecosysteme NewGRF/scripts.

## Relations entre les versions

```
                 jgrpp-0.73.1 (ancetre commun)
                 /                 \
 jgrpp branche (63 commits)        pulsexlb px-patch (152 commits)
 ├ Mises a jour recentes jgrpp      ├ Decouplage (branche decouple)
 ├ Fonctionnalites propres jrpm     └ Aeroports modulaires (branche multitile-airport)
 └ Renommage jrpm-0.1.0
                 \                 /
                  jrpm branche (git merge)
```

## Branches et commits

- Branche : `jrpm` (branche principale de developpement)
- Commits cles :
  - `d4c45740` 5 fonctionnalites propres a jrpm
  - `71fe214c` merge de pulseXLB (decouplage + aeroports modulaires)
  - `425e7207` Renommage de version openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` Compatibilite multi-versions du serveur
  - `4716b925` Nombre de connexions paralleles configurable

## Licence

Identique a OpenTTD : **GPL-2.0**.