---
title: निर्माण और स्थापना
---

# निर्माण और स्थापना

## निर्भरताएँ

| आइटम | विवरण |
|---|---|
| CMake ≥ 3.17 | निर्माण प्रणाली |
| C++20 कंपाइलर | MSVC 2019+ / GCC 11+ / Clang 14+ |
| लाइब्रेरीज़ | zlib, liblzma, lzo, zstd, libpng; GUI के लिए SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows WinHttp का उपयोग करता है, curl की आवश्यकता नहीं) |

Windows पर निर्भरताएँ स्थापित करने के लिए [vcpkg](https://github.com/microsoft/vcpkg) का उपयोग करने की अनुशंसा की जाती है (प्रोजेक्ट में अपना `vcpkg.json` है)।

## निर्माण चरण

```bash
# 1. कॉन्फ़िगरेशन (प्रोजेक्ट रूट के build उपनिर्देशिका में)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. संकलन
cmake --build build --config Release -j

# 3. आउटपुट
# build/openttd-jrpm.exe        (निष्पादन योग्य फ़ाइल नाम, jrpm संस्करण)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip संस्करण पहचानकर्ता
निर्माण के दौरान रूट निर्देशिका से `.ottdrev-vc` पढ़कर संस्करण संख्या उत्पन्न की जाती है। वर्तमान में `jrpm-0.1.0` (टैग किया गया)। `openttd-jrpm -v` चलाकर संस्करण और निर्माण जानकारी की पुष्टि करें।
:::

## सामान्य प्रश्न

### CMake को निर्भरताएँ नहीं मिल रहीं
पहले vcpkg के माध्यम से स्थापित करें: `vcpkg install` (`vcpkg.json` के अनुसार)। या सिस्टम पैकेज स्थापित करें (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`)।

### केवल समर्पित सर्वर चलाना चाहते हैं (GUI के बिना)
`cmake -B build -DOPTION_DEDICATED=ON`, फिर `openttd-jrpm -D -c server.cfg`।

### निर्माण में [safe-delete] ऑपरेशन विफल (Windows सैंडबॉक्स/CI)
निर्माण कमांड से पहले `NODE_OPTIONS=""` उपसर्ग जोड़ें ताकि सुरक्षा हटाने वाले shim द्वारा `.temp`/`.cache` सफाई में हस्तक्षेप से बचा जा सके।

## स्रोत से चलाना

```bash
# समर्पित सर्वर
openttd-jrpm -D -c openttd.cfg

# GUI क्लाइंट
openttd-jrpm
```