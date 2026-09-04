---
title: "Resource Download: Multi-Mirror + Parallel Download"
---

## Current Status (Research Findings)

| Item | Current Status |
|---|---|
| Content server (metadata protocol) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: environment variable `OTTD_CONTENT_SERVER_CS`, default `content.openttd.org` (TCP port 3978) |
| Download mirror | `NetworkContentMirrorUriString()`: environment variable `OTTD_CONTENT_MIRROR_URI`, default `https://binaries.openttd.org/bananas` |
| Download method | `network_content.cpp` `DownloadSelectedContentHTTP()`: a **single** POST request with all content IDs → mirror returns a file header list (`id,type,filesize,url` per line) → **serial download** of each file one by one (one GET connection per file) → `AfterDownload()` gunzip + tar extraction |
| Thread model | HTTP layer (WinHttp) runs in background threads, but **file downloads are serialized**; no mirror list, no in-game settings, no parallelism |

## This Feature Implementation

### 1. New Settings (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, default empty = use official source)
- `network.content_mirrors` (SLE_STR, comma-separated multiple mirror URIs, default empty = official mirror)

Priority: environment variable > game setting > official default.

### 2. Configuration Parsing (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` reads the setting;
- New `NetworkContentMirrorUris()` parses comma-separated list (trim whitespace, skip empty items, fallback to official source);
- `NetworkContentMirrorUriString()` changed to return the first item in the list.

### 3. Parallel Download (`src/network/network_content.h/.cpp`)

- New `ContentFileDownload` (single pending download file: id/type/filesize/url/filename);
- New `ContentDownloadSession : HTTPCallback` (independent download state and callback per file; `IsCancelled` links to handler);
- `DownloadSelectedContentHTTP()`: POST to `mirrors[mirror_index]` → `ParseResponseHeaders()` parses all file headers at once → `StartDownloadSessions()` starts up to **4 parallel sessions** (`CONTENT_DOWNLOAD_PARALLEL`), each session picks the next pending file upon completion;
- Failure retry chain: session failure/mirror request failure → next mirror retry request → all mirrors failed → fallback to legacy TCP protocol (`DownloadSelectedContentFallback`);
- Graceful cleanup: on failure/cancel, set `download_cancelled`, wait for all in-flight sessions to finish (`OnAllSessionsDone`) before retry or fallback, avoiding dangling callbacks;
- `ResetMirrorIndex()`: new downloads start from the first mirror (called when triggered from GUI).

## Involved Files

- `src/table/settings/network_settings.ini` (2 new settings)
- `src/settings_type.h` (2 new fields in `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (mirror list parsing)
- `src/network/network_content.h` / `.cpp` (parallel sessions)
- `src/network/network_content_gui.cpp` (reset mirror index before download)

## Verification Points

1. After filling `network.content_mirrors` with multiple URIs (comma-separated), content downloads should work and download multiple files **in parallel** (observe download progress/capture multiple concurrent connections);
2. Network failure/wrong mirror should automatically try the next mirror, finally fallback to legacy protocol;
3. Cancelling during download should not leave `.tar.gz` partial files;
4. Must be verified on real machine compilation (this repo has no build environment, code not compiled).

## Known Limitations

- Parallelism hardcoded to 4 (`CONTENT_DOWNLOAD_PARALLEL`), can be made a setting later;
- Mirror protocol depends on the official bananas API format (POST id list returns file header list).