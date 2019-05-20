#ifndef PA_STAGES_INDEX_GENERATOR_HPP
#define PA_STAGES_INDEX_GENERATOR_HPP

#include <pa/math/integral_curves.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT index_generator
{
public:
  static void generate(integral_curves* integral_curves);
};
}

#endif