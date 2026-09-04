---
title: Groupement automatique de vehicules
---

## Etat actuel (conclusions de recherche)

- Structure de donnees de groupe : `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP` ;
- Commandes existantes (`src/group_cmd.h` / `group_cmd.cpp`) :
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (renommer/definir groupe parent)
  - `CmdAddVehicleGroup` (ajouter un seul vehicule)
  - `CmdAddSharedVehicleGroup` (ajouter les **vehicules partageant les ordres d'un vehicule** a un groupe **existant**)
  - `CmdCreateGroupFromList` (creer un groupe depuis une liste de vehicules, supporte le type de liste `VL_SHARED_ORDERS` ; nommage automatique)
- Assistant de nommage automatique : `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, nomme par villes de depart/arrivee `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) -- jgrpp a deja l'infrastructure de « nommage par ligne » ;
- Mecanisme d'enregistrement des commandes : `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (macro declenchant simultanement le processeur et l'enregistrement `CommandTraits`) ;
- Vehicule↔groupe : `vehicle_base.h` `Vehicle::group_id` ; chaine d'ordres partages `FirstShared()/NextShared()` ; `OrderList *orders`.

**Conclusion** : Il manque une commande globale pour « parcourir tous les vehicules de l'entreprise, creer automatiquement des groupes par ordres partages et y attribuer les vehicules ».

## Implementation de cette fonctionnalite

### Nouvelle commande `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Processus (`src/group_cmd.cpp`) :
1. Parcourt tous les vehicules principaux du type specifie de l'entreprise (`Vehicle::IterateTypeFrontOnly(type)`), ignore les vehicules n'appartenant pas a l'entreprise, sans ordres ou deja dans un groupe personnalise ;
2. Compte chaque chaine d'ordres partages ; quand ≥2 vehicules partagent la meme liste d'ordres :
   - Utilise `GenerateAutoNameForVehicleGroup(v)` pour generer le nom du groupe (ex. « VilleA ↔ VilleB ») ;
   - Construit `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, appelle imbrique `Command<Commands::CreateGroupFromList>::Do(flags, ...)` pour creer le groupe et deplacer tous les vehicules partages dedans ;
   - Apres cela, `group_id` des vehicules dans le groupe n'est plus le groupe par defaut, la boucle les ignore automatiquement (une seule liste d'ordres cree un seul groupe) ;
3. `GroupChangeDeferredUpdateScope` differe uniformement la mise a jour des statistiques de groupe.

### Trois points d'entree

| Point d'entree | Emplacement | Description |
|---|---|---|
| Bouton de fenetre de groupe | `src/group_gui.cpp` nouveau `WID_GL_AUTOGROUP_SHARED` (a cote du LIVERY dans la barre d'outils), OnClick envoie la commande | Sprite reuse `SPR_GROUP_CREATE_TRAIN + vtype` |
| Commande console | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (sans argument = execute les quatre types) | `IConsole::CmdRegister` |
| Script/autre | Tout code peut utiliser `Command<Commands::AutoGroupSharedOrders>::Post(...)` | La commande est synchronisee sur le reseau, sure en multijoueur |

### Fichiers concernes

- `src/command_type.h` : Nouveau `AutoGroupSharedOrders` dans l'enum `Commands` (**Attention : l'insertion au milieu de l'enum decale les IDs des commandes suivantes, les versions anciennes et nouvelles ne seront pas coherentes en multijoueur, c'est normal dans un fork**)
- `src/group_cmd.h` / `group_cmd.cpp` : Declaration et implementation de la commande
- `src/widgets/group_widget.h` : Nouvel ID de widget
- `src/group_gui.cpp` : Bouton de barre d'outils (NWidget + OnPaint sprite + OnClick)
- `src/console_cmds.cpp` : Commande console
- `src/lang/english.txt` : `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Points de verification

1. Deux vehicules ou plus partagent la meme liste d'ordres → cliquer sur le bouton/`autogroup` genere un groupe « nom de ligne » et tous les vehicules sont attribues au groupe ;
2. Les vehicules deja groupes ne sont pas deplaces ; les differentes listes d'ordres forment leurs propres groupes ;
3. Les statistiques du groupe (nombre/profit) sont correctement mises a jour (depend de `GroupChangeDeferredUpdateScope`).