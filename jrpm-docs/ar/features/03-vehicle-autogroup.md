---
title: التجميع التلقائي للمركبات
---

## الوضع الحالي (استنتاجات البحث)

- هيكل بيانات التجميع: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number)، `GroupID`، `DEFAULT_GROUP`؛
- الأوامر الحالية (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`، `CmdDeleteGroup`، `CmdAlterGroup` (تغيير الاسم/تعيين المجموعة الأم)
  - `CmdAddVehicleGroup` (إضافة مركبة واحدة إلى مجموعة)
  - `CmdAddSharedVehicleGroup` (إضافة مركبات **الطلبات المشتركة لمركبة معينة** إلى مجموعة **موجودة**)
  - `CmdCreateGroupFromList` (إنشاء مجموعة من قائمة مركبات، تدعم نوع قائمة `VL_SHARED_ORDERS`؛ تسمية تلقائية)
- مساعد التسمية التلقائية: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899، حسب بلدتي بداية/نهاية الخط `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`)——jgrpp لديه بالفعل بنية تحتية "تسمية حسب الخط"؛
- آلية تسجيل الأوامر: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (ماكرو يعلن المعالج ويسجل `CommandTraits` في نفس الوقت)؛
- المركبة↔مجموعة: `vehicle_base.h` `Vehicle::group_id`؛ سلسلة الطلبات المشتركة `FirstShared()/NextShared()`؛ `OrderList *orders`.

**الاستنتاج**: ينقص أمر كامل "يجتاز جميع مركبات الشركة، ويقوم تلقائياً بإنشاء المجموعات وتجميعها حسب الطلبات المشتركة".

## تنفيذ هذه الوظيفة

### الأمر الجديد `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

التدفق (`src/group_cmd.cpp`):
1. اجتياز جميع المركبات الرئيسية من النوع المحدد للشركة (`Vehicle::IterateTypeFrontOnly(type)`)، تخطي المركبات غير التابعة لهذه الشركة/بدون طلبات/الموجودة بالفعل في مجموعات مخصصة؛
2. لكل سلسلة طلبات مشتركة، العد؛ عندما تشترك ≥2 مركبتين في نفس قائمة الطلبات:
   - استخدم `GenerateAutoNameForVehicleGroup(v)` لإنشاء اسم المجموعة (مثل "المدينة أ ↔ المدينة ب")؛
   - أنشئ `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`، واستدع `Command<Commands::CreateGroupFromList>::Do(flags, ...)` بشكل متداخل لإنشاء المجموعة ونقل جميع المركبات المشتركة إليها؛
   - بعد الانتهاء، `group_id` للمركبات داخل المجموعة لم يعد المجموعة الافتراضية، الحلقة تتخطى تلقائياً (يتم إنشاء مجموعة واحدة فقط لكل قائمة طلبات)؛
3. `GroupChangeDeferredUpdateScope` يؤخر تحديث إحصائيات المجموعة بشكل موحد.

### ثلاث نقاط دخول

| نقطة الدخول | الموقع | الوصف |
|---|---|---|
| زر نافذة التجميع | إضافة `WID_GL_AUTOGROUP_SHARED` في `src/group_gui.cpp` (بجانب LIVERY في شريط الأدوات)، OnClick يرسل الأمر | إعادة استخدام السبرايت `SPR_GROUP_CREATE_TRAIN + vtype` |
| أمر الكونسول | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (بدون معامل = تنفيذ لجميع الأنواع الأربعة) | `IConsole::CmdRegister` |
| البرامج النصية/أخرى | أي كود يمكنه `Command<Commands::AutoGroupSharedOrders>::Post(...)` | الأمر يمر عبر مزامنة الشبكة، آمن لخوادم متعددة اللاعبين |

### الملفات المعنية

- `src/command_type.h`: إضافة `AutoGroupSharedOrders` إلى تعداد `Commands` (**ملاحظة: الإدراج في منتصف التعداد سيزيح معرفات الأوامر اللاحقة، مما يسبب عدم تطابق بين الإصدارات القديمة والجديدة عبر الشبكة، وهو أمر طبيعي داخل fork**)
- `src/group_cmd.h` / `group_cmd.cpp`: إعلان الأمر وتنفيذه
- `src/widgets/group_widget.h`: معرف عنصر تحكم جديد
- `src/group_gui.cpp`: زر شريط الأدوات (NWidget + OnPaint sprite + OnClick)
- `src/console_cmds.cpp`: أمر الكونسول
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## نقاط التحقق

1. مركبتان أو أكثر تشتركان في نفس قائمة الطلبات ← النقر على الزر/`autogroup` يولد مجموعة "اسم الخط" وتجميع جميع المركبات؛
2. المركبات المجمعة بالفعل لا يتم نقلها مرة أخرى؛ قوائم الطلبات المختلفة تشكل مجموعاتها الخاصة؛
3. إحصائيات المركبات داخل المجموعة (العدد/الربح) يتم تحديثها بشكل صحيح (تعتمد على `GroupChangeDeferredUpdateScope`).