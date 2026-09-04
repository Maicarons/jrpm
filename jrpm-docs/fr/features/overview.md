---
title: Apercu des fonctionnalites
---

# Apercu des fonctionnalites

jrpm = toutes les fonctionnalites de jgrpp + pulseXLB (decouplage de locomotives + aeroports modulaires) + modded (historique des trajets + vitesse de roulage) + inspiration cmclient (ameliorations UI multijoueur) + fonctionnalites propres a jrpm.

## Fonctionnalites propres a jrpm

| # | Fonctionnalite | Documentation | Statut |
|---|---|---|---|
| 1 | Telechargement de ressources : multi-miroirs + parallele | [Telechargement de ressources](./01-resource-download) | ✅ Implemente (nombre de connexions configurable) |
| 2 | Etude des limites du serveur | [Etude des limites du serveur](./02-server-caps) | Conclusion de recherche (limites structurelles) |
| 3 | Groupement automatique de vehicules | [Groupement automatique de vehicules](./03-vehicle-autogroup) | ✅ Implemente |
| 4 | IA consciente de la partie | [IA consciente de la partie](./05-global-ai) | ✅ Implemente |

> Note : L'ancienne fonctionnalite F4 « Infobulle de cout de construction » a ete retiree sur demande (commit `96ebfb75`).

## Portage modded (premier lot)

| Fonctionnalite | Documentation | Statut |
|---|---|---|
| Historique des trajets des vehicules | [Historique des trajets](./triphistory) | ✅ Implemente |
| Vitesse de roulage des avions reglable | [Vitesse de roulage](./plane-taxi-speed) | ✅ Implemente |

## Inspiration cmclient (deuxieme a cinquieme lots)

| Fonctionnalite | Documentation | Statut |
|---|---|---|
| Signets de position / Details de fret / Observation | [Ameliorations UI multijoueur](./ui-enhancements) | ✅ Implemente (deuxieme lot) |
| Systeme de surbrillance par objet | [Surbrillance + Plans](./highlight-blueprint-plan) | ✅ Implemente (troisieme lot, 6 commits) |
| Systeme de plans (copie/rotation/emplacements/reconstruction) | [Surbrillance + Plans](./highlight-blueprint-plan) | ✅ Implemente (troisieme lot) |
| Zonage urbain + archive growth_tiles | [Zonage urbain](./town-zoning) | ✅ Implemente (quatrieme lot) |
| Enregistrement et rejeu de commandes | [Rejeu de commandes](./command-replay) | ✅ Implemente (cinquieme lot) |

> **La couche d'objet de commande 2** (2251 lignes de code genere de cmclient) : La pratique de portage a demonte qu'elle peut **etre entierement contournee** -- la surbrillance utilise `CMD_ERROR` pour l'estimation des couts, les plans utilisent des fermetures de commande, le rejeu utilise la serialisation native de commandes jrpm, donc elle n'a pas ete portee.

## Fonctionnalites fusionnees (provenant de pulseXLB)

| Fonctionnalite | Documentation | Description |
|---|---|---|
| Decouplage de locomotives (decouple) | [Decouplage](./decouple) | Systeme complet d'attelage/decouplage de trains |
| Aeroports modulaires (multitile-airport) | [Aeroports modulaires](./multitile-airport) | Refonte du systeme d'aeroport multi-tuiles |

## Heritage complet

- **Toutes les fonctionnalites de jgrpp** : Signaux ameliores (signaux multiples/signaux programmables/emplacements et compteurs), scheduled dispatch, tracerestrict, remplacement de modeles, fenetre d'achat locomotive/wagon separee, freinage realiste, routes a sens unique ameliorees, securite des passages a niveau, etc. ;
- **Noyau OpenTTD 16.0** : Compatibilite complete avec l'ecosysteme NewGRF/scripts/archives.

## Acces rapide aux parametres

| Fonctionnalite | Parametre |
|---|---|
| Nombre de telechargements paralleles | `network.content_download_parallel` (1-8) |
| Liste des miroirs de telechargement | `network.content_mirrors` |
| Serveur de contenu | `network.content_server` |
| Modification de disposition d'aeroport | `station.allow_modify_airports` |
| Type aerien par defaut | `gui.default_air_type` |
| Interrupteur de conscience globale de l'IA | `game.script.allow_global_ai_access` |
| Vitesse de roulage des avions | `vehicle.plane_taxi_speed` (1-8, defaut 4) |

## Acces rapide aux commandes console

| Fonctionnalite | Commande |
|---|---|
| Groupement automatique de vehicules | `autogroup train\|road\|ship\|aircraft` |
| Ajouter une IA consciente de la partie | `start_ai GlobalAI` |
| Signets de position | `savelocation <1-9>` / `gotolocation <1-9>` |
| Details de fret de l'entreprise | `company_cargo <company_id>` |
| Observer une entreprise | `watch <company_id>` |
| Copie/rotation/sauvegarde/chargement/reconstruction de plan | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| Enregistrement/rejeu de commandes | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| Vitesse/statistiques/exportation/plantation d'arbres du jeu | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>`