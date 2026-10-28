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

// The statistic of a game plan, on the public types only

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "smfigapl/planner.hpp"

namespace
{
    template<class Container, class Value> std::size_t indexOf(const Container& c, const Value& v)
    {
        return static_cast<std::size_t>(std::distance(c.begin(), std::ranges::find(c, v)));
    }
}

namespace
{
    // The counts of a game for both teams (indices in Statistic::teams and fieldTypes)
    void addGame(smfigapl::Statistic& s, std::vector<std::map<std::size_t, std::uint32_t>>& opponents, bool sameClub,
                 std::size_t home, std::size_t away, std::size_t type)
    {
        for (const std::size_t t : {home, away})
        {
            smfigapl::TeamStatistic& stat = s.teams[t];
            stat.games++;
            if (type < s.fieldTypes.size())
                stat.gamesPerFieldType[type]++;
            if (sameClub)
                stat.sameClubGames++;
        }
        s.teams[home].homeGames++;
        s.teams[away].awayGames++;
        opponents[home][away]++;
        opponents[away][home]++;
    }

    // The field types and the teams of a setup, without games
    smfigapl::Statistic emptyStatistic(const smfigapl::TournamentSetup& setup, std::vector<smfigapl::TeamRef>& teams)
    {
        smfigapl::Statistic s;
        for (const smfigapl::Field& f : setup.fields)
            if (std::ranges::find(s.fieldTypes, f.type) == s.fieldTypes.end())
                s.fieldTypes.push_back(f.type);
        for (const smfigapl::Club& c : setup.clubs)
            for (const std::string& team : c.teams)
            {
                teams.push_back({c.name, team});
                smfigapl::TeamStatistic& t = s.teams.emplace_back();
                t.team = teams.back();
                t.gamesPerFieldType.assign(s.fieldTypes.size(), 0);
            }
        return s;
    }
}

smfigapl::Statistic smfigapl::computeStatistic(const TournamentSetup& setup, const SolveResult& result)
{
    std::vector<TeamRef> teams;
    Statistic s = emptyStatistic(setup, teams);
    // the opponents of every team, with the number of games against them
    std::vector<std::map<std::size_t, std::uint32_t>> opponents(teams.size());

    for (const Round& round : result.plan.rounds)
    {
        for (std::size_t j = 0; j < round.games.size(); j++)
        {
            const Game& game = round.games[j];
            if (!game.home || !game.away)
                continue;
            const TeamRef& homeTeam = *game.home;
            const TeamRef& awayTeam = *game.away;
            const std::size_t home = indexOf(teams, homeTeam);
            const std::size_t away = indexOf(teams, awayTeam);
            const std::size_t type = j < setup.fields.size() ? indexOf(s.fieldTypes, setup.fields[j].type) : s.fieldTypes.size();
            if (home < teams.size() && away < teams.size())
                addGame(s, opponents, homeTeam.club == awayTeam.club, home, away, type);
        }
        for (const TeamRef& pause : round.pauses)
            if (const std::size_t t = indexOf(teams, pause); t < teams.size())
                s.teams[t].pauses++;
    }
    for (std::size_t t = 0; t < teams.size(); t++)
    {
        TeamStatistic& stat = s.teams[t];
        stat.repeatedGames = stat.games - static_cast<std::uint32_t>(opponents[t].size());
        for (const auto& [opponent, games] : opponents[t])
            stat.maxGamesAgainstOneTeam = std::max(stat.maxGamesAgainstOneTeam, games);
    }
    return s;
}
