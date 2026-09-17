// Compatibility shim for Boost.Asio >= 1.87 where io_service.hpp was removed.
#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/post.hpp>

namespace boost {
namespace asio {

using io_service = io_context;

} // namespace asio
} // namespace boost
