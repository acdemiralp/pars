#ifndef PA_STAGES_PARTICLE_TRACER_HPP
#define PA_STAGES_PARTICLE_TRACER_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <tbb/tbb.h>

#include <pa/math/integral_curves.hpp>
#include <pa/math/integrators.hpp>
#include <pa/math/particle.hpp>
#include <pa/math/types.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT particle_tracer
{
public:
  struct PA_EXPORT round_info
  {
    using particle_map = tbb::concurrent_hash_map<integer, std::vector<particle>>; // TODO: Switch to an array of concurrent vectors and use partitioner indices for ranks.

    std::size_t  maximum_remaining_iterations    ;
    std::size_t  vertices_per_integral_curve     ;
    std::size_t  vertices_per_integral_curve_last;
    std::size_t  integral_curve_offset           ;
    std::size_t  integral_curve_count            ;
    particle_map out_of_bounds_particles         ;
    particle_map neighbor_out_of_bounds_particles;
  };

  explicit particle_tracer  (partitioner* partitioner);
  particle_tracer           (const particle_tracer&  that) = delete ;
  particle_tracer           (      particle_tracer&& temp) = delete ;
  virtual ~particle_tracer  ()                             = default;
  particle_tracer& operator=(const particle_tracer&  that) = delete ;
  particle_tracer& operator=(      particle_tracer&& temp) = delete ;

  void                         set_local_vector_field    (std::optional<vector_field>*                local_vector_field    );
  void                         set_neighbor_vector_fields(std::array<std::optional<vector_field>, 6>* neighbor_vector_fields);
  void                         set_integrator            (const variant_integrator&                   integrator            );
  void                         set_step_size             (const scalar                                step_size             );
                                                       
  std::vector<integral_curves> trace                     (std::vector<particle>                       particles             ); 

  // Trace sub-methods for separate benchmarking.
  void                         load_balance_distribute   (      std::vector<particle>& particles                                                                                   );
  round_info                   compute_round_info        (const std::vector<particle>& particles, const std::vector<integral_curves>& integral_curves                              );
  void                         allocate                  (                                              std::vector<integral_curves>& integral_curves, const round_info& round_info);
  void                         initialize                (const std::vector<particle>& particles,       std::vector<integral_curves>& integral_curves, const round_info& round_info);
  void                         trace                     (const std::vector<particle>& particles,       std::vector<integral_curves>& integral_curves,       round_info& round_info);
  void                         load_balance_collect      (                                                                                             const round_info& round_info);
  void                         out_of_bounds_distribute  (      std::vector<particle>& particles,                                                      const round_info& round_info);
  bool                         check_completion          (const std::vector<particle>& particles                                                                                   );
  void                         prune                     (                                              std::vector<integral_curves>& integral_curves                              );

protected:
  partitioner*                                partitioner_            = nullptr;

  std::optional<vector_field>*                local_vector_field_     = {};
  std::array<std::optional<vector_field>, 6>* neighbor_vector_fields_ = {};
  variant_integrator                          integrator_             = euler_integrator();
  scalar                                      step_size_              = 1.0f;
};
}

#endif