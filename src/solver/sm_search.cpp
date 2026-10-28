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
#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include "smfigapl/solver/sm_search.hpp"
#include "smfigapl/solver/sm_atomic.hpp"
#include "smfigapl/model/sm_math.hpp"
#include "smfigapl/model/sm_debug.hpp"
#include "smfigapl/model/sm_except.hpp"

namespace
{
    // a count (pairings, rounds) as text, without decimals
    std::string wholeNumber(double value)
    {
        return std::to_string(static_cast<unsigned long long>(std::llround(value)));
    }
}

void smfigapl::GamePlanSearch::preInit()
{
    atomicSets_ = calcAtomicSets(input_, condTables_);
    resultArray_.init(atomicSets_.count);
}

void smfigapl::GamePlanSearch::init()
{
    output_.indexedGamePlan.setFieldTypeByFieldId(input_.setup.mapping.indexedFieldTypeByFieldId);
    treeSearchLevel_ = 0;
    // also for the next atomic set (resize kept the indices of the previous one)
    treeSearchIndex_.assign(condTables_.atomicCapacityArray[condTables_.atomicIndex].NRounds, 0);
    lastCheck_ = {true, 0};
    tables_.build();
}

bool smfigapl::GamePlanSearch::treeSearchIndexNext()
{
    uint idx = treeSearchIndex_[treeSearchLevel_];
    if (input_.dbg.has(DebugFlag::gps))
        input_.dbg.out(DebugFlag::gps) << "jump ts: " << treeSearchLevel_ << "idx: " << treeSearchIndex_[treeSearchLevel_] << " ff: " << lastCheck_.check << " " << " j:" << lastCheck_.jump << " ";
    if (idx + lastCheck_.jump <= condTables_.searchIndexMax[treeSearchLevel_])
    {
        treeSearchIndex_[treeSearchLevel_] = idx + lastCheck_.jump;
        return true;
    }
    return false;
}

void smfigapl::GamePlanSearch::treeSearchIndexNextLevelDown()
{
    uint jump = 1;
    if (treeSearchLevel_ == (condTables_.atomicCapacityArray[condTables_.atomicIndex].NTeams - 1) && input_.conditions.iterations == 2 && input_.conditions.repeatPairings)
    {
        jump = tables_.size() / input_.conditions.iterations - treeSearchIndex_[treeSearchLevel_ - 1];
    }
    else
    {
        if constexpr(jumpFastLevelDown)
        {
            // skip the following rounds with a pairing of the previous round,
            // if that pairing may not be played again (with repeated pairings
            // it may, up to the number of iterations)
            const uint previous = treeSearchIndex_[treeSearchLevel_ - 1];
            const Candidates& c = condTables_.candidates;
            c.forEachGame(previous, [&](uint position, uint a, uint b)
            {
                if (check_.pairingLimitReached(c.pairingKey(a, b)))
                    jump = std::max(jump, c.gameJump(previous, position));
            });
        }
    }
    treeSearchIndex_[treeSearchLevel_] = treeSearchIndex_[treeSearchLevel_ - 1] + jump;
}

smfigapl::StepResult smfigapl::GamePlanSearch::searchStep(bool init, StepResult oldResult)
{
    if (init)
    {
        treeSearchState_.Add = 0;
        treeSearchState_.Sub = -1;
        if (input_.dbg.has(DebugFlag::gps))
        {
            input_.dbg.out(DebugFlag::gps) << "init"; this->displayTreeSearchIndex();
        }
        return smfigapl::StepResult::nextStep;
    }

    if (lastCheck_.check && oldResult != StepResult::levelUp)
    {
        if (treeSearchLevel_ < condTables_.atomicCapacityArray[condTables_.atomicIndex].NRounds - 1) // all conditions fulfilled, go one level down
        {
            treeSearchLevel_++;
            treeSearchIndexNextLevelDown();
            if (treeSearchIndex_[treeSearchLevel_] > condTables_.searchIndexMax[treeSearchLevel_])
            {
                // no candidate left on the new level (e.g. a single pairing):
                // nothing was added there, go back up
                treeSearchIndex_[treeSearchLevel_] = 0;
                treeSearchLevel_--;
                treeSearchState_.Add = -1;
                treeSearchState_.Sub = -1;
                return smfigapl::StepResult::levelUp;
            }
            treeSearchState_.Add = static_cast<int>(treeSearchIndex_[treeSearchLevel_]);
            treeSearchState_.Sub = -1;
            if (input_.dbg.has(DebugFlag::gps))
                input_.dbg.out(DebugFlag::gps) << "TS_NEXT_STEP" << " Add: " << treeSearchState_.Add << std::endl;
            return smfigapl::StepResult::nextStep;
        }

        if (input_.dbg.has(DebugFlag::gps))
            input_.dbg.out(DebugFlag::gps) << "TS_SUCCESS" << lastCheck_.check << std::endl;
        return smfigapl::StepResult::success;
    }
    // StepResult::levelUp or check failed: next candidate on this level
    treeSearchState_.Sub = static_cast<int>(treeSearchIndex_[treeSearchLevel_]);
    const bool nextStep = this->treeSearchIndexNext();
    if (treeSearchIndex_[treeSearchLevel_] <= condTables_.searchIndexMax[treeSearchLevel_] && nextStep)
    {
        treeSearchState_.Add = static_cast<int>(treeSearchIndex_[treeSearchLevel_]);
        if (input_.dbg.has(DebugFlag::gps))
            input_.dbg.out(DebugFlag::gps) << "TS_NEXT_STEP_2" << " Add: " << treeSearchState_.Add << " Sub: " << treeSearchState_.Sub << std::endl;
        return smfigapl::StepResult::nextStep;
    }

    // no candidate left on this level: go one level up
    treeSearchState_.Sub = -1;
    if (treeSearchLevel_ == 0)
    {
        if (input_.dbg.has(DebugFlag::gps))
            input_.dbg.out(DebugFlag::gps) << "TS_FAILED" << std::endl;
        return smfigapl::StepResult::failed;
    }
    treeSearchState_.Add = -1;
    treeSearchState_.Sub = static_cast<int>(treeSearchIndex_[treeSearchLevel_]);
    treeSearchIndex_[treeSearchLevel_] = 0;
    treeSearchLevel_--;

    if (input_.dbg.has(DebugFlag::gps))
        input_.dbg.out(DebugFlag::gps) << "TS_LEVEL_UP" << " Add: " << treeSearchState_.Add << " Sub: " << treeSearchState_.Sub;
    return smfigapl::StepResult::levelUp;
}

void smfigapl::GamePlanSearch::runTreeSearch()
{
    const uint atomicIndex = condTables_.atomicIndex;
    lastCheck_ = check_.gamePlanCheck(true);
    if (input_.dbg.has(DebugFlag::gps))
    {
        input_.dbg.out(DebugFlag::gps) << "Atomic Index: " << atomicIndex << std::endl;
        input_.dbg.out(DebugFlag::gps) << "check: " << lastCheck_.check << " " << std::endl;
    }
    bool stopped = false;
    bool cancelled = false;
    do
    {
        if (resultArray_.getStep(atomicIndex) % stepSize_ == 0 && progress_)
        {
            progress_({atomicIndex, resultArray_.getStep(atomicIndex), false});
            if (input_.dbg.has(DebugFlag::stepsDetails))
            {
                input_.dbg.out(DebugFlag::stepsDetails) << "Search: ";
                this->displayTreeSearchIndex();
            }
        }

        treeSearchState_.stepResult = this->searchStep(false, treeSearchState_.stepResult);
        if (input_.dbg.has(DebugFlag::gps))
        {
            input_.dbg.out(DebugFlag::gps) << "Sl: " << treeSearchLevel_ << " ";
            this->displayTreeSearchIndex();
        }
        if (treeSearchState_.stepResult != StepResult::success)
            lastCheck_ = check_.gamePlanCheck(false);
        resultArray_.incStep(atomicIndex);
        const uint step = resultArray_.getStep(atomicIndex);
        cancelled = stopRequested(step);
        stopped = cancelled || step >= maxSteps_ || timeUp(step);
    }
    while (!stopped && (treeSearchState_.stepResult == StepResult::nextStep || treeSearchState_.stepResult == StepResult::levelUp));
    resultArray_.setResult(atomicIndex, endResult(atomicIndex, cancelled));
    if (treeSearchState_.stepResult == StepResult::failed)
        resultArray_.setReason(atomicIndex, Exception(ErrorCode::impossibleSearch));
    if (progress_)
    {
        progress_({atomicIndex, resultArray_.getStep(atomicIndex), true});
        if (input_.dbg.has(DebugFlag::gps))
            this->displayTreeSearchIndex();
    }
}

smfigapl::ResultArray smfigapl::GamePlanSearch::gpSearch()
{
    preInit();
    if (timeLimit_)
        deadline_ = std::chrono::steady_clock::now() + *timeLimit_;
    output_.init(atomicSets_.count);
    output_.indexedGamePlan.setAtomicFieldMappingArray(condTables_.fieldMappingArray);
    for (condTables_.atomicIndex = 0; condTables_.atomicIndex < atomicSets_.count; condTables_.atomicIndex++)
    {
        ts_.setCapacity(condTables_.atomicCapacityArray[condTables_.atomicIndex]);
        ts_.setExtraPauseFields(input_.conditions.extraPauseFields);
        // the fields of the atomic set (the configuration's mapping gave the
        // second sub tournament the field types of the first fields)
        ts_.setMapping(condTables_.atomicMappingArray[condTables_.atomicIndex]);
        ts_.prepare();
        checkSize();
        ts_.init();
        check_.init();
        init();
        treeSearchState_.stepResult = StepResult::nextStep;
        this->searchStep(true, StepResult::success);
        // no candidates, or impossible by counting: no search
        std::optional<Exception> reason = tables_.size() == 0 ? Exception(ErrorCode::impossibleSearch) : check_.impossibleReason();
        if (reason)
        {
            resultArray_.setResult(condTables_.atomicIndex, StepResult::failed);
            resultArray_.setReason(condTables_.atomicIndex, std::move(*reason));
            if (progress_)
            {
                progress_({condTables_.atomicIndex, 0, false});
                progress_({condTables_.atomicIndex, 0, true});
            }
            // without a plan for this (sub) tournament there is none at all
            break;
        }
        runTreeSearch();
        if (resultArray_.getResult(condTables_.atomicIndex) == StepResult::failed)
            break;
        this->calcIndexedGamePlan();
        if (input_.conditions.homeAwayBalance)
            this->balancedHomeAwayCalc();
        if (input_.conditions.optimizeRounds)
            this->optimizeRounds();
        // the other (sub) tournaments are not searched
        if (resultArray_.getResult(condTables_.atomicIndex) == StepResult::cancelled)
            break;
    } //Atomic sets
    this->calcGamePlan();
    output_.setResult(resultArray_);
    return resultArray_;
}

void smfigapl::GamePlanSearch::displayTreeSearchIndex()
{
    input_.dbg.out() << "Sl: " << treeSearchLevel_ << " Si: " ;
    for (uint i = 0; i < input_.setup.realCapacity.NRounds; i++)
        input_.dbg.out() << " " << treeSearchIndex_[i];
    input_.dbg.out() << " (";
    for (uint i = 0; i < input_.setup.realCapacity.NRounds; i++)
        input_.dbg.out() << " " << condTables_.searchIndexMax[i];
    input_.dbg.out() << " )" << std::endl;
}

void smfigapl::GamePlanSearch::checkSize() const
{
    const double pairings = ts_.pairingCount();
    if (pairings > MAX_SEARCH_PAIRINGS)
        throw Exception(ErrorCode::searchTooManyPairings, {wholeNumber(pairings), wholeNumber(MAX_SEARCH_PAIRINGS)});
    // every candidate is checked with subtypes or if not every empty field may be empty
    const uint virtualTeams = ts_.getNTeams() - condTables_.atomicCapacityArray[condTables_.atomicIndex].NTeams;
    const bool checked = input_.conditions.fieldSubTypeDeny.enabled || virtualTeams > input_.conditions.maxFieldsEmpty;
    const double max = checked ? MAX_SEARCH_ROUNDS_CHECKED : MAX_SEARCH_ROUNDS;
    const double rounds = ts_.roundCount() * input_.conditions.iterations;
    if (rounds > max)
        throw Exception(ErrorCode::searchTooLarge, {wholeNumber(rounds), wholeNumber(max)});
}

void smfigapl::GamePlanSearch::setMaxSteps(uint m)
{
    maxSteps_ = m;
}

void smfigapl::GamePlanSearch::setTimeLimit(std::chrono::milliseconds limit)
{
    timeLimit_ = limit;
}

bool smfigapl::GamePlanSearch::timeUp(uint step) const
{
    return timeLimit_ && step % checkEvery == 1 && std::chrono::steady_clock::now() >= deadline_;
}

smfigapl::StepResult smfigapl::GamePlanSearch::endResult(uint atomicIndex, bool cancelled)
{
    if (treeSearchState_.stepResult != StepResult::nextStep && treeSearchState_.stepResult != StepResult::levelUp)
        return treeSearchState_.stepResult;
    // stopped before the end
    if (cancelled)
        return StepResult::cancelled;
    return resultArray_.getStep(atomicIndex) >= maxSteps_ ? StepResult::failedMaxSteps : StepResult::failedTimeLimit;
}

bool smfigapl::GamePlanSearch::stopRequested(uint step) const
{
    return step % checkEvery == 1 && stop_.stop_requested();
}

void smfigapl::GamePlanSearch::setStopToken(std::stop_token token)
{
    stop_ = std::move(token);
}

smfigapl::uint smfigapl::GamePlanSearch::getSteps()
{
    return resultArray_.getTotalSteps();
}

void smfigapl::GamePlanSearch::setProgressCallback(uint stepSize, ProgressCallback callback)
{
    stepSize_ = stepSize;
    progress_ = std::move(callback);
}

void smfigapl::GamePlanSearch::dispNFailArray()
{
    check_.dispNFailArray();
}
