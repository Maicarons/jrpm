---
title: Apercu de la fusion des versions jrpm
---

> Branche : `jrpm` | Version : jrpm-0.1.0 (tagged, 2026-08-14)
> Produit de construction : `openttd-jrpm` (nom de l'executable)

## Relations entre les versions

```
                        jgrpp-0.73.1 (ancetre commun)
                        /                 \
        jgrpp branche (63 commits)          pulsexlb px-patch (152 commits)
        ├ tracerestrict etc. mises a jour    ├ jgrpp-decouple (decouplage)
        ├ Mes 5 fonctionnalites (d4c45740)   └ jgrpp-multitile-airport (aeroports modulaires)
        └ Renommage jrpm-0.1.0 (425e7207)
                        \                 /
                        jrpm branche (merge 71fe214c + compatibilite cb9848b)
```

## Contenu de la fusion

### 1. pulseXLB/OpenTTD-patches (px-patch complet 152 commits) → Deja fusionne

| Fonctionnalite | Description | Fichiers principaux |
|---|---|---|
| **Decouplage (decouple)** | Attelage/decouplage de trains : ordres de decouplage, transfert de consignes, limite de longueur/vitesse d'attelage, double locomotive, attelage NewGRF, pathfinding d'attelage (YAPF/NPF), ordonnancement independant apres decouplage | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **Aeroports modulaires (multitile-airport)** | Refonte du systeme d'aeroport multi-tuiles : systeme de type air (air.h/air_type.h/newgrf_airtype.*), gestion aerienne PBS (pbs_air.*), pathfinding aerien YAPF, `station.allow_modify_airports` (modification de disposition d'aeroport), `gui.default_air_type`, sprites d'aeroport multi-tuiles | air.*, pbs_air.*, aircraft_cmd.cpp (3600 lignes de refonte), airport_cmd/gui, station_cmd |

Gestion des conflits : Seulement 2 fichiers d'en-tete en conflit (aircraft.h / airport.h) -- la refonte aerienne pulseXLB a supprime des types morts **sans reference** dans l'espace de travail (`VehicleAirFlags` bitset, `AirportMovingDataFlag`), en prenant la suppression cote pulseXLB, confirme qu'aucun autre fichier n'y fait reference.

### 2. Openttd-Cluster (projet de cluster Rust utilisateur) → Inspiration selective

| Patch | Traitement | Description |
|---|---|---|
| 0006 vanilla-native-server (compatibilite multi-versions clients) | ✅ **Deja fusionne** (cb9848b7) | Le serveur jrpm accepte simultanement les clients jrpm / jgrpp original (`jgrpp-`) / pulseXLB (`pxp`) ; la version NewGRF est toujours strictement verifiee |
| 0001 revision-handshake / 0005 version-metadata | ✅ Approche deja adoptee | jrpm utilise une chaine de revision tagged independante `jrpm-0.1.0`, poignee de main multijoueur isolee de jgrpp/pxp, realisant « nouvelle version facile pour le multijoueur » |
| 0007 parallel-download (pool de threads HTTP + telechargement par plage Range, 30KB) | Reference non fusionnee | Meme theme que F1 « parallelisme fichier + multi-miroirs » et modifie les memes fichiers ; le **pool de threads de couche transport/telechargement par plage** de 0007 est note comme direction d'amelioration ulterieure pour F1 |
| 0002-0004 snapshot/command/FFI bridge | Reference architecturale | Depend de tout le runtime Rust otc-engine (lie statiquement via FFI), constitue une « integration globale a long terme » plutot qu'une fusion au niveau patch ; jrpm conserve actuellement un binaire C++ pur unique |

### 3. Fonctionnalites propres jrpm (5 fonctionnalites precedentes, d4c45740) → Deja dans la branche jrpm

Telechargement parallele (multi-miroirs + 4 sessions concurrentes), groupement automatique, infobulle de construction, IA consciente de la partie (ScriptGlobal + GlobalAI), parametres miroir/serveur de contenu.

## Strategie multijoueur (« nouvelle version facile pour le multijoueur »)

- jrpm est une **version tagged** : `IsNetworkCompatibleVersion` exige une correspondance exacte de la chaine de revision → **le client jrpm ne se connecte qu'aux serveurs jrpm**, completement isole de jgrpp 0.73.x / pxp ;
- **Assouplissement du serveur** : Le serveur jrpm accepte supplementairement les clients `jgrpp-*` et `pxp*` (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, la version NewGRF doit etre identique) ;
- Ainsi : l'hote jrpm ouvre un serveur = seulement les joueurs jrpm (par defaut) ; si necessite de compatibilite avec les anciens clients, peut accepter les joueurs jgrpp/pxp sans modification de configuration.

## Construction et verification (execution sur la machine de l'utilisateur)

```bash
# Premiere fois (necessite CMake + dependances, voir COMPILING.md)
cmake -B build ..
cmake --build build -j
# Produit : build/openttd-jrpm.exe
```

Priorites de verification :
1. `openttd-jrpm -v` affiche `jrpm-0.1.0` ;
2. Ouvrir une partie solo et jouer 1-2 ans (la fusion implique de gros changements train/aeroport + la version d'archive peut etre augmentee a cause de allow_modify_airports etc.) ;
3. Apres ouverture du serveur : client jrpm se connecte ✓ ; client jgrpp 0.73.x original tente de se connecter (prevue : peut entrer, si NewGRF identiques) ;
4. Decouplage : ajouter des ordres d'attelage/decouplage au train, verifier l'ordonnancement independant des deux trains apres decouplage ; Aeroports modulaires : activer `station.allow_modify_airports` puis modifier la disposition de l'aeroport ;
5. Regression des 5 fonctionnalites precedentes (telechargement parallele, groupement automatique, infobulle de construction, GlobalAI).

## Risques connus

- **Non compile verifie** : Le code fusionne+modifie n'a pas ete compile sur cette machine (pas de chaine d'outils), la premiere compilation reelle pourrait avoir des changements d'interface manques (surtout les trois grandes modifications aircraft/airport/train) ;
- Version d'archive : px-patch a peut-etre augmente SLV (M9 mentionne savegame version gate), les archives jrpm et les archives jgrpp 0.73.x pourraient ne pas etre interlisibles (comme convention jgrpp, retrocompatible avec les archives trunk) ;
- Le merge apporte tout l'historique pulseXLB, pour retracer la propriete des fonctionnalites utiliser `git log --oneline pulsexlb/px-patch`.

---

## Journal de fusion : 2026-09-28 (jgrpp-0.73.3 + px-patch 2609.x)

> Commits de fusion : jgrpp 94 commits (jusqu'à `jgrpp-0.73.3`) + px-patch 113 commits (jusqu'après `pxp-2609.10`).

**Nouveau :** côté pulsexlb, transport de véhicules routiers (RoRo, voir Fonctions), 128 types de fret (CargoTypes en Uint128, via XSLFI_CARGO_TYPES_128), lot de correctifs découplage/accouplement et améliorations d'horaires ; côté jgrpp, glisser-déposer des ordres/double-clic, navires à double extrémité (XSLFI_DOUBLE_ENDED_SHIPS) et de nombreux correctifs généraux.

**Gestion clé des conflits :** la couche de sauvegarde de pulsexlb repose sur l'ancien système de macros SLE_ alors que jrpm/jgrpp 0.73.3 utilise VarFileType/VarMemType + VarTypes ; tous les conflits saveload/ et sl/ ont été réécrits dans le système moderne avec ajout du support U128 (VarFileType::U128=13, VarMemType::U128, SLE_UINT128) ; les versions 367/368 réservées par jrpm sont conservées et les nouvelles versions amont passent à 369/370 ; la compatibilité avec les sauvegardes amont passe par le sous-bloc XSLFI_UPSTREAM_VERSION ; CT_VEHICLES en chaîne, comparaison grfid via GrfID, blocs console autogroup + RORO_DEBUG_COMMANDS conservés.

**État :** compilation MinGW ninja réussie, artefact `build/openttd-jrpm.exe` ; test rapide en nouvelle partie réussi.

---

## Journal de fusion : 2026-10-07 (jgrpp 0.73.3+89 + px-patch 2610.3)

> Commits de fusion : 23 commits jgrpp (après `jgrpp-0.73.3`, jusqu'à `6318727b02`) + 55 commits px-patch (jusqu'à `pxp-2610.3`, `4a4d0724b5`).

**Nouveau :** côté pulsexlb — ferry de trains (les navires sont aménagés pour transporter des trains entiers, fret dédié RAIL/CT_RAILVEHICLES, chargement par wagon), le système de pluie (weather.cpp : périodes de pluie aléatoires, assombrissement progressif du monde, voile de gouttes qui suit le zoom ; option difficulty.rain ; un triche sandbox impose la météo ; l'état de la météo est sauvegardé via WTHR/XSLFI_WEATHER), la sortie dans le même sens après découplage en attendant l'autre partie, l'affichage de la charge de chaque calandre et de chaque wagon, le choix du panneau train/véhicule dans les options RoRo, la suppression du à-coup du pathfinding de couplage (validation de la charge transformée en pré-vérification avec recul en cas d'échec) et le build Android avec publication d'APK ; côté jgrpp — StringID devenu type fort, refus correct des noms de société en double, taille préférée calculée pour les widgets, plus de nombreux refactorings et correctifs.

**Gestion clé des conflits :** le principal obstacle de compilation a été le typage fort de Label/StringID — tous les libellés littéraux de 4 caractères du code jrpm sont passés à une construction par chaîne (CT_RAILVEHICLES{"RAIL"}) ; les comparaisons de libellés de types de route dans afterload.cpp sont devenues RoadTypeLabel{"ROAD"} ; les champs de chaîne du AirTypeInfo vide dans airport.cpp utilisent STR_NULL ; les branches switch de STR_CHEAT_RAIN dans cheat_gui.cpp utilisent .base() ; l'ancien type de bloc CH_TABLE du nouveau bloc météo WTHR est devenu ChunkType::Table ; le tableau des triches conserve le type VarMemType et le sentinelle InflationCheat de jrpm tout en ajoutant la nouvelle ligne de pluie ; SetStringTip(SPR_GOTO_LOCATION, …) dans jrpm_watch_gui.cpp est devenu SetSpriteTip ; train_cmd.cpp conserve la porte enable_decouple et la sortie de debug desync tout en adoptant le correctif amont qui ne fait marche arrière que lorsque le train de tête franchit l'aiguillage (if (v->IsMovingFront())) ainsi que les drapeaux de sortie dans le même sens ; order_cmd.cpp adopte la nouvelle sémantique DrivingBackwards basée sur TCF_NO_DRIVING_CAB tout en conservant la variable DecouplePart ; deploy-docs.yml conserve la compilation VitePress et le déploiement GitHub Pages, tout comme README/.gitignore/.ottdrev-vc conservent l'identité jrpm ; SL_UPSTREAM_VERSION reste épinglé à 368 (DoubleEndedShips amont), le static_assert de src/saveload/engine_sl.cpp passe et les versions 367/368 réservées par jrpm sont inchangées.

**État :** compilation MinGW ninja réussie (-j3), artefact `build/openttd-jrpm.exe` ; test de fumée du serveur dédié `-D` réussi — génération de carte → sauvegarde → chargement → nouvelle sauvegarde → sortie sans assertion ni plantage de SetupEngines, les portes de version des blocs amont se comportent correctement.
