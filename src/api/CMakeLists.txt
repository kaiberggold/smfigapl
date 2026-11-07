# The facade of the library (planner.hpp): validate, solve, statistic,
# the check of a given plan.
# Only the public types are visible to its users; the internal model and
# the search are private.
add_library(smfigapl_api
    planner.cpp
    statistic.cpp
    check.cpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/planner.hpp)
target_link_libraries(smfigapl_api PUBLIC smfigapl_types PRIVATE smfigapl_model smfigapl_solver $<BUILD_INTERFACE:sm_options>)
