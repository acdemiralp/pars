#include <pa/math/tensor_field.hpp>

#include <tbb/tbb.h>

namespace pa
{
void tensor_field::for_each     (const std::function<void(matrix3&)>& function)
{
  tbb::parallel_for(tbb::blocked_range3d<std::size_t>(0, data.shape()[0], 0, data.shape()[1], 0, data.shape()[2]), [&] (const tbb::blocked_range3d<std::size_t>& index) {
    for (auto x = index.pages().begin(), x_end = index.pages().end(); x < x_end; ++x) {
    for (auto y = index.rows ().begin(), y_end = index.rows ().end(); y < y_end; ++y) {
    for (auto z = index.cols ().begin(), z_end = index.cols ().end(); z < z_end; ++z) {
      function(data[x][y][z]);
  }}}});
}
void tensor_field::spectral_norm()
{
  for_each([ ] (matrix3& voxel)
  {
    voxel = voxel.transpose() * voxel;
  });
}
}
