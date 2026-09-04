---
title: নতুন স্ক্রিপ্ট API
---

# নতুন স্ক্রিপ্ট API

এই প্রকল্পে বাস্তবায়িত `ScriptGlobal` (সম্পূর্ণ গেম-সচেতন AI API) টেমপ্লেট হিসেবে ব্যবহার করে, jrpm-এ একটি নতুন স্ক্রিপ্ট API ক্লাস (AI ও GameScript-এর জন্য দৃশ্যমান) যোগ করার সম্পূর্ণ ধাপ পরিচিতি।

## পটভূমি

- স্ক্রিপ্ট API ক্লাস `src/script/api/`-এ অবস্থিত: `script_<নাম>.hpp` (ঘোষণা) + `script_<নাম>.cpp` (বাস্তবায়ন);
- `.hpp` বিল্ড-টাইম `file(GLOB script_*.hpp)` দ্বারা **স্বয়ংক্রিয়ভাবে আবিষ্কৃত** হয়ে Squirrel বাইন্ডিং (`ai_*.sq.hpp` / `gs_*.sq.hpp`) জেনারেট করে, **ক্লাস হাতে নিবন্ধনের প্রয়োজন নেই**;
- `.cpp` `src/script/api/CMakeLists.txt` সোর্স তালিকায় যোগ করতে হবে।

## ১. হেডার ফাইল `script_global.hpp`

```cpp
/** @file script_global.hpp ডকুমেন্টেশন ব্যাখ্যা। */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * ক্লাস ডকুমেন্টেশন, অবশ্যই @api চিহ্নিত করতে হবে।
 * @api ai game        # AI ও GS উভয়ের জন্য উন্মুক্ত; "game" শুধু GS; "-ai" AI বাদ দেয়
 */
class ScriptGlobal : public ScriptObject {
public:
	/** এনাম ক্লাস কনস্ট্যান্ট হিসেবে এক্সপোর্ট হবে (যেমন AIGlobal.VT_TRAIN)। */
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

## ২. বাস্তবায়ন `script_global.cpp`

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

মূল পয়েন্ট:
- মেথড সব `static`, `/* static */` উপসর্গ দিয়ে সংজ্ঞায়িত;
- রিটার্ন টাইপ Squirrel-বান্ধব টাইপ ব্যবহার করুন: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*` ইত্যাদি;
- অ্যাক্সেস নিয়ন্ত্রণ: মেথডের ভিতরে `ScriptCompanyMode::IsDeity() || সেটিংস সুইচ` চেক করুন, অসন্তুষ্ট হলে ত্রুটি মান রিটার্ন করুন।

## ৩. তালিকা ক্লাস (ঐচ্ছিক)

কোম্পানি/আইটেম তালিকা ফেরত দিতে `ScriptList` সাবক্লাস সংজ্ঞায়িত করুন:

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

## ৪. বিল্ডে নিবন্ধন

`src/script/api/CMakeLists.txt` সোর্স তালিকায় যোগ করুন:

```cmake
script_global.cpp
```

::: tip
`.hpp` নিবন্ধনের প্রয়োজন নেই (GLOB স্বয়ংক্রিয়ভাবে আবিষ্কার করে); কিন্তু নতুন `.hpp`-এর জেনারেটেড বাইন্ডিং দেখতে **CMake পুনরায় কনফিগার** করতে হবে।
:::

## ৫. স্ক্রিপ্টে ব্যবহার

```js
// AI পক্ষ: ক্লাসের নামের পূর্বে AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS পক্ষ: পূর্বে GS
GSGlobal.GetCompanyCount();
```

## সম্পূর্ণ তালিকা

- [ ] `script_<name>.hpp` (`@api` চিহ্নিত)
- [ ] `script_<name>.cpp` (`/* static */` বাস্তবায়ন)
- [ ] `CMakeLists.txt`-এ `.cpp` যোগ
- [ ] পুনরায় configure + বিল্ড
- [ ] (ঐচ্ছিক) `bin/ai/` উদাহরণ স্ক্রিপ্ট