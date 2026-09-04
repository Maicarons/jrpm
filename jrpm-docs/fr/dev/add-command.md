---
title: Ajouter une commande de jeu
---

# Ajouter une commande de jeu

En utilisant `Commands::AutoGroupSharedOrders` (groupement automatique de vehicules) implemente dans ce projet comme modele, voici les etapes completes pour ajouter une nouvelle commande dans jrpm.

## 1. Declarer dans l'enumeration des commandes

`src/command_type.h`, `enum class Commands` insertion au milieu (decalera les IDs suivants) :

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. Declarer le processeur de commande et l'enregistrement

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` declare simultanement la fonction processeur et enregistre `CommandTraits`) :

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT` : pas de parametre de tuile (avec tuile utiliser `DEF_CMD_TUPLE`) ;
- `CmdDataT<...>` : liste des types de parametres, le processeur les decompose dans l'ordre.

## 3. Implementer le processeur

`src/group_cmd.cpp` :

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... logique metier ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

Points cles :
- Retourner `CMD_ERROR` indique un echec (`CommandCost(INVALID_STRING_ID)`) ;
- `flags.Test(DoCommandFlag::Execute)` controle l'« execution reelle » vs « mode test » ;
- Les commandes imbriquees utilisent `Command<Commands::X>::Do(flags, ...)` ;
- Les appels GUI/reseau utilisent `Command<Commands::X>::Post(err_string, args...)` (synchronisation reseau automatique).

## 4. Ajouter des points d'entree

### Bouton de fenetre (optionnel)

1. Ajouter `WID_XXX` dans l'enumeration `src/widgets/<system>_widget.h` ;
2. Ajouter le bouton dans la disposition NWidget de la fenetre ;
3. Ajouter un `case` dans `OnClick` :
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### Commande console (optionnel)

`src/console_cmds.cpp` :

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// Dans IConsoleStdLibRegister() :
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### Chaine (optionnel)

Ajouter dans `src/lang/english.txt` (ex. `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. Liste de verification

- [ ] Enumeration dans `command_type.h`
- [ ] `DEF_CMD_TUPLE*` dans `<system>_cmd.h`
- [ ] Processeur dans `<system>_cmd.cpp`
- [ ] (Optionnel) Widget/console/chaine
- [ ] Adaptation reseau et archive automatique (`CommandTraits` genere automatiquement)