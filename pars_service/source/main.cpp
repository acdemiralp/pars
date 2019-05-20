#include <pars/service.hpp>

int main(const int argc, const char** argv)
{
  pars::run("tcp://*:14130");
  return 0;
}
