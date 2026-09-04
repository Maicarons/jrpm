---
title: Guide de developpement
---

# Guide de developpement

Ce document explique comment continuer le developpement sur jrpm. Tous les modeles proviennent du code reellement implemente dans ce projet (consultable dans `git log`).

## Apercu rapide de l'organisation du code

| Repertoire | Responsabilite |
|---|---|
| `src/` | Tout le code source C++ (niveau superieur nomme par systeme : `rail_cmd.cpp`, `group_gui.cpp`...) |
| `src/network/` | Reseau (serveur/client/UDP/HTTP/telechargement de contenu) |
| `src/script/` | Framework de script (Squirrel) ; `api/` pour les classes d'API AI/GS |
| `src/table/settings/*.ini` | **Sources de definition des parametres** (settingsgen genere le code) |
| `src/lang/english.txt` | Definition des chaines (strgen genere) |
| `src/sl/saveload_common.h` | Enumeration des versions d'archive (SLV) |
| `bin/ai/` | Scripts AI (`GlobalAI` est un exemple) |

## Modeles de developpement courants

- [Ajouter une commande de jeu](./add-command) : Enumeration de commande + `DEF_CMD_TUPLE_NT` + processeur + entree GUI/console
- [Ajouter une API de script](./add-script-api) : `script_*.hpp/.cpp` (liaison Squirrel automatique)
- [Ajouter un parametre](./add-setting) : `.ini` + champ `settings_type.h` + chaine

## Construction

```bash
cmake -B build ..
cmake --build build -j
```

## Notes de developpement

1. **Encodage** : Les sources/documents contenant du chinois doivent etre en UTF-8 apres modification (l'outil Write/Edit de cette machine a deja produit du GBK, utilisable avec `jgrpp-features/_fix_utf8.py` pour corriger) ;
2. **Messages de commit** : Recommande en anglais (evite les problemes d'encodage du terminal) ;
3. **Insertion au milieu de l'enumeration de commandes** : Decale les IDs des commandes suivantes -- les binaires ancienne version et nouvelle version ne seront pas cohérents en multijoueur, c'est normal dans un fork, la mise a jour doit etre synchronisee ;
4. **Version d'archive** : En cas de modification de la structure d'archive, ajouter une entree `SLV_*` dans `src/sl/saveload_common.h` et mettre a jour `SAVEGAME_VERSION` ;
5. **Nouveaux fichiers** : Les `.cpp` doivent etre ajoutes a la liste source du `CMakeLists.txt` correspondant (`src/CMakeLists.txt` ou sous-repertoire/`script/api/CMakeLists.txt`) ; les `script_*.hpp` sont decouverts automatiquement par `file(GLOB)`, pas besoin d'enregistrement ;
6. **Nouvelles chaines** : Ajouter dans `src/lang/english.txt` (les autres langues utilisent l'anglais par defaut) ;
7. **Compatibilite NewGRF/archive** : Modifier `_openttd_content_version` / le numero de version NewGRF avec prudence (affecte le multijoueur et la compatibilite du contenu).

## Suggestions de regression (importantes)

La branche actuelle **n'a pas ete compilee reellement**, apres la premiere construction, il est recommande de tester dans cet ordre :
1. Base : `openttd-jrpm -v` affiche `jrpm-0.1.0` ;
2. Archive : Ouvrir une partie solo et jouer 1-2 ans ;
3. Multijoueur : Interconnexion de clients jrpm ; connexion de clients jgrpp/pxp ;
4. Nouvelles fonctionnalites : Decouplage (attelage/decouplage/marche arriere), Aeroports modulaires (modification de disposition), Telechargement parallele, Groupement automatique, Infobulle de construction, GlobalAI ;
5. Regression des anciennes fonctionnalites : Signaux, tracerestrict, scheduled dispatch, remplacement de modeles.