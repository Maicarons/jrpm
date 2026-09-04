---
title: Agrupamento Automatico de Veiculos
---

## Situacao Atual (Conclusao da Pesquisa)

- Estrutura de dados de grupo: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- Comandos existentes (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (renomear/definir grupo pai)
  - `CmdAddVehicleGroup` (adicionar veiculo unico ao grupo)
  - `CmdAddSharedVehicleGroup` (adicionar veiculos com **ordens compartilhadas de um veiculo** a um grupo **existente**)
  - `CmdCreateGroupFromList` (criar grupo a partir de lista de veiculos, suporta tipo de lista `VL_SHARED_ORDERS`; nomenclatura automatica)
- Auxiliar de nomenclatura automatica: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, nomeia por cidade de origem/destino da rota `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) -- jgrpp ja possui infraestrutura de "nomenclatura por rota";
- Mecanismo de registro de comando: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (macro declara simultaneamente o manipulador e registra `CommandTraits`);
- Veiculo <-> grupo: `vehicle_base.h` `Vehicle::group_id`; cadeia de ordens compartilhadas `FirstShared()/NextShared()`; `OrderList *orders`.

**Conclusao**: Faltava um comando completo para "percorrer todos os veiculos da empresa, criar grupos automaticamente por ordens compartilhadas e adicionar veiculos a eles".

## Implementacao deste Recurso

### Novo Comando `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Fluxo (`src/group_cmd.cpp`):
1. Percorre todos os veiculos principais do tipo especificado da empresa (`Vehicle::IterateTypeFrontOnly(type)`), pulando veiculos que nao sao da empresa, sem ordens ou ja em grupos personalizados;
2. Para cada cadeia de ordens compartilhadas com contagem >= 2 veiculos compartilhando a mesma lista de ordens:
   - Usa `GenerateAutoNameForVehicleGroup(v)` para gerar o nome do grupo (ex: "CidadeA <-> CidadeB");
   - Constroi `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, chama aninhadamente `Command<Commands::CreateGroupFromList>::Do(flags, ...)` para criar o grupo e mover todos os veiculos compartilhados para ele;
   - Apos a conclusao, os veiculos nesse grupo ja tem `group_id` diferente do padrao, e o loop pula automaticamente (apenas um grupo criado por lista de ordens);
3. `GroupChangeDeferredUpdateScope` unifica a atualizacao atrasada das estatisticas do grupo.

### Tres Entradas

| Entrada | Local | Descricao |
|---|---|---|
| Botao na janela de grupos | `src/group_gui.cpp` novo `WID_GL_AUTOGROUP_SHARED` (ao lado de LIVERY na barra de ferramentas), OnClick envia comando | Reutiliza sprite `SPR_GROUP_CREATE_TRAIN + vtype` |
| Comando de console | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (sem parametro = executa para todos os 4 tipos) | `IConsole::CmdRegister` |
| Script/outros | Qualquer codigo pode usar `Command<Commands::AutoGroupSharedOrders>::Post(...)` | Comando passa pela sincronizacao de rede, seguro em servidor multijogador |

### Arquivos Envolvidos

- `src/command_type.h`: Enum `Commands` adiciona `AutoGroupSharedOrders` (**atencao: insercao no meio do enum desloca IDs de comandos subsequentes, versoes antigas e novas nao serao consistentes em rede, fenomeno normal dentro do fork**)
- `src/group_cmd.h` / `group_cmd.cpp`: Declaracao e implementacao do comando
- `src/widgets/group_widget.h`: Novo ID de widget
- `src/group_gui.cpp`: Botao na barra de ferramentas (NWidget + OnPaint sprite + OnClick)
- `src/console_cmds.cpp`: Comando de console
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Pontos de Verificacao

1. Dois ou mais veiculos compartilhando a mesma lista de ordens -> apos clicar no botao/`autogroup`, um grupo com "nome da rota" e criado e todos os veiculos sao adicionados a ele;
2. Veiculos ja agrupados nao sao movidos novamente; listas de ordens diferentes formam grupos separados;
3. As estatisticas do grupo (quantidade/lucro) sao atualizadas corretamente (depende de `GroupChangeDeferredUpdateScope`).