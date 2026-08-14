---
title: 联机指南
---

# 联机指南

## 版本兼容策略

jrpm 是 **tagged 版本**（`jrpm-0.1.0`），联机握手要求修订串**完全匹配**：

| 场景 | 行为 |
|---|---|
| jrpm 客户端 ↔ jrpm 服务器 | ✅ 正常联机（版本一致） |
| jrpm 客户端 ↔ 原版 jgrpp / pulsexlb 服务器 | ❌ 拒绝（版本隔离） |
| 原版 jgrpp 客户端 → jrpm 服务器 | ✅ 允许（服务器端放宽接受 `jgrpp-*` 修订） |
| pulsexlb 客户端 → jrpm 服务器 | ✅ 允许（接受 `pxp` 修订） |

::: warning NewGRF 版本
无论客户端来自哪个版本，**NewGRF 版本号必须与服务器完全一致**（`_openttd_newgrf_version` 严格校验），这是确定性模拟的底线。
:::

## 开设服务器

```bash
# 专用服务器（无 GUI）
openttd-jrpm -D -c server.cfg
```

`server.cfg` 建议配置（详见[服务器性能调优](../performance/server-tuning)）：

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

## 加入服务器

- 游戏内「多人游戏 → 加入互联网服务器/添加服务器」，或
- 命令行：`openttd-jrpm -n <host>:<port>`

## 内容下载（NewGRF/场景）

jrpm 支持**多镜像 + 并行下载**：

```ini
[network]
content_server = content.openttd.org        ; 元数据服务器
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; 并行下载文件数（1-8）
```

- 镜像列表逗号分隔，按顺序使用；某个镜像失败自动切换下一个；
- 环境变量 `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` 优先级高于设置项；
- 全部镜像失败后自动回退到旧版 TCP 下载协议。

## 服务器管理

- 控制台命令：`status`、`clients`、`kick`、`ban`、`save`、`reset_company`、`autogroup`（车辆自动分组）等；
- RCON：服务器设置 `rcon_password` 后可从远程发送控制台命令。
