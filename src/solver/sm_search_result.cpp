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


// The game plan of a finished search: indexed game plan and plan by name, home and
// away balance, order of the rounds

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "smfigapl/solver/sm_search.hpp"
#include "smfigapl/model/sm_math.hpp"
#include "smfigapl/model/sm_debug.hpp"

std::string smfigapl::GamePlanSearch::getClubNameByTeamId(uint id)
{
    return input_.setup.namedTeamByTeamId[id].getTeamClub();
}

std::string smfigapl::GamePlanSearch::getTeamColorByTeamId(uint id)
{
    return input_.setup.namedTeamByTeamId[id].getTeamColor();
}

void smfigapl::GamePlanSearch::calcIndexedGamePlan()
{
    smfigapl::IndexedRound round;
    const uint NFields = condTables_.atomicCapacityArray[condTables_.atomicIndex].NFields;
    // the candidates have their games in the order of the fields sorted by
    // type: position p is the field getFieldTypeSortArray(p); field j needs
    // the inverse
    std::vector<uint> positionByField(NFields, 0);
    for (uint p = 0; p < NFields; p++)
        if (const uint field = ts_.getFieldTypeSortArray(p); field < NFields)
            positionByField[field] = p;
    for (uint i = 0; i < condTables_.atomicCapacityArray[condTables_.atomicIndex].NRounds; i++)
    {
        round = tables_.candidate(treeSearchIndex_[i]);
        for (uint j = 0; j < NFields; j++)
        {
            IndexedGame game;
            game = round.getGame(positionByField[j]);
            for (uint k = 0; k < 2; k++)
            {
                IndexedTeam team = game.getTeamByGameId(k);
                uint teamId = team.getTeamId();
                auto a = condTables_.teamsMappingArray[condTables_.atomicIndex];
                if (teamId == TEAM_UNDEFINED)
                    team.setTeamId(TEAM_UNDEFINED);
                else
                    team.setTeamId(condTables_.teamsMappingArray[condTables_.atomicIndex][teamId]);
                game.setTeam(k, team);
            }
            if (output_.indexedGamePlan.getGame(i, condTables_.fieldMappingArray[condTables_.atomicIndex][j]).getSwap())
                game.setSwap(true);
            output_.indexedGamePlan.setGame(i, condTables_.fieldMappingArray[condTables_.atomicIndex][j], game);
        }
        for (auto&& p : round.getPauseTeams())
        {
            uint teamId = p.getTeamId();
            p.setTeamId(condTables_.teamsMappingArray[condTables_.atomicIndex][teamId]);
            output_.indexedGamePlan.addPauseTeamAtomic(i, p, condTables_.atomicIndex);
        }
    }
}

void smfigapl::GamePlanSearch::calcGamePlan()
{
    auto teamRef = [this](uint id)
    {
        return TeamRef{getClubNameByTeamId(id), getTeamColorByTeamId(id)};
    };
    GamePlan& plan = output_.plan;
    plan.rounds.assign(input_.setup.realCapacity.NRounds, Round{});
    for (uint i = 0; i < input_.setup.realCapacity.NRounds; i++)
    {
        Round& round = plan.rounds[i];
        for (uint j = 0; j < input_.setup.realCapacity.NFields; j++)
        {
            Game& game = round.games.emplace_back();
            game.field = input_.setup.fieldNameByFieldId[j];
            IndexedGame indexed = output_.indexedGamePlan.getGame(i, j);
            if (!indexed.getPause())
            {
                game.home = teamRef(indexed.getTeamByGameId(0).getTeamId());
                game.away = teamRef(indexed.getTeamByGameId(1).getTeamId());
            }
        }
        // in the order of the sub tournaments
        for (uint j = 0; j < output_.indexedGamePlan.getNPauseTeams(i); j++)
            round.pauses.push_back(teamRef(output_.indexedGamePlan.getPauseTeam(i, j).getTeamId()));
    }
    plan.subTournaments.assign(atomicSets_.count, SubTournament{});
    for (uint j = 0; j < input_.setup.realCapacity.NFields; j++)
        plan.subTournaments[atomicSets_.indexByFieldId[j]].fields.push_back(input_.setup.fieldNameByFieldId[j]);
    for (uint t = 0; t < input_.setup.realCapacity.NTeams; t++)
        plan.subTournaments[atomicSets_.indexByTeamId[t]].teams.push_back(teamRef(t));
}

namespace
{
    using smfigapl::uint;

    // Directions for the edges of a multigraph such that every vertex has as
    // many outgoing as incoming edges, one more of either at a vertex of odd
    // degree. The vertices of odd degree (an even number of them) are joined
    // to an extra vertex; then every vertex has an even degree, and a walk
    // along unused edges can only get stuck where it started: the edges
    // split into closed walks, each of which leaves a vertex as often as it
    // enters it. Without the extra edges, a vertex loses at most one of
    // either. Per edge: true if it goes from its first to its second vertex.
    std::vector<bool> balancedDirections(uint NVertices, const std::vector<std::pair<uint, uint>>& edges)
    {
        std::vector<std::pair<uint, uint>> all = edges;
        std::vector<uint> degree(NVertices + 1, 0);
        for (const auto& [a, b] : edges)
        {
            degree[a]++;
            degree[b]++;
        }
        for (uint v = 0; v < NVertices; v++)
            if (degree[v] % 2 == 1)
                all.emplace_back(v, NVertices);
        // per vertex its edges, in their order: the result is deterministic
        std::vector<std::vector<uint>> incident(NVertices + 1);
        for (uint e = 0; e < all.size(); e++)
        {
            incident[all[e].first].push_back(e);
            incident[all[e].second].push_back(e);
        }
        std::vector<bool> used(all.size(), false);
        std::vector<bool> forward(all.size(), true);
        std::vector<uint> next(NVertices + 1, 0);
        for (uint start = 0; start <= NVertices; start++)
        {
            uint v = start;
            while (true)
            {
                while (next[v] < incident[v].size() && used[incident[v][next[v]]])
                    next[v]++;
                if (next[v] == incident[v].size())
                {
                    if (v == start)
                        break;
                    // cannot happen with even degrees; ends the walk all the same
                    v = start;
                    continue;
                }
                const uint e = incident[v][next[v]];
                used[e] = true;
                forward[e] = all[e].first == v;
                v = forward[e] ? all[e].second : all[e].first;
            }
        }
        forward.resize(edges.size());
        return forward;
    }
}

void smfigapl::GamePlanSearch::countHomeAway(HomeAwayCount& count)
{
    const Capacity& capacity = condTables_.atomicCapacityArray[condTables_.atomicIndex];
    const uint atomicIndex = condTables_.atomicIndex;
    count.perTeam.clear();
    count.max = {0, 0};
    for (uint l = 0; l < capacity.NTeams; l++)
    {
        std::array<uint, 2> games = {0, 0};
        const uint team = condTables_.teamsMappingArray[atomicIndex][l];
        for (uint i = 0; i < capacity.NRounds; i++)
        {
            if (output_.indexedGamePlan.isHomeAtomic(i, team, atomicIndex))
                games[0]++;
            else if (output_.indexedGamePlan.isAwayAtomic(i, team, atomicIndex))
                games[1]++;
        }
        for (uint k = 0; k < 2; k++)
        {
            if (games[k] > count.max[k])
            {
                count.max[k] = games[k];
                count.maxTeam[k] = l;
            }
        }
        count.perTeam.push_back(games);
    }
}

void smfigapl::GamePlanSearch::swapHomeAway(const HomeAwayCount& count, uint homeAway)
{
    const Capacity& capacity = condTables_.atomicCapacityArray[condTables_.atomicIndex];
    const uint atomicIndex = condTables_.atomicIndex;
    //games of the team with the maximum of home/away games
    std::vector<std::tuple<uint, uint>> maxGames;
    for (uint i = 0; i < capacity.NRounds; i++)
    {
        for (uint j = 0; j < capacity.NFields; j++)
        {
            const uint teamId = output_.indexedGamePlan.getGameAtomic(i, j, atomicIndex).getTeamByGameId(homeAway).getTeamId();
            if (teamId != TEAM_UNDEFINED && condTables_.teamIdMappingArray[teamId] == count.maxTeam[homeAway])
                maxGames.emplace_back(i, j);
        }
    }

    // no game to swap (e.g. only empty fields)
    if (maxGames.empty())
        return;
    //From one of these choose the pairing where the other team has highest away /home games (different order!)
    uint maxOther = 0;
    uint maxIndex = 0;
    for (uint i = 0; i < maxGames.size(); i++)
    {
        const uint teamId = output_.indexedGamePlan.getGameAtomic(std::get<0>(maxGames[i]), std::get<1>(maxGames[i]), atomicIndex).getTeamByGameId(1 - homeAway).getTeamId();
        if (teamId != TEAM_UNDEFINED)
        {
            const uint l = count.perTeam[condTables_.teamIdMappingArray[teamId]][1 - homeAway];
            if (l > maxOther)
            {
                maxOther = l;
                maxIndex = i;
            }
        }
    }
    output_.indexedGamePlan.flipSwapAtomic(std::get<0>(maxGames[maxIndex]), std::get<1>(maxGames[maxIndex]), atomicIndex);
}

bool smfigapl::GamePlanSearch::homeAwayBalanced(const HomeAwayCount& count)
{
    return std::ranges::all_of(count.perTeam, [](const std::array<uint, 2>& games)
    {
        return std::max(games[0], games[1]) - std::min(games[0], games[1]) <= 1;
    });
}

void smfigapl::GamePlanSearch::balancedHomeAwayCalc()
{
    const Capacity& capacity = condTables_.atomicCapacityArray[condTables_.atomicIndex];
    const uint atomicIndex = condTables_.atomicIndex;
    // swaps a game of the team with the most home (away) games, a few times
    uint homeAway = 0; //0= start with homegame, 1= start with away game
    const uint maxLoops = capacity.NRounds * (capacity.NRounds - 1);
    uint loops = 0;
    HomeAwayCount count;
    countHomeAway(count);
    while (loops < maxLoops && !homeAwayBalanced(count))
    {
        swapHomeAway(count, homeAway);
        homeAway = count.max[0] >= count.max[1] ? 0 : 1;
        countHomeAway(count);
        loops++;
    }
    if (input_.dbg.has(DebugFlag::gps))
        input_.dbg.out(DebugFlag::gps) << "HomeAway Loops: " << loops << std::endl;
    if (homeAwayBalanced(count))
        return;
    // the swaps can move an imbalance from team to team without an end:
    // directions along closed walks balance every team (balancedDirections)
    // every game as an edge from its home to its away team (local indexes)
    std::vector<std::pair<uint, uint>> edges;
    std::vector<std::pair<uint, uint>> places;
    for (uint i = 0; i < capacity.NRounds; i++)
    {
        for (uint j = 0; j < capacity.NFields; j++)
        {
            IndexedGame game = output_.indexedGamePlan.getGameAtomic(i, j, atomicIndex);
            const uint home = game.getTeamByGameId(0).getTeamId();
            const uint away = game.getTeamByGameId(1).getTeamId();
            if (game.getPause() || home == TEAM_UNDEFINED || away == TEAM_UNDEFINED)
                continue;
            edges.emplace_back(condTables_.teamIdMappingArray[home], condTables_.teamIdMappingArray[away]);
            places.emplace_back(i, j);
        }
    }
    const std::vector<bool> forward = balancedDirections(capacity.NTeams, edges);
    uint swaps = 0;
    for (uint e = 0; e < edges.size(); e++)
    {
        if (!forward[e])
        {
            output_.indexedGamePlan.flipSwapAtomic(places[e].first, places[e].second, atomicIndex);
            swaps++;
        }
    }
    if (input_.dbg.has(DebugFlag::gps))
        input_.dbg.out(DebugFlag::gps) << "HomeAway swaps: " << swaps << std::endl;
}

namespace
{
    using smfigapl::uint;

    // The pause teams of a round as a bit mask and the game of every field
    // as an ordered pair of teams (TEAM_UNDEFINED: pause field)
    struct RoundShape
    {
        std::uint64_t pauses = 0;
        std::vector<std::pair<uint, uint>> games;
    };

    RoundShape roundShape(smfigapl::IndexedRound round, const std::vector<uint>& fieldOrder)
    {
        RoundShape shape;
        for (smfigapl::IndexedTeam& t : round.getPauseTeams())
            shape.pauses |= std::uint64_t{1} << t.getTeamId();
        for (const uint field : fieldOrder)
        {
            smfigapl::IndexedGame game = round.getGame(field);
            const uint a = game.getTeamByGameId(0).getTeamId();
            const uint b = game.getTeamByGameId(1).getTeamId();
            if (game.getPause())
                shape.games.emplace_back(smfigapl::TEAM_UNDEFINED, smfigapl::TEAM_UNDEFINED);
            else
                shape.games.emplace_back(std::min(a, b), std::max(a, b));
        }
        return shape;
    }

    bool compatibleRounds(const RoundShape& r, const RoundShape& s)
    {
        if ((r.pauses & s.pauses) != 0)
            return false;
        for (size_t j = 0; j < r.games.size(); j++)
            if (r.games[j].first != smfigapl::TEAM_UNDEFINED && r.games[j] == s.games[j])
                return false;
        return true;
    }

    // The lexicographically first order of the rounds in which round
    // order[i + 1] is in compatible[order[i]] (bit mask), empty without one.
    // Dynamic program over subsets: finish[rest] has bit v set when the
    // rounds of `rest` can follow each other starting with round v.
    std::vector<uint> firstRoundOrder(const std::vector<std::uint32_t>& compatible)
    {
        const auto NRounds = static_cast<uint>(compatible.size());
        const std::uint32_t all = (std::uint32_t{1} << NRounds) - 1;
        std::vector<std::uint32_t> finish(std::size_t{all} + 1, 0);
        for (std::uint32_t rest = 1; rest <= all; rest++)
            for (uint v = 0; v < NRounds; v++)
            {
                const std::uint32_t bit = std::uint32_t{1} << v;
                if ((rest & bit) != 0 && (rest == bit || (compatible[v] & finish[rest ^ bit]) != 0))
                    finish[rest] |= bit;
            }
        std::vector<uint> order;
        if (finish[all] == 0)
            return order;
        std::uint32_t rest = all;
        std::uint32_t allowed = all;
        while (rest != 0)
        {
            const auto v = static_cast<uint>(std::countr_zero(finish[rest] & allowed));
            order.push_back(v);
            rest ^= std::uint32_t{1} << v;
            allowed = compatible[v];
        }
        return order;
    }

    // rank of a permutation among all permutations in lexicographic order
    std::uint64_t permutationRank(const std::vector<uint>& order)
    {
        std::uint64_t rank = 0;
        for (size_t i = 0; i < order.size(); i++)
        {
            const auto smaller = std::ranges::count_if(order.begin() + static_cast<std::ptrdiff_t>(i) + 1, order.end(), [&](uint k)
            {
                return k < order[i];
            });
            rank = rank * (order.size() - i) + static_cast<std::uint64_t>(smaller);
        }
        return rank;
    }
}

// Adjacent rounds are compatible when no team of the atomic set pauses in
// both and no field has the same game (pair of teams, either order) in both.
// The order of the rounds is the lexicographically first order in which all
// adjacent rounds are compatible. The rounds are the stored rounds of the
// search (local team ids), independent of the order set before.
// Without such an order (or with more than MAX_OPTIMIZE_ROUNDS rounds or 64
// teams) the order of the search stays.
void smfigapl::GamePlanSearch::optimizeRounds()
{
    constexpr uint MAX_OPTIMIZE_ROUNDS = 20;
    const uint atomicIndex = condTables_.atomicIndex;
    const Capacity& capacity = condTables_.atomicCapacityArray[atomicIndex];
    std::vector<uint> order;
    if (capacity.NRounds <= MAX_OPTIMIZE_ROUNDS && capacity.NTeams <= 64)
    {
        std::vector<uint> fieldOrder;
        for (uint j = 0; j < capacity.NFields; j++)
            fieldOrder.push_back(ts_.getFieldTypeSortArray(j));
        std::vector<RoundShape> shapes;
        for (uint i = 0; i < capacity.NRounds; i++)
            shapes.push_back(roundShape(tables_.candidate(treeSearchIndex_[i]), fieldOrder));
        std::vector<std::uint32_t> compatible(capacity.NRounds, 0);
        for (uint i = 0; i < capacity.NRounds; i++)
            for (uint k = 0; k < capacity.NRounds; k++)
                if (i != k && compatibleRounds(shapes[i], shapes[k]))
                    compatible[i] |= std::uint32_t{1} << k;
        order = firstRoundOrder(compatible);
    }
    // 1 + the rank of the order among all permutations, 0: no valid order
    const std::uint64_t rank = order.empty() ? 0 : permutationRank(order) + 1;
    if (order.empty())
    {
        order.resize(capacity.NRounds);
        std::iota(order.begin(), order.end(), 0U);
    }
    output_.indexedGamePlan.setResortOrder(order, atomicIndex);
    if (input_.dbg.has(DebugFlag::gps))
        input_.dbg.out(DebugFlag::gps) << "Optimize Rounds, Atomic: " << atomicIndex << " Idx: " << rank << std::endl;
}
