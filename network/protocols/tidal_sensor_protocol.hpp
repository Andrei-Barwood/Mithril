// ============================================================================
// Mithril Protocol - Tidal Sensor Telemetry Protocol
// ============================================================================
// File: network/protocols/tidal_sensor_protocol.hpp
//
// Specialized compact, authenticated binary protocol for subsea tidal energy
// turbines, environmental sensor arrays, and microgrid telemetry.
// Incorporates ChaCha20-Poly1305 AEAD, Ed25519 signatures, and a sliding-window
// anti-replay filter.
// ============================================================================

#pragma once

#ifndef MITHRIL_TIDAL_SENSOR_PROTOCOL_HPP
#define MITHRIL_TIDAL_SENSOR_PROTOCOL_HPP

#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <mithril/common.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/stream_encryption.hpp>

namespace mithril::protocol::tidal {

inline constexpr uint16_t TIDAL_MAGIC_BYTES = 0x4D54; // "MT" (Mithril Tidal)
inline constexpr uint8_t TIDAL_PROTOCOL_VERSION = 2u;

enum class MessageType : uint16_t {
    Telemetry = 0x0001,
    Alert = 0x0002,
    Command = 0x0003,
    CommandAck = 0x0004,
    Heartbeat = 0x0005
};

namespace status_flags {
inline constexpr uint32_t GRID_CONNECTED   = 1u << 0;
inline constexpr uint32_t ROTOR_SPINNING   = 1u << 1;
inline constexpr uint32_t OVERHEAT_WARNING = 1u << 2;
inline constexpr uint32_t VIBRATION_ALERT  = 1u << 3;
inline constexpr uint32_t EMERGENCY_BRAKE  = 1u << 4;
inline constexpr uint32_t SENSOR_CALIBRATED= 1u << 5;
} // namespace status_flags

#pragma pack(push, 1)
struct TidalMetrics {
    uint64_t device_id = 0u;
    uint64_t timestamp_ms = 0u;
    uint32_t sequence = 0u;
    float current_velocity_mps = 0.0f; // Tidal flow speed (m/s)
    float water_pressure_kpa = 0.0f;   // Subsea depth pressure (kPa)
    float turbine_rpm = 0.0f;          // Rotor rotational speed
    float power_output_kw = 0.0f;      // Generated electrical power (kW)
    float water_temperature_c = 0.0f;  // Water temperature (°C)
    uint32_t alternator_status = 0u;   // Status bitmask
};

struct CommandPayload {
    uint64_t target_device_id = 0u;
    uint32_t command_id = 0u;
    uint32_t parameter = 0u;
    uint64_t timestamp_ms = 0u;
};
#pragma pack(pop)

// ============================================================================
// Anti-Replay Sliding Window Filter
// ============================================================================

class AntiReplayFilter {
public:
    explicit AntiReplayFilter(uint32_t window_size = 128u, uint64_t max_time_skew_ms = 60000u)
        : window_size_(window_size), max_time_skew_ms_(max_time_skew_ms) {
        bitmap_.resize((window_size_ + 63u) / 64u, 0u);
    }

    bool check_and_update(uint32_t seq, uint64_t timestamp_ms) {
        const uint64_t now_ms = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        // Check timestamp skew (prevent far-future or ancient packets)
        if (max_time_skew_ms_ > 0u) {
            uint64_t diff = (now_ms > timestamp_ms) ? (now_ms - timestamp_ms) : (timestamp_ms - now_ms);
            if (diff > max_time_skew_ms_) {
                return false;
            }
        }

        if (!initialized_) {
            max_seq_ = seq;
            set_bit(0u);
            initialized_ = true;
            return true;
        }

        if (seq > max_seq_) {
            uint32_t delta = seq - max_seq_;
            if (delta >= window_size_) {
                std::fill(bitmap_.begin(), bitmap_.end(), 0u);
            } else {
                shift_window(delta);
            }
            max_seq_ = seq;
            set_bit(0u);
            return true;
        }

        uint32_t diff = max_seq_ - seq;
        if (diff >= window_size_) {
            return false; // Packet is older than the sliding window
        }

        if (is_bit_set(diff)) {
            return false; // Replayed duplicate packet
        }

        set_bit(diff);
        return true;
    }

private:
    void shift_window(uint32_t shift) {
        if (shift >= window_size_) {
            std::fill(bitmap_.begin(), bitmap_.end(), 0u);
            return;
        }
        for (uint32_t i = window_size_ - 1u; i >= shift; --i) {
            if (is_bit_set(i - shift)) {
                set_bit(i);
            } else {
                clear_bit(i);
            }
        }
        for (uint32_t i = 0u; i < shift; ++i) {
            clear_bit(i);
        }
    }

    bool is_bit_set(uint32_t bit) const noexcept {
        const size_t word = bit / 64u;
        const size_t pos = bit % 64u;
        return (bitmap_[word] & (1ULL << pos)) != 0u;
    }

    void set_bit(uint32_t bit) noexcept {
        const size_t word = bit / 64u;
        const size_t pos = bit % 64u;
        bitmap_[word] |= (1ULL << pos);
    }

    void clear_bit(uint32_t bit) noexcept {
        const size_t word = bit / 64u;
        const size_t pos = bit % 64u;
        bitmap_[word] &= ~(1ULL << pos);
    }

    uint32_t window_size_;
    uint64_t max_time_skew_ms_;
    uint32_t max_seq_ = 0u;
    bool initialized_ = false;
    std::vector<uint64_t> bitmap_;
};

// ============================================================================
// Tidal Protocol Frame Serialization & Cryptography
// ============================================================================

class TidalProtocol {
public:
    // Serializes metrics, encrypts with AEAD, and packages into a wire packet
    static std::vector<uint8_t> pack_telemetry(
        const TidalMetrics &metrics,
        const sodium::AuthenticatedEncryption &crypto,
        const sodium::DigitalSignature *signer = nullptr) {

        const size_t raw_size = sizeof(TidalMetrics);
        std::vector<uint8_t> payload(raw_size);
        std::memcpy(payload.data(), &metrics, raw_size);

        std::vector<uint8_t> out;
        // 1. Magic (2 bytes) + Version (1 byte) + MsgType (2 bytes)
        out.push_back(static_cast<uint8_t>((TIDAL_MAGIC_BYTES >> 8u) & 0xFFu));
        out.push_back(static_cast<uint8_t>(TIDAL_MAGIC_BYTES & 0xFFu));
        out.push_back(TIDAL_PROTOCOL_VERSION);

        const uint16_t msg_type = static_cast<uint16_t>(MessageType::Telemetry);
        out.push_back(static_cast<uint8_t>((msg_type >> 8u) & 0xFFu));
        out.push_back(static_cast<uint8_t>(msg_type & 0xFFu));

        // 2. Optional Ed25519 signature over plaintext payload
        const bool has_sig = (signer != nullptr);
        out.push_back(has_sig ? 1u : 0u);

        std::vector<uint8_t> plain_to_encrypt;
        if (has_sig) {
            auto sig = signer->sign(payload);
            plain_to_encrypt.insert(plain_to_encrypt.end(), sig.begin(), sig.end());
        }
        plain_to_encrypt.insert(plain_to_encrypt.end(), payload.begin(), payload.end());

        // 3. Encrypt payload + optional signature
        auto encrypted = crypto.encrypt(plain_to_encrypt);

        // 4. Encrypted payload length (uint32)
        uint32_t enc_len = static_cast<uint32_t>(encrypted.size());
        out.push_back(static_cast<uint8_t>((enc_len >> 24u) & 0xFFu));
        out.push_back(static_cast<uint8_t>((enc_len >> 16u) & 0xFFu));
        out.push_back(static_cast<uint8_t>((enc_len >> 8u) & 0xFFu));
        out.push_back(static_cast<uint8_t>(enc_len & 0xFFu));

        out.insert(out.end(), encrypted.begin(), encrypted.end());
        return out;
    }

    // Unpacks, decrypts, and validates an incoming tidal wire packet
    static TidalMetrics unpack_telemetry(
        std::span<const uint8_t> wire_packet,
        const sodium::AuthenticatedEncryption &crypto,
        AntiReplayFilter *anti_replay = nullptr,
        const sodium::VerifyKey *trusted_verify_key = nullptr) {

        constexpr size_t kHeaderSize = 2u + 1u + 2u + 1u + 4u; // 10 bytes
        if (wire_packet.size() < kHeaderSize) {
            throw std::invalid_argument("Tidal packet too short");
        }

        uint16_t magic = (static_cast<uint16_t>(wire_packet[0]) << 8u) | wire_packet[1];
        if (magic != TIDAL_MAGIC_BYTES) {
            throw std::runtime_error("Invalid Tidal magic identifier");
        }

        uint8_t version = wire_packet[2];
        if (version != TIDAL_PROTOCOL_VERSION) {
            throw std::runtime_error("Unsupported Tidal protocol version");
        }

        uint16_t msg_type = (static_cast<uint16_t>(wire_packet[3]) << 8u) | wire_packet[4];
        if (msg_type != static_cast<uint16_t>(MessageType::Telemetry)) {
            throw std::runtime_error("Unexpected Tidal message type");
        }

        bool has_sig = (wire_packet[5] == 1u);

        uint32_t enc_len = (static_cast<uint32_t>(wire_packet[6]) << 24u) |
                           (static_cast<uint32_t>(wire_packet[7]) << 16u) |
                           (static_cast<uint32_t>(wire_packet[8]) << 8u) |
                            static_cast<uint32_t>(wire_packet[9]);

        if (wire_packet.size() < kHeaderSize + enc_len) {
            throw std::invalid_argument("Incomplete Tidal packet payload");
        }

        std::span<const uint8_t> cipher_span = wire_packet.subspan(kHeaderSize, enc_len);
        auto decrypted = crypto.decrypt(cipher_span);

        size_t offset = 0u;
        if (has_sig) {
            if (decrypted.size() < sodium::MITHRIL_SIGN_SIGNATURE_BYTES + sizeof(TidalMetrics)) {
                throw std::runtime_error("Decrypted payload too small for signature + metrics");
            }

            sodium::Signature sig{};
            std::copy_n(decrypted.begin(), sig.size(), sig.begin());
            offset += sig.size();

            if (trusted_verify_key != nullptr) {
                std::span<const uint8_t> payload_span(decrypted.data() + offset, sizeof(TidalMetrics));
                sodium::DigitalSignature verifier;
                if (!verifier.verify(payload_span, sig, *trusted_verify_key)) {
                    throw std::runtime_error("Tidal telemetry signature verification failed");
                }
            }
        }

        if (decrypted.size() - offset < sizeof(TidalMetrics)) {
            throw std::runtime_error("Decrypted payload size mismatch");
        }

        TidalMetrics metrics;
        std::memcpy(&metrics, decrypted.data() + offset, sizeof(TidalMetrics));

        if (anti_replay != nullptr) {
            if (!anti_replay->check_and_update(metrics.sequence, metrics.timestamp_ms)) {
                throw std::runtime_error("Anti-replay violation: rejected duplicate or expired tidal packet");
            }
        }

        return metrics;
    }
};

} // namespace mithril::protocol::tidal

#endif // MITHRIL_TIDAL_SENSOR_PROTOCOL_HPP
