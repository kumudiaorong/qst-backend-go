#ifndef QST_APPINFO_H
#define QST_APPINFO_H
#include <cstdint>
#include <string>
#include <string_view>

#include "trie.hpp"

// name for app header
namespace qst {
  enum class AppInfoFlags : uint32_t {
    None = 0,
    HasArgFile = 1 << 0,
    HasArgFiles = 1 << 1,
    HasArgUrl = 1 << 2,
    HasArgUrls = 1 << 3,
  };
  AppInfoFlags operator|(AppInfoFlags a, AppInfoFlags b);
  bool operator&(AppInfoFlags a, AppInfoFlags b);
  AppInfoFlags& operator|=(AppInfoFlags& a, AppInfoFlags b);
  AppInfoFlags& operator^=(AppInfoFlags& a, AppInfoFlags b);
  class AppInfo {
  public:
    std::string name{};
    std::string exec{};
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