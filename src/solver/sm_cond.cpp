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
#include <bit>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>
#include <string>

#include "smfigapl/solver/sm_cond.hpp"
#include "smfigapl/model/sm_debug.hpp"
#include "smfigapl/model/sm_out.hpp"

namespace
{
    // a - b, 0 if b is larger
    smfigapl::uint minus(smfigapl::uint a, smfigapl::uint b)
    {
        return a > b ? a - b : 0;
    }
}

void smfigapl::CheckGamePlan::init()
{
    // the atomic set to check (it was never set: all sets were checked with
    // the number of teams and field types of the first one)
    atomicIndex_ = condTables_.atomicIndex;
    NPairingIndexShort_ = getNPairingIndexShort(condTables_.atomicCapacityArray[atomicIndex_].NTeams);
    res_.check = false;
    count_.pairing.assign(NPairingIndexShort_, 0);
    jumpCheck_.assign(NPairingIndexShort_, false);
    checkArray_.assign(NPairingIndexShort_, false);
    count_.sameClub.assign(condTables_.atomicCapacityArray[atomicIndex_].NTeams, 0);
    count_.pause.assign(condTables_.atomicCapacityArray[atomicIndex_].NTeams, 0);
    std::vector<uint> temp;
    temp.assign(condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes, 0);
    count_.fieldType.assign(condTables_.atomicCapacityArray[atomicIndex_].NTeams, temp);
    std::vector<bool> temp2;
    temp2.assign(condTables_.atomicCapacityArray[atomicIndex_].NTeams, false);
    checkMax_.assign(condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes, temp2);

    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const Mapping& mapping = condTables_.atomicMappingArray[atomicIndex_];
    clubMates_.assign(capacity.NTeams, 0);
    for (uint m = 0; m < capacity.NTeams; m++)
        for (uint o = 0; o < capacity.NTeams; o++)
            if (o != m && mapping.indexedClubByTeamId[o] == mapping.indexedClubByTeamId[m])
                clubMates_[m]++;
    // the field type limits apply where the type has fields: a sub
    // tournament without a field of a type has no limit for it
    limits_.min.assign(capacity.NFieldTypes, 0);
    limits_.max.assign(capacity.NFieldTypes, std::numeric_limits<uint>::max());
    if (input_.conditions.limGamesFieldType.enabled)
        for (const uint type : mapping.indexedFieldTypeByFieldId)
        {
            limits_.min[type] = input_.conditions.limGamesFieldType.values.min[type];
            limits_.max[type] = input_.conditions.limGamesFieldType.values.max[type];
        }
    // the limit of gamePlanCheckRepeatedGames in the last round
    pairingLimit_ = std::min((capacity.NRounds - 1) / std::max(capacity.NTeams - 1, 1U) + 1, input_.conditions.iterations);
    symmetryInit();
}

void smfigapl::CheckGamePlan::symmetryInit()
{
    // the class of a team: everything a condition knows about it (club and
    // club limit with a club limit, the subtypes it may not use)
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const Mapping& mapping = condTables_.atomicMappingArray[atomicIndex_];
    const auto& club = input_.conditions.maxGamesSameClub;
    const auto& deny = input_.conditions.fieldSubTypeDeny;
    std::map<std::vector<uint>, uint> classes;
    symmetry_.teamClass.clear();
    for (uint m = 0; m < capacity.NTeams; m++)
    {
        std::vector<uint> properties;
        if (club.enabled)
            properties = {mapping.indexedClubByTeamId[m].value(), club.values[m]};
        if (deny.enabled)
            properties.insert(properties.end(), mapping.fieldDenyBySubTypeAndTeamId[m].begin(), mapping.fieldDenyBySubTypeAndTeamId[m].end());
        const auto [it, added] = classes.try_emplace(properties, static_cast<uint>(classes.size()));
        symmetry_.teamClass.push_back(it->second);
    }
    symmetry_.NClasses = static_cast<uint>(classes.size());
    symmetry_.first.clear();
    symmetry_.scanned = 0;
}

std::vector<smfigapl::uint> smfigapl::CheckGamePlan::roundKind(uint i) const
{
    const Candidates& c = condTables_.candidates;
    const Candidates::View v = c.view(i);
    const uint NClasses = symmetry_.NClasses + 1;
    auto teamClass = [&](uint team)
    {
        return team == TEAM_UNDEFINED ? symmetry_.NClasses : symmetry_.teamClass[team];
    };
    // the iteration, then per game: group of fields, the classes of both teams
    std::vector<uint> kind;
    kind.push_back(c.full(i) / (c.NPairings * c.NMixes));
    for (uint j = 0; j < c.NGames; j++)
    {
        const uint a = teamClass(v.team(j, 0));
        const uint b = teamClass(v.team(j, 1));
        kind.push_back((c.group[j] * NClasses + std::min(a, b)) * NClasses + std::max(a, b));
    }
    std::sort(kind.begin() + 1, kind.end());
    return kind;
}

bool smfigapl::CheckGamePlan::symmetryFirst(uint i)
{
    for (; symmetry_.scanned <= i; symmetry_.scanned++)
        symmetry_.first.try_emplace(roundKind(symmetry_.scanned), symmetry_.scanned);
    return symmetry_.first.at(roundKind(i)) == i;
}

smfigapl::uint smfigapl::CheckGamePlan::symmetryNext(uint i)
{
    const uint last = condTables_.searchIndexMax[0];
    for (uint k = std::max(i + 1, symmetry_.scanned); k <= last; k++)
    {
        symmetry_.scanned = k + 1;
        if (symmetry_.first.try_emplace(roundKind(k), k).second)
            return k;
    }
    return last + 1;
}


bool smfigapl::CheckGamePlan::pairingLimitReached(uint pairing) const
{
    // the same limit as in gamePlanCheckRepeatedGames, for one more game
    return count_.pairing[pairing] >= std::min(treeSearchLevel_ / (condTables_.atomicCapacityArray[atomicIndex_].NTeams - 1) + 1, input_.conditions.iterations);
}

void smfigapl::CheckGamePlan::calcRepeatedGamesJump()
{
    const Candidates& c = condTables_.candidates;
    const auto add = static_cast<uint>(treeSearchState_.Add);
    // the games of Add from updateCounters
    const auto games = std::span(games_).first(NGames_);
    std::fill(jumpCheck_.begin(), jumpCheck_.end(), false);
    for (const Game& game : games)
        jumpCheck_[game.key] = true;
    c.forEachGame(static_cast<uint>(treeSearchState_.Sub), [&](uint, uint a, uint b)
    {
        jumpCheck_[c.pairingKey(a, b)] = false;
    });

    // the failed pairings of the added round that the replaced round did not have
    for (const Game& game : games)
        if (jumpCheck_[game.key] && !checkArray_[game.key])
            res_.jump = std::max(res_.jump, c.gameJump(add, game.position));
}

bool smfigapl::CheckGamePlan::gamePlanCheckRepeatedGames(bool init)
{
    bool check = true;
    std::fill(checkArray_.begin(), checkArray_.end(), true);
    const Candidates& c = condTables_.candidates;
    if (!init && treeSearchState_.Add >= 0)
    {
        // the games of Add from updateCounters
        for (const Game& game : std::span(games_).first(NGames_))
        {
            // with a game there are at least two teams
            const uint limit = std::min(treeSearchLevel_ / (condTables_.atomicCapacityArray[atomicIndex_].NTeams - 1) + 1, input_.conditions.iterations);
            if (count_.pairing[game.key] > limit)
            {
                check = false;
                checkArray_[game.key] = false;
            }
        }
    }
    //Same level, current  check failed, calc jump
    if (!check && treeSearchState_.stepResult == StepResult::nextStep && treeSearchState_.Add > 0 && treeSearchState_.Sub > 0 && \
            !c.sameHash(static_cast<uint>(treeSearchState_.Add), static_cast<uint>(treeSearchState_.Sub)))
        calcRepeatedGamesJump();
    if constexpr(!fastJump)
    {
        res_.jump = 1;
    }
    if (input_.dbg.has(DebugFlag::cond))
    {
        input_.dbg.out(DebugFlag::cond) << "jump " << res_.jump << std::endl;
        uint count = 0;
        for (uint i = 0; i < NPairingIndexShort_; i++)
        {
            input_.dbg.out(DebugFlag::cond) << count_.pairing[i] << " ";
            count += count_.pairing[i];
        }
        input_.dbg.out(DebugFlag::cond) << "count: " << count;
    }
    return check;
}


template<class Op> void smfigapl::CheckGamePlan::forEachCounter(uint index, Op op)
{
    const Candidates& c = condTables_.candidates;
    const uint NFieldTypes = condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes;
    const bool club = input_.conditions.maxGamesSameClub.enabled;
    const Candidates::View v = c.view(index);
    NGames_ = 0;
    for (uint j = 0; j < c.NGames; j++)
    {
        const uint a = v.team(j, 0);
        const uint b = v.team(j, 1);
        const bool real = a != TEAM_UNDEFINED && b != TEAM_UNDEFINED;
        if (j >= c.NPositions || !real)
        {
            // a pause: the real teams of a pause field or of a game with a virtual team
            if (a != TEAM_UNDEFINED)
                op(count_.pause[a]);
            if (b != TEAM_UNDEFINED)
                op(count_.pause[b]);
            continue;
        }
        // a game: its pairing, a club game, the teams on the field type of its field
        const uint key = c.pairingKey(a, b);
        games_[NGames_++] = {j, key};
        op(count_.pairing[key]);
        if (club && c.club[a] == c.club[b])
        {
            op(count_.sameClub[a]);
            op(count_.sameClub[b]);
        }
        const uint type = c.fieldType[j];
        if (type < NFieldTypes)
        {
            op(count_.fieldType[a][type]);
            op(count_.fieldType[b][type]);
        }
    }
}

void smfigapl::CheckGamePlan::updateCounters(bool init)
{
    if (init)
    {
        // one entry per search index 0..searchIndexMax.back(), only for the
        // debug output of the conditions (a bit per candidate: 280 MB for
        // a billion candidates)
        const size_t size = input_.dbg.has(DebugFlag::cond) ? condTables_.searchIndexMax.back() + size_t{1} : 0;
        NFailedArray_.searchIndex.assign(size, true);
        NFailedArray_.searchIndexFalse.assign(size, false);
        forEachCounter(0, [](uint & c)
        {
            c = 1;
        });
        return;
    }
    if (treeSearchState_.Sub >= 0)
        forEachCounter(static_cast<uint>(treeSearchState_.Sub), [](uint & c)
    {
        c--;
    });
    if (treeSearchState_.Add >= 0)
        forEachCounter(static_cast<uint>(treeSearchState_.Add), [](uint & c)
    {
        c++;
    });
}


bool smfigapl::CheckGamePlan::gamePlanCheckMaxGamesSameClub([[maybe_unused]] bool init)
{
    bool check = true;
    for (uint i = 0; i < condTables_.atomicCapacityArray[atomicIndex_].NTeams; i++)
    {
        if (count_.sameClub[i] > input_.conditions.maxGamesSameClub.values[i])
        {
            check = false;
        }
    }

    if (input_.dbg.has(DebugFlag::cond))
    {
        input_.dbg.out(DebugFlag::cond) << std::endl << "scc: ";
        for (uint i = 0; i < condTables_.atomicCapacityArray[atomicIndex_].NTeams; i++)
        {
            input_.dbg.out(DebugFlag::cond) << count_.sameClub[i] << " ";
        }
        input_.dbg.out(DebugFlag::cond) << std::endl;
    }

    return check;
};


bool smfigapl::CheckGamePlan::gamePlanCheckMaxPauseGames()
{
    bool check = true;

    for (uint i = 0; i < condTables_.atomicCapacityArray[atomicIndex_].NTeams; i++)
    {
        if (count_.pause[i] > input_.conditions.maxPauseGames.value)
        {
            check = false;
        }
    }
    if (input_.dbg.has(DebugFlag::cond))
    {
        input_.dbg.out(DebugFlag::cond) << std::endl << "pc: ";
        for (uint i = 0; i < condTables_.atomicCapacityArray[atomicIndex_].NTeams; i++)
        {
            input_.dbg.out(DebugFlag::cond) << count_.pause[i] << " ";
        }
        input_.dbg.out(DebugFlag::cond) << std::endl;
    }
    return check;
}


void smfigapl::CheckGamePlan::calcFieldTypeJump()
{
    if (treeSearchState_.Add < 0)
        return;
    for (uint l = 0; l < condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes; l++)
        for (uint m = 0; m < condTables_.atomicCapacityArray[atomicIndex_].NTeams; m++)
            if (checkMax_[l][m])
                res_.jump = std::max(res_.jump, condTables_.candidates.fieldTypeJump(static_cast<uint>(treeSearchState_.Add), m));
}

bool smfigapl::CheckGamePlan::gamePlanCheckFieldTypes()
{
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const auto& limits = limits_;
    bool check = true;
    if (maxFieldTypeJump && ftJump_)
    {
        for (auto& teams : checkMax_)
            std::fill(teams.begin(), teams.end(), false);
        ftJump_ = false;
    }
    for (uint m = 0; m < capacity.NTeams; m++)
    {
        for (uint l = 0; l < capacity.NFieldTypes; l++)
        {
            // too few games left to reach the minimum, or above the maximum
            if (limits.min[l] > capacity.NRounds - treeSearchLevel_ - 1 + count_.fieldType[m][l])
                check = false;
            else if (limits.max[l] < count_.fieldType[m][l])
            {
                if constexpr(maxFieldTypeJump)
                {
                    checkMax_[l][m] = true;
                    ftJump_ = true;
                }
                check = false;
            }
        }
    }
    if (maxFieldTypeJump && ftJump_)
        calcFieldTypeJump();
    return check;
}


smfigapl::CheckResult smfigapl::CheckGamePlan::failed(uint& counter, const char* tag)
{
    if (input_.dbg.has(DebugFlag::cond))
        input_.dbg.out(DebugFlag::cond) << tag;
    counter++;
    if (treeSearchState_.Add >= 0 && !NFailedArray_.searchIndexFalse.empty())
        NFailedArray_.searchIndexFalse[treeSearchState_.Add] = true;
    return res_;
}

// The counting checks: necessary conditions for the rounds not chosen yet,
// from the counters of the chosen ones and the fewest/most teams per
// candidate on every field type and pausing. A completion of the chosen
// rounds that passes the checks of the last round passes them as well, so
// they only cut subtrees without a game plan: the search finds the same
// first plan in fewer steps. With no round chosen (impossibleReason) they
// find configurations without any game plan.
bool smfigapl::CheckGamePlan::countingCheck(uint rem, std::optional<Exception>* reason) const
{
    if (rem == 0)
        return true;
    return countingCheckPauses(rem, reason) && countingCheckTeams(rem, reason) && countingCheckFieldTypes(rem, reason);
}

smfigapl::uint smfigapl::CheckGamePlan::pausesLeft(uint team, uint rem) const
{
    if (condTables_.bounds.maxPauses == 0)
        return 0;
    if (!input_.conditions.maxPauseGames.enabled)
        return rem;
    return std::min(rem, minus(input_.conditions.maxPauseGames.value, count_.pause[team]));
}

std::string smfigapl::CheckGamePlan::teamName(uint team) const
{
    return input_.setup.namedTeamByTeamId[condTables_.teamsMappingArray[atomicIndex_][team]].getTeamName();
}

// every remaining round has at least minPauses pause teams
bool smfigapl::CheckGamePlan::countingCheckPauses(uint rem, std::optional<Exception>* reason) const
{
    const uint needed = rem * condTables_.bounds.minPauses;
    if (!input_.conditions.maxPauseGames.enabled || needed == 0)
        return true;
    uint left = 0;
    for (uint m = 0; m < condTables_.atomicCapacityArray[atomicIndex_].NTeams; m++)
        left += pausesLeft(m, rem);
    if (left >= needed)
        return true;
    if (reason != nullptr)
        *reason = Exception(ErrorCode::impossiblePauses, {std::to_string(needed), std::to_string(left)});
    return false;
}

// every team plays all remaining rounds except the pauses it has left: on
// the field types within their limits, and against opponents it has not
// met too often yet (within the club limit). Together the teams fill the
// places every round has to use.
bool smfigapl::CheckGamePlan::countingCheckTeams(uint rem, std::optional<Exception>* reason) const
{
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const uint chosen = capacity.NRounds - rem;
    const bool club = input_.conditions.maxGamesSameClub.enabled;
    uint gamesAll = 0;
    for (uint m = 0; m < capacity.NTeams; m++)
    {
        const uint gamesMin = rem - pausesLeft(m, rem);
        uint gamesMax = rem;
        if (input_.conditions.limGamesFieldType.enabled && !countingCheckTeamFieldTypes(m, rem, gamesMin, gamesMax, reason))
            return false;
        const uint games = minus(chosen, count_.pause[m]);
        uint possible = minus((capacity.NTeams - 1) * pairingLimit_, games);
        if (club)
        {
            const uint sameClub = count_.sameClub[m];
            const uint clubLeft = std::min(minus(input_.conditions.maxGamesSameClub.values[m], sameClub), minus(clubMates_[m] * pairingLimit_, sameClub));
            possible = clubLeft + minus((capacity.NTeams - 1 - clubMates_[m]) * pairingLimit_, games - sameClub);
        }
        if (gamesMin > possible)
        {
            if (reason != nullptr)
                *reason = Exception(club ? ErrorCode::impossibleSameClub : ErrorCode::impossibleOpponents, {teamName(m), std::to_string(gamesMin), std::to_string(possible)});
            return false;
        }
        gamesAll += std::min(gamesMax, possible);
    }
    uint places = 0;
    for (const uint slots : condTables_.bounds.minSlots)
        places += rem * slots;
    if (gamesAll >= places)
        return true;
    if (reason != nullptr)
        *reason = Exception(ErrorCode::impossibleGames, {std::to_string(places), std::to_string(gamesAll)});
    return false;
}

bool smfigapl::CheckGamePlan::countingCheckTeamFieldTypes(uint team, uint rem, uint gamesMin, uint& gamesMax, std::optional<Exception>* reason) const
{
    const auto& limits = limits_;
    uint need = 0;
    uint possible = 0;
    for (uint l = 0; l < condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes; l++)
    {
        need += minus(limits.min[l], count_.fieldType[team][l]);
        possible += std::min(minus(limits.max[l], count_.fieldType[team][l]), rem);
    }
    if (need > rem)
    {
        if (reason != nullptr)
            *reason = Exception(ErrorCode::impossibleTeamFieldTypeMin, {teamName(team), std::to_string(need), std::to_string(rem)});
        return false;
    }
    if (possible < gamesMin)
    {
        if (reason != nullptr)
            *reason = Exception(ErrorCode::impossibleTeamGames, {teamName(team), std::to_string(gamesMin), std::to_string(possible)});
        return false;
    }
    gamesMax = std::min(gamesMax, possible);
    return true;
}

// the places of a field type in the remaining rounds hold the missing
// minimums of all teams, and the teams can fill the places that have to be used
bool smfigapl::CheckGamePlan::countingCheckFieldTypes(uint rem, std::optional<Exception>* reason) const
{
    if (!input_.conditions.limGamesFieldType.enabled)
        return true;
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const auto& limits = limits_;
    for (uint l = 0; l < capacity.NFieldTypes; l++)
    {
        uint need = 0;
        uint possible = 0;
        for (uint m = 0; m < capacity.NTeams; m++)
        {
            need += minus(limits.min[l], count_.fieldType[m][l]);
            possible += std::min(minus(limits.max[l], count_.fieldType[m][l]), rem);
        }
        const std::string name = l < input_.setup.fieldTypeNameById.size() ? input_.setup.fieldTypeNameById[l] : std::to_string(l);
        if (need > rem * condTables_.bounds.maxSlots[l])
        {
            if (reason != nullptr)
                *reason = Exception(ErrorCode::impossibleFieldTypeMin, {name, std::to_string(rem* condTables_.bounds.maxSlots[l]), std::to_string(need)});
            return false;
        }
        if (possible < rem * condTables_.bounds.minSlots[l])
        {
            if (reason != nullptr)
                *reason = Exception(ErrorCode::impossibleFieldTypeMax, {name, std::to_string(rem* condTables_.bounds.minSlots[l]), std::to_string(possible)});
            return false;
        }
    }
    return true;
}

// One round left: is there a round in which every team plays or pauses
// within its final limits? Pairs of teams that may still meet (pairing and
// club limit) on a field type both may still use (all their field type
// limits kept), at most the places of every field type; the other teams
// pause (pause limit, as many pause teams as the candidates have). A
// necessary condition for a last round that passes the checks (fields,
// subtypes and empty fields are not considered), so it only cuts subtrees
// without a game plan. Depth-first over the teams, failed states are
// remembered.
bool smfigapl::CheckGamePlan::lastRoundCheck()
{
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    constexpr uint maxTeams = 16;
    if (capacity.NTeams > maxTeams || (input_.conditions.limGamesFieldType.enabled && capacity.NFieldTypes > LastRound::maxTypes))
        return true;
    lastRoundInit();
    std::array<uint, LastRound::maxTypes> pairs{};
    return lastRoundSearch(0, 0, pairs);
}

void smfigapl::CheckGamePlan::lastRoundInit()
{
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    const bool fieldTypes = input_.conditions.limGamesFieldType.enabled;
    LastRound& last = lastRound_;
    // without field type limits all field types are one
    last.NTypes = fieldTypes ? capacity.NFieldTypes : 1;
    last.minSlots.fill(0);
    last.maxSlots.fill(0);
    for (uint l = 0; l < capacity.NFieldTypes; l++)
    {
        last.minSlots[fieldTypes ? l : 0] += condTables_.bounds.minSlots[l];
        last.maxSlots[fieldTypes ? l : 0] += condTables_.bounds.maxSlots[l];
    }
    last.canPause.assign(capacity.NTeams, false);
    last.typeMask.assign(capacity.NTeams, 0);
    last.clubOk.assign(capacity.NTeams, true);
    for (uint m = 0; m < capacity.NTeams; m++)
    {
        last.typeMask[m] = fieldTypes ? lastRoundTypes(m) : 1;
        // pausing keeps the field type counts: all minimums reached
        bool complete = true;
        for (uint l = 0; fieldTypes && l < capacity.NFieldTypes; l++)
            complete = complete && count_.fieldType[m][l] >= limits_.min[l];
        last.canPause[m] = condTables_.bounds.maxPauses > 0 && pausesLeft(m, 1) > 0 && complete;
        last.clubOk[m] = !input_.conditions.maxGamesSameClub.enabled || count_.sameClub[m] < input_.conditions.maxGamesSameClub.values[m];
    }
    last.failed.clear();
}

smfigapl::uint smfigapl::CheckGamePlan::lastRoundTypes(uint team) const
{
    const auto& limits = limits_;
    const uint NFieldTypes = condTables_.atomicCapacityArray[atomicIndex_].NFieldTypes;
    uint missing = 0;
    uint missingType = 0;
    for (uint l = 0; l < NFieldTypes; l++)
        if (count_.fieldType[team][l] < limits.min[l])
        {
            missing += limits.min[l] - count_.fieldType[team][l];
            missingType = l;
        }
    // one game left: at most one game missing, then only on its field type
    if (missing > 1)
        return 0;
    uint types = 0;
    for (uint l = 0; l < NFieldTypes; l++)
        if ((missing == 0 || l == missingType) && count_.fieldType[team][l] < limits.max[l])
            types |= 1U << l;
    return types;
}

bool smfigapl::CheckGamePlan::lastRoundPairAllowed(uint a, uint b) const
{
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex_];
    if (count_.pairing[getPairingIndexShort(capacity.NTeams, a, b)] >= pairingLimit_)
        return false;
    const Mapping& mapping = condTables_.atomicMappingArray[atomicIndex_];
    if (mapping.indexedClubByTeamId[a] == mapping.indexedClubByTeamId[b] && !(lastRound_.clubOk[a] && lastRound_.clubOk[b]))
        return false;
    return (lastRound_.typeMask[a] & lastRound_.typeMask[b]) != 0;
}

bool smfigapl::CheckGamePlan::lastRoundSearch(uint mask, uint pauses, std::array<uint, LastRound::maxTypes>& pairs)
{
    const uint NTeams = condTables_.atomicCapacityArray[atomicIndex_].NTeams;
    LastRound& last = lastRound_;
    if (mask == (1U << NTeams) - 1)
    {
        if (pauses < condTables_.bounds.minPauses)
            return false;
        for (uint l = 0; l < last.NTypes; l++)
            if (2 * pairs[l] < last.minSlots[l])
                return false;
        return true;
    }
    // 16 bits teams, 5 bits pauses, 4 bits per field type
    std::uint64_t key = mask | (std::uint64_t{pauses} << 16U);
    for (uint l = 0; l < LastRound::maxTypes; l++)
        key |= std::uint64_t{pairs[l]} << (21U + 4 * l);
    if (last.failed.contains(key))
        return false;
    const auto a = static_cast<uint>(std::countr_one(mask));
    const uint next = mask | (1U << a);
    if (last.canPause[a] && pauses < condTables_.bounds.maxPauses && lastRoundSearch(next, pauses + 1, pairs))
        return true;
    for (uint b = a + 1; b < NTeams; b++)
    {
        if ((mask & (1U << b)) != 0 || !lastRoundPairAllowed(a, b))
            continue;
        const uint types = last.typeMask[a] & last.typeMask[b];
        for (uint l = 0; l < last.NTypes; l++)
        {
            if ((types & (1U << l)) == 0 || 2 * (pairs[l] + 1) > last.maxSlots[l])
                continue;
            pairs[l]++;
            const bool found = lastRoundSearch(next | (1U << b), pauses, pairs);
            pairs[l]--;
            if (found)
                return true;
        }
    }
    last.failed.insert(key);
    return false;
}

std::optional<smfigapl::Exception> smfigapl::CheckGamePlan::impossibleReason() const
{
    std::optional<Exception> reason;
    if (countingCheck(condTables_.atomicCapacityArray[atomicIndex_].NRounds, &reason))
        return std::nullopt;
    return reason;
}

smfigapl::CheckResult smfigapl::CheckGamePlan::gamePlanCheck(bool init)
{
    res_.jump = 1;
    res_.check = false;
    this->updateCounters(init);
    // the first round is the first of its kind (symmetry of the teams)
    if (treeSearchLevel_ == 0 && treeSearchState_.Add > 0 && !symmetryFirst(static_cast<uint>(treeSearchState_.Add)))
    {
        const auto add = static_cast<uint>(treeSearchState_.Add);
        res_.jump = symmetryNext(add) - add;
        return failed(NFailedArray_.symmetry, "sym ");
    }

    if (!gamePlanCheckRepeatedGames(init))
        return failed(NFailedArray_.repeatedGames, "rgF ");
    if (input_.conditions.maxGamesSameClub.enabled && !gamePlanCheckMaxGamesSameClub(init))
        return failed(NFailedArray_.sameClub, "scF ");
    if (input_.conditions.maxPauseGames.enabled && !gamePlanCheckMaxPauseGames())
        return failed(NFailedArray_.pauseGames, "pgF ");
    if (input_.conditions.limGamesFieldType.enabled && !gamePlanCheckFieldTypes())
        return failed(NFailedArray_.fieldTypes, "ft ");
    const uint rem = condTables_.atomicCapacityArray[atomicIndex_].NRounds - treeSearchLevel_ - 1;
    if (!countingCheck(rem, nullptr))
        return failed(NFailedArray_.counting, "cnt ");
    if (rem == 1 && !lastRoundCheck())
        return failed(NFailedArray_.lastRound, "lr ");

    if (input_.dbg.has(DebugFlag::cond))
        input_.dbg.out(DebugFlag::cond) << "ok ";
    res_.check = true;
    if (treeSearchState_.Add >= 0 && !NFailedArray_.searchIndex.empty())
        NFailedArray_.searchIndex[treeSearchState_.Add] = false;
    return res_;
}


void smfigapl::CheckGamePlan::dispNFailArray()
{
    input_.dbg.out() << "Failed Checks:" << std::endl;
    input_.dbg.out() << "repeatedGames: " << NFailedArray_.repeatedGames << std::endl;
    input_.dbg.out() << "sameClub: " << NFailedArray_.sameClub << std::endl;
    input_.dbg.out() << "pauseGames: " << NFailedArray_.pauseGames << std::endl;
    input_.dbg.out() << "fieldTypes: " << NFailedArray_.fieldTypes << std::endl;
    input_.dbg.out() << "counting: " << NFailedArray_.counting << std::endl;
    input_.dbg.out() << "lastRound: " << NFailedArray_.lastRound << std::endl;
    input_.dbg.out() << "symmetry: " << NFailedArray_.symmetry << std::endl;
    input_.dbg.out() << "Always failed: " << std::endl;
    for (uint i = 0; i < NFailedArray_.searchIndex.size(); i++)
        if (NFailedArray_.searchIndex[i] && NFailedArray_.searchIndexFalse[i])
            input_.dbg.out() << i << " ";
    input_.dbg.out() << std::endl;
}

