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


/// @file report/csv.hpp
/// @brief The game plan for a spreadsheet: comma separated cells placed as
/// the output layout (format file) says.

#pragma once

#include <ostream>

#include "smfigapl/layout.hpp"
#include "smfigapl/plan.hpp"
#include "smfigapl/report/labels.hpp"
#include "smfigapl/setup.hpp"
#include "smfigapl/statistic.hpp"

namespace smfigapl::report
{
    /// Writes the result of a search for setup: the result message, and if
    /// solved the information, the teams, the tournaments and (if the layout
    /// shows it and statistic is given) the statistic.
    /// layout.format.teams.textColors.colors has the text color of every
    /// team in the order of setup's clubs and teams.
    void writeCsv(std::ostream& os, const TournamentSetup& setup, const SolveResult& result, const Statistic* statistic,
                  const layout::OutputLayout& layout, const Labels& labels);
}
