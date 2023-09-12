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
  class ChildProcess {
  public:
#if defined(_WIN32) || defined(_WIN64)
    typedef HANDLE key_type;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
#elif defined(__linux)
    typedef pid_t key_type;
  private:
    key_type pid;
#endif
    std::atomic_flag flag;
    std::jthread thr;
  public:
    ChildProcess(std::string args);
    bool is_running() const;
    void wait();
    key_type key() const;
  private:
    void _wait(void);
  };

  class ProcessManager {
    std::unordered_map<ChildProcess::key_type, std::unique_ptr<ChildProcess>> children;
  public:
    ProcessManager();
    bool new_process(std::string args);
  };
}  // namespace qst
#endif