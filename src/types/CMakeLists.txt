# The public types of the API (include/smfigapl): the domain model, the
# error model, the version. Every other layer builds on them.
configure_file(${PROJECT_SOURCE_DIR}/include/smfigapl/version.hpp.in
    ${PROJECT_BINARY_DIR}/include/smfigapl/version.hpp @ONLY)
add_library(smfigapl_types
    version.cpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/layout.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/log.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/plan.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/result.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/setup.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/statistic.hpp
    ${PROJECT_BINARY_DIR}/include/smfigapl/version.hpp)
target_include_directories(smfigapl_types PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
# users of the library need C++20, not the warning settings of the project
target_compile_features(smfigapl_types PUBLIC cxx_std_20)
target_link_libraries(smfigapl_types PRIVATE $<BUILD_INTERFACE:sm_options>)
