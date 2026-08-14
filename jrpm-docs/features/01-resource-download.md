---
title: 资源下载：多镜像 + 并行下载
---

## 现状（调研结论）

| 项 | 现状 |
|---|---|
| 内容服务器（元数据协议） | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`：环境变量 `OTTD_CONTENT_SERVER_CS`，默认 `content.openttd.org`（TCP 3978 端口） |
| 下载镜像 | `NetworkContentMirrorUriString()`：环境变量 `OTTD_CONTENT_MIRROR_URI`，默认 `https://binaries.openttd.org/bananas` |
| 下载方式 | `network_content.cpp` `DownloadSelectedContentHTTP()`：**单个** POST 请求所有 content ID → 镜像返回文件头列表（`id,type,filesize,url` 每行）→ **逐个串行**下载每个文件（每个文件一个 GET 连接）→ `AfterDownload()` gunzip + tar 解包 |
| 线程模型 | HTTP 层（WinHttp）本身在后台线程处理，但**文件下载是串行排队**的；无镜像列表、无游戏内设置、无并行 |

## 本功能实现

### 1. 新增设置（`network_settings.ini` + `settings_type.h`）

- `network.content_server`（SLE_STR，默认空 = 用官方源）
- `network.content_mirrors`（SLE_STR，逗号分隔多个镜像 URI，默认空 = 官方镜像）

优先级：环境变量 > 游戏设置 > 官方默认。

### 2. 配置解析（`src/network/core/config.cpp/h`）

- `NetworkContentServerConnectionString()` 读取设置；
- 新增 `NetworkContentMirrorUris()` 解析逗号分隔列表（去空白、跳过空项、兜底官方源）；
- `NetworkContentMirrorUriString()` 改为返回列表第一个。

### 3. 并行下载（`src/network/network_content.h/.cpp`）

- 新增 `ContentFileDownload`（单个待下载文件：id/type/filesize/url/filename）；
- 新增 `ContentDownloadSession : HTTPCallback`（每个文件的独立下载状态与回调；`IsCancelled` 联动 handler）；
- `DownloadSelectedContentHTTP()`：POST 到 `mirrors[mirror_index]` → `ParseResponseHeaders()` 一次性解析全部文件头 → `StartDownloadSessions()` 启动最多 **4 个并行会话**（`CONTENT_DOWNLOAD_PARALLEL`），每个会话完成后自动领取下一个待下载文件；
- 失败重试链：会话失败/镜像请求失败 → 下一个镜像重新请求 → 全部镜像失败 → 回退旧 TCP 协议（`DownloadSelectedContentFallback`）；
- 优雅收尾：失败/取消时置 `download_cancelled`，等在途会话全部结束（`OnAllSessionsDone`）后再重试或回退，避免悬垂回调；
- `ResetMirrorIndex()`：新下载从第一个镜像开始（GUI 触发时调用）。

## 涉及文件

- `src/table/settings/network_settings.ini`（新增 2 项设置）
- `src/settings_type.h`（`NetworkSettings` 新增 2 字段）
- `src/network/core/config.cpp` / `config.h`（镜像列表解析）
- `src/network/network_content.h` / `.cpp`（并行会话）
- `src/network/network_content_gui.cpp`（下载前重置镜像索引）

## 验证要点

1. `network.content_mirrors` 填多个 URI（逗号分隔）后，内容下载应能工作并**并行**下载多个文件（观察下载进度/抓包多个并发连接）；
2. 断网/错误镜像下应自动尝试下一镜像，最后回退旧协议；
3. 下载中取消不应残留 `.tar.gz` 半成品；
4. 需在真机编译验证（本仓库无构建环境，代码未编译）。

## 已知限制

- 并行度硬编码为 4（`CONTENT_DOWNLOAD_PARALLEL`），后续可做成设置；
- 镜像协议依赖官方 bananas API 格式（POST id 列表返回文件头列表）。
