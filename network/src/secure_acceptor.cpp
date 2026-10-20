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
      acceptor_(io_ctx),
      port_(port) {
    tcp::endpoint ep(tcp::v4(), port);
    acceptor_.open(ep.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(ep);
    constexpr int kBacklogSize = 64;
    acceptor_.listen(kBacklogSize);
    ::asio::error_code ec;
    auto local_ep = acceptor_.local_endpoint(ec);
    if (!ec) {
        port_ = local_ep.port();
    }
}

::asio::awaitable<void> SecureAcceptor::run(SecureSession::MessageHandler handler) {
    constexpr int kBacklogSize = 64;
    ::asio::error_code ec;
    acceptor_.listen(kBacklogSize, ec);

    compat::enforce_supported_crypto_path();

    while (!stopped_) {
        try {
            tcp::socket socket = co_await acceptor_.async_accept(::asio::use_awaitable);
            auto session = std::make_shared<SecureSession>(std::move(socket));

            ::asio::co_spawn(
                io_ctx_,
                session->start(handler),
                ::asio::detached);
            session.reset();
        } catch (const std::exception &) {
            if (stopped_) {
                co_return;
            }
        }
    }
}

void SecureAcceptor::stop() {
    stopped_ = true;
    ::asio::error_code ec;
    acceptor_.close(ec);
}

} // namespace mithril::network
