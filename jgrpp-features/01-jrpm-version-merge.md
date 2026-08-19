# openttd-jrpm：版本合并总览

> 分支：`jrpm` ｜ 版本：jrpm-0.1.0（tagged，2026-08-14）
> 构建产物：`openttd-jrpm`（可执行文件名）

## 版本关系

```
                        jgrpp-0.73.1 (共同祖先)
                        /                 \
        jgrpp 分支(63 提交)          pulsexlb px-patch (152 提交)
        ├ tracerestrict 等近期更新      ├ jgrpp-decouple（机车换挂）
        ├ 我的 5 功能 (d4c45740)        └ jgrpp-multitile-airport（模块化机场）
        └ 版本改名 jrpm-0.1.0 (425e7207)
                        \                 /
                        jrpm 分支 (merge 71fe214c + 兼容 cb9848b)
```

## 合并内容

### 1. pulsexlb/OpenTTD-patches（px-patch 全量 152 提交）→ 已合并

| 功能 | 说明 | 主要文件 |
|---|---|---|
| **机车换挂（decouple）** | 列车解挂/挂载：解挂订单、路签转移、耦合长度/车速限制、双车头、NewGRF 耦合、耦合寻路（YAPF/NPF）、解挂后两列车独立调度 | train_cmd.cpp、order_cmd.cpp、order_gui.cpp、train.h、yapf/npf |
| **模块化机场（multitile-airport）** | 多格机场系统重构：air 类型体系（air.h/air_type.h/newgrf_airtype.*）、PBS 空中调度（pbs_air.*）、YAPF 航空寻路、`station.allow_modify_airports`（改造机场布局）、`gui.default_air_type`、多格机场精灵 | air.*、pbs_air.*、aircraft_cmd.cpp（3600 行重构）、airport_cmd/gui、station_cmd |

冲突处理：仅 2 个头文件冲突（aircraft.h / airport.h）——pulsexlb 航空重构删除了工作区**无引用**的死类型（`VehicleAirFlags` bitset、`AirportMovingDataFlag`），取 pulsexlb 侧删除，已确认无其他文件引用。

### 2. Openttd-Cluster（用户 Rust 集群项目）→ 选择性借鉴

| 补丁 | 处理 | 说明 |
|---|---|---|
| 0006 vanilla-native-server（多版本客户端兼容） | ✅ **已合并**（cb9848b7） | jrpm 服务器同时接受 jrpm / 原版 jgrpp（`jgrpp-`）/ pulsexlb（`pxp`）客户端；NewGRF 版本仍严格校验 |
| 0001 revision-handshake / 0005 version-metadata | ✅ 思路已采用 | jrpm 用独立 tagged 修订串 `jrpm-0.1.0`，联机握手与 jgrpp/pxp 隔离，实现「全新版本方便联机」 |
| 0007 parallel-download（HTTP 线程池 + Range 分块，30KB） | 📝 参考不合并 | 与本项目 F1「文件级并行 + 多镜像」同主题且改动同一批文件；0007 的**传输层线程池/分块下载**记录为 F1 后续增强方向 |
| 0002-0004 snapshot/command/FFI 桥 | 📝 架构参考 | 依赖整个 otc-engine Rust 运行时（FFI 静态链入），属「远期整体集成」而非补丁级合并；jrpm 当前保持纯 C++ 单一二进制 |

### 3. jrpm 专属（前序 5 功能，d4c45740）→ 已在 jrpm 分支内

并行下载（多镜像+4 并发会话）、自动分组、建造 tooltip、整局感知 AI（ScriptGlobal + GlobalAI）、镜像/内容服务器设置。

## 联机策略（「全新版本方便联机」）

- jrpm 为 **tagged 版本**：`IsNetworkCompatibleVersion` 要求修订串完全匹配 → **jrpm 客户端只与 jrpm 服务器联机**，与 jgrpp 0.73.x / pxp 完全隔离；
- **服务器放宽**：jrpm 服务器额外接受 `jgrpp-*` 与 `pxp*` 客户端加入（`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`，NewGRF 版本必须一致）；
- 因此：jrpm 服主开服 = 只接 jrpm 玩家（默认）；需要兼容老客户端时无需改配置即可接 jgrpp/pxp 玩家。

## 构建与验证（用户本机执行）

```bash
# 首次（需要 CMake + 依赖，参考 COMPILING.md）
cmake -B build ..
cmake --build build -j
# 产物: build/openttd-jrpm.exe
```

验证优先级：
1. `openttd-jrpm -v` 显示 `jrpm-0.1.0`；
2. 单机开档跑 1-2 年（合并涉及 train/airport 大改 + 存档版本可能因 allow_modify_airports 等新增而 bump）；
3. 开服后：jrpm 客户端加入 ✓；原版 jgrpp 0.73.x 客户端尝试加入（预期可进，NewGRF 一致时）；
4. 机车换挂：给列车加解挂/挂载订单，验证解挂后两列车独立调度；模块化机场：开启 `station.allow_modify_airports` 后改造机场布局；
5. 前序 5 功能回归（并行下载、自动分组、建造 tooltip、GlobalAI）。

## 已知风险

- **未编译验证**：合并+改造的代码未在本机编译（无工具链），真机首编可能有遗漏的接口变动（尤其 aircraft/airport/train 三系大改）；
- 存档版本：px-patch 可能 bump 了 SLV（M9 提到 savegame version gate），jrpm 存档与 jgrpp 0.73.x 存档可能不互读（同 jgrpp 惯例，向下兼容 trunk 存档）；
- merge 带入 pulsexlb 全部历史，如需追溯功能归属用 `git log --oneline pulsexlb/px-patch`。
