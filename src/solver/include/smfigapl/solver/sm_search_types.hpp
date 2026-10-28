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

#include <ostream>
#include <span>
#include <cstdint>
#include <vector>
#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_capacity.hpp"
#include "smfigapl/model/sm_out.hpp"

namespace smfigapl
{

// Search optimizations (jump predictions). They should be on, switch
// them off only for debugging the search.
    inline constexpr bool jumpFastLevelDown = true; // when going to the next search level
    inline constexpr bool fastJump = true;
    inline constexpr bool maxFieldTypeJump = true;  // for the field type conditions

    inline constexpr uint TEAM_UNDEFINED = 255;

    class CheckResult
    {
    public:
        bool check = false;
        uint jump = 0;
    };

    class FieldSortable
    {
    public:
        uint fieldType;
        uint fieldSubType;
        uint index;
    };

    bool fieldCompare(smfigapl::FieldSortable& l, smfigapl::FieldSortable& r);
    void fieldSort(std::span<FieldSortable> fs);

// Team ids in the tables (TEAM_UNDEFINED for virtual teams)
    using TableTeam = std::uint16_t;

    // the index of the pairing of teams a < b among all pairings of NTeams teams
    inline uint getPairingIndexShort(uint NTeams, uint a, uint b)
    {
        return a * (2 * NTeams - 1 - a) / 2 + b - a - 1;
    }

// The candidate rounds of the current atomic set. They are not stored one by
// one: candidate i is a pairing of the teams (from TreeSearch) and a field
// mix (the game of the pairing on every position; positions sorted by field
// type, the positions from NPositions on are pause fields), in the order
// iteration, pairing, field mix (the mixes rotated by the pairing). Virtual
// teams (ids >= NTeams) make a game a pause. Candidates that break the empty
// field or subtype conditions are left out (valid).
    class Candidates
    {
    public:
        uint NTeams = 0;     // real teams
        uint NPositions = 0; // real fields
        uint NGames = 0;     // games of a pairing: fields and pause fields
        uint NPairings = 0;
        uint NMixes = 0;
        std::vector<uint> fieldType;           // field type of every real position
        std::vector<uint> club;                // club of every real team
        std::span<const TableTeam> pairings;   // 2 * NGames teams per pairing
        std::vector<std::uint8_t> mixes;       // NGames per field mix: the game on every position
        std::vector<std::uint16_t> rotation;   // per pairing: its first field mix (pairing % NMixes)
        std::vector<std::uint8_t> positions;   // NGames per field mix: the position of every game
        std::vector<std::uint8_t> group;       // per position: the group of fields (field type and subtype, pause fields)
        std::vector<std::uint32_t> pairingHash; // per pairing without pause fields: hash()
        std::vector<std::uint32_t> valid;      // index among all candidates of every valid one, empty: all are valid
        // the repeated games jumps without pause fields: for every game of
        // every pairing (with iterations) the pairing after the run of
        // pairings that contain the game, and the first valid candidate of
        // every pairing (empty: all candidates are valid)
        std::vector<std::uint32_t> runEnd;
        std::vector<std::uint32_t> validStart;
        uint count = 0;

        // one candidate: team k of the game on a position, TEAM_UNDEFINED for a virtual team
        class View
        {
        private:
            std::span<const TableTeam> pairing_;
            std::span<const std::uint8_t> mix_;
            std::span<const std::uint8_t> positions_;
            uint NTeams_;
        public:
            View(std::span<const TableTeam> pairing, std::span<const std::uint8_t> mix, std::span<const std::uint8_t> positions, uint NTeams):
                pairing_{pairing}, mix_{mix}, positions_{positions}, NTeams_{NTeams}
            {
            }
            // the position of the game of the team and the other team of it,
            // {NGames, TEAM_UNDEFINED} for a virtual team
            [[nodiscard]] std::pair<uint, uint> find(uint team) const
            {
                for (size_t t = 0; t < pairing_.size(); t++)
                    if (pairing_[t] == team)
                    {
                        const TableTeam other = pairing_[t ^ 1U];
                        return {positions_[t / 2], other < NTeams_ ? other : TEAM_UNDEFINED};
                    }
                return {static_cast<uint>(positions_.size()), TEAM_UNDEFINED};
            }
            [[nodiscard]] TableTeam team(uint position, uint k) const
            {
                const TableTeam t = pairing_[2 * static_cast<size_t>(mix_[position]) + k];
                return t < NTeams_ ? t : TEAM_UNDEFINED;
            }
            // the game of the pairing on a position
            [[nodiscard]] uint game(uint position) const
            {
                return mix_[position];
            }
        };

        [[nodiscard]] uint size() const
        {
            return count;
        }
        // index among all candidates (iterations, pairings, field mixes)
        [[nodiscard]] uint full(uint i) const
        {
            return valid.empty() ? i : valid[i];
        }
        // the pairing with its iteration: iteration * NPairings + pairing
        [[nodiscard]] uint sequence(uint i) const
        {
            return full(i) / NMixes;
        }
        [[nodiscard]] View view(uint i) const
        {
            uint f = full(i);
            if (f >= NPairings * NMixes)
                f %= NPairings * NMixes;
            const uint pairing = f / NMixes;
            // (f % NMixes + pairing) % NMixes
            uint mix = f - pairing * NMixes + rotation[pairing];
            if (mix >= NMixes)
                mix -= NMixes;
            const size_t games = NGames;
            return {pairings.subspan(static_cast<size_t>(pairing) * 2 * games, 2 * games), std::span(mixes).subspan(mix * games, games), std::span(positions).subspan(mix * games, games), NTeams};
        }
        [[nodiscard]] bool hasPauseFields() const
        {
            return NGames > NPositions;
        }
        // f(position, a, b) for every game of two real teams on a real field
        template<class F> void forEachGame(uint i, F f) const
        {
            const View v = view(i);
            for (uint j = 0; j < NPositions; j++)
            {
                const uint a = v.team(j, 0);
                const uint b = v.team(j, 1);
                if (a != TEAM_UNDEFINED && b != TEAM_UNDEFINED)
                    f(j, a, b);
            }
        }
        // f(team) for every pause team: the real team of a game with a
        // virtual team, the teams of a pause field
        template<class F> void forEachPause(uint i, F f) const
        {
            const View v = view(i);
            for (uint j = 0; j < NGames; j++)
            {
                const uint a = v.team(j, 0);
                const uint b = v.team(j, 1);
                const bool pauseField = j >= NPositions;
                if (a != TEAM_UNDEFINED && (pauseField || b == TEAM_UNDEFINED))
                    f(a);
                if (b != TEAM_UNDEFINED && (pauseField || a == TEAM_UNDEFINED))
                    f(b);
            }
        }
        // f(team) for both teams of every game of club mates
        template<class F> void forEachSameClub(uint i, F f) const
        {
            forEachGame(i, [&](uint, uint a, uint b)
            {
                if (club[a] == club[b])
                {
                    f(a);
                    f(b);
                }
            });
        }
        // the counter index of the pairing of two teams
        [[nodiscard]] uint pairingKey(uint a, uint b) const
        {
            return getPairingIndexShort(NTeams, a, b);
        }
        // the pause teams in ascending order
        [[nodiscard]] std::vector<TableTeam> pauseTeams(uint i) const;
        // the first teams of the real fields, sorted, as one number
        [[nodiscard]] uint hash(uint i) const;
        [[nodiscard]] bool sameHash(uint i, uint k) const;
        // repeated games: the number of following candidates that contain the
        // game on the position (at most to the last candidate)
        [[nodiscard]] uint gameJump(uint i, uint position) const;
        // field types: the number of following candidates where the team plays
        // on the same field type as in candidate i (up to a pause, at most to
        // the last candidate), 0 if it pauses in i
        [[nodiscard]] uint fieldTypeJump(uint i, uint team) const;
        // the position of the team in candidate i, NPositions if it pauses
        [[nodiscard]] uint position(uint i, uint team) const;
        [[nodiscard]] bool containsGame(uint i, uint key) const;
    };

    class CondTables
    {
    public:
        Candidates candidates;
        // the fewest and the most teams per candidate on every field type
        // and pausing, over all candidates (for the counting checks)
        struct Bounds
        {
            std::vector<uint> minSlots; // [field type]
            std::vector<uint> maxSlots; // [field type]
            uint minPauses = 0;
            uint maxPauses = 0;
        } bounds;

        std::vector<uint> searchIndexMax;
        std::vector<smfigapl::Capacity> atomicCapacityArray;
        std::vector<smfigapl::Mapping> atomicMappingArray;
        std::vector<std::vector<uint>> fieldMappingArray;
        std::vector<std::vector<uint>> teamsMappingArray;
        std::vector<uint> teamIdMappingArray;
        uint atomicIndex = 0;
    };

    class TreeSearchState
    {
    public:
        int Add = 0;
        int Sub = 0;
        StepResult stepResult = StepResult::success;
    };

    uint getNPairingIndexShort(uint NTeams);
    void displayPairingIndicesShort(std::ostream& os, uint NTeams);

    class GamePlanRoundByTeamId
    {
    private:
        std::vector<uint> gamePlanRoundByTeamId_;
    public:
        GamePlanRoundByTeamId() = default;
        GamePlanRoundByTeamId(uint NFields): gamePlanRoundByTeamId_(NFields << 1)
        {
        }
        void init(uint NFields);
        void reset();
        uint getTeamId(uint index);
        uint getTeamId(uint field, uint gameIndex);
        void setTeamId(uint index, uint teamId);
        void setTeamId(uint field, uint gameIndex, uint teamId);
        uint size();
    };

    class GamePlanByTeamId
    {
    private:
        std::vector<GamePlanRoundByTeamId> gamePlanByTeamId_;
    public:
        GamePlanByTeamId(uint NRounds, uint NFields): gamePlanByTeamId_(NRounds, GamePlanRoundByTeamId(NFields))
        {
        }
        uint getTeamId(uint round, uint index);
        uint getTeamId(uint round, uint field, uint gameIndex);
        void setTeamId(uint round, uint index, uint teamId);
        void setTeamId(uint round, uint field, uint gameIndex, uint teamId);
        GamePlanRoundByTeamId getRound(uint round);

    };

    GamePlanRoundByTeamId roundFieldMix(GamePlanRoundByTeamId roundArray, std::span<const uint> mixArray);

}//namespace smfigapl
