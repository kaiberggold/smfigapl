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


// The public types of the API (include/smfigapl): version, error model,
// domain model, logging interface

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/layout.hpp"
#include "smfigapl/log.hpp"
#include "smfigapl/plan.hpp"
#include "smfigapl/result.hpp"
#include "smfigapl/setup.hpp"
#include "smfigapl/statistic.hpp"
#include "smfigapl/version.hpp"

using namespace smfigapl;

TEST(publicTypes, version)
{
    const Version v = version();
    EXPECT_EQ(v, (Version {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH}));
    const std::string numbers = std::to_string(v.major) + "." + std::to_string(v.minor) + "." + std::to_string(v.patch);
    // a development build: X.Y.Z-devel or X.Y.Z-devel.N
    EXPECT_EQ(std::string(VERSION_STRING), VERSION_PRERELEASE.empty() ? numbers : numbers + "-" + std::string(VERSION_PRERELEASE));
}

// The numbers are part of the API (and the exit codes of the command line tool)
TEST(publicTypes, problemCodesAreStable)
{
    static_assert(static_cast<int>(ProblemCode::parseError) == 1);
    static_assert(static_cast<int>(ProblemCode::invalidValue) == 14);
    static_assert(static_cast<int>(ProblemCode::searchTooLarge) == 109);
    static_assert(static_cast<int>(ProblemCode::searchTooManyPairings) == 110);
    static_assert(static_cast<int>(ProblemCode::maxLtMin) == 200);
    static_assert(static_cast<int>(ProblemCode::impossibleSearch) == 307);
    static_assert(static_cast<int>(ProblemCode::impossibleGames) == 308);
    static_assert(static_cast<int>(ProblemCode::internal) == 900);
    SUCCEED();
}

TEST(publicTypes, resultWithValueAndWarnings)
{
    const Problem warning {ProblemCode::invalidValue, {"format"}, "settings.unknown", Severity::warning, TextPosition{3, 1}};
    Result<std::string> r(std::string("plan"), {warning});
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(static_cast<bool>(r));
    EXPECT_EQ(r.value(), "plan");
    EXPECT_EQ(*r, "plan");
    EXPECT_EQ(r->size(), 4u);
    ASSERT_EQ(r.problems().size(), 1u);
    EXPECT_EQ(r.problems().front(), warning);
    const std::string moved = std::move(r).value();
    EXPECT_EQ(moved, "plan");
}

TEST(publicTypes, resultWithoutValue)
{
    Problem problem;
    problem.code = ProblemCode::nRoundsLt1;
    problem.path = "rounds";
    const auto r = Result<int>::failure(problem);
    EXPECT_FALSE(r.has_value());
    EXPECT_FALSE(static_cast<bool>(r));
    ASSERT_EQ(r.problems().size(), 1u);
    EXPECT_EQ(r.problems().front().code, ProblemCode::nRoundsLt1);
    EXPECT_EQ(r.problems().front().severity, Severity::error);
    EXPECT_EQ(r.problems().front().path, "rounds");
    EXPECT_FALSE(r.problems().front().position.has_value());
}

TEST(publicTypesDeathTest, valueOfAFailureEndsTheProgram)
{
    Problem problem;
    problem.code = ProblemCode::internal;
    const auto r = Result<int>::failure(problem);
    EXPECT_DEATH((void)r.value(), "");
}

// The defaults are those of a configuration file without the optional keys
TEST(publicTypes, setupDefaults)
{
    const TournamentSetup setup;
    EXPECT_EQ(setup.rounds, 0u);
    EXPECT_FALSE(setup.repeatPairings);
    EXPECT_EQ(setup.iterations, 1u);
    EXPECT_TRUE(setup.homeAwayBalance);
    EXPECT_TRUE(setup.optimizeRoundOrder);
    EXPECT_FALSE(setup.conditions.maxPauses.has_value());
    EXPECT_EQ(setup.conditions.maxEmptyFields, 0u);
    EXPECT_FALSE(setup.conditions.maxSameClubGames.has_value());
    EXPECT_TRUE(setup.conditions.fieldTypes.empty());
    EXPECT_TRUE(setup.conditions.subtypeDeny.empty());
}

TEST(publicTypes, setupEquality)
{
    TournamentSetup a;
    a.rounds = 3;
    a.fields = {{"1", "A", "x"}, {"2", "B", "x"}};
    a.clubs = {{"North", {"Red", "Blue"}}};
    a.conditions.fieldTypes = {{"A", 1, std::nullopt}};
    a.conditions.subtypeDeny = {{"x", {{"North", "Red"}}}};
    TournamentSetup b = a;
    EXPECT_EQ(a, b);
    b.conditions.fieldTypes.front().max = 2;
    EXPECT_NE(a, b);
}

TEST(publicTypes, solveResult)
{
    SolveResult result;
    EXPECT_EQ(result.status, SearchStatus::impossible);
    result.status = SearchStatus::solved;
    result.plan.rounds.push_back({{{"1", TeamRef{"North", "Red"}, TeamRef{"North", "Blue"}}, {"2", std::nullopt, std::nullopt}}, {}});
    result.plan.subTournaments.push_back({{"1", "2"}, {{"North", "Red"}, {"North", "Blue"}}});
    const SolveResult copy = result;
    EXPECT_EQ(copy, result);
    EXPECT_FALSE(copy.plan.rounds.front().games.back().home.has_value());
}

TEST(publicTypes, statistic)
{
    Statistic s;
    s.fieldTypes = {"A", "B"};
    s.teams.push_back({TeamRef{"North", "Red"}, 3, 2, 1, 0, 1, 0, 1, {2, 1}});
    EXPECT_EQ(s.teams.front().gamesPerFieldType.size(), s.fieldTypes.size());
    EXPECT_EQ(s, Statistic(s));
}

// Without a format file: no information, no statistic in the spreadsheet
TEST(publicTypes, layoutDefaults)
{
    const layout::OutputLayout l;
    EXPECT_TRUE(l.info.general.values.empty());
    EXPECT_FALSE(l.format.settings.showStatistic);
    EXPECT_FALSE(l.format.settings.separateTournaments);
}

namespace
{
    // A sink that keeps the wanted messages
    class Collect : public LogSink
    {
    public:
        std::vector<std::string> messages;
        [[nodiscard]] bool enabled(LogLevel level, LogCategory /*category*/) const override
        {
            return level != LogLevel::debug;
        }
        void write(LogLevel /*level*/, LogCategory /*category*/, std::string_view message) override
        {
            messages.emplace_back(message);
        }
    };
}

TEST(publicTypes, logSink)
{
    Collect collect;
    LogSink& sink = collect;
    for (const LogLevel level : {LogLevel::info, LogLevel::debug})
        if (sink.enabled(level, LogCategory::search))
            sink.write(level, LogCategory::search, "message");
    EXPECT_EQ(collect.messages, std::vector<std::string> {"message"});
}
