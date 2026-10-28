# An application that uses the smfigapl library, the two ways:
#   cmake -S . -B build                                    FetchContent of a tag
#   cmake -S . -B build -DSMFIGAPL_FIND_PACKAGE=ON         an installed library
#     (-DCMAKE_PREFIX_PATH=<install prefix>)
# The CI builds both with the sources of the commit (for FetchContent:
# -DFETCHCONTENT_SOURCE_DIR_SMFIGAPL=<checkout>).
cmake_minimum_required(VERSION 3.20)
project(smfigapl_consumer LANGUAGES CXX)

option(SMFIGAPL_FIND_PACKAGE "Use an installed smfigapl (find_package) instead of FetchContent" OFF)

if(SMFIGAPL_FIND_PACKAGE)
    find_package(smfigapl 0.5 REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(smfigapl
        GIT_REPOSITORY https://github.com/kaiberggold/smfigapl.git
        GIT_TAG v0.5.0
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(smfigapl)
endif()

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE smfigapl::smfigapl)
