---
title: Decouplage de locomotives (decouple)
---

# Decouplage de locomotives (decouple)

> Source : Branche de fonctionnalite `jgrpp-decouple` de pulseXLB/OpenTTD-patches (cœur des 152 commits), fusionnee dans jrpm via git merge.

## Presentation fonctionnelle

Le « decouplage de locomotives » permet aux trains de **decoupler et d'atteler** des wagons ou locomotives en gare/pendant l'ordonnancement, permettant :

- **Decouplage** : Le train, selon un « ordre de decouplage », laisse tomber une partie de ses wagons (ou wagon+groupe de locomotives) a une gare designee, le reste continue son chemin ;
- **Attelage** : Le train attend en gare, s'accouple automatiquement avec un autre train (ou un groupe de wagons stationne) ;
- **Apres decouplage, les deux trains executent chacun leur propre ordonnancement** : via des mecanismes comme le « saut conditionnel de commande », les deux trains apres decouplage suivent des plans d'ordonnancement differents ;
- **Transfert de consignes** : Lors du decouplage/attelage, les consignes (etiquettes de restriction de suivi) sont correctement transferees ou dedupliquees avec le train.

## Capacites centrales

| Capacite | Description |
|---|---|
| Ordres d'attelage/decouplage | Nouveaux types d'ordres, supportant les contraintes « charger/ne pas charger », « attendre attelage », « decoupler » |
| Limite de longueur et vitesse d'attelage | Limite les operations d'attelage selon la longueur du train ; les operations d'attelage ont une limitation de vitesse |
| Support de double locomotive | Locomotives avant et arriere (incluant les locomotives doubles NewGRF) correctement couplees/decouplees |
| Pathfinding d'attelage | YAPF/NPF supportent tous deux la planification de chemin d'attelage ; attelage uniquement en gare |
| Marche arriere | Possible apres attelage, supporte les techniques d'ordonnancement comme « marche arriere apres decouplage » |
| Attelage NewGRF | Supporte les proprietes d'attelage definies par NewGRF (proprietes 0xC6/0xF2, etc.) |
| Archive | Version d'archive `SLV_ORDER_DECOUPLE` ; compteur `num_decouple` persistant |

## Utilisation

1. En gare, utilisez un ordre de « decouplage » (`decouple`) pour diviser un train ;
2. Definissez un ordre d'attelage pour que le train attende la connexion ;
3. Combinez avec le « saut conditionnel de commande » (conditional order skip) pour que les deux trains apres decouplage suivent des itineraires differents ;
4. En gare, faites glisser pour ajuster l'ordre des wagons, ou utilisez des outils auxiliaires comme le « filtrage par longueur ».

## Code associe

- Ordres : `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_type.h`
- Train : `src/train_cmd.cpp`, `src/train.h`
- Pathfinding : `src/pathfinder/yapf`, `src/pathfinder/npf`
- Archive : `src/sl/saveload_common.h` (`SLV_ORDER_DECOUPLE`)

## Remarque

- Cette fonctionnalite implique des modifications profondes de l'ordre physique des trains et de la logique de reservation des signaux, **il est recommande de tester en priorite apres compilation reelle** : decouplage, attelage, marche arriere, reparation de collision, remplacement automatique (autoreplace), etc. ;
- Les chaines et GUI associes au decouplage ont ete integres (`STR_DECOUPLE*` etc.).

## Mise à jour de fusion amont 2026-09-28 (px-patch 2609.x)

113 nouveaux commits de pulsexlb ont été fusionnés, concernant le découplage/accouplement :

- L'état de revendication d'accouplement est sauvegardé (bloc CPLM + XSLFI_COUPLE_CLAIM_STATE) ;
- Un train peut poursuivre son accostage même si d'autres véhicules occupent le bloc cible hors de la direction d'accouplement ;
- Le demi-tour en gare s'assure que le bloc est libre ;
- Corrections : propulsion après découplage, numéro d'unité avec primary != first, reconversion automatique en gare, etc. ;
- Corrections de restitution des étiquettes (tracerestrict) après découplage/accouplement ;
- Les commandes de quai avec chargement/déchargement et horaire sont copiées intégralement lors du découplage ;
- Correction du crash quand la condition pré-accouplement change ; l'horaire principal n'est plus supprimé par erreur ;
- Panneau d'information pour trains sans moteur ; (jgrpp 0.73.3) glisser-déposer des ordres et double-clic.
