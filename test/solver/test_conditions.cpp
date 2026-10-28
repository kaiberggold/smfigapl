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
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "gtest/gtest.h"

#include "test_setup.hpp"

using namespace smfigapl;

namespace
{

// Counts taken directly from the indexed game plan, independent of
// the statistic and of the condition checks of the search
    struct PlanCount
    {
        std::vector<uint> games, pauses, sameClub, home, away;
        std::vector<std::vector<uint>> gamesOnFieldType;  // [team][field type]
        std::vector<uint> emptyFields;                     // [round]
        std::map<std::pair<uint, uint>, uint> pairings;
        std::vector<std::vector<bool>> playedOnSubType;    // [team][field subtype]

        explicit PlanCount(TestSetup& t)
        {
            const Capacity& c = t.in.setup.realCapacity;
            const Mapping& m = t.in.setup.mapping;
            games.assign(c.NTeams, 0);
            sameClub.assign(c.NTeams, 0);
            home.assign(c.NTeams, 0);
            away.assign(c.NTeams, 0);
            gamesOnFieldType.assign(c.NTeams, std::vector<uint>(c.NFieldTypes, 0));
            playedOnSubType.assign(c.NTeams, std::vector<bool>(c.NFieldSubTypes, false));
            emptyFields.assign(c.NRounds, 0);
            for (uint r = 0; r < c.NRounds; r++)
                for (uint f = 0; f < c.NFields; f++)
                {
                    IndexedGame g = t.out.indexedGamePlan.getGame(r, f);
                    uint a = g.getTeamByGameId(0).getTeamId();
                    uint b = g.getTeamByGameId(1).getTeamId();
                    if (g.getPause() || a >= c.NTeams || b >= c.NTeams)
                    {
                        emptyFields[r]++;
                        continue;
                    }
                    for (uint x : {a, b})
                    {
                        games[x]++;
                        gamesOnFieldType[x][m.indexedFieldTypeByFieldId[f]]++;
                        playedOnSubType[x][m.indexedFieldSubTypeByFieldId[f]] = true;
                        if (m.indexedClubByTeamId[a] == m.indexedClubByTeamId[b])
                            sameClub[x]++;
                    }
                    home[a]++;
                    away[b]++;
                    pairings[ {std::min(a, b), std::max(a, b)}]++;
                }
            for (uint x = 0; x < c.NTeams; x++)
                pauses.push_back(c.NRounds - games[x]);
        }
    };

    std::string config(uint rounds, const std::string& fields, const std::string& clubs, const std::string& conditions = "")
    {
        return "rounds: " + std::to_string(rounds) + "\n" + fields + clubs + "conditions:\n  max_pauses: 1\n" + conditions;
    }

    const std::string twoFields = field("A", "x", "1") + field("B", "x", "2");
    const std::string fourTeams = club("North", {"Red", "Blue"}) + club("South", {"Red", "Blue"});

}

TEST(conditions, everyPairingOnce)
{
    // 5 teams on 2 fields in 5 rounds: every pairing exactly once, every team pauses once
    TestSetup t(config(5, twoFields, club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue"})));
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    EXPECT_EQ(p.pairings.size(), 10u);
    for (auto [pair, n] : p.pairings)
        EXPECT_EQ(n, 1u) << pair.first << "-" << pair.second;
    for (uint x = 0; x < 5; x++)
        EXPECT_EQ(p.pauses[x], 1u) << "team " << x;
}

TEST(conditions, pauseGames)
{
    // 6 teams on 2 fields: 2 teams pause in every round
    std::string clubs = club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue", "Green"});
    TestSetup t("rounds: 3\n" + twoFields + clubs + "conditions:\n  max_pauses: 1\n");
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint x = 0; x < 6; x++)
        EXPECT_LE(p.pauses[x], 1u) << "team " << x;
    // 3 rounds * 2 pauses for 6 teams: impossible with no pause at all
    TestSetup none("rounds: 3\n" + twoFields + clubs + "conditions:\n  max_pauses: 0\n");
    EXPECT_FALSE(none.solve());
    // without the condition the teams may pause (no plan was found)
    TestSetup unlimited("rounds: 3\n" + twoFields + clubs);
    EXPECT_TRUE(unlimited.solve());
}

// [conditions.same_club] was ignored in English configurations (the English
// translation of the key is the statistic label "Same Club")
TEST(conditions, sameClub)
{
    TestSetup t(config(3, twoFields, fourTeams, "conditions:\n  max_same_club_games: 1\n"));
    EXPECT_TRUE(t.in.conditions.maxGamesSameClub.enabled);
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint x = 0; x < 4; x++)
        EXPECT_LE(p.sameClub[x], 1u) << "team " << x;
    // in 3 rounds without repeated pairings every team plays its club mate once
    TestSetup none(config(3, twoFields, fourTeams, "conditions:\n  max_same_club_games: 0\n"));
    EXPECT_FALSE(none.solve());
}

// Field types without a limit are not restricted (they used to get max 0,
// so a limit for A made every game on B impossible)
TEST(conditions, fieldTypes)
{
    TestSetup t(config(3, twoFields, fourTeams, "conditions:\n  field_types:\n    - {type: \"A\", max: 2}\n"));
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint x = 0; x < 4; x++)
    {
        EXPECT_LE(p.gamesOnFieldType[x][0], 2u) << "team " << x;
        EXPECT_EQ(p.games[x], 3u) << "team " << x;
    }
    // In each of the 3 rounds field A gets one of 2 possible games; every
    // choice gives one team 0 or 3 games on A, so 1..2 is impossible.
    TestSetup none(config(3, twoFields, fourTeams, "conditions:\n  field_types:\n    - {type: \"A\", min: 1, max: 2}\n"));
    EXPECT_FALSE(none.solve());
}

TEST(conditions, emptyFields)
{
    // 5 teams on 3 fields: 2 games and one team pauses, one field stays empty
    std::string threeFields = twoFields + field("A", "x", "3");
    std::string fiveTeams = club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue"});
    TestSetup t(config(3, threeFields, fiveTeams, "conditions:\n  max_empty_fields: 1\n"));
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint r = 0; r < 3; r++)
        EXPECT_EQ(p.emptyFields[r], 1u) << "round " << r;
    TestSetup none(config(3, threeFields, fiveTeams, "conditions:\n  max_empty_fields: 0\n"));
    EXPECT_FALSE(none.solve());
}

// An even number of teams with an empty field (4 teams on 3 fields): rounds
// without pause teams kept the pause teams of the previous round, so the
// search failed or crashed
TEST(conditions, emptyFieldEvenTeams)
{
    std::string threeFields = twoFields + field("A", "x", "3");
    TestSetup t(config(3, threeFields, fourTeams, "conditions:\n  max_empty_fields: 1\n"));
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint r = 0; r < 3; r++)
        EXPECT_EQ(p.emptyFields[r], 1u) << "round " << r;
}

TEST(conditions, homeAwayBalance)
{
    std::string clubs = club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue", "Green"});
    TestSetup t(config(5, twoFields + field("A", "x", "3"), clubs));
    ASSERT_TRUE(t.in.conditions.homeAwayBalance);
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    for (uint x = 0; x < 6; x++)
        EXPECT_LE(std::max(p.home[x], p.away[x]) - std::min(p.home[x], p.away[x]), 1u) << "team " << x;
}

TEST(conditions, subtypeDenyList)
{
    std::string fields = field("A", "Left", "1") + field("A", "Right", "2");
    TestSetup t(config(3, fields, fourTeams, "conditions:\n  subtype_deny:\n    - subtype: \"Left\"\n      teams: [{club: \"South\", team: \"Blue\"}]\n"));
    ASSERT_TRUE(t.solve());
    PlanCount p(t);
    uint southBlue = t.in.setup.teamIdArray[t.in.setup.clubIdByClubName["South"].value()][t.in.setup.colorIdByColor["Blue"].value()];
    uint left = t.in.setup.fieldSubTypeIdBySubTypeName["Left"];
    EXPECT_FALSE(p.playedOnSubType[southBlue][left]);
    EXPECT_EQ(p.games[southBlue], 3u);
}

// Sub tournaments of different sizes (3 fields and 1 field): the atomic index
// of the fields was set with the number of fields of the first set (heap
// overflow), and the tree search kept the rounds of the previous set
TEST(conditions, unevenSubTournaments)
{
    const std::string fields = field("A", "Left", "1") + field("A", "Left", "2") + field("A", "Left", "3") + field("A", "Right", "4");
    const std::string clubs = club("North", {"1", "2", "3", "4", "5", "6"}) + club("South", {"1", "2"});
    std::string deny = "conditions:\n  subtype_deny:\n    - subtype: \"Left\"\n      teams: [{club: \"South\", team: \"1\"}, {club: \"South\", team: \"2\"}]\n";
    deny += "conditions:\n  subtype_deny:\n    - subtype: \"Right\"\n      teams: [{club: \"North\", team: \"1\"}, {club: \"North\", team: \"2\"}, {club: \"North\", team: \"3\"}, {club: \"North\", team: \"4\"}, {club: \"North\", team: \"5\"}, {club: \"North\", team: \"6\"}]\n";
    deny += "conditions:\n  max_same_club_games: 3\n";
    TestSetup one(config(1, fields, clubs, deny));
    ASSERT_TRUE(one.solve());
    EXPECT_EQ(PlanCount(one).games, std::vector<uint>(8, 1));
    // 3 rounds: the two South teams play each other three times
    TestSetup three("repeat_pairings: true\niterations: 3\n" + config(3, fields, clubs, deny));
    ASSERT_TRUE(three.solve());
    PlanCount p(three);
    EXPECT_EQ(p.games, std::vector<uint>(8, 3));
    const uint right = three.in.setup.fieldSubTypeIdBySubTypeName["Right"];
    for (uint team = 0; team < 8; team++)
        EXPECT_EQ(p.playedOnSubType[team][right], team >= 6) << team;
}

TEST(conditions, maxStepsStopsTheSearch)
{
    TestSetup t(config(5, twoFields, club("North", {"Red", "Blue", "Green"}) + club("South", {"Red", "Blue"})));
    EXPECT_FALSE(t.solve(3));
    EXPECT_LE(t.steps, 3u);
}

// One field with two teams: one round is the only possible plan; more rounds
// are not possible without repeated pairings (used to crash)
TEST(conditions, singleField)
{
    const std::string oneField = field("A", "x", "1");
    const std::string twoTeams = club("North", {"Red", "Blue"});
    TestSetup one("rounds: 1\n" + oneField + twoTeams);
    ASSERT_TRUE(one.solve());
    PlanCount p(one);
    EXPECT_EQ(p.games, (std::vector<uint> {1, 1}));
    TestSetup two("rounds: 2\n" + oneField + twoTeams);
    EXPECT_FALSE(two.solve());
}

// Tournaments the search cannot handle are an error before the tables are
// built: 10 fields have 654 million pairings of the 20 teams (the tables
// used to run out of memory), 8 fields with 2 field types and a subtype
// 142 million candidates that would be checked one by one
TEST(conditions, searchTooLarge)
{
    auto error = [](const std::string & config)
    {
        TestSetup t(config);
        try
        {
            (void)t.solve();
        }
        catch (const Exception& e)
        {
            return e;
        }
        return Exception(ErrorCode::none);
    };
    std::string fields;
    for (uint i = 1; i <= 10; i++)
        fields += field("A", "x", std::to_string(i));
    const std::string clubs = club("North", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"}) + club("South", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
    const Exception pairings = error("rounds: 2\n" + fields + clubs);
    EXPECT_EQ(pairings.code(), ErrorCode::searchTooManyPairings);
    EXPECT_EQ(pairings.params().front(), "654729075");

    std::string eight;
    for (uint i = 1; i <= 8; i++)
        eight += field("T" + std::to_string(i % 2), "x", std::to_string(i));
    const std::string clubs8 = club("North", {"1", "2", "3", "4", "5", "6", "7", "8"}) + club("South", {"1", "2", "3", "4", "5", "6", "7", "8"});
    const std::string deny = "conditions:\n  subtype_deny:\n    - subtype: \"x\"\n      teams: [{club: \"North\", team: \"1\"}]\n";
    const Exception rounds = error("rounds: 2\n" + eight + clubs8 + deny);
    EXPECT_EQ(rounds.code(), ErrorCode::searchTooLarge);
    EXPECT_EQ(rounds.params(), (std::vector<std::string> {"141891750", "30000000"}));
}

// With repeated pairings the search skipped the repetitions of a pairing when
// it went one round down, so one field had no plan for more than one round
TEST(conditions, singleFieldRepeatedPairings)
{
    const std::string oneField = field("A", "x", "1");
    const std::string twoTeams = club("North", {"Red", "Blue"});
    for (uint rounds = 1; rounds <= 4; rounds++)
    {
        TestSetup t("rounds: " + std::to_string(rounds) + "\nrepeat_pairings: true\niterations: 4\n" + oneField + twoTeams);
        ASSERT_TRUE(t.solve()) << rounds << " rounds";
        PlanCount p(t);
        EXPECT_EQ(p.games, (std::vector<uint> {rounds, rounds}));
    }
    // more rounds than iterations: the pairing would be played too often
    TestSetup tooMany("rounds: 3\nrepeat_pairings: true\niterations: 2\n" + oneField + twoTeams);
    EXPECT_FALSE(tooMany.solve());
}

// The order of the rounds (optimize_rounds): adjacent rounds without a
// common pause team and without the same game on the same field, and the
// pause teams of every round shown with its games. The pause teams came
// from another round (inverse order) and the order chosen was the one after
// the first valid order; with 7 teams the pause teams were read past the end.
TEST(conditions, roundOrder)
{
    const std::string fields = field("A", "x", "1") + field("A", "x", "2") + field("A", "x", "3");
    const std::string clubs = club("North", {"c0", "c1", "c2", "c3"}) + club("South", {"c0", "c1", "c2"});
    TestSetup t("rounds: 5\n" + fields + clubs + "conditions:\n  max_pauses: 2\nconditions:\n  max_empty_fields: 1\n");
    ASSERT_TRUE(t.solve());
    const Capacity& c = t.in.setup.realCapacity;
    IndexedGamePlan& plan = t.out.indexedGamePlan;
    std::vector<std::vector<std::pair<uint, uint>>> games(c.NRounds);
    std::vector<std::vector<bool>> pauses(c.NRounds, std::vector<bool>(c.NTeams, false));
    for (uint r = 0; r < c.NRounds; r++)
    {
        std::vector<uint> seen(c.NTeams, 0);
        for (uint f = 0; f < c.NFields; f++)
        {
            IndexedGame g = plan.getGame(r, f);
            const uint a = g.getTeamByGameId(0).getTeamId();
            const uint b = g.getTeamByGameId(1).getTeamId();
            games[r].emplace_back(g.getPause() ? TEAM_UNDEFINED : std::min(a, b), g.getPause() ? TEAM_UNDEFINED : std::max(a, b));
            if (!g.getPause())
            {
                seen[a]++;
                seen[b]++;
            }
        }
        for (uint k = 0; k < plan.getNPauseTeams(r); k++)
        {
            const uint x = plan.getPauseTeam(r, k).getTeamId();
            seen[x]++;
            pauses[r][x] = true;
        }
        for (uint x = 0; x < c.NTeams; x++)
            EXPECT_EQ(seen[x], 1u) << "round " << r << " team " << x;
    }
    for (uint r = 0; r + 1 < c.NRounds; r++)
    {
        for (uint x = 0; x < c.NTeams; x++)
            EXPECT_FALSE(pauses[r][x] && pauses[r + 1][x]) << "rounds " << r << ", " << r + 1 << " team " << x;
        for (uint f = 0; f < c.NFields; f++)
            EXPECT_FALSE(games[r][f].first != TEAM_UNDEFINED && games[r][f] == games[r + 1][f]) << "rounds " << r << ", " << r + 1 << " field " << f;
    }
}

// Sub tournaments with different field types: the second one (fields of
// type B) was searched with the field types of the first fields of the
// configuration (A), and the games on B were not counted (the set counted
// its own types, B has the id 1). A limit on B had no effect. A limit
// applies in the sub tournaments with fields of the type: the minimum on B
// does not concern the North teams on the A fields.
TEST(conditions, fieldTypesOfSubTournaments)
{
    const std::string fields = field("A", "Left", "1") + field("A", "Left", "2") + field("B", "Right", "3") + field("B", "Right", "4");
    const std::string clubs = club("North", {"1", "2", "3", "4"}) + club("South", {"1", "2", "3", "4"});
    std::string deny = "conditions:\n  subtype_deny:\n    - subtype: \"Left\"\n      teams: [{club: \"South\", team: \"1\"}, {club: \"South\", team: \"2\"}, {club: \"South\", team: \"3\"}, {club: \"South\", team: \"4\"}]\n";
    deny += "conditions:\n  subtype_deny:\n    - subtype: \"Right\"\n      teams: [{club: \"North\", team: \"1\"}, {club: \"North\", team: \"2\"}, {club: \"North\", team: \"3\"}, {club: \"North\", team: \"4\"}]\n";
    // every South team plays all 3 rounds on B
    TestSetup limited(config(3, fields, clubs, deny + "conditions:\n  field_types:\n    - {type: \"B\", max: 2}\n"));
    EXPECT_FALSE(limited.solve());
    EXPECT_EQ(limited.result.status(), SearchStatus::impossible);
    TestSetup allowed(config(3, fields, clubs, deny + "conditions:\n  field_types:\n    - {type: \"B\", min: 3}\n"));
    ASSERT_TRUE(allowed.solve());
    PlanCount p(allowed);
    for (uint team = 4; team < 8; team++)
        EXPECT_EQ(p.gamesOnFieldType[team][1], 3u) << team;
}
