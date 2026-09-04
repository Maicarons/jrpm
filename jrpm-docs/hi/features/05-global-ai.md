---
title: संपूर्ण-गेम अवेयर AI इंटरफ़ेस
---

## वर्तमान स्थिति (अनुसंधान निष्कर्ष)

- NoAI फ्रेमवर्क: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) — मूल AI प्रणाली **यथावत संरक्षित** है, यह सुविधा इसे स्पर्श नहीं करती;
- स्क्रिप्ट API प्रणाली: `src/script/api/script_*.hpp/.cpp` (60+ क्लासेज़), Squirrel बाइंडिंग निर्माण-समय उपकरणों द्वारा **स्वचालित रूप से उत्पन्न** होती है:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` स्वचालित रूप से नई API क्लासेज़ खोजता है (`ai_*.sq.hpp` / `gs_*.sq.hpp` उत्पन्न करता है), **नई क्लासेज़ को मैन्युअल पंजीकरण की आवश्यकता नहीं**;
  - `.cpp` को CMake स्रोत सूची में शामिल करना होगा; क्लास टिप्पणी `@api ai game` AI/GS को एक्सपोज़र नियंत्रित करती है;
- कंपनी डेटा: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- GS देवता मोड: `ScriptCompanyMode::IsDeity()`;
- मौजूदा सीमाएँ: AI डिफ़ॉल्ट रूप से केवल अपनी कंपनी के डेटा तक ही विश्वसनीय रूप से पहुँच सकता है, कोई संपूर्ण-गेम एग्रीगेशन API नहीं, कोई एक्सेस स्विच नहीं।

**निष्कर्ष**: एक नया `ScriptGlobal` API क्लास (स्वचालित रूप से पंजीकृत) + `game.script.allow_global_ai_access` स्विच जोड़कर "NoAI को संरक्षित करना, नया संपूर्ण-गेम अवेयर AI जोड़ना, एक्सेस नियंत्रण के साथ" प्राप्त किया जा सकता है।

## इस सुविधा का कार्यान्वयन

### 1. `ScriptGlobal` API (`src/script/api/script_global.hpp/.cpp`)

AI और GS दोनों के लिए एक्सपोज़्ड (`@api ai game`) स्थैतिक विधियाँ:

| विधि | रिटर्न | डेटा स्रोत |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| सेटिंग स्विच` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` owner के अनुसार गणना |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- कंपनी ID `ScriptCompany::CompanyID` का पुन: उपयोग करती है (COMPANY_SELF वर्तमान कंपनी में हल होता है);
- **एक्सेस नियंत्रण**: सभी विधियाँ पहली पंक्ति में `IsGlobalAccessAllowed()` की जाँच करती हैं, असंतुष्ट होने पर -1/nullopt/false लौटाती हैं (GS हमेशा उपलब्ध, AI स्विच द्वारा नियंत्रित);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` सभी कंपनियों की गणना करता है।

### 2. एक्सेस स्विच (सेटिंग)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, डिफ़ॉल्ट false, `SC_EXPERT`);
- स्ट्रक्ट फ़ील्ड `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- स्ट्रिंग `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`)।

### 3. उदाहरण AI: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- प्रदर्शन: सभी कंपनियों के वित्त/वाहन/स्टेशन/रेटिंग और मानचित्र जानकारी पढ़ता है और लॉग करता है; एक्सेस नियंत्रण समझाता है (जब स्विच चालू नहीं है तो संकेत देता है);
- यह AI "संपूर्ण-गेम जागरूकता + एक्सेस नियंत्रण" का संदर्भ कार्यान्वयन है, बाद के AI नियंत्रण तर्क को इसके `Start()` में विस्तारित किया जा सकता है।

### शामिल फ़ाइलें

- नया `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (.cpp जोड़ें)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- नया `bin/ai/GlobalAI/info.nut`, `main.nut`

## सत्यापन बिंदु

1. नया AI AI कॉन्फ़िगरेशन इंटरफ़ेस में दिखाई देता है (`ai/GlobalAI` स्कैन करता है);
2. जब `game.script.allow_global_ai_access` चालू नहीं है, AI लॉग में कोई अनुमति नहीं होने का संकेत मिलता है; चालू होने पर सभी कंपनियों का पूरा डेटा आउटपुट होता है;
3. GameScript बिना स्विच के `GSGlobal` का उपयोग कर सकता है;
4. निर्माण के दौरान स्वचालित रूप से `ai_global.sq.hpp` / `gs_global.sq.hpp` बिना त्रुटि के उत्पन्न होता है (`file(GLOB)` पर निर्भर, CMake को पुन: कॉन्फ़िगर करने की आवश्यकता है)।

## विस्तार दिशाएँ

- अर्थव्यवस्था/ऋण/बुनियादी ढाँचा (`GetCompanyInfrastructure`) जैसे और अधिक एग्रीगेट डेटा जोड़ना;
- ईवेंट (कंपनी दिवालिया/अधिग्रहण/नई कंपनी स्थापना) सब्सक्रिप्शन जोड़ना;
- AI निर्णय तर्क को कॉन्फ़िगर करने योग्य पैरामीटर बनाना (`GetSettings()`)।