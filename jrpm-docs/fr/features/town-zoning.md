---
title: Zonage urbain
---

# Zonage urbain

Portage du zonage urbain de cmclient (quatrieme lot, commit `9e3f95a2`). La base jrpm (pulseXLB) possede deja son propre systeme de zonage (barre d'outils + pipeline de rendu + menu). Ce lot ajoute par-dessus les modes d'evaluation uniques de cmclient et l'**archive growth_tiles**.

## Nouveaux modes (menu deroulant de la barre d'outils de zonage)

| Mode | Description | Coloration |
|---|---|---|
| **Town zones (Tz)** | Zones concentriques urbaines, reuse `squared_town_zone_radius` | Bordure Tz0=bleu clair / Tz1=rouge / Tz2=jaune / Tz3=vert / Tz4 centre-ville=blanc |
| **Town growth tiles** | Trajectoire de construction et demolition de maisons du mois courant/precedent | Nouvelle maison=vert / Demolition=bleu clair / Reconstruction=blanc / Croissance sautee=orange / Construction sautee=jaune / Demolition serveur=rouge |

## Couche de donnees growth_tiles (`cm_town_growth.cpp/.h`)

- Deux cartes mensuelles glissantes : `TileIndex → TownGrowthTileState` (mois courant / mois precedent)
- Crochets d'evenements :
  - `BuildTownHouse` → `NEW_HOUSE` (si le mois precedent etait une demolition, mise a niveau vers `RH_REBUILT`)
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → rotation mensuelle (mois precedent = mois courant, mois courant vide)
- **Persistance d'archive** : Nouveau bloc d'archive `GRWT` (`misc_sl.cpp`), serialise sous forme de liste de paires `{tile, state}` ; les anciennes archives sans ce bloc sont entierement compatibles (verifie aller-retour sauvegarde→chargement).

## Acces

Menu barre d'outils → Carte (Zoning) → Ouvrir la barre d'outils de zonage, les cercles interieur et exterieur selectionnent respectivement le mode d'evaluation.

## Note de reduction

Les modes specifiques au serveur CityBuilder de cmclient (zone d'acceptation CB / limite urbaine CB) et les champs d'extension `ext::Town` (publicite, fonds, statistiques de marchandises) sont propres au mode de jeu serveur et n'ont pas ete portes.