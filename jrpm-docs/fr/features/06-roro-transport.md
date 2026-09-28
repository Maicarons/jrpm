---
title: Transport de véhicules routiers (RoRo)
---

# Transport de véhicules routiers (RoRo : Road-vehicle on Road-vehicle)

> Source : pulsexlb/OpenTTD-patches `px-patch` (lot de septembre 2026), fusionné dans jrpm via git merge.

## Vue d'ensemble

Le « transport de véhicules routiers » permet aux trains, navires et avions de **transporter directement des véhicules routiers** :

- Les véhicules routiers n'ont plus besoin de rouler partout par eux-mêmes : ils peuvent « se faire porter » — un porteur les emmène vers une gare lointaine, où ils redescendent sur la route ;
- Le porteur (train/navire/avion) obtient la capacité de transporter des véhicules routiers en étant **reconverti vers le fret « Véhicules (route) »** (fret dédié `VEHC`, qui occupe l'espace de fret) ;
- Côté véhicule routier, les options d'ordre « **Attendre d'être transporté** » et « **Débarquer ici** » s'apparient aux options « charger des véhicules routiers » / « décharger des véhicules routiers » du porteur.

## Utilisation

### Côté porteur (train/navire/avion)

1. Reconvertir le train dans un dépôt **manuellement** vers le fret « Véhicules (route) » — devenir porteur n'est possible que par reconversion manuelle ; la reconversion par ordre ne s'applique pas ;
2. Activer « **Charger des véhicules routiers** » sur un ordre de gare : le train embarque les véhicules en attente à cette gare ;
3. Options d'appariement facultatives :
   - « Attendre le chargement » (partir seulement chargé) ;
   - « Correspondance de destination » : n'embarquer que les véhicules dont la gare de débarquement déclarée correspond au prochain arrêt du porteur ;
   - « Débarquer ici tous les véhicules » : tout débarquer, en ignorant la gare déclarée par chacun.

### Côté véhicule routier

1. Définir « **Attendre d'être transporté** » sur un ordre de gare : le véhicule s'y arrête et attend un porteur ;
2. Définir « **Débarquer ici** » : le véhicule descend du porteur à cette gare ;
3. Les deux options sont mutuellement exclusives (une par ordre) ;
4. En débarquant, le véhicule lance une passe de recherche de chemin pour choisir la meilleure plateforme.

## Détails et règles

- **Case de fret dédiée** : le transport de véhicules routiers utilise la case de fret 128 (`NUM_CARGO - 1`), hors des 64 cases définissables par un NewGRF ; le nombre total de types de fret passe de 64 à **128** ;
- **Détection de porteur dédié** : lorsque toutes les parties d'un véhicule sont reconverties en « Véhicules (route) », ses boutons d'ordre affichent par défaut le transport de véhicules routiers ; les véhicules transportant du fret normal affichent le fret normal ;
- **Réglage des parties porteuses** : `vehicle.rv_transport_carrier_parts` décide quelles parties peuvent transporter des véhicules routiers ;
- **Chargement inter-compagnies** : autoriser facultativement le chargement/déchargement des véhicules d'autres compagnies avec règlement automatique des frais ;
- **Avertissement « transporté trop longtemps »** : un avertissement unique lorsqu'un véhicule a été porté trop longtemps ;
- **Listes d'ordres créées par le joueur** : les listes partagées/indépendantes prennent également en charge les indicateurs de transport de véhicules routiers.

## Compatibilité des sauvegardes

- L'état d'attente/transport et les indicateurs d'ordre sont persistés ;
- Les anciennes sauvegardes (sans l'indicateur XSLFI_CARGO_TYPES_128) sont lues avec 64 cases de fret et restent compatibles.

## Commandes console de débogage (désactivées par défaut)

La famille de commandes `rvtransport` n'est compilée qu'avec l'option CMake `RORO_DEBUG_COMMANDS=ON`, pour les tests de régression.

## Code associé

- Noyau : `src/roadveh_transport.h`, `src/cargo_type.h` (fret VEHC)
- Ordres : `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_base.h` (OrderExtraInfo)
- Chargement des trains : `src/train_cmd.cpp`, `src/station_cmd.cpp`
- Extension du fret : `src/sl/station_sl.cpp`, `src/sl/company_sl.cpp` (XSLFI_CARGO_TYPES_128)
