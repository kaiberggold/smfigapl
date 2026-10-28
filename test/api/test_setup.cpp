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


// The public setup to the internal model: validation with paths, the
// Input the search works on. Setups are built in code.

#include <limits>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "test_setup.hpp"
#include "smfigapl/model/sm_setup.hpp"
#include "smfigapl/formats/yaml.hpp"

using namespace smfigapl;

namespace
{
    TournamentSetup validSetup()
    {
        TournamentSetup s;
        s.rounds = 3;
        s.fields = {{"1", "Small", "Left"}, {"2", "Large", "Left"}, {"3", "Small", "Right"}};
        s.clubs = {{"North", {"Red", "Blue"}}, {"South", {"Red", "Green"}}};
        s.conditions.maxPauses = 1;
        s.conditions.maxSameClubGames = 2;
        s.conditions.maxEmptyFields = 1;
        s.conditions.fieldTypes = {{"Small", 1, 2}, {"Large", std::nullopt, 3}};
        s.conditions.subtypeDeny = {{"Right", {{"South", "Green"}}}};
        return s;
    }

    // Code and path of every problem
    std::vector<std::pair<ProblemCode, std::string>> codesAndPaths(const TournamentSetup& setup)
    {
        std::vector<std::pair<ProblemCode, std::string>> result;
        for (const Problem& p : validateSetup(setup))
        {
            EXPECT_EQ(p.severity, Severity::error);
            result.emplace_back(p.code, p.path);
        }
        return result;
    }

    using CodesAndPaths = std::vector<std::pair<ProblemCode, std::string>>;
}

TEST(setupValidation, validSetup)
{
    EXPECT_TRUE(validateSetup(validSetup()).empty());
}

TEST(setupValidation, roundsAndFields)
{
    TournamentSetup s = validSetup();
    s.rounds = 0;
    s.fields.clear();
    s.conditions = {};
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::nRoundsLt1, "rounds"}, {ProblemCode::nFieldsLt1, "fields"}}));
    s.rounds = MAX_ROUNDS + 1;
    s.fields.assign(MAX_FIELDS + 1, Field{"1", "A", "x"});
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::nRoundsGtMax, "rounds"}, {ProblemCode::nFieldsGtMax, "fields"}}));
}

TEST(setupValidation, clubsAndTeams)
{
    TournamentSetup s = validSetup();
    s.clubs.clear();
    s.conditions.subtypeDeny.clear();
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::noClubsDefined, "clubs"}}));
    // a club without teams does not count
    s.clubs = {{"North", {}}};
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::noClubsDefined, "clubs"}}));

    s.clubs.clear();
    for (uint i = 0; i <= MAX_CLUBS; i++)
        s.clubs.push_back({"Club" + std::to_string(i), {"Red"}});
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::clubIndexGtMax, "clubs[" + std::to_string(MAX_CLUBS) + "]"}}));

    s.clubs = {{"North", {}}};
    for (int i = 0; i <= MAX_TEAMCOLORS; i++)
        s.clubs.front().teams.push_back("Color" + std::to_string(i));
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths {{ProblemCode::teamColorIndexGtMax, "clubs[0].teams[" + std::to_string(MAX_TEAMCOLORS) + "]"}}));
}

// The names in the conditions must exist
TEST(setupValidation, conditions)
{
    TournamentSetup s = validSetup();
    s.conditions.fieldTypes = {{"Small", 3, 2}, {"Unknown", 1, std::nullopt}};
    s.conditions.subtypeDeny = {{"Middle", {{"North", "Red"}}}, {"Left", {{"North", "Green"}, {"South", "Red"}, {"West", "Red"}}}};
    EXPECT_EQ(codesAndPaths(s), (CodesAndPaths
    {
        {ProblemCode::maxLtMin, "conditions.fieldTypes[0]"},
        {ProblemCode::invalidValue, "conditions.fieldTypes[1].type"},
        {ProblemCode::invalidValue, "conditions.subtypeDeny[0].subtype"},
        {ProblemCode::invalidValue, "conditions.subtypeDeny[1].teams[0]"},
        {ProblemCode::invalidValue, "conditions.subtypeDeny[1].teams[2]"}
    }));
    const auto problems = validateSetup(s);
    EXPECT_EQ(problems[0].parameters, (std::vector<std::string> {"3", "2"}));
    EXPECT_EQ(problems[1].parameters, (std::vector<std::string> {"field_types", "Unknown"}));
    EXPECT_EQ(problems[3].parameters, (std::vector<std::string> {"teams_deny", "North Green"}));
}

// The Input of a setup is the Input of the same configuration as file
TEST(setupInput, sameAsFromFile)
{
    const TournamentSetup setup = validSetup();
    Input in;
    buildInput(setup, in);
    TestSetup file(formats::yaml::writeSetup(setup));

    const auto& a = in.setup;
    const auto& b = file.in.setup;
    EXPECT_EQ(a.realCapacity.NRounds, b.realCapacity.NRounds);
    EXPECT_EQ(a.realCapacity.NFields, b.realCapacity.NFields);
    EXPECT_EQ(a.realCapacity.NFieldTypes, b.realCapacity.NFieldTypes);
    EXPECT_EQ(a.realCapacity.NFieldSubTypes, b.realCapacity.NFieldSubTypes);
    EXPECT_EQ(a.realCapacity.NTeams, b.realCapacity.NTeams);
    EXPECT_EQ(a.mapping.indexedFieldTypeByFieldId, b.mapping.indexedFieldTypeByFieldId);
    EXPECT_EQ(a.mapping.indexedFieldSubTypeByFieldId, b.mapping.indexedFieldSubTypeByFieldId);
    EXPECT_EQ(a.fieldNameByFieldId, b.fieldNameByFieldId);
    EXPECT_EQ(a.teamIdArray, b.teamIdArray);
    const auto& c = in.conditions;
    const auto& d = file.in.conditions;
    EXPECT_EQ(c.maxPauseGames.value, d.maxPauseGames.value);
    EXPECT_EQ(c.maxGamesSameClub.values, d.maxGamesSameClub.values);
    EXPECT_EQ(c.maxFieldsEmpty, d.maxFieldsEmpty);
    EXPECT_EQ(c.limGamesFieldType.values.min, d.limGamesFieldType.values.min);
    EXPECT_EQ(c.limGamesFieldType.values.max, d.limGamesFieldType.values.max);
    EXPECT_EQ(c.fieldSubTypeDeny.values, d.fieldSubTypeDeny.values);

    EXPECT_EQ(c.limGamesFieldType.values.min, (std::vector<uint> {1, 0}));
    EXPECT_EQ(c.limGamesFieldType.values.max, (std::vector<uint> {2, 3}));
    EXPECT_TRUE(c.fieldSubTypeDeny.enabled);
    EXPECT_EQ(c.fieldSubTypeDeny.values[3], (std::vector<uint> {0, 1}));
}

TEST(setupInput, settings)
{
    TournamentSetup setup = validSetup();
    setup.iterations = 2;
    setup.repeatPairings = true;
    setup.homeAwayBalance = false;
    setup.optimizeRoundOrder = false;
    setup.conditions = {};
    Input in;
    buildInput(setup, in);
    EXPECT_EQ(in.conditions.iterations, 2u);
    EXPECT_TRUE(in.conditions.repeatPairings);
    EXPECT_FALSE(in.conditions.homeAwayBalance);
    EXPECT_FALSE(in.conditions.optimizeRounds);
    EXPECT_FALSE(in.conditions.maxPauseGames.enabled);
    EXPECT_FALSE(in.conditions.maxGamesSameClub.enabled);
    EXPECT_FALSE(in.conditions.limGamesFieldType.enabled);
    EXPECT_FALSE(in.conditions.fieldSubTypeDeny.enabled);
}

// The last possible club may have more than one team (it used to be an error)
TEST(setupInput, mostClubs)
{
    TournamentSetup setup = validSetup();
    setup.clubs.clear();
    setup.conditions = {};
    for (uint i = 0; i < MAX_CLUBS; i++)
        setup.clubs.push_back({"Club" + std::to_string(i), {"Red", "Blue"}});
    EXPECT_TRUE(validateSetup(setup).empty());
    Input in;
    buildInput(setup, in);
    EXPECT_EQ(in.setup.realCapacity.NClubs, MAX_CLUBS);
    EXPECT_EQ(in.setup.realCapacity.NTeams, 2 * MAX_CLUBS);
}
