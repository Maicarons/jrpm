---
title: البناء والتثبيت
---

# البناء والتثبيت

## التبعيات

| المشروع | الوصف |
|---|---|
| CMake ≥ 3.17 | نظام البناء |
| مترجم C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| المكتبات | zlib، liblzma، lzo، zstd، libpng؛ لواجهة المستخدم الرسومية أيضاً SDL2/Allegro، freetype، harfbuzz، ICU، opusfile (Windows يستخدم WinHttp، لا حاجة لـ curl) |

يوصى باستخدام [vcpkg](https://github.com/microsoft/vcpkg) لتثبيت التبعيات على Windows (المشروع يأتي مع `vcpkg.json`).

## خطوات البناء

```bash
# 1. التكوين (في المجلد الفرعي build في جذر المشروع)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. التجميع
cmake --build build --config Release -j

# 3. الناتج
# build/openttd-jrpm.exe        (اسم الملف التنفيذي، إصدار jrpm)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip معرف الإصدار
عند البناء، يُقرأ `.ottdrev-vc` في جذر المشروع لإنشاء رقم الإصدار. حالياً هو `jrpm-0.1.0` (موسوم). شغّل `openttd-jrpm -v` لتأكيد الإصدار ومعلومات البناء.
:::

## الأسئلة الشائعة

### CMake لا يجد التبعيات
قم أولاً بتثبيتها عبر vcpkg: `vcpkg install` (وفقاً لـ `vcpkg.json`). أو قم بتثبيت الحزم النظامية (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### أريد فقط تشغيل خادم مخصص (بدون واجهة رسومية)
`cmake -B build -DOPTION_DEDICATED=ON`، ثم `openttd-jrpm -D -c server.cfg`.

### خطأ بناء [safe-delete] فشل (Windows sandbox/CI)
أضف البادئة `NODE_OPTIONS=""` لأمر البناء لتجاوز تداخل حذف الأمان مع تنظيف `.temp`/`.cache`.

## التشغيل من المصدر

```bash
# خادم مخصص
openttd-jrpm -D -c openttd.cfg

# عميل بواجهة رسومية
openttd-jrpm
```