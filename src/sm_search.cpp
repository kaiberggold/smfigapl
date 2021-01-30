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
#include "../include/sm_search.hpp"

std::string smfigapl::GamePlanSearch::getClubNameByTeamId(uint id)
{
	return input->setup.namedTeamByTeamId[id].getTeamClub();
}
std::string smfigapl::GamePlanSearch::getTeamColorByTeamId(uint id)
{
	return input->setup.namedTeamByTeamId[id].getTeamColor();
}


void smfigapl::GamePlanSearch::preInit()
{
	calcAtomicFieldSets();
	resultArray_.init(NAtomicSets_);
}


void smfigapl::GamePlanSearch::init()
{
	indexedRound_.init(condTables.atomicCapacityArray[condTables.atomicIndex].NFields);
	output->indexedGamePlan.setFieldTypeByFieldId(input->setup.mapping.indexedFieldTypeByFieldId);
	treeSearchLevel=0;
	pauseClubs_=false;

	pause_.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams);
	std::fill(pause_.begin(),pause_.end(),false);

	treeSearchIndex_.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds);
	NPairingArrayFull_=ts->getNPairingArray()*ts->getNFieldMixArray()*input->conditions.iterations;
	searchIndexMaxInit();
	lastCheck_={true};
	this->initIndexedPairingArrayFull();
	this->initCondRepeatedGames();
	this->initSameClubArray();
	this->initFieldTypeArray();
	this->initPauseArray();

}


void smfigapl::GamePlanSearch::calcAtomicFieldSets()
{
	std::vector<bool> currentSubTypes;
	std::vector<bool> allSubTypes;
	std::vector<uint> newSubTypes;
	std::vector<uint> newTeams;
	std::vector<uint> newClubs;
	std::vector<uint> newColors;
	std::vector<uint> tempFieldMap;
	Mapping tempMap;
	bool update;
	uint NSubTypesAssigned=0;
	uint currentSubType=0;
	NAtomicSets_=0;
	currentSubTypes.resize(input->setup.realCapacity.NFieldSubTypes);
	std::fill(currentSubTypes.begin(),currentSubTypes.end(),false);
	allSubTypes.resize(input->setup.realCapacity.NFieldSubTypes);
	std::fill(allSubTypes.begin(),allSubTypes.end(),false);
	atomicIndexByFieldId_.resize(input->setup.realCapacity.NFields);
	atomicIndexByTeamId_.resize(input->setup.realCapacity.NTeams);
	condTables.fieldMappingArray.clear();
	condTables.teamIdMappingArray.resize(input->setup.realCapacity.NTeams);
    while (NSubTypesAssigned<input->setup.realCapacity.NFieldSubTypes)
    {
    	condTables.atomicCapacityArray.resize(condTables.atomicCapacityArray.size()+1);
    	newSubTypes.clear();
    	newTeams.clear();
    	newClubs.clear();
    	newColors.clear();
    	std::fill(currentSubTypes.begin(), currentSubTypes.end(), false);
		for(uint i=0;i<input->setup.realCapacity.NFieldSubTypes;i++)
		{
			if (!allSubTypes[i])
			{
				currentSubTypes[i]=true;
				allSubTypes[i]=true;
				currentSubType=i;
				newSubTypes.push_back(i);
				NSubTypesAssigned++;
				break;
			}
		}
		do
		{
	    	update=false;
			// Update new Teams
			for(uint i=0;i<input->setup.realCapacity.NTeams;i++)
			{
				for(uint j=0;j<input->setup.realCapacity.NFieldSubTypes;j++)
				{
					if (!input->conditions.fieldSubTypeDeny.values[i][j] && currentSubTypes[j])
					{
						bool newTeam=true;
						for (auto && t: newTeams)
							if (t==i)
								newTeam=false;
						if (newTeam)
						{
							newTeams.push_back(i);
							update=true;
							bool newClub=true;
							for (auto && t: newClubs)
								if (t==input->setup.mapping.indexedClubByTeamId[i])
									newClub=false;
							if (newClub)
							{
								newClubs.push_back(input->setup.mapping.indexedClubByTeamId[i]);
							}
							bool newColor=true;
							for (auto && t: newColors)
								if (t==input->setup.mapping.indexedColorByTeamId[i])
									newColor=false;
							if (newColor)
							{
								newColors.push_back(input->setup.mapping.indexedColorByTeamId[i]);
							}
						}
					}
				}
			}

			// Update subtypes
			for(auto && i:newTeams)
			{
				for(uint j=0;j<input->setup.realCapacity.NFieldSubTypes;j++)
				{
					if (!input->conditions.fieldSubTypeDeny.values[i][j])
					{
						bool newsubType=true;
						for (auto && s: newSubTypes)
							if (s==j)
								newsubType=false;
						if (newsubType)
						{
							newSubTypes.push_back(j);
							currentSubTypes[j]=true;
							allSubTypes[j]=true;
							NSubTypesAssigned++;
							update=true;
						}
					}
				}
			}

		}
    	while (update);

		// Update atomicCapacityArray
		std::vector<uint> ft;
		ft.clear();
		std::vector<uint> sft;
		sft.clear();
		uint k=0;
		tempMap.reset();
		tempFieldMap.clear();
		for(uint i=0;i<input->setup.realCapacity.NFields;i++)
		{
			for (uint j=0;j<newSubTypes.size();j++)
			{
				if (newSubTypes[j]==input->setup.mapping.indexedFieldSubTypeByFieldId[i])
				{
					atomicIndexByFieldId_[i]=NAtomicSets_;
					(condTables.atomicCapacityArray[NAtomicSets_].NFields)++;
					if(std::find(ft.begin(), ft.end(), input->setup.mapping.indexedFieldTypeByFieldId[i]) == ft.end())
					{
						condTables.atomicCapacityArray[NAtomicSets_].NFieldTypes++;
						ft.push_back(input->setup.mapping.indexedFieldTypeByFieldId[i]);
					}
					condTables.atomicCapacityArray[NAtomicSets_].NFieldSubTypes=static_cast<uint>(newSubTypes.size());
					condTables.atomicCapacityArray[NAtomicSets_].NTeams=static_cast<uint>(newTeams.size());
					condTables.atomicCapacityArray[NAtomicSets_].NClubs=static_cast<uint>(newClubs.size());
					condTables.atomicCapacityArray[NAtomicSets_].NTeamColors=static_cast<uint>(newColors.size());
					tempMap.indexedFieldTypeByFieldId.push_back(input->setup.mapping.indexedFieldTypeByFieldId[i]);
					tempMap.indexedFieldSubTypeByFieldId.push_back(input->setup.mapping.indexedFieldSubTypeByFieldId[i]);
					tempFieldMap.push_back(i);
				}
			}
		}
		for (auto && i: newTeams)
		{
			tempMap.indexedClubByTeamId.push_back(input->setup.mapping.indexedClubByTeamId[i]);
			tempMap.indexedColorByTeamId.push_back(input->setup.mapping.indexedColorByTeamId[i]);
			tempMap.fieldDenybySubTypeAndTeamId.push_back(input->conditions.fieldSubTypeDeny.values[i]);
			atomicIndexByTeamId_[i]=NAtomicSets_;
		}
		condTables.fieldMappingArray.push_back(tempFieldMap);
		condTables.teamsMappingArray.push_back(newTeams);
		for (uint i=0;i<newTeams.size();i++)
			condTables.teamIdMappingArray[newTeams[i]]=i;
		condTables.atomicCapacityArray[NAtomicSets_].NRounds=input->setup.realCapacity.NRounds;
		condTables.atomicMappingArray.push_back(tempMap);
		NAtomicSets_++;
    }

}

void smfigapl::GamePlanSearch::setGamePlanRound(smfigapl::GamePlanRoundByTeamId roundIn)
{
	for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
	{
		smfigapl::IndexedGame game;
		if (roundIn.getTeamId(j,0)>=condTables.atomicCapacityArray[condTables.atomicIndex].NTeams ||roundIn.getTeamId(j,1)>=condTables.atomicCapacityArray[condTables.atomicIndex].NTeams )
		{
			smfigapl::IndexedTeam team;
			team.setTeamClub(0);
			team.setTeamColor(0);
			for (smfigapl::uint k=0;k<2;k++)
			{
				if (roundIn.getTeamId(j,k)<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams)
					team.setTeamId(roundIn.getTeamId(j,k));
				else
					team.setTeamId(TEAM_UNDEFINED);
				game.setTeam(k,team);
			}
			game.setPause(true);
		}
		else
		{
			smfigapl::IndexedTeam team;
			for (uint k=0;k<2;k++)
			{
				uint id=roundIn.getTeamId(j,k);
				team.setTeamClub(condTables.atomicMappingArray[condTables.atomicIndex].indexedClubByTeamId[id]);
				team.setTeamColor(condTables.atomicMappingArray[condTables.atomicIndex].indexedColorByTeamId[id]);
				team.setTeamId(id);
				game.setTeam(k,team);
			}
		}
		indexedRound_.setGame(j,game);
	}
	bool pause_Teams=false;
	fill(pause_.begin(),pause_.end(),false);
	for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
	{
		if (indexedRound_.getGame(j).getPause())
		{
			for (uint k=0;k<2;k++)
			{
				uint idx = indexedRound_.getGame(j).getTeambyGameId(k).getTeamId();
				if (idx !=TEAM_UNDEFINED)
				{
					pause_[idx]=true;
					pause_Teams=true;
				}
			}
		}
	}
	if (condTables.atomicCapacityArray[condTables.atomicIndex].NFields<ts->getNFields())
	{
		for (uint j = condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j < ts->getNFields(); j++)
		{
			for (uint k=0;k<2;k++)
			{
				if(roundIn.getTeamId(j,k)<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams)
				{
					pause_[roundIn.getTeamId(j,k)]=true;
					pause_Teams=true;
				}
			}
		}
	}
	if (pause_Teams)
	{
		indexedRound_.pauseTeamsClear();
		for(uint l=0;l<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;l++)
		{
			if (pause_[l])
			{
				smfigapl::IndexedTeam team;
				team.setTeamClub(condTables.atomicMappingArray[condTables.atomicIndex].indexedClubByTeamId[l]);
				team.setTeamColor(condTables.atomicMappingArray[condTables.atomicIndex].indexedColorByTeamId[l]);
				team.setTeamId(l);
				indexedRound_.addPauseTeam(team);
			}
		}
	}

}

//void smfigapl::GamePlanSearch::pairingArraySort(uint start, uint end)
//{
//	std::sort ( indexedPairingArrayFull_.begin()+start, indexedPairingArrayFull_.begin()+end, pairingArrayCompare);
//}

void 	smfigapl::GamePlanSearch::initIndexedPairingArrayFull()
{
	indexedPairingArrayFull_.clear();
	//get from ts
	for (unsigned k=0;k<input->conditions.iterations;k++)
	{
		for (uint i = 0; i<ts->getNPairingArray()*ts->getNFieldMixArray(); i++)
		{

			#ifdef SM_DBG
			if (input->dbg&DBG_GPS)
				std::cout<<"iii"<<i<<std::endl;
			#endif
			this->setGamePlanRound(ts->getRound(i));
			indexedRound_.setIndex(i);
			if (!isImpossibleIndex())
				indexedPairingArrayFull_.push_back(indexedRound_);
			else
			{
				NPairingArrayFull_--;
				searchIndexMaxUpdate(static_cast<uint>(indexedPairingArrayFull_.size()));
			}
		}
	}
}

void smfigapl::GamePlanSearch::balancedHomeAwayCalc()
{

	//Precalculations
	std::vector<std::vector<uint>> NHomeAwayGamesbyTeamIdArray;
	std::vector<uint> NGames;
	std::vector<uint> maxHomeAwayGames;
	std::vector<uint> NHomeAwayGamesbyTeamId;
	std::vector<uint> maxHomeAwayGamesTeam;
	std::vector<std::tuple<uint,uint>> maxGames;
	uint maxIndex=0;
	uint maxOther;

	//Calc N games for each team
	NGames.clear();
	for (uint k=0;k<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;k++)
	{
		uint l=0;
		for (uint i=0;i<condTables.atomicCapacityArray[condTables.atomicIndex].NRounds;i++)
		{
			if (!output->indexedGamePlan.isPauseTeam(i,k))
			{
				l++;
			}
		}
		NGames.push_back(l);
	}
	std::vector<uint>  halfLower;
	std::vector<uint>  halfHigher;
	halfLower.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams,condTables.atomicCapacityArray[condTables.atomicIndex].NRounds/2);
	for (uint k=0;k<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;k++)
	{
		uint h = NGames[k]/2;
		if (h < halfLower[k])
			halfLower[k]=h;
	}
	halfHigher.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams,(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds+1)/2);
	for (uint k=0;k<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;k++)
	{
		uint h = (NGames[k]+1)/2;
		if (h > halfHigher[k])
			halfHigher[k]=h;
	}
	bool check;
	uint homeAway=0; //0= start with homegame, 1= start with away game
	//Loop until equal distribution is reached
	uint maxLoops=condTables.atomicCapacityArray[condTables.atomicIndex].NRounds*(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds-1);
	uint loops=0;
	maxHomeAwayGames.resize(2);
	maxHomeAwayGamesTeam.resize(2);
	NHomeAwayGamesbyTeamId.resize(2);
	do
	{
		//Calc N home /awaygames for each team
		NHomeAwayGamesbyTeamIdArray.clear();
		std::fill(maxHomeAwayGames.begin(),maxHomeAwayGames.end(),0);
		for (uint l=0;l<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;l++)
		{
			std::fill(NHomeAwayGamesbyTeamId.begin(),NHomeAwayGamesbyTeamId.end(),0);
			for (uint i=0;i<condTables.atomicCapacityArray[condTables.atomicIndex].NRounds;i++)
			{
				if (output->indexedGamePlan.isHomeAtomic(i, condTables.teamsMappingArray[condTables.atomicIndex][l],condTables.atomicIndex))
					NHomeAwayGamesbyTeamId[0]++;
				else if (output->indexedGamePlan.isAwayAtomic(i, condTables.teamsMappingArray[condTables.atomicIndex][l],condTables.atomicIndex))
					NHomeAwayGamesbyTeamId[1]++;
			}
			for (uint k=0;k<2;k++)
			{
				if (NHomeAwayGamesbyTeamId[k]>maxHomeAwayGames[k])
				{
					maxHomeAwayGames[k]=NHomeAwayGamesbyTeamId[k];
					maxHomeAwayGamesTeam[k]=l;
				}
			}
			NHomeAwayGamesbyTeamIdArray.push_back(NHomeAwayGamesbyTeamId);
		}

		//calc games with the Team with the maximum of home/awaygames
		maxGames.clear();
		for (uint i=0;i<condTables.atomicCapacityArray[condTables.atomicIndex].NRounds;i++)
		{
			for (uint j=0;j<condTables.atomicCapacityArray[condTables.atomicIndex].NFields;j++)
			{
				uint teamId=output->indexedGamePlan.getGameAtomic(i,j,condTables.atomicIndex).getTeambyGameId(homeAway).getTeamId();
				if (teamId!=TEAM_UNDEFINED)
				{
					if (condTables.teamIdMappingArray[teamId]==maxHomeAwayGamesTeam[homeAway])
					{
						maxGames.push_back({i,j});
					}
				}
			}
		}

		//From one of these choose the pairing where the other team has highest away /home games (different order!)
		maxOther=0;
		maxIndex=0;
		for(uint i=0;i<maxGames.size();i++)
		{
			uint teamId=output->indexedGamePlan.getGameAtomic(std::get<0>(maxGames[i]),std::get<1>(maxGames[i]),condTables.atomicIndex).getTeambyGameId(1-homeAway).getTeamId();
			if (teamId!=TEAM_UNDEFINED)
			{
				uint l=NHomeAwayGamesbyTeamIdArray[condTables.teamIdMappingArray[teamId]][1-homeAway];
				if (l>maxOther)
				{
					maxOther=l;
					maxIndex=i;
				}
			}
		}

		check = true;
		for (uint l=0;l<condTables.atomicCapacityArray[condTables.atomicIndex].NTeams;l++)
		{
			check=check &&(maxHomeAwayGames[0]>=halfLower[l]&&maxHomeAwayGames[0]<=halfHigher[l]&&maxHomeAwayGames[1]>=halfLower[l]&&maxHomeAwayGames[1]<=halfHigher[l]);
		}
		//swap game
		if (!check)
			output->indexedGamePlan.flipSwapAtomic(std::get<0>(maxGames[maxIndex]),std::get<1>(maxGames[maxIndex]),condTables.atomicIndex);

		if (maxHomeAwayGames[0]>=maxHomeAwayGames[1])
			homeAway=0;
		else
			homeAway=1;
		//abort conditions
		loops++;

	}
	while(loops<maxLoops&&!check);
	#ifdef SM_DBG
	if (input->dbg&DBG_GPS)
		std::cout<<"HomeAway Loops: "<<loops<<std::endl;
	#endif
}

void smfigapl::GamePlanSearch::calcIndexedGamePlan()
{
	smfigapl::IndexedRound round;
	for (uint i = 0; i<condTables.atomicCapacityArray[condTables.atomicIndex].NRounds; i++)
	{
		round=indexedPairingArrayFull_[treeSearchIndex_[i]];
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			IndexedGame game;
			game=round.getGame(ts->getfieldTypeSortArray(j));
			for (uint k=0;k<2;k++)
			{
				IndexedTeam team=game.getTeambyGameId(k);
				uint teamId=team.getTeamId();
				auto a=condTables.teamsMappingArray[condTables.atomicIndex];
				if (teamId==TEAM_UNDEFINED)
					team.setTeamId(TEAM_UNDEFINED);
				else
					team.setTeamId(condTables.teamsMappingArray[condTables.atomicIndex][teamId]);
				game.setTeam(k, team);
			}
			if (output->indexedGamePlan.getGame(i, condTables.fieldMappingArray[condTables.atomicIndex][j]).getSwap())
				game.setSwap(true);
			output->indexedGamePlan.setGame(i, condTables.fieldMappingArray[condTables.atomicIndex][j], game);

		}
		for (auto && p:round.getPauseTeams())
		{
			uint teamId=p.getTeamId();
			p.setTeamId(condTables.teamsMappingArray[condTables.atomicIndex][teamId]);
			output->indexedGamePlan.addPauseTeamAtomic(i, p,condTables.atomicIndex);
		}
	}
}

bool smfigapl::GamePlanSearch::treeSearchIndexNext()
{
			uint idx=treeSearchIndex_[treeSearchLevel];
			#ifdef SM_DBG
			if (input->dbg&DBG_GPS)
				std::cout<<"jump ts: "<<treeSearchLevel<<"idx: "<<treeSearchIndex_[treeSearchLevel]<<" ff: "<<lastCheck_.check<<" "<<" j:"<<lastCheck_.jump<<" ";
			#endif
			if (idx+lastCheck_.jump<=condTables.searchIndexMax[treeSearchLevel])
			{
				treeSearchIndex_[treeSearchLevel]=idx+lastCheck_.jump;
				return true;
			}
	return false;
}

void smfigapl::GamePlanSearch::treeSearchIndexNextLevelDown()
{
	uint jump=1;
	if (treeSearchLevel==(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams-1) && input->conditions.iterations==2&&input->conditions.repeatPairings)
	{
		jump=NPairingArrayFull_/input->conditions.iterations-treeSearchIndex_[treeSearchLevel-1];
	}
	else
	{
	#ifdef SM_JUMP_FAST_LEVELDOWN
		for (uint j=0;j<condTables.repeatedGames.condArray[treeSearchIndex_[treeSearchLevel-1]].size();j++)
			{
				jump=std::max(jump,condTables.repeatedGames.jumpArray[treeSearchIndex_[treeSearchLevel-1]][condTables.repeatedGames.condArray[treeSearchIndex_[treeSearchLevel-1]][j]]);
			}
	//std::cout <<"jumplevdown"<<jump;
	#endif
	}
	treeSearchIndex_[treeSearchLevel]=treeSearchIndex_[treeSearchLevel-1]+jump;
}

void smfigapl::GamePlanSearch::initCondRepeatedGames()
{
	uint index;
	uint l;
	IndexedRound round;
	std::vector<uint> temp;
	uint a;
	uint b;
	condTables.repeatedGames.condArray.clear();
	condTables.repeatedGames.hashArray.clear();
	condTables.repeatedGames.jumpArray.clear();
	for (uint i = 0; i < NPairingArrayFull_; i++)
	{
		round=indexedPairingArrayFull_[i];
		temp.clear();
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			if (!round.getGame(j).getPause())
			{
				a=round.getGame(j).getTeambyGameId(0).getTeamId();
				b=round.getGame(j).getTeambyGameId(1).getTeamId();
				index=getPairingIndexShort(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams,a,b);
				temp.push_back(index);
			}
		}
		condTables.repeatedGames.condArray.push_back(temp);

		//Hash Array
		IndexedRound roundAsc=round;
		uint hash=0;
		roundAsc.sort();
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			hash=hash|roundAsc.getGame(j).getTeambyGameId(0).getTeamId();
			hash=hash<<4;//NRealTeams<=16
		}
		condTables.repeatedGames.hashArray.push_back(hash);

		//jumpArray
		temp.resize(getNPairingIndexShort(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams));
		std::fill(temp.begin(),temp.end(),0);
		fill(temp.begin(), temp.end(), 0);
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			if(!indexedPairingArrayFull_[i].getGame(j).getPause())
			{
				l=i;
				while(l<NPairingArrayFull_ &&indexedPairingArrayFull_[l].contains(indexedPairingArrayFull_[i].getGame(j)))
				{
					l++;
				}
				uint idx=getPairingIndexShort(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams,indexedPairingArrayFull_[i].getGame(j).getTeambyGameId(0).getTeamId(),\
						indexedPairingArrayFull_[i].getGame(j).getTeambyGameId(1).getTeamId());
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
					std::cout<<"idx"<<temp.size()<<" "<<idx<<" "<<indexedPairingArrayFull_[i].getGame(j).getTeambyGameId(0).getTeamId()<<" "<<indexedPairingArrayFull_[i].getGame(j).getTeambyGameId(1).getTeamId()<<std::endl;
				#endif
				temp[idx]=std::min(l-i,NPairingArrayFull_-i-1);
			}
		}
		condTables.repeatedGames.jumpArray.push_back(temp);
	}
}

bool smfigapl::GamePlanSearch::isImpossibleIndex()
{
	uint emptyFields;
	emptyFields=0;
	for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
	{
		if (indexedRound_.getGame(j).getPause())
			emptyFields++;
	}
	bool res=emptyFields>input->conditions.maxFieldsEmpty;
	//TODO: return here if true

	//denylist
	if (input->conditions.fieldSubTypeDeny.enabled)
	{
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			for (uint k = 0; k < 2; k++)
			{
				uint teamId=indexedRound_.getGame(j).getTeambyGameId(k).getTeamId();
				if (teamId!=TEAM_UNDEFINED)
					if (condTables.atomicMappingArray[condTables.atomicIndex].fieldDenybySubTypeAndTeamId[teamId]\
						[condTables.atomicMappingArray[condTables.atomicIndex].indexedFieldSubTypeByFieldId[ts->getfieldTypeSortArray(j)]])
						res=true;
			}
		}
	}
	return res;
}

void smfigapl::GamePlanSearch::initSameClubArray ()
{
	std::vector<uint> temp;
	IndexedRound round;
	condTables.sameClub.condArray.clear();
	condTables.sameClub.hashArray.clear();
	for (uint i = 0; i < NPairingArrayFull_; i++)
	{
		round=indexedPairingArrayFull_[i];
		temp.clear();
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			if (!round.getGame(j).getPause())
			{
				if (round.getGame(j).getTeambyGameId(0).getTeamClub()==round.getGame(j).getTeambyGameId(1).getTeamClub())
				{
					temp.push_back(round.getGame(j).getTeambyGameId(0).getTeamId());
					temp.push_back(round.getGame(j).getTeambyGameId(1).getTeamId());
				}
			}
		}
		condTables.sameClub.condArray.push_back(temp);
	}
}

void smfigapl::GamePlanSearch::initPauseArray ()
{
	std::vector<uint> temp;
	IndexedRound round;
	condTables.pauseGames.condArray.clear();
	for (uint i = 0; i < NPairingArrayFull_; i++)
	{
		round=indexedPairingArrayFull_[i];
		temp.clear();
		for (uint j = 0; j < round.getNPauseTeams(); j++)
		{
			temp.push_back(round.getPauseTeam(j).getTeamId());
		}
		condTables.pauseGames.condArray.push_back(temp);
	}
}

void smfigapl::GamePlanSearch::initFieldTypeArray ()
{
	std::vector<std::vector<uint>> temp;
	IndexedRound round;
	uint l;
	condTables.fieldType.condArray.clear();
	for (uint i = 0; i < NPairingArrayFull_; i++)
	{
		temp.clear();
		temp.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NFieldTypes);
		round=indexedPairingArrayFull_[i];
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			for (uint k=0;k<2;k++)
			{
				uint id=round.getGame(j).getTeambyGameId(k).getTeamId();
				if(indexedPairingArrayFull_[i].getGame(j).getPause())
					temp[ts->getFieldTypesSorted(j)].push_back(TEAM_UNDEFINED);
				else
					temp[ts->getFieldTypesSorted(j)].push_back(id);
			}
		}
		condTables.fieldType.condArray.push_back(temp);

		#ifdef SM_MAX_FT_JUMP
		//jumpArray
		temp.clear();
		std::vector<uint> tempRow;
		tempRow.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams);
		std::fill(tempRow.begin(),tempRow.end(),0);
		temp.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NFieldTypes);
		std::fill(temp.begin(),temp.end(),tempRow);
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFieldTypes; j++)
			for (uint m = 0; m < condTables.atomicCapacityArray[condTables.atomicIndex].NTeams; m++)
			{
				if(!indexedPairingArrayFull_[i].isPauseTeam(m))
				{
					l=i;
					uint uintAct=indexedPairingArrayFull_[i].getFieldIdbyTeamId(m);
					uint uintNext=indexedPairingArrayFull_[l].getFieldIdbyTeamId(m);
					uint fieldTypeAct=ts->getFieldTypesSorted(uintAct);
					uint fieldTypeNext=ts->getFieldTypesSorted(uintNext);
					while(l<NPairingArrayFull_-1 &&uintAct!=255 &&uintNext!=255 \
							&&fieldTypeAct==fieldTypeNext)
					{
						l++;
						uintNext=indexedPairingArrayFull_[l].getFieldIdbyTeamId(m);
						if (uintNext!=255)
							fieldTypeNext=ts->getFieldTypesSorted(uintNext);
					}
					temp[j][m]=std::min(l-i,NPairingArrayFull_-i-1);
				}
			}
		condTables.fieldType.jumpArray.push_back(temp);
		#endif
	}

}

void smfigapl::GamePlanSearch::searchIndexMaxInit()
{
	condTables.searchIndexMax.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds);
	uint k=ts->getNFields()*2-1;
	uint l=ts->getNPairingArray();
	l=l/k*ts->getNFieldMixArray();
	condTables.searchIndexMax[condTables.atomicCapacityArray[condTables.atomicIndex].NRounds-1]=NPairingArrayFull_-1;
	for (uint i=2;i<input->setup.realCapacity.NRounds+1;i++)
	{
		if (condTables.searchIndexMax[input->setup.realCapacity.NRounds-i+1]>=l)
			condTables.searchIndexMax[input->setup.realCapacity.NRounds-i]=condTables.searchIndexMax[input->setup.realCapacity.NRounds-i+1]-l;
		else
			//too much rounds, not possible anyway, but limit to 0
			condTables.searchIndexMax[input->setup.realCapacity.NRounds-i]=0;
	}
}

void smfigapl::GamePlanSearch::searchIndexMaxUpdate(uint i)
{
	for (uint k=0;k<condTables.searchIndexMax.size();k++)
		if (condTables.searchIndexMax[k]>=i)
			condTables.searchIndexMax[k]--;
}

smfigapl::uint smfigapl::GamePlanSearch::searchStep(bool init, uint oldResult)
{
	bool nextStep;
	bool firsttime=true;
	if (init)
	{
		treeSearchState.Add=0;
		treeSearchState.Sub=-1;
		#ifdef SM_DBG
		if (input->dbg&DBG_GPS)
		{
			std::cout<<"init";this->displayTreeSearchIndex();
		}
		#endif
		return smfigapl::TS_NEXT_STEP;
	}
	else
	{
		if (lastCheck_.check &&oldResult!=TS_LEVEL_UP)
		{
			if (treeSearchLevel < condTables.atomicCapacityArray[condTables.atomicIndex].NRounds - 1) // all conditions fullfilled, go one level down
			{
				treeSearchLevel++;
				treeSearchIndexNextLevelDown();
				treeSearchState.Add=treeSearchIndex_[treeSearchLevel];
				treeSearchState.Sub=-1;
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
					std::cout<<"TS_NEXT_STEP"<<" Add: "<< treeSearchState.Add<<std::endl;
				#endif
				return smfigapl::TS_NEXT_STEP;
			}
			else
			{
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
					std::cout<<"TS_SUCCESS"<<lastCheck_.check<<std::endl;
				#endif
				return smfigapl::TS_SUCCESS;
			}
		}
		else //TS_LEVEL_UP or check failed
		{
			treeSearchState.Sub=treeSearchIndex_[treeSearchLevel];
			nextStep=this->treeSearchIndexNext();
			if (treeSearchIndex_[treeSearchLevel] <= condTables.searchIndexMax[treeSearchLevel]&&nextStep)
			{
				treeSearchState.Add=treeSearchIndex_[treeSearchLevel];
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
					std::cout<<"TS_NEXT_STEP_2"<<" Add: "<< treeSearchState.Add<<" Sub: "<< treeSearchState.Sub<<std::endl;
				#endif
				return smfigapl::TS_NEXT_STEP;
			}
			else
			{
				treeSearchState.Sub=-1;
				if (treeSearchLevel == 0)
				{
					std::cerr<<"TS_FAILED"<<std::endl;
					return smfigapl::TS_FAILED;
				}
				else
				{
					treeSearchState.Add=-1;
					treeSearchState.Sub=treeSearchIndex_[treeSearchLevel];
					treeSearchIndex_[treeSearchLevel]=0;
					treeSearchLevel--;

					#ifdef SM_DBG
					if (input->dbg&DBG_GPS)
						std::cout<<"TS_LEVEL_UP"<<" Add: "<< treeSearchState.Add<<" Sub: "<< treeSearchState.Sub;
					#endif
					return smfigapl::TS_LEVEL_UP;
				}
			}
		}
		std::cerr<<"TS_FAILED_2"<<std::endl;
		return smfigapl::TS_FAILED_2;
	}
}

smfigapl::ResultArray smfigapl::GamePlanSearch::gpSearch()
{
	preInit();
	output->init(NAtomicSets_);
	output->indexedGamePlan.setAtomicFieldMappingArray(condTables.fieldMappingArray);
	for (condTables.atomicIndex=0;condTables.atomicIndex<NAtomicSets_;condTables.atomicIndex++)
	{
		ts->setCapacity(condTables.atomicCapacityArray[condTables.atomicIndex]);
		ts->setExtraPauseFields(input->conditions.extraPauseFields);
		ts->setMapping(input->setup.mapping);
		ts->init();
		check->init();
		init();
		treeSearchState.stepResult=TS_NEXT_STEP;
		this->searchStep(true,0);
		if (NPairingArrayFull_>0)
		{
			lastCheck_=check->gamePlanCheck(true);
			#ifdef SM_DBG
			if (input->dbg&DBG_GPS)
			{
				std::cout <<"Atomic Index: "<<condTables.atomicIndex<<std::endl;
				std::cout <<"check: "<<lastCheck_.check<<" "<<std::endl;
			}
			#endif
			do
			{
					#ifdef SM_DBG
					//if (input->dbg&(DBG_GPS+DBG_TS))
						//std::cout <<std::endl<<"----------------------"<<std::endl;
					#endif
				if (resultArray_.getStep(condTables.atomicIndex) % stepSize_ == 0 &&stepsDisplay_)
				{
					std::cout << "step: "<< resultArray_.getStep(condTables.atomicIndex) ;
					#ifdef SM_DBG
					if (input->dbg&DBG_STEPS_DETAILS)
					{
						std::cout <<std::endl<<"Search: ";
						this->displayTreeSearchIndex();
					}
					else
						std::cout<<std::endl;
					#else
					std::cout<<std::endl;
					#endif
				}

				treeSearchState.stepResult = this->searchStep(false,treeSearchState.stepResult);
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
				{
					std::cout<<"Sl: " <<treeSearchLevel<<" ";
					this->displayTreeSearchIndex();
				}
				#endif
				if (treeSearchState.stepResult !=TS_SUCCESS)
					lastCheck_=check->gamePlanCheck(false);
				resultArray_.incStep(condTables.atomicIndex);




			} while (resultArray_.getStep(condTables.atomicIndex) < maxSteps_ && (treeSearchState.stepResult == TS_NEXT_STEP ||treeSearchState.stepResult ==TS_LEVEL_UP));
			resultArray_.setResult(condTables.atomicIndex,treeSearchState.stepResult);
			if (stepsDisplay_)
			{
				std::cout << "step: " << resultArray_.getStep(condTables.atomicIndex)<<std::endl;
				#ifdef SM_DBG
				if (input->dbg&DBG_GPS)
					this->displayTreeSearchIndex();
				#endif
			}
			this->calcIndexedGamePlan();
			if (input->conditions.homeAwayBalance)
				this->balancedHomeAwayCalc();
			if (input->conditions.optimizeRounds)
				this->optimizeRounds();
		}
		else
			resultArray_.setResult(condTables.atomicIndex,TS_FAILED);

	} //Atomic sets
	this->calcNamedGamePlan();
	output->setResult(resultArray_);
	return resultArray_;
}

void smfigapl::GamePlanSearch::calcNamedGamePlan()
{
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			if (!output->indexedGamePlan.getGame(i,j).getPause())
			{
				for (uint k = 0; k < 2; k++)
				{
					output->namedGamePlan.setTeamClub(i,j,k,this->getClubNameByTeamId(output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()));
					output->namedGamePlan.setTeamColor(i,j,k,this->getTeamColorByTeamId(output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()));
				}

			}
			else
			{
				for (uint k = 0; k < 2; k++)
				{
					output->namedGamePlan.setTeamClub(i,j,k,"-");
					output->namedGamePlan.setTeamColor(i,j,k,"");
				}
			}
		} //j


		for (uint j = 0; j < output->indexedGamePlan.getNPauseTeams(i); j++)
		{
			uint id=output->indexedGamePlan.getPauseTeam(i,j).getTeamId();
			output->namedGamePlan.addPauseTeamAtomic(i,this->getClubNameByTeamId(id),this->getTeamColorByTeamId(id),atomicIndexByTeamId_[id]);
		}
	} //i
	output->namedGamePlan.setAtomicIndexbyFieldId(atomicIndexByFieldId_);
}

void smfigapl::GamePlanSearch::displayTreeSearchIndex()
{
	std::cout << "Sl: "<< treeSearchLevel<< " Si: " ;
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
		std::cout << " " << treeSearchIndex_[i];
	std::cout<<" (";
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
			std::cout << " " << condTables.searchIndexMax[i];
	std::cout <<" )"<< std::endl;
}

void smfigapl::GamePlanSearch::calcPairingIndexShortArray()
{
	uint a;
	uint b;
	std::vector<uint> temp;
	temp.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NFields);
	std::fill(temp.begin(),temp.end(),0);
	pairingIndexShortArray_.resize(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds);
	std::fill(pairingIndexShortArray_.begin(),pairingIndexShortArray_.end(),temp);
	for (uint i = 0; i < condTables.atomicCapacityArray[condTables.atomicIndex].NRounds;i++)
	{
		for (uint j = 0; j < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; j++)
		{
			if (!output->indexedGamePlan.getPause(i, j))
			{
				//TODO UDEF
				a=condTables.teamIdMappingArray[output->indexedGamePlan.getGameAtomic(i, j,condTables.atomicIndex).getTeambyGameId(0).getTeamId()];
				b=condTables.teamIdMappingArray[output->indexedGamePlan.getGameAtomic(i, j,condTables.atomicIndex).getTeambyGameId(1).getTeamId()];
				if (b<a)
					std::swap(a,b);
				pairingIndexShortArray_[i][j]=getPairingIndexShort(condTables.atomicCapacityArray[condTables.atomicIndex].NTeams,a,b);
			}
		}
	}
}

smfigapl::GamePlanSearch::AdjescentRowsCheck smfigapl::GamePlanSearch::checkAdjacentRows(uint round, uint atomicIndex)
{
	bool pause_=true;
	bool pairing=true;
	for (uint i = 0; i < condTables.atomicCapacityArray[condTables.atomicIndex].NTeams; i++)
	{
		if(output->indexedGamePlan.isPauseTeam(round, i) &&output->indexedGamePlan.isPauseTeam(round+1, i))
			pause_=false;
	}
	for (uint l = 0; l < condTables.atomicCapacityArray[condTables.atomicIndex].NFields; l++)
	{
		if(pairingIndexShortArray_[output->indexedGamePlan.getResortIndex(round,condTables.atomicIndex)][l]==pairingIndexShortArray_[output->indexedGamePlan.getResortIndex(round+1,condTables.atomicIndex)][l])
			pairing=false;
	}
	return {pause_ ,pairing};
}


void smfigapl::GamePlanSearch::optimizeRounds()
{
	AdjescentRowsCheck check;
	uint sortIdxMax=factorial(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds);
	this->calcPairingIndexShortArray();
	uint sortIdx=0;
	do
	{
		check={true,true};
		for (uint i = 0; i < condTables.atomicCapacityArray[condTables.atomicIndex].NRounds-1; i++)
		{
			check=this->checkAdjacentRows(i,condTables.atomicIndex);
			if (!check.passed())
				break;
		}
		output->indexedGamePlan.setResortIndex(condTables.atomicCapacityArray[condTables.atomicIndex].NRounds, sortIdx,condTables.atomicIndex);
		sortIdx++;
	}
	while (sortIdx<sortIdxMax && !check.passed());
	#ifdef SM_DBG
	if (input->dbg&DBG_GPS)
	{
		std::cout<<"Optimize Rounds, Atomic: "<<condTables.atomicIndex <<" Idx: " <<sortIdx<<std::endl;
	}
	#endif
}

void smfigapl::GamePlanSearch::setMaxSteps(uint m)
{
	maxSteps_=m;
}

smfigapl::uint smfigapl::GamePlanSearch::getSteps()
{
	return resultArray_.getTotalSteps();
}

void smfigapl::GamePlanSearch::setStepsDisplay(bool ena, uint size)
{
	stepsDisplay_=ena;
	stepSize_=size;
}

#ifdef SM_DBG
void smfigapl::GamePlanSearch::dispNFailArray()
{
	check->dispNFailArray();
}
#endif
