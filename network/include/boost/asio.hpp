#ifndef MITHRIL_BOOST_ASIO_SHIM_HPP
#define MITHRIL_BOOST_ASIO_SHIM_HPP

#include <asio.hpp>

namespace boost {
namespace asio = ::asio;
namespace system {
using error_code = ::asio::error_code;
using system_error = ::asio::system_error;
}
}

#endif /* MITHRIL_BOOST_ASIO_SHIM_HPP */
