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

#include "../include/sm_cond.hpp"

void smfigapl::CheckGamePlan::init()
{
	NPairingIndexShort_=getNPairingIndexShort(condTables->atomicCapacityArray[condTables->atomicIndex].NTeams);
	res_.check=false;
	count_.pairing.resize(NPairingIndexShort_);
	std::fill(count_.pairing.begin(),count_.pairing.end(),0);
	jumpCheck_.resize(NPairingIndexShort_);
	std::fill(jumpCheck_.begin(),jumpCheck_.end(),0);
	checkArray_.resize(NPairingIndexShort_);
	std::fill(checkArray_.begin(),checkArray_.end(),0);
	count_.sameClub.resize(condTables->atomicCapacityArray[atomicIndex_].NTeams);
	std::fill(count_.sameClub.begin(),count_.sameClub.end(),0);
	count_.pause.resize(condTables->atomicCapacityArray[atomicIndex_].NTeams);
	std::fill(count_.pause.begin(),count_.pause.end(),0);
	std::vector<uint> temp;
	temp.resize(condTables->atomicCapacityArray[atomicIndex_].NFieldTypes);
	std::fill(temp.begin(),temp.end(),0);
	count_.fieldType.resize(condTables->atomicCapacityArray[atomicIndex_].NTeams);
	std::fill(count_.fieldType.begin(),count_.fieldType.end(),temp);
	std::vector<bool> temp2;
	temp2.resize(condTables->atomicCapacityArray[atomicIndex_].NTeams);
	std::fill(temp2.begin(),temp2.end(),false);
	checkMax_.resize(condTables->atomicCapacityArray[atomicIndex_].NFieldTypes);
	std::fill(checkMax_.begin(),checkMax_.end(),temp2);
}


bool smfigapl::CheckGamePlan::gamePlanCheckRepeatedGames(bool init)
{
	bool check =true;
	std::fill(checkArray_.begin(),checkArray_.end(),true);
	if (!init)
		{
			if (treeSearchState->Add>=0)
			{
				for (uint j=0;j<condTables->repeatedGames.condArray[treeSearchState->Add].size();j++)
				{
					if (count_.pairing[condTables->repeatedGames.condArray[treeSearchState->Add][j]]>std::min(*treeSearchLevel/(condTables->atomicCapacityArray[atomicIndex_].NTeams-1)+1,input->conditions.iterations))
					{
						check=false;
						checkArray_[condTables->repeatedGames.condArray[treeSearchState->Add][j]]=false;
					}
				}
			}
		}
	//Same level, current  check failed, calc jump
	if (!check && treeSearchState->stepResult==TS_NEXT_STEP &&treeSearchState->Add>0 &&treeSearchState->Sub>0 &&\
			condTables->repeatedGames.hashArray[treeSearchState->Add]!=condTables->repeatedGames.hashArray[treeSearchState->Sub])
	{
		std::fill(jumpCheck_.begin(),jumpCheck_.end(),0);
		for (uint j=0;j<condTables->repeatedGames.condArray[treeSearchState->Add].size();j++)
			{
			jumpCheck_[condTables->repeatedGames.condArray[treeSearchState->Add][j]]=true;
			}
		for (uint j=0;j<condTables->repeatedGames.condArray[treeSearchState->Sub].size();j++)
			{
			jumpCheck_[condTables->repeatedGames.condArray[treeSearchState->Sub][j]]=false;
			}

		for (uint j=0;j<getNPairingIndexShort(condTables->atomicCapacityArray[atomicIndex_].NTeams);j++)
		{
			if (jumpCheck_[j]&&!checkArray_[j])
			{
				res_.jump=std::max(res_.jump,condTables->repeatedGames.jumpArray[treeSearchState->Add][j]);
			}
		}
	}
	#ifndef SM_FAST_JUMP
		res_.jump=1;
	#endif
		#ifdef SM_DBG
		if (input->dbg&DBG_COND)
			std::cout<<"jump "<<res_.jump<<std::endl;
		#endif
	#ifdef SM_DBG
	if (input->dbg&DBG_COND)
	{
		uint count=0;
		for (uint i = 0;i<NPairingIndexShort_;i++)
		{
			std::cout<<count_.pairing[i] <<" ";
			count+=count_.pairing[i];
		}
		std::cout<<"count: "<<count;
	}
	#endif
	return check;
}


void smfigapl::CheckGamePlan::updateCounters(bool init)
{
	if (init)
	{
		#ifdef SM_DBG
			NFailedArray_.searchIndex.resize(condTables->searchIndexMax[condTables->searchIndexMax.size()-1]);
			std::fill(NFailedArray_.searchIndex.begin(),NFailedArray_.searchIndex.end(),true);
			NFailedArray_.searchIndexFalse.resize(condTables->searchIndexMax[condTables->searchIndexMax.size()-1]);
			std::fill(NFailedArray_.searchIndexFalse.begin(),NFailedArray_.searchIndexFalse.end(),false);
			#endif
		//repeated Games
		for (uint j=0;j<condTables->repeatedGames.condArray[0].size();j++)
		{
			count_.pairing[condTables->repeatedGames.condArray[0][j]]=1;
		}
		//Same Club
		for (uint i=0;i<condTables->sameClub.condArray[0].size();i++)
		{
			count_.sameClub[condTables->sameClub.condArray[0][i]]=1;
		}
		//Update Counters Fieldtypes
		for (uint i=0;i<condTables->atomicCapacityArray[atomicIndex_].NFieldTypes;i++)
		{
			for (uint j=0;j<condTables->fieldType.condArray[0][i].size();j++)
			{
				if (condTables->fieldType.condArray[0][i][j]!=TEAM_UNDEFINED)
					count_.fieldType[condTables->fieldType.condArray[0][i][j]][i]=1;
			}
		}
		//pause Games
		for (uint i=0;i<condTables->pauseGames.condArray[0].size();i++)
		{
			count_.pause[condTables->pauseGames.condArray[0][i]]=1;
		}
	}
	else
	{
		if (treeSearchState->Sub>=0)
		{
			//repeatedGames
			for (uint j=0;j<condTables->repeatedGames.condArray[treeSearchState->Sub].size();j++)
			{
				count_.pairing[condTables->repeatedGames.condArray[treeSearchState->Sub][j]]-=1;
			}
			//Same Club
			for (uint i=0;i<condTables->sameClub.condArray[treeSearchState->Sub].size();i++)
						{
							count_.sameClub[condTables->sameClub.condArray[treeSearchState->Sub][i]]-=1;
						}
			//field_types
			for (uint i=0;i<condTables->atomicCapacityArray[atomicIndex_].NFieldTypes;i++)
						{
							for (uint j=0;j<(condTables->fieldType.condArray[treeSearchState->Sub][i]).size();j++)
							{
								if (condTables->fieldType.condArray[treeSearchState->Sub][i][j]!=TEAM_UNDEFINED)
									count_.fieldType[condTables->fieldType.condArray[treeSearchState->Sub][i][j]][i]-=1;
							}
						}

			// Pause Games
			for (uint i=0;i<condTables->pauseGames.condArray[treeSearchState->Sub].size();i++)
						{
							count_.pause[condTables->pauseGames.condArray[treeSearchState->Sub][i]]-=1;
						}
		}

		if (treeSearchState->Add>=0)
		{
			//repeated Games
			for (uint j=0;j<condTables->repeatedGames.condArray[treeSearchState->Add].size();j++)
			{
				count_.pairing[condTables->repeatedGames.condArray[treeSearchState->Add][j]]+=1;
			}
			//Same Club
			for (uint i=0;i<condTables->sameClub.condArray[treeSearchState->Add].size();i++)
			{
				count_.sameClub[condTables->sameClub.condArray[treeSearchState->Add][i]]+=1;
			}
			//fieldTypes
			for (uint i=0;i<condTables->atomicCapacityArray[atomicIndex_].NFieldTypes;i++)
			{
				for (uint j=0;j<condTables->fieldType.condArray[treeSearchState->Add][i].size();j++)
				{
					if (condTables->fieldType.condArray[treeSearchState->Add][i][j]!=TEAM_UNDEFINED)
						count_.fieldType[condTables->fieldType.condArray[treeSearchState->Add][i][j]][i]+=1;
				}
			}
			// Pause Games
			for (uint i=0;i<condTables->pauseGames.condArray[treeSearchState->Add].size();i++)
			{
				count_.pause[condTables->pauseGames.condArray[treeSearchState->Add][i]]+=1;
			}

		}
	}
}


bool smfigapl::CheckGamePlan::gamePlanCheckMaxGamesSameClub(bool init)
{
	bool check =true;
	for (uint i=0;i<condTables->atomicCapacityArray[atomicIndex_].NTeams;i++)
	{
		if (count_.sameClub[i]>input->conditions.maxGamesSameClub.values[i])
		{
			check=false;
		}
	}

	#ifdef SM_DBG
	if (input->dbg&DBG_COND)
	{
		std::cout<<std::endl<<"scc: ";
		for (uint i = 0;i<condTables->atomicCapacityArray[atomicIndex_].NTeams;i++)
		{
			std::cout<<count_.sameClub[i] <<" ";
		}
		std::cout<<std::endl;
	}
	#endif

	return check;
};


bool smfigapl::CheckGamePlan::gamePlanCheckMaxPauseGames()
{
	bool check =true;

	for (uint i=0;i<condTables->atomicCapacityArray[atomicIndex_].NTeams;i++)
	{
		if (count_.pause[i]>input->conditions.maxPauseGames.value)
		{
			check=false;
		}
	}
	#ifdef SM_DBG
	if (input->dbg&DBG_COND)
	{
		std::cout<<std::endl<<"pc: ";
		for (uint i = 0;i<condTables->atomicCapacityArray[atomicIndex_].NTeams;i++)
		{
			std::cout<<count_.pause[i] <<" ";
		}
		std::cout<<std::endl;
	}
	#endif
	return check;
}


bool smfigapl::CheckGamePlan::gamePlanCheckFieldTypes()
{
	bool check = true;
	#ifdef SM_MAX_FT_JUMP
	if (ftJump_)
	{
		for (uint l = 0; l < condTables->atomicCapacityArray[atomicIndex_].NFieldTypes; l++)
			std::fill(checkMax_[l].begin(),checkMax_[l].end(),false);
		ftJump_=false;
	}
	#endif
	for (uint m = 0; m < condTables->atomicCapacityArray[atomicIndex_].NTeams; m++)
	{
		for (uint l = 0; l < condTables->atomicCapacityArray[atomicIndex_].NFieldTypes; l++)
		{
			if (input->conditions.limGamesFieldType.values.min[l] > condTables->atomicCapacityArray[atomicIndex_].NRounds- *treeSearchLevel-1+count_.fieldType[m][l])
			{
				check=false;
			}
			else if (input->conditions.limGamesFieldType.values.max[l] < count_.fieldType[m][l])
			{
				#ifdef SM_MAX_FT_JUMP
				checkMax_[l][m]=true;
				ftJump_=true;
				#endif
				check=false;
			}
		}
	}
	#ifdef SM_MAX_FT_JUMP
	//calc jump
	if (ftJump_)
	{
	for (uint l=0;l<(condTables->atomicCapacityArray[atomicIndex_].NFieldTypes);l++)
		for (uint m = 0; m < condTables->atomicCapacityArray[atomicIndex_].NTeams; m++)
		{
			if (treeSearchState->Add>=0 && checkMax_[l][m] )
			{
				res_.jump=std::max(res_.jump,condTables->fieldType.jumpArray[treeSearchState->Add][l][m]);
			}
		}
	}
	#endif
	return check;
}


smfigapl::CheckResult smfigapl::CheckGamePlan::gamePlanCheck(bool init)
{

		res_.jump=1;
		res_.check=false;
		this->updateCounters(init);

		if (!gamePlanCheckRepeatedGames(init))
				{
					#ifdef SM_DBG
					if (input->dbg&DBG_COND)
						std::cout<<"rgF ";
					NFailedArray_.repeatedGames++;
					if (treeSearchState->Add>=0)
						NFailedArray_.searchIndexFalse[treeSearchState->Add]=true;
					#endif
					return res_;
				}

		if (input->conditions.maxGamesSameClub.enabled)
			if (!gamePlanCheckMaxGamesSameClub(init))
			{
				#ifdef SM_DBG
				if (input->dbg&DBG_COND)
					std::cout<<"scF ";
					NFailedArray_.sameClub++;
					if (treeSearchState->Add>=0)
						NFailedArray_.searchIndexFalse[treeSearchState->Add]=true;
				#endif
				return res_;
			}

		if (!gamePlanCheckMaxPauseGames())
		{
			#ifdef SM_DBG
			if (input->dbg&DBG_COND)
				std::cout<<"pgF ";
				NFailedArray_.pauseGames++;
				if (treeSearchState->Add>=0)
					NFailedArray_.searchIndexFalse[treeSearchState->Add]=true;
			#endif
			return res_;
		}

		if (input->conditions.limGamesFieldType.enabled)
			if (!gamePlanCheckFieldTypes())
				{
				#ifdef SM_DBG
				if (input->dbg&DBG_COND)
					std::cout<<"ft ";
					NFailedArray_.fieldTypes++;
					if (treeSearchState->Add>=0)
						NFailedArray_.searchIndexFalse[treeSearchState->Add]=true;
				#endif
				return res_;
				}
			#ifdef SM_DBG
			if (input->dbg&DBG_COND)
				std::cout<<"ok ";
			#endif
		res_.check=true;
		#ifdef SM_DBG
		if (treeSearchState->Add>=0)
			NFailedArray_.searchIndex[treeSearchState->Add]=false;
		#endif
		return res_;
}


#ifdef SM_DBG
void smfigapl::CheckGamePlan::dispNFailArray()
{
	std::cout<<"Faild Checks:"<<std::endl;
	std::cout <<"repeatedGames: " <<NFailedArray_.repeatedGames<<std::endl;
	std::cout <<"sameClub: " <<NFailedArray_.sameClub<<std::endl;
	std::cout <<"pauseGames: " <<NFailedArray_.pauseGames<<std::endl;
	std::cout <<"fieldTypes: " <<NFailedArray_.fieldTypes<<std::endl;
	std::cout <<"Always failed: "<<std::endl;
	for (uint i=0;i<NFailedArray_.searchIndex.size();i++)
		if (NFailedArray_.searchIndex[i]&&NFailedArray_.searchIndexFalse[i])
			std::cout<<i<< " ";
	std::cout <<std::endl;
}
#endif

