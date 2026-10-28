# Golden tests: the command line tool on known configurations, its whole
# result (exit code, messages, steps, the written plan or CSV) compared
# with the expected files in expected/. They show every change of the
# behavior; after a reviewed change the expected files are written again
# with the target golden_update.

set(GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GOLDEN_WORKDIR ${CMAKE_CURRENT_BINARY_DIR}/results)
set(GOLDEN_UPDATE_COMMANDS)

# golden_test(name config language mode [FORMAT file] [MAXSTEPS n])
function(golden_test name config language mode)
    cmake_parse_arguments(ARG "" "FORMAT;MAXSTEPS" "" ${ARGN})
    set(expected ${GOLDEN_DIR}/expected/${name}.${mode}.txt)
    set(defines -DBIN=$<TARGET_FILE:smfigapl> -DCONFIG=${config} -DLANGUAGE=${language}
        -DMODE=${mode} -DEXPECTED=${expected} -DWORKDIR=${GOLDEN_WORKDIR})
    if(ARG_FORMAT)
        list(APPEND defines -DFORMAT=${ARG_FORMAT})
    endif()
    if(ARG_MAXSTEPS)
        list(APPEND defines -DMAXSTEPS=${ARG_MAXSTEPS})
    endif()
    add_test(NAME golden.${name}.${mode} COMMAND ${CMAKE_COMMAND} ${defines} -P ${GOLDEN_DIR}/run_golden.cmake)
    set_tests_properties(golden.${name}.${mode} PROPERTIES LABELS golden)
    set(GOLDEN_UPDATE_COMMANDS ${GOLDEN_UPDATE_COMMANDS}
        COMMAND ${CMAKE_COMMAND} ${defines} -DUPDATE=ON -P ${GOLDEN_DIR}/run_golden.cmake PARENT_SCOPE)
endfunction()

# The samples: plan with statistic as text, CSV with their format file
foreach(language english german)
    set(lang EN)
    if(language STREQUAL "german")
        set(lang DE)
    endif()
    file(GLOB samples ${PROJECT_SOURCE_DIR}/samples/${language}/basic/0*.yaml)
    foreach(config ${samples})
        get_filename_component(base ${config} NAME_WE)
        golden_test(samples_${language}_${base} ${config} ${lang} text)
        golden_test(samples_${language}_${base} ${config} ${lang} csv FORMAT ${PROJECT_SOURCE_DIR}/samples/${language}/basic/format.yaml)
    endforeach()
endforeach()

# The configurations of the unit tests
file(GLOB generic ${PROJECT_SOURCE_DIR}/test/cfg_files/generic/0*.yaml)
foreach(config ${generic})
    get_filename_component(base ${config} NAME_WE)
    set(lang EN)
    if(base MATCHES "german")
        set(lang DE)
    endif()
    golden_test(generic_${base} ${config} ${lang} text)
    golden_test(generic_${base} ${config} ${lang} csv)
endforeach()

# Generated configurations: growing
# sizes, field types, impossible and tight conditions
file(GLOB generated ${GOLDEN_DIR}/configs/*.yaml)
foreach(config ${generated})
    get_filename_component(base ${config} NAME_WE)
    if(base STREQUAL "gen_6f_12t_2ft_3-4_games_per_type")
        # 4.2 million steps: aborted at the step limit
        golden_test(${base} ${config} EN text MAXSTEPS 100000)
    else()
        golden_test(${base} ${config} EN text)
    endif()
endforeach()

# CSV with separate tournaments: the pausing teams of each sub tournament
# (the second one used to show the pausing teams of the first)
golden_test(pauses_two_sub_tournaments ${GOLDEN_DIR}/csv/pauses_two_sub_tournaments.yaml EN text)
golden_test(pauses_two_sub_tournaments ${GOLDEN_DIR}/csv/pauses_two_sub_tournaments.yaml EN csv
    FORMAT ${GOLDEN_DIR}/csv/separate_tournaments_format.yaml)

add_custom_target(golden_update ${GOLDEN_UPDATE_COMMANDS} DEPENDS smfigapl
    COMMENT "Writing the expected results of the golden tests")
