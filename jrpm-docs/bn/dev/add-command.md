---
title: নতুন গেম কমান্ড
---

# নতুন গেম কমান্ড

এই প্রকল্পে বাস্তবায়িত `Commands::AutoGroupSharedOrders` (যানবাহন স্বয়ংক্রিয় গ্রুপিং) টেমপ্লেট হিসেবে ব্যবহার করে, jrpm-এ একটি নতুন কমান্ড যোগ করার সম্পূর্ণ ধাপ পরিচিতি।

## ১. কমান্ড এনামে ঘোষণা

`src/command_type.h`, `enum class Commands`-এর মাঝে সন্নিবেশ (পরবর্তী ID স্থানান্তরিত হবে):

```cpp
AddSharedVehiclesToGroup,               ///< একটি গ্রুপে অনুপস্থিত সব শেয়ার্ড যান যোগ করুন
AutoGroupSharedOrders,                  ///< শেয়ার্ড অর্ডার তালিকা অনুযায়ী সব যান স্বয়ংক্রিয় গ্রুপিং
```

## ২. কমান্ড হ্যান্ডলার ও নিবন্ধন ঘোষণা

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` একই সাথে হ্যান্ডলার ফাংশন ঘোষণা ও `CommandTraits` নিবন্ধন করবে):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: টাইল প্যারামিটার নেই (টাইল সহ থাকলে `DEF_CMD_TUPLE` ব্যবহার করুন);
- `CmdDataT<...>`: প্যারামিটার টাইপ তালিকা, হ্যান্ডলার ক্রম অনুযায়ী আনপ্যাক করে।

## ৩. হ্যান্ডলার বাস্তবায়ন

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... ব্যবসায়িক লজিক ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

মূল পয়েন্ট:
- `CMD_ERROR` রিটার্ন করলে ব্যর্থতা নির্দেশ করে (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` গেট "প্রকৃত নির্বাহ" ও "টেস্ট মোড" নিয়ন্ত্রণ করে;
- নেস্টেড কমান্ডের জন্য `Command<Commands::X>::Do(flags, ...)` ব্যবহার করুন;
- GUI/নেটওয়ার্ক কলের জন্য `Command<Commands::X>::Post(err_string, args...)` ব্যবহার করুন (স্বয়ংক্রিয়ভাবে নেটওয়ার্ক সিঙ্ক)।

## ৪. এন্ট্রি যোগ করুন

### উইন্ডো বাটন (ঐচ্ছিক)

১. `src/widgets/<system>_widget.h` এনামে `WID_XXX` যোগ করুন;
২. উইন্ডো NWidget লেআউটে বাটন যোগ করুন;
৩. `OnClick`-এ `case` যোগ করুন:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### কনসোল কমান্ড (ঐচ্ছিক)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// IConsoleStdLibRegister()-এ:
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### স্ট্রিং (ঐচ্ছিক)

`src/lang/english.txt`-এ যোগ করুন (যেমন `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`)।

## ৫. সম্পূর্ণ তালিকা

- [ ] `command_type.h` এনাম
- [ ] `<system>_cmd.h`-এর `DEF_CMD_TUPLE*`
- [ ] `<system>_cmd.cpp` হ্যান্ডলার
- [ ] (ঐচ্ছিক) কন্ট্রোল/কনসোল/স্ট্রিং
- [ ] নেটওয়ার্ক ও存档 স্বয়ংক্রিয় অভিযোজন (`CommandTraits` স্বয়ংক্রিয় জেনারেট)