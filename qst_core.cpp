
#include <unicode/bytestrie.h>
#include <unicode/stringpiece.h>
#include <unicode/stringtriebuilder.h>
#include <unicode/ucnv.h>
#include <unicode/urename.h>
#include <unicode/ustringtrie.h>
#include <unicode/utf8.h>

#include <filesystem>
#include <iostream>
#include <locale>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "comm/qst.pb.h"
#include "core/core.hpp"
#include "tree/trie.h"
int main(int argc, char *argv[]) {
  qst::QstCore core(argc, argv);
  core.exec();
  return 0;
}
