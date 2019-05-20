#ifndef PA_MATH_VECTOR_FIELD_HPP
#define PA_MATH_VECTOR_FIELD_HPP

#include <boost/multi_array.hpp>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
struct PA_EXPORT vector_field
{
  bool    contains   (const vector4& position) const;
  vector3 interpolate(const vector4& position) const;

  boost::multi_array<vector3, 3> data    {};
  vector3                        offset  {};
  vector3                        size    {};
  vector3                        spacing {};
};
}


#endif