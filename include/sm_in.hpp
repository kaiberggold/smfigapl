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

#ifndef SMINPUT
#define SMINPUT
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include "sm_global.hpp"
#include "sm_misc.hpp"
#include "sm_global.hpp"
#include "sm_lang.hpp"

namespace smfigapl{

class SmGamePlanSetup
{

	private:
	public:
		int validation;
		smfigapl::SmLang lang;
		smfigapl::Capacity realCapacity;
		smfigapl::Mapping mapping;

		std::vector<std::vector<uint>> teamIdArray;

		std::map<std::string,uint> colorIdByColor;
 		std::vector<std::string> fieldTypeNameByFieldId;
		std::vector<std::string> clubNameByClubId;
		std::map<std::string,uint> clubIdByClubName;
		std::vector<std::string> colorNameByColorId;
		std::vector<std::string> fieldSubTypeNameByFieldId;
		std::vector<std::string> fieldNameByFieldId;
		std::vector<std::string> fieldSubTypeNameById;
		std::vector<std::string> fieldTypeNameById;
		std::vector<smfigapl::NamedTeam> namedTeamByTeamId;
		std::map<std::string,uint> fieldSubTypeIdBySubTypeName;


	SmGamePlanSetup()
	{
		namedTeamByTeamId.resize(0);
		validation=VAL_UNVALIDATED;
		clubNameByClubId.resize(0);
		colorNameByColorId.resize(0);
		teamIdArray.clear();
		fieldTypeNameByFieldId.resize(0);
		fieldSubTypeNameByFieldId.resize(0);
		fieldNameByFieldId.resize(0);
		fieldSubTypeNameById.resize(0);
		fieldTypeNameById.resize(0);
		fieldSubTypeIdBySubTypeName.clear();
		colorIdByColor.clear();
		clubIdByClubName.clear();
		lang.setLanguage(SM_DEFAULT_LANGUAGE);
	}
};

class GamePlanConditions
{
	public:
		uint iterations;
		bool repeatPairings;
		bool homeAwayBalance;
		bool optimizeRounds;
		struct{
			bool enabled;
			struct{
			std::vector<uint> minRaw;
			std::vector<uint> min;
			std::vector<uint> maxRaw;
			std::vector<uint> max;
			} values;
		} limGamesFieldType;
		struct{
			bool enabled;
			uint value;
		} maxPauseGames;
		uint extraPauseFields;
		struct{
			bool enabled;
			std::vector<uint> values ;
		} maxGamesSameClub;
		uint maxFieldsEmpty;
		struct{
			bool enabled;
			std::vector<std::vector<uint>> values ;
		} fieldSubTypeDeny;

		GamePlanConditions()
		{
			iterations=1;
			repeatPairings=false;
			homeAwayBalance=true;
			optimizeRounds=true;
			extraPauseFields=0;
			maxFieldsEmpty=0;
			maxPauseGames.enabled=false;
			//TODO: Init
			limGamesFieldType.enabled=false;
			limGamesFieldType.values.min.resize(0);
			limGamesFieldType.values.max.resize(0);
			limGamesFieldType.values.minRaw.resize(0);
			limGamesFieldType.values.maxRaw.resize(0);
			//TODO: Init
			setMaxPauseGames(0);
			maxGamesSameClub.enabled=false;
			maxGamesSameClub.values.resize(0);
			fieldSubTypeDeny.values.resize(0);
			fieldSubTypeDeny.enabled=false;
		}
		void setMaxPauseGames(uint i);
		void setMinGamesFieldType(uint ft, uint i);

		void setmaxGamesSameClub(uint i);
		void setMaxFieldsEmpty(uint i);
};

class SmInput
{
private:

public:
	SmGamePlanSetup setup;
	GamePlanConditions conditions;
	SmBasicInformation info;
	SmFormat format;

	#ifdef SM_DBG
		uint dbg;
	#endif

	void setNRounds(uint n);
	void setNRealFields(uint n);
	void initClubNames(std::vector<std::string> s);
	void initTeamColors(std::vector<std::string> s);
	uint addClubNameIfNew(std::string s);
	uint addTeamColorIfNew(std::string s);
	uint addIfNew(std::string s, uint max, uint valId,std::vector<std::string> *array, uint *counter);

	void initFieldTypes(std::vector<std::string> s);
	void setFieldType(uint i, uint ft);
	void addTeam(uint cl);
	void addTeam(uint cl, uint co);
	void addTeam(std::string name, std::string color);
	void addField(std::string type, std::string subType,std::string name);
	void maxGamesSameClubInit();
	void limGamesFieldTypeInit();
	void setMinGamesFieldType(std::string ft, uint i);
	void setMaxGamesFieldType(std::string ft, uint i);
};

} //namespace smGamePlan

#endif
