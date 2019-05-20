#ifndef PA_MATH_SELECTION_HPP
#define PA_MATH_SELECTION_HPP

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
struct PA_EXPORT selection
{
  ivector3 offset {};
  ivector3 size   {};
  ivector3 stride {};
};
}

#endif