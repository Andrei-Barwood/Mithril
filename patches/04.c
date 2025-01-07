#include <flint/fmpz_mod.h>
#include <flint/fmpz.h>
#include <stdio.h>

void aggregate_signature() {
    const int n = 3;  // Número de usuarios
    fmpz_t p, g, H_M, signature, verification;
    fmpz_t public_keys[n];  // Claves públicas de los usuarios
    fmpz_t hash_exponents[n];  // Hash del mensaje como exponente

    // Inicializar números
    fmpz_init(p);
    fmpz_init(g);
    fmpz_init(H_M);
    fmpz_init(signature);
    fmpz_init(verification);

    for (int i = 0; i < n; i++) {
        fmpz_init(public_keys[i]);
        fmpz_init(hash_exponents[i]);
    }

    // Configurar valores
    fmpz_set_str(p, "104729", 10);     // Número primo grande como módulo
    fmpz_set_ui(g, 3);                 // Generador del grupo
    fmpz_set_ui(H_M, 12345);           // Hash del mensaje (simulado)

    // Claves públicas simuladas
    fmpz_set_ui(public_keys[0], 23);   // P1
    fmpz_set_ui(public_keys[1], 47);   // P2
    fmpz_set_ui(public_keys[2], 89);   // P3

    // Hash como exponente para cada clave pública
    for (int i = 0; i < n; i++) {
        fmpz_set(hash_exponents[i], H_M);  // H(M) es constante para todos
    }

    // Calcular la firma agregada: P1^H(M) * P2^H(M) * P3^H(M) mod p
    mexp5m_l(signature, public_keys, hash_exponents, n, p);

    printf("Firma agregada: ");
    fmpz_print(signature);
    printf("\n");

    // Verificar la firma: g^H(M) * sigma^-1 mod p
    fmpz_t inverse_sig;
    fmpz_init(inverse_sig);

    fmpz_invert(inverse_sig, signature, p);  // Calcular sigma^-1
    fmpz_powm_ui(verification, g, fmpz_get_ui(H_M), p);  // g^H(M)
    fmpz_mul(verification, verification, inverse_sig);   // g^H(M) * sigma^-1
    fmpz_mod(verification, verification, p);

    printf("Resultado de la verificación: ");
    fmpz_print(verification);
    printf("\n");

    // Liberar memoria
    fmpz_clear(p);
    fmpz_clear(g);
    fmpz_clear(H_M);
    fmpz_clear(signature);
    fmpz_clear(verification);
    fmpz_clear(inverse_sig);
    for (int i = 0; i < n; i++) {
        fmpz_clear(public_keys[i]);
        fmpz_clear(hash_exponents[i]);
    }
}

int main() {
    aggregate_signature();
    return 0;
}
