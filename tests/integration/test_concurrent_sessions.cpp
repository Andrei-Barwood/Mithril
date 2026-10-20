// ============================================================================
// Mithril Tests - Integration: Concurrent Sessions
// ============================================================================
// File: tests/integration/test_concurrent_sessions.cpp
// ============================================================================

#include <atomic>
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
        const uint16_t test_port = 19452;
        const int num_clients = 6;
        std::atomic<int> completed_clients{0};

        // Echo server handler
        auto message_handler = [](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            std::string msg(data.begin(), data.end());
            std::string reply = "ECHO:" + msg;
            co_return std::vector<uint8_t>(reply.begin(), reply.end());
        };

        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, test_port);

        ::asio::co_spawn(
            io_ctx,
            acceptor->run(message_handler),
            ::asio::detached);

        // Client spawner
        for (int i = 0; i < num_clients; ++i) {
            auto client_routine = [i, &completed_clients, &io_ctx, acceptor]() -> ::asio::awaitable<void> {
                try {
                    tcp::socket sock(io_ctx);
                    tcp::endpoint ep(::asio::ip::make_address("127.0.0.1"), test_port);

                    co_await sock.async_connect(ep, ::asio::use_awaitable);

                    auto session = std::make_shared<SecureSession>(std::move(sock));
                    co_await session->client_handshake();
                    MITHRIL_TEST_ASSERT(session->is_established());

                    std::string payload = "Session-" + std::to_string(i) + "-data";
                    std::vector<uint8_t> p(payload.begin(), payload.end());
                    co_await session->write_encrypted(p);

                    auto response = co_await session->read_encrypted();
                    std::string resp_str(response.begin(), response.end());

                    MITHRIL_TEST_ASSERT_EQ(resp_str, "ECHO:" + payload);

                    if (++completed_clients == num_clients) {
                        acceptor->stop();
                        io_ctx.stop();
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Client " << i << " error: " << e.what() << "\n";
                    MITHRIL_TEST_ASSERT(false);
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
