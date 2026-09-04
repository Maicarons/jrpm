---
title: بحث هيكل المشروع وتنظيم الكود
---

> موضوع البحث: `G:\GitHub\OpenTTD-patches` (مساحة عمل كود مصدر jgrpp)
> الموارد المرجعية: `G:\game\openttd-jgrpp` (لعبة كاملة مجمعة، للقراءة فقط)
> تاريخ البحث: 2026-08-14
> الفرع: jgrpp (HEAD الحالي `5b5c452e1b`، حوالي إصدار 16.0)

---

## 1. نظرة عامة على المشروع

OpenTTD jgrpp (JGR's Patchpack) هو فرع محسّن معروف يعتمد على OpenTTD، كود المصدر في مساحة العمل هذه يقابل الإصدار **16.0**، يحتوي على عدد كبير من الميزات الخاصة بـ JGR (tracerestrict، إشارات قابلة للبرمجة، جدولة مجدولة، استبدال القوالب، إشارات محسّنة، إلخ).

| المشروع | الوصف |
|---|---|
| اللغة | C++20 (`CMAKE_CXX_STANDARD 20`، بدون امتدادات) |
| البناء | CMake (≥3.17)، `src/CMakeLists.txt` منظم حسب الدليل |
| محرك البرامج النصية | Squirrel (`src/3rdparty/squirrel/`)، يُستخدم لـ AI / GameScript / البرامج النصية للقوالب |
| الشبكة | طبقة بروتوكول TCP/UDP مخصصة (`src/network/`)، HTTP يستخدم WinHttp (Windows) / libcurl (أخرى) / JS (Emscripten) |
| الحفظ | تنسيق ثنائي مخصص `src/saveload/` + `src/sl/` |
| الخيوط | `src/thread.h` + `src/worker_thread.cpp` (تجمع مهام WorkerThreadPool)، `src/timer/` مؤقتات |

---

## 2. هيكل دليل المصدر وتقسيم الوحدات

المستوى العلوي: `CMakeLists.txt` هو سكريبت البناء الجذر؛ `src/` هو كود C++ بالكامل؛ `bin/` هو بيانات وقت التشغيل (سكريبتات AI المتوافقة، ملفات اللغة، إلخ)؛ `media/`، `os/`، `cmake/`، `docs/` تحتوي على الموارد وكود المنصة وسكريبتات البناء والوثائق على التوالي.

الأدلة الفرعية الرئيسية داخل `src/` ومسؤولياتها:

| الدليل | المسؤولية |
|---|---|
| `src/core/` | أدوات أساسية: أنواع pool، bitset، حاويات، تغليف الخيوط، عمليات البت، إلخ |
| `src/network/` | الشبكات: خادم/عميل/UDP/HTTP/تنزيل المحتوى/بروتوكول الإدارة (`core/` دليل فرعي لأساسيات البروتوكول) |
| `src/script/` | إطار تشغيل البرامج النصية: تغليف Squirrel، مثيلات، تكوين؛ `api/` هو جميع فئات API المعرّضة لـ AI/GS |
| `src/ai/` | إطار NoAI (مثيلات AI، ماسح ضوئي، تكوين، واجهة مستخدم رسومية) |
| `src/game/` | إطار GameScript (مثيلات GS، تكوين، واجهة مستخدم رسومية) |
| `src/newgrf/` | فك تشفير ومعالجة NewGRF |
| `src/pathfinder/` | التوجيه (YAPF، NPF) |
| `src/saveload/`، `src/sl/` | قراءة وكتابة الحفظ |
| `src/blitter/`، `src/video/`، `src/fontcache/`، `src/music/`، `src/sound/` | الواجهات الخلفية للعرض/الصوت والفيديو |
| `src/lang/` | مصادر سلاسل الترجمة (يتم توليدها بواسطة strgen) |
| `src/table/` | جداول ثابتة؛ `table/settings/*.ini` هو **مصدر تعريف الإعدادات** (يتم توليدها بواسطة settingsgen) |
| `src/3rdparty/` | مكتبات طرف ثالث (squirrel، llvm، icu، إلخ) |
| `src/timer/`، `src/os/`، `src/misc/` | مؤقتات، منصة، متنوعات |

الملفات المنتشرة في المستوى العلوي مسماة حسب النظام (مثل `rail_gui.cpp`، `group_cmd.cpp`، `vehicle.cpp`، `order_cmd.cpp`، `economy.cpp`)، تتبع اصطلاح OpenTTD: `*_cmd` لمنطق الأوامر، `*_gui` للنوافذ، `*_base/_type/_func` لهياكل البيانات والدوال المضمنة.

---

## 3. طريقة البناء

- **ثلاث خطوات CMake**: `cmake -B build ..` ← `cmake --build build` ← الناتج `openttd.exe`. المستودع يأتي مع `build.sh` / `build-dedicated.sh`.
- **التبعيات**: معلنة في `vcpkg.json` (zlib، lzma، lzo، zstd، png، SDL2، freetype، harfbuzz، icu، opus، إلخ)؛ Windows يستخدم WinHttp (لا حاجة لـ curl)، غير Windows يستخدم libcurl (`CMakeLists.txt:121-127`).
- **سلسلة أدوات (أدوات المضيف)**: `strgen` (ملفات اللغة)، `settingsgen` (توليد كود الإعدادات من `src/table/settings/*.ini`)، `squirrel_export` (توليد ربط Squirrel من `src/script/api/script_*.hpp`).
- **المخرجات الرئيسية**: `generated/script/api/<ai|gs>/...sq.hpp` (ربط API، **يتم اكتشافه تلقائياً بواسطة `file(GLOB script_*.hpp)`، إضافة فئات API جديدة لا يتطلب تعديل قائمة التسجيل**، فقط أضف `.hpp` وأضف `.cpp` إلى قائمة مصادر `src/script/api/CMakeLists.txt`)؛ `generated/rev.cpp`؛ `generated/ottdres.rc`.
- **نظام الإعدادات**: الإصدار الحديث تم تغييره إلى **محرك INI**——`src/table/settings/*.ini` (تحتوي على أقسام `[SDTC_VAR]`، `cat=SC_*` تصنيف، `flags`، `post_cb`، إلخ)، يتم توليد `settings_*.cpp/h` بواسطة settingsgen؛ أعضاء الهيكل المقابلون في `src/settings_type.h` (مثل `NetworkSettings` يبدأ من سطر 575).

---

## 4. مواقع تنفيذ أنظمة الوظائف الخمس الرئيسية

### 4.1 تنزيل الموارد (تنزيل المحتوى / BaNaNaSplit)

| نقطة الاهتمام | الموقع |
|---|---|
| الفئة الرئيسية لعميل المحتوى | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler` (أيضاً `ContentCallback` + `HTTPCallback`) |
| واجهة مستخدم تنزيل المحتوى | `src/network/network_content_gui.cpp/.h` |
| عميل HTTP | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`، غير متزامن يعتمد على الأحداث (غير معطل، استقصاء الحلقة الرئيسية) |
| سلسلة اتصال خادم المحتوى | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`: متغير البيئة `OTTD_CONTENT_SERVER_CS`، الافتراضي `content.openttd.org` (بروتوكول بيانات وصفية TCP) |
| URI المرآة | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`: متغير البيئة `OTTD_CONTENT_MIRROR_URI`، الافتراضي `https://binaries.openttd.org/bananas` |
| تدفق التنزيل | `DownloadSelectedContent()` ← `DownloadSelectedContentHTTP()` (POST لجميع معرفات المحتوى إلى المرآة، تعيد المرآة تدفق tar متعدد الملفات، كتابة ملف تلو الآخر) ← `AfterDownload()` يقوم بـ gunzip + `TarScanner` فك الضغط |
| فك الضغط | `GunzipFile()` (zlib)، `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| تسهيلات الخيوط (يمكن استخدامها للتوازي) | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`؛ `src/thread.h` هو تغليف خيوط المنصة |

**استنتاج الوضع الحالي**: ① يوجد مرآة واحدة فقط ويمكن تكوينها فقط عبر متغيرات البيئة، لا يوجد إعداد داخل اللعبة؛ ② التنزيل هو **اتصال واحد، تسلسلي** (POST واحد لجميع الملفات)؛ ③ فك الضغط يتم تنفيذه بشكل متزامن في الخيط الرئيسي. ← نقاط تحسين متعددة الخيوط/متعددة المرايا واضحة.

### 4.2 الحد الأقصى للاعبين عبر الإنترنت / الشركات في الخادم

| نقطة الاهتمام | الموقع |
|---|---|
| ثابت حد العميل | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| تجمع العميل | نفس الملف `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`؛ `ClientID` هو `uint32_t` |
| تجمع معرف الشركة | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`؛ الشركات الوهمية تشغل 253/254/255 |
| قناع الشركة | نفس الملف `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 بت، يمكنه تتبع 16 شركة فقط) |
| التحقق من قبول الخادم | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`؛ `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| عناصر إعداد العميل | `src/table/settings/network_settings.ini:231/241` — `network.max_companies` (def 15, max MAX_COMPANIES)، `network.max_clients` (def 25, max MAX_CLIENTS)؛ الهيكل في `src/settings_type.h` `NetworkSettings` |
| **عرض بت البروتوكول (قيد صارم)** | `src/network/core/network_game_info.cpp` — يتم إرسال `companies_max` و `clients_max` عبر **`Send_uint8`/`Recv_uint8`** (السطور 251-296، 422-432) |
| عرض قائمة الخادم | `src/network/network_gui.cpp:519`، إلخ |

**استنتاج الوضع الحالي**:
- **حد العميل = 255 هو بالفعل حد البروتوكول** (حقل uint8 + توافق منسق اللعبة/متصفح الخادم). لتجاوزه، يجب تغيير الحقول ذات الصلة في `network_game_info` إلى uint16 (رابط client↔server بالكامل + بث UDP + بروتوكول منسق اللعبة)، وهو تغيير في البروتوكول الخارجي.
- **حد الشركة = 15** (`CompanyIDTag` End=0xF). يمكن رفعه بأمان إلى **252** (End=0xFC): القاعدة لا تزال uint8، عرض بايت الحفظ لم يتغير (متوافق مع الحفظ القديم)، يحتاج إلى توسيع `CompanyMask` (uint16→uint32) وافتراضات الواجهة/الحلقة.

### 4.3 تجميع المركبات

| نقطة الاهتمام | الموقع |
|---|---|
| هيكل بيانات التجميع | `src/group.h` — `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number)؛ `GroupID`، `DEFAULT_GROUP`، `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| أوامر التجميع | `src/group_cmd.cpp` — `CmdCreateGroup`(536)، `CmdDeleteGroup`(585)، `CmdAlterGroup`(646)، `CmdAddVehicleGroup`، `CmdAddSharedVehicleGroup` (إضافة مركبات الطلبات المشتركة إلى مجموعة موجودة، بالقرب من سطر 718 `AddVehicleToGroup`) |
| تسجيل الأوامر | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`؛ التعداد في `src/command_type.h` `enum class Commands` (من سطر 492) |
| واجهة مستخدم التجميع | `src/group_gui.cpp/.h`، `src/vehiclelist.cpp` |
| مركبة↔مجموعة | `src/vehicle_base.h` (`Vehicle::group_id`)، `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| الطلبات/الجدولة المشتركة | `src/order_base.h` (`OrderList`، `VehicleOrdersID`)، `src/order_cmd.cpp`، `src/order_func.h`، `src/schdispatch.h/.cpp` (جدولة مجدولة، مرتبطة بقائمة الطلبات) |
| إحصائيات التجميع | `GroupStatistics` (group.h:60-66)، `GetGroupNumVehicle`، إلخ (group.h:125-128) |

**استنتاج الوضع الحالي**: يوجد بالفعل `CmdAddSharedVehicleGroup` (إضافة مركبات الطلبات المشتركة لمركبة إلى مجموعة ما) و `CmdCreateGroupFromList` (إنشاء مجموعة من قائمة)، لكن **لا يوجد منطق كامل لـ "إنشاء المجموعات وتجميعها تلقائياً حسب الطلبات المشتركة"**. الأمر الجديد `AutoGroupSharedOrders` (اجتياز جميع المركبات الرئيسية للشركة ← تجميع حسب `OrderList` ← إنشاء مجموعة تلقائياً وتجميعها) مسار تنفيذه واضح.

### 4.4 تلميح البناء (تلميح سعر فوق الماوس عند بناء السكك، إلخ)

| نقطة الاهتمام | الموقع |
|---|---|
| واجهة مستخدم بناء السكك / منطقها | `src/rail_gui.cpp` (`BuildRailToolbarWindow`)، `src/rail_cmd.cpp`، `src/rail.h/.cpp`؛ الطريق `road_gui.cpp/road_cmd.cpp` |
| تقدير التكلفة | `DoCommand` في كل `*_cmd.cpp` يرجع `CommandCost`؛ داخل واجهة المستخدم يمكن استخدام وضع `DC_QUERY_COST` للاستعلام عن السعر |
| البلاطة تحت الماوس | `src/viewport_func.h:36` — `GetTileBelowCursor()`؛ `_cursor.pos` (إحداثيات الشاشة)؛ `src/viewport.cpp:1056` |
| آلية تلميح النص الحالية | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)` (نص عائم في إحداثيات العالم)، `UpdateTextEffect`؛ `src/texteff.cpp` |
| تجربة مستخدم تلميح البناء الحالية | شريط أدوات السكك عند `OnPlaceDrag` السحب تظهر المنطقة المحددة وتكلفة متراكمة (`_thd` tilehighlight، `src/tilehighlight_func.h`)؛ شريط الحالة `statusbar_gui.cpp` يمكنه عرض تكلفة الأداة |
| نقطة التحديث لكل إطار | نوافذ شريط الأدوات المختلفة `OnMouseLoop` / `viewport.cpp` `HandleMouseEvents` (السطور 5422/5733) |

**استنتاج الوضع الحالي**: لا يوجد "تلميح سعر فوق الماوس". يمكن استخدام `AddTextEffect` لتثبيته على بلاطة الماوس (يتبع مؤشر الماوس)، أو رسم تلميح بإحداثيات الشاشة بنفسك؛ يمكن استخدام `DC_QUERY_COST` للأداة الحالية للاستعلام عن سعر بلاطة واحدة + تراكم منطقة التحديد المسحوبة.

### 4.5 واجهة AI (الحفاظ على NoAI + ذكاء اصطناعي يدرك اللعبة بأكملها)

| نقطة الاهتمام | الموقع |
|---|---|
| إطار NoAI | `src/ai/` — `ai_core.cpp` (الحلقة الرئيسية AICore)، `ai_instance.cpp` (AIInstance/Squirrel VM)، `ai_scanner.cpp` (مسح دليل `ai/`)، `ai_gui.cpp` (اختيار/تكوين)، `ai_config.cpp` |
| إطار GameScript | `src/game/` — `game_core.cpp`، `game_instance.cpp`، إلخ (GS هو "وضع إلهي"، صلاحياته أعلى من AI) |
| فئات API للبرامج النصية | `src/script/api/script_*.hpp/.cpp` (`script_company`، `script_map`، `script_vehicle`، `script_industry`، `script_town`، `script_game`، `script_admin`، إلخ، أكثر من 60 فئة) |
| تسجيل API التلقائي | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` يولد تلقائياً روابط `ai_*.sq.hpp`/`gs_*.sq.hpp`؛ يجب إضافة `.cpp` إلى قائمة المصادر (من سطر 235) |
| ماكرو ربط Squirrel | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`؛ `ai/ai_controller.sq.hpp` هو ربط وحدة التحكم AI |
| جدولة المثيلات | `src/script/script_instance.cpp`، `src/script/script_suspend.hpp` (تعليق/استئناف)، أحداث `script_event*` |
| التحكم في الوصول للشركة | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`؛ `ScriptCompanyMode` (`IsDeity()`) يميز وضع GS الإلهي؛ التحقق من معامل API للشركة `ResolveCompanyID`، `EnforceCompanyModeValid` |
| إعدادات البرامج النصية | `src/table/settings/script_settings.ini` (`game.script.*`)؛ تكوين مثيل AI `ai_config.cpp` |

**استنتاج الوضع الحالي**: في واجهة برمجة تطبيقات البرامج النصية الحديثة، بعض معلومات المنافسين (مثل `GetBankBalance`) لم تعد مقيدة، لكن **لا توجد واجهة برمجة تطبيقات تجميعية موجهة للإدراك الكامل للعبة** (لا توجد مدخلات موحدة لـ "تعداد جميع الشركات/الاقتصاد العام/إحصائيات الخريطة العامة")، ولا توجد نقطة وصول "AI عام" يتحكم فيها مفتاح. إضافة فئة API `ScriptGlobal` (تسجيل GLOB تلقائي) + إعداد `game.script` "السماح للإدراك العام للعبة" يمكن أن يحقق ذلك، GS متاح دائماً (وضع إلهي)، AI يتحكم فيه المفتاح——أي "الحفاظ على NoAI، إضافة ذكاء اصطناعي يدرك اللعبة بأكملها، مع تحكم في الوصول".

---

## 5. نظرة عامة على مخاطر التعديل

| الوظيفة | ملفات التعديل الرئيسية | المخاطر |
|---|---|---|
| F1 تنزيل متعدد المصادر/متعدد الخيوط | `src/table/settings/network_settings.ini`، `src/settings_type.h`، `src/network/core/config.cpp`، `src/network/network_content.h/.cpp` | متوسط (آلة حالة استدعاء الشبكة تحتاج إلى عناية) |
| F2 توسيع الحدود | `src/company_type.h`، `src/table/settings/network_settings.ini`، `src/network/core/network_game_info.cpp` (uint16 اختياري) | منخفض-متوسط (الشركة 252 منخفض المخاطر؛ عميل >255 يتطلب تغيير بروتوكول) |
| F3 تجميع المركبات تلقائياً | `src/group_cmd.h/.cpp`، `src/command_type.h`، `src/console_cmds.cpp`، `src/group_gui.cpp`، `src/lang/english.txt` | منخفض |
| F4 تلميح البناء | جديد `src/construction_cost_tip.h/.cpp`، `src/rail_gui.cpp`/`road_gui.cpp` خطافات، `src/lang/english.txt` | منخفض-متوسط |
| F5 ذكاء اصطناعي يدرك اللعبة بأكملها | جديد `src/script/api/script_global.hpp/.cpp`، `src/script/api/CMakeLists.txt`، `src/table/settings/script_settings.ini`، AI نموذجي `bin/ai/GlobalAI/` | منخفض (تسجيل تلقائي للإطار) |

> ملاحظة: جميع التعديلات تستند إلى مساحة العمل المحلية لفرع `jgrpp`، لم يتم التحقق من التجميع (لا توجد سلسلة أدوات بناء متاحة على هذا الجهاز)؛ يمكن مراجعة جميع التصحيحات عبر `git diff` ثم `git apply`.