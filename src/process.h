#ifndef QST_BACKEND_PROCESS_H
#define QST_BACKEND_PROCESS_H
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
namespace qst {
  class Process {
  public:
    Process();
  };
}  // namespace qst
#endif