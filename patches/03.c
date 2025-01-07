#include <flint/fmpz_mod.h>

void diffie_hellman() {
    fmpz_t base, private_key, public_key, prime, shared_secret;

    // Inicializar números
    fmpz_init(base);
    fmpz_init(private_key);
    fmpz_init(public_key);
    fmpz_init(prime);
    fmpz_init(shared_secret);

    // Ejemplo de valores
    fmpz_set_ui(base, 5);               // Generador
    fmpz_set_ui(private_key, 7);        // Clave privada
    fmpz_set_str(prime, "23", 10);      // Primo

    // Calcular clave pública: base^private_key mod prime
    fmpz_powm(public_key, base, private_key, prime);

    // Simular clave compartida: public_key^private_key mod prime
    fmpz_powm(shared_secret, public_key, private_key, prime);

    printf("Clave pública: ");
    fmpz_print(public_key);
    printf("\nClave compartida: ");
    fmpz_print(shared_secret);
    printf("\n");

    // Liberar memoria
    fmpz_clear(base);
    fmpz_clear(private_key);
    fmpz_clear(public_key);
    fmpz_clear(prime);
    fmpz_clear(shared_secret);
}


void diffie_hellman_key_exchange() {
    fmpz_t base, private_key, prime, public_key;

    // Inicializar números
    fmpz_init(base);
    fmpz_init(private_key);
    fmpz_init(prime);
    fmpz_init(public_key);

    // Configurar valores
    fmpz_set_ui(base, 5);                // Generador
    fmpz_set_ui(private_key, 7);         // Clave privada
    fmpz_set_str(prime, "23", 10);       // Primo

    // Calcular clave pública: base^private_key mod prime usando base k = 8
    mexpk_l(public_key, base, private_key, prime, 8);

    printf("Clave pública: ");
    fmpz_print(public_key);
    printf("\n");

    // Liberar memoria
    fmpz_clear(base);
    fmpz_clear(private_key);
    fmpz_clear(prime);
    fmpz_clear(public_key);
}
