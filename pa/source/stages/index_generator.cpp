#include <pa/stages/index_generator.hpp>

#include <algorithm>
#include <cstddef>

#include <tbb/tbb.h>

namespace pa
{
void index_generator::generate(integral_curves* integral_curves)
{
  auto& vertices = integral_curves->vertices;
  auto& indices  = integral_curves->indices ;

  indices.resize(vertices.size(), invalid_index);

  tbb::parallel_for(std::size_t(0), vertices.size() - 1, std::size_t(1), [&] (const std::size_t index)
  {
    if (vertices[index] != termination_vertex && vertices[index + 1] != termination_vertex)
      indices[index] = index;
  });
  
  indices.erase (std::remove(indices.begin(), indices.end(), invalid_index), indices.end());
}
}