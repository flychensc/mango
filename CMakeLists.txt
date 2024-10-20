cmake_minimum_required(VERSION 3.22.1)

project(mango)

add_subdirectory(lib/loquat)

find_package(spdlog REQUIRED)

file(GLOB LIBS src/*.cc)
add_library(mango ${LIBS})
target_include_directories(mango PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mango PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

#add_executable(mango ${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp)
#target_link_libraries(mango PRIVATE loquat)
