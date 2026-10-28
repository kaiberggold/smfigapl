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


// What only the YAML adapter has: the keys, the positions in the text,
// the YAML syntax, the files of the command line tool

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/formats/yaml.hpp"

using namespace smfigapl;
namespace yaml = smfigapl::formats::yaml;

namespace
{
    const std::string fieldsAndClubs = "fields:\n  - {name: '1', type: A, subtype: x}\nclubs:\n  - name: North\n    teams: [Red, Blue]\n";
    const std::string base = "rounds: 2\n" + fieldsAndClubs;

    std::string fileText(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    // The line of a problem, 0 without a position
    std::uint32_t line(const Problem& p)
    {
        return p.position.value_or(TextPosition{}).line;
    }

    // The problems of a text that has no setup
    std::vector<Problem> errors(const std::string& text)
    {
        const auto r = yaml::readSetup(text);
        EXPECT_FALSE(r.has_value()) << text;
        return r.problems();
    }
}

TEST(yamlSetup, allKeys)
{
    const auto r = yaml::readSetup("format_version: 1\nrounds: 3\niterations: 2\nrepeat_pairings: true\nhome_away_balance: false\noptimize_round_order: FALSE\n"
                                   "fields:\n  - name: '1'\n    type: A\n    subtype: Left\n  - {name: 2, type: B, subtype: Right}\n"
                                   "clubs:\n  - {name: North, teams: [Red, '2']}\n"
                                   "conditions:\n  max_pauses: 1\n  max_same_club_games: 0\n  max_empty_fields: 1\n"
                                   "  field_types:\n    - {type: A, min: 1}\n    - {type: B, max: 2}\n"
                                   "  subtype_deny:\n    - subtype: Left\n      teams:\n        - {club: North, team: Red}\n");
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(r.problems().empty());
    TournamentSetup s;
    s.rounds = 3;
    s.iterations = 2;
    s.repeatPairings = true;
    s.homeAwayBalance = false;
    s.optimizeRoundOrder = false;
    // a plain number is read as text where the key is a name
    s.fields = {{"1", "A", "Left"}, {"2", "B", "Right"}};
    s.clubs = {{"North", {"Red", "2"}}};
    s.conditions.maxPauses = 1;
    s.conditions.maxSameClubGames = 0;
    s.conditions.maxEmptyFields = 1;
    s.conditions.fieldTypes = {{"A", 1, std::nullopt}, {"B", std::nullopt, 2}};
    s.conditions.subtypeDeny = {{"Left", {{"North", "Red"}}}};
    EXPECT_EQ(*r, s);
}

TEST(yamlSetup, emptyTextAndComments)
{
    EXPECT_EQ(errors("").front().code, ProblemCode::nRoundsNotSet);
    EXPECT_EQ(errors("# only a comment\n").front().code, ProblemCode::nRoundsNotSet);
    // tabs after ':' as in hand written texts
    EXPECT_TRUE(yaml::readSetup("rounds:\t2\n" + fieldsAndClubs).has_value());
}

TEST(yamlSetup, syntaxErrorWithPosition)
{
    const auto problems = errors("rounds: 2\nfields: [1, 2\nclubs: []\n");
    ASSERT_EQ(problems.size(), 1u);
    EXPECT_EQ(problems.front().code, ProblemCode::parseError);
    EXPECT_GE(line(problems.front()), 2u);
    EXPECT_FALSE(problems.front().parameters.empty());
}

TEST(yamlSetup, notOneMap)
{
    for (const char* text : {"just text\n", "- rounds: 2\n", "rounds: 2\n---\nrounds: 3\n"})
    {
        const auto problems = errors(text);
        ASSERT_EQ(problems.size(), 1u) << text;
        EXPECT_EQ(problems.front().code, ProblemCode::parseError) << text;
    }
}

TEST(yamlSetup, duplicateKeys)
{
    const auto top = errors(base + "rounds: 3\n");
    ASSERT_EQ(top.size(), 1u);
    EXPECT_EQ(top.front().code, ProblemCode::parseError);
    EXPECT_EQ(top.front().position, (TextPosition{7, 1}));
    // also inside a list
    EXPECT_EQ(errors("rounds: 2\nfields:\n  - {name: '1', name: '2', type: A, subtype: x}\n").front().code, ProblemCode::parseError);
}

// All problems of the format at once, with path and position
TEST(yamlSetup, problemsWithPathAndPosition)
{
    const auto problems = errors("rounds: '2'\nfields:\n  - type: A\n    name: [1]\nclubs:\n  - name: North\n    teams: [Red, {a: 1}]\n");
    ASSERT_EQ(problems.size(), 4u);
    EXPECT_EQ(problems[0], (Problem{ProblemCode::invalidValue, {"rounds"}, "rounds", Severity::error, TextPosition{1, 1}}));
    EXPECT_EQ(problems[1].code, ProblemCode::invalidValue);
    EXPECT_EQ(problems[1].path, "fields[0].name");
    EXPECT_EQ(line(problems[1]), 4u);
    EXPECT_EQ(problems[2].code, ProblemCode::missingValue);
    EXPECT_EQ(problems[2].parameters, std::vector<std::string> {"subtype"});
    EXPECT_EQ(problems[2].path, "fields[0].subtype");
    EXPECT_EQ(line(problems[2]), 3u);
    EXPECT_EQ(problems[3].path, "clubs[0].teams[1]");
    EXPECT_EQ(line(problems[3]), 7u);
}

TEST(yamlSetup, missingSections)
{
    EXPECT_EQ(errors(fieldsAndClubs).front().code, ProblemCode::nRoundsNotSet);
    EXPECT_EQ(errors("rounds: 1\n").front().code, ProblemCode::noFieldsDefined);
    EXPECT_EQ(errors("rounds: 1\nfields:\n  - {name: '1', type: A, subtype: x}\n").front().code, ProblemCode::noClubsDefined);
}

TEST(yamlSetup, valuesOfTheWrongType)
{
    for (const char* value : {"-1", "4294967296", "1.5", "'3'", "~", "", "[3]", "three"})
        EXPECT_EQ(errors(std::string("iterations: ") + value + "\n" + base).front().path, "iterations") << value;
    for (const char* value : {"yes", "1", "'true'", "~"})
        EXPECT_EQ(errors(std::string("repeat_pairings: ") + value + "\n" + base).front().path, "repeatPairings") << value;
    // a name may be any text, but not null or a list
    EXPECT_EQ(errors("rounds: 2\nfields:\n  - {name: ~, type: A, subtype: x}\n" + fieldsAndClubs.substr(fieldsAndClubs.find("clubs"))).front().path, "fields[0].name");
}

TEST(yamlSetup, denyListTeams)
{
    const std::string deny = base + "conditions:\n  subtype_deny:\n    - subtype: x\n      teams:\n";
    const auto r = yaml::readSetup(deny + "        - {club: North, team: Red}\n        - {club: North, team: Blue}\n");
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->conditions.subtypeDeny, (std::vector<SubtypeDeny> {{"x", {{"North", "Red"}, {"North", "Blue"}}}}));
    const auto problems = errors(deny + "        - {club: North}\n");
    EXPECT_EQ(problems.front(), (Problem{ProblemCode::missingValue, {"team"}, "conditions.subtypeDeny[0].teams[0].team", Severity::error, TextPosition{11, 11}}));
}

// The reader checks the format, not whether the setup can be planned
TEST(yamlSetup, noValidation)
{
    const auto r = yaml::readSetup("rounds: 0\nfields: []\nclubs: []\nconditions:\n  field_types:\n    - {type: Unknown, min: 3, max: 1}\n");
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(r.problems().empty());
}

// Only the canonical keys: other spellings (the TOML files accepted German
// and capitalized keys) are unknown
TEST(yamlSetup, unknownKeysAreWarnings)
{
    const auto r = yaml::readSetup("rouns: 3\nRunden: 3\n" + base + "conditions:\n  max_pauses: 1\n  pause_games: {max: 1}\n");
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.problems().size(), 3u);
    EXPECT_EQ(r.problems()[0], (Problem{ProblemCode::unknownKey, {"rouns"}, "rouns", Severity::warning, TextPosition{1, 1}}));
    EXPECT_EQ(r.problems()[1].parameters, std::vector<std::string> {"Runden"});
    EXPECT_EQ(r.problems()[2].path, "conditions.pause_games");
    EXPECT_EQ(r->conditions.maxPauses, 1u);
    EXPECT_EQ(r->rounds, 2u);
}

TEST(yamlSetup, newerFormatVersion)
{
    const auto r = yaml::readSetup("format_version: 2\n" + base);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.problems().size(), 1u);
    EXPECT_EQ(r.problems().front(), (Problem{ProblemCode::unsupportedFormatVersion, {"2", "1"}, "format_version", Severity::warning, TextPosition{1, 1}}));
    EXPECT_TRUE(yaml::readSetup("format_version: 1\n" + base).problems().empty());
}

TEST(yamlSetup, writtenText)
{
    TournamentSetup s;
    s.rounds = 2;
    s.fields = {{"1", "A", "x"}};
    s.clubs = {{"North", {"Red", "null", "true", "2nd"}}};
    const std::string text = yaml::writeSetup(s);
    EXPECT_EQ(text.rfind("format_version: 1\nrounds: 2\n", 0), 0u) << text;
    EXPECT_NE(text.find("  - {name: '1', type: A, subtype: x}\n"), std::string::npos) << text;
    // texts that other readers could take for null, a boolean or a number are quoted
    EXPECT_NE(text.find("teams: [Red, 'null', 'true', '2nd']"), std::string::npos) << text;
    EXPECT_EQ(text.find("conditions"), std::string::npos) << text;
}

// Several texts read at once (no global state in the adapter)
TEST(yamlSetup, parallelReading)
{
    const std::string text = yaml::writeSetup(*yaml::readSetup(base));
    std::vector<std::thread> threads;
    threads.reserve(4);
    for (int t = 0; t < 4; t++)
        threads.emplace_back([&text]()
    {
        for (int i = 0; i < 200; i++)
        {
            EXPECT_TRUE(yaml::readSetup(text).has_value());
            EXPECT_FALSE(yaml::readSetup("rounds: [\n").has_value());
        }
    });
    for (std::thread& t : threads)
        t.join();
}

TEST(yamlLayout, tolerant)
{
    // missing positions keep the defaults, unknown keys are warnings
    const auto r = yaml::readLayout("format:\n  settings:\n    show_filter_column: true\n    show_everything: true\n  tournament:\n    columns:\n      column: [{type: field, text: Field}]\n");
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(r->format.settings.showFilterColumn);
    EXPECT_EQ(r->format.tournament.columns.col.at("field").text, "Field");
    EXPECT_EQ(r->format.tournament.columns.col.at("field").xRel, 0);
    ASSERT_EQ(r.problems().size(), 1u);
    EXPECT_EQ(r.problems().front().path, "format.settings.show_everything");
    EXPECT_EQ(r.problems().front().severity, Severity::warning);
}

TEST(yamlLayout, datesAndTimes)
{
    const auto r = yaml::readLayout("information:\n  date_time:\n    day: 2021-06-01\n    start: '14:30'\n    round: 00:07:05\n    both: 2021-06-01T14:30:00\n    spaced: 2021-06-01 14:30:00\n");
    ASSERT_TRUE(r.has_value());
    const auto& v = r->info.dateTime.values;
    EXPECT_EQ(v.at("day"), (layout::DateTimeValue{2021, 6, 1, 0, 0, 0}));
    EXPECT_EQ(v.at("start"), (layout::DateTimeValue{0, 0, 0, 14, 30, 0}));
    EXPECT_EQ(v.at("round"), (layout::DateTimeValue{0, 0, 0, 0, 7, 5}));
    EXPECT_EQ(v.at("both"), (layout::DateTimeValue{2021, 6, 1, 14, 30, 0}));
    EXPECT_EQ(v.at("spaced"), v.at("both"));
}

TEST(yamlLayout, errors)
{
    for (const char* text :
            {"format:\n  information:\n    text_fields: [1]\n", "format:\n  settings:\n    fixed_fields: -1\n", "information:\n  date_time:\n    start: today\n",
             "information:\n  date_time:\n    start: '25:00'\n", "information:\n  date_time:\n    start: 2021-13-01\n",
             "format:\n  statistic:\n    rows:\n      row: [{text: no type}]\n", "layout_version: 1\nformat: {\n"
            })
    {
        const auto r = yaml::readLayout(text);
        EXPECT_FALSE(r.has_value()) << text;
        EXPECT_FALSE(r.problems().empty()) << text;
    }
}

TEST(yamlLayout, newerFormatVersion)
{
    const auto r = yaml::readLayout("layout_version: 3\n");
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.problems().size(), 1u);
    EXPECT_EQ(r.problems().front().code, ProblemCode::unsupportedFormatVersion);
}

// The samples read without a problem: every key is known, a setup file
// has no layout problems and the other way round
TEST(yamlFiles, samples)
{
    for (const char* language : {"english", "german"})
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(SM_SAMPLES_DIR) / language / "basic"))
        {
            if (entry.path().extension() != ".yaml")
                continue;
            const std::string text = fileText(entry.path());
            const auto layout = yaml::readLayout(text);
            ASSERT_TRUE(layout.has_value()) << entry.path();
            for (const Problem& p : layout.problems())
                ADD_FAILURE() << entry.path() << " " << p.path;
            if (entry.path().filename() == "format.yaml")
            {
                // written and read again: the same layout
                EXPECT_EQ(*yaml::readLayout(yaml::writeLayout(*layout)), *layout) << entry.path();
                continue;
            }
            const auto setup = yaml::readSetup(text);
            ASSERT_TRUE(setup.has_value()) << entry.path();
            for (const Problem& p : setup.problems())
                ADD_FAILURE() << entry.path() << " " << p.path;
            // written and read again: the same setup
            EXPECT_EQ(*yaml::readSetup(yaml::writeSetup(*setup)), *setup) << entry.path();
        }
}
