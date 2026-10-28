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
// OR OTHER DEALINGS IN THE SOFTWARE.#ifndef SMTREESEARCH
#pragma once

#include <iostream>
#include <span>
#include <vector>
#include <string>
#include <utility>

#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_debug.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_capacity.hpp"
#include "smfigapl/solver/sm_search_types.hpp"


namespace smfigapl
{

    class TreeSearch
    {
    public:

    private:
        struct TsInput
        {
            Capacity atomicCapacity;
            Mapping mapping;
            uint extraPauseFields = 0;
        } input_;
        Capacity capacity_;
        uint NHalfFields_ = 0;
        uint searchIndexMax_ = 0;
        uint NFieldMixArray_ = 0;
        uint NPairingArray_ = 0;
        uint subLevelInit_ = 0;
        bool checkAll_ = false;
        bool pauseFields_ = false;
        DebugLog dbg_;

        std::vector<uint> indexedFieldTypeArray_;
        std::vector<uint> subLevel_;
        std::vector<uint> NSubLevel_;
        std::vector<uint> NRemaining_;
        // the teams of every pairing (NHalfFields_ per pairing)
        std::vector<TableTeam> pairingArray_;
        std::vector<std::vector<uint>> fieldMixArray_;
        std::vector<uint> indexArray_;
        std::vector<uint> indexArrayInit_;
        std::vector<std::vector<uint>> limGamesFieldType_;
        std::vector<std::vector<std::vector<uint>>> indexArrays_;
        std::vector<uint> fieldTypeSortArray_;
        std::vector<uint> fieldTypesSorted_;
        std::vector<uint> NFieldTypeArray_;


    public:
        TreeSearch(DebugLog dbg): dbg_{std::move(dbg)}
        {
        }

        // Normalized capacity and field types (part of init, without the tables)
        void prepare();
        // Number of possible rounds (pairings with field type assignments), after prepare
        [[nodiscard]] double roundCount() const;
        // Number of pairings of the teams (with virtual teams), after prepare
        [[nodiscard]] double pairingCount() const;
        void init();
        void indexedFieldTypeArrayInit();
        void indexedClubArrayInit();
        void initNSubLevel();
        bool fieldMixArraySubLevelStep(uint i);
        void fieldMixArraySubLevelInit(uint j);
        void initIndexArrays();
        void initPairingArray();
        void addPairings(std::span<const TableTeam> teams, std::vector<TableTeam>& round, uint depth, std::vector<std::vector<TableTeam>>& free);
        void initFieldMixArray();
        // A row of fieldMixArray_ for the current sub levels
        std::vector<uint> fieldMixRow();
        void displayPairingArray();
        void displayPairingArrayFull();
        void displaySearchArray();
        void displayIndexedFieldTypeArray();
        void displayFieldTypes();
        void displayIndexedClubArray();
        void displayIndexArray();
        void displayIndexArrayInit();
        void displayFieldMixArray();
        uint getTeam(uint pairingIndexFull, uint field, uint gameIndex);
        [[nodiscard]] uint getNPairingArray() const;
        [[nodiscard]] uint getNFieldMixArray() const;
        [[nodiscard]] uint getNHalfFields() const;
        GamePlanRoundByTeamId getRound(uint pairingIndexFull);
        // pairing i (without field mix)
        [[nodiscard]] GamePlanRoundByTeamId pairing(uint i) const;
        // the teams of all pairings, 2 * getNFields() per pairing
        [[nodiscard]] std::span<const TableTeam> pairingTeams() const
        {
            return pairingArray_;
        }
        // the group of fields (field type and subtype, the pause fields last) of a position
        [[nodiscard]] uint fieldGroup(uint position) const
        {
            uint group = 0;
            for (uint end = NFieldTypeArray_[0]; position >= end; end += NFieldTypeArray_[group])
                group++;
            return group;
        }
        // field mix m: the game on every position
        [[nodiscard]] std::span<const uint> fieldMix(uint m) const
        {
            return fieldMixArray_[m];
        }
        uint getFieldTypesSorted(uint j);
        uint getFieldTypeSortArray(uint j);
        [[nodiscard]] uint getNFields() const;
        [[nodiscard]] uint getNClubs() const;
        [[nodiscard]] uint getNTeams() const;
        void setCapacity(Capacity c) {input_.atomicCapacity = c;};
        void setExtraPauseFields(uint n) {input_.extraPauseFields = n;};
        void setMapping(Mapping m) {input_.mapping = std::move(m);};

    };
}//namespace smfigapl
