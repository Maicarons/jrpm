---
title: Guide d'ajustement des performances du serveur
---

> Base sur la branche jrpm apres fusion (incluant les fonctionnalites pulseXLB et les optimisations propres a jrpm).
> Les valeurs par defaut des parametres proviennent de `src/table/settings/network_settings.ini`, il est recommande de se referer aux tests de charge reels.

## I. Optimisations de performances deja mises en oeuvre (cette version)

| Optimisation | Commit | Description |
|---|---|---|
| Nombre de telechargements paralleles configurable | 4716b925 | `network.content_download_parallel` (defaut 4, 1-8) : nombre de fichiers telecharges simultanement, remplace la valeur codee en dur 4 |
| Multi-miroirs + telechargement parallele au niveau fichier | d4c45740 (F1) | `network.content_mirrors` miroirs multiples separes par des virgules, reessai par miroir en cas d'echec, repli vers l'ancien protocole |
| Compatibilite multi-versions du serveur | cb9848b7 | Le serveur peut accepter simultanement les clients jrpm / jgrpp original / pulseXLB |

## II. Parametres cles des performances du serveur (systeme jgrpp)

| Parametre | Defaut | Plage | Signification | Suggestion d'ajustement |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | Nombre de trames entre chaque verification de synchronisation (detection desync). Plus eleve = economise la bande passante, plus faible = detecte plus tot les desynchronisations | Priorite stabilite multijoueur : reduire a 20-50 ; si desync frequents, reduire |
| `network.frame_freq` | 0 | 0-100 | Nombre de trames entre chaque envoi de trame de commandes par le serveur (0 = envoi chaque trame). Plus eleve = economise bande passante/CPU, mais augmente la latence des operations | Generalement 0-3 ; serveurs a forte affluence jusqu'a 5, tester pour trouver le bon equilibre |
| `network.commands_per_frame` | 2 | 1-65535 | Limite maximale de commandes client traitees par trame (anti-spam/malveillance) | Beaucoup de joueurs/operations frequentes : augmenter a 4-8 |
| `network.commands_per_frame_server` | 16 | 1-65535 | Limite maximale de commandes du serveur lui-meme par trame | Generalement inutile de modifier |
| `network.bytes_per_frame` | 8 | 1-65535 | Limite maximale moyenne d'octets recus par trame (regulation de bande passante) | Bonne bande passante : augmenter a 16-32, ameliore la vitesse de synchronisation des grandes cartes |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | Limite maximale en rafale d'octets (permet des pics de courte duree) | Ajuster avec le parametre ci-dessus, ex. 512 |
| `network.max_init_time` | 60 | 0-32000 | Delai d'initialisation du client (tick) | Beaucoup de joueurs avec connexion faible : augmenter |
| `network.max_join_time` | 500 | 0-32000 | Delai de connexion du client (telechargement de carte + synchronisation) (tick) | Grandes cartes/bande passante lente : augmenter a 1000+ |
| `network.max_download_time` | 1000 | 0-32000 | Delai de telechargement de carte (tick) | Grandes cartes (4096+) : recommande 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | Tolerance maximale de latence du client (tick) | Beaucoup de joueurs a haute latence : augmenter, mais augmente le risque de desync |

> Note : Les `max_*_time` sont en ticks de jeu (1/74 seconde ≈13.5ms) ; `bytes_per_frame` se refere a la moyenne d'octets de la fenetre de synchronisation par trame.

## III. Configuration de depart recommandee pour le serveur (scenario multijoueur)

```ini
[network]
max_clients = 32            ; ou selon les besoins
max_companies = 15
frame_freq = 3              ; 0=envoyer les commandes chaque trame (le plus fluide) ; 3=compromis economie bande passante
sync_freq = 50              ; detecter plus tot les desynchronisations
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; telechargement parallele de contenu cote client
```

## IV. Directions d'optimisation des performances ulterieures (par priorite)

1. **Pool de threads HTTP + Telechargement par plage Range** (approche Openttd-Cluster 0007)
   Etat actuel : F1 a deja implemente le « parallelisme au niveau fichier » (plusieurs fichiers telecharges simultanement). 0007 fait un pool de threads au niveau HTTP + `CURLOPT_RANGE` pour le decoupage en plages, ce qui peut encore accelerer les **gros fichiers uniques** (scenarios geants .tar.gz). Les deux sont complementaires mais modifient les memes fichiers, recommande de le faire de maniere incrementale apres stabilisation de F1.

2. **Optimisation de l'envoi de carte**
   Verifier si jrpm active par defaut la compression de carte (zstd/lzma) ; pour les grandes cartes, comparer le temps des differents algorithmes de compression.

3. **Runtime serveur Rust (architecture de reference a long terme)**
   Le otc-engine d'Openttd-Cluster (Admin/RCON, metriques Prometheus, basculement de cluster, panneau Web, pont snapshot) depend de l'integration FFI complete d'un projet Rust, constitue une transformation de niveau « serveur de prochaine generation », pas une fusion au niveau patch ; jrpm conserve actuellement un binaire C++ pur unique.

4. **Performances de simulation du jeu**
   - Grandes cartes + beaucoup de vehicules : surveiller les parametres `economy`/`linkgraph` (linkgraph_settings.ini) ;
   - Pour des performances extremes, etudier les parametres de la classe `settings_game.economy.` et les parametres de limite du pathfinder (parametres `pathfinding`).

## V. Suggestions de verification

- Ouvrir un serveur `openttd-jrpm -D -c server.cfg`, tester avec plusieurs clients jrpm : temps de connexion, stabilite de la synchronisation des trames (pas d'indication desync), occupation CPU/memoire ;
- Telechargement de contenu : definir `content_download_parallel=8` et comparer le temps de telechargement d'un lot de NewGRF ;
- Test sous faible bande passante : combinaison `max_lag_time` et `sync_freq` pour trouver la limite de stabilite.