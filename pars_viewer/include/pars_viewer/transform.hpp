#ifndef PARS_VIEWER_TRANSFORM_HPP
#define PARS_VIEWER_TRANSFORM_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace pars
{
class transform
{
public:
  explicit transform  ();
  transform           (const transform&  that) = default;
  transform           (      transform&& temp) = default;
  virtual ~transform  ()                       = default;
  transform& operator=(const transform&  that) = default;
  transform& operator=(      transform&& temp) = default;

  Eigen::Vector3f    translation       () const;
  Eigen::Quaternionf rotation          () const;
  Eigen::Vector3f    rotation_euler    () const;
  Eigen::Vector3f    scale             () const;
                              
  Eigen::Vector3f    right             () const;
  Eigen::Vector3f    up                () const;
  Eigen::Vector3f    forward           () const;
            
  void               set_translation   (const Eigen::Vector3f   & translation);
  void               set_rotation      (const Eigen::Quaternionf& rotation   );
  void               set_rotation_euler(const Eigen::Vector3f   & rotation   );
  void               set_scale         (const Eigen::Vector3f   & scale      );
                                                                    
  void               translate         (const Eigen::Vector3f   & value      );
  void               rotate            (const Eigen::Quaternionf& value      , const bool postmultiply = false);
  void               rotate_euler      (const Eigen::Vector3f   & value      , const bool postmultiply = false);
  void               scale             (const Eigen::Vector3f   & value      );
  void               look_at           (const Eigen::Vector3f   & target     , const Eigen::Vector3f& up = Eigen::Vector3f(0.0f, 1.0f, 0.0f));
  void               reset             ();
            
protected: 
  Eigen::Vector3f    translation_;
  Eigen::Quaternionf rotation_   ;
  Eigen::Vector3f    scale_      ;
};
}

#endif