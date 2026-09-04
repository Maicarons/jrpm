---
title: Perekaman & Pemutaran Ulang Perintah
---

# Perekaman & Pemutaran Ulang Perintah (Command Record / Replay)

Porting pemutaran ulang perintah cmclient (batch kelima, komit `f113acce28`). **Tidak mem-porting lapisan objek perintah cmclient**——langsung menggunakan infrastruktur serialisasi perintah bawaan jrpm (`DynBaseCommandContainer`), melewati seluruh "lapisan objek perintah".

## Perintah Konsol

```
cmdrecord [start [file]]    # Mulai merekam (file default cmdrecord.jrcm, disimpan di direktori pribadi)
cmdrecord stop              # Berhenti (tunda 10 tick hingga antrian perintah kosong lalu simpan)
cmdreplay <file>            # Putar ulang: deserialisasi dan segera jalankan setiap perintah
```

## Poin Implementasi

- **Kait perekaman**: `CommandRecordLog` dipasang setelah eksekusi perintah `DoCommandPInternal`——ini adalah **satu-satunya titik eksekusi nyata** untuk perintah lokal, jaringan, dan pemutaran ulang, tidak akan direkam dua kali.
- **Serialisasi**: `cmd / tile / error_msg / payload / company` menggunakan `DynBaseCommandContainer::Serialise` untuk serialisasi tanpa kehilangan; format file `JRCM` magic + version + count + entri.
- **Berhenti tertunda**: `cmdrecord stop` menandai tunda 10 tick, `StateGameLoop` memeriksa setiap frame (`CommandRecordTick`) memastikan perintah yang sudah diantrekan juga ditangkap sebelum flush.
- **Eksekusi pemutaran ulang**: Setiap perintah segera dieksekusi dengan jalur perintah server (`DCIF_NETWORK_COMMAND`), file pemutaran ulang bahkan jika header count belum diperbarui tetap dapat membaca data sesuai data aktual.

## Verifikasi

Server khusus E2E: Rekam `pause` → file rekaman 26 byte → peta baru putar ulang → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## Penggunaan Khas

```
cmdrecord start build1     # Mulai merekam
# ... dalam game, bangun rel, stasiun ...
cmdrecord stop             # Berhenti dan simpan
# Ganti peta atau setelah pemulihan crash:
cmdreplay build1           # Bangun ulang semua operasi dengan satu tombol
```