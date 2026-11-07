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

// The check of a given plan (checkPlan): a solved plan keeps every
// condition, plans changed by hand break them.

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/planner.hpp"

using namespace smfigapl;

static_assert(noexcept(checkPlan(TournamentSetup{}, GamePlan{})));

namespace
{
    TournamentSetup setup()
    {
        TournamentSetup s;
        s.rounds = 5;
        s.fields = {{"1", "Small", "x"}, {"2", "Large", "x"}};
        s.clubs = {{"North", {"Red", "Blue", "Green"}}, {"South", {"Red", "Blue"}}};
        s.conditions.maxPauses = 2;
        s.conditions.maxSameClubGames = 2;
        return s;
    }

    GamePlan solved(const TournamentSetup& s)
    {
        const Result<SolveResult> result = solve(s);
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(result.has_value() ? result->status : SearchStatus::cancelled, SearchStatus::solved);
        return result.has_value() ? result->plan : GamePlan{};
    }

    std::vector<ProblemCode> codes(const std::vector<Problem>& problems)
    {
        std::vector<ProblemCode> c;
        c.reserve(problems.size());
        for (const Problem& p : problems)
            c.push_back(p.code);
        return c;
    }

    const Problem* find(const std::vector<Problem>& problems, ProblemCode code)
    {
        const auto it = std::ranges::find(problems, code, &Problem::code);
        return it == problems.end() ? nullptr : &*it;
    }

    std::string teamName(const TeamRef& team)
    {
        return team.club + " " + team.team;
    }

    // The first game of a round with two teams
    Game& firstGame(Round& round)
    {
        return *std::ranges::find_if(round.games, [](const Game & g)
        {
            return g.home.has_value() && g.away.has_value();
        });
    }
}

TEST(checkPlan, aSolvedPlanKeepsTheConditions)
{
    const TournamentSetup s = setup();
    EXPECT_EQ(checkPlan(s, solved(s)), std::vector<Problem> {});
}

TEST(checkPlan, swappedGamesOfTwoFieldsKeepTheConditionsWithoutFieldTypes)
{
    const TournamentSetup s = setup();
    GamePlan plan = solved(s);
    std::swap(plan.rounds[0].games[0].home, plan.rounds[0].games[1].home);
    std::swap(plan.rounds[0].games[0].away, plan.rounds[0].games[1].away);
    EXPECT_EQ(checkPlan(s, plan), std::vector<Problem> {});
}

TEST(checkPlan, aTeamTwiceInARound)
{
    const TournamentSetup s = setup();
    GamePlan plan = solved(s);
    Round& round = plan.rounds[1];
    // the away team of the second field also plays home on the first
    round.games[0].home = round.games[1].away;
    const std::vector<Problem> problems = checkPlan(s, plan);
    const Problem* twice = find(problems, ProblemCode::planTeamTwice);
    ASSERT_NE(twice, nullptr);
    EXPECT_EQ(twice->path, "rounds[1]");
    EXPECT_EQ(twice->parameters.at(1), "2");
    EXPECT_EQ(twice->severity, Severity::warning);
}

TEST(checkPlan, pausesSameClubAndRepeatedPairings)
{
    TournamentSetup s = setup();
    s.conditions.maxPauses = 0;
    s.conditions.maxSameClubGames = 0;
    GamePlan plan;
    const TeamRef red{"North", "Red"};
    const TeamRef blue{"North", "Blue"};
    for (std::uint32_t r = 0; r < s.rounds; r++)
        plan.rounds.push_back({{{"1", red, blue}, {"2", std::nullopt, std::nullopt}}, {}});
    s.conditions.maxEmptyFields = 1;
    const std::vector<Problem> problems = checkPlan(s, plan);
    const Problem* pauses = find(problems, ProblemCode::planPauses);
    ASSERT_NE(pauses, nullptr);
    // North Green plays no game: five pauses
    EXPECT_EQ(pauses->parameters, (std::vector<std::string> {"North Green", "5", "0"}));
    const Problem* club = find(problems, ProblemCode::planSameClub);
    ASSERT_NE(club, nullptr);
    EXPECT_EQ(club->parameters, (std::vector<std::string> {"North Red", "5", "0"}));
    const Problem* pairing = find(problems, ProblemCode::planRepeatedPairing);
    ASSERT_NE(pairing, nullptr);
    EXPECT_EQ(pairing->parameters, (std::vector<std::string> {"North Red", "North Blue", "5", "1"}));
    // repeated pairings allowed as often as the iterations say
    s.repeatPairings = true;
    s.iterations = 5;
    EXPECT_EQ(find(checkPlan(s, plan), ProblemCode::planRepeatedPairing), nullptr);
}

TEST(checkPlan, emptyFieldsPerSubTournament)
{
    TournamentSetup s = setup();
    GamePlan plan = solved(s);
    Game& game = firstGame(plan.rounds[2]);
    game.home.reset();
    game.away.reset();
    std::vector<Problem> problems = checkPlan(s, plan);
    const Problem* empty = find(problems, ProblemCode::planEmptyFields);
    ASSERT_NE(empty, nullptr);
    EXPECT_EQ(empty->parameters, (std::vector<std::string> {"3", "1", "0"}));
    EXPECT_EQ(empty->path, "rounds[2]");
    // two sub tournaments of one field each: one empty field in each is allowed
    s.conditions.maxEmptyFields = 1;
    plan.subTournaments = {{{"1"}, {}}, {{"2"}, {}}};
    for (Game& g : plan.rounds[2].games)
    {
        g.home.reset();
        g.away.reset();
    }
    EXPECT_EQ(find(checkPlan(s, plan), ProblemCode::planEmptyFields), nullptr);
    // one sub tournament of both: two empty fields are too many
    plan.subTournaments = {{{"1", "2"}, {}}};
    EXPECT_NE(find(checkPlan(s, plan), ProblemCode::planEmptyFields), nullptr);
}

TEST(checkPlan, fieldTypesAndSubtypes)
{
    TournamentSetup s = setup();
    s.conditions.fieldTypes = {{"Small", 5, std::nullopt}, {"Large", std::nullopt, 0}};
    s.conditions.subtypeDeny = {{"x", {{"South", "Red"}}}};
    GamePlan plan;
    const TeamRef red{"South", "Red"};
    const TeamRef blue{"South", "Blue"};
    plan.rounds.push_back({{{"1", std::nullopt, std::nullopt}, {"2", red, blue}}, {}});
    const std::vector<Problem> problems = checkPlan(s, plan);
    const Problem* min = find(problems, ProblemCode::planFieldTypeMin);
    ASSERT_NE(min, nullptr);
    EXPECT_EQ(min->parameters, (std::vector<std::string> {"North Red", "Small", "0", "5"}));
    const Problem* max = find(problems, ProblemCode::planFieldTypeMax);
    ASSERT_NE(max, nullptr);
    EXPECT_EQ(max->parameters, (std::vector<std::string> {"South Red", "Large", "1", "0"}));
    const Problem* denied = find(problems, ProblemCode::planSubtypeDenied);
    ASSERT_NE(denied, nullptr);
    EXPECT_EQ(denied->parameters, (std::vector<std::string> {"South Red", "2", "1"}));
    EXPECT_EQ(denied->path, "rounds[0].games[1]");
}

TEST(checkPlan, aPlanThatDoesNotFitTheSetup)
{
    const TournamentSetup s = setup();
    GamePlan plan = solved(s);
    plan.rounds.pop_back();
    plan.rounds[0].games[0].field = "9";
    firstGame(plan.rounds[1]).home = TeamRef{"East", "Red"};
    plan.rounds[2].games.pop_back();
    const std::vector<Problem> problems = checkPlan(s, plan);
    const std::vector<ProblemCode> found = codes(problems);
    for (const ProblemCode code : {ProblemCode::planRounds, ProblemCode::planFields, ProblemCode::planUnknownTeam})
    {
        const Problem* p = find(problems, code);
        ASSERT_NE(p, nullptr) << static_cast<int>(code);
        EXPECT_EQ(p->severity, Severity::error);
    }
    EXPECT_EQ(find(problems, ProblemCode::planRounds)->parameters, (std::vector<std::string> {"4", "5"}));
    EXPECT_EQ(find(problems, ProblemCode::planUnknownTeam)->parameters, (std::vector<std::string> {"East Red", "2"}));
    EXPECT_EQ(std::ranges::count(found, ProblemCode::planFields), 2);
}

TEST(checkPlan, anOpenGame)
{
    TournamentSetup s = setup();
    s.conditions.maxPauses = 5;
    GamePlan plan = solved(s);
    Game& game = firstGame(plan.rounds[0]);
    const TeamRef alone = game.home.value_or(TeamRef{});
    game.away.reset();
    const std::vector<Problem> problems = checkPlan(s, plan);
    const Problem* open = find(problems, ProblemCode::planOpenGame);
    ASSERT_NE(open, nullptr);
    EXPECT_EQ(open->parameters, (std::vector<std::string> {teamName(alone), "1"}));
    EXPECT_EQ(open->severity, Severity::warning);
    // the field counts as empty
    EXPECT_NE(find(problems, ProblemCode::planEmptyFields), nullptr);
}
