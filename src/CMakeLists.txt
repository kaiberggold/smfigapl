# One library per layer. Each layer only sees its own headers and the
# headers of the layers it links, so a dependency on a higher layer
# does not compile.
#
#   types   the public types of the API (include/smfigapl)
#   model   internal data model, errors, public setup -> internal Input
#   formats the format adapters (YAML): public types <-> text
#   api     the facade (planner.hpp): public setup -> internal model -> result
#   solver  tree search and conditions
#   report  the output writers (text, CSV)
add_subdirectory(types)
add_subdirectory(model)
add_subdirectory(formats)
add_subdirectory(api)
add_subdirectory(solver)
add_subdirectory(report)

# The library of the public API: the target an application links. Its
# users see include/smfigapl only; the internal headers are private.
add_library(smfigapl_lib INTERFACE)
add_library(smfigapl::smfigapl ALIAS smfigapl_lib)
set_target_properties(smfigapl_lib PROPERTIES EXPORT_NAME smfigapl)
target_link_libraries(smfigapl_lib INTERFACE smfigapl_types smfigapl_formats smfigapl_api smfigapl_report)

# The internal libraries, for the unit tests of the internals and the
# benchmark tool
add_library(smfigapl_core INTERFACE)
target_link_libraries(smfigapl_core INTERFACE smfigapl_types smfigapl_model smfigapl_formats smfigapl_api smfigapl_solver smfigapl_report)
