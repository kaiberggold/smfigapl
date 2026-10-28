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

#pragma once
#include <chrono>
#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "smfigapl/model/sm_except.hpp"
#include "smfigapl/plan.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_indexed.hpp"
#include "smfigapl/model/sm_named.hpp"


namespace smfigapl
{

// Result of the tree search
    enum class StepResult : int
    {
        success = 0,
        nextStep = 1,
        levelUp = 2,
        failed = -1, // proven: no game plan exists
        failedMaxSteps = -3, // aborted at the step limit
        failedTimeLimit = -4, // aborted at the time limit
        cancelled = -5 // stopped by the caller
    };

// The result of the whole search is a SearchStatus of the public API
// (smfigapl/plan.hpp); this search never ends with cancelled.

// The result of one atomic set (sub tournament)
    class AtomicResult
    {
    public:
        uint steps = 0;
        StepResult stepResult = StepResult::success;
        // why no game plan exists (stepResult failed)
        std::optional<Exception> reason;
    } ;
    class ResultArray
    {
    private:
        std::vector<AtomicResult> resultArray_;
    public:
        ResultArray() = default;
        ResultArray(uint size): resultArray_(size)
        {
        }
        [[nodiscard]] bool getResult();
        [[nodiscard]] StepResult getResult(uint index);
        void setResult(uint index, StepResult res);
        // impossible if one (sub) tournament has no game plan, cancelled if
        // one was cancelled, aborted if one was aborted, solved if all have one
        [[nodiscard]] SearchStatus status() const;
        void setReason(uint index, Exception reason);
        // the reasons of the (sub) tournaments without a game plan
        [[nodiscard]] std::vector<Exception> reasons() const;
        void incStep(uint index);
        [[nodiscard]] uint getStep(uint index) const;
        [[nodiscard]] uint getTotalSteps();
        // the number of (sub) tournaments
        [[nodiscard]] uint size() const;
        void init(uint size);
    } ;

    class Output
    {
    private:
        const smfigapl::Input& input_;
        bool solved_ = false;
        SearchStatus status_ = SearchStatus::aborted;
    public:
        IndexedGamePlan indexedGamePlan;
        // the game plan by name, built from the indexed one after the search
        GamePlan plan;
        Output(const smfigapl::Input& in): input_{in}
        {
        }
        void init(uint na);
        [[nodiscard]] bool isSolved() const;
        void setSolved();
        void resetSolved();
        void setResult(const ResultArray& res);
        [[nodiscard]] SearchStatus status() const;
        // The public result of the search: status, reasons, the plan (empty
        // if there is none), the steps of every (sub) tournament
        [[nodiscard]] SolveResult solveResult(const ResultArray& result, std::chrono::milliseconds duration) const;
    };
}; //namespace smGamePlan
