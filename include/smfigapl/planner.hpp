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


/// @file planner.hpp
/// @brief The planner: validate a setup, search its game plan, compute the
/// statistic of a plan.
///
/// The functions do not throw and have no shared state: they can run on a
/// worker thread, several at once. A search is stopped by its step or time
/// limit or by the caller (SolveOptions::stopToken).

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <vector>

#include "smfigapl/log.hpp"
#include "smfigapl/plan.hpp"
#include "smfigapl/result.hpp"
#include "smfigapl/setup.hpp"
#include "smfigapl/statistic.hpp"

namespace smfigapl
{
    /// How far a search is.
    struct Progress
    {
        /// The sub tournament being searched (they are searched one after another).
        std::uint32_t subTournament = 0;
        /// The steps of this sub tournament so far.
        std::uint64_t steps = 0;
        /// True once for every sub tournament, when its search ends.
        bool finished = false;
        friend bool operator==(const Progress&, const Progress&) = default;
    };

    /// The settings of a search.
    struct SolveOptions
    {
        /// The most steps of the search of a sub tournament (status aborted
        /// when reached); none: no limit.
        std::optional<std::uint32_t> maxSteps;
        /// The longest time of the search (status aborted when reached).
        std::optional<std::chrono::milliseconds> timeLimit;
        /// A stop request ends the search soon with status cancelled.
        std::stop_token stopToken;
        /// Called from the thread of the search every progressInterval steps
        /// and at the end of every sub tournament; it should return quickly.
        std::function<void(const Progress&)> progress;
        /// The steps between two progress calls.
        std::uint32_t progressInterval = 1000;
        /// The debug messages of the search; must outlive the call. None:
        /// no messages, and nothing is formatted.
        LogSink* log = nullptr;
    };

    /// All problems of a setup: errors with their path in the setup. Empty
    /// if solve() can search it (the size of the search is checked by
    /// solve(), ProblemCode::searchTooLarge and searchTooManyPairings).
    [[nodiscard]] std::vector<Problem> validate(const TournamentSetup& setup) noexcept;

    /// Searches the game plan of a setup. Without a value if the setup has
    /// errors (the problems of validate(), or a search that is too large)
    /// or an internal error happened (ProblemCode::internal). With a value,
    /// the status says whether there is a plan; the same setup and options
    /// give the same result (apart from the duration).
    [[nodiscard]] Result<SolveResult> solve(const TournamentSetup& setup, const SolveOptions& options = {}) noexcept;

    /// The statistic of the game plan of a result of solve() for setup
    /// (empty numbers if the result has no plan).
    [[nodiscard]] Statistic computeStatistic(const TournamentSetup& setup, const SolveResult& result);
}
