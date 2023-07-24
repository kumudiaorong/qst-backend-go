#include <functional>
#include <ostream>
#include <QCoreApplication>
#include <QSettings>
#include <QLocale>
#include "appopt.h"
#include "trie.hpp"
uint8_t utf8len(char c) {
  if((c & 0xF0) == 0xF0) {
    return 4;
  } else if((c & 0xE0) == 0xE0) {
    return 3;
  } else if((c & 0xC0) == 0xC0) {
    return 2;
  } else if(!(c & 0x80)) {
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  // Trie<int> trie;
  // trie.insert("hello", 1);
  // trie.insert("world", 2);
  // trie.insert("hello world", 3);
  // trie.insert("hello world", 4);
  // std::function<void(QByteArrayView, int&)> func = [](QByteArrayView word, int& info) {
  //   qDebug() << word << "" << info << "\n";
  // };
  // trie.print(func);
  auto locale = QLocale::system();
  locale.languageToString(locale.language());
  qDebug() << locale.name();
  return a.exec();
}
