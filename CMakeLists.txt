cmake_minimum_required(VERSION 3.20)
project(smfigapl VERSION "0.5.0" LANGUAGES CXX)
# The prerelease mark of the development line ("devel"); empty in a
# release. The development build N is the tag vX.Y.Z-devel.N of the commit
# (cmake/version.cmake).
set(SM_VERSION_PRERELEASE "")
include(cmake/version.cmake)

# The tests, the command line tool and the install rules only when
# smfigapl is the main project, not when an application adds it
# (FetchContent, add_subdirectory)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(SM_MAIN_PROJECT ON)
else()
    set(SM_MAIN_PROJECT OFF)
endif()

option(SM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(SM_BUILD_TESTS "Build the tests" ${SM_MAIN_PROJECT})
option(SM_BUILD_CLI "Build the command line tool" ${SM_MAIN_PROJECT})
option(SM_BUILD_DOCS "Add the target docs (Doxygen)" OFF)
option(SM_INSTALL "Install rules for the library (find_package(smfigapl))" ${SM_MAIN_PROJECT})

include(GNUInstallDirs)
set(CMAKE_CXX_EXTENSIONS OFF)
if(SM_MAIN_PROJECT)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Common settings for all project targets (not for the external libraries,
# not passed on to the users of the library)
add_library(sm_options INTERFACE)
target_compile_features(sm_options INTERFACE cxx_std_20)
if(MSVC)
    target_compile_options(sm_options INTERFACE /W4 $<$<BOOL:${SM_WARNINGS_AS_ERRORS}>:/WX>)
else()
    target_compile_options(sm_options INTERFACE -Wall -Wextra -Wpedantic -Wshadow $<$<BOOL:${SM_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

add_subdirectory(extern)
# The library
add_subdirectory(src)
# The command line tool
if(SM_BUILD_CLI)
    add_subdirectory(apps)
endif()
if(SM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
# tools of the development repository (not in a release)
include(cmake/devel.cmake OPTIONAL)
if(SM_BUILD_DOCS)
    add_subdirectory(docs)
endif()
if(SM_INSTALL)
    include(cmake/install.cmake)
endif()
