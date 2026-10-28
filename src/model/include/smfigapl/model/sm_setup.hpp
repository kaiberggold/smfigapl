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

// From the public setup to the internal model: the validation of a
// TournamentSetup and the Input (ids, mappings) the search works on.

#pragma once
#include <vector>

#include "smfigapl/result.hpp"
#include "smfigapl/setup.hpp"
#include "smfigapl/model/sm_in.hpp"

namespace smfigapl
{
    // All problems of a setup, with their paths (e.g. "fields[2]",
    // "conditions.fieldTypes[0].type"); empty if the setup can be planned
    [[nodiscard]] std::vector<Problem> validateSetup(const TournamentSetup& setup);

    // Builds the input of the search from a setup into an empty Input.
    // The setup must be valid (validateSetup); throws Exception otherwise.
    void buildInput(const TournamentSetup& setup, Input& in);
}
