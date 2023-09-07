add_library(qst-back-end-pm STATIC)

target_sources(qst-back-end-pm
  PRIVATE
  processmanager.cpp
  PUBLIC
  processmanager.h
)
target_link_libraries(qst-back-end-pm
  PRIVATE
  spdlog::spdlog
)
target_include_directories(qst-back-end-pm PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)