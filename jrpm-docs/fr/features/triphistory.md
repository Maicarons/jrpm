---
title: Historique des trajets des vehicules
---

# Historique des trajets des vehicules

Provenant de **embeddedt/OpenTTD-modded** (premier lot de portage), deja adapte aux nouvelles API de chaines et de dates de jrpm.

## Fonctionnalite

Chaque vehicule memorise les donnees des **10 derniers trajets**, consultables via le nouveau bouton **History (Historique)** dans la fenetre de details du vehicule :

| Colonne | Signification |
|---|---|
| Received (Arrivee) | Date de reception de la marchandise pour ce trajet |
| Profit (Profit) | Profit de ce trajet (negatif affiche en jaune) |
| % Change | Pourcentage de changement de profit par rapport au trajet precedent (vert + / rouge -) |
| TBT | Intervalle de temps (jours) depuis le trajet precedent |
| Change | Changement de temps (jours) par rapport au trajet precedent |
| Occupancy | Taux moyen de chargement/passagers pour ce trajet (%) |

En bas de la fenetre, des statistiques récapitulatives :

- **Total income for the last N trips** : Profit total des N trajets + profit moyen par jour par trajet
- **Average trip length** : Intervalle moyen entre les trajets (jours)
- **Improvement over last N trips** : Pourcentage de changement global du profit

## Mecanisme d'enregistrement

- **AddValue** : Enregistre le profit, la date, le taux d'occupation et la distance entre les gares lorsqu'un vehicule termine un transport (destructeur de `CargoPayment`)
- **NewRound** : Commence un nouveau trajet lorsque le vehicule atteint la premiere destination selon l'horaire
- **Taux d'occupation** : Collecte lorsque le vehicule quitte une gare (reutilise le mecanisme `trip_occupancy` existant de jrpm)

## Notes d'implementation

- Les donnees sont **NOSAVE** (execution uniquement, non ecrites dans l'archive), donc pas besoin d'augmenter la version d'archive, les anciennes archives sont entierement compatibles
- Nouvelle classe de fenetre `WindowClass::VehicleTripHistory`, l'ID de fenetre est l'ID du vehicule
- La barre de titre de la fenetre de details du vehicule (train/non-train) a un nouveau bouton History
- La fenetre d'historique se ferme automatiquement lorsque le vehicule est supprime

## Fichiers concernes

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (nouveaux)
- `src/vehicle_base.h` (nouveau champ `trip_history` dans Vehicle)
- `src/economy.cpp` (enregistrement du trajet lors de la destruction de CargoPayment)
- `src/timetable_cmd.cpp` (debut d'un nouveau trajet a l'arrivee a la premiere station)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (bouton History)
- `src/lang/english.txt` (chaines `STR_TRIP_HISTORY_*`)