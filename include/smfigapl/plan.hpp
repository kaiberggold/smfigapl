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


/// @file plan.hpp
/// @brief The result of a search: its status, the reasons, the game plan.

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "smfigapl/result.hpp"
#include "smfigapl/setup.hpp"

namespace smfigapl
{
    /// How a search ended.
    enum class SearchStatus : std::uint8_t
    {
        solved,     ///< a game plan was found
        impossible, ///< there is no game plan (proven; the reasons say why)
        aborted,    ///< a step or time limit stopped the search, it is open whether there is a plan
        cancelled   ///< the caller cancelled the search
    };

    /// What happens on a field in a round: a game, or nothing (empty field).
    struct Game
    {
        /// The name of the field.
        std::string field;
        /// The home team; empty (with away) if the field is empty in this round.
        std::optional<TeamRef> home;
        /// The away team; empty (with home) if the field is empty in this round.
        std::optional<TeamRef> away;
        friend bool operator==(const Game&, const Game&) = default;
    };

    /// A round: every field in the order of the setup, the pausing teams.
    struct Round
    {
        std::vector<Game> games;     ///< A game (or an empty field) for every field.
        std::vector<TeamRef> pauses; ///< The teams without a game, by sub tournament.
        friend bool operator==(const Round&, const Round&) = default;
    };

    /// Fields and teams that play only among themselves (subtypes with
    /// teams denied to them); a setup without that is one sub tournament.
    struct SubTournament
    {
        std::vector<std::string> fields; ///< Its fields, in the order of the setup.
        std::vector<TeamRef> teams;      ///< Its teams, in the order of the setup.
        friend bool operator==(const SubTournament&, const SubTournament&) = default;
    };

    /// The game plan: the rounds in their order, the sub tournaments.
    struct GamePlan
    {
        std::vector<Round> rounds;                 ///< The rounds in the order they are played.
        std::vector<SubTournament> subTournaments; ///< The sub tournaments (at least one if there are rounds).
        friend bool operator==(const GamePlan&, const GamePlan&) = default;
    };

    /// How the search went.
    struct SearchInfo
    {
        /// The search steps of every sub tournament.
        std::vector<std::uint64_t> steps;
        /// The time of the whole search.
        std::chrono::milliseconds duration{0};
        friend bool operator==(const SearchInfo&, const SearchInfo&) = default;
    };

    /// The result of a search. The same setup and library version give the
    /// same result (apart from the duration).
    struct SolveResult
    {
        /// How the search ended.
        SearchStatus status = SearchStatus::impossible;
        /// Why there is no game plan (status impossible).
        std::vector<Problem> reasons;
        /// The game plan if solved; if aborted or cancelled the rounds where
        /// the search stopped (not a valid plan); empty if impossible.
        GamePlan plan;
        /// The steps and the time of the search.
        SearchInfo info;
        friend bool operator==(const SolveResult&, const SolveResult&) = default;
    };
}
