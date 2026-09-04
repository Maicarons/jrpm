# Rapport d'etude de difficulte de portage -- Quatrieme lot / Cinquieme lot / Commandes serveur CM

> Objet d'etude : Lecture approfondie des sources de citymania-org/cmclient (branche vanilla 15.3)
> Date d'etude : 2026-08-14
> Conclusion : Les trois blocs de travail ont des difficultes tres differentes, evaluation par element et ordre recommande ci-dessous.

---

## I. Quatrieme lot : Zonage urbain (avec archive growth_tiles)

### Composition fonctionnelle

| Composant | Fichier | Taille | Description |
|---|---|---|---|
| Enumeration des modes de zonage | cm_zoning.hpp | 37 lignes | 12 modes d'evaluation (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| Logique d'evaluation | cm_zoning_cmd.cpp | 413 lignes | Une fonction de requete par mode (retourne un SpriteID de palette), incluant l'algorithme de rayon TownZone/Tz, verification de couverture StationFinder, verification des batiments/industries non desservis |
| GUI de la barre d'outils de zonage | cm_zoning_gui.cpp | 204 lignes | Fenetre de selection de mode de zonage interne/externe (etats globaux `_zoning.inner/outer`) |
| Archive growth_tiles | cm_saveload.cpp/.hpp | 90 lignes | Champs d'extension Town `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), stockes avec le gestionnaire SaveLoad vanilla |
| Extension de donnees | extensions/cmext_town.hpp | ~80 lignes | `ext::Town` : growth_tiles ×2 + **nombreux champs de jeu serveur CM** (statistiques de marchandises CBTownInfo, suivi publicite/fonds, compteurs de croissance urbaine hs/cs/hr) |
| Rotation mensuelle + declenchement | cm_game.cpp / town_cmd.cpp | -- | Rotation mensuelle NewMonth de growth_tiles ; enregistrement de l'etat aux endroits de construction/demolition/reconstruction de maisons |

### Evaluation des difficultes de portage par point

| Difficulte | Gravite | Description |
|---|---|---|
| **Reecriture du systeme d'archive** | Orange moyen | growth_tiles utilise `DefaultSaveLoadHandler` vanilla + `SlSetStructListLength`/`SlObject`, jrpm a le nouveau systeme `sl/` (SlTableHeader/SlObjectSaveFiltered). Necessite une reecriture avec NSL/SLE + **controle de fonctionnalites XSLF** (nouveau `XSLFI_TOWN_GROWTH_TILES`, version 1), les anciennes archives ne sont pas affectees |
| **Extension de la structure Town** | Vert faible | Prendre seulement les deux maps growth_tiles (**sauter** CBTownInfo/champs publicite/fonds -- ceux-ci sont propres au mode de jeu CityBuilder du serveur CM) ; ajout de champ dans Town jrpm + montage de la table d'archive town_sl.cpp |
| **Logique d'evaluation** | Vert faible-moyen | Principalement des requetes pures (GetTileType/StationFinder/cache Town/verification de maisons), l'API jrpm existe ; l'algorithme de rayon TownZone (`squared_town_zone_radius`) necessite de verifier les noms de champs jrpm |
| **Pipeline de rendu** | Orange moyen-eleve | `DrawTileZoning` doit etre accroche dans le pipeline de rendu viewport -- **partage le mecanisme de rendu TileHighlight avec la surbrillance du troisieme lot**. Sans pipeline de surbrillance, il faut le construire separement (recommande : **faire d'abord la surbrillance, puis le zonage**) |
| **Ressources de sprites** | Orange moyen | 12 palettes utilisent des **sprites personnalises** `CM_SPR_PALETTE_ZONING_*`, jrpm ne les a pas → necessite de les remplacer par des sprites de palette existants ou d'ajouter des ressources |
| **Crochets d'evenements** | Orange moyen | L'enregistrement growth_tiles depend du **bus d'evenements** de cmclient (event::HouseBuilt/HouseCleared/..., Emit dans cm_main.cpp). jrpm n'a pas ce mecanisme → ajouter directement des crochets aux endroits de construction/demolition de maisons dans town_cmd.cpp + rotation NewMonth (IntervalTimer) |

### Conclusion de difficulte : Orange moyen-eleve (environ 1.5-2 sessions specialisees, 4-6 heures chacune)

- **Prerequis** : Fortement recommande de terminer d'abord le **systeme de surbrillance** du troisieme lot (pipeline de rendu partage)
- Si seulement « 12 modes de zonation colores, sans archive growth_tiles » : difficulte reduite a Jaune moyen (economie de l'extension d'archive, environ 1 session)

---

## II. Cinquieme lot : Rejeu de commandes + Exportation/Enregistrement

### 2.1 Rejeu de commandes (cm_command_log + cm_commands + generated)

| Composant | Taille | Description |
|---|---|---|
| Couche d'objet de commande | cm_command_type.hpp + generated/cm_gen_commands (2251+1418 lignes) | Chaque commande vanilla est encapsulee en objet programmable (as_company/with_callback/set_auto), incluant **serialisation commande→bitstream** |
| Chargement du journal de commandes | cm_command_log.cpp (203 lignes) | Decompression lzma + analyse BitOStream → file `_fake_commands` (tick counter/resultat attendu/graine aleatoire/CommandPacket) |
| Execution de commandes | ExecuteFakeCommands | Execute selon l'ordre tick counter : `ExecuteCommand(&cp)` (API interne vanilla) + **verification de graine aleatoire/resultat** (anti-triche), transmet a tous les clients en multijoueur |

**Differences cles (determinant la difficulte)** :

| cmclient | jrpm | Impact |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | Pas cette fonction, le noyau de commande est `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | Necessite d'ecrire une couche de conversion CommandPacket → DoCommandPInternal payload Orange |
| Champs CommandPacket (vanilla) | CommandPacket existe mais structure differente (GeneralCommandPacket\<DynBaseCommandContainer\>) | Necessite d'adapter le mapping des champs Orange |
| `GetCommandName` | ✅ Existe (command_func.h:166) | Vert |
| `outgoing_queue` (transmission multijoueur) | La couche reseau jrpm est differente (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | Le rejeu multijoueur necessite de reecrire la logique de transmission Orange |
| Enregistreur (comment generer le fichier .cmd) | **Le crochet d'enregistrement de cmclient depend de l'interception post() de la couche d'objet de commande** | Les commandes jrpm sont des Post templatees → necessite d'ajouter un crochet d'enregistrement au point de distribution des commandes Rouge travail central |
| Format de fichier | Prive (ID de commande vanilla + bitstream) | Les IDs de commande jrpm sont completement differents de vanilla → fichier de rejeu incompatible, **format a reconcevoir** Orange |

### 2.2 Exportation/Enregistrement (cm_export.cpp, 536 lignes)

| Fonctionnalite | Description | Difficulte |
|---|---|---|
| ExportOpenttdData | Exportation JSON des specifications de maisons/marchandises/palettes/informations moteur (JsonWriter) | Vert faible-moyen (independant, lire les structures Spec et ecrire du JSON) |
| ViewportExport / ExportFrameSprites | Exportation trame par trame des sprites du viewport (depend des vecteurs internes de rendu viewport TileSpriteToDrawVector/ParentSpriteToSortVector) | Orange moyen-eleve (le pipeline viewport jgrpp est different, necessite d'aligner les interfaces vectorielles) |

### Conclusion de difficulte : Orange moyen-eleve (environ 2 sessions specialisees)

- **Le rejeu de commandes est le plus gros morceau** : Travail central = 1. Ajouter un **crochet d'enregistrement** au point de distribution des commandes (adaptation du systeme de commandes templatees de jrpm) 2. Couche d'execution CommandPacket → DoCommandPInternal 3. Conception du nouveau format de fichier. **Recommande de porter d'abord la couche d'objet de commande** (fondation du troisieme lot plans, et aussi du rejeu)
- Exportation : L'exportation de donnees JSON peut etre faite independamment en premier (faible-moyen) ; l'enregistrement de trames depend de l'alignement viewport (a traiter avec le meme lot que la surbrillance)

---

## III. Commandes serveur CM (cm_console_cmds.cpp, 289 lignes)

### Liste des commandes et difficulte par commande

| Commande | Fonctionnalite | Difficulte pour jrpm | Remarques |
|---|---|---|---|
| `cmgamespeed [n]` | Changer la vitesse du jeu | Vert **Tres faible** (~20 lignes) | jrpm a deja le global `_game_speed` (gfx.cpp:52), manque seulement l'enveloppe de commande |
| `cmstep [n]` | Avancer de n ticks | ⏭️ **jrpm a deja** | Commande `step` (ConStepGame), fonctionnalite identique, sautee |
| `cmexport` | Exporter openttd.json | Vert faible-moyen | Depend de ExportOpenttdData (voir cinquieme lot) |
| `cmtreemap <file>` | Planter des arbres sur carte de hauteur | Jaune moyen | Lecture de carte de hauteur + commande de plantation d'arbres, independant |
| `cmreset_town_growth` | Vider les enregistrements de croissance urbaine | Vert faible | Depend des champs de zonage urbain |
| `cmload_commands` | Charger un rejeu de commandes | Orange moyen-eleve | Depend de l'infrastructure de rejeu de commandes |
| `cmstart_record` / `cmstop_record` | Enregistrement de trames | Orange moyen-eleve | Depend de l'infrastructure d'enregistrement |
| `cmgamestats` | Statistiques de session de jeu | Vert faible | Independante |
| `cmgfxdebug` | Debogage graphique | Vert faible | Independante |

### Conclusion de difficulte : Vert Globalement faible (0.5-2 heures par commande)

**Ordre recommande** : `cmgamespeed` (10 minutes) → `cmgamestats`/`cmgfxdebug` (~1h chacun) → `cmexport` (avec le cinquieme lot d'exportation) → `cmtreemap` (~2h) → `cmreset_town_growth` (avec le quatrieme lot) → `cmload_commands`/`cmstart_record` (avec le cinquieme lot rejeu/enregistrement).

---

## IV. Feuille de route globale recommandee

```
1. Systeme de surbrillance (cœur du troisieme lot, ~2-3 sessions)      ← Fondation de rendu pour plans/zonage
2. Couche d'objet de commande (fondation du troisieme lot plans + cinquieme lot rejeu, ~1-2 sessions)
3. Plans (~1 session)
4. Zonage urbain (~1.5 sessions, depend du pipeline de rendu 1. ; growth_tiles avec controle XSLF)
5. Rejeu de commandes + Exportation (~2 sessions, depend de 2. ; l'exportation JSON peut etre faite independamment en avance)
6. Commandes serveur CM (a inserer entre les lots, gamespeed/step peuvent etre faits immediatement)
```

## V. Elements realisables immediatement a faible cout (ne dependent d'aucun grand projet)

1. Commande `cmgamespeed` (~20 lignes, modifie `_game_speed`)
2. `step` deja existant → pas besoin de le faire
3. `cmgamestats` / `cmgfxdebug` (~1h chacun, independants)
4. Exportation de donnees JSON de `cmexport` (~2h, independante)
5. `cmtreemap` (~2h, independant)

Ces 5 elements representent environ **une demi-journee de travail**, peuvent etre realises comme « lot zero » en premier, sans conflit avec les lots trois/quatre/cinq.