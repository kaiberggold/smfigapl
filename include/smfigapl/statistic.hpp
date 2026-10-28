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


/// @file statistic.hpp
/// @brief The statistic of a game plan per team, as data.
///
/// The labels of the rows are not part of the API: a client names them in
/// its own language.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "smfigapl/setup.hpp"

namespace smfigapl
{
    /// The numbers of one team.
    struct TeamStatistic
    {
        TeamRef team;                ///< The team.
        std::uint32_t games = 0;     ///< Its games.
        std::uint32_t homeGames = 0; ///< Its games as home team.
        std::uint32_t awayGames = 0; ///< Its games as away team.
        std::uint32_t pauses = 0;    ///< The rounds without a game.
        /// Games against teams of the own club.
        std::uint32_t sameClubGames = 0;
        /// Games against an opponent met before.
        std::uint32_t repeatedGames = 0;
        /// The most games against one opponent.
        std::uint32_t maxGamesAgainstOneTeam = 0;
        /// Games per field type, in the order of Statistic::fieldTypes.
        std::vector<std::uint32_t> gamesPerFieldType;
        friend bool operator==(const TeamStatistic&, const TeamStatistic&) = default;
    };

    /// The statistic of a game plan.
    struct Statistic
    {
        /// The field types in the order of the setup's fields (first appearance).
        std::vector<std::string> fieldTypes;
        /// Every team, in the order of the setup's clubs and teams.
        std::vector<TeamStatistic> teams;
        friend bool operator==(const Statistic&, const Statistic&) = default;
    };
}
