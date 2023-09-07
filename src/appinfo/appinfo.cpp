#include <spdlog/spdlog.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

// name for app header
#if defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
#include "appinfo.h"
namespace qst {
  AppInfo::AppInfo(std::string_view name, std::string_view exec)
    : _name(name)
    , _exec(exec)
    , _icon()
    , _flags(0) {
  }
  std::string_view AppInfo::name() const {
    return _name;
  }
  void AppInfo::set_name(std::string_view name) {
    this->_name = name;
  }
  void AppInfo::set_name(std::string&& name) {
    this->_name = name;
  }
  std::string_view AppInfo::exec() const {
    return _exec;
  }
  void AppInfo::set_exec(std::string_view exec) {
    this->_exec = exec;
  }
  void AppInfo::set_exec(std::string&& exec) {
    this->_exec = exec;
  }
  void AppInfo::set_flags(uint32_t flags) {
    this->_flags = flags;
  }
  void AppInfo::add_flag(AppInfoFlags flag) {
    this->_flags |= static_cast<uint32_t>(flag);
  }
  uint32_t AppInfo::flags() const {
    return _flags;
  }
  std::string_view AppInfo::icon() const {
    return _icon;
  }
  void AppInfo::set_icon(std::string_view icon) {
    this->_icon = icon;
  }
  void AppInfo::set_icon(std::string&& icon) {
    this->_icon = icon;
  }
  std::string_view AppInfo::working_dir() const {
    return _working_dir;
  }
  void AppInfo::set_working_dir(std::string_view working_dir) {
    this->_working_dir = working_dir;
  }
  void AppInfo::set_working_dir(std::string&& working_dir) {
    this->_working_dir = working_dir;
  }
  std::string_view AppInfo::description() const {
    return _description;
  }
  void AppInfo::set_description(std::string_view description) {
    this->_description = description;
  }
  void AppInfo::set_description(std::string&& description) {
    this->_description = description;
  }
  AppSearcher::AppSearcher() {
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
    IShellLink *pShellLink = NULL;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pShellLink);
    if(FAILED(hr)) {
      spdlog::error("Failed to create shell link");
      return;
    }
    auto resolve_link = [this, pShellLink](const std::filesystem::directory_entry& de) {
      if(de.path().extension() == ".lnk") {
        IPersistFile *pPersistFile = NULL;
        HRESULT hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
        if(SUCCEEDED(hr)) {
          // 加载快捷方式
          hr = pPersistFile->Load(de.path().c_str(), STGM_READ);
          if(SUCCEEDED(hr)) {
            qst::AppInfo app;
            app.set_name(de.path().stem().string());
            // 获取快捷方式的目标路径
            auto path = std::make_unique<CHAR[]>(MAX_PATH);
            hr = pShellLink->GetPath(path.get(), MAX_PATH, NULL, SLGP_RAWPATH);
            if(SUCCEEDED(hr)) {
              app.set_exec(std::string_view(path.get()));
              std::memset(path.get(), 0, MAX_PATH);
            }
            // 获取快捷方式的工作目录
            hr = pShellLink->GetWorkingDirectory(path.get(), MAX_PATH);
            if(SUCCEEDED(hr)) {
              app.set_working_dir(std::string_view(path.get()));
              std::memset(path.get(), 0, MAX_PATH);
            }
            // 获取快捷方式的参数
            hr = pShellLink->GetArguments(path.get(), MAX_PATH);
            if(SUCCEEDED(hr)) {
              std::memset(path.get(), 0, MAX_PATH);
            }
            // 获取快捷方式的描述
            CHAR description[MAX_PATH];
            hr = pShellLink->GetDescription(description, MAX_PATH);
            if(SUCCEEDED(hr)) {
              app.set_description(std::string_view(description));
              std::memset(description, 0, MAX_PATH);
            }
            // 获取快捷方式的图标路径
            int iconIndex;
            hr = pShellLink->GetIconLocation(path.get(), MAX_PATH, &iconIndex);
            if(SUCCEEDED(hr)) {
              app.set_icon(std::string_view(path.get()));
              std::memset(path.get(), 0, MAX_PATH);
            }
            this->apps.insert(app.name(), std::move(app));
          }
          pPersistFile->Release();
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
    pShellLink->Release();
    CoUninitialize();
#elif defined(__linux__)
    // addr = "unix:/tmp/qst.sock";
    std::filesystem::path p("/usr/share/applications");
    for(auto& e : std::filesystem::directory_iterator(p)) {
      if(e.path().extension() == ".desktop") {
        std::ifstream f{e.path()};
        std::string line;
        qst::AppInfo app;
        do
          std::getline(f, line);
        while(!f.eof() && line != "[Desktop Entry]");
        while(!f.eof()) {
          std::getline(f, line);
          if(line.starts_with("Name=")) {
            app.set_name(std::move(line.substr(5)));
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
                app.add_flag(qst::AppInfoFlags::HasArgFile);
              } else if(line[pos] == 'F') {
                app.add_flag(qst::AppInfoFlags::HasArgFiles);
              } else if(line[pos] == 'u') {
                app.add_flag(qst::AppInfoFlags::HasArgUrl);
              } else if(line[pos] == 'U') {
                app.add_flag(qst::AppInfoFlags::HasArgUrls);
              } else if(line[pos] == 'd' || line[pos] == 'D' || line[pos] == 'n' || line[pos] == 'N' || line[pos] == 'v'
                        || line[pos] == 'm') {
                line.erase(pos, 2);
              } else if(line[pos] == 'i') {
              } else if(line[pos] == 'c') {
              } else if(line[pos] == 'k') {
              }
            } while(pos != std::string::npos);
            app.set_exec(std::move(line.substr(5)));
            // app.set_exec(std::move(line.substr(5)));
          } else if(line.starts_with("[")) {
            break;
          }
        }
        // spdlog::trace("Add app: name={} exec={} flags={}", app.name(), app.exec(), app.flags());
        apps.insert(app.name(), std::move(app));
        app.set_flags(0);
      }
    }
#endif
  }
  std::vector<AppInfo *> AppSearcher::search(std::string_view word) {
    return apps.find_prefix(word);
  }
}  // namespace qst