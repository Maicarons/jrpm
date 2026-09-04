# Cuarta tanda / Quinta tanda / Comandos de servidor CM -- Informe de investigacion de dificultad de portabilidad

> Objeto de investigacion: lectura profunda del codigo fuente de citymania-org/cmclient (rama vanilla 15.3)
> Fecha de investigacion: 2026-08-14
> Conclusion: Las tres areas de trabajo tienen niveles de dificultad muy diferentes, se proporciona evaluacion detallada y orden recomendado.

---

## I. Cuarta tanda: Zonificacion urbana (incluyendo archivo growth_tiles)

### Componentes funcionales

| Componente | Archivo | Tamano | Descripcion |
|---|---|---|---|
| Enumeracion de modos de zonificacion | cm_zoning.hpp | 37 lineas | 12 modos de evaluacion (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| Logica de evaluacion | cm_zoning_cmd.cpp | 413 lineas | Una funcion de consulta por modo (devuelve SpriteID de paleta), incluye algoritmo de radio TownZone/Tz, verificacion de cobertura StationFinder, verificacion de edificios/industrias no servidos |
| GUI de barra de herramientas de zonificacion | cm_zoning_gui.cpp | 204 lineas | Ventana de seleccion de modo de zonificacion interior/exterior (estado global `_zoning.inner/outer`) |
| Archivo de growth_tiles | cm_saveload.cpp/.hpp | 90 lineas | Campos extendidos de Town `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), almacenados con el manejador SaveLoad de vanilla |
| Extension de datos | extensions/cmext_town.hpp | ~80 lineas | `ext::Town`: growth_tiles ×2 + **muchos campos de modo de juego del servidor CM** (CBTownInfo estadisticas de carga, seguimiento de publicidad/fondos, contadores de crecimiento urbano hs/cs/hr) |
| Rotacion mensual + activacion | cm_game.cpp / town_cmd.cpp | -- | NewMonth rotacion de growth_tiles; registro de estado en construccion/demolicion/reconstruccion de casas |

### Evaluacion detallada de dificultades de portabilidad

| Dificultad | Severidad | Descripcion |
|---|---|---|
| **Reescritura del sistema de archivos** | 🟠 Media | growth_tiles usa `DefaultSaveLoadHandler` + `SlSetStructListLength`/`SlObject` de vanilla, jrpm usa el nuevo sistema `sl/` (SlTableHeader/SlObjectSaveFiltered). Necesita reescribirse con NSL/SLE + **control de caracteristica XSLF** de jrpm (nuevo `XSLFI_TOWN_GROWTH_TILES`, version 1), los archivos antiguos no se ven afectados |
| **Extension de estructura Town** | 🟢 Baja | Solo tomar los dos mapas de growth_tiles (**saltar** los campos CBTownInfo/publicidad/fondos -- esos son exclusivos del modo de juego CityBuilder del servidor CM); anadir campos en Town de jrpm + montar tabla de archivo en town_sl.cpp |
| **Logica de evaluacion** | 🟢 Baja-Media | En su mayoria son consultas puras (GetTileType/StationFinder/cache de Town/verificacion de casas), la API de jrpm existe; el algoritmo de radio TownZone (`squared_town_zone_radius`) necesita verificar los nombres de campo de jrpm |
| **Pipeline de renderizado** | 🟠 Media-Alta | `DrawTileZoning` necesita integrarse en el pipeline de dibujo del viewport -- **comparte el mecanismo de renderizado TileHighlight con el resaltado de la tercera tanda**. Sin el pipeline de resaltado, hay que construirlo por separado (se recomienda **hacer primero el resaltado, luego la zonificacion**) |
| **Recursos de sprites** | 🟠 Media | 12 paletas usan sprites personalizados `CM_SPR_PALETTE_ZONING_*`, jrpm no los tiene → necesita reemplazar con paletas existentes o anadir nuevos recursos |
| **Ganchos de eventos** | 🟠 Media | El registro de growth_tiles depende del **bus de eventos** de cmclient (event::HouseBuilt/HouseCleared/..., Emit de cm_main.cpp). jrpm no tiene este mecanismo → anadir ganchos directamente en town_cmd.cpp en construccion/demolicion de casas + rotacion NewMonth (IntervalTimer) |

### Conclusion de dificultad: 🟠 Media-Alta (aproximadamente 1.5-2 rondas especializadas, 4-6 horas cada una)

- **Prerrequisito**: Se recomienda encarecidamente completar primero el **sistema de resaltado** de la tercera tanda (pipeline de renderizado compartido)
- Si solo se hace "coloracion de 12 zonas, sin archivo de growth_tiles": la dificultad baja a 🟡 Media (ahorra la extension de archivo, aproximadamente 1 ronda)

---

## II. Quinta tanda: Reproduccion de comandos + Exportacion/Grabacion

### 2.1 Reproduccion de comandos (cm_command_log + cm_commands + generated)

| Componente | Tamano | Descripcion |
|---|---|---|
| Capa de objetos de comando | cm_command_type.hpp + generated/cm_gen_commands (2251+1418 lineas) | Cada comando vanilla se envuelve en un objeto programable (as_company/with_callback/set_auto), incluye **serializacion comando→flujo de bits** |
| Carga de registro de comandos | cm_command_log.cpp (203 lineas) | Descompresion lzma + analisis de flujo de bits BitOStream → cola `_fake_commands` (contador de tick/resultado esperado/semilla aleatoria/CommandPacket) |
| Ejecucion de comandos | ExecuteFakeCommands | Ejecuta en orden por contador de tick: `ExecuteCommand(&cp)` (API interna de vanilla) + **verificacion de semilla aleatoria/resultado** (antitrampas), en multijugador reenvia a todos los clientes |

**Diferencias clave (determinan la dificultad)**:

| cmclient | jrpm | Impacto |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | No existe esta funcion, el nucleo del comando es `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | Necesita escribir capa de conversion de CommandPacket → payload de DoCommandPInternal 🟠 |
| Campos de CommandPacket (vanilla) | CommandPacket existe pero con estructura diferente (GeneralCommandPacket\<DynBaseCommandContainer\>) | Necesita adaptar mapeo de campos 🟠 |
| `GetCommandName` | ✅ Existe (command_func.h:166) | 🟢 |
| `outgoing_queue` (reenvio multijugador) | La capa de red de jrpm es diferente (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | La reproduccion multijugador necesita reescribir la logica de reenvio 🟠 |
| Grabador (como generar archivos .cmd) | **El gancho de grabacion de cmclient depende de la intercepcion de post() de la capa de objetos de comando** | Los comandos de jrpm son Post templatizado → necesita anadir gancho de grabacion en el punto de distribucion de comandos 🔴 Trabajo central |
| Formato de archivo | Privado (ID de comando vanilla + flujo de bits) | Los IDs de comando de jrpm son completamente diferentes de vanilla → los archivos de reproduccion no son compatibles, **el formato necesita rediseno** 🟠 |

### 2.2 Exportacion / Grabacion (cm_export.cpp, 536 lineas)

| Funcion | Descripcion | Dificultad |
|---|---|---|
| ExportOpenttdData | Exportacion JSON de especificaciones de casas/especificaciones de carga/paletas/informacion de motores (JsonWriter) | 🟢 Baja-Media (independiente, lee estructura Spec y escribe JSON) |
| ViewportExport / ExportFrameSprites | Exportacion cuadro por cuadro de sprites del viewport (depende de los vectores de dibujo internos del viewport TileSpriteToDrawVector/ParentSpriteToSortVector) | 🟠 Media-Alta (el pipeline de viewport de jgrpp es diferente, necesita alinear las interfaces de vectores) |

### Conclusion de dificultad: 🟠 Media-Alta (aproximadamente 2 rondas especializadas)

- **La reproduccion de comandos es la parte mas grande**: Trabajo central = ① anadir **gancho de grabacion** en el punto de distribucion de comandos (adaptacion al sistema de comandos templatizados de jrpm) ② capa de ejecucion de CommandPacket → DoCommandPInternal ③ diseno de nuevo formato de archivo. **Se recomienda portar primero la capa de objetos de comando** (base de los planos de la tercera tanda, tambien base de la reproduccion)
- Exportacion: la exportacion de datos JSON se puede hacer de forma independiente primero (baja-media); la grabacion de cuadros depende de la alineacion del viewport (mismo lote que el resaltado)

---

## III. Comandos de servidor CM (cm_console_cmds.cpp, 289 lineas)

### Lista de comandos y dificultad detallada

| Comando | Funcion | Dificultad para jrpm | Notas |
|---|---|---|---|
| `cmgamespeed [n]` | Cambiar velocidad del juego | 🟢 **Muy baja** (~20 lineas) | jrpm ya tiene `_game_speed` global (gfx.cpp:52), solo falta la envoltura del comando |
| `cmstep [n]` | Avanzar n ticks | ⏭️ **jrpm ya tiene** | El comando `step` (ConStepGame) tiene la misma funcionalidad, se omite |
| `cmexport` | Exportar openttd.json | 🟢 Baja-Media | Depende de ExportOpenttdData (ver quinta tanda) |
| `cmtreemap <file>` | Plantar arboles segun mapa de altura | 🟡 Media | Leer mapa de altura + comando de plantar arboles, independiente |
| `cmreset_town_growth` | Limpiar registros de crecimiento urbano | 🟢 Baja | Depende del campo de zonificacion urbana |
| `cmload_commands` | Cargar reproduccion de comandos | 🟠 Media-Alta | Depende de la infraestructura de reproduccion de comandos |
| `cmstart_record` / `cmstop_record` | Grabacion de cuadros | 🟠 Media-Alta | Depende de la infraestructura de grabacion |
| `cmgamestats` | Estadisticas de sesion de juego | 🟢 Baja | Independiente |
| `cmgfxdebug` | Depuracion grafica | 🟢 Baja | Independiente |

### Conclusion de dificultad: 🟢 Generalmente baja (0.5-2 horas por comando)

**Orden recomendado**: `cmgamespeed` (10 minutos) → `cmgamestats`/`cmgfxdebug` (~1h cada uno) → `cmexport` (junto con la exportacion de la quinta tanda) → `cmtreemap` (~2h) → `cmreset_town_growth` (junto con la cuarta tanda) → `cmload_commands`/`cmstart_record` (junto con la reproduccion/grabacion de la quinta tanda).

---

## IV. Ruta general recomendada

```
① Sistema de resaltado (nucleo de la tercera tanda, ~2-3 rondas)      ← Base de renderizado para planos/zonificacion
② Capa de objetos de comando (base de la tercera tanda planos + quinta tanda reproduccion, ~1-2 rondas)
③ Planos (~1 ronda)
④ Zonificacion urbana (~1.5 rondas, depende de ① pipeline de renderizado; growth_tiles con control XSLF)
⑤ Reproduccion de comandos + exportacion (~2 rondas, depende de ②; exportacion JSON se puede hacer antes de forma independiente)
⑥ Comandos de servidor CM (insertar entre tandas, gamespeed/step se pueden hacer inmediatamente)
```

## V. Elementos que se pueden implementar inmediatamente con bajo costo (sin depender de grandes proyectos)

1. Comando `cmgamespeed` (~20 lineas, cambiar `_game_speed`)
2. `step` ya existe → no es necesario hacerlo
3. `cmgamestats` / `cmgfxdebug` (~1h cada uno, independiente)
4. Exportacion de datos JSON de `cmexport` (~2h, independiente)
5. `cmtreemap` (~2h, independiente)

Estos 5 elementos suman aproximadamente **medio dia de trabajo**, se pueden implementar como "tanda cero", en paralelo con las tandas tres/cuatro/ cinco sin conflicto.