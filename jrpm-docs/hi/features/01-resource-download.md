---
title: "संसाधन डाउनलोड: मल्टी-मिरर + समानांतर डाउनलोड"
---

## वर्तमान स्थिति (अनुसंधान निष्कर्ष)

| आइटम | वर्तमान स्थिति |
|---|---|
| सामग्री सर्वर (मेटाडेटा प्रोटोकॉल) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: पर्यावरण चर `OTTD_CONTENT_SERVER_CS`, डिफ़ॉल्ट `content.openttd.org` (TCP पोर्ट 3978) |
| डाउनलोड मिरर | `NetworkContentMirrorUriString()`: पर्यावरण चर `OTTD_CONTENT_MIRROR_URI`, डिफ़ॉल्ट `https://binaries.openttd.org/bananas` |
| डाउनलोड विधि | `network_content.cpp` `DownloadSelectedContentHTTP()`: **एकल** POST अनुरोध सभी content ID → मिरर फ़ाइल हेडर सूची लौटाता है (`id,type,filesize,url` प्रति पंक्ति) → प्रत्येक फ़ाइल **क्रमिक रूप से** डाउनलोड होती है (प्रति फ़ाइल एक GET कनेक्शन) → `AfterDownload()` gunzip + tar अनपैक |
| थ्रेड मॉडल | HTTP लेयर (WinHttp) पृष्ठभूमि थ्रेड में संसाधित होती है, लेकिन **फ़ाइल डाउनलोड क्रमिक रूप से कतारबद्ध** है; कोई मिरर सूची नहीं, कोई इन-गेम सेटिंग नहीं, कोई समानांतरता नहीं |

## इस सुविधा का कार्यान्वयन

### 1. नई सेटिंग्स (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, डिफ़ॉल्ट खाली = आधिकारिक स्रोत का उपयोग करें)
- `network.content_mirrors` (SLE_STR, अल्पविराम से अलग कई मिरर URI, डिफ़ॉल्ट खाली = आधिकारिक मिरर)

प्राथमिकता: पर्यावरण चर > गेम सेटिंग्स > आधिकारिक डिफ़ॉल्ट।

### 2. कॉन्फ़िगरेशन पार्सिंग (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` सेटिंग पढ़ता है;
- नया `NetworkContentMirrorUris()` अल्पविराम से अलग सूची पार्स करता है (रिक्त स्थान हटाएं, खाली आइटम छोड़ें, आधिकारिक स्रोत पर वापस जाएं);
- `NetworkContentMirrorUriString()` अब सूची का पहला तत्व लौटाता है।

### 3. समानांतर डाउनलोड (`src/network/network_content.h/.cpp`)

- नया `ContentFileDownload` (एकल डाउनलोड के लिए फ़ाइल: id/type/filesize/url/filename);
- नया `ContentDownloadSession : HTTPCallback` (प्रत्येक फ़ाइल के लिए स्वतंत्र डाउनलोड स्थिति और कॉलबैक; `IsCancelled` हैंडलर से जुड़ा);
- `DownloadSelectedContentHTTP()`: POST `mirrors[mirror_index]` पर → `ParseResponseHeaders()` एक बार में सभी फ़ाइल हेडर पार्स करता है → `StartDownloadSessions()` अधिकतम **4 समानांतर सत्र** शुरू करता है (`CONTENT_DOWNLOAD_PARALLEL`), प्रत्येक सत्र पूरा होने पर स्वचालित रूप से अगली डाउनलोड फ़ाइल लेता है;
- विफलता पुनर्प्रयास श्रृंखला: सत्र विफलता/मिरर अनुरोध विफलता → अगले मिरर पर पुनः अनुरोध → सभी मिरर विफल → पुराने TCP प्रोटोकॉल पर वापसी (`DownloadSelectedContentFallback`);
- सुरुचिपूर्ण समापन: विफलता/रद्दीकरण पर `download_cancelled` सेट करें, सभी चल रहे सत्रों के समाप्त होने की प्रतीक्षा करें (`OnAllSessionsDone`) फिर पुनर्प्रयास या वापसी करें, लटकते कॉलबैक से बचें;
- `ResetMirrorIndex()`: नया डाउनलोड पहले मिरर से शुरू होता है (GUI ट्रिगर होने पर कॉल किया जाता है)।

## शामिल फ़ाइलें

- `src/table/settings/network_settings.ini` (2 नई सेटिंग्स)
- `src/settings_type.h` (`NetworkSettings` में 2 नए फ़ील्ड)
- `src/network/core/config.cpp` / `config.h` (मिरर सूची पार्सिंग)
- `src/network/network_content.h` / `.cpp` (समानांतर सत्र)
- `src/network/network_content_gui.cpp` (डाउनलोड से पहले मिरर इंडेक्स रीसेट)

## सत्यापन बिंदु

1. `network.content_mirrors` में कई URI (अल्पविराम से अलग) भरने के बाद, सामग्री डाउनलोड काम करना चाहिए और कई फ़ाइलों को **समानांतर** डाउनलोड करना चाहिए (डाउनलोड प्रगति देखें/एकाधिक समवर्ती कनेक्शन कैप्चर करें);
2. नेटवर्क डाउन/गलत मिरर पर स्वचालित रूप से अगले मिरर का प्रयास करना चाहिए, अंत में पुराने प्रोटोकॉल पर वापस आना चाहिए;
3. डाउनलोड के दौरान रद्द करने पर `.tar.gz` अधूरी फ़ाइलें नहीं रहनी चाहिए;
4. वास्तविक मशीन पर संकलन और सत्यापन आवश्यक है (इस रिपॉजिटरी में कोई निर्माण वातावरण नहीं है, कोड संकलित नहीं किया गया)।

## ज्ञात सीमाएँ

- समानांतरता हार्डकोडेड 4 (`CONTENT_DOWNLOAD_PARALLEL`), बाद में सेटिंग बनाई जा सकती है;
- मिरर प्रोटोकॉल आधिकारिक bananas API प्रारूप पर निर्भर करता है (POST id सूची फ़ाइल हेडर सूची लौटाती है)।