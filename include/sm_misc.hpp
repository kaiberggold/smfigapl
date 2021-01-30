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

#ifndef SMMISC
#define SMMISC

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <map>
#include <unordered_map>


namespace smfigapl{

typedef unsigned int uint;
typedef unsigned int ColorId;
typedef unsigned int ClubId;

//
static const uint NSTEPSDISPLAY=1000;

//Error Codes
static const uint VAL_UNVALIDATED=0;
static const uint VAL_VALIDATED=1;
static const uint 	TOML_PARSE_ERROR=1;
static const uint NROUNDS_NOT_SET=10;
static const uint NO_FIELDS_DEFINED=11;
static const uint NO_CLUBS_DEFINED=12;
static const uint VAL_ERROR_CLUB_NOT_EXISTANT=100; //addTeam with a Club index large than defined Clubs
static const uint VAL_ERROR_CLUB_INDEX_GT_MAX=101; //Club Index > MAX_CLUBS
static const uint VAL_ERROR_CLUB_INDEX_OVERFLOW=102; //Club Index > NClubs
static const uint 	VAL_ERROR_TEAM_COLOR_INDEX_GT_MAX=103; //Team Color  > MAX_TEAMCOLORS
static const uint 	VAL_ERROR_FIELD_TYPE_INDEX_GT_MAX=104; //n Fieldtype > MAX_FIELDTYPES
static const uint 	VAL_ERROR_NFIELDS_GT_MAX=105; //n Fields > MAX_FIELDTYPES
static const uint   VAL_ERROR_NFIELDS_LT_1=106;
static const uint 	VAL_ERROR_NROUNDS_GT_MAX=107; //NROUNDS > MAX_ROUNDS
static const uint 	VAL_ERROR_NROUNDS_LT_1=108; //NROUNDS < 1

static const uint 	INPUT_ERROR_MAX_LT_MIN=200;

const uint DBG_TS=1;
const uint DBG_GPS=2;
const uint DBG_COND=4;
const uint DBG_FILE=8;
const uint DBG_STEPS_DETAILS=16;
// Constant declarations
static const uint MAX_CLUBS=16;
static const uint MAX_ROUNDS=10;
static const uint MAX_FIELDS=10;
static const  int MAX_TEAMCOLORS=16;
static const uint MAX_FIELDTYPES=4;
static const uint TEAM_UNDEFINED=255;

// Enums
static const int TS_SUCCESS=0;
static const int TS_NEXT_STEP=1;
static const int TS_LEVEL_UP=2;
static const int TS_FAILED=-1;
static const int TS_FAILED_2=-2;
static const int TS_FAILED_MAX_STEPS=-3;

class SmDateTimeValue
{
	public:
		uint year;
		uint month;
		uint day;
		uint hour;
		uint minute;
		uint second;
		SmDateTimeValue()
		{
			year=0;
			month=0;
			day=0;
			hour=0;
			minute=0;
			second=0;
		}

};

class SmPos
{
public:
	uint x;
	uint y;
	SmPos()
	{
		x=0;
		y=0;
	}
};

class SmPosRel
{
public:
	int xRel;
	int yRel;
	SmPosRel()
	{
		xRel=0;
		yRel=0;
	}
};


class SmTextField: public SmPosRel
{
public:
	std::string text;
	SmTextField()
	{
		xRel=0;
		yRel=0;
		text="";
	}
};

class SmHeader: public SmPosRel
{
public:
	std::string text;
	SmHeader()
	{
		xRel=0;
		yRel=0;
		text="";
	}
};

class SmDateTime: public SmPosRel
{
	public:
		std::string formatString;
		SmDateTime()
		{
			xRel=0;
			yRel=0;
			formatString="";
		}
};

class SmBasicInformation
{
	public:
		struct General
		{
			std::unordered_map<std::string,std::string> values;
		} general;
		struct DateTime
		{
			std::unordered_map<std::string,SmDateTimeValue> values;
		} dateTime;
};

class SmFormat
{
	public:
		struct Settings
		{
			bool separateTournaments;
			bool showTeams;
			bool showTeamsTextColors;
			bool teamsUseCollumn;
			bool statisticUseCollumn;
			bool showStatistic;
			bool showFilterCollumn;
			bool useTeamColor;
			uint fixedFields;
			uint fixedRounds;
			uint fixedTournaments;
			uint filterCollumn;
			std::vector<uint> fixedRows;
		} settings;

		struct InfoFields
		{
			 SmPos basePos;
			 SmPosRel resultMessage;
			 std::unordered_map<std::string,SmPosRel> general;//text taken from information class
			 std::unordered_map<std::string,SmDateTime> dateTime;
			 std::unordered_map<std::string,SmHeader> dateTimeHeader;
		} infoFields;

		struct Teams
				{
					SmPos basePos;
					SmTextField title;
					struct TextColors
					{
						std::vector<std::string> colors;
						SmPos posRel;
						SmTextField title;
					} textColors;

				} teams;

		struct GamePlan
		{
			SmPos basePos;
			SmTextField title;
		} gamePlan;

		struct Tournament
		{
			SmPosRel relPos;
			SmTextField title;
			struct Collums
			{
				SmPosRel relPos;
				std::unordered_map<std::string,SmHeader> col;
			} collumns;
		} tournament;

		struct Statistic
		{
			SmPos basePos;
			SmTextField title;
			struct Rows
			{
				SmPosRel relPos;
				std::unordered_map<std::string,SmHeader> row;
			} rows;
		} statistic;
		SmFormat()
		{
			settings.separateTournaments=false;
			settings.showStatistic=false;
			settings.showTeams=false;
			settings.showTeamsTextColors=false;
			settings.teamsUseCollumn=false;
			settings.statisticUseCollumn=false;
			settings.useTeamColor=false;;
			settings.fixedRows.clear();
			//TODO: init rest
		}
};

class Result
{
	public:
	uint steps;
	int stepResult;
	Result()
	{
		steps=0;
		stepResult=0;
	}
} ;
class ResultArray
{
	private:
		std::vector<Result> resultArray_;
	public:
	ResultArray()
	{
		resultArray_.clear();
	}
	ResultArray(uint size)
	{
		resultArray_.resize(size);
	}
	bool getResult();
	uint getResult(uint index);
	void setResult(uint index, uint res);
	void incStep (uint index);
	uint getStep (uint index);
	uint getTotalSteps ();
	void init (uint size);
} ;

class CheckResult
{
	public:
		bool check;
		uint jump;
};

class fieldSortable
{
	public:
		uint fieldType;
		uint fieldSubType;
		uint index;
};

class Capacity
{
	public:
		uint NRounds;
		uint NFields;
		uint NFieldTypes;
		uint NFieldSubTypes;
		uint NTeams;
		uint NClubs;
		uint NTeamColors;
	Capacity()
	{
		NRounds=0;
		NFields=0;
		NFieldTypes=0;
		NFieldSubTypes=0;
		NTeams=0;
		NClubs=0;
		NTeamColors=0;
	}
};

class Mapping
{
	public:
		std::vector<uint> indexedFieldTypeByFieldId;
		std::vector<uint> indexedFieldSubTypeByFieldId;
		std::vector<std::vector<uint>> fieldDenybySubTypeAndTeamId;
		std::vector<smfigapl::ClubId> indexedClubByTeamId;
		std::vector<smfigapl::ColorId> indexedColorByTeamId;
		Mapping()
		{
			indexedFieldTypeByFieldId.clear();
			indexedFieldSubTypeByFieldId.clear();
			fieldDenybySubTypeAndTeamId.clear();
			indexedClubByTeamId.clear();
			indexedColorByTeamId.clear();
		}
		void reset()
		{
			indexedFieldTypeByFieldId.clear();
			indexedFieldSubTypeByFieldId.clear();
			fieldDenybySubTypeAndTeamId.clear();
			indexedClubByTeamId.clear();
			indexedColorByTeamId.clear();
		}

};

bool fieldCompare(smfigapl::fieldSortable & l, smfigapl::fieldSortable & r);
void fieldSort(std::vector<fieldSortable> *fs);

class CondTables
{
	public:
		struct RepeatedGames
		{
			std::vector<uint> hashArray;
			std::vector<std::vector<uint>> condArray;
			std::vector<std::vector<uint>> jumpArray;
		} repeatedGames;
		struct SameClub
		{
			std::vector<std::vector<uint>> hashArray;
			std::vector<std::vector<uint>> condArray;
			std::vector<std::vector<uint>> jumpArray;
		} sameClub;
		struct pauseGames
		{
			std::vector<std::vector<uint>> hashArray;
			std::vector<std::vector<uint>> condArray;
			std::vector<std::vector<uint>> jumpArray;
		} pauseGames;
		struct fieldType
		{
			std::vector<std::vector<uint>> hashArray;
			std::vector<std::vector<std::vector<uint>>> condArray;
			std::vector<std::vector<std::vector<uint>>> jumpArray;
		} fieldType;

		std::vector<uint> searchIndexMax;
		std::vector<smfigapl::Capacity> atomicCapacityArray;
		std::vector<smfigapl::Mapping> atomicMappingArray;
		std::vector<std::vector<uint>> fieldMappingArray;
		std::vector<std::vector<uint>> teamsMappingArray;
		std::vector<uint> teamIdMappingArray;
		uint atomicIndex;
		CondTables()
		{
			searchIndexMax.clear();
			atomicCapacityArray.clear();
			atomicMappingArray.clear();
			repeatedGames.hashArray.clear();
			repeatedGames.condArray.clear();
			repeatedGames.jumpArray.clear();
			sameClub.hashArray.clear();
			sameClub.condArray.clear();
			sameClub.jumpArray.clear();
			pauseGames.hashArray.clear();
			pauseGames.condArray.clear();
			pauseGames.jumpArray.clear();
			fieldType.hashArray.clear();
			fieldType.condArray.clear();
			fieldType.jumpArray.clear();
			fieldMappingArray.clear();
			teamsMappingArray.clear();
			teamIdMappingArray.clear();
			atomicIndex=0;
		}
};


class TreeSearchState
{
	public:
		int Add;
		int Sub;
		int stepResult;
	TreeSearchState()
		{
			Add=0;
			Sub=0;
			stepResult=0;
		}
};

class CondCheckIf
{
public:
};

smfigapl::uint factorial(uint n);

uint getPairingIndexShort(uint NTeams, uint a, uint b);
uint getNPairingIndexShort(uint NTeams);
void displayPairingIndicesShort(uint NTeams);

class GamePlanRoundByTeamId
{
	private:
		std::vector<uint> gamePlanRoundByTeamId_;
	public:
		GamePlanRoundByTeamId()
		{
			gamePlanRoundByTeamId_.clear();
		}
		GamePlanRoundByTeamId(uint NFields)
		{
			gamePlanRoundByTeamId_.resize(NFields<<1);
		}
		void init(uint NFields);
		void reset();
		uint getTeamId(uint index);
		uint getTeamId(uint field, uint gameIndex);
		void setTeamId(uint index,uint teamId);
		void setTeamId(uint field,uint gameIndex,uint teamId);
		uint size();
};

class GamePlanByTeamId
{
	private:
		std::vector<GamePlanRoundByTeamId> gamePlanByTeamId_;
	public:
		GamePlanByTeamId(uint NRounds,uint NFields)
		{
			GamePlanRoundByTeamId temp(NFields);
			gamePlanByTeamId_.resize(NRounds,temp);
		}
		uint getTeamId(uint round,uint index);
		uint getTeamId(uint round,uint field, uint gameIndex);
		void setTeamId(uint round,uint index,uint teamId);
		void setTeamId(uint round,uint field,uint gameIndex,uint teamId);
		GamePlanRoundByTeamId getRound(uint round);

};

GamePlanRoundByTeamId roundFieldMix(GamePlanRoundByTeamId roundArray,std::vector<uint> mixArray);


class IndexedTeam
{
private:
	uint Team_;
	ClubId Club_;
	ColorId Color_;

public:
	IndexedTeam()
	{
		Team_=0;
		Club_=0;
		Color_=0;

	}

	bool operator==(IndexedTeam &other);

	void setTeamClub(ClubId cl);

	void setTeamColor(ColorId co);

	void setTeamId(uint teamId);

	ClubId getTeamClub();

	ColorId getTeamColor();

	uint getTeamId();

};

class IndexedGame
{
private:
	IndexedTeam teams_[2];
	bool Pause_;
	bool swap_;


public:
	IndexedGame();

	IndexedGame(IndexedTeam a, IndexedTeam b);

	//Return first (0) or second (1) team of a game
	IndexedTeam getTeambyGameId(uint gameIndex);

	//Return first (0) or second (1) team of a game
	bool gameCompare(IndexedGame b);

	void setTeam(uint gameIndex, IndexedTeam team);

	// Return the Game
	IndexedGame getGame();

	void setPause (bool p);

	bool getPause();

	void setSwap(bool sw);
	void flipSwap();
	bool getSwap();
	bool contains(uint team);


	// Set the Game
};

bool compare(smfigapl::IndexedGame & l, smfigapl::IndexedGame & r);

class IndexedRound
{
private:
	std::vector<IndexedGame> roundArray_;
	std::vector<IndexedTeam> pauseTeams_;
	uint  NFields_;
	uint index_;

public:
	IndexedRound(uint n):NFields_{n}
	{
		roundArray_.resize(NFields_);
		pauseTeams_.clear();
		index_=0;
	}
	IndexedRound()
	{
		roundArray_.clear();
		pauseTeams_.clear();
		NFields_=0;
		index_=0;
	}

	IndexedGame &operator[](size_t i)
	{
		return roundArray_[i];
	}

	void setGame(uint field,IndexedGame game);
	IndexedGame getGame(uint field);
	uint getNPauseTeams();
	IndexedTeam getPauseTeam (uint pauseIndex);
	std::vector<IndexedTeam> getPauseTeams ();
	bool isPauseTeam (uint team);
	void addPauseTeam(IndexedTeam team);
    void pauseTeamsClear();
    void sort();
    uint size();
    bool contains(IndexedGame game);
    void init(uint n);
    bool getHomeAwayState(uint team, bool home);
    bool isHome(uint team);
    bool isAway(uint team);
    bool isHomeAway(uint team,bool home);
    void flipSwap(uint field);
    void setIndex(uint i);
    uint  getIndex();
    smfigapl::uint getFieldIdbyTeamId(smfigapl::uint team);
};


class IndexedGamePlan
{
private:
	std::vector<std::vector<smfigapl::IndexedGame>> gamePlan_;
	std::vector<std::vector<smfigapl::IndexedTeam>> pauseTeams_;
	std::vector<std::vector<uint>> pauseTeamsAtomicIndex_;
	uint NRounds_;
	uint NFields_;
	uint NAtomic_;
	std::vector<uint> fieldTypeByFieldId_;
	std::vector<uint> atomicIndexArray_;
	std::vector<std::vector<uint>> atomicFieldMappingArray_;
	std::vector<std::vector<uint>> resortIndex_;

public:
	IndexedGamePlan()
	{
		NRounds_=0;
		NFields_=0;
		NAtomic_=0;
		gamePlan_.clear();
		atomicIndexArray_.clear();
		resortIndex_.clear();
		pauseTeams_.clear();
		pauseTeamsAtomicIndex_.clear();
		atomicFieldMappingArray_.clear();
	}
	IndexedGamePlan(uint n):NRounds_{n}
	{
		NAtomic_=0;
		NFields_=0;
		gamePlan_.resize(NRounds_);
		pauseTeams_.resize(NRounds_);
		pauseTeamsAtomicIndex_.resize(NRounds_);
		atomicIndexArray_.clear();
		std::vector<uint> temp;
		temp.resize(NRounds_);
		for (uint i=0;i<NRounds_;i++)
		{
			temp[i]=i;
		}
		resortIndex_.clear();
		resortIndex_.push_back(temp);
	}
	IndexedGamePlan(uint NRounds,uint NFields, uint NAtomic)
	{
		this->init(NRounds,NFields,NAtomic);
	}

  	void setGame(uint round, uint field,IndexedGame game);
	IndexedGame getGame(uint round, uint field);
	IndexedGame getGameAtomic(uint round, uint field,uint atomicIndex);
	void reset();
	void init(uint nr, uint nf,uint na);
	uint size();
    uint getFieldTypeByFieldId(uint field);
    uint getFieldTypeByFieldIdAtomic(uint field, uint atomicIndex);
    void setFieldTypeByFieldId(std::vector<uint> f);
	void addPauseTeamAtomic(uint round,IndexedTeam team,uint atomicIndex);
	void pauseTeamsClear(uint round);
	void flipSwap(uint round,uint field);
	void flipSwapAtomic(uint round,uint field,uint atomicIndex);
	bool getHomeAwayState(uint round, uint team, bool home);
	bool isHome(uint round, uint team);
    bool isAway(uint round, uint team);
	bool getHomeAwayStateAtomic(uint round, uint team, bool home, uint atomicIndex);
	bool isHomeAtomic(uint round, uint team,uint atomicIndex);
    bool isAwayAtomic(uint round, uint team,uint atomicIndex);
    bool isPauseTeam(uint round, uint team);
    uint getNPauseTeams(uint round);
    uint getNPauseTeamsAtomic(uint round,uint atomicIndex);
	IndexedTeam getPauseTeam (uint round,uint pauseIndex);
	IndexedTeam getPauseTeamAtomic (uint round,uint pauseIndex,uint atomicIndex);
	bool getPause(uint round, uint field);
	bool getPauseAtomic(uint round, uint field,uint atomicIndex);
    bool isHomeAway(uint round,uint team,bool home);
	void setResortIndex(uint NTeams, uint index,uint atomicIndex);
	void setAtomicFieldMappingArray(std::vector<std::vector<uint>> m);
	uint getAtomicIndex(uint field);
	uint getResortIndex(uint round,uint atomicIndex);
	uint getNAtomic();
	uint getNRounds(uint atomicIndex);
};

class NamedTeam
{
private:
	std::string teamClub;
	std::string teamColor;

public:
	NamedTeam();

	bool operator==(NamedTeam &other);

	std::string getTeamName();

	std::string getTeamColor();

	std::string getTeamClub();

	void setTeamColor(std::string s);

	void setTeamClub(std::string s);

};

class NamedRound
{
private:
	std::vector<std::vector<NamedTeam>> gamePlanGames_;
	std::vector<std::string> gamePlanPauseName_;
	std::vector<std::string> gamePlanPauseColor_;
	std::vector<uint> atomicIndexPause_;
	uint NFields_;

public:
	NamedRound(uint NFields);
	NamedRound()
	{
		NFields_=0;
		gamePlanGames_.clear();
		gamePlanPauseName_.clear();
		gamePlanPauseColor_.clear();
		atomicIndexPause_.clear();
	}

	void addPauseTeam(std::string name, std::string color, uint atomicIndex);
	void setTeamColor(uint i, uint j, std::string s);

	void setTeamClub(uint i, uint j, std::string s);

	NamedTeam getTeam(uint i, uint j);

	std::string getTeamName(uint i, uint j);

	std::string getPauseTeam(uint i);
	std::string getPauseTeamColor(uint i);
	uint getPauseTeamAtomicIndex(uint i);

	uint getNPauseTeams();
	void init(uint n);
};

class NamedGamePlan
{
	private:
		std::vector<NamedRound> rounds_;
	public:
		std::vector<uint> atomicIndexbyFieldId;
		NamedGamePlan()
		{
			rounds_.resize(0);
			atomicIndexbyFieldId.clear();
		}
		NamedGamePlan(uint nr)
		{
			rounds_.resize(nr);
			atomicIndexbyFieldId.clear();
		}
		NamedGamePlan(uint nr, uint nf)
		{

			NamedRound temp(nf);
			rounds_.resize(nr,temp);
			atomicIndexbyFieldId.clear();
		}

	void setTeamClub(uint round, uint field, uint gameIndex, std::string s);
	void setTeamColor(uint round, uint field, uint gameIndex, std::string s);
	void addPauseTeamAtomic(uint round,std::string name, std::string color,uint AtomicIndex);
	void addRound(NamedRound round);
	std::string getTeamClub(uint round, uint field, uint gameIndex);
	std::string getTeamColor(uint round, uint field, uint gameIndex);
	std::string getTeamFullName(uint round, uint field, uint gameIndex);
	NamedRound getRound(uint round);
	std::string getPauseTeam(uint round, uint pauseTeamIndex);
	std::string getPauseTeamColor(uint round, uint pauseTeamIndex);
	std::string getPauseTeamAtomic(uint round, uint pauseTeamIndex,uint atomicIndex);
	std::string getPauseTeamColorAtomic(uint round, uint pauseTeamIndex,uint atomicIndex);
	uint getNPauseTeams(uint round);
	uint getNPauseTeamsAtomic(uint round, uint atomicIndex);
	uint getNRounds();
	void init(uint n, uint m);
	void setAtomicIndexbyFieldId(std::vector<uint> in)
	{
		atomicIndexbyFieldId=in;
	}
};
}//namespace smfigapl
#endif

