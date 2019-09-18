#ifndef PA_STAGES_DATA_LOADER_HPP
#define PA_STAGES_DATA_LOADER_HPP

#include <array>
#include <memory>
#include <optional>

#include <highfive/H5File.hpp>

#include <pa/math/integral_curves.hpp>
#include <pa/math/types.hpp>
#include <pa/math/scalar_field.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT data_io
{
public:
  explicit data_io  (partitioner* partitioner) ;
  data_io           (const data_io&  that) = delete ;
  data_io           (      data_io&& temp) = delete ;
  virtual ~data_io  ()                     = default;
  data_io& operator=(const data_io&  that) = delete ;
  data_io& operator=(      data_io&& temp) = delete ;

  void                                       set_file                   (const std::string& filepath);
  ivector3                                   load_dimensions            ();
  std::optional<scalar_field>                load_local_scalar_field    (const std::string& name    );
  std::optional<vector_field>                load_local_vector_field    ();
  std::array<std::optional<vector_field>, 6> load_neighbor_vector_fields();

  void                                       save_ftle_field            (const std::string& name  , scalar_field*                 ftle_field     );

  void                                       save_integral_curves       (const std::string& prefix, std::vector<integral_curves>* integral_curves);

protected:
  void                                       load_scalar_field          (const std::string& name  , const partitioner::rank_info& rank_info, std::optional<scalar_field>& scalar_field);
  void                                       load_vector_field          (                           const partitioner::rank_info& rank_info, std::optional<vector_field>& vector_field);

  partitioner*                    partitioner_ = nullptr;
  std::unique_ptr<HighFive::File> file_        = nullptr;
};
}

#endif