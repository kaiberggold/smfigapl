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


/// @file layout.hpp
/// @brief The output layout (the format file): positions and settings of
/// the spreadsheet/CSV output and the information shown with the plan.
///
/// Independent of the setup: a setup works with any layout, and without a
/// format file the defaults are used.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace smfigapl::layout
{
    /// A date, a time or both (the parts that are not given are 0).
    class DateTimeValue
    {
    public:
        std::uint32_t year = 0;   ///< The year, e.g. 2021.
        std::uint32_t month = 0;  ///< The month, 1 to 12.
        std::uint32_t day = 0;    ///< The day of the month, 1 to 31.
        std::uint32_t hour = 0;   ///< The hour, 0 to 23.
        std::uint32_t minute = 0; ///< The minute, 0 to 59.
        std::uint32_t second = 0; ///< The second, 0 to 59.
        friend bool operator==(const DateTimeValue&, const DateTimeValue&) = default;
    };

    /// A position in the spreadsheet: column and row, starting at 0.
    class Pos
    {
    public:
        std::uint32_t x = 0; ///< The column.
        std::uint32_t y = 0; ///< The row.
        friend bool operator==(const Pos&, const Pos&) = default;
    };

    /// A position relative to the base position of its part.
    class PosRel
    {
    public:
        int xRel = 0; ///< Columns to the right (negative: to the left).
        int yRel = 0; ///< Rows down (negative: up).
        friend bool operator==(const PosRel&, const PosRel&) = default;
    };

    /// A text at a relative position (a title).
    class TextField: public PosRel
    {
    public:
        std::string text; ///< The text; "$0" in a tournament title is its number.
        friend bool operator==(const TextField&, const TextField&) = default;
    };

    /// A column or row header at a relative position.
    class Header: public PosRel
    {
    public:
        std::string text; ///< The text; "$0" in a statistic row is the name of the field type.
        friend bool operator==(const Header&, const Header&) = default;
    };

    /// A date or time of the information at a relative position.
    class DateTimeField: public PosRel
    {
    public:
        std::string formatString; ///< The format, as for std::strftime (e.g. "%d.%m.%Y").
        friend bool operator==(const DateTimeField&, const DateTimeField&) = default;
    };

    /// The information shown with the plan (the table `information`).
    class Information
    {
    public:
        /// Texts such as the title or the place, by key.
        struct General
        {
            std::unordered_map<std::string, std::string> values; ///< The texts by key.
            friend bool operator==(const General&, const General&) = default;
        } general; ///< The texts.
        /// Dates and times by key; the keys `start_time`, `round_time` and
        /// `break_time` give the times of the rounds.
        struct DateTime
        {
            std::unordered_map<std::string, DateTimeValue> values; ///< The dates and times by key.
            friend bool operator==(const DateTime&, const DateTime&) = default;
        } dateTime; ///< The dates and times.
        friend bool operator==(const Information&, const Information&) = default;
    };

    /// The layout of the spreadsheet output (the table `format`).
    class Format
    {
    public:
        /// What is shown, and the fixed sizes of the parts.
        struct Settings
        {
            bool separateTournaments = false; ///< Every sub tournament in its own block.
            bool showTeams = false;           ///< The list of the teams.
            bool showTeamsTextColors = false; ///< The text colors of the teams.
            bool teamsUseColumn = false;      ///< The teams in a column (else in a row).
            bool statisticUseColumn = false;  ///< The statistic of a team in a column (else in a row).
            bool showStatistic = false;       ///< The statistic.
            bool showFilterColumn = false;    ///< A column that marks the used rows (for a spreadsheet filter).
            bool useTeamColor = false;        ///< The team name (its color) as text color, not the color list.
            std::uint32_t fixedFields = 0;      ///< Rows per round at least (for a fixed layout).
            std::uint32_t fixedRounds = 0;      ///< Rounds per tournament block at least.
            std::uint32_t fixedTournaments = 0; ///< Tournament blocks at least.
            std::uint32_t filterColumn = 0;     ///< The filter column, relative to the information.
            std::vector<std::uint32_t> fixedRows; ///< Rows that are always marked in the filter column.
            friend bool operator==(const Settings&, const Settings&) = default;
        } settings; ///< The settings.

        /// Where the information is.
        struct InfoFields
        {
            Pos basePos;          ///< The base position of the whole output.
            PosRel resultMessage; ///< The result of the search.
            std::unordered_map<std::string, PosRel> general; ///< The texts of Information::general, by key.
            std::unordered_map<std::string, DateTimeField> dateTime; ///< The dates and times of Information::dateTime, by key.
            std::unordered_map<std::string, Header> dateTimeHeader;  ///< Headers of the dates and times, by key.
            friend bool operator==(const InfoFields&, const InfoFields&) = default;
        } infoFields; ///< The information.

        /// The list of the teams.
        struct Teams
        {
            Pos basePos;     ///< Its position.
            TextField title; ///< Its title.
            /// The text color of every team.
            struct TextColors
            {
                std::vector<std::string> colors; ///< A color per team (in the order of the clubs and teams).
                Pos posRel;      ///< The position relative to the list.
                TextField title; ///< The title.
                friend bool operator==(const TextColors&, const TextColors&) = default;
            } textColors; ///< The text colors.
            friend bool operator==(const Teams&, const Teams&) = default;
        } teams; ///< The teams.

        /// The game plan.
        struct GamePlan
        {
            Pos basePos;     ///< Its position.
            TextField title; ///< Its title.
            friend bool operator==(const GamePlan&, const GamePlan&) = default;
        } gamePlan; ///< The game plan.

        /// A tournament block of the game plan.
        struct Tournament
        {
            PosRel relPos;   ///< The position relative to the game plan.
            TextField title; ///< The title ("$0": the number of the tournament).
            /// The columns of a game.
            struct Columns
            {
                PosRel relPos; ///< The position relative to the block.
                /// The columns by key: round, start_time, end_time, field_name,
                /// field_type, home_team, away_team, pause_teams and the
                /// text colors home_team_text_color, away_team_text_color,
                /// pause_teams_text_color.
                std::unordered_map<std::string, Header> col;
                friend bool operator==(const Columns&, const Columns&) = default;
            } columns; ///< The columns.
            friend bool operator==(const Tournament&, const Tournament&) = default;
        } tournament; ///< The tournament blocks.

        /// The statistic.
        struct Statistic
        {
            Pos basePos;     ///< Its position.
            TextField title; ///< Its title.
            /// The rows of the numbers.
            struct Rows
            {
                PosRel relPos; ///< The position relative to the statistic.
                /// The rows by key: n_games, n_same_pairing,
                /// max_games_same_team, same_club, home_games, away_games,
                /// field_types (one entry for all field types).
                std::unordered_map<std::string, Header> row;
                friend bool operator==(const Rows&, const Rows&) = default;
            } rows; ///< The rows.
            friend bool operator==(const Statistic&, const Statistic&) = default;
        } statistic; ///< The statistic.
        friend bool operator==(const Format&, const Format&) = default;
    };


    /// Everything the output needs besides the game plan: the layout of the
    /// spreadsheet/CSV output (format) and the information shown with the
    /// plan (title, dates). The defaults describe the output without a
    /// format file.
    class OutputLayout
    {
    public:
        Information info; ///< The information shown with the plan.
        Format format;    ///< The layout of the output.
        friend bool operator==(const OutputLayout&, const OutputLayout&) = default;
    };

}
