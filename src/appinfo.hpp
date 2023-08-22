#ifndef QST_APPINFO_HPP
#define QST_APPINFO_HPP
#include <cstdint>
#include <string>
#include <string_view>
namespace qst {
  class AppInfo {
  public:
    std::string _name;
    std::string _exec;
    std::string _icon;
    uint32_t _flags;
    AppInfo() = default;
    AppInfo(const AppInfo&) = default;
    AppInfo(AppInfo&&) = default;
    AppInfo& operator=(const AppInfo&) = default;
    AppInfo& operator=(AppInfo&&) = default;
    ~AppInfo() = default;
    AppInfo(std::string_view name, std::string_view exec)
      : _name(name)
      , _exec(exec)
      , _icon()
      , _flags(0) {
    }
    std::string_view name() const {
      return _name;
    }
    void set_name(std::string_view name) {
      this->_name = name;
    }
    void set_name(std::string&& name) {
      this->_name = name;
    }
    std::string_view exec() const {
      return _exec;
    }
    void set_exec(std::string_view exec) {
      this->_exec = exec;
    }
    void set_exec(std::string&& exec) {
      this->_exec = exec;
    }
    uint32_t flags() const {
      return _flags;
    }
  };
}  // namespace qst
#endif  // QST_APPINFO_H