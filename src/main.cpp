
#include "core.h"
#include "spdlog/spdlog.h"
int main(int argc, char *argv[]) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  spdlog::set_level(spdlog::level::trace);
  qst::QstBackendCore core(argc, argv);
  core.exec();
  return 0;
}
