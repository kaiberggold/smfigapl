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


// The planner (planner.hpp): validate, solve with limits, cancellation,
// progress and log, the statistic. Setups are built in code, a long
// search is read from a golden test configuration.

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/planner.hpp"
#include "test_setup.hpp"

using namespace smfigapl;

static_assert(noexcept(validate(TournamentSetup{})));
static_assert(noexcept(solve(TournamentSetup{}, SolveOptions{})));

namespace
{
    TournamentSetup smallSetup()
    {
        TournamentSetup s;
        s.rounds = 5;
        s.fields = {{"1", "Small", "x"}, {"2", "Large", "x"}};
        s.clubs = {{"North", {"Red", "Blue", "Green"}}, {"South", {"Red", "Blue"}}};
        s.conditions.maxPauses = 2;
        s.conditions.maxSameClubGames = 2;
        return s;
    }

    // A setup whose search takes millions of steps
    TournamentSetup longSearch()
    {
        std::ifstream file(std::filesystem::path(SM_GOLDEN_DIR) / "configs" / "gen_6f_12t_2ft_3-4_games_per_type.yaml", std::ios::binary);
        const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        auto setup = formats::yaml::readSetup(text);
        EXPECT_TRUE(setup.has_value());
        return setup.has_value() ? *setup : TournamentSetup{};
    }

    class Collect : public LogSink
    {
    public:
        bool wanted = true;
        std::vector<std::string> messages;
        [[nodiscard]] bool enabled(LogLevel /*level*/, LogCategory category) const override
        {
            return wanted && category == LogCategory::search;
        }
        void write(LogLevel /*level*/, LogCategory category, std::string_view message) override
        {
            EXPECT_EQ(category, LogCategory::search);
            messages.emplace_back(message);
        }
    };
}

TEST(planner, validate)
{
    EXPECT_TRUE(validate(smallSetup()).empty());
    TournamentSetup s = smallSetup();
    s.rounds = 0;
    s.conditions.fieldTypes = {{"Unknown", 1, std::nullopt}};
    const auto problems = validate(s);
    ASSERT_EQ(problems.size(), 2u);
    EXPECT_EQ(problems[0].path, "rounds");
    EXPECT_EQ(problems[1].path, "conditions.fieldTypes[0].type");
}

TEST(planner, solveRejectsAnInvalidSetup)
{
    TournamentSetup s = smallSetup();
    s.fields.clear();
    const auto r = solve(s);
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.problems().front().code, ProblemCode::nFieldsLt1);
}

// The same plan as the command line tool finds for the same configuration
TEST(planner, solveLikeTheCommandLineTool)
{
    const TournamentSetup setup = smallSetup();
    const auto r = solve(setup);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->status, SearchStatus::solved);
    TestSetup t(formats::yaml::writeSetup(setup));
    ASSERT_TRUE(t.solve());
    EXPECT_EQ(r->plan, t.out.plan);
    ASSERT_EQ(r->info.steps.size(), 1u);
    EXPECT_EQ(r->info.steps.front(), t.steps);
}

TEST(planner, searchTooLarge)
{
    TournamentSetup s;
    s.rounds = 2;
    for (int i = 1; i <= 10; i++)
        s.fields.push_back({std::to_string(i), "A", "x"});
    s.clubs = {{"North", {}}, {"South", {}}};
    for (int i = 1; i <= 10; i++)
        for (Club& c : s.clubs)
            c.teams.push_back(std::to_string(i));
    EXPECT_TRUE(validate(s).empty());
    const auto r = solve(s);
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.problems().front().code, ProblemCode::searchTooManyPairings);
}

TEST(planner, limits)
{
    const TournamentSetup setup = longSearch();
    SolveOptions steps;
    steps.maxSteps = 5000;
    const auto r = solve(setup, steps);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->status, SearchStatus::aborted);
    EXPECT_EQ(r->info.steps, std::vector<std::uint64_t> {5000});
    // where the search stopped
    EXPECT_EQ(r->plan.rounds.size(), setup.rounds);

    SolveOptions time;
    time.timeLimit = std::chrono::milliseconds(0);
    const auto t = solve(setup, time);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->status, SearchStatus::aborted);
    EXPECT_EQ(t->info.steps, std::vector<std::uint64_t> {1});
}

TEST(planner, cancel)
{
    const TournamentSetup setup = longSearch();
    // stopped before the start
    std::stop_source before;
    before.request_stop();
    SolveOptions options;
    options.stopToken = before.get_token();
    const auto r = solve(setup, options);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->status, SearchStatus::cancelled);
    EXPECT_EQ(r->info.steps, std::vector<std::uint64_t> {1});

    // stopped while searching (from the progress callback): soon after the request
    std::stop_source during;
    options.stopToken = during.get_token();
    options.progressInterval = 10000;
    options.progress = [&during](const Progress & p)
    {
        if (p.steps >= 10000)
            during.request_stop();
    };
    const auto c = solve(setup, options);
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(c->status, SearchStatus::cancelled);
    ASSERT_EQ(c->info.steps.size(), 1u);
    EXPECT_GE(c->info.steps.front(), 10000u);
    EXPECT_LE(c->info.steps.front(), 10000u + 4096u);
}

TEST(planner, progress)
{
    std::vector<Progress> reports;
    SolveOptions options;
    options.progressInterval = 7;
    options.progress = [&reports](const Progress & p)
    {
        reports.push_back(p);
    };
    const auto r = solve(smallSetup(), options);
    ASSERT_TRUE(r.has_value());
    ASSERT_FALSE(reports.empty());
    for (std::size_t i = 0; i + 1 < reports.size(); i++)
    {
        EXPECT_FALSE(reports[i].finished);
        EXPECT_EQ(reports[i].steps % 7, 0u);
    }
    EXPECT_EQ(reports.back(), (Progress{0, r->info.steps.front(), true}));
}

TEST(planner, log)
{
    Collect sink;
    SolveOptions options;
    options.log = &sink;
    ASSERT_TRUE(solve(smallSetup(), options).has_value());
    EXPECT_FALSE(sink.messages.empty());
    for (const std::string& m : sink.messages)
        EXPECT_EQ(m.find('\n'), std::string::npos);
    // nothing wanted, nothing written
    Collect none;
    none.wanted = false;
    options.log = &none;
    ASSERT_TRUE(solve(smallSetup(), options).has_value());
    EXPECT_TRUE(none.messages.empty());
}

// Games against teams of the own club, not pauses (config 0011: club D has one team)
TEST(planner, statisticSameClub)
{
    std::ifstream file(std::filesystem::path(SM_TEST_DATA_DIR) / "generic" / "0011_4f_7t_3c_2ft.yaml", std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    const auto setup = formats::yaml::readSetup(text);
    ASSERT_TRUE(setup.has_value());
    const auto result = solve(*setup);
    ASSERT_TRUE(result.has_value());
    const Statistic s = computeStatistic(*setup, *result);
    for (const TeamStatistic& team : s.teams)
    {
        std::uint32_t sameClub = 0;
        for (const Round& round : result->plan.rounds)
            for (const Game& game : round.games)
                if (game.home && game.away && game.home->club == game.away->club && (*game.home == team.team || *game.away == team.team))
                    sameClub++;
        EXPECT_EQ(team.sameClubGames, sameClub) << team.team.club << " " << team.team.team;
        if (team.team.club == "D")
        {
            EXPECT_EQ(team.sameClubGames, 0u);
        }
    }
}

TEST(planner, statisticWithoutPlan)
{
    const Statistic s = computeStatistic(smallSetup(), SolveResult{});
    EXPECT_EQ(s.fieldTypes, (std::vector<std::string> {"Small", "Large"}));
    ASSERT_EQ(s.teams.size(), 5u);
    EXPECT_EQ(s.teams[4], (TeamStatistic{TeamRef{"South", "Blue"}, 0, 0, 0, 0, 0, 0, 0, {0, 0}}));
}
