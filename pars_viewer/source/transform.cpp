#include <pars_viewer/transform.hpp>

namespace pars
{
namespace detail
{
Eigen::Matrix3f look_at(const Eigen::Vector3f& eye, const Eigen::Vector3f& forward, const Eigen::Vector3f& up)
{
  Eigen::Vector3f right   = up.normalized().cross(forward.normalized()).normalized();
  Eigen::Vector3f true_up = forward.normalized().cross(right);
  Eigen::Matrix3f matrix;
  matrix << 
   right  .x(), right  .y(), right  .z(),
   true_up.x(), true_up.y(), true_up.z(),
   forward.x(), forward.y(), forward.z();
  return matrix;
}
}

transform::transform() 
: translation_(Eigen::Vector3f   ::Zero    ())
, rotation_   (Eigen::Quaternionf::Identity())
, scale_      (Eigen::Vector3f   ::Ones    ())
{

}

Eigen::Vector3f    transform::translation       () const
{
  return translation_;
}
Eigen::Quaternionf transform::rotation          () const
{
  return rotation_;
}
Eigen::Vector3f    transform::rotation_euler    () const
{
  return rotation_.toRotationMatrix().eulerAngles(2, 0, 1);
}
Eigen::Vector3f    transform::scale             () const
{
  return scale_;
}
         
Eigen::Vector3f    transform::right             () const
{
  return rotation()._transformVector(Eigen::Vector3f::UnitX());
}
Eigen::Vector3f    transform::up                () const
{
  return rotation()._transformVector(Eigen::Vector3f::UnitY());
}
Eigen::Vector3f    transform::forward           () const
{
  return rotation()._transformVector(Eigen::Vector3f::UnitZ());
}
          
void               transform::set_translation   (const Eigen::Vector3f   & translation                      )
{
  translation_ = translation;
}
void               transform::set_rotation      (const Eigen::Quaternionf& rotation                         )
{
  rotation_ = rotation.derived();
}
void               transform::set_rotation_euler(const Eigen::Vector3f   & rotation                         )
{
  set_rotation(
    Eigen::AngleAxisf(rotation[2], Eigen::Vector3f::UnitZ()) *
    Eigen::AngleAxisf(rotation[0], Eigen::Vector3f::UnitX()) *
    Eigen::AngleAxisf(rotation[1], Eigen::Vector3f::UnitY()));
}
void               transform::set_scale         (const Eigen::Vector3f   & scale                            )
{
  scale_ = scale;
}
          
void               transform::translate         (const Eigen::Vector3f   & value                            )
{
  set_translation(value + translation());
}
void               transform::rotate            (const Eigen::Quaternionf& value                            , const bool postmultiply)
{
  set_rotation   (postmultiply ? rotation() * value : value * rotation());
}
void               transform::rotate_euler      (const Eigen::Vector3f   & value                            , const bool postmultiply)
{
  rotate(
    Eigen::AngleAxisf(value[2], Eigen::Vector3f::UnitZ()) *
    Eigen::AngleAxisf(value[0], Eigen::Vector3f::UnitX()) *
    Eigen::AngleAxisf(value[1], Eigen::Vector3f::UnitY()), postmultiply);
}
void               transform::scale             (const Eigen::Vector3f   & value                            )
{
  set_scale      (value.array() * scale().array());
}
void               transform::look_at           (const Eigen::Vector3f   & forward, const Eigen::Vector3f& up)
{
  set_rotation   (Eigen::Quaternionf(detail::look_at(translation(), forward, up)));
}
void               transform::reset             ()
{
  translation_ = Eigen::Vector3f   ::Zero    ();
  rotation_    = Eigen::Quaternionf::Identity();
  scale_       = Eigen::Vector3f   ::Ones    ();
}
}