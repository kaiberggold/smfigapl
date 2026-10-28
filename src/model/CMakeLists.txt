add_library(smfigapl_model
    sm_debug.cpp
    sm_except.cpp
    sm_in.cpp
    sm_indexed.cpp
    sm_math.cpp
    sm_named.cpp
    sm_out.cpp
    sm_setup.cpp
    include/smfigapl/model/sm_capacity.hpp
    include/smfigapl/model/sm_debug.hpp
    include/smfigapl/model/sm_error_codes.hpp
    include/smfigapl/model/sm_except.hpp
    include/smfigapl/model/sm_in.hpp
    include/smfigapl/model/sm_indexed.hpp
    include/smfigapl/model/sm_math.hpp
    include/smfigapl/model/sm_named.hpp
    include/smfigapl/model/sm_out.hpp
    include/smfigapl/model/sm_setup.hpp
    include/smfigapl/model/sm_types.hpp)
# internal: the headers are not installed
target_include_directories(smfigapl_model PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(smfigapl_model PUBLIC smfigapl_types $<BUILD_INTERFACE:sm_options>)
