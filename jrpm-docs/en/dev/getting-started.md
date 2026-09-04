---
title: Development Guide
---

# Development Guide

This document explains how to continue development on jrpm. All patterns are based on actual code implemented in this project (cross-reference with commits in `git log`).

## Code Organization Quick Reference

| Directory | Responsibility |
|---|---|
| `src/` | All C++ source code (top-level named by system: `rail_cmd.cpp`, `group_gui.cpp`...) |
| `src/network/` | Networking (server/client/UDP/HTTP/content download) |
| `src/script/` | Script framework (Squirrel); `api/` for AI/GS API classes |
| `src/table/settings/*.ini` | **Setting definition sources** (settingsgen generates code) |
| `src/lang/english.txt` | String definitions (strgen generates) |
| `src/sl/saveload_common.h` | Save version (SLV) enum |
| `bin/ai/` | AI scripts (`GlobalAI` is an example) |

## Common Development Patterns

- [Adding a Game Command](./add-command): Command enum + `DEF_CMD_TUPLE_NT` + handler + GUI/console entry point
- [Adding a Script API](./add-script-api): `script_*.hpp/.cpp` (auto-registers Squirrel bindings)
- [Adding a Setting](./add-setting): `.ini` + `settings_type.h` field + string

## Building

```bash
cmake -B build ..
cmake --build build -j
```

## Development Notes

1. **Encoding**: When modifying source code/documentation containing Chinese characters, ensure UTF-8 (the Write/Edit tool on this machine may produce GBK encoding; use `jgrpp-features/_fix_utf8.py` to fix);
2. **Commit messages**: Recommended to use English (to avoid terminal encoding issues);
3. **Inserting into command enum mid-sequence** shifts subsequent command IDs — old binary versions will be inconsistent with new versions in multiplayer; this is normal within a fork, upgrade must be synchronized;
4. **Save version**: When modifying save structure, add a new `SLV_*` entry in `src/sl/saveload_common.h` and update `SAVEGAME_VERSION`;
5. **New files**: `.cpp` must be added to the corresponding `CMakeLists.txt` source list (`src/CMakeLists.txt` or subdirectory/`script/api/CMakeLists.txt`); `script_*.hpp` is auto-discovered by `file(GLOB)`, no registration needed;
6. **New strings**: Add to `src/lang/english.txt` (other languages fall back to English);
7. **NewGRF/save compatibility**: Changing `_openttd_content_version` / NewGRF version number needs caution (affects multiplayer and content compatibility).

## Regression Testing Suggestions (Important)

The current branch **has not been compiled on a real machine**. After the first build, it is recommended to test in this order:
1. Basic: `openttd-jrpm -v` shows `jrpm-0.1.0`;
2. Save: single-player run for 1-2 game years;
3. Multiplayer: jrpm clients connect to each other; jgrpp/pxp clients join;
4. New features: train decoupling (decouple/couple/reverse), modular airports (modify layout), parallel download, auto-grouping, build tooltip, GlobalAI;
5. Legacy feature regression: signals, tracerestrict, scheduled dispatch, template replacement.