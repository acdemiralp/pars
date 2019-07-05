#include <pa/stages/flow_map_generator.hpp>

namespace pa
{
flow_map_generator::flow_map_generator(partitioner* partitioner) : particle_tracer(partitioner)
{

}

std::unique_ptr<scalar_field> flow_map_generator::generate(float time)
{

}
}
