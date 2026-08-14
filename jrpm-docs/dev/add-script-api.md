---
title: 新增脚本 API
---

# 新增脚本 API

以本项目实现的 `ScriptGlobal`（整局感知 AI API）为模板，介绍在 jrpm 新增一个脚本 API 类（对 AI 和 GameScript 可见）的完整步骤。

## 背景

- 脚本 API 类位于 `src/script/api/`：`script_<名字>.hpp`（声明）+ `script_<名字>.cpp`（实现）；
- `.hpp` 由构建期 `file(GLOB script_*.hpp)` **自动发现**并生成 Squirrel 绑定（`ai_*.sq.hpp` / `gs_*.sq.hpp`），**无需手工注册类**；
- `.cpp` 需加入 `src/script/api/CMakeLists.txt` 源列表。

## 1. 头文件 `script_global.hpp`

```cpp
/** @file script_global.hpp 文档说明。 */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * 类文档，必须标注 @api。
 * @api ai game        # 对 AI 与 GS 都暴露；"game" 仅 GS；"-ai" 排除 AI
 */
class ScriptGlobal : public ScriptObject {
public:
	/** 枚举会被导出为类常量（如 AIGlobal.VT_TRAIN）。 */
	enum VehicleType {
		VT_TRAIN = ::VehicleType::Train,
		VT_AIRCRAFT = ::VehicleType::Aircraft,
	};

	/** @api ai game */
	static bool IsGlobalAccessAllowed();

	/** @api ai game */
	static SQInteger GetCompanyCount();

	/** @api ai game */
	static std::optional<std::string> GetCompanyName(ScriptCompany::CompanyID company);
};

#endif /* SCRIPT_GLOBAL_HPP */
```

## 2. 实现 `script_global.cpp`

```cpp
#include "../../stdafx.h"
#include "script_global.hpp"
#include "../../company_base.h"
// ...

/* static */ SQInteger ScriptGlobal::GetCompanyCount()
{
	if (!IsGlobalAccessAllowed()) return 0;
	SQInteger count = 0;
	for (const Company *c : Company::Iterate()) count++;
	return count;
}
```

要点：
- 方法均为 `static`，`/* static */` 前缀定义；
- 返回值类型用 Squirrel 友好类型：`SQInteger`、`bool`、`Money`、`std::optional<std::string>`、`std::string`、`ScriptList*` 等；
- 访问控制：方法内检查 `ScriptCompanyMode::IsDeity() || 设置开关`，不满足返回错误值。

## 3. 列表类（可选）

需要返回公司/物品列表时，定义 `ScriptList` 子类：

```cpp
class ScriptGlobalCompanyList : public ScriptList {
public:
#ifdef DOXYGEN_API
	ScriptGlobalCompanyList();
#else
	ScriptGlobalCompanyList(HSQUIRRELVM vm);
#endif
};
```

```cpp
ScriptGlobalCompanyList::ScriptGlobalCompanyList(HSQUIRRELVM vm)
{
	ScriptList::FillList<Company>(vm, this);
}
```

## 4. 注册到构建

`src/script/api/CMakeLists.txt` 源列表加：

```cmake
script_global.cpp
```

::: tip
`.hpp` 无需注册（GLOB 自动发现）；但**重新 configure CMake** 才能看到新 `.hpp` 生成的绑定。
:::

## 5. 在脚本中使用

```js
// AI 侧：类名前缀 AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS 侧：前缀 GS
GSGlobal.GetCompanyCount();
```

## 完成清单

- [ ] `script_<name>.hpp`（`@api` 标注）
- [ ] `script_<name>.cpp`（`/* static */` 实现）
- [ ] `CMakeLists.txt` 加 `.cpp`
- [ ] 重新 configure + 构建
- [ ] （可选）`bin/ai/` 示例脚本
