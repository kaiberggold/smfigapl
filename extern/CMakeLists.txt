# External libraries, fetched when configuring (pinned versions)
include(FetchContent)

# rapidyaml: the YAML adapter. Only the sources are fetched (with the c4core
# files it carries). They are compiled here with their own warnings and
# become part of smfigapl_formats (not a library of their own), so that the
# installed library needs nothing else.
FetchContent_Declare(rapidyaml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG v0.16.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
    SOURCE_SUBDIR sources-only)
FetchContent_MakeAvailable(rapidyaml)
file(GLOB sm_ryml_sources CONFIGURE_DEPENDS
    ${rapidyaml_SOURCE_DIR}/src/c4/yml/*.cpp
    ${rapidyaml_SOURCE_DIR}/ext/c4core.src/c4/*.cpp)
set(SM_RYML_INCLUDE ${rapidyaml_SOURCE_DIR}/src ${rapidyaml_SOURCE_DIR}/ext/c4core.src PARENT_SCOPE)
# tabs after ':' and '-' as in hand written texts; the same in every file
# that includes rapidyaml
set(SM_RYML_DEFINITIONS RYML_WITH_TAB_TOKENS PARENT_SCOPE)
add_library(sm_ryml OBJECT ${sm_ryml_sources})
target_include_directories(sm_ryml SYSTEM PRIVATE ${rapidyaml_SOURCE_DIR}/src ${rapidyaml_SOURCE_DIR}/ext/c4core.src)
target_compile_definitions(sm_ryml PRIVATE RYML_WITH_TAB_TOKENS)
target_compile_features(sm_ryml PRIVATE cxx_std_20)

# cxxopts: the options of the command line tool
if(SM_BUILD_CLI)
    set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.3.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(cxxopts)
endif()

if(SM_BUILD_TESTS)
    # googletest uses the DLL runtime with MSVC, like the rest of the project
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.0
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(googletest)
endif()
