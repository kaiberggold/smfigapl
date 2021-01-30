add_library(stopwatch INTERFACE)
target_include_directories(stopwatch INTERFACE ./Stopwatch/include/stopwatch)

add_library(lib_toml INTERFACE)
target_include_directories(lib_toml INTERFACE ./tomlplusplus/)
target_compile_features(lib_toml INTERFACE cxx_std_17)

#add_subdirectory(cxxopts)
add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE ./cxxopts/include/)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    include(GoogleTest)
endif()
