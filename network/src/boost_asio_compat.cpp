#include <mithril/boost_asio_compat.hpp>

namespace mithril::network::compat {

const char *backend_name() noexcept {
    return "standalone-asio-via-boost-shim";
}

} // namespace mithril::network::compat
