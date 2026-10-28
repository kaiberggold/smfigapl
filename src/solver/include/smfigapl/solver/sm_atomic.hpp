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


#pragma once

#include <vector>

#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/solver/sm_search_types.hpp"

namespace smfigapl
{

// The independent sets (sub tournaments) of a configuration: teams and the
// field subtypes they may play on. Every set is searched on its own.
    struct AtomicSets
    {
        uint count = 0;
        std::vector<uint> indexByFieldId;
        std::vector<uint> indexByTeamId;
    };

// Calculates the atomic sets and fills the capacity and mapping arrays of
// the tables (atomicCapacityArray, atomicMappingArray, fieldMappingArray,
// teamsMappingArray, teamIdMappingArray)
    AtomicSets calcAtomicSets(const Input& in, CondTables& tables);

}//namespace smfigapl
