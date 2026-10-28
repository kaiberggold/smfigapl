# The output writers (include/smfigapl/report): the public types as text and CSV
add_library(smfigapl_report
    csv.cpp
    report_util.hpp
    text.cpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/report/csv.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/report/labels.hpp
    ${PROJECT_SOURCE_DIR}/include/smfigapl/report/text.hpp)
target_link_libraries(smfigapl_report PUBLIC smfigapl_types PRIVATE $<BUILD_INTERFACE:sm_options>)
