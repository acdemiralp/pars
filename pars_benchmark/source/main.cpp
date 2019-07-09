#include <pars/service.hpp>

int main(const int argc, const char** argv)
{
  pars::benchmark(std::stoll(argv[1]), argv[2]);
  return 0;
}