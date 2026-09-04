---
title: دليل اللعب عبر الشبكة
---

# دليل اللعب عبر الشبكة

## استراتيجية توافق الإصدارات

jrpm هو **إصدار موسوم** (`jrpm-0.1.0`)، تتطلب المصافحة عبر الشبكة **تطابقاً تاماً** لسلسلة المراجعة:

| السيناريو | السلوك |
|---|---|
| عميل jrpm ↔ خادم jrpm | ✅ اتصال عادي (إصدار متطابق) |
| عميل jrpm ↔ خادم jgrpp / pulsexlb الأصلي | ❌ رفض (عزل الإصدار) |
| عميل jgrpp الأصلي ← خادم jrpm | ✅ مسموح (الخادم يرخي قبول مراجعة `jgrpp-*`) |
| عميل pulsexlb ← خادم jrpm | ✅ مسموح (يقبل مراجعة `pxp`) |

::: warning إصدار NewGRF
بغض النظر عن إصدار العميل، **يجب أن يتطابق رقم إصدار NewGRF تماماً مع الخادم** (يتم التحقق من `_openttd_newgrf_version` بدقة)، هذا هو الحد الأدنى للمحاكاة الحتمية.
:::

## فتح خادم

```bash
# خادم مخصص (بدون واجهة رسومية)
openttd-jrpm -D -c server.cfg
```

التكوين الموصى به لـ `server.cfg` (راجع [ضبط أداء الخادم](../performance/server-tuning) لمزيد من التفاصيل):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## الانضمام إلى خادم

- داخل اللعبة: "تعدد اللاعبين ← الانضمام إلى خادم إنترنت/إضافة خادم"، أو
- سطر الأوامر: `openttd-jrpm -n <host>:<port>`

## تنزيل المحتوى (NewGRF/سيناريوهات)

يدعم jrpm **مرايا متعددة + تنزيل متوازي**:

```ini
[network]
content_server = content.openttd.org        ; خادم البيانات الوصفية
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; عدد ملفات التنزيل المتوازي (1-8)
```

- قائمة المرايا مفصولة بفواصل، تُستخدم بالترتيب؛ إذا فشلت مرآة، يتم التبديل تلقائياً إلى التالية؛
- متغيرات البيئة `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` لها أولوية أعلى من إعدادات اللعبة؛
- بعد فشل جميع المرايا، يتم الرجوع تلقائياً إلى بروتوكول تنزيل TCP القديم.

## إدارة الخادم

- أوامر الكونسول: `status`، `clients`، `kick`، `ban`، `save`، `reset_company`، `autogroup` (التجميع التلقائي للمركبات)، إلخ؛
- RCON: بعد ضبط `rcon_password` في إعدادات الخادم، يمكن إرسال أوامر الكونسول عن بُعد.