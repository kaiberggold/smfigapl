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

#include <iostream>
#include <vector>
#include <string>

#include "sm_in.hpp"
#include "sm_except.hpp"

void smfigapl::SmInput::setNRounds(uint n)
{
	setup.realCapacity.NRounds=n;
}

void smfigapl::SmInput::setNRealFields(uint n)
{
	setup.realCapacity.NFields=n;
	limGamesFieldTypeInit();
	setup.mapping.indexedFieldTypeByFieldId.resize(setup.realCapacity.NFields);
}

void smfigapl::SmInput::initClubNames(std::vector<std::string> s)
{
	if (s.size()<smfigapl::MAX_CLUBS)
	{
		setup.realCapacity.NClubs = static_cast<uint>(s.size());
		setup.clubNameByClubId = s;
	}
	else
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
}

smfigapl::uint smfigapl::SmInput::addClubNameIfNew(std::string s)
{
	if (setup.realCapacity.NClubs<smfigapl::MAX_CLUBS)
	{
		auto p=std::find(setup.clubNameByClubId.begin(), setup.clubNameByClubId.end(), s);
		if ( p!= setup.clubNameByClubId.end())
			return static_cast<uint>(std::distance(setup.clubNameByClubId.begin(), p));//allready contained
		else
		{
			setup.clubNameByClubId.push_back(s);
			setup.clubIdByClubName[s]=setup.realCapacity.NClubs;
			setup.realCapacity.NClubs++;
			return setup.realCapacity.NClubs-1;
		}
	}
	else
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
		return 0;
}

smfigapl::uint smfigapl::SmInput::addIfNew(std::string s, uint max, uint valId,std::vector<std::string> *array, uint *counter)
{
		if (*counter>max)
		{
			std::vector<std::string> temp;
			temp.resize(1);
			temp[0]=std::to_string(max);
			#ifdef SM_DBG
				throw SmValueException("smfigapl::SmInput::addIfNew",valId,temp,this);
			#else
				throw SmValueException(valId,temp,this);
			#endif
		}
		auto p=std::find(array->begin(), array->end(), s);
		if ( p!= array->end())
			return static_cast<uint>(std::distance(array->begin(), p));//allready contained
		else
		{
			(*counter)++;
			array->push_back(s);
			return *counter-1;
		}
}

smfigapl::uint smfigapl::SmInput::addTeamColorIfNew(std::string s)
{
	if (s.size()<smfigapl::MAX_TEAMCOLORS)
	{
		auto p = std::find(setup.colorNameByColorId.begin(), setup.colorNameByColorId.end(), s);
		if (p != setup.colorNameByColorId.end())
			return static_cast<uint>(std::distance(setup.colorNameByColorId.begin(), p));//allready contained
		else
		{
			setup.colorNameByColorId.push_back(s);
			setup.colorIdByColor[s]=setup.realCapacity.NTeamColors;
			setup.realCapacity.NTeamColors++;
			return setup.realCapacity.NTeamColors-1;
		}
	}
	else
		setup.validation=smfigapl::VAL_ERROR_TEAM_COLOR_INDEX_GT_MAX;
		return 0;
}

void smfigapl::SmInput::initTeamColors(std::vector<std::string> s)
{
	if (s.size()<smfigapl::MAX_TEAMCOLORS)
	{
		setup.realCapacity.NTeamColors = static_cast<uint>(s.size());
		setup.colorNameByColorId = s;
	}
	else
		setup.validation=smfigapl::VAL_ERROR_TEAM_COLOR_INDEX_GT_MAX;
}

void smfigapl::SmInput::addField(std::string type, std::string subType,std::string name)
{
	{
		uint ft=addIfNew(type,MAX_FIELDS,VAL_ERROR_NFIELDS_GT_MAX,&setup.fieldTypeNameById,&setup.realCapacity.NFieldTypes);
		uint fst=addIfNew(subType,MAX_FIELDS,VAL_ERROR_NFIELDS_GT_MAX,&setup.fieldSubTypeNameById,&setup.realCapacity.NFieldSubTypes);
		setup.mapping.indexedFieldTypeByFieldId.push_back(ft);
		setup.mapping.indexedFieldSubTypeByFieldId.push_back(fst);
		setup.fieldSubTypeIdBySubTypeName[subType]=fst;
		setup.fieldNameByFieldId.push_back(name);
		setup.fieldTypeNameByFieldId.push_back(type);
		setup.fieldSubTypeNameByFieldId.push_back(subType);
		setup.realCapacity.NFields++;
		limGamesFieldTypeInit();
	}
}


void smfigapl::SmInput::initFieldTypes(std::vector<std::string> s)
{
	if (s.size()<smfigapl::MAX_FIELDTYPES)
	{
		setup.realCapacity.NFieldTypes = static_cast<uint>(s.size());
		setup.fieldTypeNameByFieldId = s;
	}
	else
		setup.validation=smfigapl::VAL_ERROR_TEAM_COLOR_INDEX_GT_MAX;
}

void smfigapl::SmInput::setFieldType(smfigapl::uint i, smfigapl::uint ft)
{
	if (i <setup.realCapacity.NFields && ft <setup.realCapacity.NFieldTypes)
		setup.mapping.indexedFieldTypeByFieldId[i] = ft;
	else
		setup.validation=smfigapl::VAL_ERROR_FIELD_TYPE_INDEX_GT_MAX;
}

void smfigapl::SmInput::addTeam(std::string name, std::string color)
{
	uint clubId=addClubNameIfNew(name);
	uint colorId=addTeamColorIfNew(color);
	addTeam(clubId, colorId);
	setup.teamIdArray.resize(setup.clubNameByClubId.size());
	for (auto && i : setup.teamIdArray)
		i.resize(setup.colorNameByColorId.size());
	setup.teamIdArray[clubId][colorId]=setup.realCapacity.NTeams-1;
	conditions.fieldSubTypeDeny.values.resize(setup.realCapacity.NTeams);
		for (auto && i : conditions.fieldSubTypeDeny.values)
			i.resize(setup.realCapacity.NFieldSubTypes);
}
void smfigapl::SmInput::addTeam(smfigapl::uint cl)
{
	if (cl>MAX_CLUBS)
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
	else if (cl>setup.realCapacity.NClubs)
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
	else
	{
		uint count = 0;
		for (uint i = 0; i < setup.realCapacity.NTeams; i++)
		{
			if (setup.mapping.indexedClubByTeamId[i] == cl)
				count++;
		}
		setup.mapping.indexedClubByTeamId.push_back(cl);
		setup.mapping.indexedColorByTeamId.push_back(count);
		setup.realCapacity.NTeams++;
		smfigapl::NamedTeam  team;
		team.setTeamClub(setup.clubNameByClubId[cl]);
		team.setTeamColor(setup.colorNameByColorId[count]);
		setup.namedTeamByTeamId.push_back(team);
		maxGamesSameClubInit();
		limGamesFieldTypeInit();
	}
}

void smfigapl::SmInput::addTeam(uint cl, uint co)
{
	if (cl>MAX_CLUBS)
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
	else if (cl>setup.realCapacity.NClubs)
		setup.validation=smfigapl::VAL_ERROR_CLUB_INDEX_OVERFLOW;
	else
	{
		setup.mapping.indexedClubByTeamId.push_back(cl);
		setup.mapping.indexedColorByTeamId.push_back(co);
		setup.realCapacity.NTeams++;
		smfigapl::NamedTeam  team;
		team.setTeamClub(setup.clubNameByClubId[cl]);
		team.setTeamColor(setup.colorNameByColorId[co]);
		setup.namedTeamByTeamId.push_back(team);
		maxGamesSameClubInit();
		limGamesFieldTypeInit();
	}
}

void smfigapl::GamePlanConditions::setMaxPauseGames(uint i)
{
	maxPauseGames.value = i;
	maxPauseGames.enabled=true;
}

void smfigapl::GamePlanConditions::setMaxFieldsEmpty(uint i)
{
	maxFieldsEmpty = i;
}

void smfigapl::GamePlanConditions::setmaxGamesSameClub(uint i)
{
	fill(maxGamesSameClub.values.begin(), maxGamesSameClub.values.end(), i);
	maxGamesSameClub.enabled=true;
}

void smfigapl::GamePlanConditions::setMinGamesFieldType(uint ft, uint i)
{
	limGamesFieldType.values.minRaw[ft] = i;
	limGamesFieldType.values.min=limGamesFieldType.values.minRaw;
	limGamesFieldType.enabled=true;
}

void smfigapl::SmInput::setMinGamesFieldType(std::string ft, uint i)
{
	auto p=std::find(setup.fieldTypeNameByFieldId.begin(), setup.fieldTypeNameByFieldId.end(), ft);
	if ( p!= setup.fieldTypeNameByFieldId.end())
	{
		conditions.limGamesFieldType.values.minRaw[std::distance(setup.fieldTypeNameByFieldId.begin(), p)] = i;
		conditions.limGamesFieldType.values.min=	conditions.limGamesFieldType.values.minRaw;
		conditions.limGamesFieldType.enabled=true;
	}
	else
	{
		//Error
	}
}

void smfigapl::SmInput::setMaxGamesFieldType(std::string ft, uint i)
{
	auto p=std::find(setup.fieldTypeNameByFieldId.begin(), setup.fieldTypeNameByFieldId.end(), ft);
	if ( p!= setup.fieldTypeNameByFieldId.end())
	{
		conditions.limGamesFieldType.values.maxRaw[std::distance(setup.fieldTypeNameByFieldId.begin(), p)] = i;
		conditions.limGamesFieldType.values.max=	conditions.limGamesFieldType.values.maxRaw;
		conditions.limGamesFieldType.enabled=true;
	}
	else
	{
		//Error
	}
}

void smfigapl::SmInput::maxGamesSameClubInit()
{
	conditions.maxGamesSameClub.values.resize(setup.realCapacity.NTeams);
}

void smfigapl::SmInput::limGamesFieldTypeInit()
{
	conditions.limGamesFieldType.values.min.resize(setup.realCapacity.NFieldTypes);
	conditions.limGamesFieldType.values.max.resize(setup.realCapacity.NFieldTypes);
	conditions.limGamesFieldType.values.minRaw.resize(setup.realCapacity.NFieldTypes);
	conditions.limGamesFieldType.values.maxRaw.resize(setup.realCapacity.NFieldTypes);
}
