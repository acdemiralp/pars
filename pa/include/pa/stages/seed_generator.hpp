#ifndef PA_STAGES_SEED_GENERATOR_HPP
#define PA_STAGES_SEED_GENERATOR_HPP

#include <pa/math/particle.hpp>
#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT seed_generator
{
public:
  static std::vector<particle> generate(const ivector3& size, const ivector3& stride, const vector3& spacing, integer remaining_iterations);
};
}

#endif