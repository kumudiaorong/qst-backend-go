#ifndef QST_APPINFO_H
#define QST_APPINFO_H
#include <cstdint>
#include <string>
#include <string_view>

#include "trie.hpp"
#include "../def.h"
// name for app header
namespace qst {
  class AppInfo {
  public:
    std::string name{};
    std::basic_string<env_char> exec{};
    std::string icon{};
    std::string args_hint{};
    bool is_config{false};
    std::string working_dir{};
    std::string description{};
    uint32_t run_count = 0;
    AppInfo() = default;
    AppInfo(const AppInfo&) = default;
    AppInfo(AppInfo&&) = default;
    AppInfo& operator=(const AppInfo&) = default;
    AppInfo& operator=(AppInfo&&) = default;
    ~AppInfo() = default;
    AppInfo(std::string_view name, std::string_view exec);
  };
  class AppSearcher {
    Trie<AppInfo> apps;
  public:
    AppSearcher();
    void init();
    std::vector<AppInfo *> search(std::string_view word);
  };
}  // namespace qst
#endif  // QST_APPINFO_H