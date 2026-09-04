---
title: Guia de ajuste de rendimiento del servidor
---

> Basado en la rama jrpm post-fusion (incluyendo caracteristicas de pulsexlb y optimizaciones exclusivas de jrpm).
> Los valores predeterminados de los parametros provienen de `src/table/settings/network_settings.ini`, se recomienda basarse en pruebas de carga reales.

## I. Optimizaciones de rendimiento implementadas (esta version)

| Optimizacion | Commit | Descripcion |
|---|---|---|
| Concurrencia de descarga paralela configurable | 4716b925 | `network.content_download_parallel` (predeterminado 4, 1-8): numero de archivos de descarga simultanea de contenido, reemplaza el valor fijo de 4 |
| Descarga paralela a nivel de archivo + multiples espejos | d4c45740 (F1) | `network.content_mirrors` multiples espejos separados por comas, reintenta por espejo en caso de fallo, finalmente recurre al protocolo antiguo |
| Compatibilidad multiversion de clientes del servidor | cb9848b7 | El servidor puede aceptar simultaneamente clientes jrpm / jgrpp original / pulsexlb |

## II. Parametros clave de rendimiento del servidor (sistema jgrpp)

| Configuracion | Predeterminado | Rango | Significado | Sugerencia de ajuste |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | Cada cuantos frames hacer una verificacion de sincronizacion de frames (deteccion de desync). Cuanto mayor, menos ancho de banda; cuanto menor, antes se detecta la perdida de sincronizacion | Si la estabilidad de la conexion es prioritaria, se puede reducir a 20-50; si se detectan desyncs frecuentes, reducir |
| `network.frame_freq` | 0 | 0-100 | Cada cuantos frames el servidor empaqueta y envia un frame de comando (0 = enviar cada frame). Cuanto mayor, menos ancho de banda/CPU, pero la latencia de operacion aumenta | Normal 0-3; en servidores con muchos jugadores se puede subir a 5, ajustar tras pruebas de carga |
| `network.commands_per_frame` | 2 | 1-65535 | Limite maximo de comandos de cliente procesados por frame (anti-spam/malicioso) | Con muchos jugadores y operaciones frecuentes, se puede aumentar a 4-8 |
| `network.commands_per_frame_server` | 16 | 1-65535 | Limite maximo de comandos del propio servidor por frame | Generalmente no necesita cambios |
| `network.bytes_per_frame` | 8 | 1-65535 | Limite maximo de bytes recibidos por frame en promedio a largo plazo (modelado de ancho de banda) | Con buena banda ancha, se puede aumentar a 16-32, mejora la velocidad de sincronizacion de mapas grandes |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | Limite maximo de bytes en rafagas (permite picos cortos) | Ajustar junto con el anterior, ej. 512 |
| `network.max_init_time` | 60 | 0-32000 | Tiempo de espera de inicializacion del cliente (ticks) | Con muchos jugadores de red debil, se puede ampliar |
| `network.max_join_time` | 500 | 0-32000 | Tiempo de espera de entrada del cliente (descarga de mapa + sincronizacion) (ticks) | Con mapas grandes/banda ancha lenta, ampliar a 1000+ |
| `network.max_download_time` | 1000 | 0-32000 | Tiempo de espera de descarga de mapa (ticks) | Mapas grandes (4096+) recomiendan 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | Tolerancia maxima de latencia del cliente (ticks) | Con muchos jugadores de alta latencia, ampliar, pero aumenta el riesgo de desync |

> Nota: Las unidades de `max_*_time` son ticks de juego (1/74 segundos ≈ 13.5ms); `bytes_per_frame` se refiere al promedio de bytes de la ventana de sincronizacion por frame.

## III. Configuracion inicial recomendada del servidor (escenario multijugador)

```ini
[network]
max_clients = 32            ; o segun necesidad
max_companies = 15
frame_freq = 3              ; 0=enviar comando cada frame (mas fluido); 3=compromiso para ahorrar ancho de banda
sync_freq = 50              ; detectar perdida de sincronizacion antes
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; concurrencia de descarga de contenido del lado del cliente
```

## IV. Direcciones de optimizacion de rendimiento futuras (por prioridad)

1. **Pool de hilos de capa de transporte HTTP + descarga por fragmentos Range** (idea de Openttd-Cluster 0007)
   Estado actual: F1 ya implemento "paralelismo a nivel de archivo" (multiples archivos descargados simultaneamente). 0007 agrega un pool de hilos en la capa HTTP + `CURLOPT_RANGE` para fragmentos, lo que puede acelerar **archivos grandes individuales** (escenarios gigantes .tar.gz). Ambos son complementarios pero modifican los mismos archivos, se recomienda hacerlo de forma incremental despues de que F1 este estable.

2. **Optimizacion de envio de mapas**
   Verificar si jrpm tiene habilitada la compresion de mapas de forma predeterminada (zstd/lzma); los mapas grandes pueden comparar el tiempo de diferentes algoritmos de compresion.

3. **Runtime de servidor Rust (arquitectura futura a largo plazo)**
   El otc-engine de Openttd-Cluster (Admin/RCON, metricas de Prometheus, failover de cluster, panel web, puente de snapshots) depende de la integracion FFI de un proyecto Rust completo, pertenece a una transformacion de "servidor de nueva generacion", no a una fusion a nivel de parche; jrpm actualmente mantiene un unico binario C++ puro.

4. **Rendimiento de simulacion del juego**
   - En mapas grandes con muchos vehiculos, prestar atencion a los parametros de `economy`/`linkgraph` (linkgraph_settings.ini);
   - Si se necesita rendimiento extremo, se pueden estudiar los parametros de `settings_game.economy.` y los limites del pathfinder (configuracion de `pathfinding`).

## V. Sugerencias de verificacion

- Abrir servidor `openttd-jrpm -D -c server.cfg`, probar con multiples clientes jrpm: tiempo de entrada, estabilidad de sincronizacion de frames (sin indicaciones de desync), uso de CPU/memoria;
- Descarga de contenido: configurar `content_download_parallel=8` y comparar el tiempo de descarga de un lote de NewGRF;
- Pruebas de carga con red debil: probar combinaciones de `max_lag_time` y `sync_freq` para encontrar el limite estable.