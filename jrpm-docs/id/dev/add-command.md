---
title: Menambah Perintah Game
---

# Menambah Perintah Game

Dengan `Commands::AutoGroupSharedOrders` (pengelompokan kendaraan otomatis) yang diimplementasikan dalam proyek ini sebagai templat, memperkenalkan langkah lengkap untuk menambah perintah baru di jrpm.

## 1. Deklarasi di Enum Perintah

`src/command_type.h`, sisipkan di tengah `enum class Commands` (akan menggeser ID berikutnya):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. Deklarasi Handler Perintah dan Registrasi

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT` akan sekaligus mendeklarasikan fungsi handler dan mendaftarkan `CommandTraits`):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: Tanpa parameter tile (dengan tile gunakan `DEF_CMD_TUPLE`);
- `CmdDataT<...>`: Daftar tipe parameter, handler membongkar sesuai urutan.

## 3. Implementasi Handler

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
    if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

    CommandCost total_cost;
    for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
        // ... logika bisnis ...
        CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
        if (ret.Failed()) return ret;
        total_cost.AddCost(ret.GetCost());
    }
    return total_cost;
}
```

Poin penting:
- Mengembalikan `CMD_ERROR` menunjukkan kegagalan (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)` mengontrol "eksekusi nyata" dan "mode uji";
- Perintah bersarang menggunakan `Command<Commands::X>::Do(flags, ...)`;
- Panggilan GUI/jaringan menggunakan `Command<Commands::X>::Post(err_string, args...)` (otomatis melalui sinkronisasi jaringan).

## 4. Tambah Titik Masuk

### Tombol Jendela (Opsional)

1. `src/widgets/<system>_widget.h` enum tambah `WID_XXX`;
2. Tata letak NWidget jendela tambah tombol;
3. `OnClick` tambah `case`:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### Perintah Konsol (Opsional)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// Dalam IConsoleStdLibRegister():
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### String (Opsional)

`src/lang/english.txt` tambah (mis. `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. Daftar Periksa

- [ ] `command_type.h` enum
- [ ] `<system>_cmd.h` `DEF_CMD_TUPLE*`
- [ ] `<system>_cmd.cpp` handler
- [ ] (Opsional) Kontrol/konsol/string
- [ ] Jaringan dan arsip beradaptasi otomatis (`CommandTraits` dihasilkan otomatis)