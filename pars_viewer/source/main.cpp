#include <QApplication>
#include <pars_viewer/viewer.hpp>

int main(int argc, char** argv)
{
  QApplication application(argc, argv);
  pars::viewer viewer;
  application.exec();
  return 0;
}
