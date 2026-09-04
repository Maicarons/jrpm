---
title: إضافة عنصر إعداد جديد
---

# إضافة عنصر إعداد جديد

نظام الإعدادات في jrpm هو **محرك INI**: `src/table/settings/*.ini` هو مصدر تعريف الإعدادات، يتم توليد الكود في وقت البناء بواسطة settingsgen؛ حقول الهيكل المقابلة موجودة في `src/settings_type.h`. نستخدم `network.content_download_parallel` المطبق في هذا المشروع كمثال.

## 1. حقل الهيكل

`src/settings_type.h`، ابحث عن هيكل الإعداد المقابل (الشبكة ← `NetworkSettings`، اللعبة ← هياكل فرعية لـ `GameSettings` ذات الصلة، البرامج النصية ← `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< عناوين URI للمرايا مفصولة بفواصل
	uint8_t content_download_parallel = 4;                ///< عدد ملفات التنزيل المتوازي
	// ...
};
```

## 2. تعريف INI

`src/table/settings/network_settings.ini`، أضف كتلة `[SDTC_VAR]` (إعدادات العميل تستخدم `SDTC_*`، إعدادات اللعبة تستخدم `SDT_*`):

```ini
[SDTC_VAR]
var      = network.content_download_parallel
type     = SLE_UINT8
flags    = SettingFlag::NotInSave, SettingFlag::NoNetworkSync
def      = 4
min      = 1
max      = 8
cat      = SC_BASIC
```

الحقول الشائعة:

| الحقل | الوصف |
|---|---|
| `var` | الاسم الكامل للإعداد (مسار الهيكل، مثل `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`، `SLE_INT*`، `SLE_BOOL`، `SLE_STR` (سلسلة نصية، تحتاج `length`) |
| `flags` | `SettingFlag::NotInSave` (لا يدخل الحفظ)، `NoNetworkSync` (لا تتم مزامنته)، `NetworkOnly`، `GuiZeroIsSpecial`، إلخ |
| `def/min/max/interval` | القيمة الافتراضية/النطاق/الخطوة |
| `str/strhelp` | سلسلة عرض واجهة الإعدادات |
| `cat` | تصنيف الإعداد (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. السلسلة النصية (عرض واجهة الإعدادات)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :السماح لـ AIs بالوصول إلى بيانات اللعبة بأكملها: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :السماح لـ AIs باستخدام واجهة Global API...
```

- الإعدادات المنطقية تنتهي بـ `{STRING2}` (تشغيل/إيقاف)؛
- الإعدادات الرقمية راجع تنسيق السلاسل المشابهة.

## 4. الاستخدام في الكود

```cpp
// إعدادات العميل
_settings_client.network.content_download_parallel

// إعدادات اللعبة
_settings_game.script.allow_global_ai_access
```

## 5. سيناريوهات خاصة

- **إعدادات الخادم**: `network.*` في اللعب متعدد اللاعبين يتم إرسالها من الخادم (`NetworkOnly` + آلية مزامنة الخادم)؛
- **ارتباط الحفظ**: إعدادات اللعبة (`game.*`) تُحفظ مع الحفظ (افتراضي)؛ إعدادات العميل (`network.*`/`gui.*`) افتراضياً `NotInSave`؛
- **الاستدعاء**: `pre_cb`/`post_cb` يمكن ربطها بمعالجة تغيير القيمة (مثل `UpdateClientConfigValues()`).

## قائمة الإنجاز

- [ ] حقل `settings_type.h`
- [ ] كتلة `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` المقابلة في `.ini`
- [ ] سلسلة english.txt (لعرض واجهة الإعدادات)
- [ ] إعادة تكوين + بناء