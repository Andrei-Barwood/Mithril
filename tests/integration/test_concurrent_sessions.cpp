// ============================================================================
// Mithril Tests - Integration: Concurrent Sessions
// ============================================================================
// File: tests/integration/test_concurrent_sessions.cpp
// ============================================================================

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <mithril/secure_session.hpp>
#include <mithril/secure_acceptor.hpp>
#include "../test_helper.hpp"

using namespace mithril::network;

int main() {
    std::cout << "[TEST] Starting Integration Test: Concurrent Sessions\n";

    try {
        ::asio::io_context io_ctx;
        const int num_clients = 6;
        std::atomic<int> completed_clients{0};

        // Echo server handler
        auto message_handler = [](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            std::string msg(data.begin(), data.end());
            std::string reply = "ECHO:" + msg;
            co_return std::vector<uint8_t>(reply.begin(), reply.end());
        };

        // Use dynamic ephemeral port (0) to eliminate any port collision or TIME_WAIT issues
        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, 0);
        const uint16_t test_port = acceptor->port();

        ::asio::co_spawn(
            io_ctx,
            acceptor->run(message_handler),
            ::asio::detached);

        // 10-second watchdog timer to guarantee the test never hangs indefinitely
        auto watchdog = std::make_shared<::asio::steady_timer>(io_ctx, std::chrono::seconds(10));
        watchdog->async_wait([&completed_clients, acceptor, &io_ctx](const boost::system::error_code &ec) {
            if (!ec) {
                std::cerr << "[WATCHDOG] Test timed out after 10s! Completed clients: "
                          << completed_clients.load() << "/" << num_clients << std::endl;
                MITHRIL_TEST_ASSERT(false);
                acceptor->stop();
                io_ctx.stop();
            }
        });

        // Client spawner
        for (int i = 0; i < num_clients; ++i) {
            auto client_routine = [i, &completed_clients, &io_ctx, acceptor, watchdog, test_port]() -> ::asio::awaitable<void> {
                try {
                    tcp::socket sock(io_ctx);
                    tcp::endpoint ep(::asio::ip::make_address("127.0.0.1"), test_port);

                    // Connect with backoff retry to handle asynchronous scheduler delays gracefully
                    boost::system::error_code ec;
                    for (int attempt = 0; attempt < 20; ++attempt) {
                        co_await sock.async_connect(ep, ::asio::redirect_error(::asio::use_awaitable, ec));
                        if (!ec) {
                            break;
                        }
                        ::asio::steady_timer retry_timer(io_ctx, std::chrono::milliseconds(10));
                        co_await retry_timer.async_wait(::asio::use_awaitable);
                    }
                    if (ec) {
                        throw boost::system::system_error(ec);
                    }

                    auto session = std::make_shared<SecureSession>(std::move(sock));
                    co_await session->client_handshake();
                    MITHRIL_TEST_ASSERT(session->is_established());

                    std::string payload = "Session-" + std::to_string(i) + "-data";
                    std::vector<uint8_t> p(payload.begin(), payload.end());
                    co_await session->write_encrypted(p);

                    auto response = co_await session->read_encrypted();
                    std::string resp_str(response.begin(), response.end());

                    MITHRIL_TEST_ASSERT_EQ(resp_str, "ECHO:" + payload);
                } catch (const std::exception &e) {
                    std::cerr << "Client " << i << " error: " << e.what() << std::endl;
                    MITHRIL_TEST_ASSERT(false);
                }

                if (++completed_clients == num_clients) {
                    watchdog->cancel();
                    acceptor->stop();
                    io_ctx.stop();
                }
            };

            ::asio::co_spawn(io_ctx, client_routine(), ::asio::detached);
        }

        io_ctx.run();

        MITHRIL_TEST_ASSERT_EQ(completed_clients.load(), num_clients);

    } catch (const std::exception &e) {
        std::cerr << "Exception in test_concurrent_sessions: " << e.what() << "\n";
        MITHRIL_TEST_ASSERT(false);
    }

    return mithril::test::report_and_exit("Integration: Concurrent Sessions");
}
