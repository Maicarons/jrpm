---
title: "Planification du troisieme lot : Systeme de surbrillance + Systeme de plans"
---

# Planification du troisieme lot : Systeme de surbrillance + Systeme de plans

Deux fonctionnalites de grande valeur provenant de cmclient ont ete etudiees en profondeur. Cette page documente le plan de portage et l'evaluation de la charge de travail.

## I. Systeme de surbrillance par objet (cm_highlight, 2888 lignes)

**Fonctionnalite** : Lors de l'activation d'un outil de construction, surbrillance en temps reel des objets a construire -- apercu precis de **15 types d'objets** : voies ferrees entirees, gares completes, routes/parkings, signaux, ponts, tunnels, quais, aeroports, industries, etc.

**Dependances de portage (API vanilla → API jrpm)** :

| Dependance cmclient | Correspondance jrpm |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (viewport.cpp interne) | Existe mais signature differente (jgrpp refactore), necessite alignement un par un |
| `TileZoning` (coloration de zonage urbain) | jrpm n'en a pas → necessite creation |
| Etats globaux `_fn_mod` etc. | Specifique cmclient → necessite reconception |
| Constructeurs d'objets `ObjectTileHighlight::make_rail/road_stop/...` | Dependent des API NewGRF gares/routes, jrpm differe de vanilla |

**Charge de travail** : Environ 2-3 sessions (4-6 heures chacune), la difficulte principale etant l'alignement du pipeline de rendu viewport.

## II. Systeme de plans (cm_blueprint, 660 lignes)

**Fonctionnalite** : Selectionner une zone → enregistrer la sequence de commandes de construction (voies/gares/tunnels/ponts/signaux) → stockage sur 16 emplacements → reconstruction en un clic + rotation.

**Dependances de portage** :

| Dependance cmclient | Correspondance jrpm |
|---|---|
| **Couche d'objet de commande** `cm_commands.hpp` (as_company / with_callback / set_auto / no_estimate + 100+ classes de commande generees, 2251 lignes) | jrpm n'a pas cette abstraction → **doit d'abord etre portee/reescrite** |
| Etat `_station_gui` de `cm_station_gui.hpp` | Structure d'etat GUI de gare jrpm differente |
| Pointeur intelligent `sp<Blueprint>` | jrpm peut utiliser `std::shared_ptr` |
| Parcours de tuiles `BlueprintCopyArea` | Depend des API generiques `TileIndexDiffC` (jrpm les a) |

**Charge de travail** : Environ 2-3 sessions (incluant la couche d'objet de commande).

## III. Ordre de mise en oeuvre recommande

```
Etape 1 : Couche d'objet de commande (conception cm_command_type, reimplementation avec Command<T>::Do/Post de jrpm)
         -- fondation commune pour les plans et le rejeu de commandes (load_commands)
Etape 2 : Surbrillance par objet (alignement du pipeline viewport, portage objet par objet)
Etape 3 : Plans (copie/rotation/emplacements/rejeu)
Etape 4 (optionnel) : Rejeu de commandes (rejeu complet de partie lzma) + Zonage urbain (growth_tiles, necessite extension d'archive)
```

## Remarque preliminaire

Ces trois fonctionnalites totalisent environ **5000+ lignes**, traversent trois couches centrales (systeme de commandes/archive/rendu viewport) et dependent d'une serie de couches intermediaires que jrpm n'a pas (objet de commande, bus d'evenements, serialisation bitstream). Il est plus prudent de les traiter comme des projets speciaux independants -- il est recommande de les executer par lots, chaque lot etant compile et verifie en multijoueur avant validation, pour eviter les difficultes de regression liees a un changement massif unique.