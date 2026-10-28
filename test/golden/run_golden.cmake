# Runs the command line tool for one golden test and compares its result
# with the expected file, or writes the expected file (UPDATE=ON).
#
#   cmake -DBIN=... -DCONFIG=... -DLANGUAGE=EN|DE -DMODE=text|csv [-DFORMAT=...]
#         [-DMAXSTEPS=...] -DEXPECTED=... -DWORKDIR=... [-DUPDATE=ON] -P run_golden.cmake
#
# The result is one text: the exit code, stdout (without the measured
# time), stderr and the written output file, line endings normalized.

foreach(var BIN CONFIG LANGUAGE MODE EXPECTED WORKDIR)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "run_golden.cmake: ${var} is not set")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORKDIR}")
get_filename_component(name "${EXPECTED}" NAME_WLE)
set(output "${WORKDIR}/${name}.out")
file(REMOVE "${output}")

# -w prints the steps of the search (and the time, removed below)
set(args -w -l ${LANGUAGE} -i "${CONFIG}" -o "${output}")
if(MODE STREQUAL "text")
    list(APPEND args -t -s)
elseif(DEFINED FORMAT)
    list(APPEND args -f "${FORMAT}")
endif()
if(DEFINED MAXSTEPS)
    list(APPEND args -m ${MAXSTEPS})
endif()

execute_process(COMMAND "${BIN}" ${args}
    RESULT_VARIABLE exitCode OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)

if(EXISTS "${output}")
    file(READ "${output}" outputFile)
else()
    set(outputFile "(no file)\n")
endif()
string(REGEX REPLACE "(^|\n)Time: [0-9]+ ms\n" "\\1" stdout "${stdout}")
set(actual "== exit code\n${exitCode}\n== stdout\n${stdout}\n== stderr\n${stderr}\n== output file\n${outputFile}")
string(REPLACE "\r\n" "\n" actual "${actual}")

if(UPDATE)
    file(WRITE "${EXPECTED}" "${actual}")
    message(STATUS "updated ${EXPECTED}")
    return()
endif()

if(NOT EXISTS "${EXPECTED}")
    file(WRITE "${WORKDIR}/${name}.actual" "${actual}")
    message(FATAL_ERROR "no expected result ${EXPECTED} (actual result in ${WORKDIR}/${name}.actual)")
endif()
file(READ "${EXPECTED}" expected)
string(REPLACE "\r\n" "\n" expected "${expected}")
if(NOT actual STREQUAL expected)
    file(WRITE "${WORKDIR}/${name}.actual" "${actual}")
    message(FATAL_ERROR "result differs from ${EXPECTED}\n"
        "actual result: ${WORKDIR}/${name}.actual\n"
        "after a reviewed change: cmake --build <build dir> --target golden_update")
endif()
