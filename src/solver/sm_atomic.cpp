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
#include <vector>

#include "smfigapl/solver/sm_atomic.hpp"

namespace
{
    using smfigapl::uint;

    // Appends value if it is not contained yet; true if it was appended
    template<class T> bool pushIfNew(std::vector<T>& v, T value)
    {
        if (std::ranges::find(v, value) != v.end())
            return false;
        v.push_back(value);
        return true;
    }

    // Field subtypes, teams, clubs and colors of one independent set
    struct AtomicSet
    {
        std::vector<uint> subTypes;
        std::vector<uint> teams;
        std::vector<smfigapl::ClubId> clubs;
        std::vector<smfigapl::ColorId> colors;
    };

    // The set that contains the subtype: subtypes and teams that may play together
    AtomicSet collectAtomicSet(const smfigapl::Input& in, uint firstSubType)
    {
        const auto& deny = in.conditions.fieldSubTypeDeny.values;
        const auto& mapping = in.setup.mapping;
        const uint NSubTypes = in.setup.realCapacity.NFieldSubTypes;
        AtomicSet set;
        std::vector<bool> currentSubTypes(NSubTypes, false);
        currentSubTypes[firstSubType] = true;
        set.subTypes.push_back(firstSubType);
        bool update = false;
        do
        {
            update = false;
            // teams allowed on one of the subtypes
            for (uint i = 0; i < in.setup.realCapacity.NTeams; i++)
                for (uint j = 0; j < NSubTypes; j++)
                    if (deny[i][j] == 0U && currentSubTypes[j] && pushIfNew(set.teams, i))
                    {
                        update = true;
                        pushIfNew(set.clubs, mapping.indexedClubByTeamId[i]);
                        pushIfNew(set.colors, mapping.indexedColorByTeamId[i]);
                    }
            // subtypes allowed for one of the teams
            for (uint i : set.teams)
                for (uint j = 0; j < NSubTypes; j++)
                    if (deny[i][j] == 0U && pushIfNew(set.subTypes, j))
                    {
                        currentSubTypes[j] = true;
                        update = true;
                    }
        }
        while (update);
        return set;
    }

    // Capacity and mappings of the set with the index sets.count
    void addAtomicSet(const smfigapl::Input& in, const AtomicSet& set, smfigapl::CondTables& tables, smfigapl::AtomicSets& sets)
    {
        const auto& mapping = in.setup.mapping;
        smfigapl::Capacity& capacity = tables.atomicCapacityArray[sets.count];
        smfigapl::Mapping atomicMap;
        std::vector<uint> fieldMap;
        for (uint i = 0; i < in.setup.realCapacity.NFields; i++)
        {
            if (std::ranges::find(set.subTypes, mapping.indexedFieldSubTypeByFieldId[i]) == set.subTypes.end())
                continue;
            sets.indexByFieldId[i] = sets.count;
            capacity.NFields++;
            capacity.NFieldSubTypes = static_cast<uint>(set.subTypes.size());
            capacity.NTeams = static_cast<uint>(set.teams.size());
            capacity.NClubs = static_cast<uint>(set.clubs.size());
            capacity.NTeamColors = static_cast<uint>(set.colors.size());
            atomicMap.indexedFieldTypeByFieldId.push_back(mapping.indexedFieldTypeByFieldId[i]);
            atomicMap.indexedFieldSubTypeByFieldId.push_back(mapping.indexedFieldSubTypeByFieldId[i]);
            fieldMap.push_back(i);
        }
        for (uint i : set.teams)
        {
            atomicMap.indexedClubByTeamId.push_back(mapping.indexedClubByTeamId[i]);
            atomicMap.indexedColorByTeamId.push_back(mapping.indexedColorByTeamId[i]);
            atomicMap.fieldDenyBySubTypeAndTeamId.push_back(in.conditions.fieldSubTypeDeny.values[i]);
            sets.indexByTeamId[i] = sets.count;
        }
        tables.fieldMappingArray.push_back(fieldMap);
        tables.teamsMappingArray.push_back(set.teams);
        for (uint i = 0; i < set.teams.size(); i++)
            tables.teamIdMappingArray[set.teams[i]] = i;
        capacity.NRounds = in.setup.realCapacity.NRounds;
        // the field type ids stay those of the configuration (the limits of
        // the conditions belong to them); a type without a field in the set
        // has no place there. Counting only the types of the set made the
        // games on a type with a higher id disappear from the counters.
        capacity.NFieldTypes = in.setup.realCapacity.NFieldTypes;
        tables.atomicMappingArray.push_back(atomicMap);
    }
}

smfigapl::AtomicSets smfigapl::calcAtomicSets(const Input& in, CondTables& tables)
{
    const uint NSubTypes = in.setup.realCapacity.NFieldSubTypes;
    std::vector<bool> assigned(NSubTypes, false);
    AtomicSets sets;
    sets.indexByFieldId.resize(in.setup.realCapacity.NFields);
    sets.indexByTeamId.resize(in.setup.realCapacity.NTeams);
    tables.atomicCapacityArray.clear();
    tables.atomicMappingArray.clear();
    tables.fieldMappingArray.clear();
    tables.teamsMappingArray.clear();
    tables.teamIdMappingArray.resize(in.setup.realCapacity.NTeams);
    for (uint first = 0; first < NSubTypes; first++)
    {
        if (assigned[first])
            continue;
        const AtomicSet set = collectAtomicSet(in, first);
        for (uint j : set.subTypes)
            assigned[j] = true;
        tables.atomicCapacityArray.resize(tables.atomicCapacityArray.size() + 1);
        addAtomicSet(in, set, tables, sets);
        sets.count++;
    }
    return sets;
}
