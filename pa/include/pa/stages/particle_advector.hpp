#ifndef PA_STAGES_PARTICLE_ADVECTOR_HPP
#define PA_STAGES_PARTICLE_ADVECTOR_HPP

#include <array>
#include <optional>
#include <vector>

#include <tbb/tbb.h>

#include <pa/math/integrators.hpp>
#include <pa/math/particle.hpp>
#include <pa/math/types.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT particle_advector
{
public:
  using particle_map = tbb::concurrent_hash_map<integer, std::vector<particle>>; // TODO: Switch to an array of concurrent vectors and use partitioner indices for ranks.
  
  explicit particle_advector  (partitioner* partitioner);
  particle_advector           (const particle_advector&  that) = delete ;
  particle_advector           (      particle_advector&& temp) = delete ;
  virtual ~particle_advector  ()                               = default;
  particle_advector& operator=(const particle_advector&  that) = delete ;
  particle_advector& operator=(      particle_advector&& temp) = delete ;

  void         set_vector_field        (vector_field*             vector_field);
  void         set_integrator          (const variant_integrator& integrator  );
  void         set_step_size           (const scalar              step_size   );
                                                                              
  void         advect                  (std::vector<particle>&    particles   );

  // Advect sub-methods for separate benchmarking.
  particle_map create_neighborhood_map ();
  void         advect                  (      std::vector<particle>& active_particles, std::vector<particle>& inactive_particles,       particle_map& neighborhood_map);
  void         out_of_bounds_distribute(      std::vector<particle>& active_particles,                                            const particle_map& neighborhood_map);
  bool         check_completion        (const std::vector<particle>& active_particles);

protected:
  partitioner*       partitioner_  = nullptr;

  vector_field*      vector_field_ = nullptr;
  variant_integrator integrator_   = euler_integrator();
  scalar             step_size_    = 1.0f;
};
}

#endif