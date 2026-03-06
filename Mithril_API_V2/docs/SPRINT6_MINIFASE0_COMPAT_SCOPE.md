# Sprint 6 - Minifase 0

## Objetivo

Definir el alcance exacto de compatibilidad `v1 -> v2` antes de codificar wrappers, evitando migraciones ambiguas y regresiones semanticas.

## Diagnostico del repo actual

- La API `v2` estable expone primitivas por contexto `mithril_context` y buffers `uint8_t*` en big-endian:
  - `mithril_bigint_add/sub/mul`
  - `mithril_modarith_add_mod/mul_mod`
  - `mithril_rng/hash/aead/kex/sign` (no aplican al adapter v1 numerico inicial)
- La superficie `v1` utilizable esta en:
  - `/Users/kirtantegsingh/documents/mithril/Mithril_Criptography_API/Mithril_Framework/*.h`
- La superficie `v1` mezcla:
  - APIs libsodium con arreglos de 32 bytes y convencion little-endian.
  - APIs FLINT (`fmpz_t`) con semantica de enteros arbitrarios.
  - utilidades de code generation (fuera del runtime criptografico).

## Inventario v1 y clasificacion de migracion

### Grupo A: Wrapper prioritario (Sprint 6 commit 1)

1. `mithril_add_scalar`
2. `mithril_add_mod_l`
3. `mithril_add_constant_time`
4. `sodium_sub_with_underflow`
5. `fmpz_mul_safe`
6. `fmpz_square_safe`

Razon: representan operaciones criticas de compatibilidad matematica usadas por clientes v1.

### Grupo B: Wrapper secundario (Sprint 6+)

1. `fmpz_inc`
2. `fmpz_inc_inplace`
3. `fmpz_inc_mod`
4. `fmpz_dec`
5. `fmpz_sub_ushort`
6. `fmpz_mul_ui_mod`

Razon: utiles, pero no bloquean el primer smoke de compatibilidad.

### Grupo C: Diferido por brecha de API v2

1. `div_fmpz`
2. `rem_mod_pow_of_2`
3. `flint_kmul`
4. `flint_kmul_limbs`

Razon: requieren division/limb-level no expuesto en API publica v2 actual.

### Grupo D: No runtime criptografico (fuera de Sprint 6)

1. `mithril_get_all_operations`
2. `mithril_get_operations_count`
3. `mithril_generate_*`

Razon: tooling de plantillas/codigo, no parte del adapter criptografico en ejecucion.

## Matriz de mapeo v1 -> v2

| Funcion v1 | Estado | Mapping propuesto | Notas de compatibilidad |
| --- | --- | --- | --- |
| `mithril_add_scalar` | Directo con conversion | `mithril_bigint_add` | v1 usa 32 bytes LE; v2 opera BE variable. Wrapper debe convertir LE<->BE y forzar longitud 32. |
| `mithril_add_mod_l` | Compuesto | `mithril_modarith_add_mod` | Requiere constante `L` (orden Ed25519) en BE y conversion LE<->BE. |
| `mithril_add_constant_time` | Local + opcional v2 | algoritmo local CT | Mantener semantica CT byte a byte y retorno carry; no delegar a provider variable-time. |
| `sodium_sub_with_underflow` | Compuesto | `mithril_bigint_sub` | Si `a>=b`: `a-b`; si `a<b`: devolver `(b-a)-1` y underflow=1 como v1. |
| `fmpz_mul_safe` | Directo con puente FLINT | `mithril_bigint_mul` | Convertir `fmpz_t` <-> BE; preservar retorno 0 en exito. |
| `fmpz_square_safe` | Compuesto | `mithril_bigint_mul(a,a)` | Igual puente de conversion `fmpz_t` <-> BE. |
| `fmpz_inc*` | Compuesto | `add +1` y/o `add_mod` | Planificado en etapa secundaria. |
| `fmpz_dec` | Compuesto | `sub 1` + politica underflow | Requiere definir semantica negativa/underflow estable. |
| `div_fmpz` | Diferido | N/A | v2 no expone division entera. |
| `rem_mod_pow_of_2` | Diferido | N/A | v2 no expone bit-ops mod 2^k. |

## Decisiones tecnicas para implementacion

### 1) Ubicacion del adapter

- Header publico compat:
  - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/include/mithril/compat/mithril_v1_compat.h`
- Fuente:
  - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/src/compat/mithril_v1_compat.c`
- Helpers internos:
  - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/src/compat/mithril_v1_compat_internal.h`

### 2) Gestion de contexto

- Adapter v1 tendra contexto singleton interno (`mithril_init` lazy, `mithril_shutdown` explicito opcional).
- API de control sugerida:
  - `int mithril_v1_compat_init(void);`
  - `void mithril_v1_compat_shutdown(void);`

### 3) Normalizacion de endianess

- Estandar interno del adapter:
  - Entradas v1 libsodium: LE fijo (32 bytes).
  - Entradas v1 FLINT: convertir `fmpz_t` a BE canonical.
  - API v2: BE canonical.
- Implementar helpers dedicados:
  - `le_to_be_fixed32`
  - `be_to_le_fixed32`
  - `fmpz_to_be`
  - `be_to_fmpz`

### 4) Politica de errores

- Capa compat retorna codigos legacy cuando aplique.
- Traduccion sugerida:
  - `MITHRIL_OK -> 0`
  - `MITHRIL_ERR_INVALID_ARGUMENT -> -1`
  - `MITHRIL_ERR_NO_PROVIDER -> -1`
  - `MITHRIL_ERR_INTERNAL -> -1`
- Casos de verificacion (AEAD/SIGN) no aplican en minifase numerica.

### 5) Guards de compilacion

- `MITHRIL_COMPAT_V1_ENABLE` para compilar adapter.
- Wrappers `fmpz_*` condicionados por `MITHRIL_USE_FLINT`.
- Si `MITHRIL_USE_FLINT=OFF`, exponer fallback controlado (retorno `-1`) y tests que lo verifiquen.

## Riesgos y mitigaciones

1. Riesgo: discrepancias LE/BE silenciosas.
   - Mitigacion: tests de vectores fijos con golden inputs v1.
2. Riesgo: semantica de underflow distinta en resta legacy.
   - Mitigacion: pruebas de contrato explicitas para `a<b`, `a=b`, `a>b`.
3. Riesgo: wrappers CT delegando a rutas no CT.
   - Mitigacion: `mithril_add_constant_time` implementado localmente en compat.
4. Riesgo: acoplamiento fuerte a FLINT.
   - Mitigacion: separar wrappers libsodium-style y wrappers `fmpz_*` por macros.

## Entregables de cierre minifase 0

1. Inventario y alcance de compatibilidad documentado (este archivo).
2. Matriz de mapeo aprobada para Sprint 6 commit 1.
3. Criterio claro de funciones diferidas vs prioritarias.

## Criterio de entrada para minifase 1 (implementacion)

1. Crear `include/src` de compat.
2. Implementar wrappers Grupo A.
3. Agregar smoke tests de compatibilidad legacy basica.
