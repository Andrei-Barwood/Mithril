// ============================================================================
// Mithril Example - IoT Gateway Server
// ============================================================================
// File: examples/iot_devices/gateway_server.cpp
//
// Central concentrator gateway receiving AEAD encrypted telemetry frames
// from distributed IoT sensor clients.
// ============================================================================

#include <iostream>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <mithril/boost_asio_compat.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include <protocols/iot_protocol.hpp>

using namespace mithril::protocol::iot;
using tcp = ::asio::ip::tcp;

int main(int argc, char *argv[]) {
    const uint16_t port = (argc > 1) ? static_cast<uint16_t>(std::atoi(argv[1])) : 19600;

    std::cout << "============================================================\n";
    std::cout << "🏢 Mithril IoT Gateway Server\n";
    std::cout << "   Listening on port: " << port << "\n";
    std::cout << "   Backend: " << mithril::network::compat::backend_name() << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), port));

        auto session_loop = [](tcp::socket socket) -> ::asio::awaitable<void> {
            try {
                mithril::sodium::KeyExchange kex;

                // Read client public key
                mithril::sodium::PublicKey client_pubkey;
                co_await ::asio::async_read(
                    socket,
                    ::asio::buffer(client_pubkey.data(), client_pubkey.size()),
                    ::asio::use_awaitable);

                // Send gateway public key
                const auto &our_pubkey = kex.get_public_key();
                co_await ::asio::async_write(
                    socket,
                    ::asio::buffer(our_pubkey.data(), our_pubkey.size()),
                    ::asio::use_awaitable);

                auto shared_secret = kex.derive_shared_secret(client_pubkey);
                mithril::sodium::AuthenticatedEncryption crypto(shared_secret);
                AntiReplayFilter anti_replay(64u, 60000u);

                std::cout << "[gateway] Client connected & authenticated.\n";

                while (true) {
                    uint32_t net_len = 0u;
                    co_await ::asio::async_read(
                        socket,
                        ::asio::buffer(&net_len, sizeof(net_len)),
                        ::asio::use_awaitable);

                    uint32_t len = ntohl(net_len);
                    if (len == 0u || len > 16384u) {
                        break;
                    }

                    std::vector<uint8_t> buf(len);
                    co_await ::asio::async_read(
                        socket,
                        ::asio::buffer(buf),
                        ::asio::use_awaitable);

                    DeviceTelemetry t = IoTProtocol::unpack_telemetry(buf, crypto, &anti_replay);
                    std::cout << "[gateway] DevID: 0x" << std::hex << t.device_id << std::dec
                              << " | Seq: " << t.sequence
                              << " | Temp: " << t.temperature_c << " °C"
                              << " | Hum: " << t.humidity_pct << " %"
                              << " | Bat: " << t.battery_voltage << " V\n";
                }
            } catch (const ::asio::system_error &) {
                // Client closed socket normally
            } catch (const std::exception &e) {
                std::cerr << "[gateway] Client error: " << e.what() << "\n";
            }
        };

        auto accept_loop = [&]() -> ::asio::awaitable<void> {
            while (true) {
                tcp::socket socket = co_await acceptor.async_accept(::asio::use_awaitable);
                ::asio::co_spawn(
                    io_ctx,
                    session_loop(std::move(socket)),
                    ::asio::detached);
            }
        };

        ::asio::co_spawn(io_ctx, accept_loop(), ::asio::detached);
        std::cout << "[gateway] Gateway listening for IoT devices...\n";
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[gateway] Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
