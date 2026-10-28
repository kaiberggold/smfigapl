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


// The statistic (computeStatistic) and the output writers (text, CSV) on
// plans built in code

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/planner.hpp"
#include "smfigapl/report/csv.hpp"
#include "smfigapl/report/text.hpp"

using namespace smfigapl;

namespace
{
    const TeamRef northRed{"North", "Red"};
    const TeamRef northBlue{"North", "Blue"};
    const TeamRef southRed{"South", "Red"};
    const TeamRef southBlue{"South", "Blue"};

    // 2 rounds on field 1 (type A) and 2 (type B)
    TournamentSetup setup()
    {
        TournamentSetup s;
        s.rounds = 2;
        s.fields = {{"1", "A", "x"}, {"2", "B", "x"}};
        s.clubs = {{"North", {"Red", "Blue"}}, {"South", {"Red", "Blue"}}};
        return s;
    }

    // round 1: North Red - North Blue on 1, South Red - South Blue on 2
    // round 2: the given game on 1, South Blue - North Blue on 2
    SolveResult result(const TeamRef& home, const TeamRef& away)
    {
        SolveResult r;
        r.status = SearchStatus::solved;
        r.plan.rounds = {{{{"1", northRed, northBlue}, {"2", southRed, southBlue}}, {}}, {{{"1", home, away}, {"2", southBlue, northBlue}}, {}}};
        r.plan.subTournaments = {{{"1", "2"}, {northRed, northBlue, southRed, southBlue}}};
        return r;
    }
}

TEST(statistic, gamesClubsFieldTypes)
{
    const Statistic s = computeStatistic(setup(), result(northRed, southRed));
    EXPECT_EQ(s.fieldTypes, (std::vector<std::string> {"A", "B"}));
    ASSERT_EQ(s.teams.size(), 4u);
    for (const TeamStatistic& t : s.teams)
    {
        EXPECT_EQ(t.games, 2u) << t.team.club << t.team.team;
        EXPECT_EQ(t.sameClubGames, 1u);
        EXPECT_EQ(t.repeatedGames, 0u);
        EXPECT_EQ(t.maxGamesAgainstOneTeam, 1u);
        EXPECT_EQ(t.pauses, 0u);
    }
    EXPECT_EQ(s.teams[0].gamesPerFieldType, (std::vector<std::uint32_t> {2, 0}));
    EXPECT_EQ(s.teams[1].gamesPerFieldType, (std::vector<std::uint32_t> {1, 1}));
    EXPECT_EQ(s.teams[3].gamesPerFieldType, (std::vector<std::uint32_t> {0, 2}));
}

TEST(statistic, homeAndAway)
{
    const Statistic s = computeStatistic(setup(), result(northRed, southRed));
    EXPECT_EQ(s.teams[0].homeGames, 2u);
    EXPECT_EQ(s.teams[1].homeGames, 0u);
    EXPECT_EQ(s.teams[1].awayGames, 2u);
    EXPECT_EQ(s.teams[2].homeGames, 1u);
    EXPECT_EQ(s.teams[3].awayGames, 1u);
}

TEST(statistic, repeatedGames)
{
    // round 2: North Blue - North Red again
    const Statistic s = computeStatistic(setup(), result(northBlue, northRed));
    EXPECT_EQ(s.teams[0].repeatedGames, 1u);
    EXPECT_EQ(s.teams[0].maxGamesAgainstOneTeam, 2u);
    EXPECT_EQ(s.teams[2].repeatedGames, 0u);
}

// A pause and an empty field are no games (a pause used to count as a game against the own club)
TEST(statistic, pausesAndEmptyFields)
{
    SolveResult r = result(northRed, southRed);
    r.plan.rounds[1].games[0] = {"1", std::nullopt, std::nullopt};
    r.plan.rounds[1].pauses = {northRed, southRed};
    const Statistic s = computeStatistic(setup(), r);
    EXPECT_EQ(s.teams[0].games, 1u);
    EXPECT_EQ(s.teams[0].pauses, 1u);
    EXPECT_EQ(s.teams[0].sameClubGames, 1u);
    EXPECT_EQ(s.teams[0].gamesPerFieldType, (std::vector<std::uint32_t> {1, 0}));
}

TEST(reportText, plan)
{
    SolveResult r = result(northRed, southRed);
    r.plan.rounds[1].games[0] = {"1", std::nullopt, std::nullopt};
    r.plan.rounds[1].pauses = {northRed, southRed};
    report::Labels labels;
    labels.round = "Runde";
    labels.pause = "Pause";
    std::ostringstream os;
    report::writeText(os, r.plan, labels);
    EXPECT_EQ(os.str(), "Runde: 1\n"
              "                  North Red : North Blue                    \n"
              "                  South Red : South Blue                    \n"
              "\n"
              "Runde: 2\n"
              "                         -  : -                             \n"
              "                 South Blue : North Blue                    \n"
              "                       Pause: North Red, South Red\n"
              "\n");
}

// With several sub tournaments: the fields and the pausing teams of each
TEST(reportText, bySubTournament)
{
    GamePlan plan;
    plan.rounds = {{{{"1", northRed, northBlue}, {"2", southRed, southBlue}}, {TeamRef{"North", "Green"}, TeamRef{"South", "Green"}}}};
    plan.subTournaments = {{{"1"}, {northRed, northBlue, {"North", "Green"}}}, {{"2"}, {southRed, southBlue, {"South", "Green"}}}};
    std::ostringstream os;
    report::writeText(os, plan, report::Labels{}, true);
    const std::string text = os.str();
    const std::size_t second = text.find("Tournament: 2");
    ASSERT_NE(second, std::string::npos);
    EXPECT_EQ(text.find("Tournament: 1"), 0u);
    EXPECT_NE(text.find("Break: North Green\n"), std::string::npos);
    EXPECT_NE(text.find("Break: South Green\n"), std::string::npos);
    EXPECT_LT(text.find("North Red"), second);
    EXPECT_GT(text.find("South Red"), second);
}

TEST(reportText, statistic)
{
    const Statistic s = computeStatistic(setup(), result(northRed, southRed));
    report::Labels labels;
    labels.games = "Spiele";
    std::ostringstream os;
    report::writeStatisticText(os, s, labels);
    std::istringstream lines(os.str());
    std::vector<std::string> rows;
    for (std::string line; std::getline(lines, line);)
        rows.push_back(line);
    // the teams, 6 numbers, 2 field types
    ASSERT_EQ(rows.size(), 9u);
    EXPECT_EQ(rows[0].substr(0, 30), "Team:" + std::string(25, ' '));
    EXPECT_EQ(rows[0].substr(30, 25), "North Red" + std::string(16, ' '));
    EXPECT_EQ(rows[1].substr(0, 31), "Spiele" + std::string(24, ' ') + "2");
    EXPECT_EQ(rows[7].substr(0, 31), "A" + std::string(29, ' ') + "2");
    EXPECT_EQ(rows[8].substr(0, 31), "B" + std::string(29, ' ') + "0");
}

TEST(reportCsv, resultMessageOnly)
{
    SolveResult r;
    r.status = SearchStatus::impossible;
    report::Labels labels;
    labels.impossible = "no plan";
    std::ostringstream os;
    report::writeCsv(os, setup(), r, nullptr, layout::OutputLayout{}, labels);
    std::istringstream lines(os.str());
    std::string first;
    std::getline(lines, first);
    EXPECT_EQ(first.substr(0, 8), "no plan,");
    // the empty table of the default size
    const std::string csv = os.str();
    EXPECT_EQ(std::ranges::count(csv, '\n'), 18);
}

TEST(reportCsv, gamesAtTheirPositions)
{
    layout::OutputLayout l;
    l.format.settings.fixedTournaments = 1;
    // the columns with their headers in the row above the games
    auto& col = l.format.tournament.columns.col;
    for (const auto& [key, x] : std::vector<std::pair<std::string, int>> {{"field_name", 0}, {"home_team", 1}, {"away_team", 2}, {"round", 3}, {"field_type", 4},
    {"start_time", 5}, {"end_time", 6}, {"pause_teams", 7}, {"home_team_text_color", 8}, {"away_team_text_color", 9}, {"pause_teams_text_color", 10}
})
    {
        col[key].xRel = x;
        col[key].yRel = -1;
        col[key].text = key;
    }
    l.format.gamePlan.basePos = {0, 2};
    l.format.tournament.title.xRel = 12;
    l.format.tournament.title.yRel = -1;
    l.format.tournament.title.text = "Tournament $0";
    std::ostringstream os;
    report::writeCsv(os, setup(), result(northRed, southRed), nullptr, l, report::Labels{});
    std::istringstream lines(os.str());
    std::vector<std::string> rows;
    for (std::string line; std::getline(lines, line);)
        rows.push_back(line);
    ASSERT_GT(rows.size(), 5u);
    EXPECT_EQ(rows[0].substr(0, 7), "Solved,");
    EXPECT_EQ(rows[1].substr(0, 47), "field_name,home_team,away_team,round,field_type");
    EXPECT_NE(rows[1].find(",Tournament 1,"), std::string::npos);
    EXPECT_EQ(rows[2].substr(0, 27), "1,North Red,North Blue,1,A,");
    EXPECT_EQ(rows[3].substr(0, 26), "2,South Red,South Blue,,B,");
    EXPECT_EQ(rows[4].substr(0, 26), "1,North Red,South Red,2,A,");
}
