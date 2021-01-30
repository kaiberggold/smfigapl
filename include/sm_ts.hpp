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
// OR OTHER DEALINGS IN THE SOFTWARE.#ifndef SMTREESEARCH
#ifndef SMTREESEARCH
#define SMTREESEARCH

#include <iostream>
#include <vector>
#include <string>

#include "sm_in.hpp"
#include "sm_misc.hpp"


namespace smfigapl{

class TreeSearch
{
public:

private:
	struct TsInput{
		Capacity atomicCapacity;
		Mapping mapping;
		uint extraPauseFields;
	}input;
	Capacity capacity_;
	uint NHalfFields_;
	uint NLevels_;
	uint level_;
	uint searchIndexMax_;
	uint NFieldMixArray_;
	uint NPairingArray_;
	uint subLevelInit_;
	bool checkAll_;
	bool pauseFields_;
	#ifdef SM_DBG
	bool dbg_;
	#endif

	std::vector<uint> nodeIndex_;
	std::vector<uint> nodeIndexMax_;
	std::vector<uint> indexedFieldTypeArray_;
	std::vector<uint> subLevel_;
	std::vector<uint> NSubLevel_;
	std::vector<uint> NRemaining_;
	std::vector<GamePlanRoundByTeamId> pairingArray_;
	std::vector<GamePlanRoundByTeamId> pairingArrayFull_;
	GamePlanRoundByTeamId CurrentPairing_;
	std::vector<std::vector<uint>> fieldMixArray_;
	std::vector<uint> indexArray_;
	std::vector<uint> indexArrayInit_;
	std::vector<std::vector<uint>> limGamesFieldType_;
	std::vector<std::vector<std::vector<uint>>> indexArrays_;
	std::vector<uint> fieldTypeSortArray_;
	std::vector<uint> fieldTypesSorted_;
	std::vector<uint> NFieldTypeArray_;


	public:
		#ifdef SM_DBG
		TreeSearch(bool dbg):dbg_{dbg}
		#else
		TreeSearch()
		#endif
		{
			input.extraPauseFields=0;
			NHalfFields_=0;
			NLevels_=0;
			level_=0;
			searchIndexMax_=0;
			NFieldMixArray_=0;
			NPairingArray_=0;
			subLevelInit_=0;
			checkAll_=false;
			nodeIndex_.clear();
			nodeIndexMax_.clear();
			indexedFieldTypeArray_.clear();
			subLevel_.clear();
			NSubLevel_.clear();
			NRemaining_.clear();
			pairingArray_.clear();
			pairingArrayFull_.clear();
			CurrentPairing_.init(0);
			fieldMixArray_.clear();
			indexArray_.clear();
			indexArrayInit_.clear();
			limGamesFieldType_.clear();
			indexArrays_.clear();
			pauseFields_=0;
		}

		void init();
		void nodeIndexMaxInit();
		void indexedFieldTypeArrayInit();
		void indexedClubArrayInit();
		bool nodeNext();
		void setNodeRound();
		uint calcIndexSize();
		void initNSubLevel();
		bool fieldMixArraySubLevelStep(uint i);
		void fieldMixArraySubLevelInit(uint j);
		void initIndexArrays();
		void initPairingArray();
		void initFieldMixArray();
		void initPairingArrayFull();
		void displayPairingArray();
		void displayPairingArrayFull();
		void displaySearchArray();
		void displayIndexedFieldTypeArray();
		void displayFieldTypes();
		void displayIndexedClubArray();
		void displayIndexArray();
		void displayIndexArrayInit();
		void displayFieldMixArray();
		void displaytreeSearchIfdex();
	    uint getTeam(uint pairingIndexFull, uint field, uint gameIndex);
		uint getNPairingArray();
		uint getNFieldMixArray();
		uint getNHalfFields();
		GamePlanRoundByTeamId getRound(uint pairingIndexFull);
		uint getFieldTypesSorted(uint j);
		uint getfieldTypeSortArray(uint j);
		uint getNFields();
		uint getNClubs();
		uint getNTeams();
		void setCapacity(Capacity c){input.atomicCapacity=c;};
		void setExtraPauseFields(uint n){input.extraPauseFields=n;};
		void setMapping(Mapping m){input.mapping=m;};


};
}//namespace smfigapl
#endif
