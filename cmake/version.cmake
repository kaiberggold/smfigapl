# The version string of the build: X.Y.Z in a release; X.Y.Z-devel.N if the
# commit has the tag vX.Y.Z-devel.N (set by CI on every merge into devel),
# else X.Y.Z-devel. Read when configuring.
set(SM_VERSION_PRERELEASE_FULL "${SM_VERSION_PRERELEASE}")
if(SM_VERSION_PRERELEASE)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match --match "v${PROJECT_VERSION}-${SM_VERSION_PRERELEASE}.*" HEAD
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            OUTPUT_VARIABLE SM_VERSION_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE SM_VERSION_TAG_RESULT)
        if(SM_VERSION_TAG_RESULT EQUAL 0 AND SM_VERSION_TAG MATCHES "^v${PROJECT_VERSION}-(${SM_VERSION_PRERELEASE}\\.[0-9]+)$")
            set(SM_VERSION_PRERELEASE_FULL "${CMAKE_MATCH_1}")
        endif()
    endif()
    set(SM_VERSION_STRING "${PROJECT_VERSION}-${SM_VERSION_PRERELEASE_FULL}")
else()
    set(SM_VERSION_STRING "${PROJECT_VERSION}")
endif()
# The initializer of VERSION_PRERELEASE in version.hpp: {} in a release,
# because = "" is a redundant initialization for clang-tidy
if(SM_VERSION_PRERELEASE_FULL)
    set(SM_VERSION_PRERELEASE_INIT " = \"${SM_VERSION_PRERELEASE_FULL}\"")
else()
    set(SM_VERSION_PRERELEASE_INIT "{}")
endif()
message(STATUS "smfigapl version ${SM_VERSION_STRING}")
