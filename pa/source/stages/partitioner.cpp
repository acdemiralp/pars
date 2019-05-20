#include <pa/stages/partitioner.hpp>

#include <pa/math/index.hpp>
#include <pa/math/prime_factorize.hpp>

namespace pa
{
partitioner::rank_info::rank_info(const integer rank, const ivector3& grid_size, const ivector3& block_size) 
: rank              (rank)
, multi_rank        (unravel_index(rank, grid_size))
, offset            (block_size.array() * multi_rank.array())
, ghosted_block_size(block_size)
{
  for (auto i = 2; i >= 0; --i)
    if (multi_rank[i] + 1 < grid_size[i]) // If not at border in axis.
      ghosted_block_size[i]++;            // Add one voxel ghost region (note: positive XYZ only).
}

partitioner::partitioner(boost::mpi::communicator* communicator): communicator_(communicator)
{

}

void                                                        partitioner::set_domain_size   (const ivector3& domain_size)
{
  domain_size_ = domain_size;

  auto prime_factors = prime_factorize(communicator_->size());
  auto current_size  = domain_size_;
  grid_size_.setConstant(1);
  while (!prime_factors.empty())
  {
    auto dimension = 0;
    for (auto i = 0; i < 3; ++i)
      if (current_size[i] > current_size[dimension])
        dimension = i;

    current_size[dimension] /= prime_factors.back();
    grid_size_  [dimension] *= prime_factors.back();
    prime_factors.pop_back();
  }

  block_size_ = domain_size_.array() / grid_size_.array();

  local_rank_info_.emplace(communicator_->rank(), grid_size_, block_size_);
  neighbor_rank_info_ = std::array<std::optional<rank_info>, 6>();
  if (local_rank_info_->multi_rank[0] - 1 >= 0           ) neighbor_rank_info_[0].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3(-1,  0,  0).array(), grid_size_), grid_size_, block_size_);
  if (local_rank_info_->multi_rank[0] + 1 < grid_size_[0]) neighbor_rank_info_[1].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3( 1,  0,  0).array(), grid_size_), grid_size_, block_size_);
  if (local_rank_info_->multi_rank[1] - 1 >= 0           ) neighbor_rank_info_[2].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3( 0, -1,  0).array(), grid_size_), grid_size_, block_size_);
  if (local_rank_info_->multi_rank[1] + 1 < grid_size_[1]) neighbor_rank_info_[3].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3( 0,  1,  0).array(), grid_size_), grid_size_, block_size_);
  if (local_rank_info_->multi_rank[2] - 1 >= 0           ) neighbor_rank_info_[4].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3( 0,  0, -1).array(), grid_size_), grid_size_, block_size_);
  if (local_rank_info_->multi_rank[2] + 1 < grid_size_[2]) neighbor_rank_info_[5].emplace(ravel_multi_index<ivector3>(local_rank_info_->multi_rank.array() + ivector3( 0,  0,  1).array(), grid_size_), grid_size_, block_size_);
}

boost::mpi::communicator*                                   partitioner::communicator      ()
{
  return communicator_;
}
const ivector3&                                             partitioner::domain_size       () const
{
  return domain_size_;
}
const ivector3&                                             partitioner::grid_size         () const
{
  return grid_size_;
}
const ivector3&                                             partitioner::block_size        () const
{
  return block_size_;
}

const std::optional<partitioner::rank_info>&                partitioner::local_rank_info   () const
{
  return local_rank_info_;
}
const std::array<std::optional<partitioner::rank_info>, 6>& partitioner::neighbor_rank_info() const
{
  return neighbor_rank_info_;
}
}
