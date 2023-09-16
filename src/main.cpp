
#include "core.h"
#include "spdlog/spdlog.h"
int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::trace);
  qst::QstBackendCore core(argc, argv);
  core.exec();
  return 0;
}
