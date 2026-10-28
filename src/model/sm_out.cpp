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
#include <utility>
#include <vector>
#include <string>

#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_out.hpp"


void smfigapl::Output::init(uint na)
{
    indexedGamePlan.init(input_.setup.realCapacity.NRounds, input_.setup.realCapacity.NFields, na);
    plan = {};
}
bool smfigapl::Output::isSolved() const
{
    return solved_;
}

void smfigapl::Output::setSolved()
{
    solved_ = true;
}

void smfigapl::Output::resetSolved()
{
    solved_ = false;
}

void smfigapl::Output::setResult(const smfigapl::ResultArray& res)
{
    status_ = res.status();
    solved_ = status_ == SearchStatus::solved;
}

smfigapl::SolveResult smfigapl::Output::solveResult(const ResultArray& result, std::chrono::milliseconds duration) const
{
    SolveResult r;
    r.status = result.status();
    for (const Exception& reason : result.reasons())
    {
        Problem p;
        p.code = reason.code();
        p.parameters = reason.params();
        r.reasons.push_back(std::move(p));
    }
    if (r.status != SearchStatus::impossible)
        r.plan = plan;
    for (uint i = 0; i < result.size(); i++)
        r.info.steps.push_back(result.getStep(i));
    r.info.duration = duration;
    return r;
}

smfigapl::SearchStatus smfigapl::Output::status() const
{
    return status_;
}

//ResultArray
bool smfigapl::ResultArray::getResult()
{
    return std::ranges::all_of(resultArray_, [](const auto & r)
    {
        return r.stepResult == StepResult::success;
    });
}

smfigapl::StepResult smfigapl::ResultArray::getResult(uint index)
{
    return resultArray_[index].stepResult;
}

void smfigapl::ResultArray::setResult(uint index, StepResult res)
{
    resultArray_[index].stepResult = res;
}

void smfigapl::ResultArray::incStep(uint  index)
{
    (resultArray_[index].steps)++;
}

smfigapl::uint smfigapl::ResultArray::getStep(uint  index) const
{
    return resultArray_[index].steps;
}

smfigapl::uint smfigapl::ResultArray::size() const
{
    return static_cast<uint>(resultArray_.size());
}

smfigapl::uint smfigapl::ResultArray::getTotalSteps()
{
    uint total = 0;
    for (auto&&   i : resultArray_)
        total += i.steps;
    return total;
}


void smfigapl::ResultArray::init(uint size)
{
    resultArray_.resize(size);
}

smfigapl::SearchStatus smfigapl::ResultArray::status() const
{
    bool solved = true;
    bool cancelled = false;
    for (const auto& r : resultArray_)
    {
        if (r.stepResult == StepResult::failed)
            return SearchStatus::impossible;
        solved = solved && r.stepResult == StepResult::success;
        cancelled = cancelled || r.stepResult == StepResult::cancelled;
    }
    if (cancelled)
        return SearchStatus::cancelled;
    return solved ? SearchStatus::solved : SearchStatus::aborted;
}

void smfigapl::ResultArray::setReason(uint index, Exception reason)
{
    resultArray_[index].reason = std::move(reason);
}

std::vector<smfigapl::Exception> smfigapl::ResultArray::reasons() const
{
    std::vector<Exception> res;
    for (const auto& r : resultArray_)
        if (r.reason)
            res.push_back(*r.reason);
    return res;
}
