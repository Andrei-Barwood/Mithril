# 튜orial: Integración de Mithril en Proyectos Externos

Este tutorial explica cómo integrar la biblioteca Mithril en proyectos existentes de C o C++ mediante CMake (`add_subdirectory` o `FetchContent`).

---

## 1. Integración mediante Submódulo o Subdirectorio

Si incorporas Mithril como submódulo Git dentro de la carpeta `third_party/Mithril`:

```cmake
# En el CMakeLists.txt de tu proyecto principal:
cmake_minimum_required(VERSION 3.20)
project(MiProyecto CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opciones de configuracion de Mithril
set(MITHRIL_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(MITHRIL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MITHRIL_USE_LIBSODIUM ON CACHE BOOL "" FORCE)

# Añadir el subdirectorio de Mithril
add_subdirectory(third_party/Mithril)

# Tu ejecutable o biblioteca
add_executable(mi_aplicacion src/main.cpp)

# Vincular los targets de Mithril
target_link_libraries(mi_aplicacion PRIVATE
    mithril_network
    mithril_v2
)
```

---

## 2. Uso de la API C Pura en Aplicaciones C

Para proyectos escritos en C23 o C17 que solo requieren la capa criptográfica sin red C++:

```c
// main.c
#include <mithril/mithril_crypto.h>
#include <stdio.h>

int main(void) {
    if (mithril_init() != MITHRIL_SUCCESS) {
        fprintf(stderr, "Error al inicializar Mithril.\n");
        return 1;
    }

    uint8_t random_bytes[32];
    mithril_rng_fill(random_bytes, sizeof(random_bytes));
    printf("Bytes aleatorios generados con exito.\n");

    // Generar par de claves X25519
    uint8_t pk[MITHRIL_KEX_PUBLICKEY_BYTES];
    uint8_t sk[MITHRIL_KEX_SECRETKEY_BYTES];
    mithril_kex_keypair(pk, sk);

    printf("Clave publica efimera creada.\n");

    mithril_shutdown();
    return 0;
}
```

Vincular únicamente con el target `mithril_v2`:
```cmake
add_executable(mi_app_c main.c)
target_link_libraries(mi_app_c PRIVATE mithril_v2)
```

---

## 3. Uso de la Capa de Abstracción C++20

```cpp
// main.cpp
#include "mithril/mithril_sodium.hpp"
#include <iostream>

int main() {
    using namespace mithril::sodium;

    // Crear clave segura con zeroization automática
    SecureKey<32> key;
    key.randomize();

    AuthenticatedEncryption aead(key);
    std::string secret = "Confidencial";

    std::vector<uint8_t> ciphertext = aead.encrypt(
        std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(secret.data()), secret.size())
    );

    std::cout << "Texto cifrado de tamaño: " << ciphertext.size() << " bytes.\n";
    return 0;
}
```
