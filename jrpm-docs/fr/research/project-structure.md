---
title: Etude de la structure du projet et de l'organisation du code
---

> Objet d'etude : `G:\GitHub\OpenTTD-patches` (espace de travail des sources jgrpp)
> Ressource de reference : `G:\game\openttd-jgrpp` (jeu complet compile, lecture seule)
> Date d'etude : 2026-08-14
> Branche : jgrpp (HEAD actuel `5b5c452e1b`, environ version 16.0)

---

## 1. Apercu du projet

OpenTTD jgrpp (JGR's Patchpack) est une branche d'amelioration bien connue basee sur OpenTTD. Les sources de cet espace de travail correspondent a la version **16.0**, contenant de nombreuses fonctionnalites privees JGR (tracerestrict, signaux programmables, scheduled dispatch, remplacement de modeles, signaux ameliores, etc.).

| Projet | Description |
|---|---|
| Langage | C++20 (`CMAKE_CXX_STANDARD 20`, sans extension) |
| Construction | CMake (≥3.17), `src/CMakeLists.txt` organise par repertoire |
| Moteur de script | Squirrel (`src/3rdparty/squirrel/`), utilise pour AI / GameScript / scripts de template |
| Reseau | Couche protocolaire TCP/UDP proprietaire (`src/network/`), HTTP avec WinHttp (Windows) / libcurl (autres) / JS (Emscripten) |
| Archive | `src/saveload/` + `src/sl/` format binaire proprietaire |
| Threads | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool pool de taches), `src/timer/` temporisateurs |

---

## 2. Structure des repertoires sources et division modulaire

Niveau superieur : `CMakeLists.txt` comme script de construction racine ; `src/` pour tout le code source C++ ; `bin/` pour les donnees d'execution (scripts AI compatibles, fichiers de langue, etc.) ; `media/`, `os/`, `cmake/`, `docs/` pour les ressources, le code plateforme, les scripts de construction et la documentation.

Sous-repertoires principaux dans `src/` et leurs responsabilites :

| Repertoire | Responsabilite |
|---|---|
| `src/core/` | Outils de base : types pool, bitset, conteneurs, encapsulation de threads, operations binaires, etc. |
| `src/network/` | Reseau : serveur/client/UDP/HTTP/telechargement de contenu/protocole administrateur (`core/` sous-repertoire pour les bases protocolaires) |
| `src/script/` | Framework d'execution de scripts : encapsulation Squirrel, instances, configuration ; `api/` pour toutes les classes d'API exposees a AI/GS |
| `src/ai/` | Framework NoAI (instances AI, scanner, configuration, GUI) |
| `src/game/` | Framework GameScript (instances GS, configuration, GUI) |
| `src/newgrf/` | Decodage et traitement NewGRF |
| `src/pathfinder/` | Pathfinding (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | Lecture/ecriture d'archives |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | Backends rendu/audio/video |
| `src/lang/` | Sources de chaines de localisation (generees par strgen) |
| `src/table/` | Tables statiques ; `table/settings/*.ini` sont les **sources de definition des parametres** (generees par settingsgen) |
| `src/3rdparty/` | Bibliotheques tierces (squirrel, llvm, icu, etc.) |
| `src/timer/`, `src/os/`, `src/misc/` | Temporisateurs, plateforme, divers |

Les fichiers epars du niveau superieur sont nommes par systeme (ex. `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), suivant les conventions OpenTTD : `*_cmd` pour la logique de commande, `*_gui` pour les fenetres, `*_base/_type/_func` pour les structures de donnees et fonctions inline.

---

## 3. Methodes de construction

- **CMake en trois etapes** : `cmake -B build ..` → `cmake --build build` → produit `openttd.exe`. Le depot fournit `build.sh` / `build-dedicated.sh`.
- **Dependances** : `vcpkg.json` declare (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus, etc.) ; Windows utilise WinHttp (pas besoin de curl), non-Windows utilise libcurl (`CMakeLists.txt:121-127`).
- **Chaine d'outils (host tools)** : `strgen` (fichiers de langue), `settingsgen` (genere le code de parametres depuis `src/table/settings/*.ini`), `squirrel_export` (genere les liaisons Squirrel depuis `src/script/api/script_*.hpp`).
- **Produits de generation cles** : `generated/script/api/<ai|gs>/...sq.hpp` (liaisons API, **decouvertes automatiquement par `file(GLOB script_*.hpp)`, les nouvelles classes d'API n'ont pas besoin de modifier la liste d'enregistrement**, seulement ajouter le `.hpp` et le `.cpp` dans la liste source de `src/script/api/CMakeLists.txt`) ; `generated/rev.cpp` ; `generated/ottdres.rc`.
- **Systeme de parametres** : La version moderne a ete modifiee pour etre **pilotee par INI** -- `src/table/settings/*.ini` (contenant des sections `[SDTC_VAR]`, `cat=SC_*` pour la categorisation, `flags`, `post_cb`, etc.), les `settings_*.cpp/h` sont generes par settingsgen ; les membres de structure correspondants sont dans `src/settings_type.h` (ex. `NetworkSettings` a partir de la ligne 575).

---

## 4. Emplacements d'implementation des cinq grands systemes fonctionnels

### 4.1 Telechargement de ressources (telechargement de contenu / BaNaNaS)

| Point d'interet | Emplacement |
|---|---|
| Classe principale du client de contenu | `src/network/network_content.h/.cpp` -- `ClientNetworkContentSocketHandler` (a la fois `ContentCallback` + `HTTPCallback`) |
| GUI de telechargement de contenu | `src/network/network_content_gui.cpp/.h` |
| Client HTTP | `src/network/core/http.h/.cpp` -- `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, asynchrone pilote par evenements (non bloquant, boucle principale interroge) |
| Chaine de connexion du serveur de contenu | `src/network/core/config.cpp` -- `NetworkContentServerConnectionString()` : variable d'environnement `OTTD_CONTENT_SERVER_CS`, defaut `content.openttd.org` (protocole de metadonnees TCP) |
| URI miroir | `src/network/core/config.cpp` -- `NetworkContentMirrorUriString()` : variable d'environnement `OTTD_CONTENT_MIRROR_URI`, defaut `https://binaries.openttd.org/bananas` |
| Processus de telechargement | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (POST de tous les content ID au miroir, le miroir renvoie un flux tar multi-fichiers, ecrit fichier par fichier) → `AfterDownload()` fait gunzip + `TarScanner` decompression |
| Decompression | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| Infrastructure de threads (utilisable pour le parallelisme) | `src/worker_thread.h/.cpp` -- `WorkerThreadPool` + `EnqueueJob` ; `src/thread.h` encapsulation de threads plateforme |

**Conclusion sur l'etat actuel** : 1. Un seul miroir configurable uniquement par variable d'environnement, pas de parametre en jeu ; 2. Le telechargement est **mono-connexion, serialise** (un seul POST pour tous les fichiers) ; 3. La decompression s'execute de maniere synchrone sur le thread principal. → Les points de modification pour multi-threading/multi-miroirs sont clairs.

### 4.2 Limite de joueurs en ligne / entreprises du serveur

| Point d'interet | Emplacement |
|---|---|
| Constante de limite de clients | `src/network/network_type.h:21` -- `static const uint MAX_CLIENTS = 255;` |
| Pool de clients | Meme fichier `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` ; `ClientID` est `uint32_t` |
| Pool d'ID d'entreprise | `src/company_type.h` -- `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` ; les fausses entreprises occupent 253/254/255 |
| Masque d'entreprise | Meme fichier `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 bits, ne peut suivre que 16 entreprises) |
| Verification d'acceptation du serveur | `src/network/network_server.cpp:360` -- `_network_clients_connected < MAX_CLIENTS` ; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| Element de parametre client | `src/table/settings/network_settings.ini:231/241` -- `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS) ; structure dans `src/settings_type.h` `NetworkSettings` |
| **Largeur de bit du protocole (contrainte materielle)** | `src/network/core/network_game_info.cpp` -- `companies_max` et `clients_max` sont tous deux envoyes avec **`Send_uint8`/`Recv_uint8`** (lignes 251-296, 422-432) |
| Affichage de la liste des serveurs | `src/network/network_gui.cpp:519` etc. |

**Conclusion sur l'etat actuel** :
- **Limite de clients = 255 est deja la limite protocolaire** (champ uint8 + compatibilite coordinateur de jeu/navigateur de serveur). Pour la depasser, il faut changer les champs `network_game_info` en uint16 (chaine complete client↔serveur + diffusion UDP + protocole du coordinateur de jeu), ce qui constitue un changement de protocole externe.
- **Limite d'entreprises = 15** (`CompanyIDTag` End=0xF). Peut etre augmentee en toute securite a **252** (End=0xFC) : le fond reste uint8, la largeur en octets de l'archive ne change pas (compatible avec les anciennes archives), necessite d'elargir simultanement `CompanyMask` (uint16→uint32) et les hypotheses d'interface/boucle.

### 4.3 Groupement de vehicules

| Point d'interet | Emplacement |
|---|---|
| Structure de donnees de groupe | `src/group.h` -- `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number) ; `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| Commandes de groupe | `src/group_cmd.cpp` -- `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (ajoute les vehicules a ordres partages a un groupe existant, `AddVehicleToGroup` vers ligne 718) |
| Enregistrement de commande | `src/group_cmd.h:27-35` -- `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)` ; enumeration dans `src/command_type.h` `enum class Commands` (a partir de la ligne 492) |
| GUI de groupe | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| Vehicule↔groupe | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| Ordonnancement/partage d'ordres | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, lie a la liste d'ordres) |
| Statistiques de groupe | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` etc. (group.h:125-128) |

**Conclusion sur l'etat actuel** : `CmdAddSharedVehicleGroup` (ajouter les vehicules a ordres partages d'un vehicule a un groupe) et `CmdCreateGroupFromList` (creer un groupe depuis une liste) existent deja, mais **il manque la logique complete de « creation/attribution automatique de groupe par ordres partages »**. La nouvelle commande `AutoGroupSharedOrders` (parcourir tous les vehicules principaux de l'entreprise → agreger par `OrderList` → creer automatiquement le groupe et attribuer) a un chemin d'implementation clair.

### 4.4 Infobulle de construction (indication de prix au-dessus de la souris lors de la construction de rails, etc.)

| Point d'interet | Emplacement |
|---|---|
| GUI/logique de construction de rails | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp` ; routes `road_gui.cpp/road_cmd.cpp` |
| Estimation des couts | Chaque `*_cmd.cpp` a `DoCommand` retourne `CommandCost` ; le mode `DC_QUERY_COST` peut etre utilise dans la GUI pour demander le prix |
| Tuile sous la souris | `src/viewport_func.h:36` -- `GetTileBelowCursor()` ; `_cursor.pos` (coordonnees ecran) ; `src/viewport.cpp:1056` |
| Mecanisme existant d'indication textuelle | `src/texteff.hpp` -- `AddTextEffect(msg, x, y, duration, mode, ...)` (texte flottant en coordonnees monde), `UpdateTextEffect` ; `src/texteff.cpp` |
| UX existante d'indication de construction | La barre d'outils des rails `OnPlaceDrag` affiche la zone selectionnee et le cout cumule (`_thd` tilehighlight, `src/tilehighlight_func.h`) ; la barre d'etat `statusbar_gui.cpp` peut afficher le cout de l'outil |
| Point de rafraichissement par trame | Chaque fenetre de barre d'outils `OnMouseLoop` / `viewport.cpp` `HandleMouseEvents` (lignes 5422/5733) |

**Conclusion sur l'etat actuel** : Pas d'infobulle de prix au-dessus de la souris. Peut etre implemente avec `AddTextEffect` ancre sur la tuile sous la souris (suit la case du curseur), ou en dessinant une infobulle en coordonnees ecran ; le cout peut etre obtenu avec `DC_QUERY_COST` pour l'outil courant sur une seule tuile + cumul pour la zone de selection etiree.

### 4.5 Interface AI (conservation NoAI + IA consciente de la partie)

| Point d'interet | Emplacement |
|---|---|
| Framework NoAI | `src/ai/` -- `ai_core.cpp` (boucle principale AICore), `ai_instance.cpp` (AIInstance/VM Squirrel), `ai_scanner.cpp` (scanne `ai/`), `ai_gui.cpp` (selection/configuration), `ai_config.cpp` |
| Framework GameScript | `src/game/` -- `game_core.cpp`, `game_instance.cpp` etc. (GS est le « mode divin », permissions superieures a AI) |
| Classes d'API de script | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin`, etc., 60+ classes) |
| Enregistrement automatique d'API | `src/script/api/CMakeLists.txt` -- `file(GLOB script_*.hpp)` genere automatiquement les liaisons `ai_*.sq.hpp`/`gs_*.sq.hpp` ; les `.cpp` doivent etre ajoutes a la liste source (a partir de la ligne 235) |
| Macros de liaison Squirrel | `src/script/squirrel_class.hpp` -- `DefSQClass` / `DefSQStaticMethod` ; `ai/ai_controller.sq.hpp` est la liaison du controleur AI |
| Ordonnancement des instances | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (suspension/reprise), evenements `script_event*` |
| Controle d'acces aux entreprises | `src/script/api/script_object.hpp:318` -- `ScriptObject::GetCompany()` ; `ScriptCompanyMode` (`IsDeity()`) distingue le mode divin GS ; validation des parametres d'API entreprise `ResolveCompanyID`, `EnforceCompanyModeValid` |
| Parametres de script | `src/table/settings/script_settings.ini` (`game.script.*`) ; configuration d'instance AI `ai_config.cpp` |

**Conclusion sur l'etat actuel** : Dans l'API de script moderne, certaines informations des concurrents (comme `GetBankBalance`) ne sont plus limitees, mais **il n'existe pas d'API d'agregation orientee conscience globale** (pas d'entree unique pour « enumerer toutes les entreprises/economie globale/statistiques globales de la carte »), ni de point d'acces « AI globale » controllable par interrupteur. Ajouter une classe d'API `ScriptGlobal` (enregistrement automatique GLOB) + un parametre `game.script` « permettre la perception globale de l'AI » suffit pour realiser l'objectif : GS toujours disponible (mode divin), AI controlee par interrupteur -- soit « conserver NoAI, ajouter une IA consciente de la partie, avec controle d'acces ».

---

## 5. Apercu des risques de modification

| Fonctionnalite | Principaux fichiers modifies | Risque |
|---|---|---|
| F1 Telechargement multi-source/multi-thread | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | Moyen (machine d'etat de callback reseau necessite de la prudence) |
| F2 Extension des limites | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (uint16 optionnel) | Faible-moyen (entreprise 252 faible risque ; client >255 necessite changement de protocole) |
| F3 Groupement automatique de vehicules | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | Faible |
| F4 Infobulle de construction | Nouveau `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` crochets, `src/lang/english.txt` | Faible-moyen |
| F5 IA consciente de la partie | Nouveau `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, AI exemple `bin/ai/GlobalAI/` | Faible (enregistrement automatique du framework) |

> Note : Toutes les modifications sont basees sur l'espace de travail local de la branche `jgrpp`, non compilees (pas de chaine d'outils de construction disponible sur cette machine) ; tous les patches peuvent etre examines avec `git diff` puis `git apply`.