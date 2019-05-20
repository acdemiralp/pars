#include <pa/stages/seed_generator.hpp>

#include <tbb/tbb.h>

#include <pa/math/index.hpp>

namespace pa
{
std::vector<particle> seed_generator::generate(const ivector3& size, const ivector3& stride, const vector3& spacing, integer remaining_iterations)
{
  ivector3 particles_size  = size.array() / stride.array();
  integer  particles_count = particles_size.prod();
  
  std::vector<particle> particles(particles_count);
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    ivector3 multi_index = unravel_index(index, particles_size);
    vector3  position    = (multi_index.array() * stride.array()).cast<float>().array() * spacing.array();
    particles[index] = particle {vector4(position[0], position[1], position[2], 0), remaining_iterations, -1};
  });
  return particles;
}
}