#ifndef QST_BACKEND_PROCESS_H
#define QST_BACKEND_PROCESS_H
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
// #include <processthreadsapi.h>

#elif defined(__linux__)
#include <unistd.h>
#endif
namespace qst {
#if defined(_WIN32) || defined(_WIN64)
  class ChildProcess {
  public:
    typedef HANDLE key_type;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    std::atomic_flag flag;
    std::jthread thr;
    ChildProcess(std::string args);
    ChildProcess(ChildProcess&& ano) = default;
    bool is_running() const;
    void wait();
    key_type key() const;
  private:
    void _wait(void);
  };
#endif
  class ProcessManager {
    std::unordered_map<ChildProcess::key_type,std::unique_ptr<ChildProcess>> children;
  public:
    ProcessManager();
    bool new_process(std::string args);
  };
}  // namespace qst
#endif