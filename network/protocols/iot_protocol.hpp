// ============================================================================
// Mithril Protocol - Generic IoT Sensor Protocol
// ============================================================================
// File: network/protocols/iot_protocol.hpp
//
// Lightweight binary protocol for resource-constrained IoT telemetry devices.
// Encrypted with Authenticated Encryption (AEAD) and protected against replay attacks.
// ============================================================================

#pragma once

#ifndef MITHRIL_IOT_PROTOCOL_HPP
#define MITHRIL_IOT_PROTOCOL_HPP

#include <chrono>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <vector>

#include <mithril/common.hpp>
#include <mithril/stream_encryption.hpp>
#include "tidal_sensor_protocol.hpp"

namespace mithril::protocol::iot {

inline constexpr uint16_t IOT_MAGIC_BYTES = 0x4D49; // "MI" (Mithril IoT)
inline constexpr uint8_t IOT_PROTOCOL_VERSION = 1u;

enum class MessageType : uint8_t {
    Telemetry = 0x01,
    Heartbeat = 0x02,
    Command   = 0x03,
    Alert     = 0x04
};

#pragma pack(push, 1)
struct DeviceTelemetry {
    uint32_t device_id = 0u;
    uint64_t timestamp_ms = 0u;
    uint32_t sequence = 0u;
    float temperature_c = 0.0f;
    float humidity_pct = 0.0f;
    float battery_voltage = 0.0f;
    uint32_t status_flags = 0u;
};
#pragma pack(pop)

using AntiReplayFilter = tidal::AntiReplayFilter;

class IoTProtocol {
public:
    static std::vector<uint8_t> pack_telemetry(
        const DeviceTelemetry &data,
        const sodium::AuthenticatedEncryption &crypto) {

        const size_t raw_size = sizeof(DeviceTelemetry);
        std::vector<uint8_t> payload(raw_size);
        std::memcpy(payload.data(), &data, raw_size);

        std::vector<uint8_t> out;
        // Magic (2B) + Version (1B) + Type (1B)
        out.push_back(static_cast<uint8_t>((IOT_MAGIC_BYTES >> 8u) & 0xFFu));
        out.push_back(static_cast<uint8_t>(IOT_MAGIC_BYTES & 0xFFu));
        out.push_back(IOT_PROTOCOL_VERSION);
        out.push_back(static_cast<uint8_t>(MessageType::Telemetry));

        auto encrypted = crypto.encrypt(payload);

        // Payload length (2B, network byte order)
        uint16_t enc_len = static_cast<uint16_t>(encrypted.size());
        out.push_back(static_cast<uint8_t>((enc_len >> 8u) & 0xFFu));
        out.push_back(static_cast<uint8_t>(enc_len & 0xFFu));

        out.insert(out.end(), encrypted.begin(), encrypted.end());
        return out;
    }

    static DeviceTelemetry unpack_telemetry(
        std::span<const uint8_t> wire_packet,
        const sodium::AuthenticatedEncryption &crypto,
        AntiReplayFilter *anti_replay = nullptr) {

        constexpr size_t kHeaderSize = 2u + 1u + 1u + 2u; // 6 bytes
        if (wire_packet.size() < kHeaderSize) {
            throw std::invalid_argument("IoT packet too short");
        }

        uint16_t magic = (static_cast<uint16_t>(wire_packet[0]) << 8u) | wire_packet[1];
        if (magic != IOT_MAGIC_BYTES) {
            throw std::runtime_error("Invalid IoT magic identifier");
        }

        uint8_t version = wire_packet[2];
        if (version != IOT_PROTOCOL_VERSION) {
            throw std::runtime_error("Unsupported IoT protocol version");
        }

        uint8_t msg_type = wire_packet[3];
        if (msg_type != static_cast<uint8_t>(MessageType::Telemetry)) {
            throw std::runtime_error("Unexpected IoT message type");
        }

        uint16_t enc_len = (static_cast<uint16_t>(wire_packet[4]) << 8u) | wire_packet[5];
        if (wire_packet.size() < kHeaderSize + enc_len) {
            throw std::invalid_argument("Incomplete IoT packet payload");
        }

        std::span<const uint8_t> cipher_span = wire_packet.subspan(kHeaderSize, enc_len);
        auto decrypted = crypto.decrypt(cipher_span);

        if (decrypted.size() < sizeof(DeviceTelemetry)) {
            throw std::runtime_error("Decrypted IoT payload size mismatch");
        }

        DeviceTelemetry telemetry;
        std::memcpy(&telemetry, decrypted.data(), sizeof(DeviceTelemetry));

        if (anti_replay != nullptr) {
            if (!anti_replay->check_and_update(telemetry.sequence, telemetry.timestamp_ms)) {
                throw std::runtime_error("Anti-replay violation in IoT telemetry");
            }
        }

        return telemetry;
    }
};

} // namespace mithril::protocol::iot

#endif // MITHRIL_IOT_PROTOCOL_HPP
