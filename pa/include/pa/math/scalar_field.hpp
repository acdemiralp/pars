#ifndef PA_MATH_SCALAR_FIELD_HPP
#define PA_MATH_SCALAR_FIELD_HPP

#include <boost/multi_array.hpp>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
struct PA_EXPORT scalar_field
{
  boost::multi_array<scalar, 3> data    {};
  vector3                       offset  {};
  vector3                       size    {};
  vector3                       spacing {};
};
}

#endif