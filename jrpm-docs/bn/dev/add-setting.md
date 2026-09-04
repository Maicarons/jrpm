---
title: নতুন সেটিংস আইটেম
---

# নতুন সেটিংস আইটেম

jrpm-এর সেটিংস সিস্টেম **INI-চালিত**: `src/table/settings/*.ini` সেটিংস ডেফিনিশন উৎস, বিল্ড-টাইম settingsgen দ্বারা কোড জেনারেট; সংশ্লিষ্ট স্ট্রাকচার ফিল্ড `src/settings_type.h`-এ। এই প্রকল্পে বাস্তবায়িত `network.content_download_parallel` উদাহরণ হিসেবে ব্যবহৃত।

## ১. স্ট্রাকচার ফিল্ড

`src/settings_type.h`, সংশ্লিষ্ট সেটিংস স্ট্রাকচার খুঁজুন (নেটওয়ার্ক → `NetworkSettings`, গেম → `GameSettings` সম্পর্কিত সাবস্ট্রাকচার, স্ক্রিপ্ট → `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< কমা-বিচ্ছিন্ন মিরর URI
	uint8_t content_download_parallel = 4;                ///< সমান্তরাল ডাউনলোড ফাইল সংখ্যা
	// ...
};
```

## ২. INI সংজ্ঞা

`src/table/settings/network_settings.ini`, `[SDTC_VAR]` ব্লক যোগ করুন (ক্লায়েন্ট সেটিংস `SDTC_*` ব্যবহার করে, গেম সেটিংস `SDT_*` ব্যবহার করে):

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

সাধারণ ফিল্ড:

| ফিল্ড | ব্যাখ্যা |
|---|---|
| `var` | সেটিংসের সম্পূর্ণ নাম (স্ট্রাকচার পাথ, যেমন `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (স্ট্রিং, `length` প্রয়োজন) |
| `flags` | `SettingFlag::NotInSave` (存档ে না), `NoNetworkSync` (সিঙ্ক না), `NetworkOnly`, `GuiZeroIsSpecial` ইত্যাদি |
| `def/min/max/interval` | ডিফল্ট মান/পরিসীমা/ধাপ |
| `str/strhelp` | সেটিংস ইন্টারফেসে প্রদর্শিত স্ট্রিং |
| `cat` | সেটিংস ক্যাটাগরি (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## ৩. স্ট্রিং (সেটিংস ইন্টারফেস প্রদর্শন)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- বুলিয়ান সেটিংস `{STRING2}` (চালু/বন্ধ) দিয়ে শেষ হয়;
- সংখ্যাসূচক সেটিংসের জন্য অনুরূপ স্ট্রিং ফরম্যাট দেখুন।

## ৪. কোডে ব্যবহার

```cpp
// ক্লায়েন্ট সেটিংস
_settings_client.network.content_download_parallel

// গেম সেটিংস
_settings_game.script.allow_global_ai_access
```

## ৫. বিশেষ দৃশ্য

- **সার্ভার সেটিংস**: `network.*` মাল্টিপ্লেয়ার গেমে সার্ভার থেকে下发 হয় (`NetworkOnly` + সার্ভার `sync` মেকানিজম);
- **存档 সম্পর্ক**: গেম সেটিংস (`game.*`)存档সহ সংরক্ষিত হয় (ডিফল্ট); ক্লায়েন্ট সেটিংস (`network.*`/`gui.*`) ডিফল্ট `NotInSave`;
- **কলব্যাক**: `pre_cb`/`post_cb` মান পরিবর্তন হ্যান্ডলিং-এ হুক করা যেতে পারে (যেমন `UpdateClientConfigValues()`)।

## সম্পূর্ণ তালিকা

- [ ] `settings_type.h` ফিল্ড
- [ ] সংশ্লিষ্ট `.ini`-তে `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` ব্লক
- [ ] (সেটিংস ইন্টারফেস প্রদর্শনের জন্য) english.txt স্ট্রিং
- [ ] পুনরায় configure + বিল্ড