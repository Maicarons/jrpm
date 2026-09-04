---
title: যানবাহন স্বয়ংক্রিয় গ্রুপিং
---

## বর্তমান অবস্থা (গবেষণা সিদ্ধান্ত)

- গ্রুপ ডেটা স্ট্রাকচার: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- বিদ্যমান কমান্ড (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (নাম পরিবর্তন/প্যারেন্ট গ্রুপ সেট)
  - `CmdAddVehicleGroup` (একক যান গ্রুপে যোগ)
  - `CmdAddSharedVehicleGroup` (একটি **যানের শেয়ার্ড অর্ডার যান** একটি **বিদ্যমান** গ্রুপে যোগ)
  - `CmdCreateGroupFromList` (যান তালিকা থেকে গ্রুপ তৈরি, `VL_SHARED_ORDERS` তালিকা টাইপ সমর্থন করে; স্বয়ংক্রিয় নামকরণ)
- স্বয়ংক্রিয় নামকরণ সহায়ক: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, রুটের শুরু/শেষ শহর অনুযায়ী `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`)——jgrpp-এ ইতিমধ্যে "রুট অনুযায়ী নামকরণ" পরিকাঠামো আছে;
- কমান্ড রেজিস্ট্রেশন মেকানিজম: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (ম্যাক্রো একই সাথে হ্যান্ডলার ঘোষণা ও `CommandTraits` রেজিস্ট্রেশন করে);
- যান↔গ্রুপ: `vehicle_base.h` `Vehicle::group_id`; শেয়ার্ড অর্ডার চেইন `FirstShared()/NextShared()`; `OrderList *orders`।

**সিদ্ধান্ত**: একটি "পুরো কোম্পানির যানবাহন ট্রাভার্স করে, শেয়ার্ড অর্ডার অনুযায়ী স্বয়ংক্রিয় গ্রুপ তৈরি ও বাছাই" সম্পূর্ণ কমান্ডের অভাব।

## এই বৈশিষ্ট্যের বাস্তবায়ন

### নতুন কমান্ড `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

প্রক্রিয়া (`src/group_cmd.cpp`):
১. কোম্পানির নির্দিষ্ট টাইপের সব প্রধান যান ট্রাভার্স করুন (`Vehicle::IterateTypeFrontOnly(type)`), নিজের কোম্পানি নয়/অর্ডার নেই/কাস্টম গ্রুপে আছে এমন যান স্কিপ করুন;
২. প্রতিটি শেয়ার্ড অর্ডার চেইন গণনা করুন; ≥২ যান একই অর্ডার তালিকা শেয়ার করলে:
   - `GenerateAutoNameForVehicleGroup(v)` ব্যবহার করে গ্রুপের নাম তৈরি করুন (যেমন "A শহর ↔ B শহর");
   - `VehicleListIdentifier(VL_SHARED_ORDERS, ...)` তৈরি করুন, নেস্টেড `Command<Commands::CreateGroupFromList>::Do(flags, ...)` কল করে গ্রুপ তৈরি করুন ও সব শেয়ার্ড যান সরান;
   - শেষে গ্রুপের ভিতরের যানের `group_id` আর ডিফল্ট গ্রুপ নয়, লুপ স্বয়ংক্রিয়ভাবে স্কিপ করবে (একই অর্ডার তালিকার জন্য শুধু একটি গ্রুপ তৈরি);
৩. `GroupChangeDeferredUpdateScope` ইউনিফাইড ডিফার্ড আপডেট গ্রুপ পরিসংখ্যান।

### তিনটি প্রবেশ বিন্দু

| প্রবেশ বিন্দু | অবস্থান | ব্যাখ্যা |
|---|---|---|
| গ্রুপিং উইন্ডো বাটন | `src/group_gui.cpp` নতুন `WID_GL_AUTOGROUP_SHARED` (টুলবারে LIVERY-এর পাশে), OnClick কমান্ড পাঠায় | স্প্রাইট পুনঃব্যবহার `SPR_GROUP_CREATE_TRAIN + vtype` |
| কনসোল কমান্ড | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (প্যারামিটার ছাড়া = চার টাইপই সম্পাদন) | `IConsole::CmdRegister` |
| স্ক্রিপ্ট/অন্যান্য | যেকোনো কোড `Command<Commands::AutoGroupSharedOrders>::Post(...)` করতে পারে | কমান্ড নেটওয়ার্ক সিঙ্কের মাধ্যমে যায়, মাল্টিপ্লেয়ার সার্ভারে নিরাপদ |

### জড়িত ফাইল

- `src/command_type.h`: `Commands` এনামে নতুন `AutoGroupSharedOrders` (**লক্ষ্য: এনামের মাঝে সন্নিবেশ করলে পরবর্তী কমান্ড ID স্থানান্তরিত হবে, পুরনো ও নতুন সংস্করণের মধ্যে মাল্টিপ্লেয়ার অসামঞ্জস্যপূর্ণ হবে, এটি ফর্কের মধ্যে স্বাভাবিক ঘটনা**)
- `src/group_cmd.h` / `group_cmd.cpp`: কমান্ড ঘোষণা ও বাস্তবায়ন
- `src/widgets/group_widget.h`: নতুন কন্ট্রোল ID
- `src/group_gui.cpp`: টুলবার বাটন (NWidget + OnPaint স্প্রাইট + OnClick)
- `src/console_cmds.cpp`: কনসোল কমান্ড
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## যাচাইকরণ পয়েন্ট

১. দুই বা ততোধিক যান একই অর্ডার তালিকা শেয়ার করলে → বাটন/`autogroup` চাপার পর "রুট নাম" গ্রুপ তৈরি হয় ও সব যান গ্রুপে যায়;
২. ইতিমধ্যে গ্রুপকৃত যান পুনরায় সরানো হয় না; বিভিন্ন অর্ডার তালিকা আলাদা গ্রুপ গঠন করে;
৩. গ্রুপের ভিতরের যান পরিসংখ্যান (সংখ্যা/লাভ) সঠিকভাবে রিফ্রেশ হয় (`GroupChangeDeferredUpdateScope`-এর উপর নির্ভরশীল)।