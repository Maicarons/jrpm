---
title: Etude des limites du serveur
---

## Conclusion de recherche : les deux limites sont structurelles

### Limite de joueurs en ligne = 255 (contrainte materielle du protocole)

| Emplacement | Contenu |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` transmis avec **`Send_uint8` / `Recv_uint8`** dans `SerializeNetworkGameInfo` (lignes 251-296) et la deserialisation (lignes 422-432) |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, defaut 25) |
| `src/network/network_server.cpp:360` | Verification d'acceptation `_network_clients_connected < MAX_CLIENTS` |

**Conclusion** : 255 est deja la limite du champ uint8 du protocole. Pour la depasser, il faudrait changer les champs `clients_max` et apparentes en uint16 -- ceci modifierait le protocole client↔serveur, la diffusion UDP et le protocole du **Game Coordinator**, constituant un changement de protocole externe (jouable entre deux extremites du fork, mais la compatibilite avec la liste des serveurs publics serait affectee).

### Limite du nombre d'entreprises = 15 (contrainte materielle du format de stockage des tuiles)

| Emplacement | Contenu |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner` : `SB(_m[tile].m1, 0, 5, owner.base())` -- le proprietaire de tuile ne stocke que **5 bits** (bits 0-4 de `_m[].m1`) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`, `OWNER_NONE{0x10}`, `OWNER_WATER{0x11}`, `OWNER_DEITY{0x12}`, `OWNER_END{0x13}` partagent le meme espace d'octet que les ID d'entreprise |

**Conclusion** : `Owner` et `CompanyID` sont le meme type, le champ de propriete de tuile n'a que 5 bits (valeur 0-31), dont 15-18 sont occupes par des propriétaires speciaux. Ainsi, la limite reelle de 15 entreprises est determinee par le **format du tableau de la carte** -- pour la depasser, il faudrait elargir le stockage du proprietaire de tuile (`m1` complet sur 8 bits ou modifier la structure `_m`) et migrer les constantes `OWNER_*`, impliquant **une conversion complete des archives** et une augmentation de la memoire de la carte, constituant une refonte majeure du format (c'est pourquoi OpenTTD amont ne l'a pas fait depuis des annees).

## Options possibles

| Option | Modification | Compatibilite | Suggestion |
|---|---|---|---|
| A. Maintenir l'etat actuel | Aucune | Archive/protocole/coordinateur completement compatibles | ✅ Recommande : 255 clients / 15 entreprises depasse deja largement l'original pour le multijoueur |
| B. Client → protocole uint16 | Modifier `network_game_info.cpp` serialisation/deserialisation en `Send_uint16/Recv_uint16` ; porter `MAX_CLIENTS` a 4095 ; changer `max_clients` dans `network_settings.ini` en SLE_UINT16 ; changer le champ dans `settings_type.h` en uint16 ; `console_cmds.cpp:1056` largeur d'affichage | Jouable entre deux extremites du fork ; coordinateur public/anciens clients incompatibles | Optionnel, si besoin reel de >255 clients |
| C. Entreprise → refonte du format de tuile | Elargir le champ proprietaire de tuile + migration des constantes `OWNER_*` + conversion complete des archives | Changement de format d'archive (conversion unique) | Deconseille a court terme |

## Contenu livre

- Ce document de conception (avec fichiers/lignes exacts) ;
- Patch optionnel `option-clients-uint16.diff` (modifications completes de l'option B, **non applique**, a examiner avec `git apply`).

> Conclusion preliminaire : **L'extension des deux limites est essentiellement une mise a niveau de format/protocole, pas un ajustement de parametre** ; dans le respect de la compatibilite des archives et du reseau public, les limites actuelles 255/15 sont les limites maximales. Si le cout de compatibilite est acceptable, les options B/C sont modifiables comme ci-dessus.