---
title: Guide multijoueur
---

# Guide multijoueur

## Strategie de compatibilite des versions

jrpm est une **version tagged** (`jrpm-0.1.0`), la poignee de main multijoueur exige une **correspondance exacte** de la chaine de revision :

| Scenario | Comportement |
|---|---|
| Client jrpm ↔ Serveur jrpm | ✅ Connexion normale (versions identiques) |
| Client jrpm ↔ Serveur jgrpp / pulseXLB original | ❌ Refus (isolation de version) |
| Client jgrpp original → Serveur jrpm | ✅ Autorise (le serveur accepte les revisions `jgrpp-*`) |
| Client pulseXLB → Serveur jrpm | ✅ Autorise (accepte les revisions `pxp`) |

::: warning Version NewGRF
Quel que soit le client, le **numero de version NewGRF doit etre strictement identique** a celui du serveur (verification rigoureuse de `_openttd_newgrf_version`), c'est la limite minimale de la simulation deterministe.
:::

## Ouvrir un serveur

```bash
# Serveur dedie (sans GUI)
openttd-jrpm -D -c server.cfg
```

Configuration recommandee pour `server.cfg` (voir [Ajustement des performances du serveur](../performance/server-tuning)) :

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## Rejoindre un serveur

- Dans le jeu : « Multijoueur → Rejoindre un serveur Internet/Ajouter un serveur », ou
- En ligne de commande : `openttd-jrpm -n <host>:<port>`

## Telechargement de contenu (NewGRF/Scenarios)

jrpm supporte le **telechargement multi-miroirs + parallele** :

```ini
[network]
content_server = content.openttd.org        ; Serveur de metadonnees
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; Nombre de telechargements paralleles (1-8)
```

- La liste des miroirs est separee par des virgules, utilisee dans l'ordre ; si un miroir echoue, passage automatique au suivant ;
- Les variables d'environnement `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` ont priorite sur les parametres ;
- Si tous les miroirs echouent, bascule automatique vers l'ancien protocole de telechargement TCP.

## Administration du serveur

- Commandes console : `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (groupement automatique de vehicules), etc. ;
- RCON : apres avoir defini `rcon_password` dans les parametres du serveur, vous pouvez envoyer des commandes console a distance.