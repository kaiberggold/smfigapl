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

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/model/sm_debug.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_except.hpp"
#include "smfigapl/model/sm_indexed.hpp"
#include "smfigapl/model/sm_math.hpp"
#include "smfigapl/model/sm_named.hpp"
#include "smfigapl/model/sm_out.hpp"

using namespace smfigapl;

namespace
{

    IndexedTeam team(uint id, ClubId club = ClubId{0})
    {
        IndexedTeam t;
        t.setTeamId(id);
        t.setTeamClub(club);
        return t;
    }
}

// ---------- Input ----------

TEST(input, fieldsGetTypeIdsInOrderOfFirstUse)
{
    Input in;
    in.addField("Small", "Left", "1");
    in.addField("Large", "Left", "2");
    in.addField("Small", "Right", "3");
    EXPECT_EQ(in.setup.realCapacity.NFields, 3u);
    EXPECT_EQ(in.setup.realCapacity.NFieldTypes, 2u);
    EXPECT_EQ(in.setup.realCapacity.NFieldSubTypes, 2u);
    EXPECT_EQ(in.setup.mapping.indexedFieldTypeByFieldId, (std::vector<uint> {0, 1, 0}));
    EXPECT_EQ(in.setup.mapping.indexedFieldSubTypeByFieldId, (std::vector<uint> {0, 0, 1}));
    EXPECT_EQ(in.setup.fieldSubTypeIdBySubTypeName["Right"], 1u);
}

TEST(input, teamsByClubAndColor)
{
    Input in;
    in.addTeam("North", "Red");
    in.addTeam("North", "Blue");
    in.addTeam("South", "Red");
    EXPECT_EQ(in.setup.realCapacity.NTeams, 3u);
    EXPECT_EQ(in.setup.realCapacity.NClubs, 2u);
    EXPECT_EQ(in.setup.realCapacity.NTeamColors, 2u);
    EXPECT_EQ(in.setup.mapping.indexedClubByTeamId, (std::vector<ClubId> {ClubId{0}, ClubId{0}, ClubId{1}}));
    EXPECT_EQ(in.setup.mapping.indexedColorByTeamId, (std::vector<ColorId> {ColorId{0}, ColorId{1}, ColorId{0}}));
    EXPECT_EQ(in.setup.teamIdArray[1][0], 2u);
    EXPECT_EQ(in.setup.namedTeamByTeamId[1].getTeamName(), "North Blue");
}

// Club and color ids cannot be mixed up with each other or with numbers
static_assert(!std::is_convertible_v<uint, ClubId>);
static_assert(!std::is_convertible_v<ClubId, uint>);
static_assert(!std::is_convertible_v<ClubId, ColorId>);
static_assert(ClubId{1} < ClubId{2});

TEST(input, fieldTypeLimits)
{
    Input in;
    in.addField("A", "x", "1");
    in.addField("A", "x", "2");
    in.addField("B", "x", "3");
    in.setMinGamesFieldType("B", 1);
    in.setMaxGamesFieldType("B", 2);
    // unknown types are an error (they used to be ignored)
    EXPECT_THROW(in.setMaxGamesFieldType("C", 5), Exception);
    EXPECT_THROW(in.setMinGamesFieldType("C", 5), Exception);
    EXPECT_TRUE(in.conditions.limGamesFieldType.enabled);
    EXPECT_EQ(in.conditions.limGamesFieldType.values.min, (std::vector<uint> {0, 1}));
    // types without a limit are not restricted
    EXPECT_EQ(in.conditions.limGamesFieldType.values.max, (std::vector<uint> {std::numeric_limits<uint>::max(), 2}));
}

// ---------- indexed game plan ----------

TEST(indexedGame, swapExchangesTheTeams)
{
    IndexedGame g(team(1), team(2));
    EXPECT_EQ(g.getTeamByGameId(0).getTeamId(), 1u);
    g.flipSwap();
    EXPECT_TRUE(g.getSwap());
    EXPECT_EQ(g.getTeamByGameId(0).getTeamId(), 2u);
    EXPECT_EQ(g.getTeamByGameId(1).getTeamId(), 1u);
}

TEST(indexedGame, compareIgnoresHomeAndAway)
{
    IndexedGame a(team(1), team(2));
    IndexedGame b(team(2), team(1));
    IndexedGame c(team(1), team(3));
    EXPECT_TRUE(a.gameCompare(b));
    EXPECT_FALSE(a.gameCompare(c));
    EXPECT_TRUE(a.contains(2));
    EXPECT_FALSE(a.contains(3));
}

TEST(indexedRound, pauseTeams)
{
    IndexedRound r(2);
    r.setGame(0, IndexedGame(team(0), team(1)));
    r.setGame(1, IndexedGame(team(2), team(3)));
    r.addPauseTeam(team(4));
    EXPECT_EQ(r.getNPauseTeams(), 1u);
    EXPECT_TRUE(r.isPauseTeam(4));
    EXPECT_FALSE(r.isPauseTeam(0));
    EXPECT_TRUE(r.contains(IndexedGame(team(3), team(2))));
    r.pauseTeamsClear();
    EXPECT_EQ(r.getNPauseTeams(), 0u);
}

TEST(indexedRound, outOfRangeThrows)
{
    IndexedRound r(2);
    EXPECT_THROW(r.getGame(2), std::out_of_range);
}

TEST(indexedGamePlan, homeAndAway)
{
    IndexedGamePlan plan;
    plan.init(2, 1, 1);
    plan.setGame(0, 0, IndexedGame(team(0), team(1)));
    plan.setGame(1, 0, IndexedGame(team(1), team(0)));
    EXPECT_TRUE(plan.isHome(0, 0));
    EXPECT_TRUE(plan.isAway(0, 1));
    EXPECT_TRUE(plan.isHome(1, 1));
    plan.flipSwap(1, 0);
    EXPECT_TRUE(plan.isHome(1, 0));
    EXPECT_TRUE(plan.isAway(1, 1));
}

// ---------- results, helpers ----------

TEST(resultArray, solvedOnlyIfAllSucceeded)
{
    ResultArray r(2);
    EXPECT_TRUE(r.getResult());
    r.setResult(1, StepResult::failedMaxSteps);
    EXPECT_FALSE(r.getResult());
    EXPECT_EQ(r.getResult(1), StepResult::failedMaxSteps);
    r.incStep(0);
    r.incStep(1);
    r.incStep(1);
    EXPECT_EQ(r.getStep(1), 2u);
    EXPECT_EQ(r.getTotalSteps(), 3u);
}

TEST(math, factorial)
{
    EXPECT_EQ(factorial(0), 1u);
    EXPECT_EQ(factorial(5), 120u);
    // beyond the table
    EXPECT_EQ(factorial(12), 479001600u);
}

TEST(debugLog, linesToTheSink)
{
    std::ostringstream out;
    StreamLogSink sink(DebugFlags{static_cast<uint>(DebugFlag::cond) | static_cast<uint>(DebugFlag::file)}, out);
    DebugLog log(&sink);
    EXPECT_TRUE(log.has(DebugFlag::cond));
    EXPECT_TRUE(log.has(DebugFlag::file));
    EXPECT_FALSE(log.has(DebugFlag::ts));
    // a line goes to the sink at its end
    log.out(DebugFlag::cond) << "a " << 1;
    EXPECT_EQ(out.str(), "");
    log.out(DebugFlag::cond) << std::endl;
    log.out() << "b\nc";
    EXPECT_EQ(out.str(), "a 1\nb\n");
    // without a sink: nothing is enabled, the stream discards
    const DebugLog none;
    EXPECT_FALSE(none.has(DebugFlag::gps));
    none.out(DebugFlag::gps) << "x" << std::endl;
}

// The category of a line is the one of its first part
TEST(debugLog, categories)
{
    struct Collect : LogSink
    {
        std::vector<std::pair<LogCategory, std::string>> lines;
        [[nodiscard]] bool enabled(LogLevel /*level*/, LogCategory category) const override
        {
            return category != LogCategory::treeSearch;
        }
        void write(LogLevel level, LogCategory category, std::string_view message) override
        {
            EXPECT_EQ(level, LogLevel::debug);
            lines.emplace_back(category, message);
        }
    } sink;
    {
        DebugLog log(&sink);
        EXPECT_TRUE(log.has(DebugFlag::gps));
        EXPECT_FALSE(log.has(DebugFlag::ts));
        log.out(DebugFlag::stepsDetails) << "Search: ";
        log.out() << "index" << std::endl;
        log.out(DebugFlag::file) << "rest";
    }
    EXPECT_EQ(sink.lines, (std::vector<std::pair<LogCategory, std::string>> {{LogCategory::searchSteps, "Search: index"}, {LogCategory::file, "rest"}}));
}
