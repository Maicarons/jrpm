---
title: मल्टीप्लेयर गाइड
---

# मल्टीप्लेयर गाइड

## संस्करण संगतता रणनीति

jrpm एक **टैग किया गया संस्करण** है (`jrpm-0.1.0`), मल्टीप्लेयर हैंडशेक के लिए संशोधन स्ट्रिंग का **पूर्ण मिलान** आवश्यक है:

| परिदृश्य | व्यवहार |
|---|---|
| jrpm क्लाइंट ↔ jrpm सर्वर | ✅ सामान्य मल्टीप्लेयर (संस्करण समान) |
| jrpm क्लाइंट ↔ मूल jgrpp / pulsexlb सर्वर | ❌ अस्वीकृत (संस्करण पृथक्करण) |
| मूल jgrpp क्लाइंट → jrpm सर्वर | ✅ अनुमति (सर्वर-साइड `jgrpp-*` संशोधन स्वीकार करता है) |
| pulsexlb क्लाइंट → jrpm सर्वर | ✅ अनुमति (`pxp` संशोधन स्वीकार करता है) |

::: warning NewGRF संस्करण
क्लाइंट किसी भी संस्करण से आए, **NewGRF संस्करण संख्या सर्वर से पूरी तरह मेल खानी चाहिए** (`_openttd_newgrf_version` सख्त जाँच), यह नियतात्मक सिमुलेशन की आधार रेखा है।
:::

## सर्वर स्थापित करना

```bash
# समर्पित सर्वर (GUI के बिना)
openttd-jrpm -D -c server.cfg
```

`server.cfg` के लिए अनुशंसित कॉन्फ़िगरेशन (विस्तृत जानकारी के लिए [सर्वर प्रदर्शन ट्यूनिंग](../performance/server-tuning) देखें):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## सर्वर से जुड़ना

- गेम के अंदर "मल्टीप्लेयर → इंटरनेट सर्वर से जुड़ें/सर्वर जोड़ें", या
- कमांड लाइन: `openttd-jrpm -n <host>:<port>`

## सामग्री डाउनलोड (NewGRF/परिदृश्य)

jrpm **मल्टी-मिरर + समानांतर डाउनलोड** का समर्थन करता है:

```ini
[network]
content_server = content.openttd.org        ; मेटाडेटा सर्वर
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; समानांतर डाउनलोड फ़ाइलें (1-8)
```

- मिरर सूची अल्पविराम से अलग, क्रम में उपयोग की जाती है; एक मिरर विफल होने पर स्वचालित रूप से अगले पर स्विच हो जाती है;
- पर्यावरण चर `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` सेटिंग्स से अधिक प्राथमिकता रखते हैं;
- सभी मिरर विफल होने पर स्वचालित रूप से पुराने TCP डाउनलोड प्रोटोकॉल पर वापस आ जाता है।

## सर्वर प्रबंधन

- कंसोल कमांड: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (स्वचालित वाहन समूहीकरण) आदि;
- RCON: सर्वर सेटिंग `rcon_password` सेट करने के बाद दूरस्थ रूप से कंसोल कमांड भेजे जा सकते हैं।