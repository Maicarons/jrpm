---
title: বিল্ড ও ইনস্টলেশন
---

# বিল্ড ও ইনস্টলেশন

## নির্ভরতা (ডিপেন্ডেন্সি)

| আইটেম | বিবরণ |
|---|---|
| CMake ≥ 3.17 | বিল্ড সিস্টেম |
| C++20 কম্পাইলার | MSVC 2019+ / GCC 11+ / Clang 14+ |
| লাইব্রেরি | zlib, liblzma, lzo, zstd, libpng; GUI-এর জন্য SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows-এ WinHttp ব্যবহার করে, curl-এর প্রয়োজন নেই) |

Windows-এর জন্য [vcpkg](https://github.com/microsoft/vcpkg) ব্যবহার করে নির্ভরতা ইনস্টল করার পরামর্শ দেওয়া হচ্ছে (প্রকল্পে `vcpkg.json` আছে)।

## বিল্ড ধাপ

```bash
# 1. কনফিগার (প্রকল্প রুট ডিরেক্টরির build সাবডিরেক্টরিতে)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. কম্পাইল
cmake --build build --config Release -j

# 3. আউটপুট
# build/openttd-jrpm.exe        (এক্সিকিউটেবল ফাইলের নাম, jrpm সংস্করণ)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip সংস্করণ শনাক্তকারী
বিল্ডের সময় রুট ডিরেক্টরির `.ottdrev-vc` ফাইল থেকে সংস্করণ নম্বর তৈরি হয়। বর্তমানে `jrpm-0.1.0` (tagged)। `openttd-jrpm -v` চালিয়ে সংস্করণ ও বিল্ড তথ্য নিশ্চিত করা যায়।
:::

## সাধারণ সমস্যা

### CMake নির্ভরতা খুঁজে পাচ্ছে না
প্রথমে vcpkg দিয়ে ইনস্টল করুন: `vcpkg install` (`vcpkg.json` অনুযায়ী)। অথবা সিস্টেম প্যাকেজ ইনস্টল করুন (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`)।

### শুধু ডেডিকেটেড সার্ভার চালাতে চান (GUI ছাড়া)
`cmake -B build -DOPTION_DEDICATED=ON`, তারপর `openttd-jrpm -D -c server.cfg`।

### বিল্ডে [safe-delete] অপারেশন ব্যর্থ (Windows স্যান্ডবক্স/CI)
বিল্ড কমান্ডের আগে `NODE_OPTIONS=""` যোগ করে নিরাপদ ডিলিট শিমের `.temp`/`.cache` ক্লিনিং-এর ব্যাঘাত এড়ানো যায়।

## সোর্স থেকে চালানো

```bash
# ডেডিকেটেড সার্ভার
openttd-jrpm -D -c openttd.cfg

# GUI ক্লায়েন্ট
openttd-jrpm
```