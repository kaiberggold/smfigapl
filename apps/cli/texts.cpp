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

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "texts.hpp"


const std::string* smfigapl::cli::Lang::find(const std::string& l, const std::string& in) const
{
    auto language = translations_.find(l);
    if (language == translations_.end())
        return nullptr;
    auto text = language->second.find(in);
    return text == language->second.end() ? nullptr : &text->second;
}

std::string smfigapl::cli::Lang::get(const std::string& in) const
{
    return get(in, language_);
}

std::string smfigapl::cli::Lang::get(const std::string& in, const std::string& l) const
{
    const std::string* text = find(l, in);
    return text != nullptr ? *text : in;
}

void smfigapl::cli::Lang::setLanguage(std::string l)
{
    language_ = std::move(l);
}

std::string smfigapl::cli::Lang::getErrorMessage(smfigapl::ProblemCode errorId) const
{
    auto language = errorTranslations_.find(language_);
    if (language == errorTranslations_.end())
        return "";
    auto text = language->second.find(errorId);
    return text == language->second.end() ? "" : text->second;
}

namespace
{
    using smfigapl::ProblemCode;

    // The values of the message of a problem: keys in the language of lang
    std::vector<std::string> messageValues(const smfigapl::Problem& p, const smfigapl::cli::Lang& lang)
    {
        switch (p.code)
        {
            // the description and the position are the detail
            case ProblemCode::parseError:
                return {};
            case ProblemCode::nRoundsNotSet:
            case ProblemCode::noFieldsDefined:
            case ProblemCode::noClubsDefined:
            case ProblemCode::missingValue:
                return {p.parameters.empty() ? "" : lang.get(p.parameters.front())};
            // the invalid value is the detail
            case ProblemCode::invalidValue:
                return {p.parameters.empty() ? "" : p.parameters.front()};
            default:
                return p.parameters;
        }
    }

    std::string detail(const smfigapl::Problem& p, const std::string& source)
    {
        if (p.code == ProblemCode::invalidValue)
            return p.parameters.size() > 1 ? p.parameters[1] : "";
        if (p.code != ProblemCode::parseError)
            return "";
        std::string d = p.parameters.empty() ? "" : p.parameters.front();
        if (p.position)
        {
            d += "\n\t(error occurred at line " + std::to_string(p.position->line) + ", column " + std::to_string(p.position->column);
            if (!source.empty())
                d += " of '" + source + "'";
            d += ")";
        }
        return d;
    }
}

std::string smfigapl::cli::problemText(const Problem& problem, const Lang& lang)
{
    std::string text = lang.getErrorMessage(problem.code);
    const std::vector<std::string> values = messageValues(problem, lang);
    for (std::size_t i = 0; i < values.size(); i++)
    {
        const std::string placeholder = "$" + std::to_string(i);
        for (std::size_t p = text.find(placeholder); p != std::string::npos; p = text.find(placeholder, p + values[i].size()))
            text.replace(p, placeholder.size(), values[i]);
    }
    return text;
}

std::string smfigapl::cli::problemMessage(const Problem& problem, const Lang& lang, const std::string& source)
{
    const std::string prefix = problem.severity == Severity::warning ? "warningId" : "errorId";
    std::string m = lang.get(prefix) + std::to_string(static_cast<std::uint32_t>(problem.code)) + ": " + problemText(problem, lang);
    if (const std::string d = detail(problem, source); !d.empty())
        m += "\n" + d;
    return m;
}

smfigapl::report::Labels smfigapl::cli::labels(const Lang& lang)
{
    report::Labels l;
    l.tournament = lang.get("tournament");
    l.round = lang.get("round");
    l.pause = lang.get("break");
    l.solved = lang.get("Solved");
    l.impossible = lang.get("Impossible");
    l.aborted = lang.get("Aborted");
    l.cancelled = lang.get("Aborted");
    l.games = lang.get("n_games");
    l.repeatedGames = lang.get("n_same_pairing");
    l.maxGamesAgainstOneTeam = lang.get("max_games_same_team");
    l.sameClubGames = lang.get("same_club");
    l.homeGames = lang.get("home_games");
    l.awayGames = lang.get("away_games");
    return l;
}
