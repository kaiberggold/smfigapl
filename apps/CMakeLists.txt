# The texts of the command line tool (a library for its tests)
add_library(smfigapl_cli STATIC cli/texts.cpp cli/texts.hpp)
target_include_directories(smfigapl_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smfigapl_cli PUBLIC smfigapl_types sm_options)

# The command line tool uses the public API only
add_executable(smfigapl smfigapl.cpp)
target_link_libraries(smfigapl PRIVATE smfigapl::smfigapl smfigapl_cli cxxopts::cxxopts sm_options)
