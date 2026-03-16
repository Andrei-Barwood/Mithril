// ============================================================================
// Mithril Network Layer - Secure Client (v2 consumer)
// ============================================================================

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <arpa/inet.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <mithril/mithril_sodium.hpp>
#include <mithril/boost_asio_compat.hpp>

using tcp = asio::ip::tcp;
using mithril::sodium::AuthenticatedEncryption;
using mithril::sodium::KeyExchange;
using mithril::sodium::PublicKey;

class SecureClient {
public:
    explicit SecureClient(asio::io_context &io_ctx)
        : io_ctx_(io_ctx), socket_(io_ctx) {}

    asio::awaitable<void> connect_and_communicate(const std::string &host, const std::string &port) {
        try {
            std::cout << "[client] transport backend: "
                      << mithril::network::compat::backend_name() << "\n";
            std::cout << "[client] resolving " << host << ":" << port << "\n";
            tcp::resolver resolver(io_ctx_);
            auto endpoints = co_await resolver.async_resolve(host, port, asio::use_awaitable);

            std::cout << "[client] connecting...\n";
            co_await asio::async_connect(socket_, endpoints, asio::use_awaitable);
            std::cout << "[client] connected to " << socket_.remote_endpoint() << "\n";

            co_await perform_key_exchange();
            co_await send_encrypted_messages();

            std::cout << "[client] session complete\n";
        } catch (const std::exception &e) {
            std::cerr << "[client] error: " << e.what() << "\n";
        }
    }

private:
    asio::awaitable<void> perform_key_exchange() {
        std::cout << "[client] key exchange using v2 API wrapper\n";

        KeyExchange kex;
        PublicKey server_pubkey;

        co_await asio::async_read(
            socket_,
            asio::buffer(server_pubkey.data(), server_pubkey.size()),
            asio::use_awaitable);

        const auto &our_pubkey = kex.get_public_key();
        co_await asio::async_write(
            socket_,
            asio::buffer(our_pubkey.data(), our_pubkey.size()),
            asio::use_awaitable);

        auto shared_secret = kex.derive_shared_secret(server_pubkey);
        crypto_ = std::make_unique<AuthenticatedEncryption>(shared_secret);

        std::cout << "[client] secure channel established\n";
    }

    asio::awaitable<void> send_encrypted_messages() {
        const std::vector<std::string> messages = {
            "Hello from Mithril secure client!",
            "This path now uses Mithril API v2",
            "Network consumer migration sprint 7",
            "End-to-end encrypted over v2 wrapper"};

        for (const auto &message : messages) {
            std::vector<uint8_t> plaintext(message.begin(), message.end());
            auto encrypted = crypto_->encrypt(plaintext);

            uint32_t length = htonl(static_cast<uint32_t>(encrypted.size()));
            std::array<asio::const_buffer, 2> buffers{
                asio::buffer(&length, sizeof(length)),
                asio::buffer(encrypted)};

            co_await asio::async_write(socket_, buffers, asio::use_awaitable);

            uint32_t response_length = 0u;
            co_await asio::async_read(
                socket_,
                asio::buffer(&response_length, sizeof(response_length)),
                asio::use_awaitable);

            response_length = ntohl(response_length);
            if (response_length == 0u || response_length > 1024u * 1024u) {
                throw std::runtime_error("invalid response length");
            }

            std::vector<uint8_t> encrypted_response(response_length);
            co_await asio::async_read(
                socket_,
                asio::buffer(encrypted_response),
                asio::use_awaitable);

            auto decrypted = crypto_->decrypt(encrypted_response);
            std::string response(decrypted.begin(), decrypted.end());
            std::cout << "[client] recv: \"" << response << "\"\n";

            asio::steady_timer timer(io_ctx_);
            timer.expires_after(std::chrono::milliseconds(250));
            co_await timer.async_wait(asio::use_awaitable);
        }
    }

    asio::io_context &io_ctx_;
    tcp::socket socket_;
    std::unique_ptr<AuthenticatedEncryption> crypto_;
};

int main(int argc, char *argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "usage: " << argv[0] << " <host> <port>\n";
            return 1;
        }

        asio::io_context io_ctx;
        SecureClient client(io_ctx);

        asio::co_spawn(
            io_ctx,
            client.connect_and_communicate(argv[1], argv[2]),
            [](std::exception_ptr e) {
                if (e) {
                    try {
                        std::rethrow_exception(e);
                    } catch (const std::exception &ex) {
                        std::cerr << "[client] fatal: " << ex.what() << "\n";
                    }
                }
            });

        io_ctx.run();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "[client] startup error: " << e.what() << "\n";
        return 1;
    }
}
