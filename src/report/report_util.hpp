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

// Helpers of the output writers

#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "smfigapl/plan.hpp"
#include "smfigapl/statistic.hpp"
#include "smfigapl/report/labels.hpp"

namespace smfigapl::report::detail
{
    // "Club Team"
    inline std::string teamName(const TeamRef& team)
    {
        return team.club + " " + team.team;
    }

    // An empty field shows "- " (club "-" without team, as it always did)
    inline std::string teamName(const std::optional<TeamRef>& team)
    {
        return team ? teamName(*team) : "- ";
    }

    // The sub tournament of every field (the fields of the rounds, in order)
    inline std::vector<std::size_t> subTournamentOfFields(const GamePlan& plan)
    {
        std::vector<std::size_t> result;
        if (plan.rounds.empty())
            return result;
        // the fields of every sub tournament are in the order of the plan
        std::vector<std::size_t> next(plan.subTournaments.size(), 0);
        for (const Game& game : plan.rounds.front().games)
        {
            std::size_t sub = 0;
            for (std::size_t t = 0; t < plan.subTournaments.size(); t++)
            {
                const auto& fields = plan.subTournaments[t].fields;
                if (next[t] < fields.size() && fields[next[t]] == game.field)
                {
                    sub = t;
                    next[t]++;
                    break;
                }
            }
            result.push_back(sub);
        }
        return result;
    }

    // A row of the statistic: its label (format key, text) and the numbers of the teams
    struct StatisticRow
    {
        std::string key;
        std::string label;
        std::vector<std::uint32_t> values;
    };

    // The rows in the order of the output; the field type rows have the
    // key "field_types" and the name of the type as label
    inline std::vector<StatisticRow> statisticRows(const Statistic& s, const Labels& labels)
    {
        auto row = [&s](std::string key, std::string label, auto value)
        {
            StatisticRow r{std::move(key), std::move(label), {}};
            for (const TeamStatistic& t : s.teams)
                r.values.push_back(value(t));
            return r;
        };
        std::vector<StatisticRow> rows =
        {
            row("n_games", labels.games, [](const TeamStatistic & t) { return t.games; }),
            row("n_same_pairing", labels.repeatedGames, [](const TeamStatistic & t) { return t.repeatedGames; }),
            row("max_games_same_team", labels.maxGamesAgainstOneTeam, [](const TeamStatistic & t) { return t.maxGamesAgainstOneTeam; }),
            row("same_club", labels.sameClubGames, [](const TeamStatistic & t) { return t.sameClubGames; }),
            row("home_games", labels.homeGames, [](const TeamStatistic & t) { return t.homeGames; }),
            row("away_games", labels.awayGames, [](const TeamStatistic & t) { return t.awayGames; })
        };
        for (std::size_t ft = 0; ft < s.fieldTypes.size(); ft++)
            rows.push_back(row("field_types", s.fieldTypes[ft], [ft](const TeamStatistic & t)
        {
            return ft < t.gamesPerFieldType.size() ? t.gamesPerFieldType[ft] : 0U;
        }));
        return rows;
    }
}
