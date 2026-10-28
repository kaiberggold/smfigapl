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

// The CSV writer: a table of cells, placed as the layout says. The layout is
// a copy: entries the writer asks for and the format file does not have
// are added with their defaults (as the output always did).

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "smfigapl/report/csv.hpp"
#include "report_util.hpp"

namespace
{
    using smfigapl::layout::Pos;
    using std::uint32_t;

    // Date / time as text, format as for std::strftime
    std::string formatDateTime(const std::string& format, const std::tm& tim)
    {
        std::array<char, 50> text{};
        std::strftime(text.data(), text.size(), format.c_str(), &tim);
        return text.data();
    }

    // Time of day from seconds since midnight
    std::tm timeOfDay(uint32_t seconds)
    {
        std::tm tim{};
        tim.tm_hour = static_cast<int>(seconds / 3600);
        tim.tm_min = static_cast<int>(seconds % 3600 / 60);
        tim.tm_sec = static_cast<int>(seconds % 60);
        return tim;
    }

    std::string toLower(std::string s)
    {
        std::ranges::transform(s, s.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }

    // text with every "$0" replaced by value
    std::string replacePlaceholder(std::string text, const std::string& value)
    {
        for (std::size_t p = text.find("$0"); p != std::string::npos; p = text.find("$0", p + value.size()))
            text.replace(p, 2, value);
        return text;
    }

    class CsvWriter
    {
    private:
        const smfigapl::TournamentSetup& setup_;
        const smfigapl::SolveResult& result_;
        const smfigapl::Statistic* statistic_;
        smfigapl::layout::OutputLayout layout_;
        const smfigapl::report::Labels& labels_;
        std::vector<smfigapl::TeamRef> teams_;
        std::vector<std::size_t> subOfField_;
        std::vector<std::vector<std::string>> table_;
        uint32_t sizeX_ = 0;
        uint32_t sizeY_ = 0;

        struct RoundTimes
        {
            std::vector<std::string> start;
            std::vector<std::string> end;
        };

        [[nodiscard]] uint32_t nFields() const
        {
            return static_cast<uint32_t>(setup_.fields.size());
        }
        [[nodiscard]] uint32_t nRounds() const
        {
            return static_cast<uint32_t>(result_.plan.rounds.size());
        }
        [[nodiscard]] uint32_t nTeams() const
        {
            return static_cast<uint32_t>(teams_.size());
        }

        void setSize(uint32_t x, uint32_t y)
        {
            sizeX_ = x;
            sizeY_ = y;
            table_.resize(y);
            for (auto&& r : table_)
                r.resize(x);
        }

        // a cell; a second text in a cell is added after a tab (unless overWrite)
        void set(uint32_t x, uint32_t y, const std::string& s, bool overWrite = false)
        {
            if (sizeY_ < y + 1)
            {
                table_.resize(y + 1);
                for (uint32_t i = sizeY_; i < y + 1; i++)
                    table_[i].resize(sizeX_);
                sizeY_ = y + 1;
            }
            if (sizeX_ < x + 1)
            {
                for (uint32_t i = 0; i < sizeY_; i++)
                    table_[i].resize(x + 1);
                sizeX_ = x + 1;
            }
            std::string& cell = table_[y][x];
            if (overWrite || cell.empty())
                cell = s;
            else
                cell += "\t" + s;
        }

        void markFilterRow(uint32_t row)
        {
            if (layout_.format.settings.showFilterColumn)
                set(layout_.format.infoFields.basePos.x + layout_.format.settings.filterColumn, row, "1", true);
        }

        [[nodiscard]] std::string resultMessage() const
        {
            switch (result_.status)
            {
                case smfigapl::SearchStatus::solved:
                    return labels_.solved;
                case smfigapl::SearchStatus::impossible:
                    return labels_.impossible;
                case smfigapl::SearchStatus::aborted:
                    return labels_.aborted;
                case smfigapl::SearchStatus::cancelled:
                    return labels_.cancelled;
            }
            return {};
        }

        RoundTimes writeInfoFields(const Pos& pos)
        {
            auto& infoFields = layout_.format.infoFields;
            for (auto& [k, v] : infoFields.general)
            {
                set(pos.x + v.xRel, pos.y + v.yRel, layout_.info.general.values[k]);
                markFilterRow(pos.y + v.yRel);
            }
            uint32_t startTime = 0;
            uint32_t roundTime = 0;
            uint32_t breakTime = 0;
            for (auto& [k, v] : infoFields.dateTime)
            {
                const smfigapl::layout::DateTimeValue dt = layout_.info.dateTime.values[k];
                const uint32_t seconds = dt.hour * 3600 + dt.minute * 60 + dt.second;
                std::tm tim = timeOfDay(seconds);
                tim.tm_year = static_cast<int>(dt.year) - 1900;
                tim.tm_mon = static_cast<int>(dt.month) - 1;
                tim.tm_mday = static_cast<int>(dt.day);
                if (k == "start_time")
                    startTime = seconds;
                if (k == "round_time")
                    roundTime = seconds;
                if (k == "break_time")
                    breakTime = seconds;
                set(pos.x + v.xRel, pos.y + v.yRel, formatDateTime(v.formatString, tim));
                markFilterRow(pos.y + v.yRel);
                if (infoFields.dateTimeHeader.contains(k))
                {
                    const auto& header = infoFields.dateTimeHeader[k];
                    set(pos.x + v.xRel + header.xRel, pos.y + v.yRel + header.yRel, header.text);
                    markFilterRow(pos.y + v.yRel + header.yRel);
                }
            }
            RoundTimes times;
            const std::string ft = infoFields.dateTime["round_time"].formatString;
            for (uint32_t i = 0; i < setup_.rounds; i++)
            {
                const uint32_t absTim = startTime + i * roundTime + (i > 0 ? i* breakTime : 0);
                times.start.push_back(formatDateTime(ft, timeOfDay(absTim)));
                times.end.push_back(formatDateTime(ft, timeOfDay(absTim + roundTime)));
            }
            return times;
        }

        [[nodiscard]] std::string textColor(const smfigapl::TeamRef& team) const
        {
            const auto& colors = layout_.format.teams.textColors.colors;
            const auto index = static_cast<std::size_t>(std::distance(teams_.begin(), std::ranges::find(teams_, team)));
            return index < colors.size() ? colors[index] : "";
        }

        void writeTeams(const Pos& pos)
        {
            const auto& teams = layout_.format.teams;
            const uint32_t xFac = layout_.format.settings.teamsUseColumn ? 0 : 1;
            const uint32_t yFac = 1 - xFac;
            const uint32_t colBase = pos.x + teams.basePos.x;
            const uint32_t rowBase = pos.y + teams.basePos.y;
            set(colBase + teams.title.xRel, rowBase + teams.title.yRel, teams.title.text);
            for (uint32_t i = 0; i < nTeams(); i++)
            {
                set(colBase + i * xFac, rowBase + i * yFac, smfigapl::report::detail::teamName(teams_[i]));
                markFilterRow(rowBase + i * yFac);
            }
            if (!layout_.format.settings.showTeamsTextColors)
                return;
            set(colBase + teams.textColors.title.xRel, rowBase + teams.textColors.title.yRel, teams.textColors.title.text);
            markFilterRow(rowBase);
            for (uint32_t i = 0; i < nTeams(); i++)
            {
                set(colBase + teams.textColors.posRel.x + i * xFac, rowBase + teams.textColors.posRel.y + i * yFac, textColor(teams_[i]));
                markFilterRow(rowBase + teams.textColors.posRel.y + i * yFac);
            }
        }

        void writeGame(uint32_t colBase, uint32_t row, const smfigapl::Game& game)
        {
            using smfigapl::report::detail::teamName;
            auto& col = layout_.format.tournament.columns.col;
            set(colBase + col["home_team"].xRel, row, teamName(game.home));
            if (layout_.format.settings.showTeamsTextColors)
            {
                if (layout_.format.settings.useTeamColor)
                {
                    set(colBase + col["home_team_text_color"].xRel, row, toLower(game.home ? game.home->team : ""));
                    set(colBase + col["away_team_text_color"].xRel, row, toLower(game.away ? game.away->team : ""));
                }
                else
                {
                    set(colBase + col["home_team_text_color"].xRel, row, game.home ? textColor(*game.home) : "");
                    set(colBase + col["away_team_text_color"].xRel, row, game.away ? textColor(*game.away) : "");
                }
            }
            set(colBase + col["away_team"].xRel, row, teamName(game.away));
        }

        void writePauseTeams(uint32_t colBase, uint32_t rowBase, const smfigapl::Round& round, uint32_t tournament)
        {
            auto& col = layout_.format.tournament.columns.col;
            // the format may show more tournaments than there are (fixed_tournaments)
            if (tournament >= result_.plan.subTournaments.size())
                return;
            const auto& teams = result_.plan.subTournaments[tournament].teams;
            uint32_t k = 0;
            for (const smfigapl::TeamRef& team : round.pauses)
            {
                if (std::ranges::find(teams, team) == teams.end())
                    continue;
                set(colBase + col["pause_teams"].xRel, rowBase + k, smfigapl::report::detail::teamName(team));
                markFilterRow(rowBase + k);
                // without useTeamColor the pause teams get no text color
                if (layout_.format.settings.showTeamsTextColors && layout_.format.settings.useTeamColor)
                    set(colBase + col["pause_teams_text_color"].xRel, rowBase + k, toLower(team.team));
                k++;
            }
        }

        void writeTournament(const Pos& pos, uint32_t tournament, const RoundTimes& times)
        {
            const auto& settings = layout_.format.settings;
            const auto& format = layout_.format.tournament;
            auto& col = layout_.format.tournament.columns.col;
            const auto& gamePlan = layout_.format.gamePlan;
            const uint32_t roundFactor = std::max(settings.fixedFields, nFields());
            const uint32_t tournamentFactor = std::max(settings.fixedRounds, setup_.rounds) * roundFactor + format.relPos.yRel + format.columns.relPos.yRel;
            const uint32_t tournamentOffset = tournament * tournamentFactor;
            const uint32_t colBase = pos.x + gamePlan.basePos.x + format.relPos.xRel + format.columns.relPos.xRel;
            const uint32_t rowBase = pos.y + gamePlan.basePos.y + format.relPos.yRel + format.columns.relPos.yRel + tournamentOffset;
            bool active = false;
            for (uint32_t i = 0; i < nRounds(); i++)
            {
                const smfigapl::Round& round = result_.plan.rounds[i];
                bool timeWritten = false;
                for (uint32_t j = 0; j < nFields() && j < round.games.size(); j++)
                {
                    if (subOfField_[j] != tournament)
                        continue;
                    const uint32_t row = rowBase + i * roundFactor + j;
                    markFilterRow(row);
                    if (round.games[j].home)
                    {
                        active = true;
                        writeGame(colBase, row, round.games[j]);
                    }
                    set(colBase + col["field_name"].xRel, row, setup_.fields[j].name);
                    set(colBase + col["field_type"].xRel, row, setup_.fields[j].type);
                    if (!timeWritten && active)
                    {
                        timeWritten = true;
                        set(colBase + col["round"].xRel, row, std::to_string(i + 1));
                        set(colBase + col["start_time"].xRel, row, times.start[i]);
                        set(colBase + col["end_time"].xRel, row, times.end[i]);
                    }
                }
                if (!round.pauses.empty())
                    writePauseTeams(colBase, rowBase + i * roundFactor, round, tournament);
            }
            if (!active)
                return;
            const std::string title = replacePlaceholder(format.title.text, std::to_string(tournament + 1));
            const uint32_t titleRow = pos.y + gamePlan.basePos.y + format.relPos.yRel + tournamentOffset + format.title.yRel;
            set(pos.x + gamePlan.basePos.x + format.relPos.xRel + format.title.xRel, titleRow, title);
            markFilterRow(titleRow);
            for (auto& [k, v] : col)
            {
                set(colBase + v.xRel, rowBase + v.yRel, v.text);
                markFilterRow(rowBase + v.yRel);
            }
        }

        void writeStatistic(const Pos& pos)
        {
            auto& format = layout_.format.statistic;
            const uint32_t xFac = layout_.format.settings.statisticUseColumn ? 0 : 1;
            const uint32_t yFac = 1 - xFac;
            const uint32_t colBase = pos.x + format.basePos.x + format.rows.relPos.xRel;
            const uint32_t rowBase = pos.y + format.basePos.y + format.rows.relPos.yRel;
            set(pos.x + format.basePos.x + format.title.xRel, pos.y + format.basePos.y + format.title.yRel, format.title.text);
            markFilterRow(pos.y + format.basePos.y + format.title.yRel);
            for (uint32_t i = 0; i < nTeams(); i++)
            {
                set(colBase + i * xFac - yFac, rowBase - xFac + i * yFac, smfigapl::report::detail::teamName(teams_[i]));
                markFilterRow(rowBase - xFac + yFac);
            }
            uint32_t fieldTypeOffset = 0;
            const auto rows = smfigapl::report::detail::statisticRows(*statistic_, labels_);
            for (uint32_t i = 0; i < rows.size(); i++)
            {
                // all field type rows share one format entry, shifted by their number
                const bool fieldType = rows[i].key == "field_types";
                const auto& row = format.rows.row[rows[i].key];
                const std::string rowHeader = replacePlaceholder(row.text, rows[i].label);
                set(colBase + row.xRel + fieldTypeOffset * yFac, rowBase + row.yRel + fieldTypeOffset * xFac, rowHeader);
                markFilterRow(rowBase + row.yRel);
                for (uint32_t j = 0; j < rows[i].values.size(); j++)
                {
                    set(colBase + i * yFac + j * xFac, rowBase + i * xFac + j * yFac, std::to_string(rows[i].values[j]));
                    markFilterRow(rowBase + i * xFac + j * yFac);
                }
                if (fieldType)
                    fieldTypeOffset++;
            }
        }

    public:
        CsvWriter(const smfigapl::TournamentSetup& setup, const smfigapl::SolveResult& result, const smfigapl::Statistic* statistic,
                  smfigapl::layout::OutputLayout layout, const smfigapl::report::Labels& labels):
            setup_{setup}, result_{result}, statistic_{statistic}, layout_{std::move(layout)}, labels_{labels},
            subOfField_{smfigapl::report::detail::subTournamentOfFields(result.plan)}
        {
            for (const smfigapl::Club& c : setup.clubs)
                for (const std::string& team : c.teams)
                    teams_.push_back({c.name, team});
        }

        void write(std::ostream& os)
        {
            setSize(134, 18);
            const Pos pos = layout_.format.infoFields.basePos;
            const auto& settings = layout_.format.settings;
            set(pos.x + layout_.format.infoFields.resultMessage.xRel, pos.y + layout_.format.infoFields.resultMessage.yRel, resultMessage());
            if (result_.status == smfigapl::SearchStatus::solved)
            {
                const RoundTimes times = writeInfoFields(pos);
                if (settings.showTeams)
                    writeTeams(pos);
                uint32_t nTournaments = settings.fixedTournaments;
                if (settings.separateTournaments)
                    nTournaments = std::max(nTournaments, static_cast<uint32_t>(result_.plan.subTournaments.size()));
                for (uint32_t l = 0; l < nTournaments; l++)
                    writeTournament(pos, l, times);
                if (settings.showStatistic && statistic_ != nullptr)
                    writeStatistic(pos);
                for (auto&& r : settings.fixedRows)
                    markFilterRow(r);
            }
            for (auto&& row : table_)
            {
                for (const auto& cell : row)
                    os << cell << ',';
                os << '\n';
            }
        }
    };
}

void smfigapl::report::writeCsv(std::ostream& os, const TournamentSetup& setup, const SolveResult& result, const Statistic* statistic,
                                const layout::OutputLayout& layout, const Labels& labels)
{
    CsvWriter(setup, result, statistic, layout, labels).write(os);
}
