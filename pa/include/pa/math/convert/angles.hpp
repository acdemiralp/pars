#ifndef PA_MATH_CONVERT_ANGLES_HPP
#define PA_MATH_CONVERT_ANGLES_HPP

#define _USE_MATH_DEFINES

#include <cmath>

namespace pa
{
template<typename type>
type to_radians(type degrees)
{
  return degrees * type(M_PI / 180.0);
}
template<typename type>
type to_degrees(type radians)
{
  return radians * type(180.0 / M_PI);
}
}

#endif