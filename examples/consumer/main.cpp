// https://github.com/kaiberggold/smfigapl
//
// MIT License
//
// Copyright 2020 Kai Berggold
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


// An application of the library: a setup built in code, the game plan, the
// statistic and the plan as text, the setup as YAML. Exit code 0 if a plan
// was found.

#include <iostream>

#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/planner.hpp"
#include "smfigapl/report/text.hpp"
#include "smfigapl/version.hpp"

int main()
{
    using namespace smfigapl;
    std::cout << "smfigapl " << VERSION_STRING << "\n\n";

    TournamentSetup setup;
    setup.rounds = 3;
    setup.fields = {{"1", "Small", "x"}, {"2", "Large", "x"}};
    setup.clubs = {{"North", {"Red", "Blue"}}, {"South", {"Red", "Blue"}}};
    setup.conditions.maxSameClubGames = 1;

    for (const Problem& p : validate(setup))
        std::cerr << "problem " << static_cast<int>(p.code) << " at " << p.path << "\n";

    SolveOptions options;
    options.maxSteps = 100000;
    const Result<SolveResult> result = solve(setup, options);
    if (!result || result->status != SearchStatus::solved)
    {
        std::cerr << "no game plan\n";
        return 1;
    }
    report::writeText(std::cout, result->plan, report::Labels{});
    report::writeStatisticText(std::cout, computeStatistic(setup, *result), report::Labels{});
    std::cout << "\n" << formats::yaml::writeSetup(setup);
    return 0;
}
