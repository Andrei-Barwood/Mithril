// ============================================================================
// Mithril Network Layer - Secure Session Implementation
// ============================================================================
// File: network/src/secure_session.cpp
// ============================================================================

#include <mithril/secure_session.hpp>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <array>
#include <iostream>

namespace mithril::network {

SecureSession::SecureSession(tcp::socket socket)
    : socket_(std::move(socket)) {
    ::asio::error_code ec;
    remote_endpoint_ = socket_.remote_endpoint(ec);
}

::asio::awaitable<void> SecureSession::server_handshake() {
    sodium::KeyExchange kex;
    const auto &our_pubkey = kex.get_public_key();

    co_await ::asio::async_write(
        socket_,
        ::asio::buffer(our_pubkey.data(), our_pubkey.size()),
        ::asio::use_awaitable);

    sodium::PublicKey peer_pubkey;
    co_await ::asio::async_read(
        socket_,
        ::asio::buffer(peer_pubkey.data(), peer_pubkey.size()),
        ::asio::use_awaitable);

    auto shared_secret = kex.derive_shared_secret(peer_pubkey);
    crypto_ = std::make_unique<sodium::AuthenticatedEncryption>(shared_secret);
    established_ = true;
}

::asio::awaitable<void> SecureSession::client_handshake() {
    sodium::KeyExchange kex;
    sodium::PublicKey server_pubkey;

    co_await ::asio::async_read(
        socket_,
        ::asio::buffer(server_pubkey.data(), server_pubkey.size()),
        ::asio::use_awaitable);

    const auto &our_pubkey = kex.get_public_key();
    co_await ::asio::async_write(
        socket_,
        ::asio::buffer(our_pubkey.data(), our_pubkey.size()),
        ::asio::use_awaitable);

    auto shared_secret = kex.derive_shared_secret(server_pubkey);
    crypto_ = std::make_unique<sodium::AuthenticatedEncryption>(shared_secret);
    established_ = true;
}

::asio::awaitable<void> SecureSession::write_encrypted(const std::vector<uint8_t> &plaintext) {
    if (!crypto_) {
        throw std::runtime_error("Secure session not established");
    }

    auto ciphertext = crypto_->encrypt(plaintext);
    uint32_t len = htonl(static_cast<uint32_t>(ciphertext.size()));

    std::array<::asio::const_buffer, 2> buffers{
        ::asio::buffer(&len, sizeof(len)),
        ::asio::buffer(ciphertext)};

    co_await ::asio::async_write(socket_, buffers, ::asio::use_awaitable);
}

::asio::awaitable<std::vector<uint8_t>> SecureSession::read_encrypted() {
    if (!crypto_) {
        throw std::runtime_error("Secure session not established");
    }

    uint32_t net_len = 0u;
    co_await ::asio::async_read(
        socket_,
        ::asio::buffer(&net_len, sizeof(net_len)),
        ::asio::use_awaitable);

    uint32_t len = ntohl(net_len);
    if (len == 0u || len > 16u * 1024u * 1024u) {
        throw std::runtime_error("Invalid or excessive message length");
    }

    std::vector<uint8_t> cipher(len);
    co_await ::asio::async_read(
        socket_,
        ::asio::buffer(cipher),
        ::asio::use_awaitable);

    co_return crypto_->decrypt(cipher);
}

::asio::awaitable<void> SecureSession::start(MessageHandler handler) {
    auto self = shared_from_this();
    try {
        co_await server_handshake();

        while (true) {
            auto plaintext = co_await read_encrypted();
            if (handler) {
                auto response = co_await handler(plaintext);
                if (!response.empty()) {
                    co_await write_encrypted(response);
                }
            } else {
                // Default echo behavior
                co_await write_encrypted(plaintext);
            }
        }
    } catch (const ::asio::system_error &) {
        // Peer disconnected or socket closed cleanly
    } catch (const std::exception &e) {
        // Encryption or framing error
    }
}

} // namespace mithril::network
