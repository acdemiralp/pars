#ifndef PA_MATH_INTEGRAL_CURVES_HPP
#define PA_MATH_INTEGRAL_CURVES_HPP

#include <vector>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
struct PA_EXPORT integral_curves
{
  std::vector<vector4> vertices {};
  std::vector<vector4> colors   {};
  std::vector<integer> indices  {};
};

inline const vector4 termination_vertex = vector4(-2.0f, -2.0f, -2.0f, 0.0f);
inline const vector4 invalid_vertex     = vector4(-1.0f, -1.0f, -1.0f, 0.0f);
inline const integer invalid_index      = -1;
}

#endif