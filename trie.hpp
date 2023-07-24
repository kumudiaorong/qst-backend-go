#ifndef XSL_TRIE_HPP
#define XSL_TRIE_HPP
#include <functional>
#include <QByteArray>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QVector>
#include <utility>
uint8_t utf8len(char c);
template <typename Info>
class TrieNode {
private:
  QSharedPointer<TrieNode> parent;
  QSharedPointer<QMap<QByteArray, TrieNode>> children;
  QSharedPointer<QVector<Info>> infos;
public:
  TrieNode()
    : TrieNode(nullptr) {
  }
  TrieNode(TrieNode *parent)
    : parent(parent)
    , children(nullptr)
    , infos(nullptr) {
  }
  TrieNode(QSharedPointer<TrieNode> parent)
    : parent(parent)
    , children(nullptr)
    , infos(nullptr) {
  }
  ~TrieNode() {
  }
  TrieNode *find_or_insert(QByteArrayView word) {
    if(word.empty()) {
      return this;
    }
    uint8_t len = utf8len(word[0]);
    if(len == 0) {
      return nullptr;
    }
    if(this->children.isNull()) {
      this->children = QSharedPointer<QMap<QByteArray, TrieNode>>::create();
    }
    auto utf8 = word.first(len).toByteArray();
    auto iter = this->children->find(utf8);
    return (iter == this->children->end() ? this->children->insert(std::move(utf8), TrieNode<Info>(this)) : iter)
      .value()
      .find_or_insert(word.sliced(len));
  }
  TrieNode *find(QByteArrayView word) {
    if(word.empty()) {
      return this;
    }
    uint8_t len = utf8len(word[0]);
    if(len == 0) {
      return nullptr;
    }
    if(this->children.isNull()) {
      return nullptr;
    }
    auto utf8 = word.toByteArray();
    auto iter = this->children->find(utf8);
    return (iter == this->children->end() ? nullptr : iter).value().find(word.sliced(len));
  }
  template <typename _Info>
  void set_info(_Info&& info) {
    if(this->infos.isNull()) {
      this->infos = QSharedPointer<QVector<Info>>::create();
    }
    this->infos->emplace_back(std::forward<_Info>(info));
  }
  QSharedPointer<QVector<Info>> get_info() {
    return this->infos;
  }
  void print(QByteArrayView prefix, std::function<void(QByteArrayView, Info&)> callback) {
    if(this->children != nullptr) {
      for(auto iter = this->children->begin(); iter != this->children->end(); ++iter) {
        if(iter.value().get_info() != nullptr)
          for(auto& info : *iter.value().get_info())
            callback(prefix.toByteArray() + iter.key(), info);
        iter.value().print(prefix.toByteArray() + iter.key(), callback);
      }
    }
  }
};
template <typename Info>
class Trie {
private:
  QSharedPointer<TrieNode<Info>> root;
public:
  Trie()
    : root(QSharedPointer<TrieNode<Info>>::create()) {
  }
  ~Trie() {
  }
  template <typename _Info>
  void insert(QByteArrayView word, _Info&& info) {
    root->find_or_insert(word)->set_info(std::forward<_Info>(info));
  }
  TrieNode<Info> *find(QByteArrayView word) {
    TrieNode<Info> *node = root;
    size_t i = 0;
    while(i < word.size() && node != nullptr) {
      uint8_t len = utf8len(word[i]);
      node = node->find(word.sliced(i, len));
      i += len;
    }
    return node;
  }
  void print(std::function<void(QByteArrayView, Info&)> callback) {
    root->print("", callback);
  }
};
#endif