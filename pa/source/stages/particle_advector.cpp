#include <pa/stages/particle_advector.hpp>

#include <algorithm>
#include <mutex>

#include <boost/serialization/vector.hpp>
#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#include <pa/math/integrators.hpp>

#undef min
#undef max

namespace pa
{
particle_advector::particle_advector(partitioner* partitioner) : partitioner_(partitioner)
{

}

void                            particle_advector::set_vector_field        (vector_field*             vector_field)
{
  vector_field_ = vector_field;
}               
void                            particle_advector::set_integrator          (const variant_integrator& integrator  )
{               
  integrator_   = integrator   ;
}               
void                            particle_advector::set_step_size           (const scalar              step_size   )
{               
  step_size_    = step_size    ;
}

void                            particle_advector::advect                  (std::vector<particle>&    particles   )
{
  std::vector<std::vector<particle>> inactive_particles(partitioner_->communicator()->size());

  while (!check_completion(particles))
  {
    if (partitioner_->communicator()->rank() == 0) std::cout << "2.1.2.0::particle_advector::create_neighborhood_map\n" ; auto neighborhood_map = create_neighborhood_map();
    if (partitioner_->communicator()->rank() == 0) std::cout << "2.1.2.1::particle_advector::advect\n"                  ; advect                  (particles, inactive_particles, neighborhood_map);
    if (partitioner_->communicator()->rank() == 0) std::cout << "2.1.2.2::particle_advector::out_of_bounds_distribute\n"; out_of_bounds_distribute(particles,                     neighborhood_map);
  }

  if   (partitioner_->communicator()->rank() == 0) std::cout << "2.1.2.3::particle_advector::gather_particles\n";
  std::vector<std::vector<particle>> gathered_particles(partitioner_->communicator()->size());
  boost::mpi::all_to_all(*partitioner_->communicator(), inactive_particles, gathered_particles);
  for (auto& particles_vector : gathered_particles)
    particles.insert(particles.end(), particles_vector.begin(), particles_vector.end());
}

particle_advector::particle_map particle_advector::create_neighborhood_map ()
{
  particle_map neighborhood_map;
  for (auto& neighbor : partitioner_->neighbor_rank_info())
    if (neighbor)
      neighborhood_map.emplace(neighbor->rank, std::vector<particle>());
  return neighborhood_map;
}
void                            particle_advector::advect                  (      std::vector<particle>& active_particles, std::vector<std::vector<particle>>& inactive_particles,       particle_map& neighborhood_map)
{
  tbb::mutex mutex;

  auto& neighbors = partitioner_->neighbor_rank_info();

  tbb::parallel_for(std::size_t(0), active_particles.size(), std::size_t(1), [&] (const std::size_t particle_index)
  {
    auto& particle   = active_particles[particle_index];
    auto  minimum    = vector_field_->offset;
    auto  maximum    = vector_field_->offset + vector_field_->size;
    auto  integrator = integrator_;

    for (std::size_t iteration_index = 1; iteration_index < particle.remaining_iterations; ++iteration_index)
    {
      if (!vector_field_->contains(particle.position))
      {
        particle.remaining_iterations -= iteration_index;

        auto neighbor_rank = -1;
        if      (particle.position[0] < minimum[0] && neighbors[0]) neighbor_rank = neighbors[0]->rank;
        else if (particle.position[0] > maximum[0] && neighbors[1]) neighbor_rank = neighbors[1]->rank;
        else if (particle.position[1] < minimum[1] && neighbors[2]) neighbor_rank = neighbors[2]->rank;
        else if (particle.position[1] > maximum[1] && neighbors[3]) neighbor_rank = neighbors[3]->rank;
        else if (particle.position[2] < minimum[2] && neighbors[4]) neighbor_rank = neighbors[4]->rank;
        else if (particle.position[2] > maximum[2] && neighbors[5]) neighbor_rank = neighbors[5]->rank;
        else
        {
          tbb::mutex::scoped_lock lock(mutex);
          particle.remaining_iterations = 0;
          inactive_particles[particle.original_rank].push_back(particle);
          break;
        }

        particle_map::accessor accessor;
        if (neighborhood_map.find(accessor, neighbor_rank))
          accessor->second.push_back(particle);

        break;
      }

      const auto vector = vector_field_->interpolate(particle.position);
      if (vector.isZero())
      {
        tbb::mutex::scoped_lock lock(mutex);
        particle.remaining_iterations = 0;
        inactive_particles[particle.original_rank].push_back(particle);
        break;
      }
      
      const auto system = [&] (const vector4& x, vector4& dxdt, const float t) 
      { 
        dxdt = vector4(vector[0], vector[1], vector[2], scalar(0));
      };
      if      (std::holds_alternative<euler_integrator>                       (integrator))
        std::get<euler_integrator>                       (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<modified_midpoint_integrator>           (integrator))
        std::get<modified_midpoint_integrator>           (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<runge_kutta_4_integrator>               (integrator))
        std::get<runge_kutta_4_integrator>               (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<runge_kutta_cash_karp_54_integrator>    (integrator))
        std::get<runge_kutta_cash_karp_54_integrator>    (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<runge_kutta_dormand_prince_5_integrator>(integrator))
        std::get<runge_kutta_dormand_prince_5_integrator>(integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<runge_kutta_fehlberg_78_integrator>     (integrator))
        std::get<runge_kutta_fehlberg_78_integrator>     (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<adams_bashforth_2_integrator>           (integrator))
        std::get<adams_bashforth_2_integrator>           (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      else if (std::holds_alternative<adams_bashforth_moulton_2_integrator>   (integrator))
        std::get<adams_bashforth_moulton_2_integrator>   (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      
      if (iteration_index + 1 == particle.remaining_iterations)
      {
        tbb::mutex::scoped_lock lock(mutex);
        particle.remaining_iterations = 0;
        inactive_particles[particle.original_rank].push_back(particle);
        break;
      }
    }
  });
}
void                            particle_advector::out_of_bounds_distribute(      std::vector<particle>& active_particles,                                                         const particle_map& neighborhood_map)
{
  active_particles.clear();

  std::vector<boost::mpi::request> requests;
  for (auto& neighbor : neighborhood_map)
    requests.push_back(partitioner_->communicator()->isend(neighbor.first, 3, neighbor.second));

  for (auto& neighbor : neighborhood_map)
  {
    std::vector<particle> temporary;
    partitioner_->communicator()->recv(neighbor.first, 3, temporary);
    active_particles.insert(active_particles.end(), temporary.begin(), temporary.end());
  }

  for (auto& request : requests)
    request.wait();
}
bool                            particle_advector::check_completion        (const std::vector<particle>& active_particles)
{
  std::vector<std::size_t> active_particle_counts;
  boost::mpi::gather   (*partitioner_->communicator(), active_particles.size(), active_particle_counts, 0);
  auto   complete = std::all_of(active_particle_counts.begin(), active_particle_counts.end(), std::bind(std::equal_to<std::size_t>(), std::placeholders::_1, 0));
  boost::mpi::broadcast(*partitioner_->communicator(), complete, 0);
  return complete;
}
}
