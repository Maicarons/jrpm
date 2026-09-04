---
title: واجهة الذكاء الاصطناعي المدرك للعبة بأكملها
---

## الوضع الحالي (استنتاجات البحث)

- إطار NoAI: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config)——نظام AI الأصلي **مُبقي كما هو**، هذه الوظيفة لا تمسه؛
- نظام واجهة برمجة تطبيقات البرامج النصية: `src/script/api/script_*.hpp/.cpp` (أكثر من 60 فئة)، ربط Squirrel يتم **توليده تلقائياً** بواسطة أدوات وقت البناء:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` يكتشف تلقائياً فئات API الجديدة (يولد `ai_*.sq.hpp` / `gs_*.sq.hpp`)، **الفئات الجديدة لا تحتاج إلى تسجيل يدوي**؛
  - يجب إضافة `.cpp` إلى قائمة مصادر CMake؛ تعليق الفئة `@api ai game` يتحكم في التعريض لـ AI/GS؛
- بيانات الشركة: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy)، `GetAvailableMoney()`؛
- وضع GS الإلهي: `ScriptCompanyMode::IsDeity()`؛
- القيود الحالية: AI يمكنه افتراضياً فقط الوصول الموثوق إلى بيانات شركته الخاصة، لا توجد واجهة برمجة تطبيقات تجميعية للعبة بأكملها، ولا يوجد مفتاح وصول.

**الاستنتاج**: إضافة فئة واجهة برمجة تطبيقات `ScriptGlobal` (تسجيل تلقائي) + مفتاح `game.script.allow_global_ai_access` يمكن أن يحقق "الحفاظ على NoAI وإضافة ذكاء اصطناعي يدرك اللعبة بأكملها مع تحكم في الوصول".

## تنفيذ هذه الوظيفة

### 1. واجهة برمجة تطبيقات `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

أساليب ثابتة معرّضة لـ AI و GS (`@api ai game`):

| الأسلوب | الإرجاع | مصدر البيانات |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| مفتاح الإعداد` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`، `GetDate()`، `GetYear()` | int | `MapSizeX/Y`، `EconTime::CurDate`، `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` عد حسب المالك |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- معرف الشركة يعيد استخدام `ScriptCompany::CompanyID` (يتم تحليل COMPANY_SELF إلى الشركة الحالية)؛
- **التحكم في الوصول**: جميع الأساليب تتحقق أولاً من `IsGlobalAccessAllowed()`، إذا لم يتم استيفاء الشرط، تُرجع -1/nullopt/false (GS متاح دائماً، AI يتحكم فيه المفتاح)؛
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` يعدد جميع الشركات.

### 2. مفتاح الوصول (إعدادات)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`، الافتراضي false، `SC_EXPERT`)؛
- حقل الهيكل `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`)؛
- السلسلة النصية `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. AI نموذجي: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController)؛
- عرض توضيحي: قراءة جميع البيانات المالية/المركبات/المحطات/التقييمات ومعلومات الخريطة لجميع الشركات وتسجيلها؛ شرح التحكم في الوصول (رسالة عند عدم تشغيل المفتاح)؛
- هذا AI هو "التنفيذ المرجعي للإدراك الكامل للعبة + التحكم في الوصول"، يمكن توسيع منطق التحكم AI اللاحق داخل `Start()` الخاص به.

### الملفات المعنية

- إضافة `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (إضافة .cpp)
- `src/table/settings/script_settings.ini`، `src/settings_type.h`، `src/lang/english.txt`
- إضافة `bin/ai/GlobalAI/info.nut`، `main.nut`

## نقاط التحقق

1. AI الجديد يظهر في واجهة تكوين AI (مسح `ai/GlobalAI`)؛
2. عندما لا يكون `game.script.allow_global_ai_access` مفعلاً، سجل AI يظهر رسالة عدم إذن؛ بعد التفعيل، يخرج بيانات كاملة لكل شركة؛
3. GameScript يمكنه الوصول باستخدام `GSGlobal` دون الحاجة إلى المفتاح؛
4. التوليد التلقائي في وقت البناء لـ `ai_global.sq.hpp` / `gs_global.sq.hpp` بدون أخطاء (يعتمد على `file(GLOB)`، يحتاج إلى إعادة تكوين CMake).

## اتجاهات التوسع

- إضافة المزيد من البيانات المجمعة مثل الاقتصاد/القروض/البنية التحتية (`GetCompanyInfrastructure`)؛
- إضافة اشتراك في الأحداث (إفلاس شركة/استحواذ/إنشاء شركة جديدة)؛
- جعل منطق قرار AI معلمات قابلة للتكوين (`GetSettings()`).