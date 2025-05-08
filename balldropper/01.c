#include <flint/nmod_vec.h>
#include <flint/fmpz.h>
#include <flint/fmpz_mod.h>

void rsa_key_generation() {
    fmpz_t e, d, n, result;

    // Inicializar los números
    fmpz_init(e);
    fmpz_init(d);
    fmpz_init(n);
    fmpz_init(result);

    // Ejemplo de valores (usualmente serían mucho más grandes y generados aleatoriamente)
    fmpz_set_ui(e, 65537);                              // Exponente público
    fmpz_set_str(d, "123456789123456789123456789", 10); // Exponente privado
    fmpz_set_str(n, "987654321987654321987654321", 10); // Módulo

    // Calcular e^d mod n
    fmpz_powm(result, e, d, n);

    printf("Resultado: ");
    fmpz_print(result);
    printf("\n");

    // Liberar memoria
    fmpz_clear(e);
    fmpz_clear(d);
    fmpz_clear(n);
    fmpz_clear(result);
}

void rsa_encryption() {
    fmpz_t m, e, n, c;

    // Inicializar números
    fmpz_init(m);
    fmpz_init(e);
    fmpz_init(n);
    fmpz_init(c);

    // Configurar valores (en una implementación real, usa números grandes y seguros)
    fmpz_set_str(m, "123456", 10);       // Mensaje
    fmpz_set_ui(e, 65537);               // Exponente público
    fmpz_set_str(n, "987654321987654321", 10); // Módulo

    // Calcular c = m^e mod n usando base k = 2
    mexpk_l(c, m, e, n, 2);

    printf("Mensaje cifrado: ");
    fmpz_print(c);
    printf("\n");

    // Liberar memoria
    fmpz_clear(m);
    fmpz_clear(e);
    fmpz_clear(n);
    fmpz_clear(c);
}
