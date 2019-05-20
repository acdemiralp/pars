#include <pars/pipeline.hpp>

#include <tbb/tbb.h>

#include <pa/stages/color_generator.hpp>
#include <pa/stages/index_generator.hpp>
#include <pa/stages/seed_generator.hpp>

namespace pars
{
pipeline::pipeline(const std::size_t thread_count)
{

}
image                               pipeline::execute     (const settings& settings)
{
  partitioner_.set_domain_size({settings.dataset_size(0), settings.dataset_size(1), settings.dataset_size(2)});

  data_loader_.set_file (settings.dataset_filepath());
  local_vector_field_     = data_loader_.load_local    ();
  neighbor_vector_fields_ = data_loader_.load_neighbors();

  auto seeds = pa::seed_generator::generate(
    local_vector_field_->offset, 
    local_vector_field_->size  , 
    {settings.particle_tracing_seeds_stride(0), settings.particle_tracing_seeds_stride(1), settings.particle_tracing_seeds_stride(2)},
    settings.particle_tracing_iterations());

  particle_tracer_.set_local_vector_field    (&local_vector_field_);
  particle_tracer_.set_neighbor_vector_fields(&neighbor_vector_fields_);
  particle_tracer_.set_step_size             (settings.particle_tracing_step_size());
  if      (settings.particle_tracing_integrator() == 0)
    particle_tracer_.set_integrator(pa::euler_integrator                       ());
  else if (settings.particle_tracing_integrator() == 1)
    particle_tracer_.set_integrator(pa::modified_midpoint_integrator           ());
  else if (settings.particle_tracing_integrator() == 2)
    particle_tracer_.set_integrator(pa::runge_kutta_4_integrator               ());
  else if (settings.particle_tracing_integrator() == 3)
    particle_tracer_.set_integrator(pa::runge_kutta_cash_karp_54_integrator    ());
  else if (settings.particle_tracing_integrator() == 4)
    particle_tracer_.set_integrator(pa::runge_kutta_dormand_prince_5_integrator());
  else if (settings.particle_tracing_integrator() == 5)
    particle_tracer_.set_integrator(pa::runge_kutta_fehlberg_78_integrator     ());
  else if (settings.particle_tracing_integrator() == 6)
    particle_tracer_.set_integrator(pa::adams_bashforth_2_integrator           ());
  else if (settings.particle_tracing_integrator() == 7)
    particle_tracer_.set_integrator(pa::adams_bashforth_moulton_2_integrator   ());
  integral_curves_ = particle_tracer_.trace(seeds);

  pa::color_generator::mode mode;
  if      (settings.color_mapping_mode() == 0)
    mode = pa::color_generator::mode::hsl_constant_s;
  else if (settings.color_mapping_mode() == 1)
    mode = pa::color_generator::mode::hsl_constant_l;
  else if (settings.color_mapping_mode() == 2)
    mode = pa::color_generator::mode::hsv_constant_s;
  else if (settings.color_mapping_mode() == 3)
    mode = pa::color_generator::mode::hsv_constant_v;
  else if (settings.color_mapping_mode() == 4)
    mode = pa::color_generator::mode::rgb;

  tbb::parallel_for(std::size_t(0), integral_curves_.size(), std::size_t(1), [&] (const std::size_t index)
  {
    pa::index_generator::generate(&integral_curves_[index]);
  });
  tbb::parallel_for(std::size_t(0), integral_curves_.size(), std::size_t(1), [&] (const std::size_t index)
  {
    pa::color_generator::generate(&integral_curves_[index], mode, settings.color_mapping_free_parameter());
  });

  return image();
}
std::pair<image, bm::mpi_session<>> pipeline::benchmark   (const settings& settings)
{
  return std::pair<image, bm::mpi_session<>>();
}

boost::mpi::communicator*           pipeline::communicator()
{
  return &communicator_;
}
}