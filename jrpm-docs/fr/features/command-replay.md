---
title: Enregistrement et rejeu de commandes
---

# Enregistrement et rejeu de commandes

Portage du rejeu de commandes de cmclient (cinquieme lot, commit `f113acce28`). **Ne porte pas la couche d'objet de commande de cmclient** -- utilise directement l'infrastructure de serialisation de commandes native de jrpm (`DynBaseCommandContainer`), contournant entierement la « couche d'objet de commande ».

## Commandes console

```
cmdrecord [start [file]]    # Demarre l'enregistrement (fichier par defaut cmdrecord.jrcm, stocke dans le repertoire personnel)
cmdrecord stop              # Arrete (attend 10 ticks que la file de commandes se vide avant d'ecrire sur le disque)
cmdreplay <file>            # Rejoue : deserialise et execute immediatement chaque commande
```

## Points cles de l'implementation

- **Crochet d'enregistrement** : `CommandRecordLog` est accroche apres l'execution de la commande dans `DoCommandPInternal` -- c'est le **point d'execution unique et reel** pour les commandes locales, reseau et rejeu, il n'y aura pas de double enregistrement.
- **Serialisation** : `cmd / tile / error_msg / payload / company` utilisent `DynBaseCommandContainer::Serialise` pour une serialisation sans perte ; format de fichier `JRCM` magic + version + count + entrees.
- **Arret differe** : `cmdrecord stop` marque un delai de 10 ticks, `StateGameLoop` verifie chaque trame (`CommandRecordTick`) pour s'assurer que les commandes deja en file d'attente sont aussi capturees avant le vidage.
- **Execution du rejeu** : Chaque commande est executee immediatement via le chemin de commande serveur (`DCIF_NETWORK_COMMAND`) ; meme si l'en-tete count du fichier de rejeu n'est pas mis a jour, les donnees peuvent etre lues selon les donnees reelles.

## Verification

Serveur dedie E2E : Enregistrement de `pause` → fichier d'enregistrement de 26 octets → rejeu sur nouvelle carte → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## Utilisation typique

```
cmdrecord start build1     # Demarre l'enregistrement
# ... construisez des voies, des gares dans le jeu ...
cmdrecord stop             # Arrete et ecrit sur le disque
# Apres avoir change de carte ou recupere d'un plantage :
cmdreplay build1           # Reconstruit toutes les operations en un clic
```