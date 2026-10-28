# The format adapters: the public types as text. They know the public
# types only; rapidyaml (extern) is compiled into this library and stays
# private.
add_library(smfigapl_formats
    yaml/yaml_layout.cpp
    yaml/yaml_reader.cpp
    yaml/yaml_reader.hpp
    yaml/yaml_setup.cpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/formats/yaml.hpp
    $<TARGET_OBJECTS:sm_ryml>)
target_include_directories(smfigapl_formats SYSTEM PRIVATE ${SM_RYML_INCLUDE})
target_compile_definitions(smfigapl_formats PRIVATE ${SM_RYML_DEFINITIONS})
target_link_libraries(smfigapl_formats PUBLIC smfigapl_types PRIVATE $<BUILD_INTERFACE:sm_options>)
