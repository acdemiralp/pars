#ifndef PA_STAGES_PARTITIONER_HPP
#define PA_STAGES_PARTITIONER_HPP

#include <array>
#include <optional>

#include <boost/mpi.hpp>

#include <pa/math/types.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT partitioner
{
public:
  struct PA_EXPORT rank_info
  {
    rank_info(integer rank, const ivector3& grid_size, const ivector3& block_size);

    integer  rank               = 0 ;
    ivector3 multi_rank         = {};
    ivector3 offset             = {};
    ivector3 ghosted_block_size = {};
  };

  explicit partitioner  (boost::mpi::communicator* communicator);
  partitioner           (const partitioner&  that) = default;
  partitioner           (      partitioner&& temp) = default;
 ~partitioner           ()                         = default;
  partitioner& operator=(const partitioner&  that) = default;
  partitioner& operator=(      partitioner&& temp) = default;

  void                                           set_domain_size   (const ivector3& domain_size);

  boost::mpi::communicator*                      communicator      ();
  const ivector3&                                domain_size       () const;
  const ivector3&                                grid_size         () const;
  const ivector3&                                block_size        () const;

  const std::optional<rank_info>&                local_rank_info   () const;
  const std::array<std::optional<rank_info>, 6>& neighbor_rank_info() const;

protected:
  boost::mpi::communicator*               communicator_       = nullptr;
                                          
  ivector3                                domain_size_        = {};
  ivector3                                grid_size_          = {};
  ivector3                                block_size_         = {};

  std::optional<rank_info>                local_rank_info_    = {};
  std::array<std::optional<rank_info>, 6> neighbor_rank_info_ = {};
};
}

#endif