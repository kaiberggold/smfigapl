cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0048 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
project (smfigapl VERSION "0.0.1")
set ("VERSION" ${CMAKE_PROJECT_VERSION})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# The compiled library code is here
add_subdirectory(src)
add_subdirectory(extern/)
# The executable code is here
add_subdirectory(apps)
add_subdirectory(test)
#Cxxdroid-binary: bin/test