# 튜orial: Construcción de Servidor y Cliente Seguro con Mithril

Este tutorial paso a paso muestra cómo construir un servidor de eco seguro y un cliente autenticado en C++20 utilizando `SecureAcceptor` y `SecureSession`.

---

## 1. El Servidor de Eco Seguro

Crea un archivo `echo_server.cpp`:

```cpp
#include "mithril/secure_acceptor.hpp"
#include "mithril/secure_session.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace mithril::sodium;
using tcp = ::asio::ip::tcp;

int main() {
    try {
        asio::io_context io_ctx;
        const uint16_t port = 8080;

        std::cout << "Iniciando Servidor de Eco Seguro en puerto " << port << "...\n";

        // Instanciar el aceptador seguro
        mithril::network::SecureAcceptor acceptor(io_ctx, port);

        acceptor.start([&](std::shared_ptr<mithril::network::SecureSession> session) {
            std::cout << "[Servidor] ¡Nueva sesion segura conectada!\n";

            // Configurar el manejador de mensajes recibidos
            session->set_message_handler([session](std::span<const uint8_t> payload) {
                std::string msg(reinterpret_cast<const char*>(payload.data()), payload.size());
                std::cout << "[Servidor] Mensaje recibido y descifrado: " << msg << "\n";

                // Responder con eco
                std::string reply = "ECO: " + msg;
                session->async_send(
                    std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(reply.data()), reply.size()),
                    [](const asio::error_code& ec, size_t) {
                        if (!ec) {
                            std::cout << "[Servidor] Eco enviado con exito.\n";
                        }
                    }
                );
            });
        });

        // Ejecutar el bucle de eventos asíncronos
        io_ctx.run();

    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

---

## 2. El Cliente Seguro

Crea un archivo `echo_client.cpp`:

```cpp
#include "mithril/secure_session.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace mithril::sodium;
using tcp = ::asio::ip::tcp;

int main() {
    try {
        asio::io_context io_ctx;
        tcp::socket socket(io_ctx);

        tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");

        std::cout << "[Cliente] Conectando al servidor...\n";
        asio::connect(socket, endpoints);

        // Crear sesion segura sobre el socket conectado
        auto session = std::make_shared<mithril::network::SecureSession>(std::move(socket));

        session->set_message_handler([](std::span<const uint8_t> payload) {
            std::string reply(reinterpret_cast<const char*>(payload.data()), payload.size());
            std::cout << "[Cliente] Respuesta del servidor: " << reply << "\n";
        });

        // Iniciar handshake criptografico X25519
        session->start();

        // Enviar mensaje una vez establecido el canal
        std::string secret_msg = "Hola desde el cliente seguro Mithril!";
        session->async_send(
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(secret_msg.data()), secret_msg.size()),
            [](const asio::error_code& ec, size_t bytes) {
                if (!ec) {
                    std::cout << "[Cliente] Mensaje cifrado transmitido (" << bytes << " bytes).\n";
                }
            }
        );

        io_ctx.run();

    } catch (const std::exception& e) {
        std::cerr << "Error en cliente: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

---

## 3. Compilación con CMake

En tu `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MiServicioSeguro CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

# Vincular con Mithril
target_link_libraries(echo_server PRIVATE mithril_network mithril_v2 Threads::Threads ${SODIUM_LIBRARIES})
target_link_libraries(echo_client PRIVATE mithril_network mithril_v2 Threads::Threads ${SODIUM_LIBRARIES})
```
