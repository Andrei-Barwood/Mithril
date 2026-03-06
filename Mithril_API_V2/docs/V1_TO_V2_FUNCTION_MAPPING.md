# V1 to V2 Function Mapping

## Objetivo

Documentar la migracion de clientes legacy `v1` hacia `Mithril_API_V2`, con foco en:

1. compatibilidad inmediata por adapter (`mithril/compat/mithril_v1_compat.h`)
2. reemplazo nativo progresivo sobre API `v2`
3. funciones legacy pendientes o diferidas

Estado de referencia: Sprint 6 (minifases 0, 1 y 2).

## Ubicacion del adapter

- Header publico:
  - `include/mithril/compat/mithril_v1_compat.h`
- Implementacion:
  - `src/compat/mithril_v1_compat.c`

## Requisitos de build para migracion

### Compatibilidad basica (sin FLINT)

```zsh
cmake -S . -B build-cmake \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=OFF
```

### Compatibilidad con wrappers `fmpz_*` (FLINT)

```zsh
cmake -S . -B build-cmake-flint \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=ON \
  -DMITHRIL_USE_FLINT=ON
```

## Mapeo funcion por funcion

| Funcion legacy v1 | Estado | Reemplazo recomendado | Notas |
| --- | --- | --- | --- |
| `mithril_add_scalar` | Implementada en adapter | `mithril_bigint_add` | v1 usa 32 bytes little-endian + carry; v2 usa big-endian variable. |
| `mithril_add_mod_l` | Implementada en adapter | `mithril_modarith_add_mod` | Usa orden Ed25519 `L` interno en el adapter. |
| `mithril_add_constant_time` | Implementada en adapter | Sin reemplazo directo 1:1 en v2 | Se conserva semantica legacy (suma LE con retorno carry). |
| `sodium_sub_with_underflow` | Implementada en adapter | `mithril_bigint_sub` + politica underflow | Mantiene contrato v1: si `a < b` devuelve `(b-a)-1` y `E_SODIUM_UFL`. |
| `fmpz_mul_safe` | Implementada en adapter (solo FLINT) | `mithril_bigint_mul` | Disponible cuando `MITHRIL_USE_FLINT` esta habilitado. |
| `fmpz_square_safe` | Implementada en adapter (solo FLINT) | `mithril_bigint_mul(a,a)` | Disponible cuando `MITHRIL_USE_FLINT` esta habilitado. |
| `fmpz_inc` | Implementada en adapter (solo FLINT) | `mithril_bigint_add` con `+1` | Mantiene retorno legacy `E_FMPZ_OK` en exito. |
| `fmpz_inc_inplace` | Implementada en adapter (solo FLINT) | Igual que `fmpz_inc` | Conserva semantica in-place. |
| `fmpz_inc_mod` | Implementada en adapter (solo FLINT) | `mithril_modarith_add_mod` con `+1 mod m` | Requiere modulo positivo. |
| `fmpz_dec` | Implementada en adapter (solo FLINT) | `mithril_bigint_sub` con `-1` | Conserva underflow legacy (`a=0 -> -1`, `E_FMPZ_UFL`). |
| `fmpz_sub_ushort` | Implementada en adapter (solo FLINT) | `mithril_bigint_sub` + ajuste de signo | Soporta resultado negativo cuando `a < b`. |
| `fmpz_mul_ui_mod` | Implementada en adapter (solo FLINT) | `mithril_bigint_mul` + `mithril_modarith_mul_mod` | Conserva flag legacy `E_FLINT_OFL` cuando aplica reduccion. |
| `div_fmpz` | Diferida | N/A en v2 actual | v2 no expone division entera publica. |
| `rem_mod_pow_of_2` | Diferida | N/A en v2 actual | v2 no expone primitiva bitwise `mod 2^k`. |
| `flint_kmul` | Diferida | N/A en v2 actual | Operacion de nivel interno/optimizada de FLINT. |
| `flint_kmul_limbs` | Diferida | N/A en v2 actual | Requiere API de limbs no disponible en v2. |
| `mithril_get_all_operations` | Fuera de alcance runtime | N/A | Pertenece a tooling de framework legacy. |
| `mithril_get_operations_count` | Fuera de alcance runtime | N/A | Pertenece a tooling de framework legacy. |
| `mithril_generate_*` | Fuera de alcance runtime | N/A | Generacion de templates/codigo, no API criptografica runtime. |

## Diferencias clave entre v1 y v2

1. Endianness:
   - v1: mayormente little-endian (32 bytes fijos en varias rutas)
   - v2: big-endian canonical y longitud variable
2. Modelo de errores:
   - v1: `int` legacy (`0`, `1`, `-1`, etc.)
   - v2: `mithril_status` tipado
3. Contexto:
   - v1: funciones sueltas
   - v2: `mithril_context` + provider activo
4. Seleccion de provider:
   - Adapter v1: selecciona provider matematico internamente (`flint` o `c23`)
   - v2 nativo: control explicito del cliente con `mithril_provider_activate`

## Ejemplo A: migracion sin romper llamadas legacy

```c
#include "mithril/compat/mithril_v1_compat.h"

int legacy_entry(void) {
    uint8_t a[32] = {0};
    uint8_t b[32] = {0};
    uint8_t out[32] = {0};

    if (mithril_v1_compat_init() != 0) return -1;

    (void)mithril_add_scalar(out, a, b);

    mithril_v1_compat_shutdown();
    return 0;
}
```

## Ejemplo B: migracion nativa a API v2

```c
#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_provider.h"

int native_entry(void) {
    mithril_context *ctx = NULL;
    uint8_t a_be[] = {0x01};
    uint8_t b_be[] = {0x02};
    uint8_t out[8] = {0};
    size_t written = 0u;

    if (mithril_init(&ctx, NULL) != MITHRIL_OK) return -1;
    if (mithril_provider_activate(ctx, "c23") != MITHRIL_OK) return -2;
    if (mithril_bigint_add(ctx, a_be, sizeof(a_be), b_be, sizeof(b_be), out, sizeof(out), &written) != MITHRIL_OK) return -3;

    mithril_shutdown(ctx);
    return 0;
}
```

## Estrategia recomendada de transicion

1. Fase puente:
   - incorporar `mithril_v1_compat.h`
   - compilar y validar sin cambiar callsites
2. Fase de reemplazo:
   - migrar modulo por modulo a `mithril_*` nativo v2
   - centralizar conversiones LE/BE en un unico utilitario interno del cliente
3. Fase de retiro:
   - eliminar includes legacy
   - remover dependencias del adapter
   - dejar solo API `v2`

## Cobertura de pruebas asociada

- `tests/integration/test_v1_compat_smoke.c`
- `tests/integration/test_v1_compat_legacy.c`

Los tests validan contratos legacy e integracion de wrappers en build con y sin FLINT.
