---
title: 새 스크립트 API 추가
---

# 새 스크립트 API 추가

본 프로젝트에서 구현된 `ScriptGlobal` (전판 인식 AI API)을 템플릿으로 하여, jrpm에 새 스크립트 API 클래스 (AI 및 GameScript에 노출)를 추가하는 전체 단계를 소개합니다.

## 배경

- 스크립트 API 클래스는 `src/script/api/`에 위치: `script_<이름>.hpp` (선언) + `script_<이름>.cpp` (구현);
- `.hpp`는 빌드 시 `file(GLOB script_*.hpp)`로 **자동 발견**되어 Squirrel 바인딩 생성 (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **클래스 수동 등록 불필요**;
- `.cpp`는 `src/script/api/CMakeLists.txt` 소스 목록에 추가해야 함.

## 1. 헤더 파일 `script_global.hpp`

```cpp
/** @file script_global.hpp 문서 설명. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * 클래스 문서, 반드시 @api를 표기해야 함.
 * @api ai game        # AI와 GS 모두에 노출; "game"은 GS만; "-ai"는 AI 제외
 */
class ScriptGlobal : public ScriptObject {
public:
	/** 열거형은 클래스 상수로 내보내짐 (예: AIGlobal.VT_TRAIN). */
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

## 2. 구현 `script_global.cpp`

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

포인트:
- 메서드는 모두 `static`, `/* static */` 접두사로 정의;
- 반환 유형은 Squirrel 친화적 유형 사용: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*` 등;
- 접근 제어: 메서드 내에서 `ScriptCompanyMode::IsDeity() || 설정 스위치` 확인, 불만족 시 오류 값 반환.

## 3. 목록 클래스 (선택 사항)

회사/항목 목록 반환이 필요할 때 `ScriptList` 하위 클래스 정의:

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

## 4. 빌드에 등록

`src/script/api/CMakeLists.txt` 소스 목록에 추가:

```cmake
script_global.cpp
```

::: tip
`.hpp`는 등록 불필요 (GLOB 자동 발견); 하지만 **CMake 재설정**해야 새 `.hpp`로 생성된 바인딩을 볼 수 있습니다.
:::

## 5. 스크립트에서 사용

```js
// AI 측: 클래스명 접두사 AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS 측: 접두사 GS
GSGlobal.GetCompanyCount();
```

## 완료 체크리스트

- [ ] `script_<name>.hpp` (`@api` 표기)
- [ ] `script_<name>.cpp` (`/* static */` 구현)
- [ ] `CMakeLists.txt`에 `.cpp` 추가
- [ ] CMake 재설정 + 빌드
- [ ] (선택 사항) `bin/ai/` 예시 스크립트