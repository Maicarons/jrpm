---
title: Ameliorations UI multijoueur (signets de position / details de fret / observation)
---

# Ameliorations UI multijoueur

Deuxieme lot de portage depuis **citymania-org/cmclient**, implemente sous forme de **commandes console** (ne depend pas de l'infrastructure de raccourcis clavier/barre d'outils de cmclient, stable et scriptable).

## Signets de position (Viewport Locations)

Sauvegarde/restauration de la position et du zoom de la fenetre principale (9 emplacements), ideal pour naviguer rapidement entre ses usines, gares et zones adverses en multijoueur.

```
savelocation <1-9>    # Sauvegarde la position+zoom actuels de la fenetre
gotolocation <1-9>    # Accede a la position sauvegardee
```

## Details de fret de l'entreprise (Company Cargo Details)

Fenetre de statistiques listant le **volume de fret livre + revenus** par marchandise pour l'entreprise, avec bascule entre les periodes **Total / Mois dernier**.

```
company_cargo <company_id>    # Ouvre la fenetre des details de fret
```

- La fenetre liste le volume et les revenus pour chaque marchandise standard + total en bas (identique a cmclient)
- Cliquez sur l'en-tete « Cargo » pour basculer entre les periodes total/mois dernier
- Les revenus par marchandise sont suivis via `CompanyEconomyEntry::cargo_income` (extension d'archive `XSLFI_COMPANY_CARGO_INCOME`), archivage aller-retour sans perte

## Aide a l'observation (Watch)

Permet a un observateur de se positionner rapidement sur la zone de construction d'une entreprise (saute aux coordonnees de la derniere construction de cette entreprise).

```
watch <company_id>    # Deplace la fenetre vers l'emplacement de l'entreprise
```

## Fonctionnalites ignorees et raisons

| Fonctionnalite cmclient | Traitement jrpm |
|---|---|
| Calque de liste de joueurs (cm_client_list_gui) | jgrpp **a deja** une fenetre Online Players (`NetworkClientList`), pas de double implementation |
| Infobulles de details au sol (cm_tooltips) | La LandInfoWindow de jrpm **couvre deja** l'affichage des details des maisons/industries/gares |

## Fichiers concernes

- `src/jrpm_locations.cpp/.h` (nouveaux : signets de position + company_cargo + commandes console watch)
- `src/jrpm_cargo_table.cpp/.h` (nouveaux : fenetre des details de fret)
- `src/window_type.h` (nouveau `WindowClass::CompanyCargos`)
- `src/console_cmds.cpp` (enregistrement des commandes)
- `src/lang/english.txt` (chaines `STR_JRPM_CARGOS_*`)