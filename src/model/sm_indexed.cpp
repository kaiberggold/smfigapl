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

#include <utility>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include "smfigapl/model/sm_indexed.hpp"
#include "smfigapl/model/sm_math.hpp"




//IndexedTeam

bool smfigapl::IndexedTeam::operator==(IndexedTeam& other) const
{
    return other.getTeamId() == team_;
}

void smfigapl::IndexedTeam::setTeamClub(ClubId cl)
{
    club_ = cl;
}

void smfigapl::IndexedTeam::setTeamColor(ColorId co)
{
    color_ = co;
}

void smfigapl::IndexedTeam::setTeamId(uint teamId)
{
    team_ = teamId;
}

smfigapl::ClubId smfigapl::IndexedTeam::getTeamClub() const
{
    return club_;
}

smfigapl::ColorId smfigapl::IndexedTeam::getTeamColor() const
{
    return color_;
}

smfigapl::uint smfigapl::IndexedTeam::getTeamId() const
{
    return team_;
}

//IndexedGame

smfigapl::IndexedGame::IndexedGame(IndexedTeam a, IndexedTeam b): teams_{a, b}
{
}

//Return first (0) or second (1) team of a game
//swap if swap is set
smfigapl::IndexedTeam smfigapl::IndexedGame::getTeamByGameId(uint gameIndex)
{
    if (!swap_)
        return teams_[gameIndex];
    return teams_[1 - gameIndex];
}

void smfigapl::IndexedGame::setSwap(bool sw)
{
    swap_ = sw;
}

void smfigapl::IndexedGame::flipSwap()
{
    swap_ = !swap_;
}

bool smfigapl::IndexedGame::getSwap() const
{
    return swap_;
}
//Return first (0) or second (1) team of a game
bool smfigapl::IndexedGame::gameCompare(IndexedGame b)
{
    return (teams_[0].getTeamId() == b.getTeamByGameId(0).getTeamId() && teams_[1].getTeamId() == b.getTeamByGameId(1).getTeamId()) || \
           (teams_[0].getTeamId() == b.getTeamByGameId(1).getTeamId() && teams_[1].getTeamId() == b.getTeamByGameId(0).getTeamId());
}

// Return the Game
smfigapl::IndexedGame smfigapl::IndexedGame::getGame()
{
    return *this;
}

void smfigapl::IndexedGame::setPause(bool p)
{
    pause_ = p;
}

bool smfigapl::IndexedGame::getPause() const
{
    return pause_;
}

// Set the Game
void smfigapl::IndexedGame::setTeam(uint gameIndex, IndexedTeam team)
{
    teams_[gameIndex] = team;
}

// contains a team
bool smfigapl::IndexedGame::contains(uint team)
{
    return teams_[0].getTeamId() == team || teams_[1].getTeamId() == team;
}

//indexedRound_
smfigapl::IndexedGame smfigapl::IndexedRound::getGame(uint field)
{
    return roundArray_.at(field);
}

void smfigapl::IndexedRound::setGame(uint field, IndexedGame game)
{
    roundArray_[field] = game;
}

smfigapl::uint smfigapl::IndexedRound::getNPauseTeams()
{
    return static_cast<uint>(pauseTeams_.size());
}

smfigapl::uint smfigapl::IndexedRound::size()
{
    return static_cast<uint>(roundArray_.size());
}


smfigapl::IndexedTeam smfigapl::IndexedRound::getPauseTeam(uint pauseIndex)
{
    return pauseTeams_[pauseIndex];
}

std::vector<smfigapl::IndexedTeam> smfigapl::IndexedRound::getPauseTeams()
{
    return pauseTeams_;
}


bool smfigapl::IndexedRound::isPauseTeam(uint team)
{
    if (static_cast<uint>(pauseTeams_.size()) > 0)
        for (auto&& t : pauseTeams_)
            if (t.getTeamId() == team)
                return true;
    return false;
}

void smfigapl::IndexedRound::addPauseTeam(IndexedTeam team)
{
    pauseTeams_.push_back(team);
}
void smfigapl::IndexedRound::pauseTeamsClear()
{
    pauseTeams_.clear();
}

bool smfigapl::compare(smfigapl::IndexedGame& l, smfigapl::IndexedGame& r)   //(2)
{
    return l.getTeamByGameId(0).getTeamId() < r.getTeamByGameId(0).getTeamId();
}

void smfigapl::IndexedRound::sort()
{
    std::ranges::sort(roundArray_, smfigapl::compare);
}

bool smfigapl::IndexedRound::contains(IndexedGame game)
{
    for (uint j = 0; j < NFields_; j++)
    {
        if (game.gameCompare(roundArray_[j]) && !roundArray_[j].getPause())
            return true;
    }
    return false;
}

bool smfigapl::IndexedRound::getHomeAwayState(uint team, bool home)
{
    for (uint j = 0; j < NFields_; j++)
    {
        if (!roundArray_[j].getPause())
        {
            if (roundArray_[j].getTeamByGameId(static_cast<uint>(!home)).getTeamId() == team)
            {
                return true;
            }
        }
    }
    return false;
}

bool smfigapl::IndexedRound::isHome(uint team)
{
    return getHomeAwayState(team, true);
}

bool smfigapl::IndexedRound::isAway(uint team)
{
    return getHomeAwayState(team, false);
}

bool smfigapl::IndexedRound::isHomeAway(uint team, bool home)
{
    return getHomeAwayState(team, home);
}

void smfigapl::IndexedRound::init(uint n)
{
    NFields_ = n;
    roundArray_.resize(NFields_);
    pauseTeams_.clear();
}

void smfigapl::IndexedRound::flipSwap(uint field)
{
    roundArray_[field].flipSwap();
}

void smfigapl::IndexedRound::setIndex(uint i)
{
    index_ = i;
}
smfigapl::uint  smfigapl::IndexedRound::getIndex() const
{
    return index_;
}

smfigapl::uint smfigapl::IndexedRound::getFieldIdByTeamId(smfigapl::uint team)
{
    uint res = 255;
    if (this->isPauseTeam(team))
        return res;
    for (uint j = 0; j < NFields_; j++)
        if (roundArray_[j].contains(team))
            res = j;
    return res;
}

//IndexedGamePlan

smfigapl::IndexedGame smfigapl::IndexedGamePlan::getGame(uint round, uint field)
{
    return gamePlan_[resortIndex_[atomicIndexArray_[field]][round]] [field];
}

smfigapl::IndexedGame smfigapl::IndexedGamePlan::getGameAtomic(uint round, uint field, uint atomicIndex)
{
    return gamePlan_[resortIndex_[atomicIndexArray_[atomicFieldMappingArray_[atomicIndex][field]]][round]] [atomicFieldMappingArray_[atomicIndex][field]];
}

void smfigapl::IndexedGamePlan::reset()
{
    NRounds_ = 0;
    gamePlan_.resize(0);
    fieldTypeByFieldId_.resize(0);
}

void smfigapl::IndexedGamePlan::addPauseTeamAtomic(uint round, IndexedTeam team, uint atomicIndex)
{
    pauseTeams_[round].push_back(team);
    pauseTeamsAtomicIndex_[round].push_back(atomicIndex);
}

void smfigapl::IndexedGamePlan::pauseTeamsClear(uint round)
{
    pauseTeams_[round].clear();
    pauseTeamsAtomicIndex_[round].clear();
}

void smfigapl::IndexedGamePlan::setGame(uint round, uint field, IndexedGame game)
{
    gamePlan_[round][field] = game;
}

smfigapl::uint smfigapl::IndexedGamePlan::size() const
{
    return NRounds_;
}

void smfigapl::IndexedGamePlan::setFieldTypeByFieldId(std::vector<uint> f)
{
    fieldTypeByFieldId_ = std::move(f);
}

void smfigapl::IndexedGamePlan::init(uint nr, uint nf, uint na)
{
    NRounds_ = nr;
    NFields_ = nf;
    NAtomic_ = na;
    pauseTeams_.clear();
    pauseTeams_.resize(NRounds_);
    pauseTeamsAtomicIndex_.clear();
    pauseTeamsAtomicIndex_.resize(NRounds_);
    smfigapl::IndexedGame game;
    std::vector<smfigapl::IndexedGame> gameRow;
    gameRow.resize(NFields_, game);
    gamePlan_.clear();
    gamePlan_.resize(NRounds_, gameRow);
    std::vector<uint> identity(NRounds_);
    std::iota(identity.begin(), identity.end(), 0U);
    resortIndex_.assign(NAtomic_, identity);
    atomicIndexArray_.assign(nf, 0);
}

smfigapl::uint smfigapl::IndexedGamePlan::getFieldTypeByFieldId(uint field)
{
    return fieldTypeByFieldId_[field];
}

void smfigapl::IndexedGamePlan::setAtomicFieldMappingArray(std::vector<std::vector<uint>> m)
{
    atomicFieldMappingArray_ = std::move(m);
    // the fields of every atomic set (it used the number of fields of the first
    // set for all sets: overflow if a set has fewer fields, missing fields if more)
    for (uint l = 0; l < NAtomic_; l++)
        for (uint field : atomicFieldMappingArray_[l])
            atomicIndexArray_[field] = l;
}

void smfigapl::IndexedGamePlan::flipSwap(uint round, uint field)
{
    gamePlan_[round][field].flipSwap();
}

void smfigapl::IndexedGamePlan::flipSwapAtomic(uint round, uint field, uint atomicIndex)
{
    gamePlan_[round][atomicFieldMappingArray_[atomicIndex][field]].flipSwap();
}

bool smfigapl::IndexedGamePlan::getHomeAwayState(uint round, uint team, bool home)
{
    for (uint j = 0; j < NFields_; j++)
    {
        if (!this->getGame(round, j).getPause())
        {
            uint gameIdx = 0;
            if (home)
            {
                gameIdx = 0;
            }
            else
            {
                gameIdx = 1;
            }
            IndexedTeam idxTeam = this->getGame(round, j).getTeamByGameId(gameIdx);
            if (idxTeam.getTeamId() == team)
            {
                return true;
            }
        }
    }
    return false;
}


bool smfigapl::IndexedGamePlan::isHome(uint round, uint team)
{
    return this->getHomeAwayState(round, team, true);
}

bool smfigapl::IndexedGamePlan::isAway(uint round, uint team)
{
    return this->getHomeAwayState(round, team, false);
}

bool smfigapl::IndexedGamePlan::getHomeAwayStateAtomic(uint round, uint team, bool home, uint atomicIndex)
{
    for (uint j = 0; j < NFields_; j++)
    {
        if (atomicIndexArray_[j] == atomicIndex)
        {
            if (!this->getGame(round, j).getPause())
            {
                uint gameIdx = 0;
                if (home)
                {
                    gameIdx = 0;
                }
                else
                {
                    gameIdx = 1;
                }
                IndexedTeam idxTeam = this->getGame(round, j).getTeamByGameId(gameIdx);
                if (idxTeam.getTeamId() == team)
                {
                    return true;
                }
            }
        }
    }
    return false;
}


bool smfigapl::IndexedGamePlan::isHomeAtomic(uint round, uint team, uint atomicIndex)
{
    return this->getHomeAwayStateAtomic(round, team, true, atomicIndex);
}

bool smfigapl::IndexedGamePlan::isAwayAtomic(uint round, uint team, uint atomicIndex)
{
    return this->getHomeAwayStateAtomic(round, team, false, atomicIndex);
}

// The pause teams of round `round` (as shown) of every atomic set come from
// the stored round resortIndex_[set][round], like the games (getGame). They
// used the inverse permutation or none, which showed the pause teams of
// another round and read past the end when the rounds have different
// numbers of pause teams.
std::vector<smfigapl::IndexedTeam> smfigapl::IndexedGamePlan::pauseTeamsAtomic(uint round, uint atomicIndex) const
{
    const uint stored = resortIndex_[atomicIndex][round];
    std::vector<IndexedTeam> teams;
    for (size_t k = 0; k < pauseTeams_[stored].size(); k++)
        if (pauseTeamsAtomicIndex_[stored][k] == atomicIndex)
            teams.push_back(pauseTeams_[stored][k]);
    return teams;
}

bool smfigapl::IndexedGamePlan::isPauseTeam(uint round, uint team) const
{
    for (uint l = 0; l < NAtomic_; l++)
        for (const IndexedTeam& t : pauseTeamsAtomic(round, l))
            if (t.getTeamId() == team)
                return true;
    return false;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNPauseTeams(uint round) const
{
    uint n = 0;
    for (uint l = 0; l < NAtomic_; l++)
        n += getNPauseTeamsAtomic(round, l);
    return n;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNPauseTeamsAtomic(uint round, uint atomicIndex) const
{
    return static_cast<uint>(pauseTeamsAtomic(round, atomicIndex).size());
}

smfigapl::IndexedTeam smfigapl::IndexedGamePlan::getPauseTeam(uint round, uint pauseIndex) const
{
    // the pause teams of all atomic sets, set by set
    std::vector<IndexedTeam> teams;
    for (uint l = 0; l < NAtomic_; l++)
        for (const IndexedTeam& t : pauseTeamsAtomic(round, l))
            teams.push_back(t);
    return teams.at(pauseIndex);
}

bool smfigapl::IndexedGamePlan::getPause(uint round, uint field)
{
    return this->getGame(round, field).getPause();
}

void smfigapl::IndexedGamePlan::setResortOrder(const std::vector<uint>& order, uint atomicIndex)
{
    // the atomic set may have fewer rounds than the game plan
    for (size_t i = 0; i < order.size(); i++)
        resortIndex_[atomicIndex].at(i) = order[i];
}

smfigapl::uint smfigapl::IndexedGamePlan::getResortIndex(uint round, uint atomicIndex)
{
    return resortIndex_[atomicIndex][round];
}

smfigapl::uint smfigapl::IndexedGamePlan::getAtomicIndex(uint field)
{
    return atomicIndexArray_[field];
}

smfigapl::uint smfigapl::IndexedGamePlan::getNAtomic() const
{
    return NAtomic_;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNRounds() const
{
    return NRounds_;
}
