#ifndef QST_TRIE_H
#define QST_TRIE_H
#include <unicode/utf8.h>
#include <unicode/utypes.h>

#include <filesystem>
#include <fstream>
#include <locale>
#include <ranges>
#include <string_view>
#include <unordered_map>

#include "cpp/qst.pb.h"
namespace qst {
  template <typename Info>
  class TrieNode {
  public:
    std::unordered_map<char, TrieNode> children;
    std::vector<Info> infos;
    TrieNode()
      : TrieNode(nullptr) {
    }
    TrieNode(TrieNode *parent)
      : children()
      , infos() {
    }
    ~TrieNode() {
    }
    TrieNode *find_or_insert(std::string_view word) {
      return word.empty()
               ? this
               : this->children.try_emplace(word[0], TrieNode<Info>(this)).first->second.find_or_insert(word.substr(1));
    }
    TrieNode *find(std::string_view word) {
      if(word.empty()) {
        return this;
      }
      auto iter = this->children.find(word[0]);
      return (iter == this->children.end() ? nullptr : iter->second.find(word.substr(1)));
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
      root->find_or_insert(word)->add_info(std::forward<_Info>(info));
    }
    Info *insert(std::string_view word) {
      return root->find_or_insert(word)->get_info();
    }
    Info *find(std::string_view word) {
      return root->find(word)->get_info();
    }
    std::vector<Info *> find_prefix(std::string_view word) {
      std::vector<Info *> infos;
      auto node = root->find(word);
      if(node != nullptr) {
        infos = std::move(node->all_info());
      }
      return infos;
    }
    void print(std::function<void(Info&)> callback) {
      root->print(callback);
    }
  };
}  // namespace qst
#endif