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
#include <array>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

#include "smfigapl/solver/sm_tables.hpp"
#include "smfigapl/model/sm_debug.hpp"

void smfigapl::RoundTables::build()
{
    NCandidates_ = ts_.getNPairingArray() * ts_.getNFieldMixArray() * input_.conditions.iterations;
    searchIndexMaxInit();
    initCandidates();
    initRepeatedGames();
}

smfigapl::IndexedTeam smfigapl::RoundTables::indexedTeam(uint id) const
{
    const Mapping& mapping = tables_.atomicMappingArray[tables_.atomicIndex];
    IndexedTeam team;
    team.setTeamClub(mapping.indexedClubByTeamId[id]);
    team.setTeamColor(mapping.indexedColorByTeamId[id]);
    team.setTeamId(id);
    return team;
}

smfigapl::IndexedRound smfigapl::RoundTables::candidate(uint i) const
{
    const Candidates& c = tables_.candidates;
    const Candidates::View v = c.view(i);
    IndexedRound round(c.NPositions);
    for (uint j = 0; j < c.NPositions; j++)
    {
        IndexedGame game;
        if (v.team(j, 0) != TEAM_UNDEFINED && v.team(j, 1) != TEAM_UNDEFINED)
        {
            for (uint k = 0; k < 2; k++)
                game.setTeam(k, indexedTeam(v.team(j, k)));
        }
        else
        {
            // a virtual team: pause, the real team (if any) without club and color
            for (uint k = 0; k < 2; k++)
            {
                IndexedTeam team;
                team.setTeamId(v.team(j, k));
                game.setTeam(k, team);
            }
            game.setPause(true);
        }
        round.setGame(j, game);
    }
    for (const TableTeam team : c.pauseTeams(i))
        round.addPauseTeam(indexedTeam(team));
    return round;
}

// The structure of the candidates (pairings and field mixes), the valid
// ones and the bounds. Only if candidates may differ (virtual teams, pause
// fields, subtypes) every one is looked at.
void smfigapl::RoundTables::initCandidates()
{
    const Capacity& capacity = tables_.atomicCapacityArray[tables_.atomicIndex];
    const Mapping& mapping = tables_.atomicMappingArray[tables_.atomicIndex];
    Candidates& c = tables_.candidates;
    c.NTeams = capacity.NTeams;
    c.NPositions = capacity.NFields;
    c.NGames = ts_.getNFields();
    c.NPairings = ts_.getNPairingArray();
    c.NMixes = ts_.getNFieldMixArray();
    c.fieldType.clear();
    for (uint j = 0; j < capacity.NFields; j++)
        c.fieldType.push_back(ts_.getFieldTypesSorted(j));
    c.club.clear();
    for (uint team = 0; team < capacity.NTeams; team++)
        c.club.push_back(mapping.indexedClubByTeamId[team].value());
    c.pairings = ts_.pairingTeams();
    c.rotation.resize(c.NPairings);
    for (uint p = 0; p < c.NPairings; p++)
        c.rotation[p] = static_cast<std::uint16_t>(p % c.NMixes);
    c.mixes.clear();
    c.positions.assign(static_cast<size_t>(c.NMixes) * c.NGames, 0);
    for (uint m = 0; m < c.NMixes; m++)
    {
        const auto mix = ts_.fieldMix(m);
        for (uint j = 0; j < c.NGames; j++)
        {
            c.mixes.push_back(static_cast<std::uint8_t>(mix[j]));
            c.positions[static_cast<size_t>(m) * c.NGames + mix[j]] = static_cast<std::uint8_t>(j);
        }
    }
    c.group.clear();
    for (uint j = 0; j < c.NGames; j++)
        c.group.push_back(static_cast<std::uint8_t>(ts_.fieldGroup(j)));
    // without pause fields the hash depends on the pairing only
    c.pairingHash.clear();
    c.count = NCandidates_;
    if (!c.hasPauseFields())
        for (uint p = 0; p < c.NPairings; p++)
            c.pairingHash.push_back(c.hash(p * c.NMixes));
    c.valid.clear();
    c.count = NCandidates_;
    boundsInit();
    // every candidate is valid without subtypes if every empty field may be
    // empty (at most one per virtual team): the bounds per pairing
    const uint virtualTeams = ts_.getNTeams() - capacity.NTeams;
    if (!input_.conditions.fieldSubTypeDeny.enabled && virtualTeams <= input_.conditions.maxFieldsEmpty && !input_.dbg.has(DebugFlag::gps))
    {
        boundsByPairing();
        return;
    }
    const uint all = NCandidates_;
    // the list of the valid candidates starts with the first one left out
    std::vector<std::uint32_t> valid;
    for (uint f = 0; f < all; f++)
    {
        if (input_.dbg.has(DebugFlag::gps))
            input_.dbg.out(DebugFlag::gps) << "iii" << f % (c.NPairings * c.NMixes) << std::endl;
        if (isImpossible(f))
        {
            if (NCandidates_ == all)
            {
                valid.resize(f);
                std::iota(valid.begin(), valid.end(), 0U);
            }
            NCandidates_--;
            searchIndexMaxUpdate(f - (all - NCandidates_ - 1));
            continue;
        }
        if (NCandidates_ < all)
            valid.push_back(f);
        boundsAdd(f);
    }
    if (NCandidates_ < all)
        c.valid = std::move(valid);
    c.count = NCandidates_;
    if (NCandidates_ == 0)
        boundsInit();
}

bool smfigapl::RoundTables::isImpossible(uint i) const
{
    const Candidates& c = tables_.candidates;
    const Candidates::View v = c.view(i);
    uint emptyFields = 0;
    for (uint j = 0; j < c.NPositions; j++)
        if (v.team(j, 0) == TEAM_UNDEFINED || v.team(j, 1) == TEAM_UNDEFINED)
            emptyFields++;
    if (emptyFields > input_.conditions.maxFieldsEmpty)
        return true;

    //denylist (also for the real team of a pause game)
    if (input_.conditions.fieldSubTypeDeny.enabled)
    {
        const Mapping& mapping = tables_.atomicMappingArray[tables_.atomicIndex];
        for (uint j = 0; j < c.NPositions; j++)
            for (uint k = 0; k < 2; k++)
            {
                const uint teamId = v.team(j, k);
                if (teamId != TEAM_UNDEFINED && mapping.fieldDenyBySubTypeAndTeamId[teamId][mapping.indexedFieldSubTypeByFieldId[ts_.getFieldTypeSortArray(j)]] != 0U)
                    return true;
            }
    }
    return false;
}

// The repeated games jumps without pause fields: every candidate of a
// pairing contains its games, so a run of candidates with a game is a run of
// pairings (with valid candidates) with it. Backwards, because a run that
// continues at the next pairing ends where the run of that pairing ends.
void smfigapl::RoundTables::initRepeatedGames()
{
    Candidates& c = tables_.candidates;
    c.runEnd.clear();
    c.validStart.clear();
    if (input_.dbg.has(DebugFlag::gps))
    {
        const uint NPairings = getNPairingIndexShort(c.NTeams);
        for (uint i = 0; i < c.size(); i++)
            c.forEachGame(i, [&](uint, uint a, uint b)
        {
            input_.dbg.out(DebugFlag::gps) << "idx" << NPairings << " " << c.pairingKey(a, b) << " " << a << " " << b << std::endl;
        });
    }
    if (c.hasPauseFields() || c.size() == 0)
        return;
    const uint NSequence = c.NPairings * input_.conditions.iterations;
    if (!c.valid.empty())
    {
        // the first valid candidate of every pairing (and the end)
        c.validStart.assign(NSequence + 1, c.size());
        uint i = 0;
        for (uint s = 0; s < NSequence; s++)
        {
            while (i < c.size() && c.valid[i] < static_cast<std::uint64_t>(s) * c.NMixes)
                i++;
            c.validStart[s] = i;
        }
    }
    auto empty = [&c](uint s)
    {
        return !c.validStart.empty() && c.validStart[s] == c.validStart[s + 1];
    };
    c.runEnd.assign(static_cast<size_t>(NSequence) * c.NGames, 0);
    const uint NKeys = getNPairingIndexShort(c.NTeams);
    std::vector<uint> end(NKeys, 0);
    std::vector<uint> last(NKeys, std::numeric_limits<uint>::max());
    // the next pairing with valid candidates
    uint next = NSequence;
    for (uint s = NSequence; s-- > 0;)
    {
        if (empty(s))
            continue;
        const auto teams = c.pairings.subspan(static_cast<size_t>(s % c.NPairings) * 2 * c.NGames, 2 * static_cast<size_t>(c.NGames));
        for (uint g = 0; g < c.NGames; g++)
        {
            const uint a = teams[2 * static_cast<size_t>(g)];
            const uint b = teams[2 * static_cast<size_t>(g) + 1];
            if (a >= c.NTeams || b >= c.NTeams)
                continue;
            const uint key = c.pairingKey(a, b);
            end[key] = last[key] == next ? end[key] : s + 1;
            last[key] = s;
            c.runEnd[static_cast<size_t>(s) * c.NGames + g] = end[key];
        }
        next = s;
    }
}

void smfigapl::RoundTables::boundsInit()
{
    const uint NFieldTypes = tables_.atomicCapacityArray[tables_.atomicIndex].NFieldTypes;
    CondTables::Bounds& bounds = tables_.bounds;
    bounds.minSlots.assign(NFieldTypes, NCandidates_ == 0 ? 0 : std::numeric_limits<uint>::max());
    bounds.maxSlots.assign(NFieldTypes, 0);
    bounds.minPauses = NCandidates_ == 0 ? 0 : std::numeric_limits<uint>::max();
    bounds.maxPauses = 0;
}

// The field mixes are all assignments of the games of a pairing to the
// groups of fields: the most and the fewest real games (both teams real) on
// the fields of a field type and on the pause fields follow from the number
// of real games of the pairing and of the other games.
void smfigapl::RoundTables::boundsByPairing()
{
    const Candidates& c = tables_.candidates;
    CondTables::Bounds& bounds = tables_.bounds;
    const auto NFieldTypes = static_cast<uint>(bounds.minSlots.size());
    std::array<uint, MAX_FIELDS> fields{};
    for (const uint type : c.fieldType)
        if (type < NFieldTypes)
            fields[type]++;
    const uint pauseFields = c.NGames - c.NPositions;
    for (uint p = 0; p < c.NPairings && NCandidates_ > 0; p++)
    {
        const auto teams = c.pairings.subspan(static_cast<size_t>(p) * 2 * c.NGames, 2 * static_cast<size_t>(c.NGames));
        uint real = 0;
        uint halfVirtual = 0;
        for (uint g = 0; g < c.NGames; g++)
        {
            const bool a = teams[2 * static_cast<size_t>(g)] < c.NTeams;
            const bool b = teams[2 * static_cast<size_t>(g) + 1] < c.NTeams;
            real += a && b ? 1 : 0;
            halfVirtual += a != b ? 1 : 0;
        }
        const uint other = c.NGames - real;
        for (uint l = 0; l < NFieldTypes; l++)
        {
            bounds.minSlots[l] = std::min(bounds.minSlots[l], 2 * (fields[l] > other ? fields[l] - other : 0));
            bounds.maxSlots[l] = std::max(bounds.maxSlots[l], 2 * std::min(fields[l], real));
        }
        // the real team of a game with a virtual team pauses, both teams of a game on a pause field
        bounds.minPauses = std::min(bounds.minPauses, halfVirtual + 2 * (pauseFields > other ? pauseFields - other : 0));
        bounds.maxPauses = std::max(bounds.maxPauses, halfVirtual + 2 * std::min(pauseFields, real));
    }
}

void smfigapl::RoundTables::boundsAdd(uint i)
{
    const Candidates& c = tables_.candidates;
    CondTables::Bounds& bounds = tables_.bounds;
    const auto NFieldTypes = static_cast<uint>(bounds.minSlots.size());
    std::array<uint, MAX_FIELDS> slots{};
    c.forEachGame(i, [&](uint j, uint, uint)
    {
        if (c.fieldType[j] < NFieldTypes)
            slots[c.fieldType[j]] += 2;
    });
    for (uint l = 0; l < NFieldTypes; l++)
    {
        bounds.minSlots[l] = std::min(bounds.minSlots[l], slots[l]);
        bounds.maxSlots[l] = std::max(bounds.maxSlots[l], slots[l]);
    }
    uint pauses = 0;
    c.forEachPause(i, [&pauses](uint)
    {
        pauses++;
    });
    bounds.minPauses = std::min(bounds.minPauses, pauses);
    bounds.maxPauses = std::max(bounds.maxPauses, pauses);
}

void smfigapl::RoundTables::searchIndexMaxInit()
{
    tables_.searchIndexMax.resize(tables_.atomicCapacityArray[tables_.atomicIndex].NRounds);
    uint k = ts_.getNFields() * 2 - 1;
    uint l = ts_.getNPairingArray();
    l = l / k * ts_.getNFieldMixArray();
    tables_.searchIndexMax[tables_.atomicCapacityArray[tables_.atomicIndex].NRounds - 1] = NCandidates_ - 1;
    for (uint i = 2; i < input_.setup.realCapacity.NRounds + 1; i++)
    {
        if (tables_.searchIndexMax[input_.setup.realCapacity.NRounds - i + 1] >= l)
            tables_.searchIndexMax[input_.setup.realCapacity.NRounds - i] = tables_.searchIndexMax[input_.setup.realCapacity.NRounds - i + 1] - l;
        else
            //too much rounds, not possible anyway, but limit to 0
            tables_.searchIndexMax[input_.setup.realCapacity.NRounds - i] = 0;
    }
}

// candidate i is dropped: the bounds behind it move one down. A bound of 0
// stays (it wrapped around and the search ran past the candidates); the
// bounds only limit the search, a larger one is safe.
void smfigapl::RoundTables::searchIndexMaxUpdate(uint i)
{
    for (uint& k : tables_.searchIndexMax)
        if (k >= i && k > 0)
            k--;
}
