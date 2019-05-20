#ifndef PARS_PIPELINE_HPP
#define PARS_PIPELINE_HPP

#define BM_MPI_SUPPORT

#include <utility>

#include <bm/bm.hpp>
#include <boost/mpi/cartesian_communicator.hpp>
#include <boost/mpi.hpp>
#include <pa/math/integral_curves.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/stages/data_loader.hpp>
#include <pa/stages/particle_tracer.hpp>
#include <tbb/tbb.h>

#include <pars/stages/ray_tracer.hpp>
#include <pars/export.hpp>

#include <image.pb.h>
#include <settings.pb.h>

namespace pars
{
class PARS_EXPORT pipeline
{
public:
  explicit pipeline  (const std::size_t thread_count = tbb::task_scheduler_init::default_num_threads());
  pipeline           (const pipeline&   that) = default;
  pipeline           (      pipeline&&  temp) = default;
 ~pipeline           ()                       = default;
  pipeline& operator=(const pipeline&   that) = default;
  pipeline& operator=(      pipeline&&  temp) = default;

  std::pair<image, bm::mpi_session<>> execute     (const settings& settings);

  boost::mpi::communicator*           communicator();

protected:
  boost::mpi::environment  environment_        ;
  boost::mpi::communicator communicator_       ;
                           
  tbb::task_scheduler_init task_scheduler_init_;
                           
  pa::partitioner          partitioner_        ;
  pa::data_loader          data_loader_        ;
  pa::particle_tracer      particle_tracer_    ;
  ray_tracer               ray_tracer_         ;
};
}

#endif
