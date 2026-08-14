---
title: 开发指南
---

# 开发指南

本文档说明如何在 jrpm 上继续开发。所有模式均来自本项目实际实现的代码（可对照 `git log` 中的提交）。

## 代码组织速览

| 目录 | 职责 |
|---|---|
| `src/` | 全部 C++ 源码（顶层按系统命名：`rail_cmd.cpp`、`group_gui.cpp`…） |
| `src/network/` | 联网（服务器/客户端/UDP/HTTP/内容下载） |
| `src/script/` | 脚本框架（Squirrel）；`api/` 为 AI/GS API 类 |
| `src/table/settings/*.ini` | **设置定义源**（settingsgen 生成代码） |
| `src/lang/english.txt` | 字符串定义（strgen 生成） |
| `src/sl/saveload_common.h` | 存档版本（SLV）枚举 |
| `bin/ai/` | AI 脚本（`GlobalAI` 为示例） |

## 常用开发模式

- [新增游戏命令](./add-command)：命令枚举 + `DEF_CMD_TUPLE_NT` + 处理器 + GUI/控制台入口
- [新增脚本 API](./add-script-api)：`script_*.hpp/.cpp`（自动注册 Squirrel 绑定）
- [新增设置项](./add-setting)：`.ini` + `settings_type.h` 字段 + 字符串

## 构建

```bash
cmake -B build ..
cmake --build build -j
```

## 开发注意事项

1. **编码**：含中文的源码/文档改动后注意 UTF-8（本机 Write/Edit 工具曾产生 GBK 编码，可用 `jgrpp-features/_fix_utf8.py` 修复）；
2. **提交信息**：建议英文（规避终端编码问题）；
3. **命令枚举中段插入**会使后续命令 ID 顺移——旧版本二进制与新版本联机会不一致，属 fork 内正常现象，升级需同步；
4. **存档版本**：改动存档结构时在 `src/sl/saveload_common.h` 新增 `SLV_*` 条目并更新 `SAVEGAME_VERSION`；
5. **新文件**：`.cpp` 必须加入对应 `CMakeLists.txt` 源列表（`src/CMakeLists.txt` 或子目录/`script/api/CMakeLists.txt`）；`script_*.hpp` 由 `file(GLOB)` 自动发现，无需注册；
6. **新字符串**：在 `src/lang/english.txt` 添加（其他语言缺省回退英文）；
7. **NewGRF/存档兼容**：改 `_openttd_content_version` / NewGRF 版本号需谨慎（影响联机与内容兼容）。

## 回归建议（重要）

当前分支**未经真机编译**，首次构建后建议按此顺序回归：
1. 基础：`openttd-jrpm -v` 显示 `jrpm-0.1.0`；
2. 存档：单机开档跑 1-2 年；
3. 联机：jrpm 客户端互连；jgrpp/pxp 客户端加入；
4. 新特性：机车换挂（解挂/挂载/反向）、模块化机场（改造布局）、并行下载、自动分组、建造 tooltip、GlobalAI；
5. 旧特性回归：信号、tracerestrict、scheduled dispatch、模板替换。
