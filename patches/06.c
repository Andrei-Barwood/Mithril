#include <flint/fmpz_mod.h>
#include <flint/fmpz.h>
#include <stdio.h>

void diffie_hellman_example() {
    fmpz_t p, g, a, b, A, B, K1, K2;

    // Inicializar números
    fmpz_init(p);
    fmpz_init(g);
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(A);
    fmpz_init(B);
    fmpz_init(K1);
    fmpz_init(K2);

    // Configurar módulo p (primo grande) y base g (generador)
    fmpz_set_str(p, "104729", 10);  // Número primo grande como módulo
    fmpz_set_ui(g, 5);              // Generador del grupo

    // Claves privadas (valores secretos)
    fmpz_set_ui(a, 12345);          // Secreto de la Parte 1
    fmpz_set_ui(b, 67890);          // Secreto de la Parte 2

    // Calcular valores públicos
    umexp_l(A, g, a, p);            // A = g^a mod p
    umexp_l(B, g, b, p);            // B = g^b mod p

    // Intercambio de valores públicos y cálculo de clave compartida
    umexp_l(K1, B, a, p);           // K1 = B^a mod p (Parte 1 calcula)
    umexp_l(K2, A, b, p);           // K2 = A^b mod p (Parte 2 calcula)

    // Imprimir resultados
    printf("Valor público de Parte 1 (A): ");
    fmpz_print(A);
    printf("\n");

    printf("Valor público de Parte 2 (B): ");
    fmpz_print(B);
    printf("\n");

    printf("Clave compartida calculada por Parte 1 (K1): ");
    fmpz_print(K1);
    printf("\n");

    printf("Clave compartida calculada por Parte 2 (K2): ");
    fmpz_print(K2);
    printf("\n");

    // Verificar que ambas claves son iguales
    if (fmpz_equal(K1, K2)) {
        printf("Las claves compartidas coinciden.\n");
    } else {
        printf("Error: Las claves no coinciden.\n");
    }

    // Liberar memoria
    fmpz_clear(p);
    fmpz_clear(g);
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(A);
    fmpz_clear(B);
    fmpz_clear(K1);
    fmpz_clear(K2);
}

int main() {
    diffie_hellman_example();
    return 0;
}
