---
title: स्वचालित वाहन समूहीकरण
---

## वर्तमान स्थिति (अनुसंधान निष्कर्ष)

- समूह डेटा संरचना: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- मौजूदा कमांड (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (नाम बदलना/मूल समूह सेट करना)
  - `CmdAddVehicleGroup` (एकल वाहन को समूह में जोड़ना)
  - `CmdAddSharedVehicleGroup` (किसी **एक वाहन के साझा ऑर्डर वाले वाहनों** को **मौजूदा** समूह में जोड़ना)
  - `CmdCreateGroupFromList` (वाहन सूची से समूह बनाना, `VL_SHARED_ORDERS` सूची प्रकार का समर्थन; स्वचालित नामकरण)
- स्वचालित नामकरण सहायक: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, रूट के आरंभ/अंत शहरों के अनुसार `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) — jgrpp में पहले से ही "रूट के अनुसार नामकरण" बुनियादी ढाँचा मौजूद है;
- कमांड पंजीकरण तंत्र: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (मैक्रो एक साथ हैंडलर और `CommandTraits` पंजीकरण घोषित करता है);
- वाहन↔समूह: `vehicle_base.h` `Vehicle::group_id`; साझा ऑर्डर श्रृंखला `FirstShared()/NextShared()`; `OrderList *orders`।

**निष्कर्ष**: एक "सभी कंपनी वाहनों को ट्रैवर्स करने, साझा ऑर्डर के अनुसार स्वचालित रूप से समूह बनाने और व्यवस्थित करने" वाला समग्र कमांड गायब है।

## इस सुविधा का कार्यान्वयन

### नया कमांड `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

प्रक्रिया (`src/group_cmd.cpp`):
1. उस कंपनी के निर्दिष्ट प्रकार के सभी मुख्य वाहनों को ट्रैवर्स करें (`Vehicle::IterateTypeFrontOnly(type)`), जो इस कंपनी के नहीं हैं/जिनके पास ऑर्डर नहीं है/जो पहले से कस्टम समूह में हैं, उन्हें छोड़ें;
2. प्रत्येक साझा ऑर्डर श्रृंखला की गणना करें; जब ≥2 वाहन एक ही ऑर्डर सूची साझा करते हैं:
   - `GenerateAutoNameForVehicleGroup(v)` के साथ समूह नाम उत्पन्न करें (जैसे "A शहर ↔ B शहर");
   - `VehicleListIdentifier(VL_SHARED_ORDERS, ...)` बनाएं, नेस्टेड रूप से `Command<Commands::CreateGroupFromList>::Do(flags, ...)` कॉल करें ताकि समूह बनाया जा सके और सभी साझा वाहनों को स्थानांतरित किया जा सके;
   - पूरा होने पर उस समूह के वाहनों का `group_id` अब डिफ़ॉल्ट समूह नहीं रहेगा, लूप स्वचालित रूप से छोड़ देगा (एक ही ऑर्डर सूची के लिए केवल एक समूह बनाया जाएगा);
3. `GroupChangeDeferredUpdateScope` समूह सांख्यिकी को एकीकृत रूप से विलंबित अपडेट करता है।

### तीन प्रवेश बिंदु

| प्रवेश बिंदु | स्थान | विवरण |
|---|---|---|
| समूहन विंडो बटन | `src/group_gui.cpp` नया `WID_GL_AUTOGROUP_SHARED` (टूलबार LIVERY के बगल में), OnClick पर कमांड भेजता है | स्प्राइट पुन: उपयोग `SPR_GROUP_CREATE_TRAIN + vtype` |
| कंसोल कमांड | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (बिना पैरामीटर = सभी चार प्रकार निष्पादित करें) | `IConsole::CmdRegister` |
| स्क्रिप्ट/अन्य | कोई भी कोड `Command<Commands::AutoGroupSharedOrders>::Post(...)` कॉल कर सकता है | कमांड नेटवर्क सिंक के माध्यम से जाता है, मल्टीप्लेयर सर्वर सुरक्षित |

### शामिल फ़ाइलें

- `src/command_type.h`: `Commands` एनम में नया `AutoGroupSharedOrders` (**नोट: एनम के बीच में सम्मिलन बाद के कमांड ID को स्थानांतरित कर देगा, पुराने और नए संस्करण मल्टीप्लेयर में असंगत होंगे, यह fork के अंदर सामान्य है**)
- `src/group_cmd.h` / `group_cmd.cpp`: कमांड घोषणा और कार्यान्वयन
- `src/widgets/group_widget.h`: नया विजेट ID
- `src/group_gui.cpp`: टूलबार बटन (NWidget + OnPaint स्प्राइट + OnClick)
- `src/console_cmds.cpp`: कंसोल कमांड
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## सत्यापन बिंदु

1. दो या अधिक वाहन एक ही ऑर्डर सूची साझा करते हैं → बटन/`autogroup` दबाने पर "रूट नाम" समूह बनता है और सभी वाहन समूह में शामिल हो जाते हैं;
2. पहले से समूहित वाहन पुनः स्थानांतरित नहीं होते; विभिन्न ऑर्डर सूचियों के अपने-अपने समूह बनते हैं;
3. समूह के भीतर वाहन आँकड़े (संख्या/लाभ) सही ढंग से ताज़ा होते हैं (`GroupChangeDeferredUpdateScope` पर निर्भर)।