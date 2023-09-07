cmake_minimum_required(VERSION 3.14)

project(qst-back-end LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND ${CMAKE_COMMAND} --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -Bbuild -G Ninja
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/qst-proto
)
execute_process(
  COMMAND ${CMAKE_COMMAND} --build build --config Release --target qst-grpc-cpp-gen
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/qst-proto
)


add_subdirectory(qst-proto)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog)

add_subdirectory(src)

install(TARGETS qst-b
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)