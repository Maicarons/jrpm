---
title: Agrupacion automatica de vehiculos
---

## Estado actual (conclusion de la investigacion)

- Estructura de datos de grupos: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- Comandos existentes (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (renombrar/establecer grupo padre)
  - `CmdAddVehicleGroup` (anadir un solo vehiculo a un grupo)
  - `CmdAddSharedVehicleGroup` (anadir vehiculos con **ordenes compartidas de un vehiculo** a un grupo **existente**)
  - `CmdCreateGroupFromList` (crear grupo desde una lista de vehiculos, soporta tipo de lista `VL_SHARED_ORDERS`; nombrado automatico)
- Asistente de nombrado automatico: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, nombre por ciudades de origen/destino de la ruta `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) -- jgrpp ya tiene la infraestructura de "nombrado por ruta";
- Mecanismo de registro de comandos: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (macro que declara simultaneamente el manejador y registra `CommandTraits`);
- Vehiculo↔Grupo: `vehicle_base.h` `Vehicle::group_id`; cadena de ordenes compartidas `FirstShared()/NextShared()`; `OrderList *orders`.

**Conclusion**: Falta un comando integral que "recorra todos los vehiculos de la empresa, cree grupos automaticamente segun ordenes compartidas y los agrupe".

## Implementacion de esta funcion

### Nuevo comando `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Flujo (`src/group_cmd.cpp`):
1. Recorre todos los vehiculos principales del tipo especificado de esa empresa (`Vehicle::IterateTypeFrontOnly(type)`), saltando los que no son de esta empresa, no tienen ordenes o ya estan en un grupo personalizado;
2. Cuenta cada cadena de ordenes compartidas; cuando ≥2 vehiculos comparten la misma lista de ordenes:
   - Usa `GenerateAutoNameForVehicleGroup(v)` para generar el nombre del grupo (ej. "Ciudad A ↔ Ciudad B");
   - Construye `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, llama anidado a `Command<Commands::CreateGroupFromList>::Do(flags, ...)` para crear el grupo y mover todos los vehiculos compartidos a el;
   - Una vez completado, los vehiculos en ese grupo tienen `group_id` diferente del grupo predeterminado, el bucle los salta automaticamente (solo se crea un grupo por lista de ordenes);
3. `GroupChangeDeferredUpdateScope` unifica la actualizacion diferida de las estadisticas del grupo.

### Tres puntos de entrada

| Entrada | Ubicacion | Descripcion |
|---|---|---|
| Boton en ventana de grupos | `src/group_gui.cpp` nuevo `WID_GL_AUTOGROUP_SHARED` (junto a LIVERY en la barra de herramientas), OnClick envia el comando | Sprite reutilizado `SPR_GROUP_CREATE_TRAIN + vtype` |
| Comando de consola | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (sin parametro = ejecutar para los 4 tipos) | `IConsole::CmdRegister` |
| Script/otro | Cualquier codigo puede usar `Command<Commands::AutoGroupSharedOrders>::Post(...)` | El comando pasa por sincronizacion de red, seguro en servidores multijugador |

### Archivos involucrados

- `src/command_type.h`: Enum `Commands` anade `AutoGroupSharedOrders` (**Nota: insertar en medio de la enumeracion desplaza los IDs de comandos posteriores, lo que hara que las versiones antiguas y nuevas no coincidan al conectarse, es normal dentro del fork**)
- `src/group_cmd.h` / `group_cmd.cpp`: Declaracion e implementacion del comando
- `src/widgets/group_widget.h`: Nuevo ID de control
- `src/group_gui.cpp`: Boton de barra de herramientas (NWidget + OnPaint sprite + OnClick)
- `src/console_cmds.cpp`: Comando de consola
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Puntos de verificacion

1. Dos o mas vehiculos comparten la misma lista de ordenes → al hacer clic en el boton/`autogroup` se genera un grupo con nombre de "ruta" y todos los vehiculos se agrupan;
2. Los vehiculos ya agrupados no se mueven de nuevo; diferentes listas de ordenes forman sus propios grupos;
3. Las estadisticas del grupo (numero/ganancia) se actualizan correctamente (depende de `GroupChangeDeferredUpdateScope`).