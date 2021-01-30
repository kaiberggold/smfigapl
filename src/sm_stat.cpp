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

#include "../include/sm_stat.hpp"

smfigapl::uint smfigapl::GamePlanStatistic::calcNGames(uint teamId)
{
	uint count = 0;
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			for (uint k = 0; k < 2; k++)
			{
				if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()== \
						teamId &&output->indexedGamePlan.getGame(i,j).getTeambyGameId(1-k).getTeamId()<input->setup.realCapacity.NTeams)
					count++;
			}
		}
	}
	return count;
}

smfigapl::uint smfigapl::GamePlanStatistic::calcNRepeatedGames(uint teamId)
{
	std::vector<uint> other;
	other.clear();
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			for (uint k = 0; k < 2; k++)
			{
				if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()== \
						teamId &&output->indexedGamePlan.getGame(i,j).getTeambyGameId(1-k).getTeamId()<input->setup.realCapacity.NTeams)
					other.push_back(output->indexedGamePlan.getGame(i,j).getTeambyGameId(1-k).getTeamId());
			}
		}
	}
	std::sort(other.begin(), other.end());
	uint count=0;
	if (other.size()>1)
	{
		for (uint i=0;i<other.size()-1;i++)
			if (other[i]==other[i+1])
				count++;
	}
	else
	{
		count=0;
	}

	return count;
}

smfigapl::uint smfigapl::GamePlanStatistic::calcMaxGamesSameTeam(uint teamId)
{
	std::vector<uint> other;
	other.clear();
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			for (uint k = 0; k < 2; k++)
			{
				if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()== \
						teamId &&output->indexedGamePlan.getGame(i,j).getTeambyGameId(1-k).getTeamId()<input->setup.realCapacity.NTeams)
					other.push_back(output->indexedGamePlan.getGame(i,j).getTeambyGameId(1-k).getTeamId());
			}
		}
	}
	std::sort(other.begin(), other.end());
	uint count=1;
	uint countMax=1;
	if (other.size()>1)
	{
		for (uint i=0;i<other.size()-1;i++)
		{
			if (other[i]==other[i+1])
			{
				count++;
				if (count>countMax)
					countMax=count;
			}
			else
			{
				count=1;
			}
		}
	}
	else
	{
		countMax=1;
	}

	return countMax;
}

smfigapl::uint smfigapl::GamePlanStatistic::calcNGamesFieldType(smfigapl::uint teamId, smfigapl::uint ft)
{
	uint count = 0;
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			for (uint k = 0; k < 2; k++)
			{
				if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId() == teamId &&(!output->indexedGamePlan.getGame(i,j).getPause()))
				{
					if (input->setup.mapping.indexedFieldTypeByFieldId[j]==ft)
						count++;
				}
			}
		}
	}
	return count;
}

smfigapl::uint smfigapl::GamePlanStatistic::calcNGamesSameClub(uint teamId)
{
	uint count = 0;
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
		{
			if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(0).getTeamClub()==output->indexedGamePlan.getGame(i,j).getTeambyGameId(1).getTeamClub())
			{
				for(uint k=0;k<2;k++)
				{
					if (output->indexedGamePlan.getGame(i,j).getTeambyGameId(k).getTeamId()==teamId)
					{
						count++;								}
					}
			}
		}
	}
	return count;
}

smfigapl::uint smfigapl::GamePlanStatistic::calcNHomeAway(uint team, bool home)
{
	uint count = 0;
	for (uint i = 0; i < input->setup.realCapacity.NRounds; i++)
	{
		if ((output->indexedGamePlan.isHome(i, team)&& home)||(output->indexedGamePlan.isAway(i, team)&& !home))
		{
			count++;
		}
	}
	return count;
}


void smfigapl::GamePlanStatistic::calcStatistic()
{
	statKeys_={"n_games", "n_same_pairing","max_games_same_team", "same_club", "home_games", "away_games"};
	for (uint j = 0; j < input->setup.realCapacity.NFieldTypes; j++)
		statKeys_.push_back("field_types_"+std::to_string(j));

	std::vector<uint> row;
	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcNGames(i));
	}
	statisticArray_["n_games"]=row;

	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcNRepeatedGames(i));
	}
	statisticArray_["n_same_pairing"]=row;

	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcMaxGamesSameTeam(i));
	}
	statisticArray_["max_games_same_team"]=row;

	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcNGamesSameClub(i));
	}
	statisticArray_["same_club"]=row;

	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcNHomeAway(i,true));
	}
	statisticArray_["home_games"]=row;
	row.clear();
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		row.push_back(calcNHomeAway(i,false));
	}
	statisticArray_["away_games"]=row;
	row.clear();
	for (uint j = 0; j < input->setup.realCapacity.NFieldTypes; j++)
	{
		row.clear();
		for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
		{
			row.push_back(calcNGamesFieldType(i,j));
		}
		statisticArray_["field_types_"+std::to_string(j)]=row;
	}
	statHeader_["n_games"]=input->setup.lang.get("n_games");
	for (uint j = 0; j < input->setup.realCapacity.NFieldTypes; j++)
		statHeader_["field_types_"+std::to_string(j)]=input->setup.fieldTypeNameByFieldId[j];
	statHeader_["same_club"]=input->setup.lang.get("same_club");
	statHeader_["home_games"]=input->setup.lang.get("home_games");
	statHeader_["away_games"]=input->setup.lang.get("away_games");
	statHeader_["n_same_pairing"]=input->setup.lang.get("n_same_pairing");
	statHeader_["max_games_same_team"]=input->setup.lang.get("max_games_same_team");

}




