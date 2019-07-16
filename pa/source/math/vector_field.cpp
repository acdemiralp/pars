#include <pa/math/vector_field.hpp>

#include <array>
#include <cmath>

#include <tbb/tbb.h>

#include <pa/math/linear_interpolate.hpp>

namespace pa
{
bool                          vector_field::contains   (const vector4& position) const
{
  for (auto i = 0; i < 3; ++i)
  {
    const auto subscript = std::floor((position[i] - offset[i]) / spacing[i]);
    if (0 > std::size_t(subscript) || std::size_t(subscript) >= data.shape()[i] - 1)
      return false;
  }
  return true;
}
vector3                       vector_field::interpolate(const vector4& position) const
{
  ivector3 multi_index;
  vector3  weights    ;
  for (auto i = 0; i < 3; ++i)
  {
    multi_index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
    weights    [i] = std::fmod ((position[i] - offset[i]) , spacing[i]) / spacing[i];
  }

  const auto& c000 = data(std::array<integer, 3>{multi_index[0]    , multi_index[1]    , multi_index[2]    });
  const auto& c001 = data(std::array<integer, 3>{multi_index[0]    , multi_index[1]    , multi_index[2] + 1});
  const auto& c010 = data(std::array<integer, 3>{multi_index[0]    , multi_index[1] + 1, multi_index[2]    });
  const auto& c011 = data(std::array<integer, 3>{multi_index[0]    , multi_index[1] + 1, multi_index[2] + 1});
  const auto& c100 = data(std::array<integer, 3>{multi_index[0] + 1, multi_index[1]    , multi_index[2]    });
  const auto& c101 = data(std::array<integer, 3>{multi_index[0] + 1, multi_index[1]    , multi_index[2] + 1});
  const auto& c110 = data(std::array<integer, 3>{multi_index[0] + 1, multi_index[1] + 1, multi_index[2]    });
  const auto& c111 = data(std::array<integer, 3>{multi_index[0] + 1, multi_index[1] + 1, multi_index[2] + 1});

  const auto  c00  = linear_interpolate(c000, c001, weights[2]);
  const auto  c01  = linear_interpolate(c010, c011, weights[2]);
  const auto  c10  = linear_interpolate(c100, c101, weights[2]);
  const auto  c11  = linear_interpolate(c110, c111, weights[2]);
  const auto  c0   = linear_interpolate(c00 , c01 , weights[1]);
  const auto  c1   = linear_interpolate(c10 , c11 , weights[1]);
  return             linear_interpolate(c0  , c1  , weights[0]);
}
std::unique_ptr<tensor_field> vector_field::gradient   ()
{
  auto tensor_field = std::make_unique<pa::tensor_field>();
  tensor_field->data.resize(boost::extents
   [data.shape()[0]]
   [data.shape()[1]]
   [data.shape()[2]]);
  tensor_field->offset  = offset ;
  tensor_field->size    = size   ;
  tensor_field->spacing = spacing;

  tbb::parallel_for(tbb::blocked_range3d<std::size_t>(0, data.shape()[0], 0, data.shape()[1], 0, data.shape()[2]), [&] (const tbb::blocked_range3d<std::size_t>& index) {
    for (auto x = index.pages().begin(), x_end = index.pages().end(); x < x_end; ++x) {
    for (auto y = index.rows ().begin(), y_end = index.rows ().end(); y < y_end; ++y) {
    for (auto z = index.cols ().begin(), z_end = index.cols ().end(); z < z_end; ++z) {
      auto& gradient          = tensor_field->data[x][y][z];

      vector3 initial_xp1_y_z = vector3(x + 1, y    , z    ).array() * tensor_field->spacing.array();
      vector3 initial_xm1_y_z = vector3(x - 1, y    , z    ).array() * tensor_field->spacing.array();
      vector3 initial_x_yp1_z = vector3(x    , y + 1, z    ).array() * tensor_field->spacing.array();
      vector3 initial_x_ym1_z = vector3(x    , y - 1, z    ).array() * tensor_field->spacing.array();
      vector3 initial_x_y_zp1 = vector3(x    , y    , z - 1).array() * tensor_field->spacing.array();
      vector3 initial_x_y_zm1 = vector3(x    , y    , z + 1).array() * tensor_field->spacing.array();
              
      vector3 final_xp1_y_z   = x + 1 < data.shape()[0] ? data[x + 1][y    ][z    ] : vector3(0, 0, 0);
      vector3 final_xm1_y_z   = x     > 0               ? data[x - 1][y    ][z    ] : vector3(0, 0, 0);
      vector3 final_x_yp1_z   = y + 1 < data.shape()[1] ? data[x    ][y + 1][z    ] : vector3(0, 0, 0);
      vector3 final_x_ym1_z   = y     > 0               ? data[x    ][y - 1][z    ] : vector3(0, 0, 0);
      vector3 final_x_y_zp1   = z + 1 < data.shape()[2] ? data[x    ][y    ][z + 1] : vector3(0, 0, 0);
      vector3 final_x_y_zm1   = z     > 0               ? data[x    ][y    ][z - 1] : vector3(0, 0, 0);

      gradient(0, 0)          = (final_xp1_y_z[0] - final_xm1_y_z[0]) / (initial_xp1_y_z[0] - initial_xm1_y_z[0]);
      gradient(0, 1)          = (final_x_yp1_z[0] - final_x_ym1_z[0]) / (initial_x_yp1_z[1] - initial_x_ym1_z[1]);
      gradient(0, 2)          = (final_x_y_zp1[0] - final_x_y_zm1[0]) / (initial_x_y_zp1[2] - initial_x_y_zm1[2]);
      gradient(1, 0)          = (final_xp1_y_z[1] - final_xm1_y_z[1]) / (initial_xp1_y_z[0] - initial_xm1_y_z[0]);
      gradient(1, 1)          = (final_x_yp1_z[1] - final_x_ym1_z[1]) / (initial_x_yp1_z[1] - initial_x_ym1_z[1]);
      gradient(1, 2)          = (final_x_y_zp1[1] - final_x_y_zm1[1]) / (initial_x_y_zp1[2] - initial_x_y_zm1[2]);
      gradient(3, 0)          = (final_xp1_y_z[2] - final_xm1_y_z[2]) / (initial_xp1_y_z[0] - initial_xm1_y_z[0]);
      gradient(3, 1)          = (final_x_yp1_z[2] - final_x_ym1_z[2]) / (initial_x_yp1_z[1] - initial_x_ym1_z[1]);
      gradient(3, 2)          = (final_x_y_zp1[2] - final_x_y_zm1[2]) / (initial_x_y_zp1[2] - initial_x_y_zm1[2]);
    }}}
  });

  return tensor_field;
}
}
