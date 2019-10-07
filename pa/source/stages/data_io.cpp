#include <pa/stages/data_io.hpp>

#include <fstream>
#include <iostream>

#include <tbb/tbb.h>

#include <pa/math/types.hpp>

#ifdef VTK_SUPPORT
#include <vtkGenericDataObjectWriter.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkSmartPointer.h>
#endif

namespace pa
{
data_io::data_io(partitioner* partitioner) : partitioner_(partitioner)
{

}

void                                       data_io::set_file                   (const std::string& filepath )
{
#ifdef H5_HAVE_PARALLEL
  file_ = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite, HighFive::MPIOFileDriver(*partitioner_->communicator(), MPI_INFO_NULL));
#else
  file_ = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite);
#endif
}

ivector3                                   data_io::load_dimensions            ()
{
  auto dimensions = file_->getDataSet("vectors").getDimensions();
  return ivector3(dimensions[0], dimensions[1], dimensions[2]);
}
std::optional<scalar_field>                data_io::load_local_scalar_field    (const std::string& name     )
{
  std::optional<scalar_field> scalar_field;

  scalar_field.emplace();
  load_scalar_field(name, partitioner_->local_rank_info().value(), scalar_field);

  return scalar_field;
}
std::optional<vector_field>                data_io::load_local_vector_field    ()
{
  std::optional<vector_field> vector_field;

  vector_field.emplace();
  load_vector_field(partitioner_->local_rank_info().value(), vector_field);

  return vector_field;
}
std::array<std::optional<vector_field>, 6> data_io::load_neighbor_vector_fields()
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

void                                       data_io::load_scalar_field          (const std::string& name, const partitioner::rank_info& rank_info, std::optional<scalar_field>& scalar_field)
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
void                                       data_io::load_vector_field          (                         const partitioner::rank_info& rank_info, std::optional<vector_field>& vector_field)
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
  
void                                       data_io::save_ftle_field            (const std::string& name    , scalar_field*                 ftle_field     )
{
  auto& rank_info  = partitioner_->local_rank_info();
  auto  offset     = rank_info->offset;
  auto  size       = ftle_field->data.shape();
  auto  total_size = partitioner_->domain_size();

  if (file_->exist(name))
  {
    auto dataset = file_->getDataSet(name);
    //dataset.resize({std::size_t(total_size[0]), std::size_t(total_size[1]), std::size_t(total_size[2])});
    dataset.select({std::size_t(offset    [0]), std::size_t(offset    [1]), std::size_t(offset    [2])}, 
                   {std::size_t(size      [0]), std::size_t(size      [1]), std::size_t(size      [2])}, 
                   {1, 1, 1}).write(ftle_field->data);
  }
  else
  {
    auto dataset = file_->createDataSet<float>(name, HighFive::DataSpace(
                   {std::size_t(total_size[0]), std::size_t(total_size[1]), std::size_t(total_size[2])}));
    dataset.select({std::size_t(offset    [0]), std::size_t(offset    [1]), std::size_t(offset    [2])}, 
                   {std::size_t(size      [0]), std::size_t(size      [1]), std::size_t(size      [2])}, 
                   {1, 1, 1}).write(ftle_field->data);
  }
}

void                                       data_io::save_integral_curves       (const std::string& prefix, std::vector<integral_curves>* integral_curves)
{
#ifdef VTK_SUPPORT
  auto points = vtkSmartPointer<vtkPoints           >::New();
  auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  auto cells  = vtkSmartPointer<vtkCellArray        >::New();
  auto line   = vtkSmartPointer<vtkPolyLine         >::New();
  colors->SetNumberOfComponents(3);
  colors->SetName("Colors");

  for (auto i = 0; i < integral_curves->size(); ++i)
  {
    auto& integral_curve = integral_curves->at(i);

    for (auto j = 0; j < integral_curve.vertices.size(); ++j)
    {
      auto& vertex = integral_curve.vertices[j];
      auto& color  = integral_curve.colors  [j];
    
      if (vertex != termination_vertex)
      {
        line->GetPointIds()->InsertNextId(points->InsertNextPoint(vertex[0], vertex[1], vertex[2]));
        colors->InsertNextTuple3(color[0] * 255.0f, color[1] * 255.0f, color[2] * 255.0f);
      }
      else if (line->GetPointIds()->GetNumberOfIds() > 0)
      {
        cells->InsertNextCell(line);
        line = vtkSmartPointer<vtkPolyLine>::New();
      }
    }
  }

  auto polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  polydata->SetLines (cells );
  polydata->GetPointData()->SetScalars(colors);

  auto writer = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
  writer->SetFileName (std::string(prefix + "_" + std::to_string(partitioner_->local_rank_info()->rank) + "r.vtk").c_str());
  writer->SetInputData(polydata);
  writer->Write       ();

#else
  throw std::runtime_error("Unable to save integral curves: Built without VTK support.");
#endif
}
}
