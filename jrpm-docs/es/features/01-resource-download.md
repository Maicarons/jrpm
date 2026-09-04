---
title: "Descarga de recursos: multiples espejos + descarga paralela"
---

## Estado actual (conclusion de la investigacion)

| Elemento | Estado actual |
|---|---|
| Servidor de contenido (protocolo de metadatos) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: variable de entorno `OTTD_CONTENT_SERVER_CS`, predeterminado `content.openttd.org` (TCP puerto 3978) |
| Espejo de descarga | `NetworkContentMirrorUriString()`: variable de entorno `OTTD_CONTENT_MIRROR_URI`, predeterminado `https://binaries.openttd.org/bananas` |
| Metodo de descarga | `network_content.cpp` `DownloadSelectedContentHTTP()`: una unica solicitud **POST** con todos los content ID → el espejo devuelve una lista de encabezados de archivo (`id,type,filesize,url` por linea) → **descarga secuencial uno por uno** de cada archivo (un GET por archivo) → `AfterDownload()` desempaqueta gunzip + tar |
| Modelo de hilos | La capa HTTP (WinHttp) se ejecuta en un hilo de fondo, pero la **descarga de archivos es en cola secuencial**; no hay lista de espejos, ni configuracion en el juego, ni paralelismo |

## Implementacion de esta funcion

### 1. Nuevas configuraciones (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, predeterminado vacio = usar fuente oficial)
- `network.content_mirrors` (SLE_STR, multiples URI de espejo separados por comas, predeterminado vacio = espejo oficial)

Prioridad: variable de entorno > configuracion del juego > predeterminado oficial.

### 2. Analisis de configuracion (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` lee la configuracion;
- Nueva `NetworkContentMirrorUris()` analiza la lista separada por comas (elimina espacios en blanco, salta elementos vacios, recurre a la fuente oficial);
- `NetworkContentMirrorUriString()` ahora devuelve el primero de la lista.

### 3. Descarga paralela (`src/network/network_content.h/.cpp`)

- Nuevo `ContentFileDownload` (un unico archivo a descargar: id/type/filesize/url/filename);
- Nuevo `ContentDownloadSession : HTTPCallback` (estado de descarga independiente y callback para cada archivo; `IsCancelled` vinculado al handler);
- `DownloadSelectedContentHTTP()`: POST a `mirrors[mirror_index]` → `ParseResponseHeaders()` analiza de una vez todos los encabezados de archivo → `StartDownloadSessions()` inicia hasta **4 sesiones paralelas** (`CONTENT_DOWNLOAD_PARALLEL`), cada sesion toma automaticamente el siguiente archivo a descargar al finalizar;
- Cadena de reintentos ante fallos: fallo de sesion/fallo de solicitud de espejo → siguiente espejo re-solicita → todos los espejos fallan → recurre al protocolo TCP antiguo (`DownloadSelectedContentFallback`);
- Cierre elegante: en caso de fallo/cancelacion, establece `download_cancelled`, espera a que todas las sesiones en curso terminen (`OnAllSessionsDone`) antes de reintentar o recurrir, evitando callbacks colgantes;
- `ResetMirrorIndex()`: nueva descarga comienza desde el primer espejo (se llama cuando se activa desde la GUI).

## Archivos involucrados

- `src/table/settings/network_settings.ini` (2 nuevas configuraciones)
- `src/settings_type.h` (2 nuevos campos en `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (analisis de lista de espejos)
- `src/network/network_content.h` / `.cpp` (sesiones paralelas)
- `src/network/network_content_gui.cpp` (restablecer indice de espejo antes de descargar)

## Puntos de verificacion

1. Despues de completar `network.content_mirrors` con multiples URI (separados por comas), la descarga de contenido deberia funcionar y descargar **en paralelo** multiples archivos (observar progreso de descarga/capturar multiples conexiones concurrentes);
2. Sin red/con espejo erroneo, deberia intentar automaticamente el siguiente espejo y finalmente recurrir al protocolo antiguo;
3. Cancelar durante la descarga no deberia dejar archivos `.tar.gz` incompletos;
4. Se debe verificar con compilacion en maquina real (este repositorio no tiene entorno de construccion, el codigo no esta compilado).

## Limitaciones conocidas

- El grado de paralelismo esta codificado como 4 (`CONTENT_DOWNLOAD_PARALLEL`), se puede convertir en configuracion en el futuro;
- El protocolo de espejo depende del formato de la API oficial de bananas (POST con lista de ids devuelve lista de encabezados de archivo).