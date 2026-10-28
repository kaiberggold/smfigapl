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

#include <array>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "smfigapl/solver/sm_search_types.hpp"


void smfigapl::displayPairingIndicesShort(std::ostream& os, uint NTeams)
{
    uint i = 0;
    os << "PairingIndixesShort:, NTeams: " << NTeams << std::endl;
    for (uint a = 0; a < NTeams; a++)
        for (uint b = a + 1; b < NTeams; b++)
        {
            os << "i: " << i << " ab: " << a << b << " " << std::endl;
            i++;
        }
    os << std::endl;
}

smfigapl::uint smfigapl::getNPairingIndexShort(uint NTeams)
{
    return (NTeams - 1) * NTeams / 2;
}

bool smfigapl::fieldCompare(smfigapl::FieldSortable& l, smfigapl::FieldSortable& r)
{
    return (l.fieldType < r.fieldType) || (l.fieldType == r.fieldType && l.fieldSubType < r.fieldSubType);
}

void smfigapl::fieldSort(std::span<FieldSortable> fs)
{
    std::ranges::sort(fs, smfigapl::fieldCompare);
}



//GamePlanRoundByTeamId

void smfigapl::GamePlanRoundByTeamId::init(uint NFields)
{
    gamePlanRoundByTeamId_.resize(NFields << 1);
}
void smfigapl::GamePlanRoundByTeamId::reset()
{
    std::ranges::fill(gamePlanRoundByTeamId_, 0);
}

smfigapl::uint smfigapl::GamePlanRoundByTeamId::getTeamId(uint index)
{
    return gamePlanRoundByTeamId_[index];
}
smfigapl::uint smfigapl::GamePlanRoundByTeamId::getTeamId(uint field, uint gameIndex)
{
    return gamePlanRoundByTeamId_[(field << 1) + gameIndex];
}
void smfigapl::GamePlanRoundByTeamId::setTeamId(uint index, uint teamId)
{
    gamePlanRoundByTeamId_[index] = teamId;
}
void smfigapl::GamePlanRoundByTeamId::setTeamId(uint field, uint gameIndex, uint teamId)
{
    smfigapl::GamePlanRoundByTeamId::gamePlanRoundByTeamId_[(field << 1) + gameIndex] = teamId;
}
smfigapl::uint smfigapl::GamePlanRoundByTeamId::size()
{
    return static_cast<uint>(gamePlanRoundByTeamId_.size());
}

//GamePlanByTeamId

smfigapl::uint smfigapl::GamePlanByTeamId::getTeamId(uint round, uint index)
{
    return gamePlanByTeamId_[round].getTeamId(index);
}
smfigapl::uint smfigapl::GamePlanByTeamId::getTeamId(uint round, uint field, uint gameIndex)
{
    return gamePlanByTeamId_[round].getTeamId(field, gameIndex);
}
void smfigapl::GamePlanByTeamId::setTeamId(uint round, uint index, uint teamId)
{
    gamePlanByTeamId_[round].setTeamId(index, teamId);
}
void smfigapl::GamePlanByTeamId::setTeamId(uint round, uint field, uint gameIndex, uint teamId)
{
    gamePlanByTeamId_[round].setTeamId(field, gameIndex, teamId);
}
smfigapl::GamePlanRoundByTeamId smfigapl::GamePlanByTeamId::getRound(uint round)
{
    return gamePlanByTeamId_[round];
}

smfigapl::GamePlanRoundByTeamId smfigapl::roundFieldMix(GamePlanRoundByTeamId roundArray_, std::span<const uint> mixArray)
{
    GamePlanRoundByTeamId temp(roundArray_.size() >> 1);
    for (uint i = 0; i < mixArray.size(); i++)
    {
        for (uint k = 0; k < 2; k++)
            temp.setTeamId(i, k, roundArray_.getTeamId(mixArray[i], k));
    }

    return temp;
}

std::vector<smfigapl::TableTeam> smfigapl::Candidates::pauseTeams(uint i) const
{
    std::vector<TableTeam> teams;
    forEachPause(i, [&teams](uint team)
    {
        teams.push_back(static_cast<TableTeam>(team));
    });
    std::ranges::sort(teams);
    return teams;
}

smfigapl::uint smfigapl::Candidates::hash(uint i) const
{
    const View v = view(i);
    std::array<uint, MAX_FIELDS> firstTeams{};
    for (uint j = 0; j < NPositions; j++)
        firstTeams[j] = v.team(j, 0);
    const auto teams = std::span(firstTeams).first(NPositions);
    std::ranges::sort(teams);
    uint hash = 0;
    for (uint team : teams)
    {
        hash = hash | team;
        hash = hash << 4; //NRealTeams<=16
    }
    return hash;
}

bool smfigapl::Candidates::sameHash(uint i, uint k) const
{
    // without pause fields all games of a pairing are on fields: the first
    // teams depend on the pairing only
    if (!pairingHash.empty())
        return pairingHash[sequence(i) % NPairings] == pairingHash[sequence(k) % NPairings];
    return hash(i) == hash(k);
}

bool smfigapl::Candidates::containsGame(uint i, uint key) const
{
    bool found = false;
    forEachGame(i, [&](uint, uint a, uint b)
    {
        found = found || pairingKey(a, b) == key;
    });
    return found;
}

smfigapl::uint smfigapl::Candidates::gameJump(uint i, uint position) const
{
    const View v = view(i);
    if (!hasPauseFields())
    {
        // every candidate of a pairing contains its games: the run ends with a pairing
        const uint end = runEnd[static_cast<size_t>(sequence(i)) * NGames + v.game(position)];
        const uint start = validStart.empty() ? end * NMixes : validStart[end];
        return std::min(start - i, count - i - 1);
    }
    // a game on a pause field is no game: follow the candidates
    const uint key = pairingKey(v.team(position, 0), v.team(position, 1));
    uint k = i + 1;
    while (k < count && containsGame(k, key))
        k++;
    return std::min(k - i, count - i - 1);
}

smfigapl::uint smfigapl::Candidates::position(uint i, uint team) const
{
    const auto [pos, other] = view(i).find(team);
    // a pause: on a pause field or against a virtual team
    return pos >= NPositions || other == TEAM_UNDEFINED ? NPositions : pos;
}

smfigapl::uint smfigapl::Candidates::fieldTypeJump(uint i, uint team) const
{
    const uint pos = position(i, team);
    if (pos == NPositions || i + 1 >= count)
        return 0;
    const uint type = fieldType[pos];
    uint jump = 1;
    for (uint k = i + 1; k + 1 < count; k++)
    {
        const uint p = position(k, team);
        if (p == NPositions || fieldType[p] != type)
            break;
        jump++;
    }
    return jump;
}
