#include <pa/stages/particle_tracer.hpp>

#include <algorithm>
#include <limits>

#include <boost/serialization/vector.hpp>
#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#include <pa/math/integrators.hpp>

#undef min
#undef max

namespace pa
{
particle_tracer::particle_tracer(partitioner* partitioner) : partitioner_(partitioner)
{

}

void                         particle_tracer::set_local_vector_field    (std::optional<vector_field>*                local_vector_field    )
{
  local_vector_field_     = local_vector_field    ;
}
void                         particle_tracer::set_neighbor_vector_fields(std::array<std::optional<vector_field>, 6>* neighbor_vector_fields)
{
  neighbor_vector_fields_ = neighbor_vector_fields;
}
void                         particle_tracer::set_integrator            (const variant_integrator&                   integrator            )
{
  integrator_    = integrator   ;
}
void                         particle_tracer::set_step_size             (const scalar                                step_size             )
{
  step_size_     = step_size    ;
}

std::vector<integral_curves> particle_tracer::trace                     (std::vector<particle>                       particles             )
{
  std::vector<integral_curves> integral_curves;

  while (!check_completion(particles))
  {
                      load_balance_distribute (particles                             );
    auto round_info = compute_round_info      (particles, integral_curves            );
                      allocate                (           integral_curves, round_info);
                      initialize              (particles, integral_curves, round_info);
                      trace                   (particles, integral_curves, round_info);
                      load_balance_collect    (                            round_info);
                      out_of_bounds_distribute(particles,                  round_info);
  }

  prune (integral_curves);
  return integral_curves;
}

void                         particle_tracer::load_balance_distribute   (      std::vector<particle>& particles                                                                                   )
{
  // Send/receive particle counts.
  auto neighbors                = partitioner_->neighbor_rank_info();
  auto neighbor_particle_counts = std::array<std::size_t, 6> {};
  neighbor_particle_counts.fill(std::numeric_limits<std::size_t>::max());

  std::vector<boost::mpi::request> requests;
  for (auto i = 0; i < neighbors.size(); ++i)
    if (neighbors[i])
      requests.push_back(partitioner_->communicator()->isend(neighbors[i]->rank, 0, particles.size()));
  for (auto i = 0; i < neighbors.size(); ++i)
    if (neighbors[i])
      partitioner_->communicator()->recv (neighbors[i]->rank, 0, neighbor_particle_counts[i]);

  for (auto& request : requests)
    request.wait();

  /// Compute workload deficit.

  // Compute average of neighbors with more workload than this process.
  auto contributions     = std::array<std::size_t, 6> {};
  auto contributor_count = 0;
  contributions.fill(0);
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    if (!neighbors[i] || neighbor_particle_counts[i] < particles.size())
      continue;
    contributions[i] = neighbor_particle_counts[i];
    contributor_count++;
  }
  auto average = (particles.size() + std::accumulate(contributions.begin(), contributions.end(), 0ull)) / (contributor_count + 1);

  // Compute average of neighbors with more workload than the average until all contributing neighbors are above the average (i.e. only this process below the average).
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    contributions.fill(0);
    contributor_count = 0;
    for (auto i = 0; i < neighbors.size(); ++i)
    {
      if (!neighbors[i] || neighbor_particle_counts[i] < average)
        continue;
      contributions[i] = neighbor_particle_counts[i];
      contributor_count++;
    }

    const auto next_average = (particles.size() + std::accumulate(contributions.begin(), contributions.end(), 0ull)) / (contributor_count + 1);
    if (average == next_average)
      break;
    average = next_average;
  }

  const auto total_deficit       = average - particles.size();
  const auto total_contributions = std::accumulate(contributions.begin(), contributions.end(), 0ull);
  
  auto deficits        = std::array<std::size_t, 6> {};
  auto maximum_surplus = std::array<std::size_t, 6> {};
  deficits       .fill(0);
  maximum_surplus.fill(0);
  for (auto i = 0; i < neighbors.size(); ++i)
    if (neighbors[i] && total_contributions != 0)
      deficits[i] = total_deficit * (contributions[i] / total_contributions);

  // Send / receive partial deficits (as partial maximum surplus).
  requests.clear();
  for (auto i = 0; i < neighbors.size(); ++i)
    if (neighbors[i])
      requests.push_back(partitioner_->communicator()->isend(neighbors[i]->rank, 10, deficits[i]));
  for (auto i = 0; i < neighbors.size(); ++i)
    if (neighbors[i])
      partitioner_->communicator()->recv (neighbors[i]->rank, 10, maximum_surplus[i]);

  for (auto& request : requests)
    request.wait();

  /// Compute workload surplus.

  // Compute average of neighbors with less workload than this process.
  contributions     = std::array<std::size_t, 6> {};
  contributor_count = 0;
  contributions.fill(0);
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    if (!neighbors[i] || neighbor_particle_counts[i] > particles.size())
      continue;
    contributions[i] = neighbor_particle_counts[i];
    contributor_count++;
  }
  average = (particles.size() + std::accumulate(contributions.begin(), contributions.end(), 0ull)) / (1 + contributor_count);
  
  // Compute average of neighbors with less workload than the average until all contributing neighbors are below the average (i.e. only this process above the average).
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    contributions.fill(0);
    contributor_count = 0;
    for (auto i = 0; i < neighbors.size(); ++i)
    {
      if (!neighbors[i] || neighbor_particle_counts[i] > average)
        continue;
      contributions[i] = neighbor_particle_counts[i];
      contributor_count++;
    }

    const auto next_average = (particles.size() + std::accumulate(contributions.begin(), contributions.end(), 0ull)) / (1 + contributor_count);
    if (average == next_average)
      break;
    average = next_average;
  }
  
  // Compute surplus particles.
  auto surplus_particles = std::array<std::vector<particle>, 6> {};
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    if (!neighbors[i] || neighbor_particle_counts[i] > average)
      continue;

    const auto particle_count = std::min(maximum_surplus[i], std::size_t(average - neighbor_particle_counts[i]));

    if (particle_count > particles.size())
      continue;

    surplus_particles[i].insert(surplus_particles[i].end(), particles.end() - particle_count, particles.end());
    particles.erase(particles.end() - particle_count, particles.end());

    tbb::parallel_for(std::size_t(0), surplus_particles[i].size(), std::size_t(1), [&](const std::size_t index)
    {
      surplus_particles[i][index].vector_field_index = i % 2 == 0 ? i + 1 : i - 1; // This process' +X neighbor treats this process as its -X neighbor and so on. 
    });
  }

  // Send/receive particles.
  requests.clear();
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    if (!neighbors[i])
      continue;

    requests.push_back(partitioner_->communicator()->isend(neighbors[i]->rank, 1, surplus_particles[i]));
  }
  for (auto i = 0; i < neighbors.size(); ++i)
  {
    if (!neighbors[i]) 
      continue;

    std::vector<particle> temporary;
    partitioner_->communicator()->recv (neighbors[i]->rank, 1, temporary);
    particles.insert(particles.end(), temporary.begin(), temporary.end());
  }

  for (auto& request : requests)
    request.wait();
}
particle_tracer::round_info  particle_tracer::compute_round_info        (const std::vector<particle>& particles, const std::vector<integral_curves>& integral_curves                              )
{
  round_info round_info;

  round_info.maximum_remaining_iterations        = std::max_element(particles.begin(), particles.end(), [ ] (const particle& lhs, const particle& rhs) { return lhs.remaining_iterations < rhs.remaining_iterations; })->remaining_iterations;
 
  const auto maximum_vertices_per_integral_curve = std::numeric_limits<integer>::max() / sizeof(vector4);
  const auto particles_per_integral_curve        = maximum_vertices_per_integral_curve / round_info.maximum_remaining_iterations;

  round_info.vertices_per_integral_curve         =                    particles_per_integral_curve * round_info.maximum_remaining_iterations;
  round_info.vertices_per_integral_curve_last    = particles.size() % particles_per_integral_curve * round_info.maximum_remaining_iterations;
  round_info.integral_curve_offset               = integral_curves.size();
  round_info.integral_curve_count                = particles.size() == 0 ? 0 : 1 + particles.size() / particles_per_integral_curve;
  
  for (auto& neighbor : partitioner_->neighbor_rank_info())
  {
    if (neighbor)
    {
      round_info.out_of_bounds_particles         .emplace(neighbor->rank, std::vector<particle>());
      round_info.neighbor_out_of_bounds_particles.emplace(neighbor->rank, std::vector<particle>());
    }
  }

  return     round_info;
}
void                         particle_tracer::allocate                  (                                              std::vector<integral_curves>& integral_curves, const round_info& round_info)
{
  integral_curves.resize(round_info.integral_curve_offset + round_info.integral_curve_count);
  tbb::parallel_for(round_info.integral_curve_offset, integral_curves.size(), std::size_t(1), [&] (const std::size_t index)
  {
    integral_curves[index].vertices.resize(index != integral_curves.size() - 1 ? round_info.vertices_per_integral_curve : round_info.vertices_per_integral_curve_last, invalid_vertex);
  });
}
void                         particle_tracer::initialize                (const std::vector<particle>& particles,       std::vector<integral_curves>& integral_curves, const round_info& round_info)
{
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    const auto absolute_vertex_index = index * round_info.maximum_remaining_iterations;
    const auto relative_vertex_index = absolute_vertex_index % round_info.vertices_per_integral_curve;
    const auto integral_curve_index  = absolute_vertex_index / round_info.vertices_per_integral_curve + round_info.integral_curve_offset;
    integral_curves[integral_curve_index].vertices[relative_vertex_index] = particles[index].position;
  });
}
void                         particle_tracer::trace                     (const std::vector<particle>& particles,       std::vector<integral_curves>& integral_curves,       round_info& round_info)
{
  auto& local     = partitioner_->local_rank_info   ();
  auto& neighbors = partitioner_->neighbor_rank_info();

  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t particle_index)
  {
    auto& particle      = particles[particle_index];
    auto& vector_field  = particle.vector_field_index == -1 ? local_vector_field_->value() : neighbor_vector_fields_->at(particle.vector_field_index).value();
    auto  minimum       = vector_field.offset;
    auto  maximum       = vector_field.offset + vector_field.size;
    auto  integrator    = integrator_;

    for (std::size_t iteration_index = 1; iteration_index < particle.remaining_iterations; ++iteration_index)
    {
      const auto  absolute_vertex_index = particle_index * round_info.maximum_remaining_iterations + iteration_index;
      const auto  relative_vertex_index = absolute_vertex_index % round_info.vertices_per_integral_curve;
      const auto  integral_curve_index  = absolute_vertex_index / round_info.vertices_per_integral_curve + round_info.integral_curve_offset;

      const auto& last_vertex           = integral_curves[integral_curve_index].vertices[relative_vertex_index - 1];
            auto& vertex                = integral_curves[integral_curve_index].vertices[relative_vertex_index    ];
      
      vertex = termination_vertex;

      if (iteration_index == particle.remaining_iterations - 1)
        break;

      if (!vector_field.contains(last_vertex))
      {
        pa::particle neighbor_particle {last_vertex, particle.remaining_iterations - iteration_index, -1};

        if (particle.vector_field_index == -1)
        {
          auto neighbor_rank = -1;
          
          if      (neighbor_particle.position[0] < minimum[0] && neighbors[0]) neighbor_rank = neighbors[0]->rank;
          else if (neighbor_particle.position[0] > maximum[0] && neighbors[1]) neighbor_rank = neighbors[1]->rank;
          else if (neighbor_particle.position[1] < minimum[1] && neighbors[2]) neighbor_rank = neighbors[2]->rank;
          else if (neighbor_particle.position[1] > maximum[1] && neighbors[3]) neighbor_rank = neighbors[3]->rank;
          else if (neighbor_particle.position[2] < minimum[2] && neighbors[4]) neighbor_rank = neighbors[4]->rank;
          else if (neighbor_particle.position[2] > maximum[2] && neighbors[5]) neighbor_rank = neighbors[5]->rank;
          
          round_info::particle_map::accessor accessor;
          if (round_info.out_of_bounds_particles.find(accessor, neighbor_rank))
            accessor->second.push_back(neighbor_particle);
        }
        else
        {
          auto neighbor_rank = neighbors[particle.vector_field_index]->rank;

          round_info::particle_map::accessor accessor;
          if (round_info.neighbor_out_of_bounds_particles.find(accessor, neighbor_rank))
            accessor->second.push_back(neighbor_particle);
        }

        break;
      }

      const auto vector = vector_field.interpolate(last_vertex);
      if (vector.isZero())
        break;

      const auto system = [&] (const vector4& x, vector4& dxdt, const float t) 
      { 
        dxdt = vector4(vector[0], vector[1], vector[2], scalar(0));
      };
      if      (std::holds_alternative<euler_integrator>                       (integrator))
        std::get<euler_integrator>                       (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<modified_midpoint_integrator>           (integrator))
        std::get<modified_midpoint_integrator>           (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<runge_kutta_4_integrator>               (integrator))
        std::get<runge_kutta_4_integrator>               (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<runge_kutta_cash_karp_54_integrator>    (integrator))
        std::get<runge_kutta_cash_karp_54_integrator>    (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<runge_kutta_dormand_prince_5_integrator>(integrator))
        std::get<runge_kutta_dormand_prince_5_integrator>(integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<runge_kutta_fehlberg_78_integrator>     (integrator))
        std::get<runge_kutta_fehlberg_78_integrator>     (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<adams_bashforth_2_integrator>           (integrator))
        std::get<adams_bashforth_2_integrator>           (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
      else if (std::holds_alternative<adams_bashforth_moulton_2_integrator>   (integrator))
        std::get<adams_bashforth_moulton_2_integrator>   (integrator).do_step(system, last_vertex, iteration_index * step_size_, vertex, step_size_);
    }
  });
}
void                         particle_tracer::load_balance_collect      (                                                                                                   round_info& round_info)
{
  auto& neighbors = partitioner_->neighbor_rank_info();
  auto  minimum   = local_vector_field_->value().offset;
  auto  maximum   = local_vector_field_->value().offset + local_vector_field_->value().size;

  std::vector<boost::mpi::request> requests;
  for (auto& neighbor : round_info.neighbor_out_of_bounds_particles)
    requests.push_back(partitioner_->communicator()->isend(neighbor.first, 2, neighbor.second));

  for (auto& neighbor : round_info.neighbor_out_of_bounds_particles)
  {
    std::vector<particle> temporary;
    partitioner_->communicator()->recv (neighbor.first, 2, temporary);

    tbb::parallel_for(std::size_t(0), temporary.size(), std::size_t(1), [&] (const std::size_t index)
    {
      auto& particle      = temporary[index];
      auto  neighbor_rank = -1;

      particle.vector_field_index = -1;

      if      (particle.position[0] < minimum[0] && neighbors[0]) neighbor_rank = neighbors[0]->rank;
      else if (particle.position[0] > maximum[0] && neighbors[1]) neighbor_rank = neighbors[1]->rank;
      else if (particle.position[1] < minimum[1] && neighbors[2]) neighbor_rank = neighbors[2]->rank;
      else if (particle.position[1] > maximum[1] && neighbors[3]) neighbor_rank = neighbors[3]->rank;
      else if (particle.position[2] < minimum[2] && neighbors[4]) neighbor_rank = neighbors[4]->rank;
      else if (particle.position[2] > maximum[2] && neighbors[5]) neighbor_rank = neighbors[5]->rank;

      round_info::particle_map::accessor accessor;
      if (round_info.out_of_bounds_particles.find(accessor, neighbor_rank))
        accessor->second.push_back(particle);
    });
  }

  for (auto& request : requests)
    request.wait();
}
void                         particle_tracer::out_of_bounds_distribute  (      std::vector<particle>& particles,                                                      const round_info& round_info)
{
  particles.clear();

  std::vector<boost::mpi::request> requests;
  for (auto& neighbor : round_info.out_of_bounds_particles)
    requests.push_back(partitioner_->communicator()->isend(neighbor.first, 3, neighbor.second));

  for (auto& neighbor : round_info.out_of_bounds_particles)
  {
    std::vector<particle> temporary;
    partitioner_->communicator()->recv (neighbor.first, 3, temporary);
    particles.insert(particles.end(), temporary.begin(), temporary.end());
  }

  for (auto& request : requests)
    request.wait();
}
bool                         particle_tracer::check_completion          (const std::vector<particle>& particles                                                                                   )
{
  std::vector<std::size_t> particle_sizes;
  boost::mpi::gather   (*partitioner_->communicator(), particles.size(), particle_sizes, 0);
  auto   complete = std::all_of(particle_sizes.begin(), particle_sizes.end(), std::bind(std::equal_to<std::size_t>(), std::placeholders::_1, 0));
  boost::mpi::broadcast(*partitioner_->communicator(), complete, 0);
  return complete;
}
void                         particle_tracer::prune                     (                                              std::vector<integral_curves>& integral_curves                              )
{
  tbb::parallel_for(std::size_t(0), integral_curves.size(), std::size_t(1), [&] (const std::size_t index)
  {
    auto& vertices = integral_curves[index].vertices;
    vertices.erase(std::remove(vertices.begin(), vertices.end(), invalid_vertex), vertices.end());
  });
}
}