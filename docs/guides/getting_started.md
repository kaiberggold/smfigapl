# Getting started {#getting_started}

## Add the library

With FetchContent (CMake 3.20 or newer, a C++20 compiler):

```cmake
include(FetchContent)
FetchContent_Declare(smfigapl
    GIT_REPOSITORY https://github.com/kaiberggold/smfigapl.git
    GIT_TAG v0.5.0)
FetchContent_MakeAvailable(smfigapl)
target_link_libraries(my_app PRIVATE smfigapl::smfigapl)
```

Added this way, only the library is built (no tests, no command line
tool). Or install it (`cmake --install <build> --prefix <dir>`) and use

```cmake
find_package(smfigapl 0.5 REQUIRED)
target_link_libraries(my_app PRIVATE smfigapl::smfigapl)
```

*examples/consumer* is a complete project for both ways.

## The first game plan

```cpp
#include "smfigapl/planner.hpp"
#include "smfigapl/report/text.hpp"

smfigapl::TournamentSetup setup;
setup.rounds = 3;
setup.fields = {{"1", "Small", "x"}, {"2", "Large", "x"}};
setup.clubs = {{"North", {"Red", "Blue"}}, {"South", {"Red", "Blue"}}};
setup.conditions.maxSameClubGames = 1;

const auto result = smfigapl::solve(setup);
if (result && result->status == smfigapl::SearchStatus::solved)
    smfigapl::report::writeText(std::cout, result->plan, smfigapl::report::Labels{});
```

- smfigapl::validate gives all problems of a setup with their path (e.g.
  `conditions.fieldTypes[0].type`), for showing them at the input.
- smfigapl::solve has no value if the setup has errors (the problems say
  which). With a value, the status says whether there is a plan; without
  one, the reasons say why (e.g. ProblemCode::impossiblePauses).
- Neither throws. A search runs on the calling thread; several searches
  can run at once on different threads.

## Long searches

A search can take long for large setups. smfigapl::SolveOptions has a
step and a time limit (status aborted), a `std::stop_token` (status
cancelled, checked every few thousand steps) and a progress callback:

```cpp
std::jthread worker([&](std::stop_token stop) {
    smfigapl::SolveOptions options;
    options.stopToken = stop;
    options.progressInterval = 10000;
    options.progress = [](const smfigapl::Progress& p) { /* show p.steps */ };
    result = smfigapl::solve(setup, options);
});
// worker.request_stop() cancels the search
```

## Messages

Problems are codes with values (smfigapl::Problem::parameters); the
meaning of the values is at each smfigapl::ProblemCode. The application
writes the messages in its language.
