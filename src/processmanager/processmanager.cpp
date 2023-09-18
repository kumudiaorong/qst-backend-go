#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "processmanager.h"
#include "../def.h"
#if defined(_WIN32) || defined(_WIN64)
#include <processthreadsapi.h>
#include <Windows.h>
#elif defined(__linux__)
#include <sys/wait.h>
#include <unistd.h>

#endif
#include "spdlog/spdlog.h"
namespace qst {
  ChildProcess::ChildProcess(std::basic_string<env_char> cmd, std::string args) {
#if defined(_WIN32) || defined(_WIN64)
    if(auto p = cmd.find(TEXT("%")); p != std::basic_string<env_char>::npos) {
#ifdef UNICODE
      cmd.replace(p, 2, std::wstring(args.data(), args.size()));
      auto wargs = std::make_unique<wchar_t[]>(args.size() * 2);
      size_t size = 0;
      std::mbstate_t state{};
      const char *pargs = args.c_str();
      if(mbsrtowcs_s(&size, wargs.get(), args.size() * 2, &pargs, args.size(), &state) != 0) {
        spdlog::error("mbsrtowcs_s failed");
      }
      cmd.replace(p, 2, std::wstring(wargs.get(), (size - 1) * 2));
#else
      cmd.replace(p, 2, args);
#endif
    }
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    auto err = CreateProcess(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    if(err == 0) {
      spdlog::error("CreateProcess failed: {}", GetLastError());
      pi.hProcess = INVALID_HANDLE_VALUE;
    }
#elif defined(__linux__)
    if(auto p = cmd.find("%"); p != std::string::npos) {
      cmd.replace(p, 2, args);
    }
    spdlog::debug("cmd: {}", cmd);
    this->pid = fork();
    if(pid == 0) {
      // if(!stdio) {
      //   fclose(stdin);
      //   fclose(stdout);
      //   fclose(stderr);
      // }
      setpgid(0, 0);
      std::system(cmd.data());
      exit(0);
    }
#endif
    flag.test_and_set();
  }
  bool ChildProcess::is_running(void) const {
    return this->flag.test();
  }
  ChildProcess::key_type ChildProcess::key() const {
#if defined(_WIN32) || defined(_WIN64)
    return pi.hProcess;
#elif defined(__linux__)
    return this->pid;
#endif
  }
  void ChildProcess::_wait(void) {
#if defined(_WIN32) || defined(_WIN64)
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    flag.clear();
#elif defined(__linux__)
    int status;
    waitpid(this->pid, &status, 0);
    flag.clear();
#endif
  }
  void ChildProcess::wait(void) {
    this->thr = std::jthread(&ChildProcess::_wait, this);
  }
  ProcessManager::ProcessManager()
    : children() {
  }
  bool ProcessManager::new_process(std::basic_string<env_char>
   cmd, std::string args) {
    // auto f = std::jthread(&qst::new_process, std::move(args));
    auto cp = std::make_unique<ChildProcess>(cmd, args);
    if(cp->is_running()) {
      auto ret = children.emplace(cp->key(), std::move(cp));
      if(ret.second) {
        ret.first->second->wait();
        return true;
      }
    }
    return false;
  }

}  // namespace qst