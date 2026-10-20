// ============================================================================
// Mithril Example - Connecting Client
// ============================================================================
// File: examples/basic/connecting_client_sodium.cpp
//
// Simple client establishing an authenticated connection to a Mithril server,
// sending an encrypted string, and awaiting an echo response.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <mithril/boost_asio_compat.hpp>
#include <mithril/secure_session.hpp>

using namespace mithril::network;

int main(int argc, char *argv[]) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const std::string port = (argc > 2) ? argv[2] : "19455";

    std::cout << "============================================================\n";
    std::cout << "🔌 Mithril Secure Connecting Client\n";
    std::cout << "   Target: " << host << ":" << port << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        tcp::socket socket(io_ctx);
        tcp::resolver resolver(io_ctx);

        auto endpoints = resolver.resolve(host, port);
        ::asio::connect(socket, endpoints);
        std::cout << "[client] Connected to " << socket.remote_endpoint() << "\n";

        auto session = std::make_shared<SecureSession>(std::move(socket));

        auto run_client = [&]() -> ::asio::awaitable<void> {
            co_await session->client_handshake();
            std::cout << "[client] ✓ Handshake complete. Secure channel open.\n";

            std::string message = "Mithril authenticated client test payload";
            std::vector<uint8_t> payload(message.begin(), message.end());

            std::cout << "[client] Sending: \"" << message << "\"\n";
            co_await session->write_encrypted(payload);

            auto response = co_await session->read_encrypted();
            std::string resp_str(response.begin(), response.end());
            std::cout << "[client] Server reply: \"" << resp_str << "\"\n";

            io_ctx.stop();
        };

        ::asio::co_spawn(io_ctx, run_client(), ::asio::detached);
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[client] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
