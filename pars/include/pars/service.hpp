#ifndef PARS_SERVICE_HPP_
#define PARS_SERVICE_HPP_

#include <cstddef>
#include <string>

#include <pars/export.hpp>

namespace pars
{
PARS_EXPORT void run      (const std::string& address);
PARS_EXPORT void benchmark(const std::size_t thread_count, const std::string& settings_filepath);
}

#endif
