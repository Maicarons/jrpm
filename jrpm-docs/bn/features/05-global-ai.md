---
title: সম্পূর্ণ গেম-সচেতন AI ইন্টারফেস
---

## বর্তমান অবস্থা (গবেষণা সিদ্ধান্ত)

- NoAI ফ্রেমওয়ার্ক: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config)——মূল AI সিস্টেম **যেমন আছে তেমনই রাখা**, এই বৈশিষ্ট্য এতে হস্তক্ষেপ করে না;
- স্ক্রিপ্ট API সিস্টেম: `src/script/api/script_*.hpp/.cpp` (60+ ক্লাস), Squirrel বাইন্ডিং বিল্ড-টাইম টুল দ্বারা **স্বয়ংক্রিয়ভাবে জেনারেটেড**:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` স্বয়ংক্রিয়ভাবে নতুন API ক্লাস আবিষ্কার করে (`ai_*.sq.hpp` / `gs_*.sq.hpp` জেনারেট করে), **নতুন ক্লাসের জন্য হাতের রেজিস্ট্রেশন প্রয়োজন নেই**;
  - `.cpp` CMake সোর্স তালিকায় যোগ করতে হবে; ক্লাস কমেন্ট `@api ai game` AI/GS-এ এক্সপোজার নিয়ন্ত্রণ করে;
- কোম্পানি ডেটা: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- GS গড মোড: `ScriptCompanyMode::IsDeity()`;
- বর্তমান সীমাবদ্ধতা: AI ডিফল্টভাবে শুধু নিজের কোম্পানি ডেটা অ্যাক্সেস করতে পারে, কোনো সম্পূর্ণ খেলা সমষ্টিকরণ API নেই, কোনো অ্যাক্সেস সুইচ নেই।

**সিদ্ধান্ত**: একটি নতুন `ScriptGlobal` API ক্লাস (স্বয়ংক্রিয় নিবন্ধিত) + `game.script.allow_global_ai_access` সুইচ যোগ করলেই "NoAI সুরক্ষিত, নতুন সম্পূর্ণ-গেম-সচেতন AI, অ্যাক্সেস নিয়ন্ত্রণ সহ" বাস্তবায়ন করা যায়।

## এই বৈশিষ্ট্যের বাস্তবায়ন

### ১. `ScriptGlobal` API (`src/script/api/script_global.hpp/.cpp`)

AI ও GS-এর জন্য উন্মুক্ত (`@api ai game`) স্ট্যাটিক মেথড:

| মেথড | রিটার্ন | ডেটা উৎস |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| সেটিংস সুইচ` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` ওনার অনুযায়ী গণনা |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- কোম্পানি ID `ScriptCompany::CompanyID` পুনঃব্যবহার করে (COMPANY_SELF বর্তমান কোম্পানিতে রেজল্ভ করে);
- **অ্যাক্সেস নিয়ন্ত্রণ**: সব মেথডের প্রথম লাইন `IsGlobalAccessAllowed()` যাচাই করে, অসন্তুষ্ট হলে -1/nullopt/false রিটার্ন করে (GS সর্বদা ব্যবহারযোগ্য, AI সুইচ নিয়ন্ত্রিত);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` সব কোম্পানি এনুমারেট করে।

### ২. অ্যাক্সেস সুইচ (সেটিংস)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, ডিফল্ট false, `SC_EXPERT`);
- স্ট্রাকচার ফিল্ড `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- স্ট্রিং `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`)।

### ৩. উদাহরণ AI: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- প্রদর্শন: সব কোম্পানির আর্থিক/যান/স্টেশন/রেটিং ও ম্যাপ তথ্য পড়ে লগ করে; অ্যাক্সেস নিয়ন্ত্রণ ব্যাখ্যা করে (সুইচ চালু না থাকলে বিজ্ঞপ্তি);
- এই AI-ই "সম্পূর্ণ খেলা উপলব্ধি + অ্যাক্সেস নিয়ন্ত্রণ"-এর রেফারেন্স বাস্তবায়ন, পরবর্তী AI নিয়ন্ত্রণ লজিক এর `Start()`-এ প্রসারিত করা যেতে পারে।

### জড়িত ফাইল

- নতুন `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (.cpp যোগ)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- নতুন `bin/ai/GlobalAI/info.nut`, `main.nut`

## যাচাইকরণ পয়েন্ট

১. নতুন AI AI কনফিগারেশন ইন্টারফেসে দৃশ্যমান হয় (স্ক্যান `ai/GlobalAI`);
২. `game.script.allow_global_ai_access` চালু না থাকলে, AI লগে অনুমতি নেই বলে বিজ্ঞপ্তি দেয়; চালু করলে প্রতিটি কোম্পানির সম্পূর্ণ ডেটা আউটপুট করে;
৩. GameScript সুইচ ছাড়াই `GSGlobal` ব্যবহার করতে পারে;
৪. বিল্ড-টাইম স্বয়ংক্রিয়ভাবে `ai_global.sq.hpp` / `gs_global.sq.hpp` জেনারেট করে কোনো ত্রুটি ছাড়া (`file(GLOB)`-এর উপর নির্ভরশীল, CMake পুনরায় কনফিগার করতে হবে)।

## সম্প্রসারণের দিক

- অর্থনীতি/ঋণ/পরিকাঠামো (`GetCompanyInfrastructure`) ইত্যাদি আরও সমষ্টিকৃত ডেটা যোগ করা;
- ইভেন্ট (কোম্পানি দেউলিয়া/অধিগ্রহণ/নতুন কোম্পানি প্রতিষ্ঠা) সাবস্ক্রিপশন যোগ করা;
- AI সিদ্ধান্ত গ্রহণের লজিক কনফিগারেবল প্যারামিটার (`GetSettings()`) হিসেবে তৈরি করা।