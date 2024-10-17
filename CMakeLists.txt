cmake_minimum_required(VERSION 3.22.1)

project(mango)

add_subdirectory(lib/loquat)

add_executable(mango ${PROJECT_SOURCE_DIR}/examples/main.cpp)

target_link_libraries(mango PRIVATE loquat)
