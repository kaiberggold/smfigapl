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

// The conditions a given game plan breaks, on the public types only. The
// app checks plans changed by hand with it.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "smfigapl/planner.hpp"

namespace
{
    using smfigapl::Problem;
    using smfigapl::ProblemCode;
    using smfigapl::Severity;
    using smfigapl::TeamRef;

    std::string teamName(const TeamRef& team)
    {
        return team.club + " " + team.team;
    }

    std::string roundPath(std::size_t round)
    {
        return "rounds[" + std::to_string(round) + "]";
    }

    std::string gamePath(std::size_t round, std::size_t game)
    {
        return roundPath(round) + ".games[" + std::to_string(game) + "]";
    }

    Problem problem(ProblemCode code, std::vector<std::string> parameters, std::string path, Severity severity = Severity::warning)
    {
        return Problem{code, std::move(parameters), std::move(path), severity, std::nullopt};
    }

    // What the plan does per team, by the team's index in the setup
    struct TeamCounts
    {
        std::uint32_t games = 0;
        std::uint32_t sameClubGames = 0;
        std::map<std::string, std::uint32_t> gamesPerFieldType;
    };

    class Checker
    {
    public:
        Checker(const smfigapl::TournamentSetup& setup, const smfigapl::GamePlan& plan): setup_(setup), plan_(plan)
        {
            for (const smfigapl::Club& club : setup.clubs)
                for (const std::string& team : club.teams)
                    teams_.push_back({club.name, team});
            counts_.resize(teams_.size());
        }

        std::vector<Problem> run()
        {
            if (plan_.rounds.size() != setup_.rounds)
                problems_.push_back(problem(ProblemCode::planRounds, {std::to_string(plan_.rounds.size()), std::to_string(setup_.rounds)}, "rounds",
                                            Severity::error));
            for (std::size_t r = 0; r < plan_.rounds.size(); r++)
                round(r);
            totals();
            return std::move(problems_);
        }

    private:
        const smfigapl::TournamentSetup& setup_;
        const smfigapl::GamePlan& plan_;
        std::vector<TeamRef> teams_;
        std::vector<TeamCounts> counts_;
        // the games of every pairing (team indices, the smaller first)
        std::map<std::pair<std::size_t, std::size_t>, std::uint32_t> pairings_;
        std::vector<Problem> problems_;

        // The index of a team in the setup; teams_.size() if it is not there
        [[nodiscard]] std::size_t indexOf(const TeamRef& team) const
        {
            return static_cast<std::size_t>(std::ranges::find(teams_, team) - teams_.begin());
        }

        [[nodiscard]] bool denied(const TeamRef& team, const std::string& subtype) const
        {
            return std::ranges::any_of(setup_.conditions.subtypeDeny, [&](const smfigapl::SubtypeDeny & deny)
            {
                return deny.subtype == subtype && std::ranges::find(deny.teams, team) != deny.teams.end();
            });
        }

        // The teams of a game: known, once in the round, on a field they may use
        void team(const TeamRef& team, std::size_t r, std::size_t g, std::set<std::size_t>& playing)
        {
            const std::string round = std::to_string(r + 1);
            const std::size_t t = indexOf(team);
            if (t == teams_.size())
            {
                problems_.push_back(problem(ProblemCode::planUnknownTeam, {teamName(team), round}, gamePath(r, g), Severity::error));
                return;
            }
            if (!playing.insert(t).second)
                problems_.push_back(problem(ProblemCode::planTeamTwice, {teamName(team), round}, roundPath(r)));
            if (g < setup_.fields.size() && denied(team, setup_.fields[g].subtype))
                problems_.push_back(problem(ProblemCode::planSubtypeDenied, {teamName(team), setup_.fields[g].name, round}, gamePath(r, g)));
        }

        void game(const smfigapl::Game& game, std::size_t r, std::size_t g, std::set<std::size_t>& playing)
        {
            if (game.home.has_value() != game.away.has_value())
            {
                // an open place (one team removed by hand): the field counts
                // as empty, the remaining team as pausing
                const TeamRef& alone = game.home ? *game.home : *game.away;
                problems_.push_back(problem(ProblemCode::planOpenGame, {teamName(alone), std::to_string(r + 1)}, gamePath(r, g)));
                return;
            }
            if (!game.home || !game.away)
                return;
            team(*game.home, r, g, playing);
            team(*game.away, r, g, playing);
            const std::size_t home = indexOf(*game.home);
            const std::size_t away = indexOf(*game.away);
            if (home == teams_.size() || away == teams_.size() || home == away)
                return;
            const bool sameClub = game.home->club == game.away->club;
            for (const std::size_t t : {home, away})
            {
                counts_[t].games++;
                if (sameClub)
                    counts_[t].sameClubGames++;
                if (g < setup_.fields.size())
                    counts_[t].gamesPerFieldType[setup_.fields[g].type]++;
            }
            pairings_[ {std::min(home, away), std::max(home, away)}]++;
        }

        void round(std::size_t r)
        {
            const smfigapl::Round& round = plan_.rounds[r];
            if (round.games.size() != setup_.fields.size())
            {
                problems_.push_back(problem(ProblemCode::planFields, {std::to_string(r + 1), std::to_string(round.games.size()), std::to_string(setup_.fields.size())},
                                            roundPath(r), Severity::error));
                return;
            }
            std::set<std::size_t> playing;
            for (std::size_t g = 0; g < round.games.size(); g++)
            {
                if (round.games[g].field != setup_.fields[g].name)
                    problems_.push_back(problem(ProblemCode::planFields, {std::to_string(r + 1), round.games[g].field, setup_.fields[g].name}, gamePath(r, g),
                                                Severity::error));
                game(round.games[g], r, g, playing);
            }
            emptyFields(r);
        }

        // Every sub tournament may leave maxEmptyFields of its fields empty
        void emptyFields(std::size_t r)
        {
            const smfigapl::Round& round = plan_.rounds[r];
            std::vector<std::vector<std::string>> groups;
            groups.reserve(plan_.subTournaments.size() + 1);
            for (const smfigapl::SubTournament& sub : plan_.subTournaments)
                groups.push_back(sub.fields);
            if (groups.empty())
            {
                std::vector<std::string>& all = groups.emplace_back();
                all.reserve(setup_.fields.size());
                for (const smfigapl::Field& field : setup_.fields)
                    all.push_back(field.name);
            }
            for (const std::vector<std::string>& fields : groups)
            {
                const auto empty = static_cast<std::uint32_t>(std::ranges::count_if(round.games, [&](const smfigapl::Game & game)
                {
                    return (!game.home || !game.away) && std::ranges::find(fields, game.field) != fields.end();
                }));
                if (empty > setup_.conditions.maxEmptyFields)
                    problems_.push_back(problem(ProblemCode::planEmptyFields, {std::to_string(r + 1), std::to_string(empty), std::to_string(setup_.conditions.maxEmptyFields)},
                                                roundPath(r)));
            }
        }

        void fieldTypes(const TeamRef& team, const TeamCounts& counts)
        {
            for (const smfigapl::FieldTypeLimit& limit : setup_.conditions.fieldTypes)
            {
                const auto found = counts.gamesPerFieldType.find(limit.type);
                const std::uint32_t games = found == counts.gamesPerFieldType.end() ? 0 : found->second;
                if (limit.min && games < *limit.min)
                    problems_.push_back(problem(ProblemCode::planFieldTypeMin, {teamName(team), limit.type, std::to_string(games), std::to_string(*limit.min)}, ""));
                if (limit.max && games > *limit.max)
                    problems_.push_back(problem(ProblemCode::planFieldTypeMax, {teamName(team), limit.type, std::to_string(games), std::to_string(*limit.max)}, ""));
            }
        }

        // The conditions over all rounds, team by team, then the pairings
        void totals()
        {
            const smfigapl::Conditions& c = setup_.conditions;
            const auto rounds = static_cast<std::uint32_t>(plan_.rounds.size());
            for (std::size_t t = 0; t < teams_.size(); t++)
            {
                const TeamCounts& counts = counts_[t];
                const std::string name = teamName(teams_[t]);
                const std::uint32_t pauses = rounds - std::min(rounds, counts.games);
                if (c.maxPauses && pauses > *c.maxPauses)
                    problems_.push_back(problem(ProblemCode::planPauses, {name, std::to_string(pauses), std::to_string(*c.maxPauses)}, ""));
                if (c.maxSameClubGames && counts.sameClubGames > *c.maxSameClubGames)
                    problems_.push_back(problem(ProblemCode::planSameClub, {name, std::to_string(counts.sameClubGames), std::to_string(*c.maxSameClubGames)}, ""));
                fieldTypes(teams_[t], counts);
            }
            const std::uint32_t allowed = setup_.repeatPairings ? std::max<std::uint32_t>(1, setup_.iterations) : 1;
            for (const auto& [pairing, games] : pairings_)
                if (games > allowed)
                    problems_.push_back(problem(ProblemCode::planRepeatedPairing,
                {teamName(teams_[pairing.first]), teamName(teams_[pairing.second]), std::to_string(games), std::to_string(allowed)}, ""));
        }
    };
}

std::vector<smfigapl::Problem> smfigapl::checkPlan(const TournamentSetup& setup, const GamePlan& plan) noexcept
{
    try
    {
        return Checker(setup, plan).run();
    }
    catch (const std::exception& e)
    {
        return {problem(ProblemCode::internal, {e.what()}, "", Severity::error)};
    }
    catch (...)
    {
        return {problem(ProblemCode::internal, {"unknown exception"}, "", Severity::error)};
    }
}
