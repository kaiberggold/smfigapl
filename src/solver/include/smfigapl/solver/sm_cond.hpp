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

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <unordered_set>
#include <vector>
#include <string>

#include "smfigapl/model/sm_except.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/solver/sm_search_types.hpp"

namespace smfigapl
{

    class CheckGamePlan
    {
    private:
        const smfigapl::Input& input_;
        const smfigapl::CondTables& condTables_;
        const smfigapl::TreeSearchState& treeSearchState_;
        const uint& treeSearchLevel_;
        CheckResult res_;
        struct
        {
            std::vector<uint> pairing;
            std::vector<uint> sameClub;
            std::vector<uint> pause;
            std::vector<std::vector<uint>> fieldType;
        } count_;
        std::vector<bool> jumpCheck_;
        std::vector<bool> checkArray_;
        uint NPairingIndexShort_ = 0;
        std::vector<std::vector<bool>> checkMax_;
        uint atomicIndex_ = 0;
        bool ftJump_ = false;
        // per team: the other teams of its club in the atomic set
        std::vector<uint> clubMates_;
        // how often a pairing may be played in the whole plan
        uint pairingLimit_ = 1;
        // the field type limits of the atomic set (none for types without a field in it)
        struct
        {
            std::vector<uint> min;
            std::vector<uint> max;
        } limits_;
        // the last round check: what every team may still do, the places
        // per field type, the states without a solution
        struct LastRound
        {
            static constexpr uint maxTypes = 4;
            std::vector<bool> canPause;
            std::vector<uint> typeMask;
            std::vector<bool> clubOk;
            std::array<uint, maxTypes> minSlots{};
            std::array<uint, maxTypes> maxSlots{};
            uint NTypes = 1;
            std::unordered_set<std::uint64_t> failed;
        } lastRound_;
        struct
        {
            uint repeatedGames = 0;
            uint sameClub = 0;
            uint pauseGames = 0;
            uint fieldTypes = 0;
            uint counting = 0;
            uint lastRound = 0;
            uint symmetry = 0;
            std::vector<bool> searchIndex;
            std::vector<bool> searchIndexFalse;
        } NFailedArray_;

        // the games of the candidate of the last forEachCounter: position and pairing
        struct Game
        {
            uint position;
            uint key;
        };
        std::array<Game, MAX_FIELDS> games_{};
        uint NGames_ = 0;
        // Symmetry of the teams: teams of one class are interchangeable for
        // all conditions, so are the virtual teams. Rounds with the same
        // kind of game (classes of its teams) on every group of fields are
        // equivalent; the first round of a plan is the first of its kind.
        struct Symmetry
        {
            std::vector<uint> teamClass;  // per real team
            uint NClasses = 0;            // the virtual teams have class NClasses
            std::map<std::vector<uint>, uint> first; // the first candidate of every kind
            uint scanned = 0;             // the candidates before are in first
        } symmetry_;
        void symmetryInit();
        // the kind of candidate i: the games with the classes of their teams on the groups of fields
        [[nodiscard]] std::vector<uint> roundKind(uint i) const;
        // true if candidate i is the first of its kind
        [[nodiscard]] bool symmetryFirst(uint i);
        // the next candidate after i that is the first of its kind (beyond the first round if none)
        [[nodiscard]] uint symmetryNext(uint i);
        // Applies op to every counter of the pairing at the search index
        template<class Op> void forEachCounter(uint index, Op op);
        void calcRepeatedGamesJump();
        void calcFieldTypeJump();
        // Counts a failed check, the result for gamePlanCheck
        CheckResult failed(uint& counter, const char* tag);
        // The counting checks for the `rem` rounds not chosen yet; reason
        // (if not nullptr) gets the explanation of a failed check
        [[nodiscard]] bool countingCheck(uint rem, std::optional<Exception>* reason) const;
        [[nodiscard]] bool countingCheckPauses(uint rem, std::optional<Exception>* reason) const;
        [[nodiscard]] bool countingCheckTeams(uint rem, std::optional<Exception>* reason) const;
        // gamesMax: the most games the team can still play, reduced by the field type maximums
        [[nodiscard]] bool countingCheckTeamFieldTypes(uint team, uint rem, uint gamesMin, uint& gamesMax, std::optional<Exception>* reason) const;
        [[nodiscard]] bool countingCheckFieldTypes(uint rem, std::optional<Exception>* reason) const;
        // how often the team may still pause in the `rem` rounds
        [[nodiscard]] uint pausesLeft(uint team, uint rem) const;
        // one round left: can every team play or pause within its limits?
        [[nodiscard]] bool lastRoundCheck();
        void lastRoundInit();
        // the field types (bit mask) on which the team may play its last game
        [[nodiscard]] uint lastRoundTypes(uint team) const;
        [[nodiscard]] bool lastRoundPairAllowed(uint a, uint b) const;
        // the teams in mask are placed (pauses of them pause, pairs[l] games on type l)
        [[nodiscard]] bool lastRoundSearch(uint mask, uint pauses, std::array<uint, LastRound::maxTypes>& pairs);
        [[nodiscard]] std::string teamName(uint team) const;

    public:
        CheckGamePlan(const smfigapl::Input& in, const uint& ts, const smfigapl::CondTables& tb, const smfigapl::TreeSearchState& tc): \
            input_{in}, condTables_{tb}, treeSearchState_{tc}, treeSearchLevel_{ts}
        {
        }
        [[nodiscard]] CheckResult gamePlanCheck(bool init);
        // Before the search (after init): why the atomic set has no game
        // plan, found by counting (places per field type, pauses, opponents)
        [[nodiscard]] std::optional<Exception> impossibleReason() const;
        [[nodiscard]] bool gamePlanCheckImpossibleIndex();
        [[nodiscard]] bool gamePlanCheckRepeatedGames(bool init);
        [[nodiscard]] bool gamePlanCheckMaxPauseGames();
        [[nodiscard]] bool gamePlanCheckFieldTypes();
        [[nodiscard]] bool gamePlanCheckMaxGamesSameClub(bool init);
        void updateCounters(bool init);
        // true if the pairing may not be played again in the current round
        // (treeSearchLevel_) of the search
        [[nodiscard]] bool pairingLimitReached(uint pairing) const;
        void updateCountersFieldType(bool init);
        void init();
        void dispNFailArray();

    };
}//namespace smfigapl
