cmake_minimum_required(VERSION 3.22.1)

project(mango)

option(SAMPLE "Enable the sample feature for demonstration purposes." ON)
option(DEBUG "Enable debug mode for detailed logging and error checking." OFF)
option(TEST "Enable building and running unit tests using Google Test." ON)

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

if (TEST)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(lib/loquat)

find_package(spdlog REQUIRED)

file(GLOB LIBS src/*.cc)
add_library(mango ${LIBS})
target_include_directories(mango PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mango PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if (SAMPLE)
    file(GLOB_RECURSE mains CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/examples/*.cc)
    foreach(mainfile IN LISTS mains)
        get_filename_component(mainname ${mainfile} NAME_WE)

        add_executable(${mainname} ${mainfile})
        target_link_libraries(${mainname} mango loquat)
    endforeach(mainfile IN LISTS mains)
endif()
