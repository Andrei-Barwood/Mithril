#include <flint/fmpz_mod.h>
#include <flint/fmpz.h>
#include <stdio.h>

void rsa_encryption_multiple_keys() {
    const int num_keys = 3;  // Número de claves públicas
    fmpz_t message, ciphertexts[num_keys], exponents[num_keys], moduli[num_keys];

    // Inicializar números
    fmpz_init(message);
    for (int i = 0; i < num_keys; i++) {
        fmpz_init(ciphertexts[i]);
        fmpz_init(exponents[i]);
        fmpz_init(moduli[i]);
    }

    // Configurar mensaje y claves públicas
    fmpz_set_ui(message, 42);  // Mensaje a cifrar

    // Clave pública 1: e1 = 3, n1 = 101
    fmpz_set_ui(exponents[0], 3);
    fmpz_set_ui(moduli[0], 101);

    // Clave pública 2: e2 = 5, n2 = 103
    fmpz_set_ui(exponents[1], 5);
    fmpz_set_ui(moduli[1], 103);

    // Clave pública 3: e3 = 7, n3 = 107
    fmpz_set_ui(exponents[2], 7);
    fmpz_set_ui(moduli[2], 107);

    // Cifrar mensaje simultáneamente para todas las claves
    mexpkm_l(ciphertexts, message, exponents, num_keys, moduli);

    // Mostrar los resultados
    for (int i = 0; i < num_keys; i++) {
        printf("Cifrado para clave %d: ", i + 1);
        fmpz_print(ciphertexts[i]);
        printf("\n");
    }

    // Liberar memoria
    fmpz_clear(message);
    for (int i = 0; i < num_keys; i++) {
        fmpz_clear(ciphertexts[i]);
        fmpz_clear(exponents[i]);
        fmpz_clear(moduli[i]);
    }
}

int main() {
    rsa_encryption_multiple_keys();
    return 0;
}
