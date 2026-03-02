# 🚀 Mithril API v2 (C23) — Guía Completa en Español

> API criptográfica modular y escalable para macOS Sequoia, diseñada para crecer por fases con arquitectura por *providers* (`sodium`, `c23`, `flint`).

---

## 🧭 Tabla de Contenidos

1. [¿Qué es Mithril API v2?](#-qué-es-mithril-api-v2)
2. [Estado actual del proyecto](#-estado-actual-del-proyecto)
3. [Arquitectura (resumen visual)](#-arquitectura-resumen-visual)
4. [Requisitos](#-requisitos)
5. [Tutorial 1: Build + tests con CMake (recomendado)](#-tutorial-1-build--tests-con-cmake-recomendado)
6. [Tutorial 2: Script rápido con libsodium](#-tutorial-2-script-rápido-con-libsodium)
7. [Tutorial 3: Integración con Xcode (Sequoia)](#-tutorial-3-integración-con-xcode-sequoia)
8. [Cómo funciona el sistema de providers](#-cómo-funciona-el-sistema-de-providers)
9. [API pública disponible hoy](#-api-pública-disponible-hoy)
10. [Tests actuales](#-tests-actuales)
11. [Troubleshooting (errores comunes)](#-troubleshooting-errores-comunes)
12. [Roadmap sugerido](#-roadmap-sugerido)
13. [Buenas prácticas de seguridad](#-buenas-prácticas-de-seguridad)

---

## 🔐 ¿Qué es Mithril API v2?

`Mithril_API_V2` es la evolución de la capa criptográfica hacia un diseño moderno en **C23**, con estas metas:

- ✅ API estable y predecible para clientes C/C++
- ✅ Arquitectura desacoplada por *providers*
- ✅ Escalabilidad a nuevos algoritmos y librerías
- ✅ Build reproducible en macOS Sequoia
- ✅ Base sólida para CI y endurecimiento criptográfico

---

## 📌 Estado actual del proyecto

Estado implementado (fases 1 → 10):

- ✅ Núcleo API/context/config/capabilities
- ✅ Dispatch por provider activo
- ✅ Utilities de memoria y operaciones constant-time
- ✅ Provider `sodium` para RNG/HASH/AEAD/KEX/SIGN (modo estricto)
- ✅ Provider `c23` para `bigint/modarith` (longitud arbitraria)
- ✅ Provider `flint` integrado como esqueleto (pendiente implementación real)
- ✅ Tests unitarios para `c23` bigint/modarith
- ✅ CMake + ctest funcionando
- ✅ Scripts de build en macOS

---

## 🧱 Arquitectura (resumen visual)

```text
include/mithril/*.h       -> API pública (contratos)
src/api/*.c               -> capa de entrada API
src/core/*.c              -> registro/dispatch/validación
src/providers/provider_*  -> implementación por backend
src/platform/apple/*.c    -> utilidades de plataforma (Sequoia)
tests/unit/*.c            -> pruebas unitarias
```

Flujo típico:

1. `mithril_init()`
2. registro de providers
3. activación de provider
4. llamada API (ej. hash/sign/modarith)
5. dispatch al provider activo

---

## 🛠 Requisitos

### macOS

- macOS Sequoia (o compatible reciente)
- `clang` / `cc`
- `cmake >= 3.20`
- `pkg-config`
- `libsodium` instalado

### Instalación rápida de dependencias (Homebrew)

```zsh
brew install cmake pkg-config libsodium
```

Verificación:

```zsh
pkg-config --modversion libsodium
cmake --version
cc --version
```

---

## 🧪 Tutorial 1: Build + tests con CMake (recomendado)

> Este flujo es el oficial para desarrollo continuo.

### Paso 1: ir a la carpeta del proyecto

```zsh
cd /Users/kirtantegsingh/documents/mithril/Mithril_API_V2
```

### Paso 2: configurar

```zsh
cmake -S . -B build-cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=OFF
```

### Paso 3: compilar

```zsh
cmake --build build-cmake --parallel
```

### Paso 4: ejecutar tests

```zsh
ctest --test-dir build-cmake --output-on-failure
```

Salida esperada:

- `c23_bigint` ✅
- `c23_modarith` ✅

---

## ⚡ Tutorial 2: Script rápido con libsodium

Si quieres una ejecución rápida todo-en-uno:

```zsh
/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/scripts/build_and_run_phase82_with_sodium.zsh
```

Este script:

1. compila binarios de test
2. habilita `MITHRIL_USE_LIBSODIUM`
3. enlaza `Security` framework en Apple
4. ejecuta tests

---

## 🍎 Tutorial 3: Integración con Xcode (Sequoia)

Configuraciones disponibles en:

- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Base.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Debug.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Release.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Warnings.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Sanitizers.xcconfig`

### Paso a paso

1. Crea/abre tu `.xcodeproj` o `.xcworkspace`.
2. Aplica `Base.xcconfig` al target de librería.
3. En Debug/Release, referencia sus xcconfig respectivos.
4. Verifica que `HEADER_SEARCH_PATHS` incluya `Mithril_API_V2/include`.
5. Verifica `OTHER_LDFLAGS` con `-lsodium`.
6. En macOS, enlaza `Security.framework`.

---

## 🔌 Cómo funciona el sistema de providers

Provider por defecto en arranque:

- `sodium` (estricto)

Providers adicionales:

- `c23` (bigint/modarith puro C23)
- `flint` (registro opcional vía macro, implementación pendiente)

### Activar provider manualmente

```c
mithril_provider_activate(ctx, "c23");
```

### Consultar provider activo

```c
const char *name = mithril_provider_active_name(ctx);
```

### ¿Por qué es importante?

- permite separar API de implementación
- simplifica migraciones de backend
- facilita testing cruzado entre providers

---

## 📚 API pública disponible hoy

Headers principales:

- `mithril_api.h` (init/shutdown/context)
- `mithril_rng.h`
- `mithril_hash.h`
- `mithril_aead.h`
- `mithril_kex.h`
- `mithril_sign.h`
- `mithril_bigint.h`
- `mithril_modarith.h`
- `mithril_provider.h`
- `mithril_memory.h`
- `mithril_util.h`
- `mithril_platform.h`

---

## ✅ Tests actuales

Tests implementados:

- `tests/unit/test_c23_bigint.c`
- `tests/unit/test_c23_modarith.c`

Cobertura funcional actual:

- suma, resta, multiplicación bigint (c23)
- suma/multiplicación modular con reducción (c23)
- casos límite de módulo inválido
- casos de longitudes no triviales

---

## 🧯 Troubleshooting (errores comunes)

### 1) `MITHRIL_ERR_NOT_IMPLEMENTED` en `mithril_init`

Causa típica:

- no se compiló con `MITHRIL_USE_LIBSODIUM`
- o falta link de libsodium

Solución:

- activa `-DMITHRIL_USE_LIBSODIUM`
- verifica `pkg-config --libs libsodium`

### 2) Error de link con `SecRandomCopyBytes`

Causa:

- falta `Security.framework`

Solución:

- añade `-framework Security` al link en macOS

### 3) Tests fallan por provider activo

Causa:

- estás usando provider distinto al esperado por el test

Solución:

- fuerza `mithril_provider_activate(ctx, "c23")` en pruebas de bigint/modarith

### 4) `ld: library 'sodium' not found`

Causa:

- ruta de libsodium no visible al linker

Solución:

- usa `pkg-config` en CMake (`PkgConfig::LIBSODIUM`)
- revisa instalación Homebrew

---

## 🗺 Roadmap sugerido

1. Implementar provider `flint` real para bigint/modarith de alto rendimiento.
2. Agregar tests de conformidad cruzada `c23` vs `flint`.
3. Incorporar fuzz tests para API boundaries.
4. Añadir benchmarks reproducibles.
5. Definir políticas de versionado ABI/API (`v2.x`).
6. Publicar artifacts para integración externa.

---

## 🛡 Buenas prácticas de seguridad

- 🔒 No uses fallback criptográfico no validado en producción.
- 🔒 Mantén `libsodium` actualizado.
- 🔒 Evita logging de secretos/keys.
- 🔒 Usa limpieza segura de memoria para buffers sensibles.
- 🔒 Activa sanitizers en Debug cuando sea posible.
- 🔒 Separa claramente entorno dev vs prod.

---

## 🤝 Contribución rápida

Flujo recomendado:

1. Crea rama `codex/<feature>`.
2. Implementa cambios pequeños y coherentes.
3. Ejecuta `ci/run_phase10_ci.zsh`.
4. Adjunta evidencia de tests al PR.
5. Revisión enfocada en seguridad + regresión.

---

## 🧩 Ejemplo ultra-rápido de uso

```c
#include <stdint.h>
#include "mithril/mithril_api.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_modarith.h"

int main(void) {
    mithril_context *ctx = 0;
    if (mithril_init(&ctx, 0) != MITHRIL_OK) return 1;

    if (mithril_provider_activate(ctx, "c23") != MITHRIL_OK) return 2;

    uint8_t a[] = {0x12};
    uint8_t b[] = {0x0E};
    uint8_t m[] = {0x17};
    uint8_t out[8] = {0};
    size_t written = 0;

    if (mithril_modarith_mul_mod(ctx, a, 1, b, 1, m, 1, out, sizeof(out), &written) != MITHRIL_OK) return 3;

    mithril_shutdown(ctx);
    return 0;
}
```

---

## 🎯 Cierre

`Mithril_API_V2` ya tiene una base sólida para evolucionar hacia una API criptográfica de producción: modular, testeable y extensible.

Si vas a continuar inmediatamente, el mejor siguiente paso técnico es: **conformance tests + implementación real de `provider_flint`**. 🔥
