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
// OR OTHER DEALINGS IN THE SOFTWARE.#include <iostream>
#include <vector>
#include <string>

#include "../include/sm_ts.hpp"

void smfigapl::TreeSearch::init()
{
	//NFields
	capacity_.NFields=std::max(input.atomicCapacity.NFields,((input.atomicCapacity.NClubs+1)>>1));
	capacity_.NFields=std::max(capacity_.NFields,((input.atomicCapacity.NTeams+1)>>1)+input.extraPauseFields);
	//NTeams
	capacity_.NTeams=std::max(input.atomicCapacity.NTeams,capacity_.NFields<<1);
	//NClubs
	capacity_.NClubs=input.atomicCapacity.NClubs;
	if (capacity_.NTeams>input.atomicCapacity.NTeams)
		capacity_.NClubs++;

	std::vector<fieldSortable> fields;
	fieldTypeSortArray_.resize(input.atomicCapacity.NFields);
	fieldTypesSorted_.resize(input.atomicCapacity.NFields);
	fields.resize(input.atomicCapacity.NFields);
	for (uint j = 0; j < input.atomicCapacity.NFields; j++)
	{
		fields[j].index=j;
		fields[j].fieldType=input.mapping.indexedFieldTypeByFieldId[j];
		fields[j].fieldSubType=input.mapping.indexedFieldSubTypeByFieldId[j];
	}

	fieldSort(&fields);
	NFieldTypeArray_.clear();
	uint k=0;
	for (uint j = 0; j < input.atomicCapacity.NFields; j++)
	{
		fieldTypeSortArray_[j]=fields[j].index;
		fieldTypesSorted_[j]=fields[j].fieldType;
		if (j>0 &&(fields[j].fieldType!=fields[j-1].fieldType ||fields[j].fieldSubType!=fields[j-1].fieldSubType))
		{
			NFieldTypeArray_.push_back(k);
			k=1;
		}
		else
		{
			k++;
		}
	}
	NFieldTypeArray_.push_back(k);
	//pauseFields_
	uint NPauseFields = capacity_.NFields - input.atomicCapacity.NFields;
	if (NPauseFields > 0)
		{
			NFieldTypeArray_.push_back(NPauseFields);
			fieldTypeSortArray_.push_back(static_cast<uint>(fieldTypeSortArray_.size()));
			pauseFields_ = true;
		}
	capacity_.NFieldTypes=static_cast<uint>(NFieldTypeArray_.size());

	NHalfFields_ = capacity_.NFields << 1;
	NLevels_ = (NHalfFields_ >> 1) - 1;
	checkAll_ = false;
	CurrentPairing_.init(capacity_.NFields);
	capacity_.NFieldTypes=static_cast<uint>(NFieldTypeArray_.size());
	level_ = NLevels_ - 1;
	nodeIndexMaxInit();
	initPairingArray();
	indexedFieldTypeArrayInit();
	initNSubLevel();
	initIndexArrays();
	initFieldMixArray();
	initPairingArrayFull();
	NFieldMixArray_ = smfigapl::factorial(capacity_.NFields);
	for (uint i = 0; i < capacity_.NFieldTypes; i++)
		NFieldMixArray_ /= factorial(NFieldTypeArray_[i]);
	searchIndexMax_ = NPairingArray_ * NFieldMixArray_;
	#ifdef SM_DBG
	if (dbg_&DBG_TS)
		displayPairingArrayFull();
	#endif




}

void smfigapl::TreeSearch::nodeIndexMaxInit()
{
	nodeIndex_.clear();
	nodeIndexMax_.clear();
	for (uint i = 0; i < NLevels_; i++)
	{
		nodeIndex_.push_back(0);
		nodeIndexMax_.push_back((NLevels_ - i) * 2);
	}
}

void smfigapl::TreeSearch::indexedFieldTypeArrayInit()
{
	indexedFieldTypeArray_.resize(capacity_.NFields);
	uint i = 0;
	for (uint j = 0; j < capacity_.NFieldTypes; j++)
	{
		for (uint k = 0; k < NFieldTypeArray_[j]; k++)
		{
			indexedFieldTypeArray_[i] = j;
			i++;
		}
	}
}


bool smfigapl::TreeSearch::nodeNext()
{
	level_ = NLevels_ - 1;
	if (nodeIndex_[level_] < nodeIndexMax_[level_])
	{
		nodeIndex_[level_]++;
		return (false);
	}
	else
	{
		bool exit =false;
		while (level_ >= 0 && !exit && nodeIndex_[level_] == nodeIndexMax_[level_])
		{
			nodeIndex_[level_] = 0;
			if (level_>0)
				level_--;
			else
				exit=true;
		}
		if (exit)
		{
			return (true);
		}
		else
		{
			nodeIndex_[level_]++;
			return (false);
		}
	}
}

void smfigapl::TreeSearch::setNodeRound()
{
	CurrentPairing_.reset();
	std::vector<bool> marker(NHalfFields_);
	for (uint i = 0; i < NLevels_; i++)
	{
		uint j = 0;
		while (j < capacity_.NTeams)
		{
			if (!marker[j])
			{
				marker[j] = true;
				CurrentPairing_.setTeamId(i << 1,j);
				j++;
				break;
			}
			j++;
		}
		uint l = 0;
		uint k = 0;
		do
		{
			if (!marker[k] && l == nodeIndex_[i])
				break;
			if (!marker[k])
				l++;
			k++;
		} while (k < capacity_.NTeams);
		marker[k] = true;
		CurrentPairing_.setTeamId((i << 1) + 1,k);
	}
	uint j = 0;
	while (j < capacity_.NTeams)
	{
		if (!marker[j])
		{
			CurrentPairing_.setTeamId(NLevels_ << 1,j);
			marker[j] = true;
			j++;
			break;
		}
		j++;
	}
	while (j < capacity_.NTeams)
	{
		if (!marker[j])
		{
			CurrentPairing_.setTeamId((NLevels_ << 1) + 1,j);
			break;
		}
		j++;
	}
}

smfigapl::uint smfigapl::TreeSearch::calcIndexSize()
{
	uint indexSize = capacity_.NFields - (static_cast<uint>(indexArray_.size()) - subLevelInit_);
	for (uint i = 0; i < level_; i++)
		indexSize -= NFieldTypeArray_[i];
	return indexSize;
}

void smfigapl::TreeSearch::initNSubLevel()
{
	uint count = 0;
	NSubLevel_.resize(capacity_.NFieldTypes - 1);
	NRemaining_.resize(capacity_.NFieldTypes - 1);
	for (uint i = 0; i < capacity_.NFieldTypes - 1; i++)
	{
		NSubLevel_[i] = factorial(capacity_.NFields - count) / \
				factorial(NFieldTypeArray_[i]) / factorial(capacity_.NFields - count - NFieldTypeArray_[i]);
		NRemaining_[i] = capacity_.NFields - count;
		count += NFieldTypeArray_[i];
	}
}

bool smfigapl::TreeSearch::fieldMixArraySubLevelStep(uint i)
{
	if (indexArray_[subLevelInit_] < NRemaining_[i] -(NFieldTypeArray_[i]- subLevelInit_))
	{
		indexArray_[subLevelInit_]++;
		return false;
	}
	else
		return true;
}

void smfigapl::TreeSearch::fieldMixArraySubLevelInit(uint j)
{
	for (uint i = subLevelInit_ + 1; i < NFieldTypeArray_[j] ; i++)
	{
		indexArray_[i] = indexArray_[i - 1] + 1;
	}
}


/* Precalculate all possible pairingArray_ for all fieldTypes,
 * these are accesable by a index
 *
 */
void smfigapl::TreeSearch::initIndexArrays()
{
	std::vector<std::vector<uint>> row;
	std::vector<std::vector<uint>> rowInit;
	indexArray_.clear();
	for (uint i = 0; i < capacity_.NFieldTypes - 1; i++)
	{
		//initialization of Variables
		row.clear();
		rowInit.clear();
		indexArray_.resize(NFieldTypeArray_[i]);
		for (uint j = 0; j < NFieldTypeArray_[i]; j++)
		{
			indexArray_[j] = j;
		}
		indexArrayInit_ = indexArray_;
		row.push_back(indexArray_);
		rowInit.push_back(indexArray_);
		subLevelInit_ = NFieldTypeArray_[i] - 1; //Start at lowest level
		uint j = 1;
		while (j < NSubLevel_[i])
		{
			if (fieldMixArraySubLevelStep(i))
			{
				//end of subLevel_ reached
				if (subLevelInit_ > 0)
					subLevelInit_--;
				else
				{
					break;
				}
			}
			else
			{
				//step
				if (subLevelInit_ < NFieldTypeArray_[i] - 1)
				{
					fieldMixArraySubLevelInit(i);
					subLevelInit_ = NFieldTypeArray_[i] - 1;
				}
				else
					j++;
				row.push_back(indexArray_);
				rowInit.push_back(indexArray_);
			}
		} //while (j < NSubLevel_[i])
		indexArrayInit_ = indexArray_;
		indexArrays_.push_back(row);
	} //i
}

void smfigapl::TreeSearch::initPairingArray()
{
	this->setNodeRound();
	pairingArray_.push_back(CurrentPairing_);
	while (!this->nodeNext())
	{
		this->setNodeRound();
		pairingArray_.push_back(CurrentPairing_);
	}
	NPairingArray_ = static_cast<uint>(pairingArray_.size());
	#ifdef SM_DBG
	if (dbg_&DBG_TS)
		this->displayPairingArray();
	#endif
}

/* Caculate all non redundant permutations of the fields
 *	NFields=4, field_types={2,2}: N=(4!)/(2!*2!)=6
 *
 *	steps	fieldMixArray_ 	indexArray_	level
 *	0		0123			01			0
 *	1		0213			02			1
 *	2		0312			03			2
 *	3		1203			12			3
 *	4		1302			13			4
 *	5		2301			23			5
 *
 *	NFields=5, field_types={2,3}: N=(5!)/(2!*3!)=10
 *
 *	steps	fieldMixArray_ 	indexArray_	subLevel_
 *	0		01234			01			0
 *	1		02314			02			1
 *	2		03124			03			2
 *	3		04123			04			3
 *	4		12034			12			4
 *	5		13024			13			5
 *	6		14023			14			6
 *	7		23014			23			7
 *	8		24013			24			8
 *	9		34012			34			9

 *
 *	field_types={1,2,1}: N=(4!)/(1!*2!1!)=12
 *
 *	steps	fieldMixArray_ 	indexArray_	subLevel_ indexArrays_{0}   indexArrays_[ft][subLevel_[ft][pos]
 *	0		0123			0 01		0 0      {{0},{1},{2},{3}}
 *	1		0132			0 02		0 1      indexArrays_{1}
 *	2		0231			0 12		0 2      {{0,1},{0,2},{1,2}}
 *	3		1023			1 01		1 0
 *	4		1032			1 02		1 1
 *	5		1230			1 12		1 2
 *	6		2013			2 00		2 0
 *	7		2031			2 01		2 1
 *	8		2130			2 12		2 2
 *	9		3012			3 00		3 0
 *	10		3021			3 01		3 1
 *	11		3120			3 12		3 2
 */

void smfigapl::TreeSearch::initFieldMixArray()
{
	std::vector<uint> addRow; // the next row to be added to fieldMixArray_
	std::vector<bool> marker;
	addRow.resize(capacity_.NFields);
	std::fill(addRow.begin(),addRow.end(),0);
	marker.resize(capacity_.NFields);
	std::fill(marker.begin(),marker.end(),0);
	uint level;
	level = capacity_.NFieldTypes - 2;
	subLevel_.resize(capacity_.NFieldTypes - 1);
	std::fill(subLevel_.begin(),subLevel_.end(),0);

	uint N = factorial(capacity_.NFields);
	for (uint i = 0; i < capacity_.NFieldTypes; i++)
		N /= factorial(NFieldTypeArray_[i]);

	// The Main Loop for the N rows to be added
	uint steps = 0;
	bool add;
	while (steps < N)
	{
		add = false;
		if (steps > 0)
		{
			if (subLevel_[level] < NSubLevel_[level] - 1)
			{
				//step
				subLevel_[level]++;
				if (level < capacity_.NFieldTypes - 2)
				{
					for (uint i = level + 1; i < capacity_.NFieldTypes - 1; i++)
					{
						subLevel_[i] = 0;
					}
					level = capacity_.NFieldTypes - 2;
				}
				steps++;
				add = true;
			}
			else
			{
				//levelup: No check because only N loops
				level--;
			}
		}
		else
		{
			steps++;
			add = true;
		}

		//add

		if (add)
		{
			uint m = 0;
			// Iterate over all field_types
			for (uint j = 0; j < capacity_.NFieldTypes; j++)
			{
				uint k = 0;
				uint lmax;
				if (j < capacity_.NFieldTypes - 1)
				{
					lmax  = NFieldTypeArray_[j];
					for (uint l = 0; l < lmax; l++)
					{
						k = 0;
						bool first =true;
						if (j < capacity_.NFieldTypes - 1) // not the last field_types+
						{
							for (uint i = 0; i < indexArrays_[j][subLevel_[j]][l] - indexArrays_[j][0][l] + 1; i++)
							{
								if (!first)
									k++;
								first=false;
								while (marker[k])
								{
									k++;
								};
							}
							marker[k] = true;
							addRow[m] = k;
							m++;
						} //if (j < capacity_.NFieldTypes - 1)
					}
				} //if (j < capacity_.NFieldTypes - 1)
				else //fill up the remaining places for the last fieldtype and reset
					for (uint i = 0; i < capacity_.NFields; i++)
					{
						if (!marker[i]&&m<capacity_.NFields)
						{
							addRow[m] = i;
							m++;
						}
					}
			} // for (uint j = 0; j < capacity_.NFieldTypes; j++)
			//add the row
			fieldMixArray_.push_back(addRow);
			//reset markers
			fill(marker.begin(), marker.end(), 0);
			fill(addRow.begin(), addRow.end(), 0);
		}//add
	} //while (steps < N)
	#ifdef SM_DBG
	if (dbg_&DBG_TS)
		this->displayFieldMixArray();
	#endif
}

void smfigapl::TreeSearch::initPairingArrayFull()
{
	pairingArrayFull_.clear();
	GamePlanRoundByTeamId round;
	for (uint i = 0; i < pairingArray_.size()*fieldMixArray_.size(); i++)
	{
		round=smfigapl::roundFieldMix(pairingArray_[i/fieldMixArray_.size()],fieldMixArray_[i%fieldMixArray_.size()]);
		pairingArrayFull_.push_back(round);
	}
}


void smfigapl::TreeSearch::displayPairingArray()
{
	std::cout << " PairingIndex, Rows: " << pairingArray_.size() << std::endl;
	for (uint i = 0; i < pairingArray_.size(); i++)
	{
		std::cout << "i: " << i << " ";
		for (uint j = 0; j < pairingArray_[i].size(); j++)
			std::cout << " " << pairingArray_[i].getTeamId(j);
		std::cout << std::endl;
	}
	std::cout << std::endl;
	;
}

void smfigapl::TreeSearch::displayPairingArrayFull()
{
	std::cout << "pairingIndexFull_, Rows: " << pairingArray_.size()*fieldMixArray_.size() << std::endl;
	for (uint i = 0; i < pairingArrayFull_.size(); i++)
	{
		std::cout << "i: " << i << " ";
		for (uint j = 0; j < pairingArrayFull_[i].size(); j++)
		{
			std::cout << " " << pairingArrayFull_[i].getTeamId(j);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	;
}

void smfigapl::TreeSearch::displaySearchArray()
{
	std::cout << "Rounds: " << std::endl;
	for (uint i = 0; i < input.atomicCapacity.NRounds; i++)
	{
		for (uint j = 0; j < capacity_.NFields; j++)
			std::cout << getTeam(i, j, 0) << " " << getTeam(i, j, 1) << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void smfigapl::TreeSearch::displayIndexedFieldTypeArray()
{
	std::cout << "capacity_.NFieldTypes: " << capacity_.NFieldTypes;
	std::cout << "indexedFieldTypeArray_: " << std::endl;
	for (uint i = 0; i < capacity_.NFields; i++)
	{
		std::cout << " " << indexedFieldTypeArray_[i];
	}
	std::cout << std::endl;
}

void smfigapl::TreeSearch::displayFieldTypes()
{
	std::cout << "capacity_.NFieldTypes: " << capacity_.NFieldTypes;
	std::cout << "Fieldtypes: " << std::endl;
	for (uint i = 0; i < capacity_.NFieldTypes; i++)
	{
		std::cout << " " << NFieldTypeArray_[i];
	}
	std::cout << std::endl;
}

void smfigapl::TreeSearch::displayIndexArray()
{
	std::cout << "indexArray_: " << std::endl;
	for (uint j = 0; j < indexArray_.size(); j++)
		std::cout << " " << indexArray_[j];
	std::cout << std::endl;
}

void smfigapl::TreeSearch::displayIndexArrayInit()
{
	std::cout << "indexArrayInit_: " << std::endl;
	for (uint j = 0; j < indexArrayInit_.size(); j++)
		std::cout << " " << indexArrayInit_[j];

	std::cout << std::endl;
}

void smfigapl::TreeSearch::displayFieldMixArray()
{
	std::cout << "fieldMixArray_:, Rows: " << fieldMixArray_.size()<<std::endl;
	for (uint i = 0; i < fieldMixArray_.size(); i++)
	{
		for (uint j = 0; j < fieldMixArray_[i].size(); j++)
			std::cout << " " << fieldMixArray_[i][j];
		std::cout << std::endl;
	}
	std::cout << std::endl;
	;
}



smfigapl::uint smfigapl::TreeSearch::getTeam(uint pairingIndexFull_, uint field, uint gameIndex )
{
	uint factor=1;
	return smfigapl::roundFieldMix(pairingArray_[pairingIndexFull_ / NFieldMixArray_],\
			fieldMixArray_[((pairingIndexFull_ % NFieldMixArray_)+pairingIndexFull_/NFieldMixArray_*factor)%NFieldMixArray_]).getTeamId(field,gameIndex);
}

smfigapl::GamePlanRoundByTeamId smfigapl::TreeSearch::getRound(uint pairingIndexFull_)
{
	GamePlanRoundByTeamId temp(capacity_.NFields);
	for (uint j = 0; j < capacity_.NFields; j++)
	{
		for (uint k=0;k<2;k++)
		{
			temp.setTeamId((j<<1)+k,getTeam(pairingIndexFull_, j, k));
		}
	}
	return temp;
}


smfigapl::uint smfigapl::TreeSearch::getNPairingArray()
{
	return NPairingArray_;
}

smfigapl::uint smfigapl::TreeSearch::getNFieldMixArray()
{
	return NFieldMixArray_;
}

smfigapl::uint smfigapl::TreeSearch::getNHalfFields()
{
	return NHalfFields_;
}

smfigapl::uint smfigapl::TreeSearch::getFieldTypesSorted(uint j)
{
	return fieldTypesSorted_[j];
}

smfigapl::uint smfigapl::TreeSearch::getfieldTypeSortArray(uint j)
{
	return fieldTypeSortArray_[j];
}

smfigapl::uint smfigapl::TreeSearch::getNFields()
{
	return capacity_.NFields;
}

smfigapl::uint smfigapl::TreeSearch::getNClubs()
{
	return capacity_.NClubs;
}

smfigapl::uint smfigapl::TreeSearch::getNTeams()
{
	return capacity_.NTeams;
}


