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


// Contract: the plans of the generic test configurations keep every
// condition of their setup, through the public API only (YAML adapter,
// solve, computeStatistic). The step counts must not grow (steps.yaml;
// SMFIGAPL_UPDATE_STEPS=1 stores lower counts).

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/planner.hpp"
#include "test_misc.hpp"

using namespace smfigapl;

namespace
{
    std::string stepsKey(const std::string& file)
    {
        std::string name = file.substr(file.find_last_of('/') + 1);
        return name.substr(0, name.find_last_of('.'));
    }

    // The stored step counts: lines "name: steps" (a YAML map, read
    // without the adapter, which is not for this file)
    std::map<std::string, std::uint64_t> readSteps()
    {
        std::map<std::string, std::uint64_t> steps;
        std::ifstream in(testDataDir + "/generic/steps.yaml");
        for (std::string line; std::getline(in, line);)
            if (const std::size_t colon = line.find(": "); colon != std::string::npos)
                steps[line.substr(0, colon)] = std::stoull(line.substr(colon + 2));
        return steps;
    }

    std::vector<std::tuple<std::string, std::uint64_t>> configs()
    {
        const std::map<std::string, std::uint64_t> steps = readSteps();
        std::vector<std::tuple<std::string, std::uint64_t>> result;
        for (const std::string& f : folderList(testDataDir + "/generic"))
        {
            const auto stored = steps.find(stepsKey(f));
            result.emplace_back(f, stored == steps.end() ? 0 : stored->second);
        }
        return result;
    }
}

namespace
{
    bool eachOnce(const std::vector<TeamRef>& teams)
    {
        return std::ranges::all_of(teams, [&teams](const TeamRef & t)
        {
            return std::ranges::count(teams, t) == 1;
        });
    }
}

class genericConfigs : public ::testing::TestWithParam<std::tuple<std::string, std::uint64_t>>
{
};

TEST_P(genericConfigs, conditionsKept)
{
    const auto& [file, storedSteps] = GetParam();
    std::ifstream in(file, std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    const auto setup = formats::yaml::readSetup(text);
    ASSERT_TRUE(setup.has_value()) << file;
    ASSERT_TRUE(validate(*setup).empty()) << file;
    const auto result = solve(*setup);
    ASSERT_TRUE(result.has_value()) << file;
    ASSERT_EQ(result->status, SearchStatus::solved) << file;
    const GamePlan& plan = result->plan;
    const Statistic s = computeStatistic(*setup, *result);
    const Conditions& c = setup->conditions;

    ASSERT_EQ(plan.rounds.size(), setup->rounds) << file;
    for (const Round& round : plan.rounds)
    {
        ASSERT_EQ(round.games.size(), setup->fields.size()) << file;
        // every team plays or pauses exactly once
        std::vector<TeamRef> teams = round.pauses;
        std::uint32_t empty = 0;
        for (const Game& game : round.games)
        {
            if (game.home && game.away)
            {
                teams.push_back(*game.home);
                teams.push_back(*game.away);
            }
            else
                empty++;
        }
        EXPECT_EQ(teams.size(), s.teams.size()) << file;
        EXPECT_TRUE(eachOnce(teams)) << file;
        EXPECT_LE(empty, c.maxEmptyFields) << file;
    }
    for (const TeamStatistic& t : s.teams)
    {
        const std::string team = file + " " + t.team.club + " " + t.team.team;
        EXPECT_EQ(t.games + t.pauses, setup->rounds) << team;
        if (c.maxPauses)
        {
            EXPECT_LE(t.pauses, *c.maxPauses) << team;
        }
        if (c.maxSameClubGames)
        {
            EXPECT_LE(t.sameClubGames, *c.maxSameClubGames) << team;
        }
        if (setup->homeAwayBalance)
        {
            EXPECT_LE(std::max(t.homeGames, t.awayGames) - std::min(t.homeGames, t.awayGames), 1u) << team;
        }
        if (!setup->repeatPairings)
        {
            EXPECT_EQ(t.repeatedGames, 0u) << team;
        }
        else
        {
            EXPECT_LE(t.maxGamesAgainstOneTeam, setup->iterations) << team;
        }
        for (const FieldTypeLimit& limit : c.fieldTypes)
        {
            const auto type = std::ranges::find(s.fieldTypes, limit.type);
            ASSERT_NE(type, s.fieldTypes.end()) << team;
            const std::uint32_t games = t.gamesPerFieldType[static_cast<std::size_t>(type - s.fieldTypes.begin())];
            if (limit.min)
            {
                EXPECT_GE(games, *limit.min) << team << " " << limit.type;
            }
            if (limit.max)
            {
                EXPECT_LE(games, *limit.max) << team << " " << limit.type;
            }
        }
    }
    // no team plays on a subtype denied to it
    for (const SubtypeDeny& deny : c.subtypeDeny)
        for (const Round& round : plan.rounds)
            for (std::size_t f = 0; f < round.games.size(); f++)
                if (setup->fields[f].subtype == deny.subtype && round.games[f].home && round.games[f].away)
                    for (const TeamRef& t : deny.teams)
                    {
                        EXPECT_NE(round.games[f].home.value_or(TeamRef{}), t) << file;
                        EXPECT_NE(round.games[f].away.value_or(TeamRef{}), t) << file;
                    }

    const std::uint64_t steps = std::accumulate(result->info.steps.begin(), result->info.steps.end(), std::uint64_t{0});
    if (storedSteps > 0)
    {
        EXPECT_LE(steps, storedSteps) << file;
    }
    // store a lower number of steps only on request
    const char* update = std::getenv("SMFIGAPL_UPDATE_STEPS");
    if (update != nullptr && std::string(update) == "1" && (storedSteps == 0 || steps < storedSteps))
    {
        std::map<std::string, std::uint64_t> stored = readSteps();
        stored[stepsKey(file)] = steps;
        std::ofstream out(testDataDir + "/generic/steps.yaml");
        for (const auto& [name, n] : stored)
            out << name << ": " << n << "\n";
    }
}

INSTANTIATE_TEST_SUITE_P(generic_cfg_files, genericConfigs, testing::ValuesIn(configs()));
