# openttd-jrpm 服务器性能调优指南

> 基于 merge 后的 jrpm 分支（含 pulsexlb 特性与 jrpm 专属优化）。
> 参数默认值取自 `src/table/settings/network_settings.ini`，建议以实际压测为准。

## 一、已落地的性能优化（本次）

| 优化 | 提交 | 说明 |
|---|---|---|
| 并行下载并发数可配置 | 4716b925 | `network.content_download_parallel`（默认 4，1-8）：内容下载同时进行的文件数，替代硬编码 4 |
| 多镜像 + 文件级并行下载 | d4c45740（F1） | `network.content_mirrors` 逗号分隔多镜像，失败按镜像重试，最终回退旧协议 |
| 服务器多版本客户端兼容 | cb9848b7 | 服务器可同时接 jrpm / 原版 jgrpp / pulsexlb 客户端 |

## 二、服务器性能关键参数（jgrpp 体系）

| 设置 | 默认 | 范围 | 含义 | 调优建议 |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | 每多少帧做一次帧同步校验（desync 检测）。越大越省带宽，越小越早发现失步 | 联机稳定优先可降到 20-50；发现频繁 desync 时调低 |
| `network.frame_freq` | 0 | 0-100 | 服务器每多少帧打包发送一次命令帧（0 = 每帧发送）。越大越省带宽/CPU，但操作延迟上升 | 常规 0-3；人多的服务器可到 5，压测后权衡 |
| `network.commands_per_frame` | 2 | 1-65535 | 每帧处理的客户端命令数上限（防刷屏/恶意） | 玩家多、操作频繁时可上调到 4-8 |
| `network.commands_per_frame_server` | 16 | 1-65535 | 服务器自身每帧命令数上限 | 一般无需改 |
| `network.bytes_per_frame` | 8 | 1-65535 | 长期平均每帧接收字节数上限（带宽整形） | 宽带好可上调到 16-32，提升大图同步速度 |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | 突发字节上限（允许短时峰值） | 配合上项上调，如 512 |
| `network.max_init_time` | 60 | 0-32000 | 客户端初始化超时（tick） | 弱网玩家多可放宽 |
| `network.max_join_time` | 500 | 0-32000 | 客户端加入（下载地图+同步）超时（tick） | 大图/慢速带宽下放宽到 1000+ |
| `network.max_download_time` | 1000 | 0-32000 | 地图下载超时（tick） | 大图（4096+）建议 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | 客户端最大延迟容忍（tick） | 高延迟玩家多时放宽，但会加大 desync 风险 |

> 注意：`max_*_time` 单位为游戏 tick（1/74 秒≈13.5ms）；`bytes_per_frame` 指每帧同步窗口的平均字节。

## 三、推荐的服务器起步配置（多人联机场景）

```ini
[network]
max_clients = 32            ; 或按需
max_companies = 15
frame_freq = 3              ; 0=每帧发命令（最流畅）；3=折中省带宽
sync_freq = 50              ; 更早发现失步
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; 客户端侧内容下载并发
```

## 四、后续可落地的性能优化方向（按优先级）

1. **HTTP 传输层线程池 + Range 分块下载**（Openttd-Cluster 0007 思路）
   现状：F1 已实现「文件级并行」（多个文件同时下载）。0007 是在 HTTP 层做线程池 + `CURLOPT_RANGE` 分块，对**单个大文件**（巨型场景 .tar.gz）还能再提速。两者互补但改动同一批文件，建议在 F1 稳定后增量做。

2. **地图发送优化**
   验证 jrpm 是否默认启用 map 压缩（zstd/lzma）；大图可对比不同压缩算法耗时。

3. **Rust 服务器运行时（远期架构参考）**
   Openttd-Cluster 的 otc-engine（Admin/RCON、Prometheus 指标、集群 failover、Web 面板、快照桥）依赖完整 Rust 工程 FFI 集成，属「下一代服务器」级改造，非补丁级合并；jrpm 当前保持纯 C++ 单一二进制。

4. **游戏模拟性能**
   - 大图 + 大量车辆时关注 `economy`/`linkgraph` 参数（linkgraph_settings.ini）；
   - 若需极限性能，可研究 `settings_game.economy.` 类与 pathfinder 上限参数（`pathfinding` 设置）。

## 五、验证建议

- 开服 `openttd-jrpm -D -c server.cfg`，用多个 jrpm 客户端实测：加入时间、帧同步稳定性（无 desync 提示）、CPU/内存占用；
- 内容下载：设置 `content_download_parallel=8` 对比下载一批 NewGRF 的耗时；
- 弱网压测：`max_lag_time` 与 `sync_freq` 组合测试，找到稳定边界。
