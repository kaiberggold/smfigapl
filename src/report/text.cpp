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
#include <cstddef>
#include <string>
#include <vector>

#include "smfigapl/report/text.hpp"
#include "report_util.hpp"

namespace
{
    constexpr std::size_t spacing = 25;
    constexpr std::size_t spacingLarge = 30;

    // the number of characters of a UTF-8 text (code points)
    std::size_t textWidth(const std::string& text)
    {
        return static_cast<std::size_t>(std::ranges::count_if(text, [](char c)
        {
            return (static_cast<unsigned char>(c) & 0xC0U) != 0x80U;
        }));
    }

    // the text with spaces after it (padRight) or before it (padLeft) to width characters
    std::string padRight(const std::string& text, std::size_t width)
    {
        const std::size_t w = textWidth(text);
        return w >= width ? text : text + std::string(width - w, ' ');
    }

    std::string padLeft(const std::string& text, std::size_t width)
    {
        const std::size_t w = textWidth(text);
        return w >= width ? text : std::string(width - w, ' ') + text;
    }
}

namespace
{
    // A round: the games of the fields in fields, the pausing teams in teams (all if teams is null)
    void writeRound(std::ostream& os, const smfigapl::Round& round, std::size_t number, const smfigapl::report::Labels& labels,
                    const std::vector<bool>& fields, const std::vector<smfigapl::TeamRef>* teams)
    {
        using smfigapl::report::detail::teamName;
        os << labels.round << ": " << number << '\n';
        for (std::size_t j = 0; j < round.games.size(); j++)
            if (fields[j])
                os << padLeft(teamName(round.games[j].home) + " : ", spacingLarge) << padRight(teamName(round.games[j].away), spacingLarge) << '\n';
        std::vector<std::string> pauses;
        for (const smfigapl::TeamRef& team : round.pauses)
            if (teams == nullptr || std::ranges::find(*teams, team) != teams->end())
                pauses.push_back(teamName(team));
        if (!pauses.empty())
        {
            os << padLeft(labels.pause + ": ", spacingLarge);
            for (std::size_t k = 0; k < pauses.size(); k++)
                os << (k > 0 ? ", " : "") << pauses[k];
            os << '\n';
        }
        os << '\n';
    }
}

void smfigapl::report::writeText(std::ostream& os, const GamePlan& plan, const Labels& labels, bool bySubTournament)
{
    const std::vector<std::size_t> subOfField = detail::subTournamentOfFields(plan);
    if (!bySubTournament)
    {
        for (std::size_t i = 0; i < plan.rounds.size(); i++)
            writeRound(os, plan.rounds[i], i + 1, labels, std::vector<bool>(plan.rounds[i].games.size(), true), nullptr);
        return;
    }
    for (std::size_t l = 0; l < plan.subTournaments.size(); l++)
    {
        os << labels.tournament << ": " << l + 1 << '\n';
        std::vector<bool> fields;
        fields.reserve(subOfField.size());
        for (const std::size_t sub : subOfField)
            fields.push_back(sub == l);
        for (std::size_t i = 0; i < plan.rounds.size(); i++)
            writeRound(os, plan.rounds[i], i + 1, labels, fields, &plan.subTournaments[l].teams);
    }
}

void smfigapl::report::writeStatisticText(std::ostream& os, const Statistic& statistic, const Labels& labels)
{
    os << padRight(labels.team, spacingLarge);
    for (const TeamStatistic& t : statistic.teams)
        os << padRight(detail::teamName(t.team), spacing);
    os << '\n';
    for (const detail::StatisticRow& row : detail::statisticRows(statistic, labels))
    {
        os << padRight(row.label, spacingLarge);
        for (std::uint32_t v : row.values)
            os << padRight(std::to_string(v), spacing);
        os << '\n';
    }
}
