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

#include <algorithm>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>
#include <string>

#include "smfigapl/solver/sm_ts.hpp"
#include "smfigapl/model/sm_math.hpp"
#include "smfigapl/model/sm_debug.hpp"

double smfigapl::TreeSearch::roundCount() const
{
    // pairings (2n-1)!! times the assignments of the field types n! / (k1! k2! ...)
    double count = 1;
    for (uint i = 1; i < 2 * capacity_.NFields; i += 2)
        count *= i;
    for (uint i = 2; i <= capacity_.NFields; i++)
        count *= i;
    for (uint k : NFieldTypeArray_)
        for (uint i = 2; i <= k; i++)
            count /= i;
    return count;
}

double smfigapl::TreeSearch::pairingCount() const
{
    // (2n-1)!!
    double count = 1;
    for (uint i = 1; i < 2 * capacity_.NFields; i += 2)
        count *= i;
    return count;
}

void smfigapl::TreeSearch::init()
{
    // the tree search is reused for every atomic set: the arrays of the
    // previous set used to stay and the new rounds were appended
    pairingArray_.clear();
    indexArrays_.clear();
    fieldMixArray_.clear();
    prepare();
    NHalfFields_ = capacity_.NFields << 1;
    checkAll_ = false;
    initPairingArray();
    indexedFieldTypeArrayInit();
    initNSubLevel();
    initIndexArrays();
    initFieldMixArray();
    NFieldMixArray_ = smfigapl::factorial(capacity_.NFields);
    for (uint i = 0; i < capacity_.NFieldTypes; i++)
        NFieldMixArray_ /= factorial(NFieldTypeArray_[i]);
    searchIndexMax_ = NPairingArray_ * NFieldMixArray_;
    if (dbg_.has(DebugFlag::ts))
        displayPairingArrayFull();
}

void smfigapl::TreeSearch::prepare()
{
    //NFields
    capacity_.NFields = std::max(input_.atomicCapacity.NFields, ((input_.atomicCapacity.NClubs + 1) >> 1));
    capacity_.NFields = std::max(capacity_.NFields, ((input_.atomicCapacity.NTeams + 1) >> 1) + input_.extraPauseFields);
    //NTeams
    capacity_.NTeams = std::max(input_.atomicCapacity.NTeams, capacity_.NFields << 1);
    //NClubs
    capacity_.NClubs = input_.atomicCapacity.NClubs;
    if (capacity_.NTeams > input_.atomicCapacity.NTeams)
        capacity_.NClubs++;

    std::vector<FieldSortable> fields;
    fieldTypeSortArray_.resize(input_.atomicCapacity.NFields);
    fieldTypesSorted_.resize(input_.atomicCapacity.NFields);
    fields.resize(input_.atomicCapacity.NFields);
    for (uint j = 0; j < input_.atomicCapacity.NFields; j++)
    {
        fields[j].index = j;
        fields[j].fieldType = input_.mapping.indexedFieldTypeByFieldId[j];
        fields[j].fieldSubType = input_.mapping.indexedFieldSubTypeByFieldId[j];
    }

    fieldSort(fields);
    NFieldTypeArray_.clear();
    uint k = 0;
    for (uint j = 0; j < input_.atomicCapacity.NFields; j++)
    {
        fieldTypeSortArray_[j] = fields[j].index;
        fieldTypesSorted_[j] = fields[j].fieldType;
        if (j > 0 && (fields[j].fieldType != fields[j - 1].fieldType || fields[j].fieldSubType != fields[j - 1].fieldSubType))
        {
            NFieldTypeArray_.push_back(k);
            k = 1;
        }
        else
        {
            k++;
        }
    }
    NFieldTypeArray_.push_back(k);
    //pauseFields_
    uint NPauseFields = capacity_.NFields - input_.atomicCapacity.NFields;
    pauseFields_ = NPauseFields > 0;
    if (NPauseFields > 0)
    {
        NFieldTypeArray_.push_back(NPauseFields);
        fieldTypeSortArray_.push_back(static_cast<uint>(fieldTypeSortArray_.size()));
        pauseFields_ = true;
    }
    capacity_.NFieldTypes = static_cast<uint>(NFieldTypeArray_.size());
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
    if (indexArray_[subLevelInit_] < NRemaining_[i] - (NFieldTypeArray_[i] - subLevelInit_))
    {
        indexArray_[subLevelInit_]++;
        return false;
    }
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
    // (2 NFields - 1)!! pairings
    size_t NPairings = 1;
    for (uint i = 1; i < NHalfFields_; i += 2)
        NPairings *= i;
    pairingArray_.reserve(NPairings * NHalfFields_);
    std::vector<TableTeam> teams(NHalfFields_);
    std::iota(teams.begin(), teams.end(), TableTeam{0});
    std::vector<TableTeam> round(NHalfFields_);
    // the free teams of every depth
    std::vector<std::vector<TableTeam>> free(NHalfFields_ / 2 + 1);
    addPairings(teams, round, 0, free);
    NPairingArray_ = static_cast<uint>(pairingArray_.size() / NHalfFields_);
    if (dbg_.has(DebugFlag::ts))
        this->displayPairingArray();
}

// All pairings of the free teams after the games of round up to depth: the
// lowest free team plays the first, second, ... of the other free teams, the
// last game changes fastest
void smfigapl::TreeSearch::addPairings(std::span<const TableTeam> teams, std::vector<TableTeam>& round, uint depth, std::vector<std::vector<TableTeam>>& free)
{
    if (teams.empty())
    {
        pairingArray_.insert(pairingArray_.end(), round.begin(), round.end());
        return;
    }
    std::vector<TableTeam>& rest = free[depth];
    for (size_t k = 1; k < teams.size(); k++)
    {
        rest.clear();
        for (size_t t = 1; t < teams.size(); t++)
            if (t != k)
                rest.push_back(teams[t]);
        round[2 * static_cast<size_t>(depth)] = teams[0];
        round[2 * static_cast<size_t>(depth) + 1] = teams[k];
        addPairings(rest, round, depth + 1, free);
    }
}

/* Caculate all non redundant permutations of the fields
 *  NFields=4, field_types={2,2}: N=(4!)/(2!*2!)=6
 *
 *  steps   fieldMixArray_  indexArray_ level
 *  0       0123            01          0
 *  1       0213            02          1
 *  2       0312            03          2
 *  3       1203            12          3
 *  4       1302            13          4
 *  5       2301            23          5
 *
 *  NFields=5, field_types={2,3}: N=(5!)/(2!*3!)=10
 *
 *  steps   fieldMixArray_  indexArray_ subLevel_
 *  0       01234           01          0
 *  1       02314           02          1
 *  2       03124           03          2
 *  3       04123           04          3
 *  4       12034           12          4
 *  5       13024           13          5
 *  6       14023           14          6
 *  7       23014           23          7
 *  8       24013           24          8
 *  9       34012           34          9

 *
 *  field_types={1,2,1}: N=(4!)/(1!*2!1!)=12
 *
 *  steps   fieldMixArray_  indexArray_ subLevel_ indexArrays_{0}   indexArrays_[ft][subLevel_[ft][pos]
 *  0       0123            0 01        0 0      {{0},{1},{2},{3}}
 *  1       0132            0 02        0 1      indexArrays_{1}
 *  2       0231            0 12        0 2      {{0,1},{0,2},{1,2}}
 *  3       1023            1 01        1 0
 *  4       1032            1 02        1 1
 *  5       1230            1 12        1 2
 *  6       2013            2 00        2 0
 *  7       2031            2 01        2 1
 *  8       2130            2 12        2 2
 *  9       3012            3 00        3 0
 *  10      3021            3 01        3 1
 *  11      3120            3 12        3 2
 */

namespace
{
    // Position of the n-th (0 based) unmarked entry
    smfigapl::uint nthFree(const std::vector<bool>& marker, smfigapl::uint n)
    {
        smfigapl::uint k = 0;
        for (smfigapl::uint i = 0; i <= n; i++)
        {
            if (i > 0)
                k++;
            while (marker[k])
                k++;
        }
        return k;
    }
}

std::vector<smfigapl::uint> smfigapl::TreeSearch::fieldMixRow()
{
    std::vector<uint> row;
    std::vector<bool> marker(capacity_.NFields, false);
    // all field types but the last: positions from the index arrays of the current sub levels
    for (uint j = 0; j + 1 < capacity_.NFieldTypes; j++)
        for (uint l = 0; l < NFieldTypeArray_[j]; l++)
        {
            const uint k = nthFree(marker, indexArrays_[j][subLevel_[j]][l] - indexArrays_[j][0][l]);
            marker[k] = true;
            row.push_back(k);
        }
    // the last field type takes the remaining places
    for (uint i = 0; i < capacity_.NFields && row.size() < capacity_.NFields; i++)
        if (!marker[i])
            row.push_back(i);
    row.resize(capacity_.NFields, 0);
    return row;
}

void smfigapl::TreeSearch::initFieldMixArray()
{
    uint level = capacity_.NFieldTypes - 2;
    subLevel_.assign(capacity_.NFieldTypes - 1, 0);

    uint N = factorial(capacity_.NFields);
    for (uint i = 0; i < capacity_.NFieldTypes; i++)
        N /= factorial(NFieldTypeArray_[i]);

    // N rows, one for every combination of the sub levels; the last level changes fastest
    fieldMixArray_.push_back(fieldMixRow());
    uint steps = 1;
    while (steps < N)
    {
        if (subLevel_[level] < NSubLevel_[level] - 1)
        {
            subLevel_[level]++;
            for (uint i = level + 1; i < capacity_.NFieldTypes - 1; i++)
                subLevel_[i] = 0;
            level = capacity_.NFieldTypes - 2;
            steps++;
            fieldMixArray_.push_back(fieldMixRow());
        }
        else
            level--; // no check needed, there are only N rows
    }
    if (dbg_.has(DebugFlag::ts))
        this->displayFieldMixArray();
}

void smfigapl::TreeSearch::displayPairingArray()
{
    dbg_.out(DebugFlag::ts) << " PairingIndex, Rows: " << NPairingArray_ << std::endl;
    for (uint i = 0; i < NPairingArray_; i++)
    {
        dbg_.out(DebugFlag::ts) << "i: " << i << " ";
        for (uint j = 0; j < NHalfFields_; j++)
            dbg_.out(DebugFlag::ts) << " " << pairingArray_[static_cast<size_t>(i) * NHalfFields_ + j];
        dbg_.out(DebugFlag::ts) << std::endl;
    }
    dbg_.out(DebugFlag::ts) << std::endl;
    ;
}

void smfigapl::TreeSearch::displayPairingArrayFull()
{
    dbg_.out(DebugFlag::ts) << "pairingIndexFull_, Rows: " << static_cast<size_t>(NPairingArray_) * fieldMixArray_.size() << std::endl;
    // every pairing with every field mix (computed here, the search uses getRound)
    uint i = 0;
    for (uint p = 0; p < NPairingArray_; p++)
        for (const auto& mix : fieldMixArray_)
        {
            GamePlanRoundByTeamId round = smfigapl::roundFieldMix(pairing(p), mix);
            dbg_.out(DebugFlag::ts) << "i: " << i++ << " ";
            for (uint j = 0; j < round.size(); j++)
                dbg_.out(DebugFlag::ts) << " " << round.getTeamId(j);
            dbg_.out(DebugFlag::ts) << std::endl;
        }
    dbg_.out(DebugFlag::ts) << std::endl;
    ;
}

void smfigapl::TreeSearch::displaySearchArray()
{
    dbg_.out(DebugFlag::ts) << "Rounds: " << std::endl;
    for (uint i = 0; i < input_.atomicCapacity.NRounds; i++)
    {
        for (uint j = 0; j < capacity_.NFields; j++)
            dbg_.out(DebugFlag::ts) << getTeam(i, j, 0) << " " << getTeam(i, j, 1) << " ";
        dbg_.out(DebugFlag::ts) << std::endl;
    }
    dbg_.out(DebugFlag::ts) << std::endl;
}

void smfigapl::TreeSearch::displayIndexedFieldTypeArray()
{
    dbg_.out(DebugFlag::ts) << "capacity_.NFieldTypes: " << capacity_.NFieldTypes;
    dbg_.out(DebugFlag::ts) << "indexedFieldTypeArray_: " << std::endl;
    for (uint i = 0; i < capacity_.NFields; i++)
    {
        dbg_.out(DebugFlag::ts) << " " << indexedFieldTypeArray_[i];
    }
    dbg_.out(DebugFlag::ts) << std::endl;
}

void smfigapl::TreeSearch::displayFieldTypes()
{
    dbg_.out(DebugFlag::ts) << "capacity_.NFieldTypes: " << capacity_.NFieldTypes;
    dbg_.out(DebugFlag::ts) << "Fieldtypes: " << std::endl;
    for (uint i = 0; i < capacity_.NFieldTypes; i++)
    {
        dbg_.out(DebugFlag::ts) << " " << NFieldTypeArray_[i];
    }
    dbg_.out(DebugFlag::ts) << std::endl;
}

void smfigapl::TreeSearch::displayIndexArray()
{
    dbg_.out(DebugFlag::ts) << "indexArray_: " << std::endl;
    for (uint j : indexArray_)
        dbg_.out(DebugFlag::ts) << " " << j;
    dbg_.out(DebugFlag::ts) << std::endl;
}

void smfigapl::TreeSearch::displayIndexArrayInit()
{
    dbg_.out(DebugFlag::ts) << "indexArrayInit_: " << std::endl;
    for (uint j : indexArrayInit_)
        dbg_.out(DebugFlag::ts) << " " << j;

    dbg_.out(DebugFlag::ts) << std::endl;
}

void smfigapl::TreeSearch::displayFieldMixArray()
{
    dbg_.out(DebugFlag::ts) << "fieldMixArray_:, Rows: " << fieldMixArray_.size() << std::endl;
    for (auto& i : fieldMixArray_)
    {
        for (unsigned int j : i)
            dbg_.out(DebugFlag::ts) << " " << j;
        dbg_.out(DebugFlag::ts) << std::endl;
    }
    dbg_.out(DebugFlag::ts) << std::endl;
    ;
}



smfigapl::uint smfigapl::TreeSearch::getTeam(uint pairingIndexFull, uint field, uint gameIndex)
{
    return getRound(pairingIndexFull).getTeamId(field, gameIndex);
}

smfigapl::GamePlanRoundByTeamId smfigapl::TreeSearch::getRound(uint pairingIndexFull)
{
    // the field mixes are rotated by the pairing index (pairing p starts with mix p)
    const uint pairingIndex = pairingIndexFull / NFieldMixArray_;
    const uint mix = (pairingIndexFull % NFieldMixArray_ + pairingIndex) % NFieldMixArray_;
    return smfigapl::roundFieldMix(pairing(pairingIndex), fieldMixArray_[mix]);
}

smfigapl::GamePlanRoundByTeamId smfigapl::TreeSearch::pairing(uint i) const
{
    GamePlanRoundByTeamId round(capacity_.NFields);
    for (uint j = 0; j < NHalfFields_; j++)
        round.setTeamId(j, pairingArray_[static_cast<size_t>(i) * NHalfFields_ + j]);
    return round;
}


smfigapl::uint smfigapl::TreeSearch::getNPairingArray() const
{
    return NPairingArray_;
}

smfigapl::uint smfigapl::TreeSearch::getNFieldMixArray() const
{
    return NFieldMixArray_;
}

smfigapl::uint smfigapl::TreeSearch::getNHalfFields() const
{
    return NHalfFields_;
}

smfigapl::uint smfigapl::TreeSearch::getFieldTypesSorted(uint j)
{
    return fieldTypesSorted_[j];
}

smfigapl::uint smfigapl::TreeSearch::getFieldTypeSortArray(uint j)
{
    return fieldTypeSortArray_[j];
}

smfigapl::uint smfigapl::TreeSearch::getNFields() const
{
    return capacity_.NFields;
}

smfigapl::uint smfigapl::TreeSearch::getNClubs() const
{
    return capacity_.NClubs;
}

smfigapl::uint smfigapl::TreeSearch::getNTeams() const
{
    return capacity_.NTeams;
}


