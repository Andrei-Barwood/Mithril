// ============================================================================
// Mithril Network Layer - Secure Acceptor
// ============================================================================
// File: network/include/mithril/secure_acceptor.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_SECURE_ACCEPTOR_HPP
#define MITHRIL_SECURE_ACCEPTOR_HPP

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <cstdint>
#include <memory>

#include <mithril/secure_session.hpp>

namespace mithril::network {

class SecureAcceptor {
public:
    SecureAcceptor(::asio::io_context &io_ctx, uint16_t port);
    ~SecureAcceptor() = default;

    ::asio::awaitable<void> run(SecureSession::MessageHandler handler = nullptr);

    void stop();
    uint16_t port() const noexcept { return port_; }

private:
    ::asio::io_context &io_ctx_;
    tcp::acceptor acceptor_;
    uint16_t port_;
    bool stopped_ = false;
};

} // namespace mithril::network

#endif // MITHRIL_SECURE_ACCEPTOR_HPP
