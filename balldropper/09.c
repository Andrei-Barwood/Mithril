#include <flint/fmpz.h>
#include <stdio.h>

// Función principal para firmar y verificar un mensaje
void rsa_signature_demo() {
    fmpz_t n, e, d, m, s, verified;

    // Inicializar números grandes
    fmpz_init(n);
    fmpz_init(e);
    fmpz_init(d);
    fmpz_init(m);
    fmpz_init(s);
    fmpz_init(verified);

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

    // Mensaje a firmar
    fmpz_set_ui(m, 42);  // Mensaje como número (se puede usar un hash del mensaje)

    printf("Mensaje original: ");
    fmpz_print(m);
    printf("\n");

    // Firma: s = m^d mod n
    wmexpm_l(s, m, d, n);

    printf("Firma generada: ");
    fmpz_print(s);
    printf("\n");

    // Verificación: verified = s^e mod n
    wmexpm_l(verified, s, e, n);

    printf("Mensaje verificado: ");
    fmpz_print(verified);
    printf("\n");

    // Comprobar si la verificación coincide con el mensaje original
    if (fmpz_equal(m, verified)) {
        printf("Firma verificada correctamente.\n");
    } else {
        printf("La firma es inválida.\n");
    }

    // Liberar memoria
    fmpz_clear(n);
    fmpz_clear(e);
    fmpz_clear(d);
    fmpz_clear(m);
    fmpz_clear(s);
    fmpz_clear(verified);
    fmpz_clear(p);
    fmpz_clear(q);
    fmpz_clear(phi);
}

int main() {
    rsa_signature_demo();
    return 0;
}
