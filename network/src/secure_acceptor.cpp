// ============================================================================
// Mithril Network Layer - Secure Acceptor Implementation
// ============================================================================
// File: network/src/secure_acceptor.cpp
// ============================================================================

#include <mithril/secure_acceptor.hpp>
#include <mithril/boost_asio_compat.hpp>

namespace mithril::network {

SecureAcceptor::SecureAcceptor(::asio::io_context &io_ctx, uint16_t port)
    : io_ctx_(io_ctx),
      acceptor_(io_ctx, tcp::endpoint(tcp::v4(), port)),
      port_(port) {}

::asio::awaitable<void> SecureAcceptor::run(SecureSession::MessageHandler handler) {
    constexpr int kBacklogSize = 30;
    acceptor_.listen(kBacklogSize);

    compat::enforce_supported_crypto_path();

    while (!stopped_) {
        tcp::socket socket = co_await acceptor_.async_accept(::asio::use_awaitable);
        auto session = std::make_shared<SecureSession>(std::move(socket));

        ::asio::co_spawn(
            io_ctx_,
            session->start(handler),
            ::asio::detached);
    }
}

void SecureAcceptor::stop() {
    stopped_ = true;
    ::asio::error_code ec;
    acceptor_.close(ec);
}

} // namespace mithril::network
