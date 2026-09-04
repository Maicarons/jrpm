---
title: إضافة واجهة برمجة تطبيقات برنامج نصي جديدة
---

# إضافة واجهة برمجة تطبيقات برنامج نصي جديدة

باستخدام `ScriptGlobal` (واجهة برمجة تطبيقات AI المدركة للعبة بأكملها) المطبق في هذا المشروع كقالب، نقدم الخطوات الكاملة لإضافة فئة واجهة برمجة تطبيقات برنامج نصي جديدة (مرئية لـ AI و GameScript) في jrpm.

## الخلفية

- فئات API للبرامج النصية موجودة في `src/script/api/`: `script_<اسم>.hpp` (إعلان) + `script_<اسم>.cpp` (تنفيذ)؛
- يتم **اكتشاف** `.hpp` تلقائياً بواسطة `file(GLOB script_*.hpp)` في وقت البناء وإنشاء ربط Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`)، **لا يحتاج إلى تسجيل يدوي للفئة**؛
- يجب إضافة `.cpp` إلى قائمة مصادر `src/script/api/CMakeLists.txt`.

## 1. ملف الرأس `script_global.hpp`

```cpp
/** @file script_global.hpp وصف الوثيقة. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * وصف الفئة، يجب وضع علامة @api.
 * @api ai game        # يُعرّض لكل من AI و GS؛ "game" فقط GS؛ "-ai" يستثني AI
 */
class ScriptGlobal : public ScriptObject {
public:
	/** سيتم تصدير التعداد كثوابت فئة (مثل AIGlobal.VT_TRAIN). */
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

## 2. التنفيذ `script_global.cpp`

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

النقاط الرئيسية:
- جميع الأساليب هي `static`، مع بادئة `/* static */` في التعريف؛
- أنواع القيمة المرجعة تستخدم أنواعاً صديقة لـ Squirrel: `SQInteger`، `bool`، `Money`، `std::optional<std::string>`، `std::string`، `ScriptList*`، إلخ؛
- التحكم في الوصول: تحقق داخل الأسلوب من `ScriptCompanyMode::IsDeity() || مفتاح الإعداد`، إذا لم يتم استيفاء الشرط، أعد قيمة خطأ.

## 3. فئة قائمة (اختياري)

عند الحاجة إلى إرجاع قائمة شركات/عناصر، عرّف فئة فرعية من `ScriptList`:

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

## 4. التسجيل في البناء

أضف إلى قائمة مصادر `src/script/api/CMakeLists.txt`:

```cmake
script_global.cpp
```

::: tip
`.hpp` لا يحتاج إلى تسجيل (GLOB يكتشف تلقائياً)؛ لكن **إعادة تكوين CMake** ضرورية لرؤية الربط الذي تم إنشاؤه لملف `.hpp` الجديد.
:::

## 5. الاستخدام في البرنامج النصي

```js
// جانب AI: بادئة اسم الفئة AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// جانب GS: بادئة GS
GSGlobal.GetCompanyCount();
```

## قائمة الإنجاز

- [ ] `script_<اسم>.hpp` (مع علامة `@api`)
- [ ] `script_<اسم>.cpp` (تنفيذ `/* static */`)
- [ ] إضافة `.cpp` في `CMakeLists.txt`
- [ ] إعادة تكوين + بناء
- [ ] (اختياري) برنامج نصي مثال في `bin/ai/`