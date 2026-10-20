// ============================================================================
// Mithril Example - Secure Echo Server
// ============================================================================
// File: examples/basic/echo_server_sodium.cpp
//
// Demonstrates a clean, coroutine-based encrypted TCP echo server using
// Mithril SecureAcceptor and AuthenticatedEncryption.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <mithril/boost_asio_compat.hpp>
#include <mithril/secure_acceptor.hpp>
#include <mithril/secure_session.hpp>

using namespace mithril::network;

int main(int argc, char *argv[]) {
    const uint16_t port = (argc > 1) ? static_cast<uint16_t>(std::atoi(argv[1])) : 19455;

    std::cout << "============================================================\n";
    std::cout << "🔁 Mithril Secure Echo Server\n";
    std::cout << "   Port: " << port << " | Backend: " << compat::backend_name() << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;

        auto echo_handler = [](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            std::string msg(data.begin(), data.end());
            std::cout << "[echo] Received: \"" << msg << "\" (" << data.size() << " bytes)\n";
            std::string reply = "Echo: " + msg;
            co_return std::vector<uint8_t>(reply.begin(), reply.end());
        };

        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, port);

        ::asio::co_spawn(
            io_ctx,
            acceptor->run(echo_handler),
            ::asio::detached);

        std::cout << "[echo] Server listening for connections...\n";
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[echo] Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
