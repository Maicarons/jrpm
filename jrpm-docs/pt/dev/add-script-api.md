---
title: Adicionar API de Script
---

# Adicionar API de Script

Usando o `ScriptGlobal` (API de IA de percepcao global) implementado neste projeto como modelo, apresenta as etapas completas para adicionar uma nova classe de API de script (visivel para AI e GameScript) no jrpm.

## Contexto

- As classes de API de script estao em `src/script/api/`: `script_<nome>.hpp` (declaracao) + `script_<nome>.cpp` (implementacao);
- O `.hpp` e **descoberto automaticamente** por `file(GLOB script_*.hpp)` em tempo de construcao e gera vinculacoes Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **sem necessidade de registro manual da classe**;
- O `.cpp` precisa ser adicionado a lista de fontes de `src/script/api/CMakeLists.txt`.

## 1. Arquivo de Cabecalho `script_global.hpp`

```cpp
/** @file script_global.hpp Documentacao. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Documentacao da classe, deve conter @api.
 * @api ai game        # Exposto para AI e GS; "game" apenas GS; "-ai" exclui AI
 */
class ScriptGlobal : public ScriptObject {
public:
    /** Enumeracoes sao exportadas como constantes de classe (ex: AIGlobal.VT_TRAIN). */
    enum VehicleType {
        VT_TRAIN = ::VehicleType::Train,
        VT_AIRCRAFT = ::VehicleType::Aircraft,
    };

    /** @api ai game */
    static bool IsGlobalAccessAllowed();

    /** @api ai game */
    static SQInteger GetCompanyCount();

    /** @api ai game */
    static std::optional<std::string> GetCompanyName(ScriptCompany::CompanyID company);
};

#endif /* SCRIPT_GLOBAL_HPP */
```

## 2. Implementacao `script_global.cpp`

```cpp
#include "../../stdafx.h"
#include "script_global.hpp"
#include "../../company_base.h"
// ...

/* static */ SQInteger ScriptGlobal::GetCompanyCount()
{
    if (!IsGlobalAccessAllowed()) return 0;
    SQInteger count = 0;
    for (const Company *c : Company::Iterate()) count++;
    return count;
}
```

Pontos importantes:
- Metodos sao `static`, prefixo `/* static */` na definicao;
- Tipos de retorno devem ser amigaveis ao Squirrel: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*`, etc.;
- Controle de acesso: verificar `ScriptCompanyMode::IsDeity() || alternador de configuracao` dentro do metodo, retornar valor de erro se nao atender.

## 3. Classe de Lista (opcional)

Ao precisar retornar lista de empresas/itens, definir subclasse de `ScriptList`:

```cpp
class ScriptGlobalCompanyList : public ScriptList {
public:
#ifdef DOXYGEN_API
    ScriptGlobalCompanyList();
#else
    ScriptGlobalCompanyList(HSQUIRRELVM vm);
#endif
};
```

```cpp
ScriptGlobalCompanyList::ScriptGlobalCompanyList(HSQUIRRELVM vm)
{
    ScriptList::FillList<Company>(vm, this);
}
```

## 4. Registrar na Construcao

Adicionar a lista de fontes de `src/script/api/CMakeLists.txt`:

```cmake
script_global.cpp
```

::: tip
O `.hpp` nao precisa de registro (GLOB descobre automaticamente); mas e necessario **reconfigurar o CMake** para que o novo `.hpp` gere as vinculacoes.
:::

## 5. Usar no Script

```js
// Lado AI: prefixo da classe AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// Lado GS: prefixo GS
GSGlobal.GetCompanyCount();
```

## Lista de Verificacao

- [ ] `script_<nome>.hpp` (com anotacao `@api`)
- [ ] `script_<nome>.cpp` (implementacao com `/* static */`)
- [ ] `CMakeLists.txt` adicionar `.cpp`
- [ ] Reconfigurar + construir
- [ ] (Opcional) Script de exemplo em `bin/ai/`