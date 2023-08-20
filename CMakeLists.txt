cmake_minimum_required(VERSION 3.14)

project(qst-back-end LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(ICU 73.0 COMPONENTS uc i18n REQUIRED)

# SET(PROTOBUF_INCLUDE_DIR, "/usr/local/include")
add_executable(qst-b)

target_sources(qst-b PRIVATE
  src/main.cpp
)

add_subdirectory(qst-proto)

target_include_directories(qst-b PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/qst-proto
)

message(STATUS ${CMAKE_CURRENT_SOURCE_DIR}/qst-proto)

target_link_libraries(qst-b PUBLIC
  qst-grpc-cpp

  # ICU::uc
  # ICU::i18n
)
install(TARGETS qst-b
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)