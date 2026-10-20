// ============================================================================
// Mithril Network Layer - Secure Session
// ============================================================================
// File: network/include/mithril/secure_session.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_SECURE_SESSION_HPP
#define MITHRIL_SECURE_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>

namespace mithril::network {

using tcp = ::asio::ip::tcp;

class SecureSession : public std::enable_shared_from_this<SecureSession> {
public:
    using MessageHandler = std::function<::asio::awaitable<std::vector<uint8_t>>(const std::vector<uint8_t> &)>;

    explicit SecureSession(tcp::socket socket);
    ~SecureSession() = default;

    // Server-side handshake & message handling loop
    ::asio::awaitable<void> start(MessageHandler handler = nullptr);

    // Client-side handshake
    ::asio::awaitable<void> client_handshake();

    // Send encrypted message (4-byte length prefix + AEAD ciphertext)
    ::asio::awaitable<void> write_encrypted(const std::vector<uint8_t> &plaintext);

    // Read single encrypted message and return decrypted plaintext
    ::asio::awaitable<std::vector<uint8_t>> read_encrypted();

    tcp::socket &socket() noexcept { return socket_; }
    const tcp::endpoint &remote_endpoint() const noexcept { return remote_endpoint_; }
    bool is_established() const noexcept { return established_; }

private:
    ::asio::awaitable<void> server_handshake();

    tcp::socket socket_;
    tcp::endpoint remote_endpoint_;
    std::unique_ptr<sodium::AuthenticatedEncryption> crypto_;
    bool established_ = false;
};

} // namespace mithril::network

#endif // MITHRIL_SECURE_SESSION_HPP
