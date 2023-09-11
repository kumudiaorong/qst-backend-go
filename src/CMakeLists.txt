add_subdirectory(appinfo)
add_subdirectory(processmanager)

add_executable(qst-b)

target_sources(qst-b
  PRIVATE
  main.cpp
  core.cpp
  PUBLIC
  core.h
)

target_link_libraries(qst-b
  PRIVATE
  qst-back-end-appinfo
  qst-back-end-pm
  qst-grpc-cpp
  spdlog::spdlog
  xcl-lib
  # ICU::uc
  # ICU::i18n
)
target_link_directories(qst-b PRIVATE
  ${PROJECT_SOURCE_DIR}/qst-proto/cpp
)
target_include_directories(qst-b PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/qst-proto/cpp
)
