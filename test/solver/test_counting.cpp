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

// The counting checks (before and during the search), the last round check
// and the result of the search: solved, impossible (with the reason) or
// aborted at a limit

#include <chrono>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "test_setup.hpp"
#include "test_misc.hpp"

using namespace smfigapl;

namespace
{
    // `fields` fields on `types` field types (round robin), two clubs of
    // `teams` teams in total, as the generated benchmark configurations
    std::string generated(uint fields, uint teams, uint types, uint rounds, const std::string& conditions = "")
    {
        std::string config = "rounds: " + std::to_string(rounds) + "\n";
        for (uint f = 0; f < fields; f++)
            config += field("T" + std::to_string(f % types), "x", std::to_string(f + 1));
        std::vector<std::string> north;
        std::vector<std::string> south;
        for (uint t = 0; t < teams; t++)
            (t < (teams + 1) / 2 ? north : south).push_back("c" + std::to_string(t));
        config += club("North", north);
        if (!south.empty())
            config += club("South", south);
        return config + conditions;
    }

    // the reason of the first (sub) tournament without a game plan
    ErrorCode reason(const TestSetup& t)
    {
        const std::vector<Exception> reasons = t.result.reasons();
        return reasons.empty() ? ErrorCode::none : reasons.front().code();
    }

    const std::string tight = "conditions:\n  field_types:\n    - {type: \"T0\", min: 3, max: 4}\nconditions:\n  field_types:\n    - {type: \"T1\", min: 3, max: 4}\n";
}

// Configurations without a game plan that a count shows are answered
// before the search (they took up to 100 million steps or more)
TEST(counting, impossibleBeforeTheSearch)
{
    // every team 4 of 7 games on T0, but T0 has 2 of 4 fields: 28 places for 32 games
    TestSetup minimum(generated(4, 8, 2, 7, "conditions:\n  field_types:\n    - {type: \"T0\", min: 4}\n"));
    EXPECT_FALSE(minimum.solve());
    EXPECT_EQ(minimum.result.status(), SearchStatus::impossible);
    EXPECT_EQ(reason(minimum), ErrorCode::impossibleFieldTypeMin);
    ASSERT_FALSE(minimum.result.reasons().empty());
    EXPECT_EQ(minimum.result.reasons().front().params(), (std::vector<std::string> {"T0", "28", "32"}));
    EXPECT_EQ(minimum.steps, 0u);

    // 10 teams, 3-4 of 7 games on each type, T0 has 3 of 5 fields: 42 places
    // to fill, the teams may play at most 40 games on it
    TestSetup types(generated(5, 10, 2, 7, tight));
    EXPECT_FALSE(types.solve());
    EXPECT_EQ(reason(types), ErrorCode::impossibleFieldTypeMax);

    // at most 2 games on T0 (the only field type), but every team plays 3 rounds
    TestSetup maximum(generated(4, 8, 1, 3, "conditions:\n  field_types:\n    - {type: \"T0\", max: 2}\n"));
    EXPECT_FALSE(maximum.solve());
    EXPECT_EQ(reason(maximum), ErrorCode::impossibleTeamGames);

    // every round needs a game (at most 2 of 3 fields empty), but each of
    // the 4 club mates may play only one club game
    const std::string threeFields = field("T0", "x", "1") + field("T0", "x", "2") + field("T0", "x", "3");
    TestSetup games("rounds: 3\n" + threeFields + club("C", {"a", "b", "c", "d"}) + "conditions:\n  max_pauses: 2\nconditions:\n  max_empty_fields: 2\nconditions:\n  max_same_club_games: 1\n");
    EXPECT_FALSE(games.solve());
    EXPECT_EQ(reason(games), ErrorCode::impossibleGames);

    // an odd number of teams without pauses
    TestSetup pauses(generated(3, 5, 1, 5, "conditions:\n  max_pauses: 0\nconditions:\n  max_empty_fields: 1\n"));
    EXPECT_FALSE(pauses.solve());
    EXPECT_EQ(reason(pauses), ErrorCode::impossiblePauses);

    // at most one club game, but 7 games of 8 teams meet the 3 club mates
    TestSetup club(generated(4, 8, 1, 7, "conditions:\n  max_same_club_games: 1\n"));
    EXPECT_FALSE(club.solve());
    EXPECT_EQ(reason(club), ErrorCode::impossibleSameClub);

    // more rounds than opponents (the search ran past its candidates)
    TestSetup opponents(generated(2, 4, 1, 4));
    EXPECT_FALSE(opponents.solve());
    EXPECT_EQ(reason(opponents), ErrorCode::impossibleOpponents);
}

// A search that tries all possibilities proves that there is no game plan:
// one field that team a may not use, so the only round is a pause of both,
// and the plan needs two different rounds
TEST(counting, impossibleBySearch)
{
    TestSetup t("rounds: 2\n" + field("A", "y", "1") + club("C", {"a", "b"}) + "conditions:\n  max_empty_fields: 1\nconditions:\n  subtype_deny:\n    - subtype: \"y\"\n      teams: [{club: \"C\", team: \"a\"}]\n");
    EXPECT_FALSE(t.solve());
    EXPECT_GT(t.steps, 0u);
    EXPECT_EQ(t.result.status(), SearchStatus::impossible);
    EXPECT_EQ(reason(t), ErrorCode::impossibleSearch);
}

// The step and the time limit abort the search without an answer
TEST(counting, aborted)
{
    TestSetup steps(generated(4, 8, 2, 7, tight));
    EXPECT_FALSE(steps.solve(100));
    EXPECT_EQ(steps.result.status(), SearchStatus::aborted);
    EXPECT_EQ(steps.result.getResult(0), StepResult::failedMaxSteps);
    EXPECT_TRUE(steps.result.reasons().empty());

    TestSetup time(generated(4, 8, 2, 7, tight));
    ASSERT_TRUE(time.valid());
    GamePlanSearch search{time.in, time.out};
    search.setTimeLimit(std::chrono::milliseconds(0));
    const ResultArray result = search.gpSearch();
    EXPECT_EQ(result.status(), SearchStatus::aborted);
    EXPECT_EQ(search.getSteps(), 1u);
    EXPECT_EQ(time.out.status(), SearchStatus::aborted);
}

// Tight field types (3 or 4 of 7 games on each type): the counts never fail,
// the last round check cuts the search (58.7 million steps without it)
TEST(counting, lastRound)
{
    TestSetup t(generated(4, 8, 2, 7, tight));
    ASSERT_TRUE(t.solve());
    EXPECT_EQ(t.result.status(), SearchStatus::solved);
    EXPECT_EQ(t.out.status(), SearchStatus::solved);
    // 2605 steps without the last round check, the same plan
    EXPECT_EQ(t.steps, 1420u);
}

// Every team pauses in every round (one field, the only team of a club
// denied on it): the home and away balance had no game to swap (crash)
TEST(counting, onlyEmptyFields)
{
    TestSetup t("rounds: 1\n" + field("T0", "y", "1") + club("C", {"a", "b"}) + "conditions:\n  max_empty_fields: 1\nconditions:\n  subtype_deny:\n    - subtype: \"y\"\n      teams: [{club: \"C\", team: \"a\"}]\n");
    EXPECT_TRUE(t.solve());
}

// Teams that are interchangeable for all conditions (here: the teams of a
// club, with a club limit) make rounds of the same kind equivalent: only the
// first round of every kind is tried as the first round of the plan. Without
// that the search needed 28.4 million steps to prove there is no plan.
TEST(counting, symmetryOfTheTeams)
{
    const std::string fields = field("T0", "y", "1") + field("T1", "y", "2") + field("T2", "y", "3") + field("T0", "y", "4") + field("T1", "y", "5");
    const std::string clubs = club("C0", {"k0", "k1"}) + club("C1", {"k0", "k1", "k2", "k3", "k4", "k5", "k6"});
    std::string conditions = "conditions:\n  max_empty_fields: 1\nconditions:\n  max_same_club_games: 1\n";
    conditions += "conditions:\n  field_types:\n    - {type: \"T0\", min: 3, max: 7}\nconditions:\n  field_types:\n    - {type: \"T1\", min: 2, max: 5}\n";
    TestSetup t("rounds: 7\n" + fields + clubs + conditions);
    EXPECT_FALSE(t.solve(1000000));
    EXPECT_EQ(t.result.status(), SearchStatus::impossible);
    EXPECT_EQ(reason(t), ErrorCode::impossibleSearch);
    EXPECT_LT(t.steps, 200000u);
}
