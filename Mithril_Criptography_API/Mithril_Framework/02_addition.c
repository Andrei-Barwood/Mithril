//
// 
// Mithril Cryptography API - libsodium Implementation
//
// Created by Andres Barbudo
//

#include "01_addition_libsodium.h"
#include <string.h>

/**
 * Inicialización de libsodium (llamar una vez al inicio del programa)
 */
static int mithril_ensure_initialized(void) {
    static int initialized = 0;
    if (!initialized) {
        if (sodium_init() < 0) {
            return -1;
        }
        initialized = 1;
    }
    return 0;
}

/**
 * Suma de enteros de 256 bits con protección timing-safe
 */
int mithril_add_scalar(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]
) {
    if (mithril_ensure_initialized() < 0) {
        return -1;
    }
    
    // Validación de parámetros
    if (!result || !a || !b) {
        return -1;
    }
    
    // Suma con carry tracking (timing-safe)
    uint16_t carry = 0;
    for (size_t i = 0; i < MITHRIL_SCALAR_BYTES; i++) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        result[i] = (uint8_t)(sum & 0xFF);
        carry = sum >> 8;
    }
    
    // Limpiar datos sensibles del stack
    sodium_memzero(&carry, sizeof(carry));
    
    return (carry != 0) ? 1 : 0;  // 1 si hubo overflow
}

/**
 * Suma modular sobre el grupo de Curve25519 (orden L)
 * Esta es la operación más segura para criptografía moderna
 */
int mithril_add_mod_l(
    uint8_t result[crypto_core_ed25519_SCALARBYTES],
    const uint8_t a[crypto_core_ed25519_SCALARBYTES],
    const uint8_t b[crypto_core_ed25519_SCALARBYTES]
) {
    if (mithril_ensure_initialized() < 0) {
        return -1;
    }
    
    if (!result || !a || !b) {
        return -1;
    }
    
    // libsodium proporciona suma modular timing-safe para escalares Ed25519
    crypto_core_ed25519_scalar_add(result, a, b);
    
    return 0;
}

/**
 * Suma constant-time para tamaños variables
 * Útil para operaciones genéricas donde necesitas timing-safety
 */
int mithril_add_constant_time(
    uint8_t *result,
    const uint8_t *a,
    const uint8_t *b,
    size_t len
) {
    if (mithril_ensure_initialized() < 0) {
        return -1;
    }
    
    if (!result || !a || !b || len == 0) {
        return -1;
    }
    
    uint16_t carry = 0;
    
    // Little-endian addition con timing constante
    for (size_t i = 0; i < len; i++) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        result[i] = (uint8_t)(sum & 0xFF);
        carry = sum >> 8;
    }
    
    int overflow = (carry != 0) ? 1 : 0;
    
    // Limpieza segura
    sodium_memzero(&carry, sizeof(carry));
    
    return overflow;
}

/**
 * Función de ejemplo de uso
 */
#ifdef MITHRIL_BUILD_EXAMPLES

#include <stdio.h>

int example_addition(void) {
    uint8_t a[MITHRIL_SCALAR_BYTES];
    uint8_t b[MITHRIL_SCALAR_BYTES];
    uint8_t result[MITHRIL_SCALAR_BYTES];
    
    // Inicializar con valores de ejemplo
    memset(a, 0xFF, MITHRIL_SCALAR_BYTES);  // Máximo valor
    memset(b, 0x01, MITHRIL_SCALAR_BYTES);  // +1
    
    // Realizar suma (detectará overflow)
    int status = mithril_add_scalar(result, a, b);
    
    printf("Addition status: %d (1=overflow, 0=ok, -1=error)\n", status);
    
    // Limpiar datos sensibles
    sodium_memzero(a, sizeof(a));
    sodium_memzero(b, sizeof(b));
    sodium_memzero(result, sizeof(result));
    
    return 0;
}

int example_modular_addition(void) {
    uint8_t a[crypto_core_ed25519_SCALARBYTES];
    uint8_t b[crypto_core_ed25519_SCALARBYTES];
    uint8_t result[crypto_core_ed25519_SCALARBYTES];
    
    // Generar escalares aleatorios
    crypto_core_ed25519_scalar_random(a);
    crypto_core_ed25519_scalar_random(b);
    
    // Suma modular (automáticamente mod L)
    int status = mithril_add_mod_l(result, a, b);
    
    printf("Modular addition status: %d\n", status);
    
    // Limpieza
    sodium_memzero(a, sizeof(a));
    sodium_memzero(b, sizeof(b));
    sodium_memzero(result, sizeof(result));
    
    return 0;
}

#endif // MITHRIL_BUILD_EXAMPLES
