// ============================================================================
// Mithril Tests - Integration: Concurrent Sessions
// ============================================================================
// File: tests/integration/test_concurrent_sessions.cpp
// ============================================================================

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
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
        ::asio::io_context server_ctx;
        ::asio::io_context client_ctx;
        const int num_clients = 6;
        std::atomic<int> completed_clients{0};

        // Echo server message handler
        auto message_handler = [](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            std::string msg(data.begin(), data.end());
            std::string reply = "ECHO:" + msg;
            co_return std::vector<uint8_t>(reply.begin(), reply.end());
        };

        // Server accepts on dynamic ephemeral port (0)
        auto acceptor = std::make_shared<SecureAcceptor>(server_ctx, 0);
        const uint16_t test_port = acceptor->port();

        ::asio::co_spawn(
            server_ctx,
            acceptor->run(message_handler),
            ::asio::detached);

        std::thread server_thread([&server_ctx]() {
            server_ctx.run();
        });

        // 10-second watchdog timer on client_ctx to prevent indefinite hangs
        ::asio::steady_timer watchdog(client_ctx, std::chrono::seconds(10));
        watchdog.async_wait([&completed_clients, acceptor, &server_ctx, &client_ctx](const boost::system::error_code &ec) {
            if (!ec) {
                std::cerr << "[WATCHDOG] Test timed out after 10s! Completed clients: "
                          << completed_clients.load() << "/" << num_clients << std::endl;
                MITHRIL_TEST_ASSERT(false);
                acceptor->stop();
                server_ctx.stop();
                client_ctx.stop();
            }
        });

        // Spawn concurrent clients on client_ctx
        for (int i = 0; i < num_clients; ++i) {
            auto client_routine = [i, &completed_clients, &client_ctx, acceptor, &server_ctx, &watchdog, test_port]() -> ::asio::awaitable<void> {
                try {
                    tcp::socket sock(client_ctx);
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
                } catch (const std::exception &e) {
                    std::cerr << "Client " << i << " error: " << e.what() << std::endl;
                    MITHRIL_TEST_ASSERT(false);
                }

                if (++completed_clients == num_clients) {
                    watchdog.cancel();
                    acceptor->stop();
                    server_ctx.stop();
                    client_ctx.stop();
                }
            };

            ::asio::co_spawn(client_ctx, client_routine(), ::asio::detached);
        }

        client_ctx.run();

        if (server_thread.joinable()) {
            server_thread.join();
        }

        MITHRIL_TEST_ASSERT_EQ(completed_clients.load(), num_clients);

    } catch (const std::exception &e) {
        std::cerr << "Exception in test_concurrent_sessions: " << e.what() << "\n";
        MITHRIL_TEST_ASSERT(false);
    }

    return mithril::test::report_and_exit("Integration: Concurrent Sessions");
}
