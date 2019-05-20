#include <pars/service.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstdint>
#include <string>

#include <google/protobuf/util/json_util.h>
#include <cppzmq/zmq.hpp>
#include <stb/stb_image_write.h>

#include <pars/pipeline.hpp>

#include <image.pb.h>
#include <settings.pb.h>

namespace pars
{
void run      (const std::string& address)
{
  pipeline pipeline;

  zmq::context_t context(1);
  zmq::socket_t  socket (context, ZMQ_PAIR);
  socket.bind(address);

  while (true)
  {
    zmq::message_t request;
    socket.recv  (&request);

    settings settings;
    settings.ParseFromArray(request.data(), static_cast<std::int32_t>(request.size()));

    auto result = pipeline.execute(settings);

    std::string buffer;
    result.first.SerializeToString(&buffer);

    zmq::message_t response(buffer.size());
    memcpy(response.data(), buffer.data(), buffer.size());
    socket.send(response);
  }
}
void benchmark(const std::size_t thread_count, const std::string& settings_filepath)
{
  pipeline pipeline(thread_count);

  settings      settings;
  std::ifstream settings_file  (settings_filepath);
  const auto    settings_string = std::string(std::istreambuf_iterator<char>(settings_file), std::istreambuf_iterator<char>());
  JsonStringToMessage(settings_string, &settings, google::protobuf::util::JsonParseOptions());

  auto result = pipeline.execute(settings);

  const auto output_filepath = settings_filepath + 
    ".nodes."   + std::to_string(pipeline.communicator()->size()) + 
    ".threads." + std::to_string(thread_count);
  result.second.gather();
  result.second.to_csv(output_filepath + ".csv");
  std::cout << "Saved benchmark.\n";

  if (pipeline.communicator()->rank() == 0)
  {
    const auto filepath = output_filepath + ".png";
    stbi_write_png(filepath.c_str(), result.first.size(0), result.first.size(1), 4, result.first.data().c_str(), result.first.size(0) * sizeof(std::uint32_t));
    std::cout << "Saved image.\n";
  }
}
}
