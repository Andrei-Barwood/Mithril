# Sprint 7 Consumer Scope

## Objetivo

Definir de forma verificable que consumidores runtime se consideran objetivo de Sprint 7 y cuales quedan fuera para evitar cierres ambiguos.

## Consumidores objetivo (in-scope)

1. `examples/basic/secure_client.cpp`
2. `examples/basic/accepting_connection_sodium.cpp`
3. `network/include/mithril/mithril_sodium.hpp`
4. `network/include/mithril/boost_asio_compat.hpp`
5. `network/src/boost_asio_compat.cpp`
6. `examples/basic/CMakeLists.txt`
7. `examples/basic/Makefile`

Los consumidores anteriores deben:

1. Usar API v2 para KEX/AEAD/SIGN/HASH/RNG via wrapper (`mithril_*`).
2. Tener path por defecto `v2`.
3. Bloquear ruta `v1` cuando el feature flag de compatibilidad este apagado.

## Fuera de alcance (out-of-scope) en Sprint 7

1. `examples/basic/binding_socket_sodium.cpp`
2. `Mithril_Criptography_API/**` (framework legacy)
3. `ASIC implementation/**` (legacy/experimental)
4. Tooling y scripts de scaffolding no runtime (ej: `add_component.zsh`)

Razon: no forman parte del flujo de consumidor objetivo definido para la migracion operativa de red/seguridad en Sprint 7.

## Criterio de cumplimiento de alcance

Sprint 7 se considera cerrado en alcance cuando:

1. Todos los archivos in-scope compilan y pasan smoke test.
2. Ningun archivo in-scope incluye `sodium.h` directamente.
3. El gate CI incluye validacion de consumidores in-scope.
