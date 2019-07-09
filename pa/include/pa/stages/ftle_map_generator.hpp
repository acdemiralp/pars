#ifndef PA_STAGES_FTLE_MAP_GENERATOR_HPP
#define PA_STAGES_FTLE_MAP_GENERATOR_HPP

#include <memory>

#include <pa/math/scalar_field.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT ftle_map_generator
{
public:
  explicit ftle_map_generator  (partitioner* partitioner);
  ftle_map_generator           (const ftle_map_generator&  that) = delete ;
  ftle_map_generator           (      ftle_map_generator&& temp) = delete ;
  virtual ~ftle_map_generator  ()                                = default;
  ftle_map_generator& operator=(const ftle_map_generator&  that) = delete ;
  ftle_map_generator& operator=(      ftle_map_generator&& temp) = delete ;

  void                          set_flow_map(vector_field* vector_field);
  void                          set_time    (scalar        time        );

  std::unique_ptr<scalar_field> generate    ();

protected:
  partitioner*  partitioner_ = nullptr;

  vector_field* flow_map_    = nullptr;
  scalar        time_        = 1;
};
}

#endif