add_library(smfigapl_solver
    sm_atomic.cpp
    sm_cond.cpp
    sm_search.cpp
    sm_search_result.cpp
    sm_search_types.cpp
    sm_tables.cpp
    sm_ts.cpp
    include/smfigapl/solver/sm_atomic.hpp
    include/smfigapl/solver/sm_cond.hpp
    include/smfigapl/solver/sm_search.hpp
    include/smfigapl/solver/sm_search_types.hpp
    include/smfigapl/solver/sm_tables.hpp
    include/smfigapl/solver/sm_ts.hpp)
# internal: the headers are not installed
target_include_directories(smfigapl_solver PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(smfigapl_solver PUBLIC smfigapl_model PRIVATE $<BUILD_INTERFACE:sm_options>)
