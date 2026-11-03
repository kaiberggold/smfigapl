# std::stop_token (SolveOptions::stopToken) is still experimental in libc++
# before version 20, e.g. in the Android NDK r28 and older (React Native
# 0.88 uses NDK 27): it needs -fexperimental-library there. The result,
# SM_STOP_TOKEN_OPTIONS, is passed on to the users of the library, because
# every file that includes planner.hpp needs it. Empty where the standard
# library has std::stop_token by default.
include(CheckCXXSourceCompiles)

function(sm_check_stop_token)
    set(code "#include <stop_token>
int main() { std::stop_source source; return source.get_token().stop_requested() ? 1 : 0; }")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_source_compiles("${code}" SM_HAS_STOP_TOKEN)
    if(SM_HAS_STOP_TOKEN)
        set(SM_STOP_TOKEN_OPTIONS "" PARENT_SCOPE)
        return()
    endif()
    set(CMAKE_REQUIRED_FLAGS -fexperimental-library)
    # only when the check links a program: when it builds a static library
    # (CMAKE_TRY_COMPILE_TARGET_TYPE, e.g. for iOS), link options go to the
    # archiver, which does not know the option
    if(NOT CMAKE_TRY_COMPILE_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        set(CMAKE_REQUIRED_LINK_OPTIONS -fexperimental-library)
    endif()
    check_cxx_source_compiles("${code}" SM_HAS_EXPERIMENTAL_STOP_TOKEN)
    if(SM_HAS_EXPERIMENTAL_STOP_TOKEN)
        message(STATUS "std::stop_token needs -fexperimental-library (libc++ before 20)")
        set(SM_STOP_TOKEN_OPTIONS -fexperimental-library PARENT_SCOPE)
    else()
        # the build shows the real error
        message(WARNING "std::stop_token was not found; smfigapl needs it (C++20)")
        set(SM_STOP_TOKEN_OPTIONS "" PARENT_SCOPE)
    endif()
endfunction()

sm_check_stop_token()
