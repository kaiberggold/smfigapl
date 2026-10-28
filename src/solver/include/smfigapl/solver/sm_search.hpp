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
#include <functional>
#include <optional>
#include <stop_token>
#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <limits>

#include "smfigapl/solver/sm_cond.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_indexed.hpp"
#include "smfigapl/model/sm_named.hpp"
#include "smfigapl/solver/sm_search_types.hpp"
#include "smfigapl/model/sm_out.hpp"
#include "smfigapl/solver/sm_ts.hpp"
#include "smfigapl/solver/sm_atomic.hpp"
#include "smfigapl/solver/sm_tables.hpp"


namespace smfigapl
{

// Progress of the search: reported every stepSize steps and once when the
// search of a (sub) tournament ends
    struct SearchProgress
    {
        uint atomicIndex;
        uint step;
        bool finished;
    };
    using ProgressCallback = std::function<void(const SearchProgress&)>;

    class GamePlanSearch
    {
    private:
        const smfigapl::Input& input_;
        smfigapl::Output& output_;
        smfigapl::CondTables condTables_;
        uint treeSearchLevel_{0};
        TreeSearchState treeSearchState_;
        // check refers to treeSearchLevel_, condTables_ and treeSearchState_, so it is declared after them
        smfigapl::CheckGamePlan check_;
        smfigapl::TreeSearch ts_;
        // refers to ts_ and condTables_
        smfigapl::RoundTables tables_;
        ProgressCallback progress_;
        CheckResult lastCheck_;
        uint maxSteps_ = std::numeric_limits<uint>::max();
        std::optional<std::chrono::milliseconds> timeLimit_;
        std::stop_token stop_;
        // the time limit and the stop token are checked at the first step and then every checkEvery steps
        static constexpr uint checkEvery = 4096;
        std::chrono::steady_clock::time_point deadline_;
        uint stepSize_ = 1000;
        AtomicSets atomicSets_;
        ResultArray resultArray_;
        std::vector<uint> treeSearchIndex_;
        // Home and away games of every team of the atomic set
        struct HomeAwayCount
        {
            std::vector<std::array<uint, 2>> perTeam;
            std::array<uint, 2> max = {0, 0};
            std::array<uint, 2> maxTeam = {0, 0};
        };
        void countHomeAway(HomeAwayCount& count);
        // Swaps a game of the team with the most home (away) games
        void swapHomeAway(const HomeAwayCount& count, uint homeAway);
        // Tree search of the current atomic set
        void runTreeSearch();
        // error if the atomic set has too many pairings or candidates for the search (after ts_.prepare)
        void checkSize() const;
        // true if the time limit is reached (checked every few thousand steps)
        [[nodiscard]] bool timeUp(uint step) const;
        // The result of the tree search of an atomic set when it ended
        [[nodiscard]] StepResult endResult(uint atomicIndex, bool cancelled);
        // true if the caller asked to stop (checked like the time limit)
        [[nodiscard]] bool stopRequested(uint step) const;

    public:
        GamePlanSearch(const smfigapl::Input& in, smfigapl::Output& out): input_{in}, output_{out},
            check_{in, treeSearchLevel_, condTables_, treeSearchState_},
            ts_{in.dbg}, tables_{in, ts_, condTables_}
        {
        }
        // check_ refers to members of this object, a copy would refer to the original
        GamePlanSearch(const GamePlanSearch&) = delete;
        GamePlanSearch& operator=(const GamePlanSearch&) = delete;
        GamePlanSearch(GamePlanSearch&&) = delete;
        GamePlanSearch& operator=(GamePlanSearch&&) = delete;
        ~GamePlanSearch() = default;
        void init();
        void preInit();
        void calcIndexedGamePlan();
        std::string getClubNameByTeamId(uint id);
        std::string getTeamColorByTeamId(uint id);
        bool treeSearchIndexNext();
        void treeSearchIndexNextLevelDown();
        smfigapl::StepResult searchStep(bool init, smfigapl::StepResult oldResult);
        [[nodiscard]] smfigapl::ResultArray  gpSearch();
        // the game plan by name (Output::plan) from the indexed one
        void calcGamePlan();
        void displayTreeSearchIndex();
        void setMaxSteps(uint m);
        // the whole search (all sub tournaments) stops after this time
        void setTimeLimit(std::chrono::milliseconds limit);
        // A stop request ends the search with StepResult::cancelled
        void setStopToken(std::stop_token token);
        void setProgressCallback(uint stepSize, ProgressCallback callback);
        [[nodiscard]] uint  getSteps();
        void balancedHomeAwayCalc();
        void dispNFailArray();
        void optimizeRounds();
    };
} //namespace
