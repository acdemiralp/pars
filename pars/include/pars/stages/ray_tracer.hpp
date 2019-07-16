#ifndef PARS_STAGES_RAY_TRACER_HPP
#define PARS_STAGES_RAY_TRACER_HPP

#define NOMINMAX
#undef min
#undef max

#include <cstddef>
#include <memory>
#include <vector>

#include <boost/mpi.hpp>
#include <ospray/ospray_cpp.h>
#include <pa/math/integral_curves.hpp>
#include <pa/math/scalar_field.hpp>
#include <pa/math/types.hpp>
#include <pa/stages/partitioner.hpp>

#include <pars/export.hpp>

#include <image.pb.h>

namespace pars
{
class PARS_EXPORT ray_tracer
{
public:
  explicit ray_tracer  (pa::partitioner* partitioner, const std::size_t thread_count);
  ray_tracer           (const ray_tracer&  that) = delete ;
  ray_tracer           (      ray_tracer&& temp) = default;
  virtual ~ray_tracer  ();
  ray_tracer& operator=(const ray_tracer&  that) = delete ;
  ray_tracer& operator=(      ray_tracer&& temp) = default;

  void  set_volume         (pa::scalar_field* scalar_field);
  void  set_integral_curves(std::vector<pa::integral_curves>* integral_curves, float radius);
  void  set_camera         (const pa::vector3& position, const pa::vector3& forward, const pa::vector3& up);
  void  set_image_size     (const pa::ivector2& image_size);

  void  trace              (const std::size_t iterations = 1);
  image serialize          ();

protected:
  pa::partitioner*                                        partitioner_          ;
  MPI_Comm                                                raw_communicator_     ;
  
  std::unique_ptr<ospray::cpp::Renderer>                  renderer_             ;
  std::unique_ptr<ospray::cpp::Camera>                    camera_               ;
  std::unique_ptr<ospray::cpp::Model>                     model_                ;

  std::vector    <std::unique_ptr<ospray::cpp::Geometry>> geometry_             ;
  std::vector    <std::unique_ptr<ospray::cpp::Data>>     vertex_data_          ;
  std::vector    <std::unique_ptr<ospray::cpp::Data>>     color_data_           ;
  std::vector    <std::unique_ptr<ospray::cpp::Data>>     index_data_           ;

  std::unique_ptr<ospray::cpp::Volume>                    volume_               ;
  std::unique_ptr<ospray::cpp::Data>                      volume_data_          ;
  std::unique_ptr<ospray::cpp::TransferFunction>          transfer_function_    ;
  std::unique_ptr<ospray::cpp::Data>                      transfer_color_data_  ;
  std::unique_ptr<ospray::cpp::Data>                      transfer_opacity_data_;
  
  std::unique_ptr<ospray::cpp::Data>                      lights_               ;
  
  std::unique_ptr<ospray::cpp::FrameBuffer>               framebuffer_          ;
  pa::ivector2                                            framebuffer_size_ {}  ;
};
}

#endif