---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "Versi OpenTTD yang Ditingkatkan dengan Fitur Pilihan"
  tagline: Kopling Lokomotif · Bandara Modular · Unduhan Paralel · AI Sadar Permainan · Pemutaran Ulang Perintah · Cetak Biru · Zonasi Kota
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: 🚀 Mulai Cepat
      link: /id/guide/build
    - theme: alt
      text: 📚 Ikhtisar Fitur
      link: /id/features/overview
    - theme: alt
      text: 🔍 GitHub
      link: https://github.com/Maicarons/jrpm

features:
  - icon: 🚂
    title: Kopling Lokomotif (decouple)
    details: " Sistem lengkap untuk memisahkan/menggandeng kereta: perintah decouple, transfer tanda, batas panjang kopling dan kecepatan, dukungan dua lokomotif, kopling NewGRF, penjadwalan independen setelah decouple."
  - icon: 🛫
    title: Bandara Modular (multitile-airport)
    details: " Restrukturisasi sistem bandara multi-ubin: sistem tipe air, penjadwalan udara PBS, navigasi udara YAPF, tata letak bandara yang dapat dimodifikasi (allow_modify_airports)."
  - icon: ⬇️
    title: Unduhan Konten Paralel
    details: Banyak cermin (dapat dikustomisasi dengan koma) + unduhan file paralel + pengalihan cermin otomatis saat gagal + konkurensi yang dapat dikonfigurasi.
  - icon: 🤖
    title: AI Sadar Permainan Penuh
    details: Mempertahankan NoAI, menambahkan API global AIGlobal (data keuangan/kendaraan/peringkat/peta semua perusahaan), dengan kontrol akses pengaturan game dan AI contoh.
  - icon: 💰
    title: Tooltip Harga Bangunan
    details: Saat membangun rel/jalan/bentuk tanah, tampilkan perkiraan biaya konstruksi real-time di atas kursor, hindari proyek besar yang tidak disengaja.
  - icon: 📦
    title: Pengelompokan Kendaraan Otomatis
    details: Buat kelompok otomatis berdasarkan pesanan/jadwal bersama, nama grup diambil dari nama rute; dukung perintah konsol dan tombol jendela.
  - icon: 🧱
    details: Saat menempatkan/menyeret alat pembangun untuk stasiun, rel, garasi, bandara, industri, tampilkan pratinjau objek real-time (bukan hanya persegi panjang sederhana).
    title: Sorotan Bangunan Objek
  - icon: 📐
    title: Sistem Cetak Biru
    details: Pilih area dengan blueprint_copy, lalu blueprint_build di tempat lain untuk membangun kembali rel, garasi, terowongan, jembatan, stasiun, dan sinyal (16 slot memori + rotasi).
  - icon: 🏘️
    title: Zonasi Kota + growth_tiles Arsip
    details: Zona kota Tz0–Tz4 dan data pembangunan/penghancuran rumah bulan ini/bulan lalu disimpan di blok arsip GRWT baru (arsip lama kompatibel secara otomatis).
  - icon: ⏺️
    title: Perekaman & Pemutaran Ulang Perintah
    details: cmdrecord start/stop merekam semua perintah yang dieksekusi; cmdreplay memutar ulang, menggunakan serialisasi perintah jrpm sendiri, tanpa lapisan objek perintah tambahan.
---

<div class="vp-doc cta-block">

### 💎 Pengenalan Singkat tentang jrpm

OpenTTD-JRPM (jrpm) adalah fork pilihan berdasarkan **JGR's Patchpack**, menggabungkan fitur pilihan dari **pulsexlb px-patch** (kopling lokomotif,
bandara modular multi-ubin), **OpenTTD-modded** dan **cmclient**, serta menambahkan kemampuan buatan jrpm sendiri seperti AI sadar permainan penuh,
unduhan paralel, dan pengelompokan kendaraan otomatis. Untuk perbandingan versi lengkap dan filosofi desain, lihat
[Riwayat Versi](/id/guide/version-history) dan [Pengenalan Proyek](/id/guide/intro).

</div>