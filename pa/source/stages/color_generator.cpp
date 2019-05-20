#include <pa/stages/color_generator.hpp>

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <cstddef>

#include <boost/algorithm/clamp.hpp>
#include <tbb/tbb.h>

#include <pa/math/convert/coordinates.hpp>

namespace pa
{
void    color_generator::generate(integral_curves* integral_curves, mode mode, scalar constant_parameter)
{
  auto& vertices = integral_curves->vertices;
  auto& colors   = integral_curves->colors  ;
  auto& indices  = integral_curves->indices ;

  colors.resize(vertices.size());

  tbb::parallel_for(std::size_t(0), indices.size(), std::size_t(1), [&] (const std::size_t i)
  {
    auto& index   = indices[i];
    auto  tangent = (vertices[index + 1] - vertices[index]).normalized();
    
    if      (mode == mode::hsl_constant_s) colors[index] = map_hsl(tangent, true , constant_parameter);
    else if (mode == mode::hsl_constant_l) colors[index] = map_hsl(tangent, false, constant_parameter);
    else if (mode == mode::hsv_constant_s) colors[index] = map_hsv(tangent, true , constant_parameter);
    else if (mode == mode::hsv_constant_v) colors[index] = map_hsv(tangent, false, constant_parameter);
    else if (mode == mode::rgb           ) colors[index] = map_rgb(tangent);

    // Last vertex also gets a color.
    if (i != indices.size() - 1 && indices[i + 1] != index + 1)
      colors[index + 1] = colors[index];
  });
}

vector4 color_generator::correct_range  (vector4 spherical )
{
  if (spherical[1] <  scalar(0))          spherical[1] += scalar(M_PI);
  if (spherical[1] >= scalar(M_PI))       spherical[1] -= scalar(M_PI);
  spherical[1] = scalar(M_PI) - spherical[1];
  
  if (spherical[2] <  scalar(0))          spherical[2]  = std::abs(spherical[2]);
  if (spherical[2] >= scalar(M_PI / 2.0)) spherical[2]  = scalar(M_PI) - spherical[2];
    
  spherical[1] = scalar(spherical[1] / M_PI);
  spherical[2] = scalar(spherical[2] / (M_PI / 2.0f));
  
  return spherical;
}
vector4 color_generator::hue_to_rgba    (const scalar   hue)
{
  return vector4
  (
    static_cast<scalar>(boost::algorithm::clamp(std::abs(6.0f * hue - 3.0f) - 1.0f, 0.0f, 1.0f)),
    static_cast<scalar>(boost::algorithm::clamp(2.0f - std::abs(6.0f * hue - 2.0f), 0.0f, 1.0f)),
    static_cast<scalar>(boost::algorithm::clamp(2.0f - std::abs(6.0f * hue - 4.0f), 0.0f, 1.0f)),
    1.0f
  );
}
vector4 color_generator::hsl_to_rgba    (const vector4& hsl)
{
  auto color      = hue_to_rgba(hsl[0]);
  auto precompute = (1.0F - std::abs(2.0F * hsl[2] - 1.0F)) * hsl[1];
  color           = (color.array() - 0.5F) * precompute + hsl[2];
  color[3]        = 1.0f;
  return color;
}
vector4 color_generator::hsv_to_rgba    (const vector4& hsv)
{
  auto color = hue_to_rgba(hsv[0]);
  color      = ((color.array() - 1.0f) * hsv[1] + 1.0f) * hsv[2];
  color[3]   = 1.0f;
  return color;
}
  
vector4 color_generator::map_hsl        (const vector4& tangent, const bool constant_saturation = true, const scalar constant_parameter = scalar(0.5))
{
  auto spherical = correct_range(to_spherical(tangent));
  return constant_saturation 
    ? hsl_to_rgba(vector4(spherical[1], constant_parameter, spherical[2]      , 0.0f))
    : hsl_to_rgba(vector4(spherical[1], spherical[2]      , constant_parameter, 0.0f));
}
vector4 color_generator::map_hsv        (const vector4& tangent, const bool constant_saturation = true, const scalar constant_parameter = scalar(0.5))
{
  auto spherical = correct_range(to_spherical(tangent));
  return constant_saturation 
    ? hsv_to_rgba(vector4(spherical[1], constant_parameter, spherical[2]      , 0.0f)) 
    : hsv_to_rgba(vector4(spherical[1], spherical[2]      , constant_parameter, 0.0f));
}
vector4 color_generator::map_rgb        (const vector4& tangent)
{
  return vector4(std::abs(tangent[0]), std::abs(tangent[2]), std::abs(tangent[1]), 1.0f);
}
}