// ============================================================================
// Mithril Network Layer - Secure Server (v2 consumer)
// ============================================================================

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <arpa/inet.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <mithril/mithril_sodium.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using mithril::sodium::AuthenticatedEncryption;
using mithril::sodium::KeyExchange;
using mithril::sodium::PublicKey;

class SecureSession : public std::enable_shared_from_this<SecureSession> {
public:
    explicit SecureSession(tcp::socket socket)
        : socket_(std::move(socket)),
          remote_endpoint_(socket_.remote_endpoint()) {}

    asio::awaitable<void> start() {
        try {
            std::cout << "[session] new connection from " << remote_endpoint_ << "\n";
            co_await perform_key_exchange();
            co_await handle_encrypted_communication();
        } catch (const std::exception &e) {
            std::cerr << "[session] error: " << e.what() << "\n";
        }

        std::cout << "[session] closed " << remote_endpoint_ << "\n";
    }

private:
    asio::awaitable<void> perform_key_exchange() {
        KeyExchange kex;

        const auto &our_pubkey = kex.get_public_key();
        co_await asio::async_write(
            socket_,
            asio::buffer(our_pubkey.data(), our_pubkey.size()),
            asio::use_awaitable);

        PublicKey peer_pubkey;
        co_await asio::async_read(
            socket_,
            asio::buffer(peer_pubkey.data(), peer_pubkey.size()),
            asio::use_awaitable);

        auto shared_secret = kex.derive_shared_secret(peer_pubkey);
        crypto_ = std::make_unique<AuthenticatedEncryption>(shared_secret);

        std::cout << "[session] secure channel established\n";
    }

    asio::awaitable<void> handle_encrypted_communication() {
        while (true) {
            uint32_t message_length = 0u;
            co_await asio::async_read(
                socket_,
                asio::buffer(&message_length, sizeof(message_length)),
                asio::use_awaitable);

            message_length = ntohl(message_length);
            if (message_length == 0u || message_length > 1024u * 1024u) {
                throw std::runtime_error("invalid message length");
            }

            std::vector<uint8_t> encrypted_message(message_length);
            co_await asio::async_read(
                socket_,
                asio::buffer(encrypted_message),
                asio::use_awaitable);

            auto plaintext = crypto_->decrypt(encrypted_message);
            std::string message(plaintext.begin(), plaintext.end());
            std::cout << "[session] recv: \"" << message << "\"\n";

            std::string response = "Echo: " + message;
            std::vector<uint8_t> response_data(response.begin(), response.end());
            auto encrypted_response = crypto_->encrypt(response_data);

            uint32_t response_length = htonl(static_cast<uint32_t>(encrypted_response.size()));
            std::array<asio::const_buffer, 2> buffers{
                asio::buffer(&response_length, sizeof(response_length)),
                asio::buffer(encrypted_response)};

            co_await asio::async_write(socket_, buffers, asio::use_awaitable);
        }
    }

    tcp::socket socket_;
    tcp::endpoint remote_endpoint_;
    std::unique_ptr<AuthenticatedEncryption> crypto_;
};

class SecureAcceptor {
public:
    SecureAcceptor(asio::io_context &io_ctx, uint16_t port)
        : io_ctx_(io_ctx),
          acceptor_(io_ctx, tcp::endpoint(tcp::v4(), port)),
          port_(port) {}

    asio::awaitable<void> run() {
        constexpr int kBacklogSize = 30;
        acceptor_.listen(kBacklogSize);

        std::cout << "[server] Mithril secure server (v2 wrapper)\n";
        std::cout << "[server] listening on 0.0.0.0:" << port_ << "\n";

        while (true) {
            auto socket = co_await acceptor_.async_accept(asio::use_awaitable);
            std::cout << "[server] accepted " << socket.remote_endpoint() << "\n";

            auto session = std::make_shared<SecureSession>(std::move(socket));
            asio::co_spawn(io_ctx_, session->start(), asio::detached);
        }
    }

private:
    asio::io_context &io_ctx_;
    tcp::acceptor acceptor_;
    uint16_t port_;
};

int main() {
    try {
        constexpr uint16_t kPort = 3333;

        asio::io_context io_ctx;
        SecureAcceptor server(io_ctx, kPort);

        asio::co_spawn(
            io_ctx,
            server.run(),
            [](std::exception_ptr e) {
                if (e) {
                    try {
                        std::rethrow_exception(e);
                    } catch (const std::exception &ex) {
                        std::cerr << "[server] fatal: " << ex.what() << "\n";
                    }
                }
            });

        io_ctx.run();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "[server] startup error: " << e.what() << "\n";
        return 1;
    }
}
