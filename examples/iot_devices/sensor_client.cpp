// ============================================================================
// Mithril Example - IoT Sensor Client
// ============================================================================
// File: examples/iot_devices/sensor_client.cpp
//
// Simple IoT sensor node establishing a secure channel, sending encrypted
// sensor data, and reporting transmission status.
// ============================================================================

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <mithril/boost_asio_compat.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include <protocols/iot_protocol.hpp>

using namespace mithril::protocol::iot;

int main(int argc, char *argv[]) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const std::string port = (argc > 2) ? argv[2] : "19600";
    const int count = (argc > 3) ? std::atoi(argv[3]) : 3;

    std::cout << "============================================================\n";
    std::cout << "🌡️  Mithril IoT Environmental Sensor Client\n";
    std::cout << "   Gateway: " << host << ":" << port << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        ::asio::ip::tcp::socket socket(io_ctx);
        ::asio::ip::tcp::resolver resolver(io_ctx);

        auto endpoints = resolver.resolve(host, port);
        ::asio::connect(socket, endpoints);

        // Handshake
        mithril::sodium::KeyExchange kex;
        const auto &our_pubkey = kex.get_public_key();
        ::asio::write(socket, ::asio::buffer(our_pubkey.data(), our_pubkey.size()));

        mithril::sodium::PublicKey gw_pubkey;
        ::asio::read(socket, ::asio::buffer(gw_pubkey.data(), gw_pubkey.size()));

        auto shared_secret = kex.derive_shared_secret(gw_pubkey);
        mithril::sodium::AuthenticatedEncryption crypto(shared_secret);
        std::cout << "[sensor] Handshake established with gateway.\n";

        for (int i = 1; i <= count; ++i) {
            DeviceTelemetry tel;
            tel.device_id = 0x20B002u;
            tel.sequence = static_cast<uint32_t>(i);
            tel.timestamp_ms = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            tel.temperature_c = 19.8f + static_cast<float>(i) * 0.1f;
            tel.humidity_pct = 62.0f;
            tel.battery_voltage = 3.70f;
            tel.status_flags = 0u;

            auto wire_packet = IoTProtocol::pack_telemetry(tel, crypto);
            uint32_t len = htonl(static_cast<uint32_t>(wire_packet.size()));

            std::array<::asio::const_buffer, 2> bufs{
                ::asio::buffer(&len, sizeof(len)),
                ::asio::buffer(wire_packet)};
            ::asio::write(socket, bufs);

            std::cout << "[sensor] Sent measurement #" << i << " (temp=" << tel.temperature_c << " °C)\n";
        }

        std::cout << "[sensor] Transmission complete.\n";

    } catch (const std::exception &e) {
        std::cerr << "[sensor] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
