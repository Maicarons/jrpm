---
title: परियोजना संरचना और कोड संगठन अनुसंधान
---

> अनुसंधान वस्तु: `G:\GitHub\OpenTTD-patches` (jgrpp स्रोत कोड कार्यक्षेत्र)
> संदर्भ संसाधन: `G:\game\openttd-jgrpp` (संकलित पूर्ण गेम, केवल पढ़ने के लिए)
> अनुसंधान तिथि: 2026-08-14
> शाखा: jgrpp (वर्तमान HEAD `5b5c452e1b`, लगभग संस्करण 16.0)

---

## 1. परियोजना अवलोकन

OpenTTD jgrpp (JGR's Patchpack) OpenTTD पर आधारित एक प्रसिद्ध उन्नत शाखा है, यह कार्यक्षेत्र स्रोत कोड संस्करण **16.0** से मेल खाता है, जिसमें बड़ी संख्या में JGR की निजी विशेषताएँ शामिल हैं (tracerestrict, programmable signals, scheduled dispatch, टेम्पलेट प्रतिस्थापन, उन्नत सिग्नल आदि)।

| आइटम | विवरण |
|---|---|
| भाषा | C++20 (`CMAKE_CXX_STANDARD 20`, बिना विस्तार) |
| निर्माण | CMake (≥3.17), `src/CMakeLists.txt` निर्देशिका के अनुसार व्यवस्थित |
| स्क्रिप्ट इंजन | Squirrel (`src/3rdparty/squirrel/`), AI / GameScript / टेम्पलेट स्क्रिप्ट के लिए |
| नेटवर्क | स्व-निर्मित TCP/UDP प्रोटोकॉल लेयर (`src/network/`), HTTP के लिए WinHttp (Windows) / libcurl (अन्य) / JS (Emscripten) |
| सेव | `src/saveload/` + `src/sl/` स्व-निर्मित बाइनरी प्रारूप |
| थ्रेड | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool कार्य पूल), `src/timer/` टाइमर |

---

## 2. स्रोत कोड निर्देशिका संरचना और मॉड्यूल विभाजन

शीर्ष स्तर: `CMakeLists.txt` मूल निर्माण स्क्रिप्ट; `src/` सभी C++ स्रोत कोड; `bin/` रनटाइम डेटा (AI संगत स्क्रिप्ट, भाषा फ़ाइलें आदि); `media/`, `os/`, `cmake/`, `docs/` क्रमशः संसाधन, प्लेटफ़ॉर्म कोड, निर्माण स्क्रिप्ट और दस्तावेज़।

`src/` के अंदर मुख्य उपनिर्देशिकाएँ और उनकी जिम्मेदारियाँ:

| निर्देशिका | जिम्मेदारी |
|---|---|
| `src/core/` | मूल उपकरण: pool प्रकार, bitset, कंटेनर, थ्रेड रैपर, बिट ऑपरेशन आदि |
| `src/network/` | नेटवर्किंग: सर्वर/क्लाइंट/UDP/HTTP/सामग्री डाउनलोड/प्रशासक प्रोटोकॉल (`core/` उपनिर्देशिका प्रोटोकॉल आधार) |
| `src/script/` | स्क्रिप्ट रन फ्रेमवर्क: Squirrel रैपर, इंस्टेंस, कॉन्फ़िगरेशन; `api/` AI/GS के लिए सभी API क्लासेज़ |
| `src/ai/` | NoAI फ्रेमवर्क (AI इंस्टेंस, स्कैनर, कॉन्फ़िगरेशन, GUI) |
| `src/game/` | GameScript फ्रेमवर्क (GS इंस्टेंस, कॉन्फ़िगरेशन, GUI) |
| `src/newgrf/` | NewGRF डिकोडिंग और प्रसंस्करण |
| `src/pathfinder/` | पथ-खोज (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | सेव रीड/राइट |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | रेंडरिंग/ऑडियो-वीडियो बैकएंड |
| `src/lang/` | स्थानीयकरण स्ट्रिंग स्रोत (strgen द्वारा उत्पन्न) |
| `src/table/` | स्थैतिक तालिकाएँ; `table/settings/*.ini` **सेटिंग परिभाषा स्रोत** (settingsgen द्वारा उत्पन्न) |
| `src/3rdparty/` | तृतीय-पक्ष लाइब्रेरी (squirrel, llvm, icu आदि) |
| `src/timer/`, `src/os/`, `src/misc/` | टाइमर, प्लेटफ़ॉर्म, विविध |

शीर्ष स्तर की बिखरी हुई फ़ाइलें सिस्टम के अनुसार नामित (जैसे `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), OpenTTD परंपरा का पालन करती हैं: `*_cmd` कमांड तर्क, `*_gui` विंडो, `*_base/_type/_func` डेटा संरचनाएँ और इनलाइन फ़ंक्शन।

---

## 3. निर्माण विधि

- **CMake तीन चरण**: `cmake -B build ..` → `cmake --build build` → आउटपुट `openttd.exe`। रिपॉजिटरी में अपने `build.sh` / `build-dedicated.sh` हैं।
- **निर्भरताएँ**: `vcpkg.json` में घोषित (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus आदि); Windows WinHttp का उपयोग करता है (curl की आवश्यकता नहीं), गैर-Win libcurl का उपयोग करता है (`CMakeLists.txt:121-127`)।
- **उपकरण श्रृंखला (होस्ट टूल्स)**: `strgen` (भाषा फ़ाइलें), `settingsgen` (`src/table/settings/*.ini` से सेटिंग कोड उत्पन्न करता है), `squirrel_export` (`src/script/api/script_*.hpp` से Squirrel बाइंडिंग उत्पन्न करता है)।
- **मुख्य उत्पाद**: `generated/script/api/<ai|gs>/...sq.hpp` (API बाइंडिंग, **`file(GLOB script_*.hpp)` द्वारा स्वचालित रूप से खोजी जाती है, नई API क्लास जोड़ने के लिए पंजीकरण सूची बदलने की आवश्यकता नहीं**, केवल `.hpp` जोड़ें और `.cpp` को `src/script/api/CMakeLists.txt` स्रोत सूची में शामिल करें); `generated/rev.cpp`; `generated/ottdres.rc`।
- **सेटिंग प्रणाली**: आधुनिक संस्करण **INI-चालित** है — `src/table/settings/*.ini` (`[SDTC_VAR]` अनुभाग, `cat=SC_*` वर्गीकरण, `flags`, `post_cb` आदि सहित), settingsgen द्वारा `settings_*.cpp/h` उत्पन्न होता है; संबंधित स्ट्रक्ट सदस्य `src/settings_type.h` में (जैसे `NetworkSettings` पंक्ति 575 से)।

---

## 4. पाँच प्रमुख सुविधा प्रणालियों के कार्यान्वयन स्थान

### 4.1 संसाधन डाउनलोड (सामग्री डाउनलोड / BaNaNaSplit)

| ध्यान बिंदु | स्थान |
|---|---|
| सामग्री क्लाइंट मुख्य क्लास | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler` (साथ ही `ContentCallback` + `HTTPCallback`) |
| सामग्री डाउनलोड GUI | `src/network/network_content_gui.cpp/.h` |
| HTTP क्लाइंट | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, एसिंक्रोनस ईवेंट-संचालित (नॉन-ब्लॉकिंग, मुख्य लूप पोलिंग) |
| सामग्री सर्वर कनेक्शन स्ट्रिंग | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`: पर्यावरण चर `OTTD_CONTENT_SERVER_CS`, डिफ़ॉल्ट `content.openttd.org` (TCP मेटाडेटा प्रोटोकॉल) |
| मिरर URI | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`: पर्यावरण चर `OTTD_CONTENT_MIRROR_URI`, डिफ़ॉल्ट `https://binaries.openttd.org/bananas` |
| डाउनलोड प्रक्रिया | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (सभी content ID का POST मिरर पर, मिरर एक मल्टी-फ़ाइल tar स्ट्रीम लौटाता है, फ़ाइल-दर-फ़ाइल डिस्क पर लिखता है) → `AfterDownload()` gunzip + `TarScanner` अनपैक करता है |
| डीकंप्रेसन | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| थ्रेड सुविधाएँ (समानांतर के लिए उपयोगी) | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` प्लेटफ़ॉर्म थ्रेड रैपर |

**वर्तमान स्थिति निष्कर्ष**: ① केवल 1 मिरर है और केवल पर्यावरण चर के माध्यम से कॉन्फ़िगर किया जा सकता है, कोई इन-गेम सेटिंग नहीं; ② डाउनलोड **एकल कनेक्शन, क्रमिक** है (एक POST सभी फ़ाइलें); ③ डीकंप्रेसन मुख्य थ्रेड में समकालिक रूप से निष्पादित होता है। → मल्टी-थ्रेड/मल्टी-मिरर संशोधन बिंदु स्पष्ट हैं।

### 4.2 सर्वर ऑनलाइन खिलाड़ी / कंपनी सीमा

| ध्यान बिंदु | स्थान |
|---|---|
| क्लाइंट सीमा स्थिरांक | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| क्लाइंट पूल | उसी फ़ाइल में `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` `uint32_t` है |
| कंपनी ID पूल | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; नकली कंपनियाँ 253/254/255 पर कब्जा करती हैं |
| कंपनी मास्क | उसी फ़ाइल में `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 बिट, केवल 16 कंपनियों को ट्रैक कर सकता है) |
| सर्वर प्रवेश निर्णय | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| क्लाइंट सेटिंग आइटम | `src/table/settings/network_settings.ini:231/241` — `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); स्ट्रक्ट `src/settings_type.h` `NetworkSettings` में |
| **प्रोटोकॉल बिट-चौड़ाई (कठोर बाधा)** | `src/network/core/network_game_info.cpp` — `companies_max` और `clients_max` दोनों **`Send_uint8`/`Recv_uint8`** के साथ भेजे जाते हैं (पंक्तियाँ 251-296, 422-432) |
| सर्वर सूची प्रदर्शन | `src/network/network_gui.cpp:519` आदि |

**वर्तमान स्थिति निष्कर्ष**:
- **क्लाइंट सीमा = 255 पहले से ही प्रोटोकॉल सीमा है** (uint8 फ़ील्ड + गेम कोऑर्डिनेटर/सर्वर ब्राउज़र संगतता)। इसे तोड़ने के लिए `network_game_info` के संबंधित फ़ील्ड को uint16 में बदलना होगा (क्लाइंट↔सर्वर पूर्ण लिंक + UDP प्रसारण + गेम कोऑर्डिनेटर प्रोटोकॉल), जो एक बाहरी प्रोटोकॉल परिवर्तन है।
- **कंपनी सीमा = 15** (`CompanyIDTag` का End=0xF)। सुरक्षित रूप से **252** (End=0xFC) तक बढ़ाया जा सकता है: अंतर्निहित अभी भी uint8 है, सेव बाइट चौड़ाई अपरिवर्तित (पुराने सेव संगत), `CompanyMask` (uint16→uint32) और इंटरफ़ेस/लूप धारणाओं को समकालिक रूप से शिथिल करने की आवश्यकता है।

### 4.3 वाहन समूहीकरण

| ध्यान बिंदु | स्थान |
|---|---|
| समूह डेटा संरचना | `src/group.h` — `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| समूह कमांड | `src/group_cmd.cpp` — `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (साझा ऑर्डर वाले वाहनों को मौजूदा समूह में जोड़ता है, पंक्ति 718 के आसपास `AddVehicleToGroup`) |
| कमांड पंजीकरण | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; एनम `src/command_type.h` `enum class Commands` (पंक्ति 492 से) |
| समूह GUI | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| वाहन↔समूह | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| ऑर्डर/साझा शेड्यूल | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (शेड्यूल्ड डिस्पैच, ऑर्डर सूची से बंधा) |
| समूह सांख्यिकी | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` आदि (group.h:125-128) |

**वर्तमान स्थिति निष्कर्ष**: पहले से `CmdAddSharedVehicleGroup` (एक वाहन के साझा ऑर्डर वाले वाहनों को समूह में जोड़ना) और `CmdCreateGroupFromList` (सूची से समूह बनाना) मौजूद हैं, लेकिन **"साझा ऑर्डर के अनुसार स्वचालित रूप से समूह बनाने/व्यवस्थित करने" का पूर्ण तर्क नहीं है**। नया कमांड `AutoGroupSharedOrders` (कंपनी के सभी मुख्य वाहनों को ट्रैवर्स करें → `OrderList` के अनुसार एकत्रित करें → स्वचालित रूप से समूह बनाएं और व्यवस्थित करें) का कार्यान्वयन पथ स्पष्ट है।

### 4.4 निर्माण टूलटिप (रेल आदि बनाते समय माउस के ऊपर मूल्य संकेत)

| ध्यान बिंदु | स्थान |
|---|---|
| रेल निर्माण GUI/तर्क | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; सड़क `road_gui.cpp/road_cmd.cpp` |
| लागत अनुमान | प्रत्येक `*_cmd.cpp` का `DoCommand` `CommandCost` लौटाता है; GUI में `DC_QUERY_COST` मोड का उपयोग करके मूल्य पूछा जा सकता है |
| माउस के नीचे टाइल | `src/viewport_func.h:36` — `GetTileBelowCursor()`; `_cursor.pos` (स्क्रीन निर्देशांक); `src/viewport.cpp:1056` |
| मौजूदा पाठ संकेत तंत्र | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)` (विश्व निर्देशांक फ्लोटिंग टेक्स्ट), `UpdateTextEffect`; `src/texteff.cpp` |
| मौजूदा निर्माण संकेत UX | रेल टूलबार `OnPlaceDrag` खींचते समय चयनित क्षेत्र और लागत संचय (`_thd` tilehighlight, `src/tilehighlight_func.h`); स्टेटस बार `statusbar_gui.cpp` उपकरण लागत दिखा सकता है |
| प्रति फ्रेम रिफ्रेश बिंदु | प्रत्येक टूलबार विंडो `OnMouseLoop` / `viewport.cpp` का `HandleMouseEvents` (पंक्तियाँ 5422/5733) |

**वर्तमान स्थिति निष्कर्ष**: "माउस के ऊपर मूल्य टूलटिप" नहीं है। `AddTextEffect` का उपयोग करके माउस के टाइल पर लंगर डाला जा सकता है (कर्सर के टाइल का अनुसरण करता है), या स्क्रीन निर्देशांक टूलटिप स्वयं खींची जा सकती है; लागत के लिए वर्तमान उपकरण के लिए `DC_QUERY_COST` का उपयोग करके एकल टाइल मूल्य पूछा जा सकता है + ड्रैग चयन क्षेत्र संचय।

### 4.5 AI इंटरफ़ेस (NoAI संरक्षित + संपूर्ण-गेम अवेयर AI)

| ध्यान बिंदु | स्थान |
|---|---|
| NoAI फ्रेमवर्क | `src/ai/` — `ai_core.cpp` (AICore मुख्य लूप), `ai_instance.cpp` (AIInstance/Squirrel VM), `ai_scanner.cpp` (`ai/` निर्देशिका स्कैन करता है), `ai_gui.cpp` (चयन/कॉन्फ़िगरेशन), `ai_config.cpp` |
| GameScript फ्रेमवर्क | `src/game/` — `game_core.cpp`, `game_instance.cpp` आदि (GS "देवता मोड" है, AI से अधिक अनुमतियाँ) |
| स्क्रिप्ट API क्लासेज़ | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin` आदि, कुल 60+ क्लासेज़) |
| API स्वचालित पंजीकरण | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` स्वचालित रूप से `ai_*.sq.hpp`/`gs_*.sq.hpp` बाइंडिंग उत्पन्न करता है; `.cpp` को स्रोत सूची में शामिल करना होगा (पंक्ति 235 से) |
| Squirrel बाइंडिंग मैक्रो | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` AI नियंत्रक बाइंडिंग |
| इंस्टेंस शेड्यूलिंग | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (निलंबित/पुनर्स्थापित), ईवेंट `script_event*` |
| कंपनी एक्सेस नियंत्रण | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) GS देवता मोड को अलग करता है; कंपनी API पैरामीटर सत्यापन `ResolveCompanyID`, `EnforceCompanyModeValid` |
| स्क्रिप्ट सेटिंग | `src/table/settings/script_settings.ini` (`game.script.*`); AI इंस्टेंस कॉन्फ़िगरेशन `ai_config.cpp` |

**वर्तमान स्थिति निष्कर्ष**: आधुनिक स्क्रिप्ट API में, प्रतिस्पर्धी कंपनियों की कुछ जानकारी (जैसे `GetBankBalance`) पहले से ही प्रतिबंधित नहीं है, लेकिन **संपूर्ण-गेम जागरूकता के लिए कोई एग्रीगेशन API नहीं है** ("सभी कंपनियों/वैश्विक अर्थव्यवस्था/वैश्विक मानचित्र सांख्यिकी की गणना" का कोई एकीकृत प्रवेश द्वार नहीं), और न ही स्विच-नियंत्रित "वैश्विक AI" एक्सेस बिंदु है। एक नया `ScriptGlobal` API क्लास (GLOB स्वचालित पंजीकरण) + एक "AI को वैश्विक जागरूकता की अनुमति दें" `game.script` सेटिंग जोड़कर कार्यान्वित किया जा सकता है, GS हमेशा उपलब्ध (देवता मोड), AI स्विच द्वारा नियंत्रित — अर्थात "NoAI संरक्षित करें, संपूर्ण-गेम अवेयर AI जोड़ें, एक्सेस नियंत्रण के साथ"।

---

## 5. संशोधन जोखिम अवलोकन

| सुविधा | मुख्य संशोधित फ़ाइलें | जोखिम |
|---|---|---|
| F1 डाउनलोड मल्टी-स्रोत/मल्टी-थ्रेड | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | मध्यम (नेटवर्क कॉलबैक स्टेट मशीन सावधानीपूर्वक) |
| F2 सीमा विस्तार | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (वैकल्पिक uint16) | कम-मध्यम (कंपनी 252 कम जोखिम; क्लाइंट >255 के लिए प्रोटोकॉल परिवर्तन आवश्यक) |
| F3 स्वचालित वाहन समूहीकरण | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | कम |
| F4 निर्माण टूलटिप | नया `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` हुक, `src/lang/english.txt` | कम-मध्यम |
| F5 संपूर्ण-गेम अवेयर AI | नया `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, उदाहरण AI `bin/ai/GlobalAI/` | कम (फ्रेमवर्क स्वचालित पंजीकरण) |

> नोट: सभी परिवर्तन `jgrpp` शाखा स्थानीय कार्यक्षेत्र पर आधारित हैं, संकलित/सत्यापित नहीं (इस मशीन पर कोई उपयोगी निर्माण उपकरण श्रृंखला नहीं है); सभी पैच `git diff` समीक्षा के बाद `git apply` किए जा सकते हैं।