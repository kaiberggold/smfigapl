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


// The result of a search as public SolveResult: the game plan by name,
// sub tournaments, status, reasons, steps

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "test_setup.hpp"

using namespace smfigapl;

namespace
{
    // Two sub tournaments (subtypes) of three teams each, one field each
    const std::string twoSubTournaments = "rounds: 3\n" + field("A", "Left", "1") + field("A", "Right", "2") +
                                          club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue", "Green"}) +
                                          "conditions:\n  subtype_deny:\n    - subtype: \"Left\"\n      teams: [{club: \"South\", team: \"Red\"}, {club: \"South\", team: \"Blue\"}, {club: \"South\", team: \"Green\"}]\n"
                                          "conditions:\n  subtype_deny:\n    - subtype: \"Right\"\n      teams: [{club: \"North\", team: \"Red\"}, {club: \"North\", team: \"Blue\"}, {club: \"North\", team: \"Green\"}]\n";

    SolveResult solved(TestSetup& t)
    {
        EXPECT_TRUE(t.solve());
        return t.out.solveResult(t.result, std::chrono::milliseconds(5));
    }
}

TEST(solveResult, gamePlanByName)
{
    TestSetup t(twoSubTournaments);
    const SolveResult r = solved(t);
    EXPECT_EQ(r.status, SearchStatus::solved);
    EXPECT_TRUE(r.reasons.empty());
    EXPECT_EQ(r.info.duration, std::chrono::milliseconds(5));
    ASSERT_EQ(r.plan.rounds.size(), 3u);
    const std::vector<TeamRef> north = {{"North", "Red"}, {"North", "Blue"}, {"North", "Green"}};
    const std::vector<TeamRef> south = {{"South", "Red"}, {"South", "Blue"}, {"South", "Green"}};
    for (const Round& round : r.plan.rounds)
    {
        ASSERT_EQ(round.games.size(), 2u);
        EXPECT_EQ(round.games[0].field, "1");
        EXPECT_EQ(round.games[1].field, "2");
        // every team plays or pauses once per round, North on field 1, South on field 2
        std::vector<TeamRef> teams = round.pauses;
        for (const Game& game : round.games)
        {
            ASSERT_TRUE(game.home && game.away);
            const auto& own = game.field == "1" ? north : south;
            for (const TeamRef& team : {game.home.value_or(TeamRef{}), game.away.value_or(TeamRef{})})
            {
                EXPECT_NE(std::ranges::find(own, team), own.end());
                teams.push_back(team);
            }
        }
        EXPECT_EQ(teams.size(), 6u);
        for (const TeamRef& team : north)
            EXPECT_EQ(std::ranges::count(teams, team), 1);
        for (const TeamRef& team : south)
            EXPECT_EQ(std::ranges::count(teams, team), 1);
    }
    EXPECT_EQ(r.plan.subTournaments, (std::vector<SubTournament> {{{"1"}, north}, {{"2"}, south}}));
    // the steps of every sub tournament
    ASSERT_EQ(r.info.steps.size(), 2u);
    EXPECT_EQ(r.info.steps[0] + r.info.steps[1], t.steps);
}

TEST(solveResult, emptyField)
{
    // 3 fields, 4 teams: one field is empty in every round
    TestSetup t("rounds: 2\n" + field("A", "x", "1") + field("A", "x", "2") + field("A", "x", "3") + club("North", {"Red", "Blue"}) +
                club("South", {"Red", "Blue"}) + "conditions:\n  max_empty_fields: 1\n");
    const SolveResult r = solved(t);
    for (const Round& round : r.plan.rounds)
    {
        EXPECT_EQ(std::ranges::count_if(round.games, [](const Game & g)
        {
            return !g.home && !g.away;
        }), 1);
        EXPECT_TRUE(round.pauses.empty());
    }
}

TEST(solveResult, impossible)
{
    TestSetup t("rounds: 3\n" + field("A", "x", "1") + field("A", "x", "2") + club("North", {"Red", "Blue", "Green"}) +
                club("South", {"Red", "Blue", "Green"}) + "conditions:\n  max_pauses: 0\n");
    EXPECT_FALSE(t.solve());
    const SolveResult r = t.out.solveResult(t.result, std::chrono::milliseconds(0));
    EXPECT_EQ(r.status, SearchStatus::impossible);
    ASSERT_FALSE(r.reasons.empty());
    EXPECT_EQ(r.reasons.front().code, ProblemCode::impossiblePauses);
    EXPECT_EQ(r.plan, GamePlan{});
}

// The same setup gives the same result
TEST(solveResult, deterministic)
{
    TestSetup a(twoSubTournaments);
    TestSetup b(twoSubTournaments);
    EXPECT_EQ(solved(a), solved(b));
}
