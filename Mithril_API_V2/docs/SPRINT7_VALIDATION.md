# Sprint 7 Validation Evidence

Fecha de referencia: 2026-03-18

## Validaciones ejecutadas

1. Build y smoke local de consumidores objetivo:

```zsh
make -C /Users/kirtantegsingh/documents/mithril/examples/basic test
```

Resultado esperado: `[test] OK`

2. Verificacion de flags (v1 bloqueado por defecto):

```zsh
cd /Users/kirtantegsingh/documents/mithril/examples/basic
MITHRIL_CRYPTO_PATH=v1 ./secure_client localhost 3333
```

Resultado esperado:

1. exit code `1`
2. mensaje: `MITHRIL_CRYPTO_PATH requests v1/legacy but v1 path is disabled...`

3. Verificacion de guardrail de includes:

```zsh
grep -nE '#include[[:space:]]*<sodium\.h>' \
  /Users/kirtantegsingh/documents/mithril/examples/basic/secure_client.cpp \
  /Users/kirtantegsingh/documents/mithril/examples/basic/accepting_connection_sodium.cpp
```

Resultado esperado: sin coincidencias.

## Cobertura CI asociada

Workflow: `.github/workflows/mithril-v2-build-test.yml`

Jobs relevantes:

1. `build-test` (Mithril_API_V2 matrix)
2. `consumers-smoke` (compila y prueba consumidores migrados)
3. `ci-required-gate` (falla si cualquiera de los dos jobs falla)

## Riesgo conocido y estado

1. Compatibilidad con ruta v1 runtime:
   - Estado: bloqueada por flag y no habilitada por defecto.
   - Mitigacion: mensaje explicito y control de feature flag en build/runtime.

2. Consumidores legacy fuera de alcance:
   - Estado: documentados como out-of-scope en `SPRINT7_CONSUMER_SCOPE.md`.
   - Mitigacion: cierre de Sprint 7 no depende de esos modulos.

## Cierre DoD Sprint 7

1. Consumidores objetivo usan v2 en entorno interno:
   - Estado: listo para sign-off interno (requiere aprobacion operativa del equipo).
2. Sin regresiones funcionales:
   - Estado: smoke local + gate CI de consumidores configurado.
