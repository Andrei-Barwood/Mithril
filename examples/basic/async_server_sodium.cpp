// ============================================================================
// Mithril Example - Asynchronous Multi-Session Server
// ============================================================================
// File: examples/basic/async_server_sodium.cpp
//
// Asynchronous server accepting multiple concurrent encrypted connections,
// logging real-time crypto telemetry metrics and request counts.
// ============================================================================

#include <atomic>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <mithril/boost_asio_compat.hpp>
#include <mithril/common.hpp>
#include <mithril/secure_acceptor.hpp>
#include <mithril/secure_session.hpp>

using namespace mithril::network;

int main(int argc, char *argv[]) {
    const uint16_t port = (argc > 1) ? static_cast<uint16_t>(std::atoi(argv[1])) : 19456;

    std::cout << "============================================================\n";
    std::cout << "⚡ Mithril Asynchronous High-Concurrency Server\n";
    std::cout << "   Port: " << port << " | Backend: " << compat::backend_name() << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        std::atomic<uint64_t> request_count{0};

        auto request_handler = [&](const std::vector<uint8_t> &data) -> ::asio::awaitable<std::vector<uint8_t>> {
            uint64_t req_id = ++request_count;
            std::string msg(data.begin(), data.end());
            std::cout << "[async-server] Request #" << req_id << ": " << msg << "\n";

            const auto counters = mithril::sodium::telemetry::snapshot();
            std::string response = "SUCCESS [Req #" + std::to_string(req_id) + " | Total Ops: " +
                                   std::to_string(counters.total_operations) + "]";
            co_return std::vector<uint8_t>(response.begin(), response.end());
        };

        auto acceptor = std::make_shared<SecureAcceptor>(io_ctx, port);

        ::asio::co_spawn(
            io_ctx,
            acceptor->run(request_handler),
            ::asio::detached);

        std::cout << "[async-server] Ready to accept concurrent requests.\n";
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[async-server] Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
