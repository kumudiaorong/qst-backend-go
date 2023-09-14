#ifndef QST_TRIE_HPP
#define QST_TRIE_HPP
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <memory>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
namespace qst {
  enum class MatchFlags {
    None = 0,
    CaseInsensitive = 1 << 0,
    Fuzzy = 1 << 1,
  };
  constexpr MatchFlags operator|(MatchFlags a, MatchFlags b) {
    return static_cast<MatchFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
  }
  constexpr bool operator&(MatchFlags a, MatchFlags b) {
    return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
  }
  constexpr MatchFlags& operator|=(MatchFlags& a, MatchFlags b) {
    a = a | b;
    return a;
  }
  constexpr MatchFlags& operator^=(MatchFlags& a, MatchFlags b) {
    a = static_cast<MatchFlags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    return a;
  }

  // class U8Char {
  //   char32_t codepoint;
  // public:
  //   U8Char(std::string_view str)
  //     : codepoint(0) {
  //     if(str.empty()) {
  //       return;
  //     }
  //     if(str[0] & 0x80) {
  //       if(str[0] & 0x40) {
  //         if(str[0] & 0x20) {
  //           if(str[0] & 0x10) {
  //             codepoint = (str[0] & 0x07) << 18;
  //             codepoint |= (str[1] & 0x3F) << 12;
  //             codepoint |= (str[2] & 0x3F) << 6;
  //             codepoint |= (str[3] & 0x3F);
  //           } else {
  //             codepoint = (str[0] & 0x0F) << 12;
  //             codepoint |= (str[1] & 0x3F) << 6;
  //             codepoint |= (str[2] & 0x3F);
  //           }
  //         } else {
  //           codepoint = (str[0] & 0x1F) << 6;
  //           codepoint |= (str[1] & 0x3F);
  //         }
  //       } else {
  //         codepoint = (str[0] & 0x3F);
  //       }
  //     } else {
  //       codepoint = (str[0] & 0x7F);
  //     }
  //   }
  //   U8Char(char32_t codepoint)
  //     : codepoint(codepoint) {
  //   }
  //   ~U8Char() {
  //   }
  //   bool operator==(const U8Char& other) const {
  //     return this->codepoint == other.codepoint;
  //   }
  //   bool operator!=(const U8Char& other) const {
  //     return this->codepoint != other.codepoint;
  //   }
  //   bool operator<(const U8Char& other) const {
  //     return this->codepoint < other.codepoint;
  //   }
  //   bool operator>(const U8Char& other) const {
  //     return this->codepoint > other.codepoint;
  //   }
  //   bool operator<=(const U8Char& other) const {
  //     return this->codepoint <= other.codepoint;
  //   }
  //   bool operator>=(const U8Char& other) const {
  //     return this->codepoint >= other.codepoint;
  //   }
  //   char32_t get_codepoint() const {
  //     return this->codepoint;
  //   }
  //   std::size_t size() const {
  //     if(this->codepoint == 0) {
  //       return 0;
  //     } else if(this->codepoint < 0x80) {
  //       return 1;
  //     } else if(this->codepoint < 0x800) {
  //       return 2;
  //     } else if(this->codepoint < 0x10000) {
  //       return 3;
  //     } else {
  //       return 4;
  //     }
  //   }
  // };
  constexpr std::pair<char32_t, std::size_t> u8char(std::string_view str) {
    if(str.empty()) {
      return {0, 0};
    }
    if(!(str[0] & 0x80)) {
      return {str[0], 1};
    }
    if((str[0] & 0x70) == 0x70) {
      return {(str[0] & 0x07) << 18 | (str[1] & 0x3F) << 12 | (str[2] & 0x3F) << 6 | (str[3] & 0x3F), 4};
    }
    if((str[0] & 0x60) == 0x60) {
      return {(str[0] & 0x0F) << 12 | (str[1] & 0x3F) << 6 | (str[2] & 0x3F), 3};
    }
    if((str[0] & 0x40) == 0x40) {
      return {(str[0] & 0x1F) << 6 | (str[1] & 0x3F), 2};
    }
    return {0, 0};
  }

  template <typename Info>
  class TrieNode {
    std::unordered_map<char32_t, TrieNode> children;
    std::vector<Info> infos;
  public:
    TrieNode()
      : TrieNode(nullptr) {
    }
    TrieNode(TrieNode *parent)
      : children()
      , infos() {
    }
    ~TrieNode() {
    }
    TrieNode *try_insert(std::string_view word) {
      auto [c, s] = u8char(word);
      // : this->children.try_emplace(c, this).first->second.try_insert(word.substr(c.size()));
      return s == 0 ? this : this->children.try_emplace(c, this).first->second.try_insert(word.substr(s));
    }
    std::vector<TrieNode *> find(std::string_view word, MatchFlags flags = MatchFlags::None) {
      std::vector<TrieNode *> nodes{};
      auto [c, s] = u8char(word);
      if(s == 0) {
        nodes.push_back(this);
        return nodes;
      }
      auto citer = this->children.end();
      if((flags & MatchFlags::CaseInsensitive) && (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z')) {
        if(citer = this->children.find(c ^ 0x20); citer != this->children.end()) {
          auto child_nodes = citer->second.find(word.substr(s), flags);
          nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
        }
      }
      auto niter = this->children.find(c);
      if(niter != this->children.end()) {
        auto child_nodes = niter->second.find(word.substr(s),flags);
        nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
      }
      if(flags & MatchFlags::Fuzzy) {
        for(auto iter = this->children.begin(); iter != this->children.end(); ++iter) {
          if(iter == citer || iter == niter) {
            continue;
          }
          auto child_nodes = iter->second.find(word,flags);
          nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
        }
      }
      return nodes;
    }
    std::vector<Info *> all_info() {
      std::vector<Info *> nodes;
      if(!infos.empty()) {
        auto v = std::views::transform(this->infos, [](auto& info) { return &info; });
        nodes.insert(nodes.end(), v.begin(), v.end());
      }
      for(auto& child : this->children) {
        auto child_nodes = child.second.all_info();
        nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
      }
      return nodes;
    }
    template <typename _Info>
    void add_info(_Info&& info) {
      this->infos.emplace_back(std::forward<_Info>(info));
    }
    std::vector<Info>& info() {
      return this->infos;
    }
    void print(std::function<void(Info&)> callback) {
      for(auto& child : this->children) {
        for(auto& info : child.second.info())
          callback(info);
        child.second.print(callback);
      }
    }
  };
  template <typename Info>
  class Trie {
  private:
    std::unique_ptr<TrieNode<Info>> root;
  public:
    Trie()
      : root(std::make_unique<TrieNode<Info>>()) {
    }
    ~Trie() {
    }
    template <typename _Info>
    void insert(std::string_view word, _Info&& info) {
      root->try_insert(word)->add_info(std::forward<_Info>(info));
    }
    Info *insert(std::string_view word) {
      return root->find_or_insert(word)->get_info();
    }
    Info *find(std::string_view word) {
      return root->find(word)->get_info();
    }
    std::vector<Info *> find_prefix(std::string_view word, MatchFlags flags = MatchFlags::None) {
      std::vector<Info *> infos;
      auto nodes = root->find(word, flags);
      for(auto node : nodes) {
        auto sub = node->all_info();
        infos.insert(infos.end(), sub.begin(), sub.end());
      }
      return infos;
    }
    void print(std::function<void(Info&)> callback) {
      root->print(callback);
    }
  };
}  // namespace qst
#endif