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


// Contract of the public API, on setups built in code (no format adapter):
// every condition holds in the plan or the setup is impossible with a
// reason, sub tournaments, the statuses, problems with paths, determinism.

#include <algorithm>
#include <cstdint>
#include <stop_token>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/planner.hpp"

using namespace smfigapl;

namespace
{
    // fields with the types (subtype "x", names "1", "2", ...), clubs North and South with teams
    TournamentSetup make(std::uint32_t rounds, const std::vector<std::string>& types, std::uint32_t north, std::uint32_t south)
    {
        TournamentSetup s;
        s.rounds = rounds;
        for (std::size_t i = 0; i < types.size(); i++)
            s.fields.push_back({std::to_string(i + 1), types[i], "x"});
        s.clubs = {{"North", {}}, {"South", {}}};
        for (std::uint32_t i = 0; i < north; i++)
            s.clubs[0].teams.push_back("N" + std::to_string(i + 1));
        for (std::uint32_t i = 0; i < south; i++)
            s.clubs[1].teams.push_back("S" + std::to_string(i + 1));
        return s;
    }

    SolveResult solved(const TournamentSetup& setup)
    {
        const auto r = solve(setup);
        EXPECT_TRUE(r.has_value());
        EXPECT_EQ(r.has_value() ? r->status : SearchStatus::impossible, SearchStatus::solved);
        return r.has_value() ? *r : SolveResult{};
    }

    // The status and the reasons of a setup without a plan
    SolveResult impossible(const TournamentSetup& setup)
    {
        const auto r = solve(setup);
        EXPECT_TRUE(r.has_value());
        if (!r.has_value())
            return {};
        EXPECT_EQ(r->status, SearchStatus::impossible);
        EXPECT_FALSE(r->reasons.empty());
        EXPECT_TRUE(r->plan.rounds.empty());
        return *r;
    }

    std::vector<ProblemCode> codes(const std::vector<Problem>& problems)
    {
        std::vector<ProblemCode> c;
        c.reserve(problems.size());
        for (const Problem& p : problems)
            c.push_back(p.code);
        return c;
    }

    std::uint32_t emptyFields(const Round& round)
    {
        return static_cast<std::uint32_t>(std::ranges::count_if(round.games, [](const Game & g)
        {
            return !g.home;
        }));
    }
}

TEST(contract, everyPairingOnce)
{
    const TournamentSetup s = make(7, {"A", "A", "A", "A"}, 4, 4);
    const SolveResult r = solved(s);
    for (const TeamStatistic& t : computeStatistic(s, r).teams)
    {
        EXPECT_EQ(t.repeatedGames, 0u);
        EXPECT_EQ(t.games, 7u);
    }
}

TEST(contract, pauses)
{
    TournamentSetup s = make(3, {"A", "A"}, 3, 3);
    s.conditions.maxPauses = 1;
    const SolveResult r = solved(s);
    for (const TeamStatistic& t : computeStatistic(s, r).teams)
        EXPECT_LE(t.pauses, 1u);
    // 6 teams on 2 fields: 2 pauses in every round, none allowed
    s.conditions.maxPauses = 0;
    EXPECT_EQ(codes(impossible(s).reasons), std::vector<ProblemCode> {ProblemCode::impossiblePauses});
}

TEST(contract, emptyFields)
{
    // 4 teams on 3 fields: one field is empty in every round
    TournamentSetup s = make(3, {"A", "A", "A"}, 2, 2);
    EXPECT_FALSE(solve(s)->status == SearchStatus::solved);
    s.conditions.maxEmptyFields = 1;
    const SolveResult r = solved(s);
    for (const Round& round : r.plan.rounds)
        EXPECT_EQ(emptyFields(round), 1u);
}

TEST(contract, sameClub)
{
    TournamentSetup s = make(3, {"A", "A"}, 2, 2);
    s.conditions.maxSameClubGames = 1;
    for (const TeamStatistic& t : computeStatistic(s, solved(s)).teams)
        EXPECT_LE(t.sameClubGames, 1u);
    // one club only: every game is a game within the club
    TournamentSetup one = make(3, {"A", "A"}, 4, 0);
    one.conditions.maxSameClubGames = 1;
    impossible(one);
}

TEST(contract, fieldTypes)
{
    TournamentSetup s = make(7, {"A", "B", "A", "B"}, 4, 4);
    s.conditions.fieldTypes = {{"A", 3, 4}, {"B", 3, 4}};
    const Statistic st = computeStatistic(s, solved(s));
    for (const TeamStatistic& t : st.teams)
        for (const std::uint32_t games : t.gamesPerFieldType)
        {
            EXPECT_GE(games, 3u);
            EXPECT_LE(games, 4u);
        }
    // 8 teams play 7 games on 2 fields of A per round: 14 places, a minimum of 4 needs 32
    s.conditions.fieldTypes = {{"A", 4, std::nullopt}};
    EXPECT_EQ(codes(impossible(s).reasons), std::vector<ProblemCode> {ProblemCode::impossibleFieldTypeMin});
}

TEST(contract, subTournaments)
{
    // the subtype Right is denied to North, Left to South: two sub tournaments
    TournamentSetup s = make(3, {"A", "A"}, 3, 3);
    s.fields[0].subtype = "Left";
    s.fields[1].subtype = "Right";
    s.conditions.maxPauses = 1;
    s.conditions.subtypeDeny = {{"Right", {{"North", "N1"}, {"North", "N2"}, {"North", "N3"}}}, {"Left", {{"South", "S1"}, {"South", "S2"}, {"South", "S3"}}}};
    const SolveResult r = solved(s);
    ASSERT_EQ(r.plan.subTournaments.size(), 2u);
    EXPECT_EQ(r.plan.subTournaments[0].fields, std::vector<std::string> {"1"});
    EXPECT_EQ(r.plan.subTournaments[1].fields, std::vector<std::string> {"2"});
    EXPECT_EQ(r.info.steps.size(), 2u);
    for (const Round& round : r.plan.rounds)
    {
        EXPECT_EQ(round.games[0].home.value_or(TeamRef{}).club, "North");
        EXPECT_EQ(round.games[0].away.value_or(TeamRef{}).club, "North");
        EXPECT_EQ(round.games[1].home.value_or(TeamRef{}).club, "South");
        EXPECT_EQ(round.games[1].away.value_or(TeamRef{}).club, "South");
    }
}

TEST(contract, repeatedPairings)
{
    // 4 teams have 6 pairings, 6 rounds of 2 games need each twice
    TournamentSetup s = make(6, {"A", "A"}, 2, 2);
    EXPECT_EQ(solve(s)->status, SearchStatus::impossible);
    s.repeatPairings = true;
    s.iterations = 2;
    for (const TeamStatistic& t : computeStatistic(s, solved(s)).teams)
        EXPECT_EQ(t.maxGamesAgainstOneTeam, 2u);
}

TEST(contract, homeAwayBalance)
{
    const TournamentSetup s = make(7, {"A", "A", "A", "A"}, 4, 4);
    for (const TeamStatistic& t : computeStatistic(s, solved(s)).teams)
        EXPECT_LE(std::max(t.homeGames, t.awayGames) - std::min(t.homeGames, t.awayGames), 1u);
}

TEST(contract, statuses)
{
    const TournamentSetup s = make(7, {"A", "B", "A", "B"}, 4, 4);
    EXPECT_EQ(solve(s)->status, SearchStatus::solved);
    TournamentSetup hard = s;
    hard.conditions.fieldTypes = {{"A", 3, 4}, {"B", 3, 4}};
    SolveOptions options;
    options.maxSteps = 1;
    const auto aborted = solve(hard, options);
    ASSERT_TRUE(aborted.has_value());
    EXPECT_EQ(aborted->status, SearchStatus::aborted);
    EXPECT_TRUE(aborted->reasons.empty());
    std::stop_source stop;
    stop.request_stop();
    SolveOptions cancel;
    cancel.stopToken = stop.get_token();
    EXPECT_EQ(solve(hard, cancel)->status, SearchStatus::cancelled);
}

// Problems of the setup with their paths, from validate() and solve()
TEST(contract, problemsWithPaths)
{
    TournamentSetup s = make(11, {}, 0, 0);
    s.conditions.fieldTypes = {{"A", 2, 1}};
    s.conditions.subtypeDeny = {{"y", {{"West", "W1"}}}};
    const std::vector<Problem> problems = validate(s);
    std::vector<std::string> paths;
    for (const Problem& p : problems)
    {
        EXPECT_EQ(p.severity, Severity::error);
        paths.push_back(p.path);
    }
    EXPECT_EQ(paths, (std::vector<std::string> {"rounds", "fields", "clubs", "conditions.fieldTypes[0]", "conditions.fieldTypes[0].type",
                      "conditions.subtypeDeny[0].subtype", "conditions.subtypeDeny[0].teams[0]"
                                               }));
    const auto r = solve(s);
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.problems(), problems);
}

// The same setup gives the same result (apart from the duration)
TEST(contract, deterministic)
{
    TournamentSetup s = make(7, {"A", "B", "A", "B"}, 4, 3);
    s.conditions.maxPauses = 1;
    s.conditions.maxEmptyFields = 1;
    SolveResult a = solved(s);
    SolveResult b = solved(s);
    a.info.duration = {};
    b.info.duration = {};
    EXPECT_EQ(a, b);
}
