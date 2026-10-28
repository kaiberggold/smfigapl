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

// The planner on the internal model: setup -> Input -> search -> SolveResult

#include <algorithm>
#include <chrono>
#include <exception>
#include <limits>
#include <new>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "smfigapl/planner.hpp"
#include "smfigapl/model/sm_setup.hpp"
#include "smfigapl/model/sm_except.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_out.hpp"
#include "smfigapl/solver/sm_search.hpp"

namespace
{
    smfigapl::Problem problem(smfigapl::ProblemCode code, std::vector<std::string> parameters)
    {
        smfigapl::Problem p;
        p.code = code;
        p.parameters = std::move(parameters);
        return p;
    }

    smfigapl::Result<smfigapl::SolveResult> search(const smfigapl::TournamentSetup& setup, const smfigapl::SolveOptions& options)
    {
        using namespace smfigapl;
        std::vector<Problem> problems = validateSetup(setup);
        if (!problems.empty())
            return Result<SolveResult>::failure(std::move(problems));
        Input in;
        in.dbg = DebugLog{options.log};
        buildInput(setup, in);
        Output out(in);
        GamePlanSearch gps(in, out);
        if (options.maxSteps)
            gps.setMaxSteps(*options.maxSteps);
        if (options.timeLimit)
            gps.setTimeLimit(*options.timeLimit);
        gps.setStopToken(options.stopToken);
        if (options.progress)
            gps.setProgressCallback(std::max(options.progressInterval, std::uint32_t{1}), [&options](const SearchProgress & p)
        {
            options.progress(Progress{p.atomicIndex, p.step, p.finished});
        });
        const auto start = std::chrono::steady_clock::now();
        const ResultArray result = gps.gpSearch();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
        // the failed checks of the conditions, for the debug output
        if (options.log != nullptr && options.log->enabled(LogLevel::debug, LogCategory::general))
        {
            in.dbg.out() << std::endl;
            gps.dispNFailArray();
        }
        return out.solveResult(result, duration);
    }
}

std::vector<smfigapl::Problem> smfigapl::validate(const TournamentSetup& setup) noexcept
{
    try
    {
        return validateSetup(setup);
    }
    catch (const std::exception& e)
    {
        return {problem(ProblemCode::internal, {e.what()})};
    }
    catch (...)
    {
        return {problem(ProblemCode::internal, {"unknown exception"})};
    }
}

smfigapl::Result<smfigapl::SolveResult> smfigapl::solve(const TournamentSetup& setup, const SolveOptions& options) noexcept
{
    try
    {
        return search(setup, options);
    }
    catch (const Exception& e)
    {
        // an error of the setup found by the search (too large)
        return Result<SolveResult>::failure(problem(e.code(), e.params()));
    }
    catch (const std::exception& e)
    {
        return Result<SolveResult>::failure(problem(ProblemCode::internal, {e.what()}));
    }
    catch (...)
    {
        return Result<SolveResult>::failure(problem(ProblemCode::internal, {"unknown exception"}));
    }
}
