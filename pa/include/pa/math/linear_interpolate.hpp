#ifndef PA_MATH_INTERPOLATE_HPP
#define PA_MATH_INTERPOLATE_HPP

namespace pa
{
template <typename data_type, typename weight_type>
data_type linear_interpolate(const data_type& x, const data_type& y, const weight_type weight)
{
  return (weight_type(1) - weight) * x + weight * y;
}
}

#endif