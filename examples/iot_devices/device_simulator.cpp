// ============================================================================
// Mithril Example - IoT Device Simulator
// ============================================================================
// File: examples/iot_devices/device_simulator.cpp
//
// Simulates an edge environmental IoT device generating telemetry packets,
// applying ChaCha20-Poly1305 AEAD authenticated encryption and streaming to a gateway.
// ============================================================================

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <mithril/boost_asio_compat.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include <protocols/iot_protocol.hpp>

using namespace mithril::protocol::iot;

int main(int argc, char *argv[]) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const uint16_t port = (argc > 2) ? static_cast<uint16_t>(std::atoi(argv[2])) : 19600;
    const int count = (argc > 3) ? std::atoi(argv[3]) : 5;

    std::cout << "============================================================\n";
    std::cout << "📡 Mithril IoT Device Simulator (Edge Node)\n";
    std::cout << "   Connecting to Gateway: " << host << ":" << port << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        ::asio::ip::tcp::socket socket(io_ctx);
        ::asio::ip::tcp::resolver resolver(io_ctx);

        auto endpoints = resolver.resolve(host, std::to_string(port));
        ::asio::connect(socket, endpoints);
        std::cout << "[device] Connected to gateway.\n";

        // Perform X25519 ECDH handshake
        mithril::sodium::KeyExchange kex;
        const auto &our_pubkey = kex.get_public_key();
        ::asio::write(socket, ::asio::buffer(our_pubkey.data(), our_pubkey.size()));

        mithril::sodium::PublicKey gw_pubkey;
        ::asio::read(socket, ::asio::buffer(gw_pubkey.data(), gw_pubkey.size()));

        auto shared_secret = kex.derive_shared_secret(gw_pubkey);
        mithril::sodium::AuthenticatedEncryption crypto(shared_secret);
        std::cout << "[device] ✓ ECDH Handshake successful with gateway.\n\n";

        for (int i = 1; i <= count; ++i) {
            DeviceTelemetry tel;
            tel.device_id = 0x10A001u;
            tel.sequence = static_cast<uint32_t>(i);
            tel.timestamp_ms = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            tel.temperature_c = 21.5f + static_cast<float>(i) * 0.3f;
            tel.humidity_pct = 55.0f - static_cast<float>(i) * 0.2f;
            tel.battery_voltage = 3.65f - static_cast<float>(i) * 0.01f;
            tel.status_flags = 0x01u; // Normal operation

            std::cout << "[device] Sending Telemetry #" << i << ": Temp="
                      << tel.temperature_c << " °C, Humidity=" << tel.humidity_pct
                      << "%, Battery=" << tel.battery_voltage << " V\n";

            auto wire_packet = IoTProtocol::pack_telemetry(tel, crypto);

            uint32_t len = htonl(static_cast<uint32_t>(wire_packet.size()));
            std::array<::asio::const_buffer, 2> bufs{
                ::asio::buffer(&len, sizeof(len)),
                ::asio::buffer(wire_packet)};
            ::asio::write(socket, bufs);

            // Wait 50ms between packets in simulator
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "\n[device] ✓ Successfully streamed " << count << " encrypted packets.\n";

    } catch (const std::exception &e) {
        std::cerr << "[device] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
