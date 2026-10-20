// ============================================================================
// Mithril Example - Tidal Energy Sensor Node (Turbine Telemetry Server)
// ============================================================================
// File: examples/tidal_energy/tidal_sensor_server.cpp
//
// Simulates a submerged tidal turbine telemetry node transmitting authenticated,
// encrypted metric frames over a Mithril secure channel to the control panel.
// ============================================================================

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <mithril/boost_asio_compat.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/secure_acceptor.hpp>
#include <mithril/secure_session.hpp>
#include <protocols/tidal_sensor_protocol.hpp>

using namespace mithril::network;
using namespace mithril::protocol::tidal;

namespace {

TidalMetrics generate_simulated_metrics(uint64_t device_id, uint32_t seq, double t) {
    TidalMetrics m;
    m.device_id = device_id;
    m.sequence = seq;
    m.timestamp_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    // Tidal hydrodynamics simulation (sinusoidal flow with depth pressure)
    m.current_velocity_mps = static_cast<float>(2.4 + 0.8 * std::sin(t * 0.1));
    m.water_pressure_kpa = static_cast<float>(340.0 + 5.0 * std::cos(t * 0.05));
    m.turbine_rpm = static_cast<float>(35.0 + 12.0 * (m.current_velocity_mps / 3.2));
    m.power_output_kw = static_cast<float>(850.0 + 400.0 * std::pow(m.current_velocity_mps / 3.0, 3.0));
    m.water_temperature_c = 11.4f;
    m.alternator_status = status_flags::GRID_CONNECTED | status_flags::ROTOR_SPINNING | status_flags::SENSOR_CALIBRATED;

    return m;
}

} // namespace

int main(int argc, char *argv[]) {
    const uint16_t port = (argc > 1) ? static_cast<uint16_t>(std::atoi(argv[1])) : 19500;
    const int max_packets = (argc > 2) ? std::atoi(argv[2]) : 5;

    std::cout << "============================================================\n";
    std::cout << "🌊 Mithril Tidal Turbine Sensor Node (TIDAL-T01)\n";
    std::cout << "   Listening on port: " << port << "\n";
    std::cout << "   Transport: " << compat::backend_name() << " | Rollout: " << compat::rollout_summary() << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), port));

        // Node cryptographic identity
        mithril::sodium::DigitalSignature turbine_identity;
        const auto &vk = turbine_identity.get_verify_key();
        std::cout << "[turbine] Node Verify Key (Ed25519): "
                  << mithril::sodium::util::to_hex({vk.data(), vk.size()}).substr(0, 16) << "...\n";

        std::cout << "[turbine] Waiting for authorized control panel connection...\n";
        tcp::socket socket = acceptor.accept();
        std::cout << "[turbine] Connection established from " << socket.remote_endpoint() << "\n";

        auto session = std::make_shared<SecureSession>(std::move(socket));
        
        // Run handshake and transmit telemetry
        auto worker = [&]() -> ::asio::awaitable<void> {
            co_await session->start(); // Runs server_handshake
            co_return;
        };

        // Standard coroutine approach for session telemetry streaming
        auto telemetry_streamer = [&]() -> ::asio::awaitable<void> {
            // Perform ECDH handshake
            mithril::sodium::KeyExchange kex;
            const auto &our_pubkey = kex.get_public_key();

            co_await ::asio::async_write(
                session->socket(),
                ::asio::buffer(our_pubkey.data(), our_pubkey.size()),
                ::asio::use_awaitable);

            mithril::sodium::PublicKey peer_pubkey;
            co_await ::asio::async_read(
                session->socket(),
                ::asio::buffer(peer_pubkey.data(), peer_pubkey.size()),
                ::asio::use_awaitable);

            auto shared_secret = kex.derive_shared_secret(peer_pubkey);
            mithril::sodium::AuthenticatedEncryption crypto(shared_secret);
            std::cout << "[turbine] ✓ Handshake complete. Authenticated channel open.\n\n";

            uint32_t seq = 1u;
            for (int i = 0; i < max_packets; ++i) {
                auto metrics = generate_simulated_metrics(0x71DA1001ULL, seq++, static_cast<double>(i));

                std::cout << "[telemetry #" << metrics.sequence << "] Velocity: "
                          << metrics.current_velocity_mps << " m/s | RPM: "
                          << metrics.turbine_rpm << " | Power: "
                          << metrics.power_output_kw << " kW | Temp: "
                          << metrics.water_temperature_c << " °C\n";

                // Package and sign frame
                auto wire_packet = TidalProtocol::pack_telemetry(metrics, crypto, &turbine_identity);

                // Send 4-byte length prefix + packet
                uint32_t len = htonl(static_cast<uint32_t>(wire_packet.size()));
                std::array<::asio::const_buffer, 2> bufs{
                    ::asio::buffer(&len, sizeof(len)),
                    ::asio::buffer(wire_packet)};

                co_await ::asio::async_write(session->socket(), bufs, ::asio::use_awaitable);
            }

            std::cout << "\n[turbine] Transmitted " << max_packets << " authenticated telemetry packets.\n";
            std::cout << "[turbine] Session completed cleanly.\n";
            io_ctx.stop();
        };

        ::asio::co_spawn(io_ctx, telemetry_streamer(), ::asio::detached);
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[turbine] Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
