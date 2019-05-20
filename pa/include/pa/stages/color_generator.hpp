#ifndef PA_STAGES_COLOR_GENERATOR_HPP
#define PA_STAGES_COLOR_GENERATOR_HPP

#include <pa/math/integral_curves.hpp>
#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT color_generator
{
public:
  enum class mode
  {
    hsl_constant_s,
    hsl_constant_l,
    hsv_constant_s,
    hsv_constant_v,
    rgb
  };

  static void    generate       (integral_curves* integral_curves, mode mode = mode::rgb, scalar constant_parameter = 0.5f);

protected:
  static vector4 correct_range  (vector4 spherical );
  static vector4 hue_to_rgba    (const scalar   hue);
  static vector4 hsl_to_rgba    (const vector4& hsl);
  static vector4 hsv_to_rgba    (const vector4& hsv);
  static vector4 map_hsl        (const vector4& tangent, const bool constant_saturation, const scalar constant_parameter);
  static vector4 map_hsv        (const vector4& tangent, const bool constant_saturation, const scalar constant_parameter);
  static vector4 map_rgb        (const vector4& tangent);
};
}

#endif