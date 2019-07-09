#include <pa/stages/seed_generator.hpp>

#include <tbb/tbb.h>

#include <pa/math/index.hpp>

namespace pa
{
std::vector<particle> seed_generator::generate(const vector3& offset, const vector3& size, const vector3& stride, integer remaining_iterations, integer rank)
{
  ivector3 particles_per_dimension = (size.array() / stride.array()).cast<integer>();

  std::vector<particle> particles(particles_per_dimension.prod());
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    ivector3 multi_index = unravel_index(index, particles_per_dimension);
    vector3  position    = offset.array() + stride.array() * multi_index.cast<scalar>().array();
    particles[index]     = particle {vector4(position[0], position[1], position[2], 0), remaining_iterations, -1, rank, multi_index};
  });
  return particles;
}
}