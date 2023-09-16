#include <spdlog/spdlog.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// name for app header
#if defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
#include <windows.h>

#elif defined(__linux__)
#include <unistd.h>
#endif
#include "appinfo.h"
namespace qst {

  AppInfoFlags operator|(AppInfoFlags a, AppInfoFlags b) {
    return static_cast<AppInfoFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
  }
  bool operator&(AppInfoFlags a, AppInfoFlags b) {
    return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
  }
  AppInfoFlags& operator|=(AppInfoFlags& a, AppInfoFlags b) {
    a = a | b;
    return a;
  }
  AppInfoFlags& operator^=(AppInfoFlags& a, AppInfoFlags b) {
    a = static_cast<AppInfoFlags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    return a;
  }
  AppInfo::AppInfo(std::string_view name, std::string_view exec)
    : name(name)
    , exec(exec)
    , icon()
    , flags(AppInfoFlags::None)
    , working_dir()
    , description()
    , run_count(0) {
  }
  AppSearcher::AppSearcher() {
    std::cout << "AppSearcher\t: start" << std::endl;
#if defined(_WIN32) || defined(_WIN64)
    CoInitialize(NULL);
    LPWSTR startMenuPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, NULL, &startMenuPath);
    if(FAILED(hr)) {
      spdlog::error("Failed to get start menu path");
      return;
    }
    std::filesystem::path p(startMenuPath);
    CoTaskMemFree(startMenuPath);
    IShellLinkW *pShellLink = NULL;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pShellLink);
    if(FAILED(hr)) {
      spdlog::error("Failed to create shell link");
      return;
    }
    auto resolve_link = [this, pShellLink](const std::filesystem::directory_entry& de) {
      if(de.path().extension() == ".lnk") {
        spdlog::debug("AppSearcher\t: resolve {}", de.path().string());
        IPersistFile *pPersistFile = NULL;
        HRESULT hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
        spdlog::debug("AppSearcher\t: QueryInterface");
        if(SUCCEEDED(hr)) {
          // 加载快捷方式
          hr = pPersistFile->Load(de.path().c_str(), STGM_READ);
          spdlog::debug("AppSearcher\t: Load");
          if(SUCCEEDED(hr)) {
            qst::AppInfo app;
            // transcode to utf8
            app.name.reserve(MAX_PATH * 2);
            auto s = de.path().stem().u8string();
            app.name = {(char*)s.data(), s.size()};
            // 获取快捷方式的目标路径
            auto path = std::make_unique<WCHAR[]>(MAX_PATH);
            hr = pShellLink->GetPath(path.get(), MAX_PATH, NULL, SLGP_RAWPATH);
            if(SUCCEEDED(hr)) {
              app.exec.resize(MAX_PATH * 2);
              app.exec.resize(WideCharToMultiByte(CP_UTF8, 0, path.get(), -1, app.exec.data(), app.exec.capacity(), NULL, NULL));
              std::memset(path.get(), 0, MAX_PATH);
              spdlog::debug("AppSearcher\t: GetPath");
            }
            // 获取快捷方式的工作目录
            hr = pShellLink->GetWorkingDirectory(path.get(), MAX_PATH);
            if(SUCCEEDED(hr)) {
              app.working_dir = WideCharToMultiByte(CP_UTF8, 0, path.get(), -1, NULL, 0, NULL, NULL);
              std::memset(path.get(), 0, MAX_PATH);
              spdlog::debug("AppSearcher\t: GetWorkingDirectory");
            }
            // 获取快捷方式的参数
            hr = pShellLink->GetArguments(path.get(), MAX_PATH);
            if(SUCCEEDED(hr)) {
              std::memset(path.get(), 0, MAX_PATH);
            }
            // 获取快捷方式的描述
            WCHAR description[MAX_PATH];
            hr = pShellLink->GetDescription(description, MAX_PATH);
            if(SUCCEEDED(hr)) {
              app.description = WideCharToMultiByte(CP_UTF8, 0, description, -1, NULL, 0, NULL, NULL);
              std::memset(description, 0, MAX_PATH);
              spdlog::debug("AppSearcher\t: GetDescription");
            }
            // 获取快捷方式的图标路径
            int iconIndex;
            hr = pShellLink->GetIconLocation(path.get(), MAX_PATH, &iconIndex);
            if(SUCCEEDED(hr)) {
              app.icon = WideCharToMultiByte(CP_UTF8, 0, path.get(), -1, NULL, 0, NULL, NULL);
              std::memset(path.get(), 0, MAX_PATH);
              spdlog::debug("AppSearcher\t: GetIconLocation");
            }
            std::string name = app.name;
            this->apps.insert(std::move(name), std::move(app));
            spdlog::debug("AppSearcher\t: insert");
          }
          pPersistFile->Release();
          spdlog::debug("AppSearcher\t: Release");
        } else {
          spdlog::debug("AppSearcher\t: {}not a link", de.path().string());
        }
      }
    };
    for(auto& i : std::filesystem::directory_iterator(p)) {
      if(i.is_directory()) {
        for(auto& j : std::filesystem::directory_iterator(i)) {
          resolve_link(j);
        }
      } else {
        resolve_link(i);
      }
    }
    std::cout << "AppSearcher\t: end" << std::endl;
    pShellLink->Release();
    CoUninitialize();
#elif defined(__linux__)
    // addr = "unix:/tmp/qst.sock";
    std::filesystem::path p("/usr/share/applications");
    if(!std::filesystem::exists(p)) {
      spdlog::warn("AppSearcher\t: cannot open /usr/share/applications");
      return;
    }
    for(auto& e : std::filesystem::directory_iterator(p)) {
      if(e.path().extension() == ".desktop") {
        std::ifstream f{e.path()};
        if(!f.is_open()) {
          spdlog::warn("AppSearcher\t: cannot open {}", e.path().string());
          continue;
        }
        std::string line;
        qst::AppInfo app{};
        do
          std::getline(f, line);
        while(!f.eof() && line != "[Desktop Entry]");
        while(!f.eof()) {
          std::getline(f, line);
          if(line.starts_with("Name=")) {
            app.name = std::move(line.substr(5));
          } else if(line.starts_with("Exec=")) {
            std::string::size_type pos = 5;
            do {
              pos = line.find_first_of('%', pos);
              if(pos == std::string::npos) {
                break;
              }
              if(line[++pos] == '%') {
                continue;
              } else if(line[pos] == 'f') {
                app.flags |= qst::AppInfoFlags::HasArgFile;
              } else if(line[pos] == 'F') {
                app.flags |= qst::AppInfoFlags::HasArgFiles;
              } else if(line[pos] == 'u') {
                app.flags |= qst::AppInfoFlags::HasArgUrl;
              } else if(line[pos] == 'U') {
                app.flags |= qst::AppInfoFlags::HasArgUrls;
              } else if(line[pos] == 'd' || line[pos] == 'D' || line[pos] == 'n' || line[pos] == 'N' || line[pos] == 'v'
                        || line[pos] == 'm') {
                line.erase(pos, 2);
              } else if(line[pos] == 'i') {
              } else if(line[pos] == 'c') {
              } else if(line[pos] == 'k') {
              }
            } while(pos != std::string::npos);
            app.exec = std::move(line.substr(5));
            // app.set_exec(std::move(line.substr(5)));
          } else if(line[0] == '[') {
            break;
          }
        }
        // spdlog::trace("Add app: name={} exec={} flags={}", app.name(), app.exec(), app.flags());
        apps.insert(app.name, std::move(app));
      }
    }
#endif
    std::cout << "AppSearcher\t: end" << std::endl;
  }
  std::vector<AppInfo *> AppSearcher::search(std::string_view word) {
    return apps.find_prefix(word, MatchFlags::CaseInsensitive | MatchFlags::Fuzzy);
  }
}  // namespace qst