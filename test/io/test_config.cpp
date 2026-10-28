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

#include <string>
#include "gtest/gtest.h"

#include "test_setup.hpp"

using namespace smfigapl;

namespace
{

    const std::string twoFields = field("Small", "Left", "1") + field("Large", "Left", "2");
    const std::string twoClubs = club("North", {"Red", "Blue"}) + club("South", {"Red", "Green"});

}

TEST(config, roundsFieldsClubs)
{
    TestSetup t("rounds: 5\n" + twoFields + twoClubs);
    const Capacity& c = t.in.setup.realCapacity;
    EXPECT_EQ(c.NRounds, 5u);
    EXPECT_EQ(c.NFields, 2u);
    EXPECT_EQ(c.NFieldTypes, 2u);
    EXPECT_EQ(c.NFieldSubTypes, 1u);
    EXPECT_EQ(c.NClubs, 2u);
    EXPECT_EQ(c.NTeamColors, 3u);
    EXPECT_EQ(c.NTeams, 4u);
    EXPECT_EQ(t.in.setup.fieldNameByFieldId, (std::vector<std::string> {"1", "2"}));
    EXPECT_EQ(t.in.setup.namedTeamByTeamId[0].getTeamName(), "North Red");
    EXPECT_EQ(t.in.setup.namedTeamByTeamId[3].getTeamName(), "South Green");
    // teams are identified by club and color
    EXPECT_EQ(t.in.setup.teamIdArray[t.in.setup.clubIdByClubName["South"].value()][t.in.setup.colorIdByColor["Red"].value()], 2u);
}

TEST(config, fieldTypeIdsInOrderOfFirstUse)
{
    TestSetup t("rounds: 1\n" + field("A", "x", "1") + field("A", "y", "2") + field("B", "x", "3") + twoClubs);
    EXPECT_EQ(t.in.setup.mapping.indexedFieldTypeByFieldId, (std::vector<uint> {0, 0, 1}));
    EXPECT_EQ(t.in.setup.mapping.indexedFieldSubTypeByFieldId, (std::vector<uint> {0, 1, 0}));
    EXPECT_EQ(t.in.setup.realCapacity.NFieldTypes, 2u);
}

// Only the canonical keys are read: other spellings (the TOML files
// accepted capitalized and German keys) are unknown keys, and the required
// ones are missing
TEST(config, canonicalKeysOnly)
{
    EXPECT_EQ(configError("Rounds: 4\n" + twoFields + twoClubs), ErrorCode::nRoundsNotSet);
    EXPECT_EQ(configError("rounds: 4\nFelder:\n  - {name: '1', type: A, subtype: x}\n" + twoClubs), ErrorCode::noFieldsDefined);
}

TEST(config, optionalSettings)
{
    TestSetup defaults("rounds: 2\n" + twoFields + twoClubs);
    EXPECT_EQ(defaults.in.conditions.iterations, 1u);
    EXPECT_FALSE(defaults.in.conditions.repeatPairings);
    EXPECT_TRUE(defaults.in.conditions.homeAwayBalance);

    TestSetup t("rounds: 2\niterations: 2\nrepeat_pairings: true\nhome_away_balance: false\n" + twoFields + twoClubs);
    EXPECT_EQ(t.in.conditions.iterations, 2u);
    EXPECT_TRUE(t.in.conditions.repeatPairings);
    EXPECT_FALSE(t.in.conditions.homeAwayBalance);
}

TEST(config, defaultConditions)
{
    TestSetup t("rounds: 2\n" + twoFields + twoClubs);
    EXPECT_FALSE(t.in.conditions.maxPauseGames.enabled);
    EXPECT_FALSE(t.in.conditions.maxGamesSameClub.enabled);
    EXPECT_FALSE(t.in.conditions.limGamesFieldType.enabled);
    EXPECT_FALSE(t.in.conditions.fieldSubTypeDeny.enabled);
    EXPECT_EQ(t.in.conditions.maxFieldsEmpty, 0u);
}

TEST(config, conditions)
{
    TestSetup t("rounds: 3\n" + twoFields + twoClubs +
                "conditions:\n"
                "  max_pauses: 1\n"
                "  max_same_club_games: 2\n"
                "  max_empty_fields: 1\n"
                "  field_types:\n"
                "    - {type: Small, min: 1, max: 2}\n"
                "    - {type: Large, max: 3}\n");
    const GamePlanConditions& c = t.in.conditions;
    EXPECT_TRUE(c.maxPauseGames.enabled);
    EXPECT_EQ(c.maxPauseGames.value, 1u);
    EXPECT_TRUE(c.maxGamesSameClub.enabled);
    EXPECT_EQ(c.maxGamesSameClub.values, (std::vector<uint> {2, 2, 2, 2}));
    EXPECT_EQ(c.maxFieldsEmpty, 1u);
    EXPECT_TRUE(c.limGamesFieldType.enabled);
    EXPECT_EQ(c.limGamesFieldType.values.min, (std::vector<uint> {1, 0}));
    EXPECT_EQ(c.limGamesFieldType.values.max, (std::vector<uint> {2, 3}));
}

// The limits are stored by field type id. They used to be stored by the
// index of the first field of that type, which wrote past the end when a
// type was used twice before the next one (A, A, B).
TEST(config, fieldTypeLimitsByTypeId)
{
    TestSetup t("rounds: 3\n" + field("A", "x", "1") + field("A", "x", "2") + field("B", "x", "3") + twoClubs +
                "conditions:\n"
                "  field_types:\n"
                "    - {type: B, min: 1, max: 2}\n"
                "    - {type: A, min: 0, max: 3}\n");
    EXPECT_EQ(t.in.conditions.limGamesFieldType.values.min, (std::vector<uint> {0, 1}));
    EXPECT_EQ(t.in.conditions.limGamesFieldType.values.max, (std::vector<uint> {3, 2}));
}

TEST(config, subtypeDenyList)
{
    TestSetup t("rounds: 2\n" + field("A", "Left", "1") + field("A", "Right", "2") + twoClubs +
                "conditions:\n"
                "  subtype_deny:\n"
                "    - {subtype: Left, teams: [{club: South, team: Green}]}\n");
    const auto& deny = t.in.conditions.fieldSubTypeDeny;
    EXPECT_TRUE(deny.enabled);
    uint southGreen = t.in.setup.teamIdArray[t.in.setup.clubIdByClubName["South"].value()][t.in.setup.colorIdByColor["Green"].value()];
    uint left = t.in.setup.fieldSubTypeIdBySubTypeName["Left"];
    for (uint team = 0; team < t.in.setup.realCapacity.NTeams; team++)
        for (uint st = 0; st < t.in.setup.realCapacity.NFieldSubTypes; st++)
            EXPECT_EQ(deny.values[team][st], team == southGreen && st == left) << team << " " << st;
}

// Errors while reading are thrown as Exception with their code
TEST(configErrors, missingSections)
{
    EXPECT_EQ(configError(twoFields + twoClubs), ErrorCode::nRoundsNotSet);
    EXPECT_EQ(configError("rounds: 2\n" + twoClubs), ErrorCode::noFieldsDefined);
    EXPECT_EQ(configError("rounds: 2\n" + twoFields), ErrorCode::noClubsDefined);
}

TEST(configErrors, minGreaterThanMax)
{
    try
    {
        TestSetup t("rounds: 2\n" + twoFields + twoClubs + "conditions:\n  field_types:\n    - {type: Small, min: 3, max: 2}\n");
        FAIL() << "no error";
    }
    catch (const Exception& e)
    {
        EXPECT_EQ(e.code(), ErrorCode::maxLtMin);
        EXPECT_EQ(e.params(), (std::vector<std::string> {"3", "2"}));
    }
}

// A syntax error used to be printed, then parsing went on with an empty
// table and reported "rounds missing"
TEST(configErrors, syntaxError)
{
    try
    {
        TestSetup t("rounds: [2\n" + twoFields + twoClubs);
        FAIL() << "no error";
    }
    catch (const Exception& e)
    {
        EXPECT_EQ(e.code(), ErrorCode::parseError);
        EXPECT_NE(e.detail().find("line "), std::string::npos) << e.detail();
    }
}

TEST(configErrors, tooManyClubsAndColors)
{
    std::string clubs;
    for (uint i = 0; i <= MAX_CLUBS; i++)
        clubs += club("Club" + std::to_string(i), {"Red"});
    EXPECT_EQ(configError("rounds: 2\n" + twoFields + clubs), ErrorCode::clubIndexGtMax);
    std::vector<std::string> manyColors;
    for (int i = 0; i <= MAX_TEAMCOLORS; i++)
        manyColors.push_back("Color" + std::to_string(i));
    EXPECT_EQ(configError("rounds: 2\n" + twoFields + club("North", manyColors)), ErrorCode::teamColorIndexGtMax);
}

// The number of colors is limited, not the length of a color name (a name
// with 16 or more characters used to become the first color)
TEST(config, longColorName)
{
    TestSetup t("rounds: 2\n" + twoFields + club("North", {"Red", "Dark blue and white stripes"}) + club("South", {"Red"}));
    EXPECT_EQ(t.in.setup.realCapacity.NTeamColors, 2u);
    EXPECT_EQ(t.in.setup.namedTeamByTeamId[1].getTeamName(), "North Dark blue and white stripes");
}

// Missing or wrong values are errors with the key, they used to crash
TEST(configErrors, missingAndInvalidValues)
{
    const std::string rounds = "rounds: 2\n";
    EXPECT_EQ(configError(rounds + "fields:\n  - {type: A, name: '1'}\n" + twoClubs), ErrorCode::missingValue);
    EXPECT_EQ(configError(rounds + twoFields + "clubs:\n  - name: North\n"), ErrorCode::missingValue);
    EXPECT_EQ(configError(rounds + twoFields + "clubs:\n  - teams: [Red]\n"), ErrorCode::missingValue);
    EXPECT_EQ(configError(rounds + twoFields + "clubs:\n  - {name: North, teams: [Red, [3]]}\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(rounds + "fields: [A]\n" + twoClubs), ErrorCode::invalidValue);
    EXPECT_EQ(configError(rounds + twoFields + twoClubs + "conditions:\n  max_pauses: {min: 1}\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(rounds + twoFields + twoClubs + "conditions:\n  field_types:\n    - {max: 1}\n"), ErrorCode::missingValue);
    // a field type limit for a type no field has
    EXPECT_EQ(configError(rounds + twoFields + twoClubs + "conditions:\n  field_types:\n    - {type: Unknown, max: 1}\n"), ErrorCode::invalidValue);
}

TEST(configErrors, formatEntriesMustBeMaps)
{
    // a format list entry that is no map is an error (used to crash)
    const std::string base = "rounds: 2\n" + twoFields + twoClubs;
    EXPECT_EQ(configError(base + "format:\n  information:\n    text_fields: [1]\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(base + "format:\n  information:\n    date_time: [x]\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(base + "format:\n  tournament:\n    columns:\n      column: [true]\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(base + "format:\n  statistic:\n    rows:\n      row: [1]\n"), ErrorCode::invalidValue);
}

TEST(configErrors, denyList)
{
    const std::string base = "rounds: 2\n" + field("A", "Left", "1") + field("A", "Right", "2") + twoClubs + "conditions:\n  subtype_deny:\n";
    // unknown subtype, unknown team, a team without its club (used to deny team 0 or be ignored)
    EXPECT_EQ(configError(base + "    - {subtype: Middle, teams: [{club: North, team: Red}]}\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(base + "    - {subtype: Left, teams: [{club: North, team: Green}]}\n"), ErrorCode::invalidValue);
    EXPECT_EQ(configError(base + "    - {subtype: Left, teams: [{team: Red}]}\n"), ErrorCode::missingValue);
    EXPECT_EQ(configError(base + "    - {subtype: Left}\n"), ErrorCode::missingValue);
    EXPECT_EQ(configError(base + "    - {subtype: Left, teams: [{club: North, team: Red}]}\n"), ErrorCode::none);
}
