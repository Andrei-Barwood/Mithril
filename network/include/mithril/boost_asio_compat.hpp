#ifndef MITHRIL_BOOST_ASIO_COMPAT_HPP
#define MITHRIL_BOOST_ASIO_COMPAT_HPP

#include <boost/asio.hpp>

namespace mithril::network::compat {

// Returns the transport backend used by the in-repo compatibility layer.
const char *backend_name() noexcept;

} // namespace mithril::network::compat

#endif /* MITHRIL_BOOST_ASIO_COMPAT_HPP */
