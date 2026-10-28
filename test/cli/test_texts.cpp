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


// The texts of the command line tool: translations, the messages of the
// problems (the library gives codes and values only), the output labels

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include "cli/texts.hpp"
#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/planner.hpp"
#include "test_misc.hpp"

using namespace smfigapl;
using cli::Lang;

namespace
{
    Lang language(const std::string& l)
    {
        Lang lang;
        lang.setLanguage(l);
        return lang;
    }

    Problem problem(ProblemCode code, std::vector<std::string> parameters)
    {
        Problem p;
        p.code = code;
        p.parameters = std::move(parameters);
        return p;
    }
}

TEST(cliTexts, translateInCurrentLanguage)
{
    Lang lang;
    lang.setLanguage("EN");
    EXPECT_EQ(lang.get("round"), "Round");
    lang.setLanguage("DE");
    EXPECT_EQ(lang.get("round"), "Runde");
    EXPECT_EQ(lang.get("round", "EN"), "Round");
    // an unknown key is returned unchanged
    EXPECT_EQ(lang.get("no_such_key"), "no_such_key");
}

TEST(cliTexts, everyProblemHasAMessage)
{
    for (const std::string l : {"EN", "DE"})
    {
        const Lang lang = language(l);
        for (ProblemCode code :
                {
                    ProblemCode::parseError, ProblemCode::nRoundsNotSet, ProblemCode::noFieldsDefined, ProblemCode::noClubsDefined,
                    ProblemCode::missingValue, ProblemCode::invalidValue, ProblemCode::unsupportedFormatVersion, ProblemCode::unknownKey,
                    ProblemCode::clubIndexGtMax, ProblemCode::clubIndexOverflow, ProblemCode::teamColorIndexGtMax, ProblemCode::nFieldsGtMax,
                    ProblemCode::nFieldsLt1, ProblemCode::nRoundsGtMax, ProblemCode::nRoundsLt1, ProblemCode::searchTooLarge,
                    ProblemCode::searchTooManyPairings, ProblemCode::maxLtMin, ProblemCode::impossibleFieldTypeMin, ProblemCode::impossibleFieldTypeMax,
                    ProblemCode::impossibleTeamFieldTypeMin, ProblemCode::impossibleTeamGames, ProblemCode::impossiblePauses,
                    ProblemCode::impossibleSameClub, ProblemCode::impossibleOpponents, ProblemCode::impossibleSearch, ProblemCode::impossibleGames
                })
            EXPECT_NE(lang.getErrorMessage(code), "") << l << " " << static_cast<int>(code);
    }
    // no text for an unknown language
    EXPECT_EQ(language("XX").getErrorMessage(ProblemCode::nRoundsLt1), "");
}

TEST(cliTexts, problemMessages)
{
    // the key in the language of the message
    EXPECT_EQ(cli::problemMessage(problem(ProblemCode::missingValue, {"subtype"}), language("EN")), "Error number13: Value missing: subtype");
    EXPECT_EQ(cli::problemMessage(problem(ProblemCode::missingValue, {"subtype"}), language("DE")), "Fehler Nummer13: Wert fehlt: Zuordnung");
    EXPECT_EQ(cli::problemMessage(problem(ProblemCode::maxLtMin, {"3", "2"}), language("DE")), "Fehler Nummer200: Minimum 3 größer als Maximum 2 (Feldtypen)");
    // the invalid value on the next line
    EXPECT_EQ(cli::problemMessage(problem(ProblemCode::invalidValue, {"teams_deny", "North Green"}), language("EN")),
              "Error number14: Invalid value in teams_deny\nNorth Green");
    // a syntax error with its position in the file
    Problem syntax = problem(ProblemCode::parseError, {"Expected a value"});
    syntax.position = TextPosition{2, 9};
    EXPECT_EQ(cli::problemMessage(syntax, language("EN"), "in.yaml"),
              "Error number1: Parsing Error\nExpected a value\n\t(error occurred at line 2, column 9 of 'in.yaml')");
    // a warning
    Problem unknown = problem(ProblemCode::unknownKey, {"rouns"});
    unknown.severity = Severity::warning;
    EXPECT_EQ(cli::problemMessage(unknown, language("EN")), "Warning number16: Unknown key rouns ignored");
    // a reason without a value
    EXPECT_EQ(cli::problemText(problem(ProblemCode::impossiblePauses, {"6", "0"}), language("EN")), "At least 6 pauses are needed, the pause limit allows only 0");
}

TEST(cliTexts, labels)
{
    const report::Labels de = cli::labels(language("DE"));
    EXPECT_EQ(de.round, "Runde");
    EXPECT_EQ(de.pause, "Pause");
    EXPECT_EQ(de.solved, "Spielplan berechnet");
    EXPECT_EQ(de.games, "Anzahl Spiele");
    EXPECT_EQ(cli::labels(language("EN")).round, "Round");
}

// The configurations with too many or too few rounds or fields: every
// problem with its message
class cliValidationMessages : public ::testing::TestWithParam<std::string>
{
};

TEST_P(cliValidationMessages, messages)
{
    std::ifstream file(GetParam(), std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    const auto setup = formats::yaml::readSetup(text);
    ASSERT_TRUE(setup.has_value()) << GetParam();
    const std::vector<Problem> problems = validate(*setup);
    EXPECT_FALSE(problems.empty()) << GetParam();
    const Lang en = language("EN");
    auto expect = [&](ProblemCode code, const std::string & message)
    {
        auto p = std::ranges::find_if(problems, [code](const Problem & x) { return x.code == code; });
        ASSERT_NE(p, problems.end()) << GetParam();
        EXPECT_NE(cli::problemMessage(*p, en).find(message), std::string::npos) << cli::problemMessage(*p, en);
    };
    if (setup->rounds > 10)
        expect(ProblemCode::nRoundsGtMax, "Number of Rounds too big (Maximum 10)");
    if (setup->rounds < 1)
        expect(ProblemCode::nRoundsLt1, "Number of Rounds too small");
    if (setup->fields.empty())
        expect(ProblemCode::nFieldsLt1, "Number of Fields too small");
    if (setup->fields.size() > 10)
        expect(ProblemCode::nFieldsGtMax, "Number of Fields larger than maximum allowed number of 10");
}

INSTANTIATE_TEST_SUITE_P(input_validation, cliValidationMessages, testing::ValuesIn(folderList(testDataDir + "/input_validation")));
