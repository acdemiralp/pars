#include <pa/stages/data_loader.hpp>

#include <fstream>
#include <iostream>

#include <tbb/tbb.h>

#include <pa/math/types.hpp>

namespace pa
{
data_loader::data_loader(partitioner* partitioner) : partitioner_(partitioner)
{

}

void                                       data_loader::set_file                   (const std::string& filepath )
{
#ifdef H5_HAVE_PARALLEL
  file_ = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite, HighFive::MPIOFileDriver(*partitioner_->communicator(), MPI_INFO_NULL));
#else
  file_ = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite);
#endif
}

ivector3                                   data_loader::load_dimensions            ()
{
  auto dimensions = file_->getDataSet("vectors").getDimensions();
  return ivector3(dimensions[0], dimensions[1], dimensions[2]);
}
std::optional<scalar_field>                data_loader::load_local_scalar_field    (const std::string& name     )
{
  std::optional<scalar_field> scalar_field;

  scalar_field.emplace();
  load_scalar_field(name, partitioner_->local_rank_info().value(), scalar_field);

  return scalar_field;
}
std::optional<vector_field>                data_loader::load_local_vector_field    ()
{
  std::optional<vector_field> vector_field;

  vector_field.emplace();
  load_vector_field(partitioner_->local_rank_info().value(), vector_field);

  return vector_field;
}
std::array<std::optional<vector_field>, 6> data_loader::load_neighbor_vector_fields()
{
  std::array<std::optional<vector_field>, 6> vector_fields;

  auto& neighbor_rank_info = partitioner_->neighbor_rank_info();
  for (auto i = 0; i < neighbor_rank_info.size(); ++i)
  {
    if (!neighbor_rank_info[i])
      continue;

    vector_fields[i].emplace();
    load_vector_field(neighbor_rank_info[i].value(), vector_fields[i]);
  }

  return vector_fields;
}

void                                       data_loader::load_scalar_field          (const std::string& name, const partitioner::rank_info& rank_info, std::optional<scalar_field>& scalar_field)
{
  file_->getDataSet(name).select(
    {std::size_t(rank_info.offset            [0]), std::size_t(rank_info.offset            [1]), std::size_t(rank_info.offset            [2])},
    {std::size_t(rank_info.ghosted_block_size[0]), std::size_t(rank_info.ghosted_block_size[1]), std::size_t(rank_info.ghosted_block_size[2])},
    {1, 1, 1}).read(scalar_field->data);
  
  std::array<scalar, 3> raw_spacing;
  file_->getAttribute("spacing").read(raw_spacing);
  scalar_field->spacing  = vector3(raw_spacing[0], raw_spacing[1], raw_spacing[2]);
  scalar_field->spacing /= scalar_field->spacing.maxCoeff(); // Divide by maximum so that the maximum is 1.

  scalar_field->offset = rank_info.offset          .cast<float>().array() * scalar_field->spacing.array();
  scalar_field->size   = partitioner_->block_size().cast<float>().array() * scalar_field->spacing.array();
}
void                                       data_loader::load_vector_field          (                         const partitioner::rank_info& rank_info, std::optional<vector_field>& vector_field)
{
  boost::multi_array<scalar, 4> data;
  file_->getDataSet("vectors").select(
    {std::size_t(rank_info.offset            [0]), std::size_t(rank_info.offset            [1]), std::size_t(rank_info.offset            [2]), 0},
    {std::size_t(rank_info.ghosted_block_size[0]), std::size_t(rank_info.ghosted_block_size[1]), std::size_t(rank_info.ghosted_block_size[2]), 3},
    {1, 1, 1, 1}).read(data);
  
  vector_field->data.resize(std::array<integer, 3>{rank_info.ghosted_block_size[0], rank_info.ghosted_block_size[1], rank_info.ghosted_block_size[2]});
  tbb::parallel_for(tbb::blocked_range3d<std::size_t>(0, rank_info.ghosted_block_size[0], 0, rank_info.ghosted_block_size[1], 0, rank_info.ghosted_block_size[2]), [&] (const tbb::blocked_range3d<std::size_t>& index) {
    for (auto x = index.pages().begin(), x_end = index.pages().end(); x < x_end; ++x) {
    for (auto y = index.rows ().begin(), y_end = index.rows ().end(); y < y_end; ++y) {
    for (auto z = index.cols ().begin(), z_end = index.cols ().end(); z < z_end; ++z) {
      vector_field->data[x][y][z] = {data[x][y][z][0], data[x][y][z][1], data[x][y][z][2]};
    }}}
  });
  
  std::array<scalar, 3> raw_spacing;
  file_->getAttribute("spacing").read(raw_spacing);
  vector_field->spacing  = vector3(raw_spacing[0], raw_spacing[1], raw_spacing[2]);
  vector_field->spacing /= vector_field->spacing.maxCoeff(); // Divide by maximum so that the maximum is 1.

  vector_field->offset = rank_info.offset          .cast<float>().array() * vector_field->spacing.array();
  vector_field->size   = partitioner_->block_size().cast<float>().array() * vector_field->spacing.array();
}
  
void                                       data_loader::save_ftle_field            (const std::string& name, scalar_field* ftle_field)
{
  auto& rank_info  = partitioner_->local_rank_info();
  auto  offset     = rank_info->offset;
  auto  size       = ftle_field->data.shape();
  auto  total_size = partitioner_->domain_size();

  if (file_->exist(name))
  {
    auto dataset = file_->getDataSet(name);
    dataset.resize({total_size[0], total_size[1], total_size[2]});
    dataset.select({offset[0], offset[1], offset[2]}, {size[0], size[1], size[2]}, {1, 1, 1}).write(ftle_field->data);
  }
  else
  {
    auto dataset = file_->createDataSet<float>(name, HighFive::DataSpace({total_size[0], total_size[1], total_size[2]}));
    dataset.select({offset[0], offset[1], offset[2]}, {size[0], size[1], size[2]}, {1, 1, 1}).write(ftle_field->data);
  }
}
}
