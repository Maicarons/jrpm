---
title: Guia multijugador
---

# Guia multijugador

## Estrategia de compatibilidad de versiones

jrpm es una **version tagged** (`jrpm-0.1.0`), el handshake de conexion requiere que la cadena de revision **coincida exactamente**:

| Escenario | Comportamiento |
|---|---|
| Cliente jrpm ↔ Servidor jrpm | ✅ Conexion normal (version coincidente) |
| Cliente jrpm ↔ Servidor jgrpp / pulsexlb original | ❌ Rechazado (aislamiento de version) |
| Cliente jgrpp original → Servidor jrpm | ✅ Permitido (el servidor acepta revisiones `jgrpp-*`) |
| Cliente pulsexlb → Servidor jrpm | ✅ Permitido (acepta revisiones `pxp`) |

::: warning Version de NewGRF
Independientemente de la version del cliente, **el numero de version de NewGRF debe coincidir exactamente con el del servidor** (validacion estricta de `_openttd_newgrf_version`),这是底线 de la simulacion determinista.
:::

## Configurar un servidor

```bash
# Servidor dedicado (sin GUI)
openttd-jrpm -D -c server.cfg
```

Configuracion recomendada para `server.cfg` (ver [ajuste de rendimiento del servidor](../performance/server-tuning)):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## Unirse a un servidor

- En el juego: "Multijugador → Unirse a servidor de Internet/Anadir servidor", o
- Linea de comandos: `openttd-jrpm -n <host>:<port>`

## Descarga de contenido (NewGRF/Escenarios)

jrpm soporta **multiples espejos + descarga paralela**:

```ini
[network]
content_server = content.openttd.org        ; Servidor de metadatos
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; Numero de archivos a descargar en paralelo (1-8)
```

- Lista de espejos separada por comas, usados en orden; si un espejo falla, cambia automaticamente al siguiente;
- Las variables de entorno `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` tienen prioridad sobre las configuraciones;
- Si todos los espejos fallan, vuelve automaticamente al protocolo de descarga TCP antiguo.

## Administracion del servidor

- Comandos de consola: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (agrupacion automatica de vehiculos), etc.;
- RCON: configure `rcon_password` en el servidor para enviar comandos de consola de forma remota.