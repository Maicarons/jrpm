---
title: 命令记录与重放
---

# 命令记录与重放（Command Record / Replay）

cmclient 命令重放移植（第五批，提交 `f113acce28`）。**不移植 cmclient 的命令对象层**——直接用 jrpm 自带的命令序列化基础设施（`DynBaseCommandContainer`），把"命令对象层"整个绕开。

## 控制台命令

```
cmdrecord [start [file]]    # 开始记录（默认文件 cmdrecord.jrcm，存于个人目录）
cmdrecord stop              # 停止（延迟 10 tick 等命令队列排空后落盘）
cmdreplay <file>            # 回放：反序列化并立即执行每条命令
```

## 实现要点

- **记录钩子**：`CommandRecordLog` 挂在 `DoCommandPInternal` 的命令执行之后——这是本地、网络、回放命令的**唯一真实执行点**，不会重复记录。
- **序列化**：`cmd / tile / error_msg / payload / company` 用 `DynBaseCommandContainer::Serialise` 无损序列化；文件格式 `JRCM` magic + version + count + 条目。
- **延迟停止**：`cmdrecord stop` 标记延迟 10 tick，`StateGameLoop` 每帧检查（`CommandRecordTick`）确保已入队的命令也被捕获后才 flush。
- **回放执行**：每条命令以服务器命令路径（`DCIF_NETWORK_COMMAND`）立即执行，回放文件即使 count 头未更新也能按实际数据读取。

## 验证

专用服务器 E2E：记录 `pause` → 26 字节记录文件 → 新地图回放 → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`。

## 典型用法

```
cmdrecord start build1     # 开始记录
# ... 在游戏里铺铁路、建车站 ...
cmdrecord stop             # 停止并落盘
# 换地图或崩溃恢复后：
cmdreplay build1           # 一键重建全部操作
```
