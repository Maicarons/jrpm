---
title: মাল্টিপ্লেয়ার গাইড
---

# মাল্টিপ্লেয়ার গাইড

## সংস্করণ সামঞ্জস্য নীতি

jrpm একটি **tagged সংস্করণ** (`jrpm-0.1.0`), মাল্টিপ্লেয়ার হ্যান্ডশেকের জন্য রিভিশন স্ট্রিং **সম্পূর্ণ মিল** প্রয়োজন:

| দৃশ্য | আচরণ |
|---|---|
| jrpm ক্লায়েন্ট ↔ jrpm সার্ভার | ✅ স্বাভাবিক মাল্টিপ্লেয়ার (সংস্করণ একই) |
| jrpm ক্লায়েন্ট ↔ মূল jgrpp / pulsexlb সার্ভার | ❌ প্রত্যাখ্যান (সংস্করণ বিচ্ছিন্ন) |
| মূল jgrpp ক্লায়েন্ট → jrpm সার্ভার | ✅ অনুমোদিত (সার্ভার পক্ষ থেকে `jgrpp-*` রিভিশন গ্রহণ করা হয়) |
| pulsexlb ক্লায়েন্ট → jrpm সার্ভার | ✅ অনুমোদিত (`pxp` রিভিশন গ্রহণ করা হয়) |

::: warning NewGRF সংস্করণ
ক্লায়েন্ট যেকোনো সংস্করণ থেকে আসুক না কেন, **NewGRF সংস্করণ নম্বর অবশ্যই সার্ভারের সাথে সম্পূর্ণ মিলতে হবে** (`_openttd_newgrf_version` কঠোরভাবে যাচাই করা হয়), এটি নির্ধারিত সিমুলেশনের নিম্নসীমা।
:::

## সার্ভার খোলা

```bash
# ডেডিকেটেড সার্ভার (GUI ছাড়া)
openttd-jrpm -D -c server.cfg
```

`server.cfg`-এর জন্য প্রস্তাবিত কনফিগারেশন (বিশদ জানতে [সার্ভার পারফরম্যান্স টিউনিং](../performance/server-tuning) দেখুন):

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

## সার্ভারে যোগদান

- গেমের ভিতরে "মাল্টিপ্লেয়ার → ইন্টারনেট সার্ভারে যোগ দিন/সার্ভার যোগ করুন", অথবা
- কমান্ড লাইন: `openttd-jrpm -n <host>:<port>`

## কন্টেন্ট ডাউনলোড (NewGRF/সিনারিও)

jrpm **একাধিক মিরর + সমান্তরাল ডাউনলোড** সমর্থন করে:

```ini
[network]
content_server = content.openttd.org        ; মেটাডেটা সার্ভার
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; সমান্তরাল ডাউনলোড ফাইল সংখ্যা (1-8)
```

- মিরর তালিকা কমা দিয়ে বিচ্ছিন্ন, ক্রমানুসারে ব্যবহার করা হয়; কোনো মিরর ব্যর্থ হলে স্বয়ংক্রিয়ভাবে পরবর্তী মিররে সুইচ করে;
- এনভায়রনমেন্ট ভেরিয়েবল `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` সেটিং আইটেমের চেয়ে উচ্চ অগ্রাধিকার পায়;
- সব মিরর ব্যর্থ হলে স্বয়ংক্রিয়ভাবে পুরনো TCP ডাউনলোড প্রোটোকলে ফিরে যায়।

## সার্ভার ব্যবস্থাপনা

- কনসোল কমান্ড: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (যানবাহন স্বয়ংক্রিয় গ্রুপিং) ইত্যাদি;
- RCON: সার্ভার সেটিং `rcon_password` সেট করলে দূর থেকে কনসোল কমান্ড পাঠানো যায়।