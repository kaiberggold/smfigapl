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

#ifndef SMGPCHECK
#define SMGPCHECK

#include <iostream>
#include <vector>
#include <string>

#include "sm_in.hpp"
#include "sm_misc.hpp"

namespace smfigapl{

class CheckGamePlan
{
	private:
		smfigapl::SmInput *input;
		smfigapl::CondTables *condTables;
		smfigapl::TreeSearchState *treeSearchState;
		uint *treeSearchLevel;
		CheckResult res_;
		struct{
			std::vector<uint> pairing;
			std::vector<uint> sameClub;
			std::vector<uint> pause;
			std::vector<std::vector<uint>> fieldType;
		} count_;
		std::vector<bool> jumpCheck_;
		std::vector<bool> checkArray_;
		uint NPairingIndexShort_;
		std::vector<std::vector<bool>> checkMax_;
		uint atomicIndex_;
		#ifdef SM_MAX_FT_JUMP
		bool ftJump_;
		#endif
		#ifdef SM_DBG
		struct{
			uint repeatedGames;
			uint sameClub;
			uint pauseGames;
			uint fieldTypes;
			std::vector<bool> searchIndex;
			std::vector<bool> searchIndexFalse;
			} NFailedArray_;
		#endif


	public:
		CheckGamePlan(smfigapl::SmInput *in, uint *ts, smfigapl::CondTables *tb, smfigapl::TreeSearchState *tc):\
		input{in}, treeSearchLevel{ts},condTables{tb}, treeSearchState{tc}
		{
			NPairingIndexShort_=0;
			res_.check=false;
			count_.pairing.resize(0);
			count_.sameClub.resize(0);
			count_.pause.resize(0);
			count_.fieldType.resize(0);
			#ifdef SM_DBG
			NFailedArray_.fieldTypes=0;
			NFailedArray_.pauseGames=0;
			NFailedArray_.repeatedGames=0;
			NFailedArray_.sameClub=0;
			#endif
			#ifdef SM_MAX_FT_JUMP
			ftJump_=false;
			atomicIndex_=0;
			#endif
			#ifdef SM_DBG
			NFailedArray_.searchIndex.clear();
			NFailedArray_.searchIndexFalse.clear();
			#endif
		}
		CheckResult gamePlanCheck(bool init);
		bool gamePlanCheckImpossibleIndex();
		bool gamePlanCheckRepeatedGames(bool init);
		bool gamePlanCheckMaxPauseGames();
		bool gamePlanCheckFieldTypes();
		bool gamePlanCheckMaxGamesSameClub(bool init);
		void updateCounters(bool init);
		void updateCountersFieldType(bool init);
		void init();
		void dispNFailArray();

};
}//namespace smfigapl
#endif
