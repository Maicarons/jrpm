---
title: Interface IA consciente de la partie
---

## Etat actuel (conclusions de recherche)

- Framework NoAI : `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) -- le systeme AI original est **conserve tel quel**, cette fonctionnalite n'y touche pas ;
- Systeme d'API de script : `src/script/api/script_*.hpp/.cpp` (60+ classes), les liaisons Squirrel sont **automatiquement generees** par les outils de construction :
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` decouvre automatiquement les nouvelles classes d'API (genere `ai_*.sq.hpp` / `gs_*.sq.hpp`), **aucun enregistrement manuel necessaire** pour les nouvelles classes ;
  - Les `.cpp` doivent etre ajoutes a la liste source CMake ; le commentaire de classe `@api ai game` controle l'exposition a AI/GS ;
- Donnees d'entreprise : `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()` ;
- Mode divin GS : `ScriptCompanyMode::IsDeity()` ;
- Limitation actuelle : L'AI par defaut ne peut acceder qu'aux donnees de sa propre entreprise, pas d'API globale d'agregation, pas d'interrupteur d'acces.

**Conclusion** : Ajouter une classe d'API `ScriptGlobal` (enregistrement automatique) + un interrupteur `game.script.allow_global_ai_access` suffit pour realiser « conserver NoAI, ajouter une IA consciente de la partie, avec controle d'acces ».

## Implementation de cette fonctionnalite

### 1. API `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

Methodes statiques exposees a AI et GS (`@api ai game`) :

| Methode | Retour | Source de donnees |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| interrupteur de parametre` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` compte par proprietaire |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- L'ID d'entreprise reutilise `ScriptCompany::CompanyID` (COMPANY_SELF resolu en entreprise actuelle) ;
- **Controle d'acces** : Toutes les methodes verifient `IsGlobalAccessAllowed()` en premiere ligne, retournent -1/nullopt/false si non satisfait (GS toujours disponible, AI controlee par l'interrupteur) ;
- `ScriptGlobalCompanyList : ScriptList` : `ScriptList::FillList<Company>` enumere toutes les entreprises.

### 2. Interrupteur d'acces (parametre)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, defaut false, `SC_EXPERT`) ;
- Champ de structure `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`) ;
- Chaine `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. AI d'exemple : `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController) ;
- Demonstration : lit toutes les donnees financieres/vehicules/gares/evaluations et informations cartographiques de toutes les entreprises et les enregistre dans le journal ; explique le controle d'acces (avertit lorsque l'interrupteur n'est pas active) ;
- Cette AI constitue l'implementation de reference pour « percevoir l'ensemble du jeu + controle d'acces », la logique de controle AI ulterieure peut etre etendue dans sa methode `Start()`.

### Fichiers concernes

- Nouveaux `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (ajout du .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- Nouveaux `bin/ai/GlobalAI/info.nut`, `main.nut`

## Points de verification

1. La nouvelle AI est visible dans l'interface de configuration AI (scanne `ai/GlobalAI`) ;
2. Quand `game.script.allow_global_ai_access` n'est pas active, le journal AI indique l'absence de permission ; une fois activee, elle affiche les donnees completes de chaque entreprise ;
3. GameScript peut utiliser `GSGlobal` sans interrupteur ;
4. La generation automatique de `ai_global.sq.hpp` / `gs_global.sq.hpp` se fait sans erreur (depend de `file(GLOB)`, necessite de reconfigurer CMake).

## Directions d'extension

- Ajouter plus de donnees agregees economiques/prets/infrastructures (`GetCompanyInfrastructure`) ;
- Ajouter un abonnement aux evenements (faillite/rachat/creation d'entreprise) ;
- Rendre la logique de decision AI configurable (`GetSettings()`).