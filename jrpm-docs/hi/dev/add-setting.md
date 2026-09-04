---
title: नई सेटिंग आइटम जोड़ना
---

# नई सेटिंग आइटम जोड़ना

jrpm की सेटिंग प्रणाली **INI-चालित** है: `src/table/settings/*.ini` सेटिंग परिभाषा स्रोत हैं, निर्माण के दौरान settingsgen कोड उत्पन्न करता है; संबंधित स्ट्रक्ट फ़ील्ड `src/settings_type.h` में हैं। इस परियोजना में कार्यान्वित `network.content_download_parallel` को उदाहरण के रूप में लें।

## 1. स्ट्रक्ट फ़ील्ड

`src/settings_type.h`, संबंधित सेटिंग स्ट्रक्ट खोजें (नेटवर्क → `NetworkSettings`, गेम → `GameSettings` संबंधित उप-स्ट्रक्ट, स्क्रिप्ट → `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< अल्पविराम से अलग मिरर URI
	uint8_t content_download_parallel = 4;                ///< समानांतर डाउनलोड फ़ाइलें
	// ...
};
```

## 2. INI परिभाषा

`src/table/settings/network_settings.ini`, `[SDTC_VAR]` ब्लॉक जोड़ें (क्लाइंट सेटिंग के लिए `SDTC_*`, गेम सेटिंग के लिए `SDT_*`):

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

सामान्य फ़ील्ड:

| फ़ील्ड | विवरण |
|---|---|
| `var` | सेटिंग का पूरा नाम (स्ट्रक्ट पथ, जैसे `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (स्ट्रिंग, `length` आवश्यक) |
| `flags` | `SettingFlag::NotInSave` (सेव में नहीं), `NoNetworkSync` (सिंक नहीं), `NetworkOnly`, `GuiZeroIsSpecial` आदि |
| `def/min/max/interval` | डिफ़ॉल्ट मान/सीमा/चरण |
| `str/strhelp` | सेटिंग इंटरफ़ेस प्रदर्शन स्ट्रिंग |
| `cat` | सेटिंग श्रेणी (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. स्ट्रिंग (सेटिंग इंटरफ़ेस प्रदर्शन)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- बूलियन सेटिंग `{STRING2}` (चालू/बंद) के साथ समाप्त होती है;
- संख्यात्मक सेटिंग के लिए समान स्ट्रिंग प्रारूप देखें।

## 4. कोड में उपयोग

```cpp
// क्लाइंट सेटिंग
_settings_client.network.content_download_parallel

// गेम सेटिंग
_settings_game.script.allow_global_ai_access
```

## 5. विशेष परिदृश्य

- **सर्वर-साइड सेटिंग**: `network.*` मल्टीप्लेयर गेम में सर्वर द्वारा भेजा जाता है (`NetworkOnly` + सर्वर `sync` तंत्र);
- **सेव संबंध**: गेम सेटिंग (`game.*`) सेव के साथ सहेजी जाती है (डिफ़ॉल्ट); क्लाइंट सेटिंग (`network.*`/`gui.*`) डिफ़ॉल्ट `NotInSave`;
- **कॉलबैक**: `pre_cb`/`post_cb` मान परिवर्तन प्रसंस्करण से जोड़ा जा सकता है (जैसे `UpdateClientConfigValues()`)।

## पूर्णता सूची

- [ ] `settings_type.h` फ़ील्ड
- [ ] संबंधित `.ini` में `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` ब्लॉक
- [ ] (सेटिंग इंटरफ़ेस प्रदर्शन के लिए) english.txt स्ट्रिंग
- [ ] पुन: कॉन्फ़िगर + निर्माण