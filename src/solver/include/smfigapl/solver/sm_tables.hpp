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

#include <span>
#include <vector>

#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_indexed.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/solver/sm_search_types.hpp"
#include "smfigapl/solver/sm_ts.hpp"

namespace smfigapl
{

// The candidate rounds of the current atomic set (tables.atomicIndex,
// tables.candidates: pairings, field mixes, the valid candidates, the
// repeated games jumps), the bounds and searchIndexMax. The search chooses
// one candidate per round; built once before the search.
    class RoundTables
    {
    private:
        const Input& input_;
        TreeSearch& ts_;
        CondTables& tables_;
        uint NCandidates_ = 0;

        void initCandidates();
        void initRepeatedGames();
        void searchIndexMaxInit();
        void searchIndexMaxUpdate(uint i);
        // the bounds (CondTables::bounds) without and with candidate i
        void boundsInit();
        void boundsAdd(uint i);
        // the bounds of all candidates, valid without a look at every one
        void boundsByPairing();
        // true if candidate i (among all) breaks the empty fields or subtype conditions
        [[nodiscard]] bool isImpossible(uint i) const;
        [[nodiscard]] IndexedTeam indexedTeam(uint id) const;

    public:
        // ts has to be initialized for the atomic set
        RoundTables(const Input& in, TreeSearch& ts, CondTables& tables): input_{in}, ts_{ts}, tables_{tables}
        {
        }
        void build();
        // candidate i as a round of games (with clubs, colors and pause teams)
        [[nodiscard]] IndexedRound candidate(uint i) const;
        [[nodiscard]] uint size() const
        {
            return NCandidates_;
        }
    };

}//namespace smfigapl
