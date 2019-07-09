#ifndef PA_STAGES_DATA_LOADER_HPP
#define PA_STAGES_DATA_LOADER_HPP

#include <array>
#include <memory>
#include <optional>

#include <highfive/H5File.hpp>

#include <pa/math/types.hpp>
#include <pa/math/scalar_field.hpp>
#include <pa/math/vector_field.hpp>
#include <pa/stages/partitioner.hpp>
#include <pa/export.hpp>

namespace pa
{
class PA_EXPORT data_loader
{
public:
  explicit data_loader  (partitioner* partitioner) ;
  data_loader           (const data_loader&  that) = delete ;
  data_loader           (      data_loader&& temp) = delete ;
  virtual ~data_loader  ()                         = default;
  data_loader& operator=(const data_loader&  that) = delete ;
  data_loader& operator=(      data_loader&& temp) = delete ;

  void                                       set_file                   (const std::string& filepath);
  ivector3                                   load_dimensions            ();
  std::optional<scalar_field>                load_local_scalar_field    (const std::string& name    );
  std::optional<vector_field>                load_local_vector_field    ();
  std::array<std::optional<vector_field>, 6> load_neighbor_vector_fields();

  void                                       save_ftle_field            (const std::string& name, scalar_field* ftle_field);

protected:
  void                                       load_scalar_field          (const std::string& name    , const partitioner::rank_info& rank_info, std::optional<scalar_field>& scalar_field);
  void                                       load_vector_field          (                             const partitioner::rank_info& rank_info, std::optional<vector_field>& vector_field);

  partitioner*                    partitioner_ = nullptr;
  std::unique_ptr<HighFive::File> file_        = nullptr;
};
}

#endif