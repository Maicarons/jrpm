---
title: إضافة أمر لعبة جديد
---

# إضافة أمر لعبة جديد

باستخدام `Commands::AutoGroupSharedOrders` (التجميع التلقائي للمركبات) المطبق في هذا المشروع كقالب، نقدم الخطوات الكاملة لإضافة أمر جديد في jrpm.

## 1. الإعلان في تعداد الأمر

`src/command_type.h`، `enum class Commands` إدراج في المنتصف (سيُزيح المعرفات اللاحقة):

```cpp
AddSharedVehiclesToGroup,               ///< إضافة جميع المركبات المشتركة الأخرى إلى مجموعة مفقودة
AutoGroupSharedOrders,                  ///< تجميع تلقائي لجميع المركبات حسب قوائم طلباتها المشتركة
```

## 2. إعلان معالج الأمر والتسجيل

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` سيُعلن في نفس الوقت عن دالة المعالج ويسجل `CommandTraits`):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: لا يوجد معامل بلاطة (مع بلاطة استخدم `DEF_CMD_TUPLE`)؛
- `CmdDataT<...>`: قائمة أنواع المعاملات، يقوم المعالج بفك تغليفها بالترتيب.

## 3. تنفيذ المعالج

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... منطق الأعمال ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

النقاط الرئيسية:
- إرجاع `CMD_ERROR` يشير إلى الفشل (`CommandCost(INVALID_STRING_ID)`)؛
- `flags.Test(DoCommandFlag::Execute)` يتحكم في "التنفيذ الفعلي" مقابل "وضع الاختبار"؛
- الأوامر المتداخلة تستخدم `Command<Commands::X>::Do(flags, ...)`؛
- استدعاء واجهة المستخدم/الشبكة يستخدم `Command<Commands::X>::Post(err_string, args...)` (يتم المزامنة عبر الشبكة تلقائياً).

## 4. إضافة مدخل

### زر نافذة (اختياري)

1. أضف `WID_XXX` في تعداد `src/widgets/<system>_widget.h`؛
2. أضف زراً في تخطيط NWidget للنافذة؛
3. أضف `case` في `OnClick`:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### أمر كونسول (اختياري)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// في IConsoleStdLibRegister():
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### سلسلة نصية (اختياري)

أضف في `src/lang/english.txt` (مثل `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. قائمة الإنجاز

- [ ] تعداد `command_type.h`
- [ ] `DEF_CMD_TUPLE*` في `<system>_cmd.h`
- [ ] معالج `<system>_cmd.cpp`
- [ ] (اختياري) عناصر تحكم/كونسول/سلاسل نصية
- [ ] التكيف التلقائي للشبكة والحفظ (`CommandTraits` يتم توليده تلقائياً)