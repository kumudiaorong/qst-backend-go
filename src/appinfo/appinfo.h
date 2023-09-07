#ifndef QST_APPINFO_HPP
#define QST_APPINFO_HPP
#include <cstdint>
#include <string>
#include <string_view>
#include "trie.hpp"
// name for app header
namespace qst {
  enum class AppInfoFlags : uint32_t {
    HasArgFile = 1 << 0,
    HasArgFiles = 1 << 1,
    HasArgUrl = 1 << 2,
    HasArgUrls = 1 << 3,
  };
  class AppInfo {
  public:
    std::string _name;
    std::string _exec;
    std::string _icon;
    uint32_t _flags;
    std::string _working_dir;
    std::string _description;
    AppInfo() = default;
    AppInfo(const AppInfo&) = default;
    AppInfo(AppInfo&&) = default;
    AppInfo& operator=(const AppInfo&) = default;
    AppInfo& operator=(AppInfo&&) = default;
    ~AppInfo() = default;
    AppInfo(std::string_view name, std::string_view exec);
    std::string_view name() const;
    void set_name(std::string_view name);
    void set_name(std::string&& name);
    std::string_view exec() const;
    void set_exec(std::string_view exec);
    void set_exec(std::string&& exec);
    uint32_t flags() const;
    void set_flags(uint32_t flags);
    void add_flag(AppInfoFlags flag);
    std::string_view icon() const;
    void set_icon(std::string_view icon);
    void set_icon(std::string&& icon);
    std::string_view working_dir() const;
    void set_working_dir(std::string_view working_dir);
    void set_working_dir(std::string&& working_dir);
    std::string_view description() const;
    void set_description(std::string_view description);
    void set_description(std::string&& description);
  };
  class AppSearcher {
  public:
    Trie<AppInfo> apps;
  public:
    AppSearcher();
    std::vector<AppInfo *> search(std::string_view word);
  };
}  // namespace qst
#endif  // QST_APPINFO_H