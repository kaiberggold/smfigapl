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

#ifndef SMSEARCH
#define SMSEARCH

#include <iostream>
#include <vector>
#include <string>
#include <limits>

#include "sm_cond.hpp"
#include "sm_in.hpp"
#include "sm_misc.hpp"
#include "sm_out.hpp"
#include "sm_ts.hpp"


namespace smfigapl{

class GamePlanSearch
{
	private:
		smfigapl::SmInput *input;
		smfigapl::SmOutput *output;
		smfigapl::IndexedRound indexedRound_;
		smfigapl::CheckGamePlan *check;
		smfigapl::TreeSearch *ts;
		smfigapl::CondTables condTables;
		uint treeSearchLevel;
		uint treeSearchLevelOld_;
		TreeSearchState treeSearchState;
		bool pauseClubs_;
		bool stepsDisplay_;
		CheckResult lastCheck_;
		uint NFieldTypes_;
		uint maxSteps_;
		uint stepSize_;
		uint NPairingArrayFull_;
		uint NAtomicSets_;
		ResultArray resultArray_;
		std::vector<smfigapl::IndexedRound> indexedPairingArrayFull_;
		std::vector<uint> TeamsSortArray_;
		std::vector<std::vector<uint>> searchArray_;
		std::vector<bool> pause_;
		std::vector<uint> treeSearchIndex_;
		std::vector<std::vector<uint>> jumpArray_;
		std::vector<std::vector<uint>> pairingIndexShortArray_;
		std::vector<uint> atomicIndexByFieldId_;
		std::vector<uint> atomicIndexByTeamId_;



	public:
		class AdjescentRowsCheck
			{
			public:
					bool pauseCheck;
					bool pairingCheck;
					bool passed()
					{
						return pauseCheck && pairingCheck;
					}
			};

		GamePlanSearch (smfigapl::SmInput *in, smfigapl::SmOutput *out):input{in},output{out}
		{
			indexedPairingArrayFull_.resize(0);
			treeSearchLevel=0;
			treeSearchLevelOld_=0;
			#ifdef SM_DBG
			ts =new TreeSearch(input->dbg);
			#else
			ts =new TreeSearch();
			#endif
			check = new smfigapl::CheckGamePlan(input, &treeSearchLevel, &condTables, &treeSearchState);
			NFieldTypes_=0;
			pauseClubs_=0;
			maxSteps_=std::numeric_limits<uint>::max();
			stepsDisplay_=false;
			stepSize_=1000;
			NPairingArrayFull_=0;
			atomicIndexByFieldId_.clear();
			atomicIndexByTeamId_.clear();
			NAtomicSets_=0;
		}
		~GamePlanSearch()
		{
			delete check;
			delete ts;

		}
		void init();
		void preInit();
		void searchIndexMaxInit();
		void searchIndexMaxUpdate(uint i);
		void calcIndexedGamePlan();
		void setGamePlanRound(GamePlanRoundByTeamId uintRound);
		std::string getClubNameByTeamId(uint id);
		std::string getTeamColorByTeamId(uint id);
		uint getTeamClub(uint i);
		bool treeSearchIndexNext();
		void treeSearchIndexNextLevelDown();
		smfigapl::uint searchStep(bool init, smfigapl::uint oldResult);
		smfigapl::ResultArray  gpSearch();
		void initGamePlan(smfigapl::SmGamePlanSetup *input,NamedGamePlan *tournament);
		void calcNamedGamePlan();
		smfigapl::GamePlanByTeamId gamePlanBackSort(smfigapl::GamePlanByTeamId  *in);
		void initCondRepeatedGames();
		void displayTreeSearchIndex();
		void initSameClubArray ();
		void initFieldTypeArray ();
		void initPauseArray ();
		void initIndexedPairingArrayFull();
		void pairingArraySort(uint start, uint end);
		bool isImpossibleIndex();
		void initImpossibleIndexJumpArray();
		void setMaxSteps(uint m);
		void setStepsDisplay(bool ena, uint size);
		uint  getSteps();
		void balancedHomeAwayCalc();
		AdjescentRowsCheck checkAdjacentRows(uint round, uint atomicIndex);
		void dispNFailArray();
		void calcPairingIndexShortArray();
		void optimizeRounds();
		void calcAtomicFieldSets();
};
} //namespace
#endif
