# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(sm_misc sm_misc.cpp ../include/sm_misc.hpp )

add_library(sm_in sm_in.cpp ../include/sm_in.hpp)
target_link_libraries(sm_in PRIVATE sm_misc)
target_link_libraries(sm_in PRIVATE sm_lang)

add_library(sm_out sm_out.cpp ../include/sm_out.hpp)
target_link_libraries(sm_out PRIVATE sm_misc)

add_library(sm_val sm_val.cpp ../include/sm_val.hpp)
target_link_libraries(sm_val PRIVATE sm_misc)
target_link_libraries(sm_val PRIVATE sm_in)

add_library(sm_search sm_search.cpp ../include/sm_search.hpp)
target_link_libraries(sm_search PRIVATE sm_misc)
target_link_libraries(sm_search PRIVATE sm_in)
target_link_libraries(sm_search PRIVATE sm_out)

add_library(sm_ts sm_ts.cpp ../include/sm_ts.hpp)
target_link_libraries(sm_ts PRIVATE sm_misc)
target_link_libraries(sm_ts PRIVATE sm_in)

add_library(sm_cond sm_cond.cpp ../include/sm_cond.hpp)
target_link_libraries(sm_cond PRIVATE sm_misc)
target_link_libraries(sm_cond PRIVATE sm_in)

add_library(sm_stat sm_stat.cpp ../include/sm_stat.hpp)
target_link_libraries(sm_stat PRIVATE sm_misc)
target_link_libraries(sm_stat PRIVATE sm_in)
target_link_libraries(sm_in PRIVATE sm_lang)

add_library(sm_file sm_file.cpp ../include/sm_file.hpp)
target_link_libraries(sm_file PRIVATE sm_misc)
target_link_libraries(sm_file PRIVATE sm_in)
target_link_libraries(sm_file PRIVATE lib_toml) 

add_library(sm_disp sm_disp.cpp ../include/sm_stat.hpp)
target_link_libraries(sm_disp PRIVATE sm_misc)
target_link_libraries(sm_disp PRIVATE sm_in)
target_link_libraries(sm_disp PRIVATE sm_lang)

add_library(sm_except sm_except.cpp ../include/sm_except.hpp)
target_link_libraries(sm_except PRIVATE sm_misc)

add_library(sm_lang sm_lang.cpp ../include/sm_lang.hpp)
target_link_libraries(sm_lang PRIVATE sm_misc)

# We need this directory, and users of our library will need it too
target_include_directories(sm_misc PUBLIC ../include)
target_include_directories(sm_in PUBLIC ../include)
target_include_directories(sm_out PUBLIC ../include)
target_include_directories(sm_val PUBLIC ../include)
target_include_directories(sm_search PUBLIC ../include)
target_include_directories(sm_ts PUBLIC ../include)
target_include_directories(sm_cond PUBLIC ../include)
target_include_directories(sm_stat PUBLIC ../include)
target_include_directories(sm_disp PUBLIC ../include)
target_include_directories(sm_except PUBLIC ../include)

# All users of this library will need at least C++17
#target_compile_features(smfigapl_lib PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
#source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

configure_file (
  "${PROJECT_SOURCE_DIR}/include/sm_version.hpp.in"
  "${PROJECT_BINARY_DIR}/sm_version.hpp"  ) 
