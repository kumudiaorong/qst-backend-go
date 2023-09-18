
#include "core.h"
int main(int argc, char *argv[]) {
  qst::QstBackendCore core(argc, argv);
  core.exec();
  return 0;
}
