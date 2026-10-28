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


/// @file report/labels.hpp
/// @brief The texts of the output writers.
///
/// The library has no texts of its own: the caller (command line tool,
/// app) gives them in its language. The defaults are English.

#pragma once

#include <string>

namespace smfigapl::report
{
    /// The texts the writers put into the output.
    struct Labels
    {
        std::string tournament = "Tournament"; ///< The title of a sub tournament ("Tournament: 1").
        std::string round = "Round";           ///< The title of a round ("Round: 1").
        /// The teams that pause in a round.
        std::string pause = "Break";
        /// The header of the team row of the statistic.
        std::string team = "Team:";

        std::string solved = "Solved";                       ///< The result: SearchStatus::solved.
        std::string impossible = "No game plan possible";    ///< The result: SearchStatus::impossible.
        /// The result: SearchStatus::aborted.
        std::string aborted = "Search aborted (limit reached), unknown whether a game plan exists";
        std::string cancelled = "Search cancelled";          ///< The result: SearchStatus::cancelled.

        // the rows of the statistic; the field type rows have the names of the types
        std::string games = "Number of games";                  ///< Statistic row: TeamStatistic::games.
        std::string repeatedGames = "Repeated Games";           ///< Statistic row: TeamStatistic::repeatedGames.
        std::string maxGamesAgainstOneTeam = "Max Games same Team"; ///< Statistic row: TeamStatistic::maxGamesAgainstOneTeam.
        std::string sameClubGames = "Same Club";                ///< Statistic row: TeamStatistic::sameClubGames.
        std::string homeGames = "Number of home games";         ///< Statistic row: TeamStatistic::homeGames.
        std::string awayGames = "Number of away games";         ///< Statistic row: TeamStatistic::awayGames.

        friend bool operator==(const Labels&, const Labels&) = default;
    };
}
