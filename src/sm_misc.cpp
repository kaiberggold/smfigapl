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

#include <vector>
#include <string>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include "../include/sm_misc.hpp"

smfigapl::uint smfigapl::factorial(uint n)
{
	static const std::vector<uint>fac={1,1,2,6,24,120,720,5040,40320,362880};
	if (n<10)
		return fac[n];
	else
		return factorial(n - 1) * n;
}

smfigapl::uint smfigapl::getPairingIndexShort(uint NTeams, uint a, uint b)
{
    return a*(2*NTeams-1-a)/2+b-a-1;
}

void smfigapl::displayPairingIndicesShort(uint NTeams)
{
	uint i=0;
	std::cout<<"PairingIndixesShort:, NTeams: "<<NTeams<<std::endl;
	for (uint a=0;a<NTeams;a++)
		for (uint b=a+1;b<NTeams;b++)
		{
			std::cout<<"i: "<<i<<" ab: "<<a<<b<<" "<<std::endl;
			i++;
		}
	std::cout<<std::endl;
}

smfigapl::uint smfigapl::getNPairingIndexShort(uint NTeams)
{
    return (NTeams-1)*NTeams/2;
}

bool smfigapl::fieldCompare(smfigapl::fieldSortable & l, smfigapl::fieldSortable & r)
{
    return (l.fieldType<r.fieldType) || (l.fieldType==r.fieldType &&l.fieldSubType <r.fieldSubType);
}

void smfigapl::fieldSort(std::vector<fieldSortable> *fs)
{
	std::sort( fs->begin(), fs->end(), smfigapl::fieldCompare);
}

//ResultArray
bool smfigapl::ResultArray::getResult()
{
  for (auto && i:resultArray_)
	  if (i.stepResult!=TS_SUCCESS)
		  return false;
  return true;
}

smfigapl::uint smfigapl::ResultArray::getResult(uint index)
{
	return resultArray_[index].stepResult;
}

void smfigapl::ResultArray::setResult(uint index, uint res)
{
	resultArray_[index].stepResult=res;
}

void smfigapl::ResultArray::incStep(uint  index)
{
	(resultArray_[index].steps)++;
}

smfigapl::uint smfigapl::ResultArray::getStep(uint  index)
{
	return resultArray_[index].steps;
}

smfigapl::uint smfigapl::ResultArray::getTotalSteps()
{
	uint total=0;
	for (auto  && i:resultArray_)
		total+=i.steps;
	return total;
}


void smfigapl::ResultArray::init(uint size)
{
	resultArray_.resize(size);
}



//GamePlanRoundByTeamId

void smfigapl::GamePlanRoundByTeamId::init(uint NFields)
{
	gamePlanRoundByTeamId_.resize(NFields<<1);
}
void smfigapl::GamePlanRoundByTeamId::reset()
{
	std::fill(gamePlanRoundByTeamId_.begin(), gamePlanRoundByTeamId_.end(), 0);
}

smfigapl::uint smfigapl::GamePlanRoundByTeamId::getTeamId(uint index)
{
	return gamePlanRoundByTeamId_[index];
}
smfigapl::uint smfigapl::GamePlanRoundByTeamId::getTeamId(uint field, uint gameIndex)
{
	return gamePlanRoundByTeamId_[(field<<1)+gameIndex];
}
void smfigapl::GamePlanRoundByTeamId::setTeamId(uint index,uint teamId)
{
	gamePlanRoundByTeamId_[index]=teamId;
}
void smfigapl::GamePlanRoundByTeamId::setTeamId(uint field,uint gameIndex,uint teamId)
{
	smfigapl::GamePlanRoundByTeamId::gamePlanRoundByTeamId_[(field<<1)+gameIndex]=teamId;
}
smfigapl::uint smfigapl::GamePlanRoundByTeamId::size()
{
	return static_cast<uint>(gamePlanRoundByTeamId_.size());
}

//GamePlanByTeamId

smfigapl::uint smfigapl::GamePlanByTeamId::getTeamId(uint round,uint index)
{
	return gamePlanByTeamId_[round].getTeamId(index);
}
smfigapl::uint smfigapl::GamePlanByTeamId::getTeamId(uint round,uint field, uint gameIndex)
{
	return gamePlanByTeamId_[round].getTeamId(field,gameIndex);
}
void smfigapl::GamePlanByTeamId::setTeamId(uint round,uint index,uint teamId)
{
	gamePlanByTeamId_[round].setTeamId(index,teamId);
}
void smfigapl::GamePlanByTeamId::setTeamId(uint round,uint field,uint gameIndex,uint teamId)
{
	gamePlanByTeamId_[round].setTeamId(field,gameIndex,teamId);
}
smfigapl::GamePlanRoundByTeamId smfigapl::GamePlanByTeamId::getRound(uint round)
{
	return gamePlanByTeamId_[round];
}




//IndexedTeam

bool smfigapl::IndexedTeam::operator==(IndexedTeam &other)
		{
	if (other.getTeamId() == Team_ )
		return true;
	else
		return false;
		}

void smfigapl::IndexedTeam::setTeamClub(ClubId cl)
{
	Club_ = cl;
}

void smfigapl::IndexedTeam::setTeamColor(ColorId co)
{
	Color_ = co;
}

void smfigapl::IndexedTeam::setTeamId(uint teamId)
{
	Team_ = teamId;
}

smfigapl::ClubId smfigapl::IndexedTeam::getTeamClub()
{
 	return Club_;
}

smfigapl::ColorId smfigapl::IndexedTeam::getTeamColor()
{
 	return Color_;
}

smfigapl::uint smfigapl::IndexedTeam::getTeamId()
{
 	return Team_;
}

//IndexedGame

smfigapl::IndexedGame::IndexedGame()
{
	IndexedTeam temp;
	temp.setTeamId(0);
	temp.setTeamClub(0);
	temp.setTeamColor(0);
	teams_[0] = temp;
	teams_[1] = temp;
	Pause_=false;
	swap_=false;
}

smfigapl::IndexedGame::IndexedGame(IndexedTeam a, IndexedTeam b)
{
	teams_[0] = a;
	teams_[1] = b;
	Pause_=false;
	swap_=false;
}

//Return first (0) or second (1) team of a game
//swap if swap is set
smfigapl::IndexedTeam smfigapl::IndexedGame::getTeambyGameId(uint gameIndex)
{
	if (!swap_)
		return teams_[gameIndex];
	else
		return teams_[1-gameIndex];
}

void smfigapl::IndexedGame::setSwap(bool sw)
{
	swap_=sw;
}

void smfigapl::IndexedGame::flipSwap()
{
	swap_=!swap_;
}

bool smfigapl::IndexedGame::getSwap()
{
	return swap_;
}
//Return first (0) or second (1) team of a game
bool smfigapl::IndexedGame::gameCompare(IndexedGame b)
{
	if ((teams_[0].getTeamId() == b.getTeambyGameId(0).getTeamId() && teams_[1].getTeamId() == b.getTeambyGameId(1).getTeamId()) ||\
		(teams_[0].getTeamId() == b.getTeambyGameId(1).getTeamId() && teams_[1].getTeamId() == b.getTeambyGameId(0).getTeamId())	)
		return true;
	else
		return false;
}

// Return the Game
smfigapl::IndexedGame smfigapl::IndexedGame::getGame()
{
	return *this;
}

void smfigapl::IndexedGame::setPause (bool p)
{
	Pause_=p;
}

bool smfigapl::IndexedGame::getPause()
{
	return Pause_;
}

// Set the Game
void smfigapl::IndexedGame::setTeam(uint gameIndex, IndexedTeam team)
{
	teams_[gameIndex]=team;
}

// contains a team
bool smfigapl::IndexedGame::contains(uint team)
{
	if (teams_[0].getTeamId()==team||teams_[1].getTeamId()==team)
		return true;
	else
		return false;

}

//indexedRound_
smfigapl::IndexedGame smfigapl::IndexedRound::getGame(uint field)
{
	if (field>=roundArray_.size())
	{
		std::cerr<<"get game index overflow field: "<<field<<std::endl;
	}
	return roundArray_[field];

}

void smfigapl::IndexedRound::setGame(uint field,IndexedGame game)
{
 	roundArray_[field]=game;
}

smfigapl::uint smfigapl::IndexedRound::getNPauseTeams()
{
	return static_cast<uint>(pauseTeams_.size());
}

smfigapl::uint smfigapl::IndexedRound::size()
{
	return static_cast<uint>(roundArray_.size());
}


smfigapl::IndexedTeam smfigapl::IndexedRound::getPauseTeam (uint pauseIndex)
{
	return pauseTeams_[pauseIndex];
}

std::vector<smfigapl::IndexedTeam> smfigapl::IndexedRound::getPauseTeams ()
{
	return pauseTeams_;
}


bool smfigapl::IndexedRound::isPauseTeam (uint team)
{
	if (static_cast<uint>(pauseTeams_.size())>0)
		for (auto && t:pauseTeams_)
			if (t.getTeamId()==team)
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

bool smfigapl::compare(smfigapl::IndexedGame & l, smfigapl::IndexedGame & r) //(2)
{
    return l.getTeambyGameId(0).getTeamId() < r.getTeambyGameId(0).getTeamId();
}

void smfigapl::IndexedRound::sort()
{
	std::sort( roundArray_.begin(), roundArray_.end(), smfigapl::compare);
}

bool smfigapl::IndexedRound::contains(IndexedGame game)
{
	for (uint j=0;j<NFields_;j++)
	{
		if (game.gameCompare(roundArray_[j])&&!roundArray_[j].getPause())
			return true;
	}
	return false;
}

bool smfigapl::IndexedRound::getHomeAwayState(uint team, bool home)
{
	for (uint j=0;j<NFields_;j++)
	{
		if (!roundArray_[j].getPause())
		{
			if (roundArray_[j].getTeambyGameId(static_cast<uint>(!home)).getTeamId()==team)
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

bool smfigapl::IndexedRound::isHomeAway(uint team,bool home)
{
	return getHomeAwayState(team, home);
}

void smfigapl::IndexedRound::init(uint n)
{
	NFields_=n;
	roundArray_.resize(NFields_);
	pauseTeams_.clear();
}

void smfigapl::IndexedRound::flipSwap(uint field)
{
	roundArray_[field].flipSwap();
}

void smfigapl::IndexedRound::setIndex(uint i)
{
	index_=i;
}
smfigapl::uint  smfigapl::IndexedRound::getIndex()
{
	return index_;
}

smfigapl::uint smfigapl::IndexedRound::getFieldIdbyTeamId(smfigapl::uint team)
{
	uint res=255;
	if (this->isPauseTeam(team))
		return res;
	for (uint j=0;j<NFields_;j++)
		if (roundArray_[j].contains(team))
			res= j;
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
	NRounds_=0;
	gamePlan_.resize(0);
	fieldTypeByFieldId_.resize(0);

}

smfigapl::GamePlanRoundByTeamId smfigapl::roundFieldMix(GamePlanRoundByTeamId roundArray_,std::vector<uint> mixArray)
{
	GamePlanRoundByTeamId temp(roundArray_.size()>>1);
	for (uint i = 0; i < mixArray.size(); i++)
	{
		for(uint k=0;k<2;k++)
			temp.setTeamId(i,k,roundArray_.getTeamId(mixArray[i],k));
	}

	return temp;
}

void smfigapl::IndexedGamePlan::addPauseTeamAtomic(uint round,IndexedTeam team,uint atomicIndex)
{
	pauseTeams_[round].push_back(team);
	pauseTeamsAtomicIndex_[round].push_back(atomicIndex);
}

void smfigapl::IndexedGamePlan::pauseTeamsClear(uint round)
{
	pauseTeams_[round].clear();
	pauseTeamsAtomicIndex_[round].clear();
}

void smfigapl::IndexedGamePlan::setGame(uint round, uint field,IndexedGame game)
{
	gamePlan_[round][field]=game;
}

smfigapl::uint smfigapl::IndexedGamePlan::size()
{
	return NRounds_;
}

void smfigapl::IndexedGamePlan::setFieldTypeByFieldId(std::vector<uint> f)
{
	fieldTypeByFieldId_=f;
}

void smfigapl::IndexedGamePlan::init(uint nr, uint nf, uint na)
{
	NRounds_=nr;
	NFields_=nf;
	NAtomic_=na;
	pauseTeams_.clear();
	pauseTeams_.resize(NRounds_);
	pauseTeamsAtomicIndex_.clear();
	pauseTeamsAtomicIndex_.resize(NRounds_);
	smfigapl::IndexedGame game;
	std::vector<smfigapl::IndexedGame> gameRow;
	gameRow.resize(NFields_,game);
	gamePlan_.clear();
	gamePlan_.resize(NRounds_,gameRow);
	resortIndex_.resize(NAtomic_);
	std::vector<uint> temp;
	temp.resize(NRounds_);
	for (uint i=0;i<NRounds_;i++)
	{
		temp[i]=i;
	}
	for (uint l=0;l<NAtomic_;l++)
		resortIndex_[l]=temp;
	atomicIndexArray_.resize(nf);
	std::fill(atomicIndexArray_.begin(),atomicIndexArray_.end(),0);
}

smfigapl::uint smfigapl::IndexedGamePlan::getFieldTypeByFieldId(uint field)
{
	return fieldTypeByFieldId_[field];
}

void smfigapl::IndexedGamePlan::setAtomicFieldMappingArray(std::vector<std::vector<uint>> m)
{
	atomicFieldMappingArray_=m;
	for (uint l=0;l<NAtomic_;l++)
		for (uint j=0;j<atomicFieldMappingArray_[0].size();j++)
			atomicIndexArray_[atomicFieldMappingArray_[l][j]]=l;
}

void smfigapl::IndexedGamePlan::flipSwap(uint round,uint field)
{
	gamePlan_[round][field].flipSwap();
}

void smfigapl::IndexedGamePlan::flipSwapAtomic(uint round,uint field,uint atomicIndex)
{
	gamePlan_[round][atomicFieldMappingArray_[atomicIndex][field]].flipSwap();
}

bool smfigapl::IndexedGamePlan::getHomeAwayState(uint round, uint team,bool home)
{
	for (uint j=0;j<NFields_;j++)
	{
		if (!this->getGame(round, j).getPause())
		{
			uint gameIdx;
			if (home)
			{
				gameIdx=0;
			}
			else
			{
				gameIdx=1;
			}
			IndexedTeam idxTeam=this->getGame(round, j).getTeambyGameId(gameIdx);
			if (idxTeam.getTeamId()==team)
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

bool smfigapl::IndexedGamePlan::getHomeAwayStateAtomic(uint round, uint team,bool home, uint atomicIndex)
{
	for (uint j=0;j<NFields_;j++)
	{
		if (atomicIndexArray_[j]==atomicIndex)
		{
			if (!this->getGame(round, j).getPause())
			{
				uint gameIdx;
				if (home)
				{
					gameIdx=0;
				}
				else
				{
					gameIdx=1;
				}
				IndexedTeam idxTeam=this->getGame(round, j).getTeambyGameId(gameIdx);
				if (idxTeam.getTeamId()==team)
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

bool smfigapl::IndexedGamePlan::isPauseTeam(uint round, uint team)
{
	for (uint l=0;l<NAtomic_;l++)
	{
		uint roundResorted=static_cast<uint>(std::distance(resortIndex_[l].begin(),std::find(resortIndex_[l].begin(),resortIndex_[l].end(),round)));
		for (uint k=0;k<static_cast<uint>(pauseTeamsAtomicIndex_[roundResorted].size());k++)
			if (pauseTeamsAtomicIndex_[roundResorted][k]==l && pauseTeams_[roundResorted][k].getTeamId()==team)
				return true;
	}
	return false;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNPauseTeams(uint round)
{
	uint n=0;
	for (uint l=0;l<NAtomic_;l++)
	{
		n+=static_cast<uint>(std::count(pauseTeamsAtomicIndex_[round].begin(),pauseTeamsAtomicIndex_[round].end(),l));
	}
	return n;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNPauseTeamsAtomic(uint round,uint atomicIndex)
{
	return static_cast<uint>(std::count(pauseTeamsAtomicIndex_[round].begin(),pauseTeamsAtomicIndex_[round].end(),atomicIndex));
}

smfigapl::IndexedTeam smfigapl::IndexedGamePlan::getPauseTeam(uint round,uint pauseIndex)
{
	std::vector<IndexedTeam> pauseTeamsResorted;
	pauseTeamsResorted.resize(this->getNPauseTeams(round));
	for (uint l=0;l<NAtomic_;l++)
	{
		uint roundResorted=static_cast<uint>(std::distance(resortIndex_[l].begin(),std::find(resortIndex_[l].begin(),resortIndex_[l].end(),round)));
		for (uint k=0;k<pauseTeamsAtomicIndex_[roundResorted].size();k++)
			if (pauseTeamsAtomicIndex_[roundResorted][k]==l)
				pauseTeamsResorted[k]=pauseTeams_[roundResorted][k];
	}
	return pauseTeamsResorted[pauseIndex];
}

bool smfigapl::IndexedGamePlan::getPause(uint round, uint field)
{
	return this->getGame(round, field).getPause();
}

void smfigapl::IndexedGamePlan::setResortIndex(uint NRounds, uint index,uint atomicIndex)
{
	uint idx=index;
	std::vector<smfigapl::uint> temp;
	temp.resize(NRounds);
	for (uint i=0;i<NRounds;i++)
		temp[i]=i;

	for (uint i=0;i<NRounds;i++)
	{
		uint l=factorial(static_cast<uint>(temp.size())-1);
		uint resIdx=idx/l;
		resortIndex_[atomicIndex][i]=temp[resIdx];
		auto eraseIdx=temp.begin()+resIdx;
		temp.erase(eraseIdx);
		idx=idx%l;
	}
}

smfigapl::uint smfigapl::IndexedGamePlan::getResortIndex(uint round,uint atomicIndex)
{
	return resortIndex_[atomicIndex][round];
}

smfigapl::uint smfigapl::IndexedGamePlan::getAtomicIndex(uint field)
{
	return atomicIndexArray_[field];
}

smfigapl::uint smfigapl::IndexedGamePlan::getNAtomic()
{
	return NAtomic_;
}

smfigapl::uint smfigapl::IndexedGamePlan::getNRounds(uint atomicIndex)
{
	return NRounds_;
}


//Class NamedTeam

smfigapl::NamedTeam::NamedTeam()
{
		teamClub = "";
		teamColor = "";
}

bool smfigapl::NamedTeam::operator==(NamedTeam &other)
		{
	if (other.getTeamClub() == teamClub && other.getTeamColor() == teamColor)
		return true;
	else
		return false;
		}

std::string smfigapl::NamedTeam::getTeamName()
{
	return teamClub + " " + teamColor;
}

std::string smfigapl::NamedTeam::getTeamColor()
{
	return teamColor;
}

std::string smfigapl::NamedTeam::getTeamClub()
{
	return teamClub;
}

void smfigapl::NamedTeam::setTeamColor(std::string s)
{
	teamColor = s;
}

void smfigapl::NamedTeam::setTeamClub(std::string s)
{
	teamClub = s;
}


//Class NamedRound

smfigapl::NamedRound::NamedRound(uint n)
{
		NFields_ = n;
		gamePlanGames_.clear();
		NamedTeam temp;
		std::vector<NamedTeam> game;
		game.push_back(temp);
		game.push_back(temp);

		for (uint i = 0; i < NFields_; i++)
		{
			gamePlanGames_.push_back(game);
		}
		gamePlanPauseName_.clear();
		gamePlanPauseColor_.clear();
}

void smfigapl::NamedRound::addPauseTeam(std::string name, std::string color, uint atomicIndex)
{
	gamePlanPauseName_.push_back(name);
	gamePlanPauseColor_.push_back(color);
	atomicIndexPause_.push_back(atomicIndex);
}


void smfigapl::NamedRound::setTeamColor(uint i, uint j, std::string s)
{
	gamePlanGames_[i][j].setTeamColor(s);
}

void smfigapl::NamedRound::setTeamClub(uint field, uint gameIndex, std::string s)
{
	gamePlanGames_[field][gameIndex].setTeamClub(s);
}

smfigapl::NamedTeam smfigapl::NamedRound::getTeam(uint i, uint j)
{
	if (i >= NFields_)
	{
		std::cerr  << "gamePlanTeam.getGame i overflow error, NRealFields = " << NFields_ << ", i = " << i;
	}
	return gamePlanGames_[i][j];
	std::cout <<"k"<<i<<j<<std::endl;
}

std::string smfigapl::NamedRound::getTeamName(uint i, uint j)
{
	return gamePlanGames_[i][j].getTeamName();
}

std::string smfigapl::NamedRound::getPauseTeam(uint i)
{
	return gamePlanPauseName_[i]+" "+gamePlanPauseColor_[i];
}

std::string smfigapl::NamedRound::getPauseTeamColor(uint i)
{
	return gamePlanPauseColor_[i];
}


smfigapl::uint smfigapl::NamedRound::getPauseTeamAtomicIndex(uint i)
{
	return atomicIndexPause_[i];
}

smfigapl::uint smfigapl::NamedRound::getNPauseTeams()
{
	return static_cast<uint>(gamePlanPauseName_.size());
}

// Class NamedGamePlan
void  smfigapl::NamedGamePlan::addRound (smfigapl::NamedRound round)
{
	rounds_.push_back(round);
}
void  smfigapl::NamedGamePlan::setTeamClub(uint round, uint field, uint gameIndex, std::string s)
{
	rounds_[round].setTeamClub(field,gameIndex,s);
}

void  smfigapl::NamedGamePlan::setTeamColor(uint round, uint field, uint gameIndex, std::string s)
{
	rounds_[round].setTeamColor(field,gameIndex,s);
}

std::string smfigapl::NamedGamePlan::getTeamClub(uint round, uint field, uint gameIndex)
{
	return rounds_[round].getTeam(field,gameIndex).getTeamClub();
}

std::string smfigapl::NamedGamePlan::getTeamColor(uint round, uint field, uint gameIndex)
{
	return rounds_[round].getTeam(field,gameIndex).getTeamColor();
}

std::string smfigapl::NamedGamePlan::getTeamFullName(uint round, uint field, uint gameIndex)
{
	return this->getTeamClub(round,field,gameIndex)+" "+this->getTeamColor(round,field,gameIndex);
}

void smfigapl::NamedGamePlan::addPauseTeamAtomic(uint round,std::string name, std::string color, uint atomicIndex)
{
	rounds_[round].addPauseTeam(name, color, atomicIndex);
}

std::string smfigapl::NamedGamePlan::getPauseTeam(uint round, uint pauseTeamIndex)
{
	return rounds_[round].getPauseTeam(pauseTeamIndex);
}

std::string smfigapl::NamedGamePlan::getPauseTeamColor(uint round, uint pauseTeamIndex)
{
	return rounds_[round].getPauseTeamColor(pauseTeamIndex);
}

std::string smfigapl::NamedGamePlan::getPauseTeamAtomic(uint round, uint pauseTeamIndex, uint atomicIndex)
{
	uint j=0;
	for (uint i=0;i<this->getNPauseTeams(round);i++)
		if (rounds_[round].getPauseTeamAtomicIndex(i)==atomicIndex)
		{
			if(j==pauseTeamIndex)
				return this->getPauseTeam(round, j);
			else
				j++;
		}
	return "";
}

std::string smfigapl::NamedGamePlan::getPauseTeamColorAtomic(uint round, uint pauseTeamIndex, uint atomicIndex)
{
	uint j=0;
	for (uint i=0;i<this->getNPauseTeams(round);i++)
		if (rounds_[round].getPauseTeamAtomicIndex(i)==atomicIndex)
		{
			if(j==pauseTeamIndex)
				return this->getPauseTeamColor(round, j);
			else
				j++;
		}
	return "";
}
smfigapl::uint smfigapl::NamedGamePlan::getNPauseTeams(uint round)
{
	return rounds_[round].getNPauseTeams();
}

smfigapl::uint smfigapl::NamedGamePlan::getNPauseTeamsAtomic(uint round, uint atomicIndex)
{
	uint temp=0;;
	for (uint i=0;i<this->getNPauseTeams(round);i++)
		if (rounds_[round].getPauseTeamAtomicIndex(i)==atomicIndex)
			temp++;
	return temp;
}

void smfigapl::NamedGamePlan::init(uint n, uint m)
{
	smfigapl::NamedRound temp(m);
//	temp.init(m);
	rounds_.resize(n,temp);
}

smfigapl::uint smfigapl::NamedGamePlan::getNRounds()
{
	return static_cast<uint>(rounds_.size());
}

