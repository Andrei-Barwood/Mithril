//
// 
// Mithril Cryptography API - libsodium Edition
//
// Created by Andres Barbudo
//

#ifndef MITHRIL_ADDITION_H
#define MITHRIL_ADDITION_H

#include <sodium.h>
#include <stdint.h>
#include <stddef.h>

// Constantes para tamaños seguros
#define MITHRIL_SCALAR_BYTES 32  // 256 bits - estándar para Curve25519/Ed25519

/**
 * @brief Suma de enteros de 256 bits con protección timing-safe
 * @param result Resultado de la suma (32 bytes)
 * @param a Primer operando (32 bytes)
 * @param b Segundo operando (32 bytes)
 * @return 0 en éxito, -1 en error, 1 si hay overflow
 */
int mithril_add_scalar(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]
);

/**
 * @brief Suma modular sobre Curve25519 (timing-safe)
 * @param result Resultado mod L (orden del grupo)
 * @param a Primer operando
 * @param b Segundo operando
 * @return 0 en éxito
 */
int mithril_add_mod_l(
    uint8_t result[crypto_core_ed25519_SCALARBYTES],
    const uint8_t a[crypto_core_ed25519_SCALARBYTES],
    const uint8_t b[crypto_core_ed25519_SCALARBYTES]
);

/**
 * @brief Suma de enteros con protección contra side-channels
 * Para uso en operaciones criptográficas donde el timing debe ser constante
 * @param result Buffer de salida (tamaño flexible)
 * @param a Primer operando
 * @param b Segundo operando
 * @param len Longitud en bytes
 * @return 1 si hubo carry, 0 si no
 */
int mithril_add_constant_time(
    uint8_t *result,
    const uint8_t *a,
    const uint8_t *b,
    size_t len
);

#endif // MITHRIL_ADDITION_H
