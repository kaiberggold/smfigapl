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
#include <vector>
#include <cstddef>
#include <numeric>
#include "smfigapl/model/sm_types.hpp"

namespace smfigapl
{

// Game plan based on numeric team / field indices
    class IndexedTeam
    {
    private:
        uint team_ = 0;
        ClubId club_;
        ColorId color_;

    public:
        bool operator==(IndexedTeam& other) const;

        void setTeamClub(ClubId cl);

        void setTeamColor(ColorId co);

        void setTeamId(uint teamId);

        [[nodiscard]] ClubId getTeamClub() const;

        [[nodiscard]] ColorId getTeamColor() const;

        [[nodiscard]] uint getTeamId() const;
    };

    class IndexedGame
    {
    private:
        std::array<IndexedTeam, 2> teams_{};
        bool pause_ = false;
        bool swap_ = false;


    public:
        IndexedGame() = default;

        IndexedGame(IndexedTeam a, IndexedTeam b);

        //Return first (0) or second (1) team of a game
        IndexedTeam getTeamByGameId(uint gameIndex);

        //Return first (0) or second (1) team of a game
        bool gameCompare(IndexedGame b);

        void setTeam(uint gameIndex, IndexedTeam team);

        // Return the Game
        IndexedGame getGame();

        void setPause(bool p);

        [[nodiscard]] bool getPause() const;

        void setSwap(bool sw);
        void flipSwap();
        [[nodiscard]] bool getSwap() const;
        bool contains(uint team);


        // Set the Game
    };

    bool compare(smfigapl::IndexedGame& l, smfigapl::IndexedGame& r);

    class IndexedRound
    {
    private:
        std::vector<IndexedGame> roundArray_;
        std::vector<IndexedTeam> pauseTeams_;
        uint  NFields_ = 0;
        uint index_ = 0;

    public:
        IndexedRound(uint n): roundArray_(n), NFields_{n}
        {
        }
        IndexedRound() = default;

        IndexedGame& operator[](size_t i)
        {
            return roundArray_[i];
        }

        void setGame(uint field, IndexedGame game);
        IndexedGame getGame(uint field);
        uint getNPauseTeams();
        IndexedTeam getPauseTeam(uint pauseIndex);
        std::vector<IndexedTeam> getPauseTeams();
        bool isPauseTeam(uint team);
        void addPauseTeam(IndexedTeam team);
        void pauseTeamsClear();
        void sort();
        uint size();
        bool contains(IndexedGame game);
        void init(uint n);
        bool getHomeAwayState(uint team, bool home);
        bool isHome(uint team);
        bool isAway(uint team);
        bool isHomeAway(uint team, bool home);
        void flipSwap(uint field);
        void setIndex(uint i);
        [[nodiscard]] uint  getIndex() const;
        smfigapl::uint getFieldIdByTeamId(smfigapl::uint team);
    };

    class IndexedGamePlan
    {
    private:
        std::vector<std::vector<smfigapl::IndexedGame>> gamePlan_;
        std::vector<std::vector<smfigapl::IndexedTeam>> pauseTeams_;
        std::vector<std::vector<uint>> pauseTeamsAtomicIndex_;
        uint NRounds_ = 0;
        uint NFields_ = 0;
        uint NAtomic_ = 0;
        std::vector<uint> fieldTypeByFieldId_;
        std::vector<uint> atomicIndexArray_;
        std::vector<std::vector<uint>> atomicFieldMappingArray_;
        std::vector<std::vector<uint>> resortIndex_;

    public:
        IndexedGamePlan() = default;
        IndexedGamePlan(uint n): gamePlan_(n), pauseTeams_(n), pauseTeamsAtomicIndex_(n), NRounds_{n}
        {
            std::vector<uint> temp(NRounds_);
            std::iota(temp.begin(), temp.end(), 0u);
            resortIndex_.push_back(temp);
        }
        IndexedGamePlan(uint NRounds, uint NFields, uint NAtomic)
        {
            this->init(NRounds, NFields, NAtomic);
        }

        void setGame(uint round, uint field, IndexedGame game);
        IndexedGame getGame(uint round, uint field);
        IndexedGame getGameAtomic(uint round, uint field, uint atomicIndex);
        void reset();
        void init(uint nr, uint nf, uint na);
        [[nodiscard]] uint size() const;
        uint getFieldTypeByFieldId(uint field);
        uint getFieldTypeByFieldIdAtomic(uint field, uint atomicIndex);
        void setFieldTypeByFieldId(std::vector<uint> f);
        void addPauseTeamAtomic(uint round, IndexedTeam team, uint atomicIndex);
        void pauseTeamsClear(uint round);
        void flipSwap(uint round, uint field);
        void flipSwapAtomic(uint round, uint field, uint atomicIndex);
        bool getHomeAwayState(uint round, uint team, bool home);
        bool isHome(uint round, uint team);
        bool isAway(uint round, uint team);
        bool getHomeAwayStateAtomic(uint round, uint team, bool home, uint atomicIndex);
        bool isHomeAtomic(uint round, uint team, uint atomicIndex);
        bool isAwayAtomic(uint round, uint team, uint atomicIndex);
        // the pause teams of the atomic set in round `round` (as shown)
        [[nodiscard]] std::vector<IndexedTeam> pauseTeamsAtomic(uint round, uint atomicIndex) const;
        [[nodiscard]] bool isPauseTeam(uint round, uint team) const;
        [[nodiscard]] uint getNPauseTeams(uint round) const;
        [[nodiscard]] uint getNPauseTeamsAtomic(uint round, uint atomicIndex) const;
        [[nodiscard]] IndexedTeam getPauseTeam(uint round, uint pauseIndex) const;
        bool getPause(uint round, uint field);
        bool getPauseAtomic(uint round, uint field, uint atomicIndex);
        bool isHomeAway(uint round, uint team, bool home);
        // round `round` (as shown) of the atomic set is the stored round order[round]
        void setResortOrder(const std::vector<uint>& order, uint atomicIndex);
        void setAtomicFieldMappingArray(std::vector<std::vector<uint>> m);
        uint getAtomicIndex(uint field);
        uint getResortIndex(uint round, uint atomicIndex);
        [[nodiscard]] uint getNAtomic() const;
        // the same for all atomic sets (sub tournaments)
        [[nodiscard]] uint getNRounds() const;
    };
}//namespace smfigapl
