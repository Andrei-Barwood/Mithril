// ============================================================================
// Mithril Example - Tidal Energy Microgrid Data Aggregator
// ============================================================================
// File: examples/tidal_energy/data_aggregator.cpp
//
// Substation concentrator node aggregating telemetry streams from multiple
// subsea tidal generation arrays. Validates cryptographic signatures, verifies
// anti-replay counters, and computes cumulative array metrics.
// ============================================================================

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <mithril/boost_asio_compat.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include <protocols/tidal_sensor_protocol.hpp>

using namespace mithril::network;
using namespace mithril::protocol::tidal;
using tcp = ::asio::ip::tcp;

namespace {

struct TurbineState {
    uint64_t device_id = 0;
    uint32_t last_seq = 0;
    float current_power_kw = 0.0f;
    float current_flow_mps = 0.0f;
    float rotor_rpm = 0.0f;
    uint64_t total_packets = 0;
};

std::mutex g_state_mutex;
std::unordered_map<uint64_t, TurbineState> g_turbines;

void record_metric(const TidalMetrics &m) {
    std::lock_guard<std::mutex> lock(g_state_mutex);
    auto &state = g_turbines[m.device_id];
    state.device_id = m.device_id;
    state.last_seq = m.sequence;
    state.current_power_kw = m.power_output_kw;
    state.current_flow_mps = m.current_velocity_mps;
    state.rotor_rpm = m.turbine_rpm;
    state.total_packets++;

    float total_power = 0.0f;
    for (const auto &[id, s] : g_turbines) {
        total_power += s.current_power_kw;
    }

    std::cout << "[aggregator] Turbine 0x" << std::hex << m.device_id << std::dec
              << " | Seq: " << m.sequence
              << " | Power: " << m.power_output_kw << " kW"
              << " | Grid Total: " << std::fixed << std::setprecision(2) << (total_power / 1000.0f) << " MW ("
              << g_turbines.size() << " active turbines)\n";
}

} // namespace

int main(int argc, char *argv[]) {
    const uint16_t port = (argc > 1) ? static_cast<uint16_t>(std::atoi(argv[1])) : 19501;

    std::cout << "============================================================\n";
    std::cout << "🏢 Mithril Tidal Substation Data Aggregator Hub\n";
    std::cout << "   Listening on port: " << port << "\n";
    std::cout << "   Backend: " << compat::backend_name() << "\n";
    std::cout << "============================================================\n";

    try {
        ::asio::io_context io_ctx;
        tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), port));

        auto session_handler = [](tcp::socket socket) -> ::asio::awaitable<void> {
            try {
                mithril::sodium::KeyExchange kex;

                // Send our key
                const auto &our_pubkey = kex.get_public_key();
                co_await ::asio::async_write(
                    socket,
                    ::asio::buffer(our_pubkey.data(), our_pubkey.size()),
                    ::asio::use_awaitable);

                // Read client key
                mithril::sodium::PublicKey peer_pubkey;
                co_await ::asio::async_read(
                    socket,
                    ::asio::buffer(peer_pubkey.data(), peer_pubkey.size()),
                    ::asio::use_awaitable);

                auto shared = kex.derive_shared_secret(peer_pubkey);
                mithril::sodium::AuthenticatedEncryption crypto(shared);
                AntiReplayFilter anti_replay(128u, 120000u);

                while (true) {
                    uint32_t net_len = 0u;
                    co_await ::asio::async_read(
                        socket,
                        ::asio::buffer(&net_len, sizeof(net_len)),
                        ::asio::use_awaitable);

                    uint32_t len = ntohl(net_len);
                    if (len == 0u || len > 65536u) {
                        break;
                    }

                    std::vector<uint8_t> buf(len);
                    co_await ::asio::async_read(
                        socket,
                        ::asio::buffer(buf),
                        ::asio::use_awaitable);

                    TidalMetrics m = TidalProtocol::unpack_telemetry(buf, crypto, &anti_replay);
                    record_metric(m);
                }
            } catch (const ::asio::system_error &) {
                // Connection closed normally
            } catch (const std::exception &e) {
                std::cerr << "[aggregator] Session error: " << e.what() << "\n";
            }
        };

        auto listener = [&]() -> ::asio::awaitable<void> {
            while (true) {
                tcp::socket socket = co_await acceptor.async_accept(::asio::use_awaitable);
                ::asio::co_spawn(
                    io_ctx,
                    session_handler(std::move(socket)),
                    ::asio::detached);
            }
        };

        ::asio::co_spawn(io_ctx, listener(), ::asio::detached);

        // Run until stopped
        std::cout << "[aggregator] Aggregator ready to receive telemetry streams.\n";
        io_ctx.run();

    } catch (const std::exception &e) {
        std::cerr << "[aggregator] Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
