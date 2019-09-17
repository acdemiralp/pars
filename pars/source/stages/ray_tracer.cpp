#include <pars/stages/ray_tracer.hpp>

#include <tbb/tbb.h>

namespace pars
{
ray_tracer::ray_tracer (pa::partitioner* partitioner, const std::size_t thread_count) : partitioner_(partitioner), raw_communicator_(*partitioner->communicator())
{
  if (ospLoadModule("ispc") != OSP_NO_ERROR)
    throw std::runtime_error("Failed to load OSPRay ISPC module");
  if (ospLoadModule("mpi") != OSP_NO_ERROR)
    throw std::runtime_error("Failed to load OSPRay MPI module");

  const auto device = ospray::cpp::Device("mpi_distributed");
  device.set       ("numThreads"       , thread_count      );
  //device.set     ("setAffinity"      , 0                 ); // Causes random hangs in distributed mode.
  device.set       ("masterRank"       , 0                 );
  device.set       ("worldCommunicator", &raw_communicator_);
#if _DEBUG
  ospDeviceSetStatusFunc(device.handle(), [ ] (                const char* message) { std::cout << message << "\n"; });
  ospDeviceSetErrorFunc (device.handle(), [ ] (OSPError error, const char* message) { std::cout << message << "\n"; });
  device.set       ("logLevel"         , 3                 );
  device.set       ("logOutput"        , "cout"            );
  device.set       ("errorOutput"      , "cerr"            );
#endif
  device.commit    ();
  device.setCurrent();

  auto material = ospray::cpp::Material("scivis", "OBJMaterial");
  material.set   ("Ks", 0.5F, 0.5F, 0.5F);
  material.set   ("Ns", 10.0F);
  material.commit();
  
  geometry_   .emplace_back(std::make_unique<ospray::cpp::Geometry>("streamlines"));
  vertex_data_.emplace_back(std::make_unique<ospray::cpp::Data>(0, OSP_FLOAT3A, nullptr)); vertex_data_.back()->commit();
  color_data_ .emplace_back(std::make_unique<ospray::cpp::Data>(0, OSP_FLOAT4 , nullptr)); color_data_ .back()->commit();
  index_data_ .emplace_back(std::make_unique<ospray::cpp::Data>(0, OSP_INT    , nullptr)); index_data_ .back()->commit();
  
  geometry_[0]->set        ("vertex"      , *vertex_data_.back());
  geometry_[0]->set        ("vertex.color", *color_data_ .back());
  geometry_[0]->set        ("index"       , *index_data_ .back());
  geometry_[0]->set        ("radius"      , 0.1F);
  geometry_[0]->setMaterial(material);
  geometry_[0]->commit     ();

  const std::vector<std::array<float, 3>> colors    {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
  const std::vector<float>                opacities {0.0001f, 0.0255f};
  transfer_function_     = std::make_unique<ospray::cpp::TransferFunction>("piecewise_linear");
  transfer_color_data_   = std::make_unique<ospray::cpp::Data>(colors   .size(), OSP_FLOAT3, colors   .data()); transfer_color_data_  ->commit();
  transfer_opacity_data_ = std::make_unique<ospray::cpp::Data>(opacities.size(), OSP_FLOAT , opacities.data()); transfer_opacity_data_->commit();
  transfer_function_->set   ("colors"    , *transfer_color_data_       );
  transfer_function_->set   ("opacities" , *transfer_opacity_data_     );
  transfer_function_->set   ("valueRange", ospcommon::vec2f{0.0f, 1.0f});
  transfer_function_->commit();
  
  model_ = std::make_unique<ospray::cpp::Model>();
  model_->addGeometry(*geometry_[0]);

  model_->set        ("id", partitioner_->local_rank_info()->rank);
  model_->commit     ();
  
  camera_ = std::make_unique<ospray::cpp::Camera>("perspective");
  camera_->set   ("fovy"      , 68.0F);
  camera_->set   ("imageStart", ospcommon::vec2f{0.0F, 1.0F});
  camera_->set   ("imageEnd"  , ospcommon::vec2f{1.0F, 0.0F});
  camera_->commit();
  
  auto ambient_light = ospray::cpp::Light("ambient");
  ambient_light.set   ("intensity", 1.0F);
  ambient_light.commit();
  const auto ambient_handle = ambient_light.handle();
  
  auto distant_light = ospray::cpp::Light("distant");
  distant_light.set   ("direction"      , 1.0F, 1.0F, -0.5F);
  distant_light.set   ("color"          , 1.0F, 1.0F,  0.8F);
  distant_light.set   ("intensity"      , 1.0F);
  distant_light.set   ("angularDiameter", 0.5F);
  distant_light.commit();
  const auto distant_handle = distant_light.handle();
  
  auto distant_light2 = ospray::cpp::Light("distant");
  distant_light2.set   ("direction"      , -1.0F, 0.0F, 0.5F);
  distant_light2.set   ("color"          , 1.0F, 1.0F,  0.8F);
  distant_light2.set   ("intensity"      , 1.0F);
  distant_light2.set   ("angularDiameter", 0.5F);
  distant_light2.commit();
  const auto distant_handle_2 = distant_light2.handle();
  
  std::vector<OSPLight> lights_list = {ambient_handle, distant_handle, distant_handle_2};
  lights_ = std::make_unique<ospray::cpp::Data>(lights_list.size(), OSP_LIGHT, lights_list.data());
  lights_->commit();
  
  renderer_ = std::make_unique<ospray::cpp::Renderer>("mpi_raycast");
  renderer_->set   ("aoSamples"            , 0       );
  renderer_->set   ("aoTransparencyEnabled", false   );
  renderer_->set   ("oneSidedLighting"     , true    );
  renderer_->set   ("spp"                  , 1       );
  renderer_->set   ("bgColor"              , 0.0F, 0.0F, 0.0F, 1.0F);
  renderer_->set   ("camera"               , *camera_);
  renderer_->set   ("lights"               , *lights_);
  renderer_->set   ("model"                , *model_ );
  renderer_->commit();
  
  framebuffer_ = std::make_unique<ospray::cpp::FrameBuffer>(ospcommon::vec2i(32, 32), OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  framebuffer_->clear(OSP_FB_COLOR | OSP_FB_ACCUM);
}
ray_tracer::~ray_tracer()
{  
  framebuffer_.reset();
  lights_     .reset();
  index_data_ .clear();
  color_data_ .clear();
  vertex_data_.clear();
  geometry_   .clear();
  model_      .reset();
  camera_     .reset();
  renderer_   .reset();
  ospShutdown();
}

void  ray_tracer::set_volume         (pa::scalar_field* scalar_field)
{
  if (volume_)
    model_->removeVolume(*volume_);

  boost::multi_array<float, 3> inverted_data(boost::extents[scalar_field->data.shape()[2]][scalar_field->data.shape()[1]][scalar_field->data.shape()[0]]);
  tbb::parallel_for(std::size_t(0), inverted_data.shape()[0], std::size_t(1), [&] (const std::size_t x) {
  tbb::parallel_for(std::size_t(0), inverted_data.shape()[1], std::size_t(1), [&] (const std::size_t y) {
  tbb::parallel_for(std::size_t(0), inverted_data.shape()[2], std::size_t(1), [&] (const std::size_t z) {
    inverted_data[x][y][z] = scalar_field->data[z][y][x];
  });});});

  volume_      = std::make_unique<ospray::cpp::Volume>("shared_structured_volume"); // "block_bricked_volume"
  volume_data_ = std::make_unique<ospray::cpp::Data>(inverted_data.num_elements(), OSP_FLOAT, inverted_data.data()); volume_data_->commit();
  volume_->set      ("dimensions"      , ospcommon::vec3i(scalar_field->data.shape()[0], scalar_field->data.shape()[1], scalar_field->data.shape()[2]));
  volume_->set      ("gridOrigin"      , ospcommon::vec3f(scalar_field->offset      [0], scalar_field->offset      [1], scalar_field->offset      [2]));
  volume_->set      ("gridSpacing"     , ospcommon::vec3f(scalar_field->spacing     [0], scalar_field->spacing     [1], scalar_field->spacing     [2]));
  volume_->set      ("transferFunction", *transfer_function_);
  volume_->set      ("voxelType"       , "float");
  //volume_->set    ("voxelRange"      , ospcommon::vec2f(0.0f, 1.0f));
  volume_->set      ("voxelData"       , *volume_data_);
  //volume_->setRegion(scalar_field->data.origin(), ospcommon::vec3i {0, 0, 0}, ospcommon::vec3i(scalar_field->data.shape()[0], scalar_field->data.shape()[1], scalar_field->data.shape()[2]));
  volume_->commit   ();

  model_ ->addVolume(*volume_);
  
  //const ospcommon::vec3f region_lower_bounds { scalar_field->offset[0]                        , scalar_field->offset[1]                        , scalar_field->offset[2]                         };
  //const ospcommon::vec3f region_upper_bounds { scalar_field->offset[0] + scalar_field->size[0], scalar_field->offset[1] + scalar_field->size[1], scalar_field->offset[2] + scalar_field->size[2] };
  //model_ ->set      ("region.lower", region_lower_bounds);
  //model_ ->set      ("region.upper", region_upper_bounds);
  
  model_ ->commit   ();

  framebuffer_->clear(OSP_FB_COLOR | OSP_FB_ACCUM);
}
void  ray_tracer::set_integral_curves(std::vector<pa::integral_curves>* integral_curves, float radius)
{
  auto material = ospray::cpp::Material("scivis", "OBJMaterial");
  material.set   ("Ks", 0.7F, 0.7F, 0.7F);
  material.set   ("Ns", 10.0F);
  material.commit();

  for (auto& geometry : geometry_)
    model_->removeGeometry(*geometry);

  geometry_   .clear();
  vertex_data_.clear();
  color_data_ .clear();
  index_data_ .clear();

  geometry_   .reserve(integral_curves->size());
  vertex_data_.reserve(integral_curves->size());
  color_data_ .reserve(integral_curves->size());
  index_data_ .reserve(integral_curves->size());

  for(auto& iteratee : *integral_curves)
  {
    if (iteratee.indices.empty())
      continue;

    const auto vertex   = vertex_data_.emplace_back(new ospray::cpp::Data(iteratee.vertices.size(), OSP_FLOAT3A, iteratee.vertices.data(), OSP_DATA_SHARED_BUFFER)).get(); vertex->commit();
    const auto color    = color_data_ .emplace_back(new ospray::cpp::Data(iteratee.colors  .size(), OSP_FLOAT4 , iteratee.colors  .data(), OSP_DATA_SHARED_BUFFER)).get(); color ->commit();
    const auto index    = index_data_ .emplace_back(new ospray::cpp::Data(iteratee.indices .size(), OSP_INT    , iteratee.indices .data(), OSP_DATA_SHARED_BUFFER)).get(); index ->commit();
    const auto geometry = geometry_   .emplace_back(new ospray::cpp::Geometry("streamlines")).get();
    geometry->setMaterial(material);
    geometry->set        ("vertex"      , *vertex);
    geometry->set        ("vertex.color", *color );
    geometry->set        ("index"       , *index );
    geometry->set        ("radius"      , radius );
    geometry->commit     ();
    model_  ->addGeometry(*geometry);
  }

  if (geometry_.empty())
  {
    const auto vertex   = vertex_data_.emplace_back(new ospray::cpp::Data(0, OSP_FLOAT3A, nullptr)).get(); vertex->commit();
    const auto color    = color_data_ .emplace_back(new ospray::cpp::Data(0, OSP_FLOAT4 , nullptr)).get(); color ->commit();
    const auto index    = index_data_ .emplace_back(new ospray::cpp::Data(0, OSP_INT    , nullptr)).get(); index ->commit();
    const auto geometry = geometry_   .emplace_back(new ospray::cpp::Geometry("streamlines")).get();
    geometry->setMaterial(material);
    geometry->set        ("vertex"      , *vertex);
    geometry->set        ("vertex.color", *color );
    geometry->set        ("index"       , *index );
    geometry->set        ("radius"      , radius );
    geometry->commit     ();
    model_->addGeometry(*geometry);
  }

  model_->commit(); // Very heavy.

  framebuffer_->clear(OSP_FB_COLOR | OSP_FB_ACCUM);
}
void  ray_tracer::set_camera         (const pa::vector3& position, const pa::vector3& forward, const pa::vector3& up)
{
  camera_->set   ("pos", ospcommon::vec3f{position[0], position[1], position[2]});
  camera_->set   ("dir", ospcommon::vec3f{forward [0], forward [1], forward [2]});
  camera_->set   ("up" , ospcommon::vec3f{up      [0], up      [1], up      [2]});
  camera_->commit();

  framebuffer_->clear (OSP_FB_COLOR | OSP_FB_ACCUM);
}
void  ray_tracer::set_image_size     (const pa::ivector2& image_size)
{
  framebuffer_size_ = image_size;

  camera_->set   ("aspect", framebuffer_size_[0] / static_cast<float>(framebuffer_size_[1]));
  camera_->commit();

  framebuffer_ = std::make_unique<ospray::cpp::FrameBuffer>(
    ospcommon::vec2i(static_cast<std::int32_t>(framebuffer_size_[0]), static_cast<std::int32_t>(framebuffer_size_[1])),
    OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  framebuffer_->clear(OSP_FB_COLOR | OSP_FB_ACCUM);
}

void  ray_tracer::trace              (const std::size_t iterations)
{
  for (std::size_t i = 0; i < iterations; ++i)
    renderer_->renderFrame(*framebuffer_, OSP_FB_COLOR | OSP_FB_ACCUM);
}
image ray_tracer::serialize          ()
{
  image image;
  if (partitioner_->local_rank_info()->rank == 0)
  {
    const auto bytes = static_cast<std::uint8_t*>(framebuffer_->map(OSP_FB_COLOR));
    image.set_data    (bytes, framebuffer_size_.prod() * sizeof(std::array<std::uint8_t, 4>));
    image.mutable_size()->Resize(2, 0);
    image.mutable_size()->Set   (0, framebuffer_size_[0]);
    image.mutable_size()->Set   (1, framebuffer_size_[1]);
    framebuffer_->unmap(bytes);
  }
  return image;
}
}