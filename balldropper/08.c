#include <flint/fmpz.h>
#include <stdio.h>

// Función principal para cifrar y descifrar usando wmexp_l
void rsa_encrypt_decrypt() {
    fmpz_t n, e, d, m, c, decrypted;

    // Inicializar números grandes
    fmpz_init(n);
    fmpz_init(e);
    fmpz_init(d);
    fmpz_init(m);
    fmpz_init(c);
    fmpz_init(decrypted);

    // Parámetros RSA
    fmpz_t p, q, phi;

    fmpz_init(p);
    fmpz_init(q);
    fmpz_init(phi);

    // Configuración de claves
    fmpz_set_ui(p, 61);  // Primo 1
    fmpz_set_ui(q, 53);  // Primo 2
    fmpz_mul(n, p, q);   // n = p * q

    // phi = (p-1)*(q-1)
    fmpz_sub_ui(p, p, 1);
    fmpz_sub_ui(q, q, 1);
    fmpz_mul(phi, p, q);

    fmpz_set_ui(e, 17);  // Exponente público
    fmpz_invmod(d, e, phi);  // Exponente privado d = e^(-1) mod phi

    // Mensaje a cifrar
    fmpz_set_ui(m, 65);  // Mensaje como número (en práctica, se usa padding)

    printf("Mensaje original: ");
    fmpz_print(m);
    printf("\n");

    // Cifrado: c = m^e mod n
    wmexp_l(c, m, e, n);

    printf("Mensaje cifrado: ");
    fmpz_print(c);
    printf("\n");

    // Descifrado: decrypted = c^d mod n
    wmexp_l(decrypted, c, d, n);

    printf("Mensaje descifrado: ");
    fmpz_print(decrypted);
    printf("\n");

    // Liberar memoria
    fmpz_clear(n);
    fmpz_clear(e);
    fmpz_clear(d);
    fmpz_clear(m);
    fmpz_clear(c);
    fmpz_clear(decrypted);
    fmpz_clear(p);
    fmpz_clear(q);
    fmpz_clear(phi);
}

int main() {
    rsa_encrypt_decrypt();
    return 0;
}
