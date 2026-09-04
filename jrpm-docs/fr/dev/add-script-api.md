---
title: Ajouter une API de script
---

# Ajouter une API de script

En utilisant `ScriptGlobal` (API AI consciente de la partie) implemente dans ce projet comme modele, voici les etapes completes pour ajouter une classe d'API de script (visible par AI et GameScript) dans jrpm.

## Contexte

- Les classes d'API de script se trouvent dans `src/script/api/` : `script_<nom>.hpp` (declaration) + `script_<nom>.cpp` (implementation) ;
- Les `.hpp` sont **decouverts automatiquement** par `file(GLOB script_*.hpp)` en phase de construction et generent les liaisons Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **aucun enregistrement manuel de classe necessaire** ;
- Les `.cpp` doivent etre ajoutes a la liste source de `src/script/api/CMakeLists.txt`.

## 1. Fichier d'en-tete `script_global.hpp`

```cpp
/** @file script_global.hpp Documentation. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Documentation de classe, doit etre marquee @api.
 * @api ai game        # Expose a AI et GS ; "game" seulement GS ; "-ai" exclut AI
 */
class ScriptGlobal : public ScriptObject {
public:
	/** Les enums sont exportees comme constantes de classe (ex. AIGlobal.VT_TRAIN). */
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

## 2. Implementation `script_global.cpp`

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

Points cles :
- Les methodes sont toutes `static`, prefixe `/* static */` pour la definition ;
- Types de retour compatibles Squirrel : `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*`, etc. ;
- Controle d'acces : verifier `ScriptCompanyMode::IsDeity() || interrupteur de parametre` dans la methode, retourner une valeur d'erreur si non satisfait.

## 3. Classe de liste (optionnel)

Pour retourner une liste d'entreprises/objets, definir une sous-classe de `ScriptList` :

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

## 4. Enregistrer dans la construction

Ajouter dans la liste source de `src/script/api/CMakeLists.txt` :

```cmake
script_global.cpp
```

::: tip
Les `.hpp` n'ont pas besoin d'etre enregistres (decouverte automatique par GLOB) ; mais **reconfigurer CMake** est necessaire pour voir les nouvelles liaisons generees par le `.hpp`.
:::

## 5. Utilisation dans un script

```js
// Cote AI : prefixe de classe AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// Cote GS : prefixe GS
GSGlobal.GetCompanyCount();
```

## Liste de verification

- [ ] `script_<nom>.hpp` (annotation `@api`)
- [ ] `script_<nom>.cpp` (implementation `/* static */`)
- [ ] Ajouter le `.cpp` dans `CMakeLists.txt`
- [ ] Reconfigurer + construire
- [ ] (Optionnel) Script d'exemple dans `bin/ai/`