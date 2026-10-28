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


/// @file report/text.hpp
/// @brief The game plan and the statistic as plain text (fixed width columns).

#pragma once

#include <ostream>

#include "smfigapl/plan.hpp"
#include "smfigapl/report/labels.hpp"
#include "smfigapl/statistic.hpp"

namespace smfigapl::report
{
    /// The rounds of the plan: the games field by field ("Club Team : Club
    /// Team", "- : -" for an empty field) and the pausing teams. With
    /// bySubTournament every sub tournament has its own rounds.
    void writeText(std::ostream& os, const GamePlan& plan, const Labels& labels, bool bySubTournament = false);

    /// The statistic: a column per team, a row per number.
    void writeStatisticText(std::ostream& os, const Statistic& statistic, const Labels& labels);
}
