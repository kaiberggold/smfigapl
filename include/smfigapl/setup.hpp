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


/// @file setup.hpp
/// @brief The setup of a tournament: rounds, fields, clubs and teams, conditions.
///
/// The domain model is independent of any storage format: the
/// format adapters read it from and write it to text, the app can build it
/// directly. Everything is named, there are no indices.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace smfigapl
{
    /// A field: its name in the plan, its type (for the field type
    /// conditions) and its subtype (for the teams it is denied to; fields
    /// of subtypes that no team shares form separate sub tournaments).
    struct Field
    {
        std::string name; ///< The name of the field in the plan.
        std::string type;    ///< The field type (for the field type conditions and the statistic).
        std::string subtype; ///< The subtype (for the teams denied to it).
        friend bool operator==(const Field&, const Field&) = default;
    };

    /// A club and the names of its teams (today: colors, e.g. "Red").
    struct Club
    {
        std::string name;               ///< The name of the club.
        std::vector<std::string> teams; ///< The names of its teams, unique within the club.
        friend bool operator==(const Club&, const Club&) = default;
    };

    /// A team: the name of its club and its own name within the club.
    struct TeamRef
    {
        std::string club; ///< The name of the club.
        std::string team; ///< The name of the team within the club.
        friend bool operator==(const TeamRef&, const TeamRef&) = default;
    };

    /// How many games every team plays on the fields of a type.
    struct FieldTypeLimit
    {
        std::string type;                 ///< The field type (Field::type).
        std::optional<std::uint32_t> min; ///< At least this many games; none: no minimum.
        std::optional<std::uint32_t> max; ///< At most this many games; none: no maximum.
        friend bool operator==(const FieldTypeLimit&, const FieldTypeLimit&) = default;
    };

    /// Teams that may not play on the fields of a subtype.
    struct SubtypeDeny
    {
        std::string subtype;        ///< The subtype (Field::subtype).
        std::vector<TeamRef> teams; ///< The teams that may not play on its fields.
        friend bool operator==(const SubtypeDeny&, const SubtypeDeny&) = default;
    };

    /// The conditions of the plan; an empty optional means no condition.
    struct Conditions
    {
        /// The most rounds a team pauses.
        std::optional<std::uint32_t> maxPauses;
        /// The most empty fields in a round.
        std::uint32_t maxEmptyFields = 0;
        /// The most games of a team against teams of its own club.
        std::optional<std::uint32_t> maxSameClubGames;
        /// Games per team on the fields of a type.
        std::vector<FieldTypeLimit> fieldTypes;
        /// Teams that may not play on fields of a subtype.
        std::vector<SubtypeDeny> subtypeDeny;
        friend bool operator==(const Conditions&, const Conditions&) = default;
    };

    /// The setup of a tournament.
    struct TournamentSetup
    {
        /// The number of rounds.
        std::uint32_t rounds = 0;
        /// Pairings may be played again, at most iterations times.
        bool repeatPairings = false;
        /// How often a pairing may be played (with repeatPairings).
        std::uint32_t iterations = 1;
        /// Swap home and away so that every team has about as many of both.
        bool homeAwayBalance = true;
        /// Order the rounds so that no team pauses in two rounds in a row
        /// and no game repeats on a field in the next round.
        bool optimizeRoundOrder = true;
        /// The fields in the order of the plan.
        std::vector<Field> fields;
        /// The clubs with their teams.
        std::vector<Club> clubs;
        /// The conditions of the plan.
        Conditions conditions;
        friend bool operator==(const TournamentSetup&, const TournamentSetup&) = default;
    };
}
