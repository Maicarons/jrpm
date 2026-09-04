---
title: Adicionar Comando do Jogo
---

# Adicionar Comando do Jogo

Usando o `Commands::AutoGroupSharedOrders` (agrupamento automatico de veiculos) implementado neste projeto como modelo, apresenta as etapas completas para adicionar um novo comando no jrpm.

## 1. Declarar na Enumeracao de Comandos

`src/command_type.h`, insercao no meio do `enum class Commands` (desloca IDs subsequentes):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. Declarar Manipulador de Comando e Registro

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` declara simultaneamente a funcao manipuladora e registra `CommandTraits`):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: Sem parametro de tile (com tile use `DEF_CMD_TUPLE`);
- `CmdDataT<...>`: Lista de tipos de parametros, o manipulador desempacota na ordem.

## 3. Implementar o Manipulador

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
    if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

    CommandCost total_cost;
    for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
        // ... logica de negocios ...
        CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
        if (ret.Failed()) return ret;
        total_cost.AddCost(ret.GetCost());
    }
    return total_cost;
}
```

Pontos importantes:
- Retornar `CMD_ERROR` indica falha (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` controla "execucao real" vs "modo de teste";
- Comandos aninhados usam `Command<Commands::X>::Do(flags, ...)`;
- Chamadas de GUI/rede usam `Command<Commands::X>::Post(err_string, args...)` (sincronizacao de rede automatica).

## 4. Adicionar Entrada

### Botao de Janela (opcional)

1. `src/widgets/<system>_widget.h` adicionar `WID_XXX` na enumeracao;
2. Adicionar botao no layout NWidget da janela;
3. Adicionar `case` no `OnClick`:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### Comando de Console (opcional)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// Em IConsoleStdLibRegister():
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### Strings (opcional)

`src/lang/english.txt` adicionar (ex: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. Lista de Verificacao

- [ ] Enumeracao em `command_type.h`
- [ ] `DEF_CMD_TUPLE*` em `<system>_cmd.h`
- [ ] Manipulador em `<system>_cmd.cpp`
- [ ] (Opcional) Widget/Console/Strings
- [ ] Adaptacao automatica de rede e arquivo (`CommandTraits` gerado automaticamente)