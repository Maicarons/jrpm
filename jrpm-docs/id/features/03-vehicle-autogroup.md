---
title: Pengelompokan Kendaraan Otomatis
---

## Status Saat Ini (Kesimpulan Riset)

- Struktur data grup: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number)、`GroupID`、`DEFAULT_GROUP`；
- Perintah yang ada (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (ganti nama/atur grup induk)
  - `CmdAddVehicleGroup` (satu kendaraan masuk grup)
  - `CmdAddSharedVehicleGroup` (memasukkan kendaraan **berbagi pesanan dari satu kendaraan** ke dalam grup **yang sudah ada**)
  - `CmdCreateGroupFromList` (buat grup dari daftar kendaraan, dukung tipe daftar `VL_SHARED_ORDERS`; penamaan otomatis)
- Asisten penamaan otomatis: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, berdasarkan kota asal/tujuan rute `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`)——jgrpp sudah memiliki infrastruktur "penamaan berdasarkan rute";
- Mekanisme registrasi perintah: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (makro sekaligus mendeklarasikan handler dan mendaftarkan `CommandTraits`);
- Kendaraan↔grup: `vehicle_base.h` `Vehicle::group_id`; rantai pesanan bersama `FirstShared()/NextShared()`; `OrderList *orders`.

**Kesimpulan**: Kurang perintah keseluruhan untuk "menelusuri semua kendaraan perusahaan, secara otomatis membuat grup dan mengelompokkan berdasarkan pesanan bersama".

## Implementasi Fitur Ini

### Perintah Baru `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

Alur (`src/group_cmd.cpp`):
1. Menelusuri semua kendaraan utama dari tipe yang ditentukan perusahaan (`Vehicle::IterateTypeFrontOnly(type)`), lewati yang bukan milik perusahaan sendiri/tidak memiliki pesanan/sudah dalam grup kustom;
2. Hitung setiap rantai pesanan bersama; jika ≥2 kendaraan berbagi daftar pesanan yang sama:
   - Gunakan `GenerateAutoNameForVehicleGroup(v)` untuk menghasilkan nama grup (mis. "Kota A ↔ Kota B");
   - Bangun `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`, panggil bersarang `Command<Commands::CreateGroupFromList>::Do(flags, ...)` untuk membuat grup dan memindahkan semua kendaraan bersama ke dalamnya;
   - Setelah selesai, `group_id` kendaraan dalam grup sudah bukan grup default, loop secara otomatis melewati (hanya satu grup yang dibuat per daftar pesanan);
3. `GroupChangeDeferredUpdateScope` secara seragam menunda pembaruan statistik grup.

### Tiga Titik Masuk

| Titik Masuk | Lokasi | Deskripsi |
|---|---|---|
| Tombol jendela grup | `src/group_gui.cpp` baru `WID_GL_AUTOGROUP_SHARED` (di toolbar di sebelah LIVERY), OnClick kirim perintah | Sprite复用 `SPR_GROUP_CREATE_TRAIN + vtype` |
| Perintah konsol | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (tanpa parameter = jalankan keempat tipe) | `IConsole::CmdRegister` |
| Script/lainnya | Kode apa pun dapat `Command<Commands::AutoGroupSharedOrders>::Post(...)` | Perintah berjalan melalui sinkronisasi jaringan, aman untuk server multiplayer |

### File Terkait

- `src/command_type.h`: Enum `Commands` baru `AutoGroupSharedOrders` (**Catatan: Penyisipan di tengah enum akan menggeser ID perintah berikutnya, versi lama dan baru tidak akan konsisten saat online, ini normal dalam fork**)
- `src/group_cmd.h` / `group_cmd.cpp`: Deklarasi dan implementasi perintah
- `src/widgets/group_widget.h`: ID kontrol baru
- `src/group_gui.cpp`: Tombol toolbar (NWidget + sprite OnPaint + OnClick)
- `src/console_cmds.cpp`: Perintah konsol
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## Poin Verifikasi

1. Dua atau lebih kendaraan berbagi daftar pesanan yang sama → klik tombol/`autogroup` menghasilkan grup "nama rute" dan semua kendaraan masuk grup;
2. Kendaraan yang sudah dikelompokkan tidak dipindahkan lagi; daftar pesanan berbeda masing-masing membentuk grup sendiri;
3. Statistik kendaraan dalam grup (jumlah/laba) diperbarui dengan benar (bergantung pada `GroupChangeDeferredUpdateScope`).