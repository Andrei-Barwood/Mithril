#include <mithril/boost_asio_compat.hpp>

#include <cstdlib>
#include <stdexcept>

#ifndef MITHRIL_CONSUMER_PATH_V2_DEFAULT
#define MITHRIL_CONSUMER_PATH_V2_DEFAULT 1
#endif

#ifndef MITHRIL_ENABLE_V1_COMPAT_PATH
#define MITHRIL_ENABLE_V1_COMPAT_PATH 0
#endif

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

std::string active_crypto_path() {
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
