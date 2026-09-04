---
title: Grabacion y reproduccion de comandos
---

# Grabacion y reproduccion de comandos (Command Record / Replay)

Portabilidad de la reproduccion de comandos de cmclient (quinta tanda, commit `f113acce28`). **No se porta la capa de objetos de comando de cmclient** -- se usa directamente la infraestructura de serializacion de comandos nativa de jrpm (`DynBaseCommandContainer`), omitiendo toda la "capa de objetos de comando".

## Comandos de consola

```
cmdrecord [start [file]]    # Inicia la grabacion (archivo predeterminado cmdrecord.jrcm, en el directorio personal)
cmdrecord stop              # Detiene (espera 10 ticks a que se vacie la cola de comandos antes de escribir)
cmdreplay <file>            # Reproduce: deserializa y ejecuta cada comando inmediatamente
```

## Puntos clave de implementacion

- **Gancho de grabacion**: `CommandRecordLog` se coloca despues de la ejecucion del comando en `DoCommandPInternal` -- este es el **unico punto de ejecucion real** para comandos locales, de red y de reproduccion, no se graban duplicados.
- **Serializacion**: `cmd / tile / error_msg / payload / company` se serializa sin perdida con `DynBaseCommandContainer::Serialise`; formato de archivo: magic `JRCM` + version + count + entradas.
- **Detencion retardada**: `cmdrecord stop` marca una demora de 10 ticks, `StateGameLoop` verifica cada frame (`CommandRecordTick`) para asegurar que los comandos ya en cola tambien se capturen antes de hacer flush.
- **Ejecucion de reproduccion**: Cada comando se ejecuta inmediatamente como ruta de comando de servidor (`DCIF_NETWORK_COMMAND`), incluso si el encabezado de count del archivo de reproduccion no esta actualizado, se lee segun los datos reales.

## Verificacion

Servidor dedicado E2E: grabar `pause` → archivo de 26 bytes → reproducir en nuevo mapa → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## Uso tipico

```
cmdrecord start build1     # Inicia la grabacion
# ... en el juego, construye vias, estaciones ...
cmdrecord stop             # Detiene y escribe el archivo
# Despues de cambiar de mapa o recuperarse de un fallo:
cmdreplay build1           # Reconstruye todas las operaciones con un clic
```