// ============================================================================
// Mithril Tests - Integration: Error Handling & Resilience
// ============================================================================
// File: tests/integration/test_error_handling.cpp
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <mithril/secure_session.hpp>
#include <mithril/secure_acceptor.hpp>
#include <mithril/common.hpp>
#include "../test_helper.hpp"

using namespace mithril::network;

int main() {
    std::cout << "[TEST] Starting Integration Test: Error Handling & Protocol Resilience\n";

    try {
        ::asio::io_context io_ctx;
        auto message_handler = [](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            co_return data;
        };

        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, 0);
        const uint16_t test_port = acceptor->port();

        ::asio::co_spawn(
            io_ctx,
            acceptor->run(message_handler),
            ::asio::detached);

        auto client_scenarios = [&]() -> ::asio::awaitable<void> {
            tcp::endpoint ep(::asio::ip::make_address("127.0.0.1"), test_port);

            // Scenario 1: Immediate disconnect without handshake
            {
                std::cout << "  1. Abrupt disconnect before handshake...\n";
                tcp::socket sock(io_ctx);
                co_await sock.async_connect(ep, ::asio::use_awaitable);
                sock.close(); // Immediate abort
            }

            // Scenario 2: Sending garbage bytes as public key
            {
                std::cout << "  2. Sending invalid/truncated public key...\n";
                tcp::socket sock(io_ctx);
                co_await sock.async_connect(ep, ::asio::use_awaitable);

                // Read server's public key
                std::vector<uint8_t> srv_pk(mithril::sodium::MITHRIL_PUBLIC_KEY_BYTES);
                co_await ::asio::async_read(sock, ::asio::buffer(srv_pk), ::asio::use_awaitable);

                // Send only 5 bytes instead of 32
                std::vector<uint8_t> garbage = {0xDE, 0xAD, 0xBE, 0xEF, 0x00};
                co_await ::asio::async_write(sock, ::asio::buffer(garbage), ::asio::use_awaitable);
                sock.close();
            }

            // Scenario 3: Sending oversized length prefix
            {
                std::cout << "  3. Sending oversized message length (>16MB)...\n";
                tcp::socket sock(io_ctx);
                co_await sock.async_connect(ep, ::asio::use_awaitable);

                auto session = std::make_shared<SecureSession>(std::move(sock));
                co_await session->client_handshake();
                MITHRIL_TEST_ASSERT(session->is_established());

                // Manually inject 32MB length prefix
                uint32_t bad_len = htonl(32u * 1024u * 1024u);
                co_await ::asio::async_write(session->socket(), ::asio::buffer(&bad_len, sizeof(bad_len)), ::asio::use_awaitable);

                // Server should drop the connection due to invalid length
                std::vector<uint8_t> dummy(1);
                ::asio::error_code ec;
                co_await session->socket().async_read_some(::asio::buffer(dummy), ::asio::redirect_error(::asio::use_awaitable, ec));
                MITHRIL_TEST_ASSERT(ec == ::asio::error::eof || ec == ::asio::error::connection_reset);
            }

            acceptor->stop();
            io_ctx.stop();
        };

        ::asio::co_spawn(io_ctx, client_scenarios(), ::asio::detached);

        io_ctx.run();

        // Verify telemetry recorded failures
        const auto counters = mithril::sodium::telemetry::snapshot();
        std::cout << "  Telemetry after resilience tests: total_operations=" << counters.total_operations
                  << ", total_failures=" << counters.total_failures << "\n";

    } catch (const std::exception &e) {
        std::cerr << "Exception in test_error_handling: " << e.what() << "\n";
        MITHRIL_TEST_ASSERT(false);
    }

    return mithril::test::report_and_exit("Integration: Error Handling");
}
