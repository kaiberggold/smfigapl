set( gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
file(GLOB TEST_SRC
       ${PROJECT_SOURCE_DIR}/test/cond_check/*.cpp
       ${PROJECT_SOURCE_DIR}/test/input_val_check/*.cpp
       ${PROJECT_SOURCE_DIR}/test/*.cpp
       )
add_executable(smtest ${TEST_SRC})
set(LOCAL_PATH_STRING 	${PROJECT_SOURCE_DIR})
configure_file(path.hpp.in path.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/test/)	
set_target_properties(smtest PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED OFF
    CXX_EXTENSIONS OFF
)
target_link_libraries(smtest PUBLIC gtest)
target_link_libraries(smtest PRIVATE sm_misc)
target_link_libraries(smtest PRIVATE sm_in)
target_link_libraries(smtest PRIVATE sm_out)
target_link_libraries(smtest PRIVATE sm_val)
target_link_libraries(smtest PRIVATE sm_search)
target_link_libraries(smtest PRIVATE sm_ts)
target_link_libraries(smtest PRIVATE sm_cond)
target_link_libraries(smtest PRIVATE sm_stat)
target_link_libraries(smtest PRIVATE sm_disp)
target_link_libraries(smtest PRIVATE stopwatch)
target_link_libraries(smtest PRIVATE sm_file)
target_link_libraries(smtest PRIVATE sm_except)
target_link_libraries(smtest PRIVATE cxxopts) 
