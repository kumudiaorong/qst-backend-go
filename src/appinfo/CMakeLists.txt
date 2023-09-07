add_library(qst-back-end-appinfo STATIC)

target_sources(qst-back-end-appinfo
  PRIVATE
  appinfo.cpp
  trie.hpp
  PUBLIC
  appinfo.h
)
target_link_libraries(qst-back-end-appinfo
  PRIVATE
  spdlog::spdlog
)
target_include_directories(qst-back-end-appinfo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)