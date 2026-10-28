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
#include <set>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/model/sm_math.hpp"
#include "smfigapl/solver/sm_search_types.hpp"
#include "smfigapl/solver/sm_ts.hpp"

using namespace smfigapl;

namespace
{

// (2n-1)!! = 1*3*5*...*(2n-1): number of ways to pair 2n teams
    uint pairings(uint n)
    {
        uint p = 1;
        for (uint i = 1; i < 2 * n; i += 2)
            p *= i;
        return p;
    }

    uint binomial(uint n, uint k)
    {
        return factorial(n) / (factorial(k) * factorial(n - k));
    }

// A tree search for n fields and 2n teams; the first nTypeA fields are of type 0
    TreeSearch treeSearch(uint n, uint nTypeA)
    {
        Capacity c;
        c.NFields = n;
        c.NTeams = 2 * n;
        c.NClubs = 1;
        c.NRounds = 1;
        c.NFieldTypes = (nTypeA == 0 || nTypeA == n) ? 1 : 2;
        Mapping m;
        for (uint j = 0; j < n; j++)
        {
            m.indexedFieldTypeByFieldId.push_back(j < nTypeA ? 0 : 1);
            m.indexedFieldSubTypeByFieldId.push_back(0);
        }
        for (uint t = 0; t < 2 * n; t++)
        {
            m.indexedClubByTeamId.emplace_back(0);
            m.indexedColorByTeamId.emplace_back(t);
        }
        TreeSearch ts{DebugLog{}};
        ts.setCapacity(c);
        ts.setMapping(m);
        ts.init();
        return ts;
    }

// the games of a round as sorted pairs, in field order
    std::vector<std::pair<uint, uint>> games(TreeSearch& ts, uint index, uint n)
    {
        std::vector<std::pair<uint, uint>> g;
        for (uint j = 0; j < n; j++)
        {
            uint a = ts.getTeam(index, j, 0);
            uint b = ts.getTeam(index, j, 1);
            g.emplace_back(std::min(a, b), std::max(a, b));
        }
        return g;
    }

}

// ---------- helpers ----------

TEST(pairingIndex, numbersAllPairs)
{
    for (uint n : {2u, 5u, 8u})
    {
        EXPECT_EQ(getNPairingIndexShort(n), n * (n - 1) / 2);
        std::set<uint> seen;
        for (uint a = 0; a < n; a++)
            for (uint b = a + 1; b < n; b++)
            {
                uint i = getPairingIndexShort(n, a, b);
                EXPECT_LT(i, getNPairingIndexShort(n));
                EXPECT_TRUE(seen.insert(i).second) << "duplicate index for " << a << "," << b;
            }
    }
}

TEST(fieldSort, byTypeThenSubtype)
{
    std::vector<FieldSortable> f = {{1, 0, 0}, {0, 1, 1}, {0, 0, 2}, {1, 1, 3}};
    fieldSort(f);
    std::vector<uint> order;
    order.reserve(f.size());
    for (auto& x : f)
        order.push_back(x.index);
    EXPECT_EQ(order, (std::vector<uint> {2, 1, 0, 3}));
}

TEST(roundFieldMix, movesGamesToOtherFields)
{
    GamePlanRoundByTeamId round(3);
    for (uint j = 0; j < 3; j++)
    {
        round.setTeamId(j, 0, 2 * j);
        round.setTeamId(j, 1, 2 * j + 1);
    }
    const std::vector<uint> mix = {2, 0, 1};
    GamePlanRoundByTeamId mixed = roundFieldMix(round, mix);
    // field 0 gets the game of field 2, ...
    EXPECT_EQ(mixed.getTeamId(0, 0), 4u);
    EXPECT_EQ(mixed.getTeamId(0, 1), 5u);
    EXPECT_EQ(mixed.getTeamId(1, 0), 0u);
    EXPECT_EQ(mixed.getTeamId(2, 1), 3u);
}

// ---------- TreeSearch: all possible rounds ----------

class treeSearchRounds : public ::testing::TestWithParam<std::pair<uint, uint>> {};

TEST_P(treeSearchRounds, allRoundsOnce)
{
    auto [n, nTypeA] = GetParam();
    TreeSearch ts = treeSearch(n, nTypeA);
    // every pairing of the teams, and every way to put its games on the field types
    EXPECT_EQ(ts.getNPairingArray(), pairings(n));
    EXPECT_EQ(ts.getNFieldMixArray(), binomial(n, nTypeA));

    uint total = ts.getNPairingArray() * ts.getNFieldMixArray();
    std::set<std::vector<std::pair<uint, uint>>> rounds;
    for (uint i = 0; i < total; i++)
    {
        auto g = games(ts, i, n);
        // every team plays exactly once
        std::vector<uint> teams;
        for (auto [a, b] : g)
        {
            teams.push_back(a);
            teams.push_back(b);
        }
        std::ranges::sort(teams);
        for (uint t = 0; t < 2 * n; t++)
            EXPECT_EQ(teams[t], t) << "round " << i;
        // a round is defined by which games are played on which field type:
        // within a type the order of the fields does not matter
        std::vector<std::pair<uint, uint>> key;
        std::vector<std::pair<uint, uint>> typeA(g.begin(), g.begin() + nTypeA);
        std::vector<std::pair<uint, uint>> typeB(g.begin() + nTypeA, g.end());
        std::ranges::sort(typeA);
        std::ranges::sort(typeB);
        key.insert(key.end(), typeA.begin(), typeA.end());
        key.emplace_back(2 * n, 2 * n);
        key.insert(key.end(), typeB.begin(), typeB.end());
        EXPECT_TRUE(rounds.insert(key).second) << "round " << i << " found twice";
    }
}

INSTANTIATE_TEST_SUITE_P(treeSearch, treeSearchRounds, ::testing::Values(
                             std::pair{2u, 0u}, std::pair{2u, 1u}, std::pair{3u, 0u}, std::pair{3u, 1u}, std::pair{3u, 2u},
                             std::pair{4u, 0u}, std::pair{4u, 2u}, std::pair{4u, 3u}));

// A single field with two teams: one pairing, nothing to choose
// (nodeNext used to underflow the level and crash)
TEST(treeSearch, singleField)
{
    TreeSearch ts = treeSearch(1, 1);
    EXPECT_EQ(ts.getNPairingArray(), 1u);
    EXPECT_EQ(ts.getNFieldMixArray(), 1u);
}
