#include <mithril/boost_asio_compat.hpp>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef MITHRIL_CONSUMER_PATH_V2_DEFAULT
#define MITHRIL_CONSUMER_PATH_V2_DEFAULT 1
#endif

#ifndef MITHRIL_ENABLE_V1_COMPAT_PATH
#define MITHRIL_ENABLE_V1_COMPAT_PATH 0
#endif

namespace {

enum class rollout_mode {
    full,
    canary,
    rollback,
};

struct rollout_decision {
    rollout_mode mode;
    int canary_percent;
    std::string canary_key;
    uint32_t bucket;
    bool enabled;
};

const char *mode_name(rollout_mode mode) noexcept {
    switch (mode) {
        case rollout_mode::full:
            return "full";
        case rollout_mode::canary:
            return "canary";
        case rollout_mode::rollback:
            return "rollback";
    }
    return "unknown";
}

std::string to_lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

rollout_mode parse_rollout_mode() {
    const char *raw = std::getenv("MITHRIL_V2_ROLLOUT_MODE");
    if (raw == nullptr || raw[0] == '\0') {
        return rollout_mode::full;
    }

    const std::string mode = to_lower_ascii(raw);
    if (mode == "full" || mode == "stable" || mode == "on") {
        return rollout_mode::full;
    }
    if (mode == "canary") {
        return rollout_mode::canary;
    }
    if (mode == "rollback" || mode == "off") {
        return rollout_mode::rollback;
    }

    throw std::runtime_error(
        "Invalid MITHRIL_V2_ROLLOUT_MODE. Allowed values: full, canary, rollback.");
}

int parse_canary_percent() {
    const char *raw = std::getenv("MITHRIL_V2_CANARY_PERCENT");
    if (raw == nullptr || raw[0] == '\0') {
        return 100;
    }

    char *end = nullptr;
    errno = 0;
    const long value = std::strtol(raw, &end, 10);
    if (errno != 0 || end == raw || *end != '\0' || value < 0L || value > 100L) {
        throw std::runtime_error(
            "Invalid MITHRIL_V2_CANARY_PERCENT. Expected integer in [0,100].");
    }

    return static_cast<int>(value);
}

std::string resolve_canary_key() {
    const char *raw = std::getenv("MITHRIL_V2_CANARY_KEY");
    if (raw != nullptr && raw[0] != '\0') {
        return std::string(raw);
    }

    raw = std::getenv("HOSTNAME");
    if (raw != nullptr && raw[0] != '\0') {
        return std::string(raw);
    }

    return "default";
}

uint32_t fnv1a_bucket_0_to_99(std::string_view key) noexcept {
    uint32_t hash = 2166136261u;
    for (const unsigned char c : key) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 16777619u;
    }
    return hash % 100u;
}

rollout_decision evaluate_rollout_decision() {
    rollout_decision decision{
        .mode = parse_rollout_mode(),
        .canary_percent = parse_canary_percent(),
        .canary_key = resolve_canary_key(),
        .bucket = 0u,
        .enabled = true,
    };

    if (decision.mode == rollout_mode::full) {
        decision.enabled = true;
        return decision;
    }

    if (decision.mode == rollout_mode::rollback) {
        decision.enabled = false;
        return decision;
    }

    decision.bucket = fnv1a_bucket_0_to_99(decision.canary_key);
    decision.enabled = (decision.bucket < static_cast<uint32_t>(decision.canary_percent));
    return decision;
}

} // namespace

namespace mithril::network::compat {

const char *backend_name() noexcept {
    return "standalone-asio-via-boost-shim";
}

const char *default_crypto_path() noexcept {
#if MITHRIL_CONSUMER_PATH_V2_DEFAULT
    return "v2";
#else
    return "v1";
#endif
}

bool v1_compat_path_enabled() noexcept {
#if MITHRIL_ENABLE_V1_COMPAT_PATH
    return true;
#else
    return false;
#endif
}

std::string rollout_summary() {
    const rollout_decision decision = evaluate_rollout_decision();
    std::ostringstream oss;
    oss << "mode=" << mode_name(decision.mode);
    if (decision.mode == rollout_mode::canary) {
        oss << ", percent=" << decision.canary_percent;
        oss << ", bucket=" << decision.bucket;
        oss << ", key=" << decision.canary_key;
    }
    oss << ", decision=" << (decision.enabled ? "enabled" : "blocked");
    return oss.str();
}

void enforce_rollout_policy() {
    const rollout_decision decision = evaluate_rollout_decision();
    if (decision.enabled) {
        return;
    }

    std::ostringstream oss;
    oss << "MITHRIL v2 rollout policy blocks this process";
    oss << " (mode=" << mode_name(decision.mode);
    if (decision.mode == rollout_mode::canary) {
        oss << ", percent=" << decision.canary_percent;
        oss << ", bucket=" << decision.bucket;
        oss << ", key=" << decision.canary_key;
    }
    oss << "). Set MITHRIL_V2_ROLLOUT_MODE=full to allow v2, ";
    oss << "or adjust canary configuration.";
    throw std::runtime_error(oss.str());
}

std::string active_crypto_path() {
    enforce_rollout_policy();

    const char *requested = std::getenv("MITHRIL_CRYPTO_PATH");

    if (requested == nullptr || requested[0] == '\0') {
        return default_crypto_path();
    }

    std::string path(requested);
    if (path == "v2") {
        return "v2";
    }
    if (path == "v1" || path == "legacy") {
        if (!v1_compat_path_enabled()) {
            throw std::runtime_error(
                "MITHRIL_CRYPTO_PATH requests v1/legacy but v1 path is disabled. "
                "Use MITHRIL_CRYPTO_PATH=v2 or enable MITHRIL_ENABLE_V1_COMPAT_PATH.");
        }
        return "v1";
    }

    throw std::runtime_error("Invalid MITHRIL_CRYPTO_PATH. Allowed values: v2, v1, legacy.");
}

void enforce_supported_crypto_path() {
    const std::string selected = active_crypto_path();
    if (selected != "v2") {
        throw std::runtime_error(
            "Selected crypto path is not implemented in this build. v2 is the supported path.");
    }
}

} // namespace mithril::network::compat
