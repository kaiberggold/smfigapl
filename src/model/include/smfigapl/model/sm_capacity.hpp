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
#include "smfigapl/model/sm_types.hpp"

namespace smfigapl
{

// Dimensions of a game plan setup and the mappings to the indexed representation
    class Capacity
    {
    public:
        uint NRounds = 0;
        uint NFields = 0;
        uint NFieldTypes = 0;
        uint NFieldSubTypes = 0;
        uint NTeams = 0;
        uint NClubs = 0;
        uint NTeamColors = 0;
    };

    class Mapping
    {
    public:
        std::vector<uint> indexedFieldTypeByFieldId;
        std::vector<uint> indexedFieldSubTypeByFieldId;
        std::vector<std::vector<uint>> fieldDenyBySubTypeAndTeamId;
        std::vector<smfigapl::ClubId> indexedClubByTeamId;
        std::vector<smfigapl::ColorId> indexedColorByTeamId;
        void reset()
        {
            indexedFieldTypeByFieldId.clear();
            indexedFieldSubTypeByFieldId.clear();
            fieldDenyBySubTypeAndTeamId.clear();
            indexedClubByTeamId.clear();
            indexedColorByTeamId.clear();
        }

    };

}//namespace smfigapl
