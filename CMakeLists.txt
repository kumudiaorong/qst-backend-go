cmake_minimum_required(VERSION 3.14)

project(qst-core LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_PREFIX_PATH "/home/kmdr/tools/Qt/6.2.4/gcc_64" ${CMAKE_PREFIX_PATH})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(qst-core
  appopt.h
  appopt.cpp
  main.cpp
)
target_link_libraries(qst-core Qt${QT_VERSION_MAJOR}::Core)

install(TARGETS qst-core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
