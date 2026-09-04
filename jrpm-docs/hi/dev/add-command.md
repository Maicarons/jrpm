---
title: नया गेम कमांड जोड़ना
---

# नया गेम कमांड जोड़ना

इस परियोजना में कार्यान्वित `Commands::AutoGroupSharedOrders` (स्वचालित वाहन समूहीकरण) को टेम्पलेट के रूप में उपयोग करते हुए, jrpm में एक नया कमांड जोड़ने के पूर्ण चरणों का परिचय।

## 1. कमांड एनम में घोषित करें

`src/command_type.h`, `enum class Commands` में मध्य में सम्मिलित करें (बाद के ID को स्थानांतरित करेगा):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. कमांड हैंडलर और पंजीकरण घोषित करें

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` एक साथ हैंडलर फ़ंक्शन घोषित करेगा और `CommandTraits` पंजीकृत करेगा):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: कोई टाइल पैरामीटर नहीं (टाइल के साथ `DEF_CMD_TUPLE` का उपयोग करें);
- `CmdDataT<...>`: पैरामीटर प्रकार सूची, हैंडलर क्रम में अनपैक करता है।

## 3. हैंडलर कार्यान्वित करें

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... व्यावसायिक तर्क ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

मुख्य बिंदु:
- `CMD_ERROR` लौटाना विफलता दर्शाता है (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` "वास्तविक निष्पादन" और "परीक्षण मोड" को गेट करता है;
- नेस्टेड कमांड के लिए `Command<Commands::X>::Do(flags, ...)` का उपयोग करें;
- GUI/नेटवर्क कॉल के लिए `Command<Commands::X>::Post(err_string, args...)` का उपयोग करें (स्वचालित रूप से नेटवर्क सिंक के माध्यम से जाता है)।

## 4. प्रवेश बिंदु जोड़ें

### विंडो बटन (वैकल्पिक)

1. `src/widgets/<system>_widget.h` एनम में `WID_XXX` जोड़ें;
2. विंडो NWidget लेआउट में बटन जोड़ें;
3. `OnClick` में `case` जोड़ें:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### कंसोल कमांड (वैकल्पिक)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// IConsoleStdLibRegister() में:
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### स्ट्रिंग (वैकल्पिक)

`src/lang/english.txt` में जोड़ें (जैसे `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`)।

## 5. पूर्णता सूची

- [ ] `command_type.h` एनम
- [ ] `<system>_cmd.h` में `DEF_CMD_TUPLE*`
- [ ] `<system>_cmd.cpp` हैंडलर
- [ ] (वैकल्पिक) विजेट/कंसोल/स्ट्रिंग
- [ ] नेटवर्क और सेव स्वचालित अनुकूलन (`CommandTraits` स्वचालित रूप से उत्पन्न)