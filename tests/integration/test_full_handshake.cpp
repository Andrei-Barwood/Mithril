// ============================================================================
// Mithril Tests - Integration: Full Network Handshake & Encrypted Channel
// ============================================================================
// File: tests/integration/test_full_handshake.cpp
// ============================================================================

#include <iostream>
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
    std::cout << "[TEST] Starting Integration Test: Full Handshake & Encrypted Channel\n";

    try {
        ::asio::io_context io_ctx;

        // Choose a dedicated high port for integration test
        const uint16_t test_port = 19451;

        std::vector<std::string> server_received;
        std::vector<std::string> client_received;

        // Message echo handler for server
        auto message_handler = [&](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            std::string msg(data.begin(), data.end());
            server_received.push_back(msg);
            std::string reply = "ACK:" + msg;
            co_return std::vector<uint8_t>(reply.begin(), reply.end());
        };

        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, test_port);

        // Run server in background coroutine
        ::asio::co_spawn(
            io_ctx,
            acceptor->run(message_handler),
            ::asio::detached);

        // Client coroutine
        auto client_routine = [&]() -> ::asio::awaitable<void> {
            tcp::socket sock(io_ctx);
            tcp::endpoint ep(::asio::ip::make_address("127.0.0.1"), test_port);

            co_await sock.async_connect(ep, ::asio::use_awaitable);

            auto session = std::make_shared<SecureSession>(std::move(sock));
            co_await session->client_handshake();
            MITHRIL_TEST_ASSERT(session->is_established());

            // Send 3 encrypted messages
            const std::vector<std::string> messages = {
                "Sensor-Alpha: 45.2kW",
                "Sensor-Beta: 120.8RPM",
                "Grid-Status: Synchronized"
            };

            for (const auto &m : messages) {
                std::vector<uint8_t> p(m.begin(), m.end());
                co_await session->write_encrypted(p);

                auto resp = co_await session->read_encrypted();
                std::string resp_str(resp.begin(), resp.end());
                client_received.push_back(resp_str);
            }

            acceptor->stop();
            io_ctx.stop();
        };

        ::asio::co_spawn(io_ctx, client_routine(), ::asio::detached);

        io_ctx.run();

        // Verify results
        MITHRIL_TEST_ASSERT_EQ(server_received.size(), 3u);
        MITHRIL_TEST_ASSERT_EQ(client_received.size(), 3u);
        if (client_received.size() == 3u) {
            MITHRIL_TEST_ASSERT_EQ(client_received[0], "ACK:Sensor-Alpha: 45.2kW");
            MITHRIL_TEST_ASSERT_EQ(client_received[1], "ACK:Sensor-Beta: 120.8RPM");
            MITHRIL_TEST_ASSERT_EQ(client_received[2], "ACK:Grid-Status: Synchronized");
        }

    } catch (const std::exception &e) {
        std::cerr << "Exception in test_full_handshake: " << e.what() << "\n";
        MITHRIL_TEST_ASSERT(false);
    }

    return mithril::test::report_and_exit("Integration: Full Handshake");
}
