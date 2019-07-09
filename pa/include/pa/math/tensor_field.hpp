#ifndef PA_MATH_TENSOR_FIELD_HPP
#define PA_MATH_TENSOR_FIELD_HPP

#include <boost/multi_array.hpp>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
struct PA_EXPORT tensor_field
{
  void for_each     (const std::function<void(matrix3&)>& function);
  void spectral_norm();

  boost::multi_array<matrix3, 3> data    {};
  vector3                        offset  {};
  vector3                        size    {};
  vector3                        spacing {};
};
}

#endif