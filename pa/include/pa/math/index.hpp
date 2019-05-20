#ifndef PA_MATH_INDEX_HPP
#define PA_MATH_INDEX_HPP

#include <cstdint>
#include <cstddef>

namespace pa
{
template <typename type>
type        unravel_index    (std::size_t index      , const type& dimensions)
{
  type subscripts(dimensions.size(), 0);
  for (std::int64_t i = dimensions.size() - 1; i >= 0; --i)
  {
    subscripts[i] = index % dimensions[i];
    index = index / dimensions[i];
  }
  return subscripts;
}
template <typename type>
std::size_t ravel_multi_index(const type& multi_index, const type& dimensions)
{
  std::size_t index(0);
  for (std::size_t i = 0; i < dimensions.size(); ++i)
    index = index * dimensions[i] + multi_index[i];
  return index;
}
}

#endif