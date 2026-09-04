---
title: Guia de Desenvolvimento
---

# Guia de Desenvolvimento

Este documento explica como continuar desenvolvendo no jrpm. Todos os padroes sao baseados em codigo realmente implementado neste projeto (consulte os commits em `git log`).

## Visao Rapida da Organizacao do Codigo

| Diretorio | Responsabilidade |
|---|---|
| `src/` | Todo o codigo fonte C++ (nivel superior nomeado por sistema: `rail_cmd.cpp`, `group_gui.cpp`...) |
| `src/network/` | Rede (servidor/cliente/UDP/HTTP/download de conteudo) |
| `src/script/` | Framework de script (Squirrel); `api/` contem classes de API AI/GS |
| `src/table/settings/*.ini` | **Fontes de definicao de configuracoes** (settingsgen gera codigo) |
| `src/lang/english.txt` | Definicao de strings (strgen gera) |
| `src/sl/saveload_common.h` | Enumeracao de versao de arquivo (SLV) |
| `bin/ai/` | Scripts AI (`GlobalAI` e o exemplo) |

## Padroes Comuns de Desenvolvimento

- [Adicionar comando do jogo](./add-command): Enumeracao de comando + `DEF_CMD_TUPLE_NT` + manipulador + entrada GUI/console
- [Adicionar API de script](./add-script-api): `script_*.hpp/.cpp` (vinculacao Squirrel registrada automaticamente)
- [Adicionar item de configuracao](./add-setting): `.ini` + campo `settings_type.h` + string

## Construcao

```bash
cmake -B build ..
cmake --build build -j
```

## Notas de Desenvolvimento

1. **Codificacao**: Alteracoes em codigo fonte/documentos contendo chines devem ser UTF-8 (as ferramentas Write/Edit desta maquina ja produziram codificacao GBK, pode ser corrigido com `jgrpp-features/_fix_utf8.py`);
2. **Mensagem de commit**: Recomendado em ingles (evita problemas de codificacao no terminal);
3. **Insercao no meio da enumeracao de comandos** desloca IDs de comandos subsequentes -- binarios de versoes antigas e novas serao inconsistentes em rede, fenomeno normal dentro do fork, atualizacao deve ser sincronizada;
4. **Versao de arquivo**: Ao alterar estrutura de arquivo, adicionar nova entrada `SLV_*` em `src/sl/saveload_common.h` e atualizar `SAVEGAME_VERSION`;
5. **Novos arquivos**: `.cpp` deve ser adicionado a lista de fontes do `CMakeLists.txt` correspondente (`src/CMakeLists.txt` ou subdiretorio/`script/api/CMakeLists.txt`); `script_*.hpp` e descoberto automaticamente por `file(GLOB)`, sem necessidade de registro;
6. **Novas strings**: Adicionar em `src/lang/english.txt` (outros idiomas usam ingles como fallback);
7. **Compatibilidade NewGRF/arquivo**: Alterar `_openttd_content_version` / numero de versao NewGRF requer cuidado (afeta compatibilidade online e de conteudo).

## Sugestoes de Regression (Importante)

O branch atual **nao foi compilado em maquina real**. Apos a primeira construcao, recomenda-se testar nesta ordem:
1. Basico: `openttd-jrpm -v` exibe `jrpm-0.1.0`;
2. Arquivo: Abrir jogo local e jogar 1-2 anos;
3. Online: Clientes jrpm se conectam entre si; clientes jgrpp/pxp entram;
4. Novos recursos: Desacoplamento de locomotivas (desengate/engate/reversao), aeroportos modulares (modificar layout), download paralelo, agrupamento automatico, dica de construcao, GlobalAI;
5. Regression de recursos antigos: Sinais, tracerestrict, scheduled dispatch, substituicao de modelos.