---
title: "تنزيل الموارد: مرايا متعددة + تنزيل متوازي"
---

## الوضع الحالي (استنتاجات البحث)

| البند | الوضع الحالي |
|---|---|
| خادم المحتوى (بروتوكول البيانات الوصفية) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: متغير البيئة `OTTD_CONTENT_SERVER_CS`، الافتراضي `content.openttd.org` (منفذ TCP 3978) |
| مرآة التنزيل | `NetworkContentMirrorUriString()`: متغير البيئة `OTTD_CONTENT_MIRROR_URI`، الافتراضي `https://binaries.openttd.org/bananas` |
| طريقة التنزيل | `network_content.cpp` `DownloadSelectedContentHTTP()`: طلب POST **واحد** لجميع معرفات المحتوى ← ترجع المرآة قائمة رؤوس الملفات (`id,type,filesize,url` لكل سطر) ← **تنزيل تسلسلي واحداً تلو الآخر** لكل ملف (اتصال GET واحد لكل ملف) ← `AfterDownload()` gunzip + tar فك الضغط |
| نموذج الخيوط | طبقة HTTP (WinHttp) تعمل في خيط خلفية، لكن **تنزيل الملفات في قائمة انتظار تسلسلية**؛ لا توجد قائمة مرايا، ولا إعدادات داخل اللعبة، ولا توازي |

## تنفيذ هذه الوظيفة

### 1. إعدادات جديدة (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR، الافتراضي فارغ = استخدام المصدر الرسمي)
- `network.content_mirrors` (SLE_STR، عدة عناوين URI للمرايا مفصولة بفواصل، الافتراضي فارغ = المرآة الرسمية)

الأولوية: متغير البيئة > إعدادات اللعبة > الافتراضي الرسمي.

### 2. تحليل التكوين (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` يقرأ الإعداد؛
- إضافة `NetworkContentMirrorUris()` لتحليل القائمة المفصولة بفواصل (إزالة المسافات، تخطي العناصر الفارغة، الرجوع إلى المصدر الرسمي كحل أخير)؛
- `NetworkContentMirrorUriString()` تغير لإرجاع أول عنصر في القائمة.

### 3. التنزيل المتوازي (`src/network/network_content.h/.cpp`)

- إضافة `ContentFileDownload` (ملف واحد في انتظار التنزيل: id/type/filesize/url/filename)؛
- إضافة `ContentDownloadSession : HTTPCallback` (حالة تنزيل مستقلة لكل ملف واستدعاء؛ `IsCancelled` مرتبط بالمعالج)؛
- `DownloadSelectedContentHTTP()`: POST إلى `mirrors[mirror_index]` ← `ParseResponseHeaders()` تحليل جميع رؤوس الملفات مرة واحدة ← `StartDownloadSessions()` بدء حتى **4 جلسات متوازية** (`CONTENT_DOWNLOAD_PARALLEL`)، كل جلسة بعد اكتمالها تستلم تلقائياً الملف التالي في انتظار التنزيل؛
- سلسلة إعادة المحاولة عند الفشل: فشل جلسة/فشل طلب مرآة ← إعادة طلب المرآة التالية ← فشل جميع المرايا ← الرجوع إلى بروتوكول TCP القديم (`DownloadSelectedContentFallback`)؛
- إنهاء أنيق: عند الفشل/الإلغاء، يتم تعيين `download_cancelled`، انتظار انتهاء جميع الجلسات الجارية (`OnAllSessionsDone`) قبل إعادة المحاولة أو الرجوع، لتجنب الاستدعاءات المعلقة؛
- `ResetMirrorIndex()`: التنزيل الجديد يبدأ من أول مرآة (يُستدعى عند تشغيل واجهة المستخدم الرسومية).

## الملفات المعنية

- `src/table/settings/network_settings.ini` (إضافة إعدادين)
- `src/settings_type.h` (إضافة حقلين في `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (تحليل قائمة المرايا)
- `src/network/network_content.h` / `.cpp` (جلسات متوازية)
- `src/network/network_content_gui.cpp` (إعادة تعيين فهرس المرآة قبل التنزيل)

## نقاط التحقق

1. بعد تعبئة `network.content_mirrors` بعدة عناوين URI (مفصولة بفواصل)، يجب أن يعمل تنزيل المحتوى وتنزيل ملفات متعددة **بالتوازي** (مراقبة تقدم التنزيل/التقاط اتصالات متزامنة متعددة)؛
2. في حالة انقطاع الشبكة/مرآة خاطئة، يجب المحاولة تلقائياً مع المرآة التالية، وأخيراً الرجوع إلى البروتوكول القديم؛
3. الإلغاء أثناء التنزيل يجب ألا يترك ملفات `.tar.gz` غير مكتملة؛
4. يجب التحقق من التجميع على جهاز فعلي (لا توجد بيئة بناء في هذا المستودع، الكود لم يُجمّع).

## القيود المعروفة

- درجة التوازي مشفرة بشكل ثابت كـ 4 (`CONTENT_DOWNLOAD_PARALLEL`)، يمكن جعلها إعداداً في المستقبل؛
- بروتوكول المرآة يعتمد على تنسيق واجهة برمجة تطبيقات bananas الرسمية (POST لقائمة المعرفات يعيد قائمة رؤوس الملفات).