---
title: Anadir nuevo comando del juego
---

# Anadir nuevo comando del juego

Usando el `Commands::AutoGroupSharedOrders` (agrupacion automatica de vehiculos) implementado en este proyecto como plantilla, se presentan los pasos completos para anadir un nuevo comando en jrpm.

## 1. Declarar en la enumeracion de comandos

`src/command_type.h`, insertar en medio de `enum class Commands` (desplazara los IDs posteriores):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. Declarar el manejador del comando y el registro

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` declara simultaneamente la funcion del manejador y registra `CommandTraits`):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: sin parametro de tile (con tile usar `DEF_CMD_TUPLE`);
- `CmdDataT<...>`: lista de tipos de parametros, el manejador desempaqueta en orden.

## 3. Implementar el manejador

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... logica de negocio ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

Puntos clave:
- Devolver `CMD_ERROR` indica fallo (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` controla entre "ejecucion real" y "modo de prueba";
- Los comandos anidados usan `Command<Commands::X>::Do(flags, ...)`;
- Las llamadas desde GUI/red usan `Command<Commands::X>::Post(err_string, args...)` (sincronizacion de red automatica).

## 4. Anadir puntos de entrada

### Boton de ventana (opcional)

1. Anadir `WID_XXX` en la enumeracion de `src/widgets/<system>_widget.h`;
2. Anadir boton en el diseno NWidget de la ventana;
3. Anadir `case` en `OnClick`:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### Comando de consola (opcional)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// En IConsoleStdLibRegister():
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### Cadenas de texto (opcional)

Anadir en `src/lang/english.txt` (ej. `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. Lista de verificacion

- [ ] Enumeracion en `command_type.h`
- [ ] `DEF_CMD_TUPLE*` en `<system>_cmd.h`
- [ ] Manejador en `<system>_cmd.cpp`
- [ ] (Opcional) Controles/consola/cadenas
- [ ] Adaptacion automatica de red y archivo (`CommandTraits` se genera automaticamente)