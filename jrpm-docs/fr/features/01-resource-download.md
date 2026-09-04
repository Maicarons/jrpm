---
title: "Telechargement de ressources : multi-miroirs + telechargement parallele"
---

## Etat actuel (conclusions de recherche)

| Element | Etat actuel |
|---|---|
| Serveur de contenu (protocole de metadonnees) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()` : variable d'environnement `OTTD_CONTENT_SERVER_CS`, defaut `content.openttd.org` (port TCP 3978) |
| Miroir de telechargement | `NetworkContentMirrorUriString()` : variable d'environnement `OTTD_CONTENT_MIRROR_URI`, defaut `https://binaries.openttd.org/bananas` |
| Methode de telechargement | `network_content.cpp` `DownloadSelectedContentHTTP()` : une **seule** requete POST avec tous les content ID → le miroir renvoie une liste d'en-tetes de fichiers (`id,type,filesize,url` par ligne) → telechargement **sequentiel** de chaque fichier (une connexion GET par fichier) → `AfterDownload()` gunzip + tar decompression |
| Modele de threads | La couche HTTP (WinHttp) s'execute en arriere-plan, mais le **telechargement de fichiers est serialise** ; pas de liste de miroirs, pas de parametre en jeu, pas de parallelisme |

## Implementation de cette fonctionnalite

### 1. Nouveaux parametres (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, defaut vide = source officielle)
- `network.content_mirrors` (SLE_STR, plusieurs URI de miroirs separes par des virgules, defaut vide = miroir officiel)

Priorite : variable d'environnement > parametre de jeu > valeur par defaut officielle.

### 2. Analyse de configuration (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` lit le parametre ;
- Nouveau `NetworkContentMirrorUris()` analyse la liste separee par des virgules (suppression des espaces, saut des elements vides, source officielle de secours) ;
- `NetworkContentMirrorUriString()` modifie pour retourner le premier de la liste.

### 3. Telechargement parallele (`src/network/network_content.h/.cpp`)

- Nouveau `ContentFileDownload` (fichier a telecharger : id/type/filesize/url/filename) ;
- Nouveau `ContentDownloadSession : HTTPCallback` (statut de telechargement individuel et callback pour chaque fichier ; `IsCancelled` lie au handler) ;
- `DownloadSelectedContentHTTP()` : POST vers `mirrors[mirror_index]` → `ParseResponseHeaders()` analyse tous les en-tetes de fichiers en une fois → `StartDownloadSessions()` lance jusqu'a **4 sessions paralleles** (`CONTENT_DOWNLOAD_PARALLEL`), chaque session termine recupere automatiquement le prochain fichier a telecharger ;
- Chaine de reessai : echec de session/requete miroir → nouvelle requete vers le miroir suivant → tous les miroirs echouent → repli vers l'ancien protocole TCP (`DownloadSelectedContentFallback`) ;
- Arret elegant : en cas d'echec/annulation, positionne `download_cancelled`, attend la fin de toutes les sessions en cours (`OnAllSessionsDone`) avant de reessayer ou de se replier, evitant les callbacks pendants ;
- `ResetMirrorIndex()` : nouveau telechargement commence par le premier miroir (appele lors du declenchement GUI).

## Fichiers concernes

- `src/table/settings/network_settings.ini` (2 nouveaux parametres)
- `src/settings_type.h` (2 nouveaux champs dans `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (analyse de la liste des miroirs)
- `src/network/network_content.h` / `.cpp` (sessions paralleles)
- `src/network/network_content_gui.cpp` (reinitialisation de l'index du miroir avant telechargement)

## Points de verification

1. Apres avoir rempli plusieurs URI dans `network.content_mirrors` (separes par des virgules), le telechargement de contenu devrait fonctionner et telecharger **parallelement** plusieurs fichiers (observer la progression/verifier les connexions concurrentes avec un analyseur reseau) ;
2. En cas de coupure reseau/miroir errone, tentative automatique du miroir suivant, puis repli vers l'ancien protocole ;
3. L'annulation pendant le telechargement ne doit pas laisser de fichiers `.tar.gz` incomplets ;
4. Necessite une compilation reelle pour verification (pas d'environnement de construction dans ce depot, code non compile).

## Limitations connues

- Le parallelisme est code en dur a 4 (`CONTENT_DOWNLOAD_PARALLEL`), pourra etre rendu configurable ulterieurement ;
- Le protocole miroir depend du format API officiel de BaNaNaS (POST d'une liste d'IDs retourne une liste d'en-tetes de fichiers).