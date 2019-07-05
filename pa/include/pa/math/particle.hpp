#ifndef PA_MATH_PARTICLE_HPP
#define PA_MATH_PARTICLE_HPP

#include <cstddef>
#include <cstdint>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
// Type used for exchanging particles among MPI processes and for initial seed generation.
struct PA_EXPORT particle
{
  // Function for boost::serialization which is used by boost::mpi.
  template<class archive_type>
  void serialize(archive_type& archive, const std::uint32_t version)
  {
    archive & position            [0];
    archive & position            [1];
    archive & position            [2];
    archive & position            [3];
    archive & remaining_iterations   ;
    archive & vector_field_index     ;
    archive & original_rank          ;
    archive & original_voxel      [0];
    archive & original_voxel      [1];
    archive & original_voxel      [2];
  }

  vector4  position             = {};
  integer  remaining_iterations = 0 ;
  integer  vector_field_index   = 0 ;
  
  integer  original_rank        = 0 ;
  ivector3 original_voxel       = {};
};
}

#endif