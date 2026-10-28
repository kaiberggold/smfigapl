# Install rules and the package files: an application finds the library
# with find_package(smfigapl) and links smfigapl::smfigapl. Only the public
# headers (include/smfigapl) are installed; the internal libraries are,
# because the static facade needs them when linking.
include(CMakePackageConfigHelpers)

set(SM_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/smfigapl)

foreach(target types formats api report model solver)
    set_target_properties(smfigapl_${target} PROPERTIES EXPORT_NAME ${target})
endforeach()

install(TARGETS smfigapl_lib smfigapl_types smfigapl_formats smfigapl_api smfigapl_report smfigapl_model smfigapl_solver
    EXPORT smfigaplTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/smfigapl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp")
install(FILES ${PROJECT_BINARY_DIR}/include/smfigapl/version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smfigapl)
install(EXPORT smfigaplTargets
    NAMESPACE smfigapl::
    DESTINATION ${SM_CMAKE_DIR})

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/smfigaplConfig.cmake.in
    ${PROJECT_BINARY_DIR}/smfigaplConfig.cmake
    INSTALL_DESTINATION ${SM_CMAKE_DIR})
# 0.x: a new minor version may change the API (docs/guides/versioning.md)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/smfigaplConfigVersion.cmake
    COMPATIBILITY SameMinorVersion)
install(FILES ${PROJECT_BINARY_DIR}/smfigaplConfig.cmake ${PROJECT_BINARY_DIR}/smfigaplConfigVersion.cmake
    DESTINATION ${SM_CMAKE_DIR})

if(SM_BUILD_CLI)
    install(TARGETS smfigapl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
