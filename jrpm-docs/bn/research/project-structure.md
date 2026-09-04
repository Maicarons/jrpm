---
title: প্রকল্প কাঠামো ও কোড অর্গানাইজেশন গবেষণা
---

> গবেষণা অবজেক্ট: `G:\GitHub\OpenTTD-patches` (jgrpp সোর্স কোড ওয়ার্কস্পেস)
> রেফারেন্স রিসোর্স: `G:\game\openttd-jgrpp` (কম্পাইলড সম্পূর্ণ গেম, শুধু পঠনযোগ্য)
> গবেষণার তারিখ: 2026-08-14
> ব্রাঞ্চ: jgrpp (বর্তমান HEAD `5b5c452e1b`, প্রায় 16.0 সংস্করণ)

---

## ১. প্রকল্প পরিচিতি

OpenTTD jgrpp (JGR's Patchpack) OpenTTD-এর একটি সুপরিচিত উন্নত ফর্ক, এই ওয়ার্কস্পেস সোর্স কোড সংস্করণ **16.0**-এর সাথে সামঞ্জস্যপূর্ণ, এতে প্রচুর JGR নিজস্ব বৈশিষ্ট্য রয়েছে (tracerestrict, programmable signals, scheduled dispatch, টেমপ্লেট প্রতিস্থাপন, উন্নত সিগন্যাল ইত্যাদি)।

| প্রকল্প | ব্যাখ্যা |
|---|---|
| ভাষা | C++20 (`CMAKE_CXX_STANDARD 20`, কোনো এক্সটেনশন নেই) |
| বিল্ড | CMake (≥3.17), `src/CMakeLists.txt` ডিরেক্টরি অনুযায়ী সংগঠিত |
| স্ক্রিপ্ট ইঞ্জিন | Squirrel (`src/3rdparty/squirrel/`), AI / GameScript / টেমপ্লেট স্ক্রিপ্টের জন্য ব্যবহৃত |
| নেটওয়ার্ক | নিজস্ব TCP/UDP প্রোটোকল লেয়ার (`src/network/`), HTTP-র জন্য WinHttp (Windows) / libcurl (অন্যান্য) / JS (Emscripten) |
|存档 | `src/saveload/` + `src/sl/` নিজস্ব বাইনারি ফরম্যাট |
| থ্রেড | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool টাস্ক পুল), `src/timer/` টাইমার |

---

## ২. সোর্স কোড ডিরেক্টরি স্ট্রাকচার ও মডিউল বিভাজন

শীর্ষ স্তর: `CMakeLists.txt` রুট বিল্ড স্ক্রিপ্ট; `src/` সব C++ সোর্স কোড; `bin/` রানটাইম ডেটা (AI সামঞ্জস্যপূর্ণ স্ক্রিপ্ট, ভাষা ফাইল ইত্যাদি); `media/`, `os/`, `cmake/`, `docs/` যথাক্রমে রিসোর্স, প্ল্যাটফর্ম কোড, বিল্ড স্ক্রিপ্ট ও ডকুমেন্টেশন।

`src/`-এর ভিতরে প্রধান সাবডিরেক্টরি ও দায়িত্ব:

| ডিরেক্টরি | দায়িত্ব |
|---|---|
| `src/core/` | বেস টুল: পুল টাইপ, বিটসেট, কনটেইনার, থ্রেড র‍্যাপার, বিট অপারেশন ইত্যাদি |
| `src/network/` | নেটওয়ার্কিং: সার্ভার/ক্লায়েন্ট/UDP/HTTP/কন্টেন্ট ডাউনলোড/অ্যাডমিন প্রোটোকল (`core/` সাবডিরেক্টরি প্রোটোকল বেস) |
| `src/script/` | স্ক্রিপ্ট রান ফ্রেমওয়ার্ক: Squirrel র‍্যাপার, ইনস্ট্যান্স, কনফিগারেশন; `api/` AI/GS-এর জন্য উন্মুক্ত সব API ক্লাস |
| `src/ai/` | NoAI ফ্রেমওয়ার্ক (AI ইনস্ট্যান্স, স্ক্যানার, কনফিগারেশন, GUI) |
| `src/game/` | GameScript ফ্রেমওয়ার্ক (GS ইনস্ট্যান্স, কনফিগারেশন, GUI) |
| `src/newgrf/` | NewGRF ডিকোড ও প্রসেসিং |
| `src/pathfinder/` | পাথফাইন্ডিং (YAPF, NPF) |
| `src/saveload/`, `src/sl/` |存档 পঠন/লেখা |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | রেন্ডার/অডিও-ভিডিও ব্যাকএন্ড |
| `src/lang/` | লোকালাইজেশন স্ট্রিং উৎস (strgen জেনারেট) |
| `src/table/` | স্ট্যাটিক টেবিল; `table/settings/*.ini` **সেটিংস ডেফিনিশন উৎস** (settingsgen জেনারেট) |
| `src/3rdparty/` | থার্ড-পার্টি লাইব্রেরি (squirrel, llvm, icu ইত্যাদি) |
| `src/timer/`, `src/os/`, `src/misc/` | টাইমার, প্ল্যাটফর্ম, বিবিধ |

শীর্ষ স্তরের ফাইলগুলি সিস্টেম অনুযায়ী নামকরণ করা হয়েছে (যেমন `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), OpenTTD নীতি অনুসরণ করে: `*_cmd` কমান্ড লজিক, `*_gui` উইন্ডো, `*_base/_type/_func` ডেটা স্ট্রাকচার ও ইনলাইন ফাংশন।

---

## ৩. বিল্ড পদ্ধতি

- **CMake তিন ধাপ**: `cmake -B build ..` → `cmake --build build` → আউটপুট `openttd.exe`। রিপোজিটরিতে `build.sh` / `build-dedicated.sh` আছে।
- **নির্ভরতা**: `vcpkg.json` ঘোষণা (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus ইত্যাদি); Windows-এ WinHttp ব্যবহার করে (curl প্রয়োজন নেই), অ-উইন্ডোজে libcurl (`CMakeLists.txt:121-127`)।
- **টুলচেইন (হোস্ট টুলস)**: `strgen` (ভাষা ফাইল), `settingsgen` (`src/table/settings/*.ini` থেকে সেটিংস কোড জেনারেট), `squirrel_export` (`src/script/api/script_*.hpp` থেকে Squirrel বাইন্ডিং জেনারেট)।
- **মূল জেনারেটেড ফাইল**: `generated/script/api/<ai|gs>/...sq.hpp` (API বাইন্ডিং, **`file(GLOB script_*.hpp)` দ্বারা স্বয়ংক্রিয়ভাবে আবিষ্কৃত, নতুন API ক্লাস যোগ করলে রেজিস্ট্রেশন তালিকা পরিবর্তনের প্রয়োজন নেই**, শুধু `.hpp` যোগ করুন এবং `.cpp` `src/script/api/CMakeLists.txt` সোর্স তালিকায় যোগ করুন); `generated/rev.cpp`; `generated/ottdres.rc`।
- **সেটিংস সিস্টেম**: আধুনিক সংস্করণ **INI-চালিত**——`src/table/settings/*.ini` (`[SDTC_VAR]` সেকশন, `cat=SC_*` ক্যাটাগরি, `flags`, `post_cb` ইত্যাদি সহ), settingsgen দ্বারা `settings_*.cpp/h` জেনারেট; সংশ্লিষ্ট স্ট্রাকচার মেম্বার `src/settings_type.h`-এ (যেমন `NetworkSettings` ৫৭৫ লাইন থেকে শুরু)।

---

## ৪. পাঁচটি প্রধান বৈশিষ্ট্য সিস্টেমের বাস্তবায়ন অবস্থান

### ৪.১ রিসোর্স ডাউনলোড (কন্টেন্ট ডাউনলোড / BaNaNaSplit)

| ফোকাস পয়েন্ট | অবস্থান |
|---|---|
| কন্টেন্ট ক্লায়েন্ট প্রধান ক্লাস | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler` (একই সাথে `ContentCallback` + `HTTPCallback`) |
| কন্টেন্ট ডাউনলোড GUI | `src/network/network_content_gui.cpp/.h` |
| HTTP ক্লায়েন্ট | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, অ্যাসিঙ্ক্রোনাস ইভেন্ট-চালিত (নন-ব্লকিং, প্রধান লুপ পোলিং) |
| কন্টেন্ট সার্ভার সংযোগ স্ট্রিং | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`: এনভায়রনমেন্ট ভেরিয়েবল `OTTD_CONTENT_SERVER_CS`, ডিফল্ট `content.openttd.org` (TCP মেটাডেটা প্রোটোকল) |
| মিরর URI | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`: এনভায়রনমেন্ট ভেরিয়েবল `OTTD_CONTENT_MIRROR_URI`, ডিফল্ট `https://binaries.openttd.org/bananas` |
| ডাউনলোড প্রক্রিয়া | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (সব content ID POST করে মিররে, মিরর একটি মাল্টি-ফাইল tar স্ট্রিম ফেরত দেয়, প্রতি ফাইল ডিস্কে লেখে) → `AfterDownload()` gunzip + `TarScanner` আনপ্যাক করে |
| ডিকম্প্রেশন | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| থ্রেড সুবিধা (সমান্তরালের জন্য ব্যবহারযোগ্য) | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` প্ল্যাটফর্ম থ্রেড র‍্যাপার |

**বর্তমান অবস্থা সিদ্ধান্ত**: ① শুধু ১টি মিরর আছে এবং শুধু এনভায়রনমেন্ট ভেরিয়েবল দিয়ে কনফিগার করা যায়, গেমের ভিতরে সেটিংস নেই; ② ডাউনলোড **একক সংযোগ, সিরিয়াল** (একবারে POST সব ফাইল); ③ ডিকম্প্রেশন প্রধান থ্রেডে সিঙ্ক্রোনাসভাবে নির্বাহিত হয়। → মাল্টি-থ্রেড/মাল্টি-মিরর পরিবর্তন পয়েন্ট স্পষ্ট।

### ৪.২ সার্ভার অনলাইন প্লেয়ার / কোম্পানি সীমা

| ফোকাস পয়েন্ট | অবস্থান |
|---|---|
| ক্লায়েন্ট সীমা কনস্ট্যান্ট | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| ক্লায়েন্ট পুল | একই ফাইল `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` `uint32_t` |
| কোম্পানি ID পুল | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; জাল কোম্পানি 253/254/255 দখল করে |
| কোম্পানি মাস্ক | একই ফাইল `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (১৬ বিট, শুধু ১৬টি কোম্পানি ট্র্যাক করতে পারে) |
| সার্ভার গ্রহণ শর্ত | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| ক্লায়েন্ট সেটিংস আইটেম | `src/table/settings/network_settings.ini:231/241` — `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); স্ট্রাকচার `src/settings_type.h` `NetworkSettings` |
| **প্রোটোকল বিট-প্রস্থ (হার্ড কনস্ট্রেইন্ট)** | `src/network/core/network_game_info.cpp` — `companies_max` ও `clients_max` উভয়ই **`Send_uint8`/`Recv_uint8`** হিসেবে পাঠানো হয় (২৫১-২৯৬, ৪২২-৪৩২ লাইন) |
| সার্ভার তালিকা প্রদর্শন | `src/network/network_gui.cpp:519` ইত্যাদি |

**বর্তমান অবস্থা সিদ্ধান্ত**:
- **ক্লায়েন্ট সীমা = ২৫৫ ইতিমধ্যেই প্রোটোকল সীমা** (uint8 ফিল্ড + গেম কোঅর্ডিনেটর/সার্ভার ব্রাউজার সামঞ্জস্য)।突破 করতে `network_game_info`-এর সম্পর্কিত ফিল্ড uint16-এ পরিবর্তন করতে হবে (client↔server সম্পূর্ণ লিংক + UDP ব্রডকাস্ট + গেম কোঅর্ডিনেটর প্রোটোকল), যা বাহ্যিক প্রোটোকল পরিবর্তন।
- **কোম্পানি সীমা = ১৫** (`CompanyIDTag`-এর End=0xF)। নিরাপদে **২৫২**-তে বাড়ানো যেতে পারে (End=0xFC): নিচের স্তর এখনও uint8,存档 বাইট প্রস্থ অপরিবর্তিত (পুরনো存档 সামঞ্জস্যপূর্ণ), একই সাথে `CompanyMask` (uint16→uint32) ও ইন্টারফেস/লুপ অনুমান শিথিল করতে হবে।

### ৪.৩ যানবাহন গ্রুপিং

| ফোকাস পয়েন্ট | অবস্থান |
|---|---|
| গ্রুপ ডেটা স্ট্রাকচার | `src/group.h` — `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| গ্রুপ কমান্ড | `src/group_cmd.cpp` — `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (শেয়ার্ড অর্ডার যান বিদ্যমান গ্রুপে যোগ, 718 লাইনের কাছে `AddVehicleToGroup`) |
| কমান্ড রেজিস্ট্রেশন | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; এনাম `src/command_type.h` `enum class Commands` (৪৯২ লাইন থেকে শুরু) |
| গ্রুপ GUI | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| যান↔গ্রুপ | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| অর্ডার/শেয়ার্ড শিডিউল | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (শিডিউল্ড ডিসপ্যাচ, অর্ডার লিস্টে বাউন্ড) |
| গ্রুপ পরিসংখ্যান | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` ইত্যাদি (group.h:125-128) |

**বর্তমান অবস্থা সিদ্ধান্ত**: ইতিমধ্যে `CmdAddSharedVehicleGroup` (একটি যানের শেয়ার্ড অর্ডার যান একটি গ্রুপে যোগ) ও `CmdCreateGroupFromList` (তালিকা থেকে গ্রুপ তৈরি) আছে, কিন্তু **"শেয়ার্ড অর্ডার অনুযায়ী স্বয়ংক্রিয় গ্রুপ তৈরি/বাছাই" সম্পূর্ণ লজিক নেই**। নতুন কমান্ড `AutoGroupSharedOrders` (কোম্পানির সব প্রধান যান ট্রাভার্স → `OrderList` অনুযায়ী সমষ্টিকরণ → স্বয়ংক্রিয় গ্রুপ তৈরি ও বাছাই) বাস্তবায়ন পথ স্পষ্ট।

### ৪.৪ নির্মাণ টুলটিপ (রেল ইত্যাদি নির্মাণের সময় মাউসের উপরে মূল্য ইঙ্গিত)

| ফোকাস পয়েন্ট | অবস্থান |
|---|---|
| রেল নির্মাণ GUI/লজিক | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; রাস্তা `road_gui.cpp/road_cmd.cpp` |
| ব্যয় অনুমান | প্রতিটি `*_cmd.cpp`-এর `DoCommand` `CommandCost` রিটার্ন করে; GUI-তে `DC_QUERY_COST` মোড ব্যবহার করে মূল্য জানা যায় |
| মাউসের নিচে টাইল | `src/viewport_func.h:36` — `GetTileBelowCursor()`; `_cursor.pos` (স্ক্রিন কোঅর্ডিনেট); `src/viewport.cpp:1056` |
| বিদ্যমান টেক্সট ইঙ্গিত মেকানিজম | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)` (ওয়ার্ল্ড কোঅর্ডিনেট ফ্লোটিং টেক্সট), `UpdateTextEffect`; `src/texteff.cpp` |
| বিদ্যমান নির্মাণ ইঙ্গিত UX | রেল টুলবার `OnPlaceDrag` ড্র্যাগ করার সময় নির্বাচিত এলাকা ও ব্যয় সমষ্টি দেখায় (`_thd` tilehighlight, `src/tilehighlight_func.h`); স্ট্যাটাস বার `statusbar_gui.cpp` টুল ব্যয় দেখাতে পারে |
| প্রতি ফ্রেম রিফ্রেশ পয়েন্ট | প্রতিটি টুলবার উইন্ডো `OnMouseLoop` / `viewport.cpp`-এর `HandleMouseEvents` (৫৪২২/৫৭৩৩ লাইন) |

**বর্তমান অবস্থা সিদ্ধান্ত**: "মাউসের উপরে মূল্য টুলটিপ" নেই। `AddTextEffect` ব্যবহার করে মাউসের টাইল অ্যাঙ্কর করে (কার্সরের অবস্থান অনুসরণ), অথবা স্ক্রিন কোঅর্ডিনেটে টুলটিপ আঁকা যায়; ব্যয় `DC_QUERY_COST` ব্যবহার করে বর্তমান টুলের জন্য একক টাইল মূল্য + ড্র্যাগ এলাকা সমষ্টি পাওয়া যায়।

### ৪.৫ AI ইন্টারফেস (NoAI সুরক্ষিত + সম্পূর্ণ গেম-সচেতন AI)

| ফোকাস পয়েন্ট | অবস্থান |
|---|---|
| NoAI ফ্রেমওয়ার্ক | `src/ai/` — `ai_core.cpp` (AICore প্রধান লুপ), `ai_instance.cpp` (AIInstance/Squirrel VM), `ai_scanner.cpp` (`ai/` ডিরেক্টরি স্ক্যান), `ai_gui.cpp` (নির্বাচন/কনফিগারেশন), `ai_config.cpp` |
| GameScript ফ্রেমওয়ার্ক | `src/game/` — `game_core.cpp`, `game_instance.cpp` ইত্যাদি (GS "গড মোড", AI-এর চেয়ে বেশি অনুমতি) |
| স্ক্রিপ্ট API ক্লাস | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin` ইত্যাদি, মোট 60+ ক্লাস) |
| API স্বয়ংক্রিয় নিবন্ধন | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` স্বয়ংক্রিয়ভাবে `ai_*.sq.hpp`/`gs_*.sq.hpp` বাইন্ডিং জেনারেট করে; `.cpp` সোর্স তালিকায় যোগ করতে হবে (২৩৫ লাইন থেকে শুরু) |
| Squirrel বাইন্ডিং ম্যাক্রো | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` AI কন্ট্রোলার বাইন্ডিং |
| ইনস্ট্যান্স শিডিউলিং | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (সাসপেন্ড/রিজিউম), ইভেন্ট `script_event*` |
| কোম্পানি অ্যাক্সেস নিয়ন্ত্রণ | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) GS গড মোড আলাদা করে; কোম্পানি API প্যারামিটার যাচাই `ResolveCompanyID`, `EnforceCompanyModeValid` |
| স্ক্রিপ্ট সেটিংস | `src/table/settings/script_settings.ini` (`game.script.*`); AI ইনস্ট্যান্স কনফিগারেশন `ai_config.cpp` |

**বর্তমান অবস্থা সিদ্ধান্ত**: আধুনিক স্ক্রিপ্ট API-তে, প্রতিযোগীর কিছু তথ্য (যেমন `GetBankBalance`) ইতিমধ্যে সীমাবদ্ধ নয়, কিন্তু **সম্পূর্ণ খেলা উপলব্ধির জন্য সমষ্টিকৃত API নেই** (কোনো "সব কোম্পানি এনুমারেট/গ্লোবাল ইকোনমি/গ্লোবাল ম্যাপ পরিসংখ্যান" এর ইউনিফাইড এন্ট্রি নেই), এবং সুইচ-নিয়ন্ত্রিত "গ্লোবাল AI" অ্যাক্সেস পয়েন্ট নেই। একটি নতুন `ScriptGlobal` API ক্লাস (GLOB স্বয়ংক্রিয় নিবন্ধিত) + একটি "AI-কে গ্লোবাল উপলব্ধি অনুমতি দিন" `game.script` সেটিংস যোগ করলেই বাস্তবায়ন করা যায়, GS সর্বদা ব্যবহারযোগ্য (গড মোড), AI সুইচ নিয়ন্ত্রিত——অর্থাৎ "NoAI সুরক্ষিত, নতুন সম্পূর্ণ গেম-সচেতন AI, অ্যাক্সেস নিয়ন্ত্রণ সহ"।

---

## ৫. পরিবর্তন ঝুঁকি সারসংক্ষেপ

| বৈশিষ্ট্য | প্রধান পরিবর্তিত ফাইল | ঝুঁকি |
|---|---|---|
| F1 ডাউনলোড মাল্টি-সোর্স/মাল্টি-থ্রেড | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | মাঝারি (নেটওয়ার্ক কলব্যাক স্টেট মেশিন সতর্কতা প্রয়োজন) |
| F2 সীমা সম্প্রসারণ | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (ঐচ্ছিক uint16) | কম-মাঝারি (কোম্পানি ২৫২ কম ঝুঁকি; ক্লায়েন্ট >২৫৫ প্রোটোকল পরিবর্তন প্রয়োজন) |
| F3 যানবাহন স্বয়ংক্রিয় গ্রুপিং | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | কম |
| F4 নির্মাণ টুলটিপ | নতুন `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` হুক, `src/lang/english.txt` | কম-মাঝারি |
| F5 সম্পূর্ণ গেম-সচেতন AI | নতুন `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, উদাহরণ AI `bin/ai/GlobalAI/` | কম (ফ্রেমওয়ার্ক স্বয়ংক্রিয় নিবন্ধন) |

> নোট: সব পরিবর্তন `jgrpp` ব্রাঞ্চ লোকাল ওয়ার্কস্পেসের উপর ভিত্তি করে, কম্পাইল করে যাচাই করা হয়নি (এই মেশিনে ব্যবহারযোগ্য বিল্ড টুলচেইন নেই); সব প্যাচ `git diff` পর্যালোচনার পর `git apply` করা যেতে পারে।