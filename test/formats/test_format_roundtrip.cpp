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


// Round trips of the format adapters: write, read, the same value again.
// The suite is independent of the format: every adapter (YAML now) is a
// type of the typed test.

#include <string>
#include <string_view>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/formats/yaml.hpp"

using namespace smfigapl;

namespace
{
    // An adapter as the round trip tests use it
    struct YamlFormat
    {
        static Result<TournamentSetup> readSetup(std::string_view text)
        {
            return formats::yaml::readSetup(text);
        }
        static std::string writeSetup(const TournamentSetup& setup)
        {
            return formats::yaml::writeSetup(setup);
        }
        static Result<layout::OutputLayout> readLayout(std::string_view text)
        {
            return formats::yaml::readLayout(text);
        }
        static std::string writeLayout(const layout::OutputLayout& layout)
        {
            return formats::yaml::writeLayout(layout);
        }
    };

    TournamentSetup minimalSetup()
    {
        TournamentSetup s;
        s.rounds = 1;
        s.fields = {{"1", "A", "x"}};
        s.clubs = {{"North", {"Red", "Blue"}}};
        return s;
    }

    // Every setting different from its default, every condition
    TournamentSetup fullSetup()
    {
        TournamentSetup s;
        s.rounds = 7;
        s.repeatPairings = true;
        s.iterations = 2;
        s.homeAwayBalance = false;
        s.optimizeRoundOrder = false;
        s.fields = {{"1", "Small", "Left"}, {"2", "Large", "Left"}, {"3", "Small", "Right"}};
        s.clubs = {{"North", {"Red", "Green"}}, {"South", {"Blue"}}, {"West", {"Red"}}};
        s.conditions.maxPauses = 2;
        s.conditions.maxEmptyFields = 1;
        s.conditions.maxSameClubGames = 0;
        s.conditions.fieldTypes = {{"Small", 1, 3}, {"Large", std::nullopt, 2}, {"Small", 0, std::nullopt}};
        s.conditions.subtypeDeny = {{"Left", {{"North", "Red"}, {"West", "Red"}}}, {"Right", {}}};
        return s;
    }

    // Names that need quoting or are not ASCII
    TournamentSetup specialNames()
    {
        TournamentSetup s = minimalSetup();
        s.fields = {{"Platz \"1\"", "Groß", "Süd\\West"}, {"tab\tname", "", "a'b"}};
        s.clubs = {{"Übersee", {"Grün", "rouge/noir", "#1"}}, {"", {""}}};
        return s;
    }

    layout::OutputLayout fullLayout()
    {
        layout::OutputLayout l;
        l.info.general.values = {{"title", "Turnier"}, {"place", "Feld \"Nord\""}};
        l.info.dateTime.values["start_date"] = {2021, 6, 1, 0, 0, 0};
        l.info.dateTime.values["start_time"] = {0, 0, 0, 14, 0, 0};
        l.info.dateTime.values["start"] = {2021, 6, 1, 14, 30, 5};
        auto& s = l.format.settings;
        s.separateTournaments = true;
        s.showTeams = true;
        s.showTeamsTextColors = true;
        s.teamsUseColumn = true;
        s.statisticUseColumn = true;
        s.showStatistic = true;
        s.showFilterColumn = true;
        s.useTeamColor = true;
        s.fixedFields = 8;
        s.fixedRounds = 7;
        s.fixedTournaments = 2;
        s.filterColumn = 3;
        s.fixedRows = {1, 2, 5};
        auto& info = l.format.infoFields;
        info.basePos = {1, 2};
        info.resultMessage.xRel = -1;
        info.resultMessage.yRel = 4;
        info.general["title"].yRel = 1;
        info.dateTime["start_date"].formatString = "%d.%m.%Y";
        info.dateTime["start_date"].xRel = 2;
        info.dateTimeHeader["start_date"].text = "Date";
        l.format.teams.basePos = {3, 4};
        l.format.teams.title.text = "Teams";
        l.format.teams.title.yRel = -1;
        l.format.teams.textColors.colors = {"#ff0000", "#00ff00"};
        l.format.teams.textColors.posRel = {1, 0};
        l.format.teams.textColors.title.text = "Color";
        l.format.gamePlan.basePos = {5, 6};
        l.format.gamePlan.title.text = "Plan";
        l.format.tournament.relPos.xRel = 2;
        l.format.tournament.title.text = "Tournament";
        l.format.tournament.columns.relPos.yRel = 1;
        l.format.tournament.columns.col["field"].text = "Field";
        l.format.tournament.columns.col["round"].xRel = -2;
        l.format.statistic.basePos = {7, 8};
        l.format.statistic.title.text = "Statistic";
        l.format.statistic.rows.relPos.xRel = 1;
        l.format.statistic.rows.row["n_games"].text = "Games";
        return l;
    }
}

template<class Format> class formatRoundTrip : public ::testing::Test
{
};

using Formats = ::testing::Types<YamlFormat>;
TYPED_TEST_SUITE(formatRoundTrip, Formats);

TYPED_TEST(formatRoundTrip, setup)
{
    for (const TournamentSetup& setup : {TournamentSetup{}, minimalSetup(), fullSetup(), specialNames()})
    {
        const std::string text = TypeParam::writeSetup(setup);
        const auto read = TypeParam::readSetup(text);
        ASSERT_TRUE(read.has_value()) << text;
        EXPECT_TRUE(read.problems().empty()) << text;
        EXPECT_EQ(*read, setup) << text;
        // the text is stable
        EXPECT_EQ(TypeParam::writeSetup(*read), text);
    }
}

TYPED_TEST(formatRoundTrip, layout)
{
    for (const layout::OutputLayout& layout : {layout::OutputLayout{}, fullLayout()})
    {
        const std::string text = TypeParam::writeLayout(layout);
        const auto read = TypeParam::readLayout(text);
        ASSERT_TRUE(read.has_value()) << text;
        EXPECT_TRUE(read.problems().empty()) << text;
        EXPECT_EQ(*read, layout) << text;
        EXPECT_EQ(TypeParam::writeLayout(*read), text);
    }
}

// The setup and the layout are independent: each reader ignores the text of the other
TYPED_TEST(formatRoundTrip, setupAndLayoutApart)
{
    const auto setup = TypeParam::readSetup(TypeParam::writeLayout(fullLayout()));
    EXPECT_FALSE(setup.has_value());
    const auto layout = TypeParam::readLayout(TypeParam::writeSetup(fullSetup()));
    ASSERT_TRUE(layout.has_value());
    EXPECT_TRUE(layout.problems().empty());
    EXPECT_EQ(*layout, layout::OutputLayout{});
}

// A text that is not of the format is an error, never a crash
TYPED_TEST(formatRoundTrip, brokenText)
{
    const std::string text = TypeParam::writeSetup(fullSetup());
    for (std::size_t n = 0; n < text.size(); n += 7)
    {
        const std::string cut = text.substr(0, n);
        const auto read = TypeParam::readSetup(cut);
        if (!read.has_value())
        {
            EXPECT_FALSE(read.problems().empty()) << cut;
        }
        (void)TypeParam::readLayout(cut);
    }
}
