#ifndef MITHRIL_BOOST_ASIO_COMPAT_HPP
#define MITHRIL_BOOST_ASIO_COMPAT_HPP

#include <boost/asio.hpp>
#include <string>

namespace mithril::network::compat {

// Returns the transport backend used by the in-repo compatibility layer.
const char *backend_name() noexcept;
const char *default_crypto_path() noexcept;
bool v1_compat_path_enabled() noexcept;
std::string rollout_summary();
void enforce_rollout_policy();
std::string active_crypto_path();
void enforce_supported_crypto_path();

} // namespace mithril::network::compat

#endif /* MITHRIL_BOOST_ASIO_COMPAT_HPP */
