---
title: नई स्क्रिप्ट API जोड़ना
---

# नई स्क्रिप्ट API जोड़ना

इस परियोजना में कार्यान्वित `ScriptGlobal` (संपूर्ण-गेम अवेयर AI API) को टेम्पलेट के रूप में उपयोग करते हुए, jrpm में एक नई स्क्रिप्ट API क्लास (AI और GameScript दोनों के लिए दृश्यमान) जोड़ने के पूर्ण चरणों का परिचय।

## पृष्ठभूमि

- स्क्रिप्ट API क्लासेज़ `src/script/api/` में स्थित हैं: `script_<नाम>.hpp` (घोषणा) + `script_<नाम>.cpp` (कार्यान्वयन);
- `.hpp` निर्माण के दौरान `file(GLOB script_*.hpp)` द्वारा **स्वचालित रूप से खोजा** जाता है और Squirrel बाइंडिंग (`ai_*.sq.hpp` / `gs_*.sq.hpp`) उत्पन्न होती है, **क्लास को मैन्युअल पंजीकृत करने की आवश्यकता नहीं**;
- `.cpp` को `src/script/api/CMakeLists.txt` स्रोत सूची में शामिल करना होगा।

## 1. हेडर फ़ाइल `script_global.hpp`

```cpp
/** @file script_global.hpp दस्तावेज़ स्पष्टीकरण। */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * क्लास दस्तावेज़, @api अंकित करना अनिवार्य है।
 * @api ai game        # AI और GS दोनों के लिए एक्सपोज़; "game" केवल GS; "-ai" AI को बाहर करता है
 */
class ScriptGlobal : public ScriptObject {
public:
	/** एनम क्लास स्थिरांक के रूप में निर्यात किया जाएगा (जैसे AIGlobal.VT_TRAIN)। */
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

## 2. कार्यान्वयन `script_global.cpp`

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

मुख्य बिंदु:
- सभी विधियाँ `static` हैं, `/* static */` उपसर्ग के साथ परिभाषित;
- वापसी प्रकार Squirrel-अनुकूल प्रकार: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*` आदि;
- एक्सेस नियंत्रण: विधि के अंदर `ScriptCompanyMode::IsDeity() || सेटिंग स्विच` जाँचें, असंतुष्ट होने पर त्रुटि मान लौटाएँ।

## 3. सूची क्लास (वैकल्पिक)

जब कंपनी/आइटम सूची लौटाने की आवश्यकता हो, तो `ScriptList` उपवर्ग परिभाषित करें:

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

## 4. निर्माण में पंजीकृत करें

`src/script/api/CMakeLists.txt` स्रोत सूची में जोड़ें:

```cmake
script_global.cpp
```

::: tip
`.hpp` को पंजीकृत करने की आवश्यकता नहीं (GLOB स्वचालित रूप से खोजता है); लेकिन नए `.hpp` से उत्पन्न बाइंडिंग देखने के लिए **CMake को पुन: कॉन्फ़िगर करना** आवश्यक है।
:::

## 5. स्क्रिप्ट में उपयोग

```js
// AI पक्ष: क्लास नाम उपसर्ग AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS पक्ष: उपसर्ग GS
GSGlobal.GetCompanyCount();
```

## पूर्णता सूची

- [ ] `script_<name>.hpp` (`@api` अंकित)
- [ ] `script_<name>.cpp` (`/* static */` कार्यान्वयन)
- [ ] `CMakeLists.txt` में `.cpp` जोड़ें
- [ ] पुन: कॉन्फ़िगर + निर्माण
- [ ] (वैकल्पिक) `bin/ai/` उदाहरण स्क्रिप्ट