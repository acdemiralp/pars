#include <pa/math/vector_field.hpp>

#include <array>
#include <cmath>

#include <pa/math/linear_interpolate.hpp>

namespace pa
{
bool    vector_field::contains   (const vector4& position) const
{
  for (auto i = 0; i < 3; ++i)
  {
    const auto subscript = std::floor(position[i] / spacing[i]);
    if (0.0 > subscript || subscript >= data.shape()[i] - 1)
      return false;
  }
  return true;
}
vector3 vector_field::interpolate(const vector4& position) const
{
  ivector3 multi_index;
  vector3  weights    ;
  for (auto i = 0; i < 3; ++i)
  {
    multi_index[i] = std::floor(position[i] / spacing[i]);
    weights    [i] = std::fmod (position[i] , spacing[i]) / spacing[i];
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
}
