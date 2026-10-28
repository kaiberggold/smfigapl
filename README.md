# Introduction
smfigapl (Small field game Planer) is a command line tool to generate a game plan. It is specially designed for the requirements of small field soccer tournaments:
- A small number of teams (e.g. 8 Teams)
- Each Club has 1-3 Teams
- Parallel games on several playing fields, typcially four fields.
- Game Plans are not complete, e.g. odd number off teams, Number of Teams > Number of Rounds +1, etc.
- Different types of playing fields (Two large goals or four small goals)
- Typically 6-8 Playing rounds.
- Often odd numbers e.g. 7 Teams, so teams have to pause
- Some side conditions (e.g. optimally no or a small number of games within the same club, an equal distribution over the different playing field, pauses distributed equally within the different teams, etc.)
- for a large amount of teams split in two subtournaments

smfigapl reads the basic configuration (team and club names, field types and names) and the side conditions from a configuration file, and generates a gameplan, which fullfills the side conditions. The output is either a simples text file (-t option), or a csv-Style file designed to be imported in a spreadsheet program. An example libreoffice sheet can be  found in the samples folder. The sheet then be formated in a way to obtain a nice printable gameplan.


# GamePlan
The generated gamplan can be imported with a spreadsheet programm, e.g. libreoffice. Here the sample sheet (See *samples/LANGUAGE/basic/tournament.ods*)). 

Teams colors are autoformated via a hidden color row. Gameplan is generic, unused rows are filtered via a generated filter row. (Note that the filter does not update automatically yet). 

![GamePlan](images/003_4-fields_7-teams.png?raw=true)

The generated statistic is shown here:
![GamePlan](images/003_4-fields_7-teams_statistic.png?raw=true)

# Basic Usage
```
Usage:
  ./build/Debug/bin/smfigapl [OPTION...]

  -h, --help               Print usage
  -v, --version            Print version
  -d, --debug              Debug
  -e, --debugoptions arg   Debug Options
  -w, --stopwatch          Enable Stopwatch
  -m, --maxsteps N         Maximum search steps
      --timelimit SECONDS  Maximum search time in seconds
  -p, --progress           Print Progress
  -s, --statistic          Print Statistic (For text output)
  -t, --text               Simple Text Output
  -z, --stepsize N         Steps printing spacing
  -i, --input FILE         Input File
  -f, --format FILE        Format File
  -l, --language LANGUAGE  Language (EN,DE) (default: DE)
  -o, --output FILE        Output File
```

Without a game plan the tool says why (on stderr):
- *No game plan possible*: proven, with the reason, e.g. "Field type T0 has
  28 places, the minimums of the teams need 32" (found by counting before
  the search) or "The search tried all possibilities".
- *Search aborted (limit reached)*: the step limit (`-m`, 10 million by
  default) or the time limit (`--timelimit`) stopped the search before it
  found a plan or proved that there is none. The plan shows where it stopped.

# Configuration files
Teams, fields, conditions etc. are defined in a configuration file in [YAML](https://yaml.org) (*.yaml*). Sample files are in *samples/english/basic* and *samples/german/basic*. The keys are English and lower case; the names of fields, clubs and teams are free. The schemas *docs/schema/setup.schema.json* and *docs/schema/layout.schema.json* describe every key (many editors use them for completion and checks, e.g. with a first line `# yaml-language-server: $schema=...`).
The output layout of the CSV file may be in a separate file (option `-f`), so that one layout serves several configurations.

# Example configuration
```yaml
# Number of rounds to play (1-10)
rounds: 7

# Fields (1-10), in the order of the plan:
# - name: the name of the field in the plan
# - type: for the field type conditions and the output
# - subtype: teams can be denied the fields of a subtype (subtype_deny);
#   fields of subtypes that no team shares form separate sub tournaments
fields:
  - {name: '1', type: Small Goals, subtype: Left}
  - {name: '2', type: Large Goals, subtype: Left}
  - {name: '3', type: Small Goals, subtype: Right}
  - {name: '4', type: Large Goals, subtype: Right}

# Clubs (1-16), in the order of the plan:
# - name: for the same club condition and the output
# - teams: the teams of the club, e.g. by the colors of their shirts
clubs:
  - name: North
    teams: [Red, Green]
  - name: South
    teams: [Red, Blue]
  - name: West
    teams: [Red, Green]
  - name: East
    teams: [Red, Black]

# Conditions: all of them hold in the plan. If there is no plan, widen the
# limits. Every pairing is played once (or `iterations` times) in any case.
conditions:
  # Every team pauses in at most max_pauses rounds
  max_pauses: 1
  # Every team plays at most max_same_club_games games against its own club
  max_same_club_games: 2
  # In every round at most max_empty_fields fields have no game
  max_empty_fields: 1
  # Every team plays min to max games on the fields of a type
  field_types:
    - {type: Small Goals, min: 3, max: 4}
    - {type: Large Goals, min: 3, max: 4}
  # Teams that may not play on the fields of a subtype, e.g. for sub tournaments
  # subtype_deny:
  #   - subtype: Left
  #     teams:
  #       - {club: North, team: Red}
```
Further keys: `repeat_pairings` and `iterations` (pairings played again), `home_away_balance`, `optimize_round_order` and `format_version` (see the samples and the schema).

# Errors
Errors in the configuration (syntax errors, missing or invalid values, values out of range) are reported with an error number and a message in the chosen language on stderr, e.g.
```
Error number13: Value missing: subtype
```
The exit code is the error number (0 if a game plan was found or the search ended without one). Wrong command line options also end with 1, like a syntax error.

# Language support
The texts and messages are in English (EN) and German (DE), option `-l`. See *samples/german* for German examples; the keys of the configuration files are English in every language. The texts and messages of the command line tool are in *apps/cli/texts*; the library itself has no texts.

# Build
smfigapl is written in c++20 and uses the cmake build system (3.20 or newer). It builds with gcc (13.3), clang (18) and MSVC (Visual Studio 2022). Build with
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Warnings are enabled for the project code; `-DSM_WARNINGS_AS_ERRORS=ON` turns them into errors, `-DSM_BUILD_TESTS=OFF` skips the tests, `-DSM_BUILD_CLI=OFF` the command line tool. `cmake --install build --prefix <dir>` installs the library (public headers, `find_package(smfigapl)`); `-DSM_BUILD_DOCS=ON` adds the target `docs` (Doxygen, pages in *build/docs/html*).

# Library
The library can be used by other programs: `find_package(smfigapl)` or FetchContent, target `smfigapl::smfigapl`, headers *include/smfigapl/*. See *docs/guides/getting_started.md* (also in the generated documentation), the example *examples/consumer*, the versioning rules in *docs/guides/versioning.md* and the changes in *CHANGELOG.md*. The external libraries are downloaded by CMake (FetchContent) when configuring, so the first configure needs network access (see *extern/CMakeLists.txt* for the versions):
- [googletest](https://github.com/google/googletest) 1.18 (for the tests)
- [rapidyaml](https://github.com/biojppm/rapidyaml) 0.16 (for reading and writing the configuration files; compiled into the library)
- [cxxopts](https://github.com/jarro2783/cxxopts) 3.3 (for parsing the commandline options)

# Development
- Formatting: [astyle](https://astyle.sourceforge.net/) with *.astylerc*:
  `astyle --options=.astylerc --suffix=none --recursive "src/*.cpp" "src/*.hpp" "include/*.hpp" "apps/*.cpp" "apps/*.hpp" "test/*.cpp" "test/*.hpp" "tools/*.cpp"`.
  `git config blame.ignoreRevsFile .git-blame-ignore-revs` makes git blame skip the formatting commit.
- Static analysis: clang-tidy with *.clang-tidy* (configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`, then `clang-tidy -p build FILE`). Every finding is an error, the CI checks all files in *src/*, *apps/* and *test/* (*test/.clang-tidy* turns off two checks for the tests).
- Naming: classes and functions in `camelCase`/`PascalCase` without prefix (namespace `smfigapl`), private members end with `_`, counts start with `N` (`NRounds`).
- CI (*.github/workflows/ci.yml*): formatting check, build and tests with GCC, Clang, GCC with ASan/UBSan, GCC with ThreadSanitizer and MSVC, the library for Android (NDK) and iOS, the CMake package and FetchContent, clang-tidy. *docs.yml*: the documentation (an undocumented public API is an error), published on GitHub Pages.

# Test
The tests use googletest and are registered with CTest: run `ctest` in the build directory (or `bin/smtest` directly).
- *test/contract*: the public API on setups built in code (every condition, sub tournaments, statuses, problems with paths, determinism) and on the configuration files in *test/cfg_files/generic*: the conditions hold and the number of search steps does not grow beyond *steps.yaml* (with `SMFIGAPL_UPDATE_STEPS=1` a lower number is written back).
- *test/api*: the public types, the planner (limits, cancellation, progress, log, threads), the setup validation.
- *test/formats*: the YAML adapter, with round trip tests that every format adapter has to pass.
- *test/report*: the statistic and the text and CSV writers.
- *test/cli*: the texts and messages of the command line tool.
- *test/model*, *test/io*, *test/solver*, *test/search_output*: unit tests of the internals. Configurations are read from strings (*test/test_setup.hpp*); the condition tests solve small configurations and recount the result from the game plan.
- *test/golden*: the command line tool on known configurations, its whole output compared with *test/golden/expected* (`cmake --build build --target golden_update` writes them again after a reviewed change).

# Configuration
There are some compile time settings:
- *jumpFastLevelDown*, *fastJump*, *maxFieldTypeJump* (*sm_search_types.hpp*): jump predictions of the tree search. They should be on, switch them off only for debugging.
- *defaultLanguage* (*apps/cli/texts.hpp*): the default language, "DE".
- *readStdInWithoutArguments* (*apps/smfigapl.cpp*): read the configuration from stdin when the tool is started without arguments, instead of printing the help (default off). Without an input file (-i) the configuration is always read from stdin.

Debug output is switched at runtime with -d and -e (debug flags).

# Code structure
The library has a public API in *include/smfigapl/* (namespace `smfigapl`): the setup, the result, the statistic, the output layout, the problems (`Result`, `Problem`), the planner (`validate`, `solve`, `computeStatistic`), the YAML adapter (*formats/yaml.hpp*) and the output writers (*report/*). An application links the target `smfigapl::smfigapl` and sees only these headers. The command line tool (*apps/*) uses the public API only; its texts are its own (*apps/cli*).

Inside, the code is split into layers. Each layer is a library in its own folder under *src/*, with its internal headers in *src/LAYER/include/smfigapl/LAYER/*. A layer can only include the headers of the layers it depends on, so a dependency in the wrong direction does not compile.

| Layer | Library | Content | Depends on |
|---|---|---|---|
| types | smfigapl_types | the public types (*include/smfigapl*), the version | - |
| model | smfigapl_model | input (*sm_in*), the setup to the input (*sm_setup*: validation, `buildInput`), output and search results (*sm_out*), game plans by index (*sm_indexed*), sizes and mappings (*sm_capacity*), basic types and limits (*sm_types*), error codes (*sm_error_codes*), debug log on a `LogSink` (*sm_debug*), helpers (*sm_math*), errors (*sm_except*) | types |
| formats | smfigapl_formats | the YAML adapter: setup and layout to and from text | types, rapidyaml |
| solver | smfigapl_solver | sub tournaments (*sm_atomic*), candidate rounds (*sm_ts*) and their condition tables (*sm_tables*), conditions (*sm_cond*), tree search (*sm_search*), game plan of the result (*sm_search_result*), search state (*sm_search_types*) | model |
| api | smfigapl_api | the planner: `validate`, `solve`, `computeStatistic` | types (model, solver private) |
| report | smfigapl_report | the text and CSV writers | types |

*smfigapl_core* combines all internal layers for the unit tests and the benchmark tool.

# Algorithm
- The YAML adapter reads the configuration into the public setup (`TournamentSetup`); `validateSetup` checks it, `buildInput` calculates a representation of teams, fields, etc. based on numeric indices (*sm_in*), with the mapping to the names.
- the algorithm is basically a treesearch done in *sm_search*. All possible combinations for a round are calculated in *sm_ts*. Here, the Teams, fields are transformed to a  normalized  gameplan, where the number of teams is equal to to number of fields, and the fields are sorted that fieldtypes are grouped. Additional virtual teams /fields /fieldtype are used to fill up the gameplan. Then all principle possible pertubations are calculated. Home-Away positions are ignore here. 
- Conditions are calculated differentially during the tree search, to do as less calculations as possbible during the treesearch. The tables used for the counters are precalculated in *sm_tables* (`RoundTables`), the counters are in *sm_cond*. 
- The normalized order off the pertubations is used to calc jump tables, which are used to go fast forward in the treesearch if it is know from the current conditons the following steps won't match the conditions too.
- When all conditions are fullfilled, the search is stopped. The resulting gameplan is optimized to obtain a equl home/away distribution, and to avoid game repetitions in a row ( if repetitions are allowed).
- Finally the gameplan is transformed back to the public game plan by name (`SolveResult`); the statistic is calculated from it (`computeStatistic`) and the writers print it (*src/report*).
- Errors of the input are problems with a code, values and the path in the setup; the command line tool turns them into messages.

# Contribute
Ideas and bug reports are welcome as issues (for a bug please attach the cfg file); see [CONTRIBUTING.md](CONTRIBUTING.md) for pull requests.

# License

smfigapl is licensed under the terms of the MIT license - see LICENSE.

# Contact
smfigapl@gmail.com
