---
title: "রিসোর্স ডাউনলোড: একাধিক মিরর + সমান্তরাল ডাউনলোড"
---

## বর্তমান অবস্থা (গবেষণা সিদ্ধান্ত)

| আইটেম | বর্তমান অবস্থা |
|---|---|
| কন্টেন্ট সার্ভার (মেটাডেটা প্রোটোকল) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: এনভায়রনমেন্ট ভেরিয়েবল `OTTD_CONTENT_SERVER_CS`, ডিফল্ট `content.openttd.org` (TCP 3978 পোর্ট) |
| ডাউনলোড মিরর | `NetworkContentMirrorUriString()`: এনভায়রনমেন্ট ভেরিয়েবল `OTTD_CONTENT_MIRROR_URI`, ডিফল্ট `https://binaries.openttd.org/bananas` |
| ডাউনলোড পদ্ধতি | `network_content.cpp` `DownloadSelectedContentHTTP()`: **একক** POST রিকোয়েস্ট সব content ID → মিরর ফাইল হেডার তালিকা ফেরত দেয় (`id,type,filesize,url` প্রতি লাইনে) → **একের পর এক সিরিয়াল** ডাউনলোড (প্রতি ফাইলের জন্য একটি GET সংযোগ) → `AfterDownload()` gunzip + tar আনপ্যাক |
| থ্রেড মডেল | HTTP লেয়ার (WinHttp) ব্যাকগ্রাউন্ড থ্রেডে কাজ করে, কিন্তু **ফাইল ডাউনলোড সিরিয়ালি সারিবদ্ধ**; মিরর তালিকা নেই, গেমের ভিতরে সেটিংস নেই, সমান্তরাল নেই |

## এই বৈশিষ্ট্যের বাস্তবায়ন

### ১. নতুন সেটিংস (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, ডিফল্ট খালি = অফিসিয়াল উৎস ব্যবহার)
- `network.content_mirrors` (SLE_STR, কমা দিয়ে বিচ্ছিন্ন একাধিক মিরর URI, ডিফল্ট খালি = অফিসিয়াল মিরর)

অগ্রাধিকার: এনভায়রনমেন্ট ভেরিয়েবল > গেম সেটিংস > অফিসিয়াল ডিফল্ট।

### ২. কনফিগারেশন পার্সিং (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` সেটিংস পড়ে;
- নতুন `NetworkContentMirrorUris()` কমা-বিচ্ছিন্ন তালিকা পার্স করে (স্পেস বাদ দেওয়া, খালি এন্ট্রি স্কিপ, অফিসিয়াল উৎসে ফিরে যাওয়া);
- `NetworkContentMirrorUriString()` তালিকার প্রথমটি ফেরত দেয়।

### ৩. সমান্তরাল ডাউনলোড (`src/network/network_content.h/.cpp`)

- নতুন `ContentFileDownload` (একক ডাউনলোড-অপেক্ষামান ফাইল: id/type/filesize/url/filename);
- নতুন `ContentDownloadSession : HTTPCallback` (প্রতি ফাইলের স্বাধীন ডাউনলোড স্টেট ও কলব্যাক; `IsCancelled` হ্যান্ডলারের সাথে সংযুক্ত);
- `DownloadSelectedContentHTTP()`: `mirrors[mirror_index]`-এ POST → `ParseResponseHeaders()` একবারে সব ফাইল হেডার পার্স করে → `StartDownloadSessions()` সর্বোচ্চ **৪টি সমান্তরাল সেশন** শুরু করে (`CONTENT_DOWNLOAD_PARALLEL`), প্রতিটি সেশন শেষ হলে স্বয়ংক্রিয়ভাবে পরবর্তী অপেক্ষমাণ ফাইল গ্রহণ করে;
- ব্যর্থতা পুনরায় চেষ্টা চেইন: সেশন ব্যর্থ/মিরর রিকোয়েস্ট ব্যর্থ → পরবর্তী মিররে পুনরায় রিকোয়েস্ট → সব মিরর ব্যর্থ → পুরনো TCP প্রোটোকলে ফিরে যাওয়া (`DownloadSelectedContentFallback`);
- সুন্দর সমাপ্তি: ব্যর্থ/বাতিল হলে `download_cancelled` সেট করে, অপেক্ষমাণ সেশন সব শেষ হওয়ার পর (`OnAllSessionsDone`) পুনরায় চেষ্টা বা ফিরে যাওয়া, ঝুলন্ত কলব্যাক এড়ানো;
- `ResetMirrorIndex()`: নতুন ডাউনলোড প্রথম মিরর থেকে শুরু করে (GUI ট্রিগার করলে কল করা হয়)।

## জড়িত ফাইল

- `src/table/settings/network_settings.ini` (২টি নতুন সেটিংস আইটেম)
- `src/settings_type.h` (`NetworkSettings`-এ ২টি নতুন ফিল্ড)
- `src/network/core/config.cpp` / `config.h` (মিরর তালিকা পার্সিং)
- `src/network/network_content.h` / `.cpp` (সমান্তরাল সেশন)
- `src/network/network_content_gui.cpp` (ডাউনলোডের আগে মিরর ইনডেক্স রিসেট)

## যাচাইকরণ পয়েন্ট

১. `network.content_mirrors`-এ একাধিক URI (কমা দিয়ে বিচ্ছিন্ন) পূরণ করার পর, কন্টেন্ট ডাউনলোড কাজ করা উচিত এবং **সমান্তরাল**ভাবে একাধিক ফাইল ডাউনলোড করা উচিত (ডাউনলোড অগ্রগতি পর্যবেক্ষণ/একাধিক সমবর্তী কানেকশন ক্যাপচার);
২. নেটওয়ার্ক বন্ধ/ভুল মিররের অধীনে স্বয়ংক্রিয়ভাবে পরবর্তী মিরর চেষ্টা করা উচিত, শেষে পুরনো প্রোটোকলে ফিরে যাওয়া;
৩. ডাউনলোড বাতিল করলে `.tar.gz` অর্ধ-সমাপ্ত ফাইল থাকা উচিত নয়;
৪. প্রকৃত মেশিনে কম্পাইল করে যাচাই করতে হবে (এই রিপোজিটরিতে বিল্ড পরিবেশ নেই, কোড কম্পাইল করা হয়নি)।

## জ্ঞাত সীমাবদ্ধতা

- সমান্তরালতা হার্ডকোডেড ৪ (`CONTENT_DOWNLOAD_PARALLEL`), পরে সেটিংস করা যেতে পারে;
- মিরর প্রোটোকল অফিসিয়াল bananas API ফরম্যাটের উপর নির্ভর করে (POST id তালিকা ফেরত ফাইল হেডার তালিকা)।