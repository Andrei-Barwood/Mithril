#include <flint/fmpz_mod.h>
#include <flint/fmpz.h>
#include <stdio.h>

void dsa_signature_verification() {
    const int num_messages = 3;  // Número de mensajes a verificar
    fmpz_t g, y, p, q, r[num_messages], s[num_messages], hashes[num_messages];
    fmpz_t bases[2 * num_messages], exponents[2 * num_messages], results[2 * num_messages];

    // Inicializar valores
    fmpz_init(g);
    fmpz_init(y);
    fmpz_init(p);
    fmpz_init(q);
    for (int i = 0; i < num_messages; i++) {
        fmpz_init(r[i]);
        fmpz_init(s[i]);
        fmpz_init(hashes[i]);
    }
    for (int i = 0; i < 2 * num_messages; i++) {
        fmpz_init(bases[i]);
        fmpz_init(exponents[i]);
        fmpz_init(results[i]);
    }

    // Parámetros públicos
    fmpz_set_ui(p, 104729);  // Módulo grande primo
    fmpz_set_ui(q, 101);     // Orden del subgrupo
    fmpz_set_ui(g, 2);       // Generador
    fmpz_set_ui(y, 45);      // Clave pública

    // Mensajes y firmas
    for (int i = 0; i < num_messages; i++) {
        fmpz_set_ui(r[i], 30 + i);        // r para firma i
        fmpz_set_ui(s[i], 40 + i);        // s para firma i
        fmpz_set_ui(hashes[i], 50 + i);   // H(M) para mensaje i
    }

    // Calcular bases y exponentes
    for (int i = 0; i < num_messages; i++) {
        fmpz_t s_inv, u, v;

        // Calcular s^-1 mod q
        fmpz_init(s_inv);
        fmpz_invmod(s_inv, s[i], q);

        // Calcular u = H(M) * s^-1 mod q
        fmpz_init(u);
        fmpz_mul(u, hashes[i], s_inv);
        fmpz_mod(u, u, q);

        // Calcular v = r * s^-1 mod q
        fmpz_init(v);
        fmpz_mul(v, r[i], s_inv);
        fmpz_mod(v, v, q);

        // Asignar bases y exponentes para umexpm_l()
        fmpz_set(bases[2 * i], g);     // Base g
        fmpz_set(exponents[2 * i], u); // Exponente u

        fmpz_set(bases[2 * i + 1], y); // Base y
        fmpz_set(exponents[2 * i + 1], v); // Exponente v

        // Limpiar variables temporales
        fmpz_clear(s_inv);
        fmpz_clear(u);
        fmpz_clear(v);
    }

    // Realizar exponenciaciones modulares simultáneamente
    umexpm_l(results, bases, exponents, 2 * num_messages, p);

    // Verificar firmas
    for (int i = 0; i < num_messages; i++) {
        fmpz_t lhs, rhs;

        // Calcular (g^u * y^v) mod p
        fmpz_init(lhs);
        fmpz_mul(lhs, results[2 * i], results[2 * i + 1]);
        fmpz_mod(lhs, lhs, p);

        // Comparar con r
        fmpz_init(rhs);
        fmpz_mod(rhs, r[i], p);

        printf("Verificación para mensaje %d: ", i + 1);
        if (fmpz_equal(lhs, rhs)) {
            printf("Firma válida\n");
        } else {
            printf("Firma inválida\n");
        }

        // Limpiar variables
        fmpz_clear(lhs);
        fmpz_clear(rhs);
    }

    // Liberar memoria
    fmpz_clear(g);
    fmpz_clear(y);
    fmpz_clear(p);
    fmpz_clear(q);
    for (int i = 0; i < num_messages; i++) {
        fmpz_clear(r[i]);
        fmpz_clear(s[i]);
        fmpz_clear(hashes[i]);
    }
    for (int i = 0; i < 2 * num_messages; i++) {
        fmpz_clear(bases[i]);
        fmpz_clear(exponents[i]);
        fmpz_clear(results[i]);
    }
}

int main() {
    dsa_signature_verification();
    return 0;
}
