#include <flint/fmpz_mod.h>

void verify_signature() {
    fmpz_t g, r, p, result;

    // Inicializar los números
    fmpz_init(g);
    fmpz_init(r);
    fmpz_init(p);
    fmpz_init(result);

    // Ejemplo de valores
    fmpz_set_ui(g, 5);   // Base
    fmpz_set_ui(r, 3);   // Exponente (parte de la firma)
    fmpz_set_str(p, "23", 10); // Primo grande

    // Calcular g^r mod p
    fmpz_powm(result, g, r, p);

    printf("g^r mod p = ");
    fmpz_print(result);
    printf("\n");

    // Liberar memoria
    fmpz_clear(g);
    fmpz_clear(r);
    fmpz_clear(p);
    fmpz_clear(result);
}
