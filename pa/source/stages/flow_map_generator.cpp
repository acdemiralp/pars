#include <pa/stages/flow_map_generator.hpp>

#include <pa/stages/seed_generator.hpp>

namespace pa
{
flow_map_generator::flow_map_generator(partitioner* partitioner) : particle_tracer(partitioner)
{

}

std::unique_ptr<vector_field> flow_map_generator::generate  (const std::size_t iterations      , const scalar resolution_scale)
{
  auto particles = std::vector     <particle>    ();
  auto flow_map  = std::make_unique<vector_field>();
  allocate  (resolution_scale,            flow_map);
  initialize(iterations      , particles, flow_map);
  trace     (                  particles, flow_map);
  return flow_map;
}

void                          flow_map_generator::allocate  (const scalar      resolution_scale,                                   std::unique_ptr<vector_field>& flow_map)
{
  // Create a vector field that has the size of the input vector field, scaled by resolution scale.
  const auto base_size    = local_vector_field_->value().data.shape();
  const auto base_spacing = local_vector_field_->value().spacing;
  flow_map->data.resize(boost::extents
   [base_size   [0] * resolution_scale]
   [base_size   [1] * resolution_scale]
   [base_size   [2] * resolution_scale]);
  flow_map->offset  = local_vector_field_->value().offset;
  flow_map->size    = local_vector_field_->value().size  ;
  flow_map->spacing = {
    base_spacing[0] / resolution_scale,
    base_spacing[1] / resolution_scale,
    base_spacing[2] / resolution_scale};
}
void                          flow_map_generator::initialize(const std::size_t iterations      , std::vector<particle>& particles, std::unique_ptr<vector_field>& flow_map)
{
  // Create particles centered at each voxel of the vector field.
  seed_generator::generate(flow_map->offset, flow_map->size, vector3(1, 1, 1), iterations, partitioner_->communicator()->rank());
}
void                          flow_map_generator::trace     (                                    std::vector<particle>& particles, std::unique_ptr<vector_field>& flow_map)
{
  // TODO: Advect (not trace) particles, storing only the final positions.
  particle_tracer::trace(particles);
}
}
