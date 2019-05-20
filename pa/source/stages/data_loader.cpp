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

void                                       data_loader::set_file      (const std::string& filepath )
{
  file_ = std::make_unique<HighFive::File>(filepath);
}
std::optional<vector_field>                data_loader::load_local    (const selection&   selection)
{
  std::optional<vector_field> vector_field;

  vector_field.emplace();
  load(selection, partitioner_->local_rank_info().value(), vector_field);

  return vector_field;
}
std::array<std::optional<vector_field>, 6> data_loader::load_neighbors(const selection&   selection)
{
  std::array<std::optional<vector_field>, 6> vector_fields;

  auto& neighbor_rank_info = partitioner_->neighbor_rank_info();
  for (auto i = 0; i < neighbor_rank_info.size(); ++i)
  {
    if (!neighbor_rank_info[i])
      continue;

    vector_fields[i].emplace();
    load(selection, neighbor_rank_info[i].value(), vector_fields[i]);
  }

  return vector_fields;
}

void                                       data_loader::load          (const selection&   selection, const partitioner::rank_info& rank_info, std::optional<vector_field>& vector_field)
{
  pa::selection rank_selection
  {
    selection.stride.array() * rank_info.offset            .array() + selection.offset.array(),
    selection.stride.array() * rank_info.ghosted_block_size.array(),
    selection.stride
  };

  ivector3 data_size = rank_selection.size.array() / rank_selection.stride.array();
  
  boost::multi_array<scalar, 4> data;
  file_->getDataSet("vectors").select(
    {std::size_t(rank_selection.offset[0]), std::size_t(rank_selection.offset[1]), std::size_t(rank_selection.offset[2]), 0},
    {std::size_t(rank_selection.size  [0]), std::size_t(rank_selection.size  [1]), std::size_t(rank_selection.size  [2]), 3},
    {std::size_t(rank_selection.stride[0]), std::size_t(rank_selection.stride[1]), std::size_t(rank_selection.stride[2]), 1}).read(data);
  
  vector_field->data.resize(std::array<integer, 3>{data_size[0], data_size[1], data_size[2]});
  tbb::parallel_for(tbb::blocked_range3d<std::size_t>(0, data_size[0], 0, data_size[1], 0, data_size[2]), [&] (const tbb::blocked_range3d<std::size_t>& index) {
    for (auto x = index.pages().begin(), x_end = index.pages().end(); x < x_end; ++x) {
    for (auto y = index.rows ().begin(), y_end = index.rows ().end(); y < y_end; ++y) {
    for (auto z = index.cols ().begin(), z_end = index.cols ().end(); z < z_end; ++z) {
      vector_field->data[x][y][z] = {data[x][y][z][0], data[x][y][z][1], data[x][y][z][2]};
    }}}
  });
  
  std::array<scalar, 3> raw_spacing;
  file_->getAttribute("spacing").read(raw_spacing);
  vector_field->spacing  = vector3(raw_spacing[0], raw_spacing[1], raw_spacing[2]);
  vector_field->spacing  = vector_field->spacing.array   () * selection.stride.cast<scalar>().array(); // Scale spacing by stride.
  vector_field->spacing /= vector_field->spacing.maxCoeff();                                           // Divide by maximum so that the maximum is 1.
}
}
