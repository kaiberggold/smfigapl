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

// The tables of the conditions and CheckGamePlan without the search

#include <algorithm>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/solver/sm_atomic.hpp"
#include "smfigapl/solver/sm_cond.hpp"
#include "smfigapl/solver/sm_tables.hpp"
#include "smfigapl/solver/sm_ts.hpp"
#include "test_setup.hpp"

using namespace smfigapl;

namespace
{
    // The tables of the first atomic set of a configuration, as the search builds them
    struct TableSetup
    {
        TestSetup t;
        CondTables tables;
        AtomicSets sets;
        TreeSearch ts;
        RoundTables rounds{t.in, ts, tables};

        explicit TableSetup(const std::string& config): t(config), ts(t.in.dbg)
        {
            sets = calcAtomicSets(t.in, tables);
            tables.atomicIndex = 0;
            ts.setCapacity(tables.atomicCapacityArray[0]);
            ts.setExtraPauseFields(t.in.conditions.extraPauseFields);
            ts.setMapping(tables.atomicMappingArray[0]);
            ts.init();
            rounds.build();
        }

        [[nodiscard]] uint NTeams() const
        {
            return tables.atomicCapacityArray[0].NTeams;
        }

        // pairing indices of the games (no pause) of a candidate round
        [[nodiscard]] std::vector<uint> pairings(uint i) const
        {
            IndexedRound round = rounds.candidate(i);
            std::vector<uint> p;
            for (uint j = 0; j < tables.atomicCapacityArray[0].NFields; j++)
            {
                IndexedGame g = round.getGame(j);
                if (g.getPause())
                    continue;
                uint a = g.getTeamByGameId(0).getTeamId();
                uint b = g.getTeamByGameId(1).getTeamId();
                p.push_back(getPairingIndexShort(NTeams(), std::min(a, b), std::max(a, b)));
            }
            return p;
        }
    };

    const std::string fourFields = field("A", "x", "1") + field("A", "x", "2") + field("B", "x", "3") + field("B", "x", "4");
    const std::string sevenTeams = club("North", {"Red", "Blue", "Green", "Black"}) + club("South", {"Red", "Blue", "Green"});
    // 7 teams on 4 fields: one field is empty in every round
    const std::string config = "rounds: 3\n" + fourFields + sevenTeams + "conditions:\n  max_empty_fields: 1\n";

    bool contains(const std::vector<uint>& v, uint x)
    {
        return std::ranges::find(v, x) != v.end();
    }
}

TEST(atomicSets, oneSetWithoutDenyList)
{
    TableSetup s(config);
    EXPECT_EQ(s.sets.count, 1u);
    EXPECT_EQ(s.tables.atomicCapacityArray[0].NTeams, 7u);
    EXPECT_EQ(s.tables.atomicCapacityArray[0].NFields, 4u);
    EXPECT_EQ(s.tables.atomicCapacityArray[0].NFieldTypes, 2u);
    EXPECT_EQ(s.tables.fieldMappingArray[0], (std::vector<uint> {0, 1, 2, 3}));
}

TEST(atomicSets, denyListSplitsTheTournament)
{
    // North plays on the left, South on the right fields
    const std::string fields = field("A", "Left", "1") + field("A", "Left", "2") + field("A", "Right", "3") + field("A", "Right", "4");
    const std::string clubs = club("North", {"Red", "Blue", "Green", "Black"}) + club("South", {"Red", "Blue", "Green", "Black"});
    std::string deny = "conditions:\n  subtype_deny:\n    - subtype: \"Left\"\n      teams: [{club: \"South\", team: \"Red\"}, {club: \"South\", team: \"Blue\"}, {club: \"South\", team: \"Green\"}, {club: \"South\", team: \"Black\"}]\n";
    deny += "conditions:\n  subtype_deny:\n    - subtype: \"Right\"\n      teams: [{club: \"North\", team: \"Red\"}, {club: \"North\", team: \"Blue\"}, {club: \"North\", team: \"Green\"}, {club: \"North\", team: \"Black\"}]\n";
    TableSetup s("rounds: 3\n" + fields + clubs + deny);
    ASSERT_EQ(s.sets.count, 2u);
    EXPECT_EQ(s.sets.indexByFieldId, (std::vector<uint> {0, 0, 1, 1}));
    EXPECT_EQ(s.sets.indexByTeamId, (std::vector<uint> {0, 0, 0, 0, 1, 1, 1, 1}));
    EXPECT_EQ(s.tables.teamsMappingArray[1], (std::vector<uint> {4, 5, 6, 7}));
    // team ids inside the set
    EXPECT_EQ(s.tables.teamIdMappingArray, (std::vector<uint> {0, 1, 2, 3, 0, 1, 2, 3}));
    for (uint a = 0; a < 2; a++)
    {
        EXPECT_EQ(s.tables.atomicCapacityArray[a].NTeams, 4u);
        EXPECT_EQ(s.tables.atomicCapacityArray[a].NFields, 2u);
        EXPECT_EQ(s.tables.atomicCapacityArray[a].NClubs, 1u);
    }
}

// The candidates are not stored: every one is computed from its pairing and
// field mix. Compared with the rounds of TreeSearch (getRound) and with the
// tables as they were stored before, by their definition: the teams per
// field, pause teams, club games, the repeated games jumps (the following
// candidates with the same game) and the field type jumps.
namespace
{
    struct Oracle
    {
        TableSetup& s;
        const Candidates& c;
        explicit Oracle(TableSetup& setup): s{setup}, c{setup.tables.candidates}
        {
        }
        // team k on position j from TreeSearch, TEAM_UNDEFINED for a virtual team
        [[nodiscard]] uint team(uint i, uint j, uint k) const
        {
            GamePlanRoundByTeamId round = s.ts.getRound(c.full(i) % (c.NPairings * c.NMixes));
            const uint t = round.getTeamId(j, k);
            return t < c.NTeams ? t : TEAM_UNDEFINED;
        }
        [[nodiscard]] bool pause(uint i, uint j) const
        {
            return team(i, j, 0) == TEAM_UNDEFINED || team(i, j, 1) == TEAM_UNDEFINED;
        }
        [[nodiscard]] std::vector<uint> keys(uint i) const
        {
            std::vector<uint> k;
            for (uint j = 0; j < c.NPositions; j++)
                if (!pause(i, j))
                    k.push_back(getPairingIndexShort(c.NTeams, team(i, j, 0), team(i, j, 1)));
            return k;
        }
        [[nodiscard]] std::vector<TableTeam> pauseTeams(uint i) const
        {
            std::vector<TableTeam> p;
            for (uint j = 0; j < c.NGames; j++)
                for (uint k = 0; k < 2; k++)
                    if (team(i, j, k) != TEAM_UNDEFINED && (j >= c.NPositions || pause(i, j)))
                        p.push_back(static_cast<TableTeam>(team(i, j, k)));
            std::ranges::sort(p);
            return p;
        }
        // the old jump: the run of following candidates with the game, at most to the last one
        [[nodiscard]] uint gameJump(uint i, uint key) const
        {
            uint k = i + 1;
            while (k < c.size() && contains(keys(k), key))
                k++;
            return std::min(k - i, c.size() - i - 1);
        }
        [[nodiscard]] uint type(uint i, uint team) const
        {
            for (uint j = 0; j < c.NPositions; j++)
                if (!pause(i, j) && (this->team(i, j, 0) == team || this->team(i, j, 1) == team))
                    return c.fieldType[j];
            return 99;
        }
        [[nodiscard]] uint fieldTypeJump(uint i, uint team) const
        {
            if (type(i, team) == 99 || i + 1 >= c.size())
                return 0;
            uint jump = 1;
            for (uint k = i + 1; k + 1 < c.size() && type(k, team) == type(i, team); k++)
                jump++;
            return jump;
        }
    };

    void compareWithOracle(const std::string& configuration)
    {
        TableSetup s(configuration);
        const Oracle o(s);
        const Candidates& c = s.tables.candidates;
        ASSERT_GT(c.size(), 1u);
        const Mapping& m = s.tables.atomicMappingArray[0];
        for (uint i = 0; i < c.size(); i++)
        {
            const Candidates::View v = c.view(i);
            std::vector<uint> keys;
            std::vector<uint> sameClub;
            for (uint j = 0; j < c.NPositions; j++)
                for (uint k = 0; k < 2; k++)
                    ASSERT_EQ(v.team(j, k), o.team(i, j, k)) << i << " " << j;
            c.forEachGame(i, [&](uint j, uint a, uint b)
            {
                keys.push_back(c.pairingKey(a, b));
                EXPECT_EQ(c.gameJump(i, j), o.gameJump(i, c.pairingKey(a, b))) << i << " " << j;
                if (m.indexedClubByTeamId[a] == m.indexedClubByTeamId[b])
                {
                    sameClub.push_back(a);
                    sameClub.push_back(b);
                }
            });
            EXPECT_EQ(keys, o.keys(i)) << i;
            EXPECT_EQ(c.pauseTeams(i), o.pauseTeams(i)) << i;
            std::vector<uint> club;
            c.forEachSameClub(i, [&club](uint team)
            {
                club.push_back(team);
            });
            EXPECT_EQ(club, sameClub) << i;
            for (uint team = 0; team < c.NTeams; team++)
                EXPECT_EQ(c.fieldTypeJump(i, team), o.fieldTypeJump(i, team)) << i << " " << team;
        }
    }
}

TEST(roundTables, candidatesAsStoredBefore)
{
    // 8 teams on 4 fields of 2 types: no virtual team, every candidate valid
    compareWithOracle("rounds: 3\n" + fourFields + club("North", {"Red", "Blue", "Green", "Black"}) + club("South", {"Red", "Blue", "Green", "Black"}));
    // 7 teams: one virtual team, one empty field in every round
    compareWithOracle(config);
    // the same with 2 iterations
    compareWithOracle("repeat_pairings: true\niterations: 2\n" + config);
    // 5 teams on 2 fields: a pause field, the games on it are pauses
    compareWithOracle("rounds: 3\n" + field("A", "x", "1") + field("B", "x", "2") + club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue"}) + "conditions:\n  max_empty_fields: 1\n");
    // a deny list: candidates are left out (valid), jumps across them
    compareWithOracle("rounds: 3\n" + field("A", "x", "1") + field("A", "y", "2") + field("B", "x", "3") + club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue", "Green"}) + "conditions:\n  subtype_deny:\n    - subtype: \"y\"\n      teams: [{club: \"North\", team: \"Red\"}]\n");
}

TEST(roundTables, repeatedGamesAndJumps)
{
    TableSetup s(config);
    const Candidates& c = s.tables.candidates;
    const uint N = s.rounds.size();
    ASSERT_EQ(c.size(), N);
    ASSERT_GT(N, 1u);
    uint maxJump = 0;
    for (uint i = 0; i < N; i++)
    {
        std::vector<uint> p = s.pairings(i);
        // 7 teams on 4 fields: 3 games, one team pauses
        EXPECT_EQ(p.size(), 3u) << i;
        std::vector<uint> games;
        std::vector<uint> jumps;
        c.forEachGame(i, [&](uint position, uint a, uint b)
        {
            games.push_back(c.pairingKey(a, b));
            jumps.push_back(c.gameJump(i, position));
        });
        std::vector<uint> cond = games;
        std::ranges::sort(cond);
        std::ranges::sort(p);
        EXPECT_EQ(cond, p) << i;
        // the jump of a pairing: the following candidates up to the jump contain it,
        // the one after the jump does not (unless the jump ends at the last candidate)
        for (size_t g = 0; g < games.size(); g++)
        {
            const uint jump = jumps[g];
            maxJump = std::max(maxJump, jump);
            for (uint k = 1; k < jump; k++)
                EXPECT_TRUE(contains(s.pairings(i + k), games[g])) << i << " " << k;
            if (i + jump < N && jump < N - i - 1)
            {
                EXPECT_FALSE(contains(s.pairings(i + jump), games[g])) << i << " " << jump;
            }
        }
    }
    // candidates with a common pairing follow each other
    EXPECT_GT(maxJump, 1u);
}

TEST(roundTables, searchIndexMax)
{
    TableSetup s(config);
    ASSERT_GT(s.rounds.size(), 1u);
    const std::vector<uint>& max = s.tables.searchIndexMax;
    ASSERT_EQ(max.size(), 3u);
    // the last round may use every candidate, the rounds before leave room for the following ones
    EXPECT_EQ(max.back(), s.rounds.size() - 1);
    EXPECT_TRUE(std::ranges::is_sorted(max));
    EXPECT_LT(max[0], max[1]);
}

TEST(checkGamePlan, countersAndPairingLimit)
{
    TableSetup s(config + "conditions:\n  max_pauses: 1\n");
    ASSERT_GT(s.rounds.size(), 1u);
    uint level = 0;
    TreeSearchState state;
    CheckGamePlan check{s.t.in, level, s.tables, state};
    check.init();
    // round 1: candidate 0
    ASSERT_TRUE(check.gamePlanCheck(true).check);
    const uint pairing = s.pairings(0).front();
    level = 1;
    EXPECT_TRUE(check.pairingLimitReached(pairing));
    // round 2: candidate 0 again repeats its pairings
    state.Add = 0;
    state.Sub = -1;
    state.stepResult = StepResult::nextStep;
    EXPECT_FALSE(check.gamePlanCheck(false).check);
    // replace it by a candidate without a common pairing and another pause team
    const std::vector<uint> first = s.pairings(0);
    const uint pauseTeam = s.tables.candidates.pauseTeams(0).front();
    uint other = 0;
    for (uint i = 1; i < s.rounds.size() && other == 0; i++)
    {
        std::vector<uint> p = s.pairings(i);
        if (std::ranges::none_of(p, [&first](uint x)
    {
        return contains(first, x);
        }) && s.tables.candidates.pauseTeams(i).front() != pauseTeam)
        other = i;
    }
    ASSERT_NE(other, 0u);
    state.Sub = 0;
    state.Add = static_cast<int>(other);
    EXPECT_TRUE(check.gamePlanCheck(false).check);
    // round 3: the pause team of round 2 pauses again, more than max = 1
    // (and no pairing of round 2, so only the pause games fail)
    const uint pauseTeam2 = s.tables.candidates.pauseTeams(other).front();
    const std::vector<uint> second = s.pairings(other);
    uint again = 0;
    for (uint i = other + 1; i < s.rounds.size() && again == 0; i++)
        if (s.tables.candidates.pauseTeams(i).front() == pauseTeam2 && std::ranges::none_of(s.pairings(i), [&second](uint x)
    {
        return contains(second, x);
        }))
    again = i;
    ASSERT_NE(again, 0u);
    level = 2;
    state.Sub = -1;
    state.Add = static_cast<int>(again);
    EXPECT_FALSE(check.gamePlanCheck(false).check);
}
