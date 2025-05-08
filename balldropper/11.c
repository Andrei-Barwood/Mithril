#include <flint/fmpz.h>
#include <stdio.h>

// Función para generar potencias de 12 modulares
void generate_powers_of_12_modulo(fmpz_t n, fmpz_t exponent_limit) {
    fmpz_t result, exponent;

    // Inicializar números grandes
    fmpz_init(result);
    fmpz_init(exponent);

    printf("Potencias de 12 modulo ");
    fmpz_print(n);
    printf(":\\n");

    // Iterar sobre los exponentes desde 0 hasta el límite
    for (fmpz_set_ui(exponent, 0); fmpz_cmp(exponent, exponent_limit) <= 0; fmpz_add_ui(exponent, exponent, 1)) {
        mexp2_l(result, exponent, n);  // Calcular 12^exponent mod n
        printf("12^");
        fmpz_print(exponent);
        printf(" mod ");
        fmpz_print(n);
        printf(" = ");
        fmpz_print(result);
        printf("\\n");
    }

    // Liberar memoria
    fmpz_clear(result);
    fmpz_clear(exponent);
}

int main() {
    fmpz_t n, exponent_limit;

    // Inicializar números grandes
    fmpz_init(n);
    fmpz_init(exponent_limit);

    // Configurar los valores
    fmpz_set_ui(n, 24);             // Módulo
    fmpz_set_ui(exponent_limit, 4); // Límite superior de los exponentes

    // Generar potencias de 12 modulares
    generate_powers_of_12_modulo(n, exponent_limit);

    // Liberar memoria
    fmpz_clear(n);
    fmpz_clear(exponent_limit);

    return 0;
}
