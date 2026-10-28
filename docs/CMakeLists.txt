# The documentation of the public API: Doxygen with the Doxygen Awesome
# theme. cmake -DSM_BUILD_DOCS=ON, then cmake --build <build> --target docs;
# the pages are in <build>/docs/html. An undocumented part of the public
# API is an error.
find_package(Doxygen 1.9.1 REQUIRED)
include(FetchContent)
FetchContent_Declare(doxygen_awesome
    GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
    GIT_TAG v2.5.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(doxygen_awesome)

set(SM_DOXYGEN_AWESOME_DIR ${doxygen_awesome_SOURCE_DIR})
configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS smfigapl_types
    COMMENT "Writing the documentation (Doxygen)")
