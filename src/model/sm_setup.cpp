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
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "smfigapl/model/sm_setup.hpp"
#include "smfigapl/model/sm_except.hpp"

namespace
{
    // An error of the setup
    smfigapl::Problem error(smfigapl::ProblemCode code, std::vector<std::string> parameters, std::string path)
    {
        smfigapl::Problem p;
        p.code = code;
        p.parameters = std::move(parameters);
        p.path = std::move(path);
        return p;
    }

    std::string at(const std::string& path, std::size_t index)
    {
        return path + "[" + std::to_string(index) + "]";
    }

    void validateClubs(const smfigapl::TournamentSetup& setup, std::vector<smfigapl::Problem>& problems)
    {
        using smfigapl::ProblemCode;
        // a club counts with its first team, names of clubs and colors are shared
        std::set<std::string> clubs;
        std::set<std::string> colors;
        bool clubsFull = false;
        bool colorsFull = false;
        for (std::size_t i = 0; i < setup.clubs.size(); i++)
            for (std::size_t j = 0; j < setup.clubs[i].teams.size(); j++)
            {
                clubs.insert(setup.clubs[i].name);
                colors.insert(setup.clubs[i].teams[j]);
                if (!clubsFull && clubs.size() > smfigapl::MAX_CLUBS)
                {
                    clubsFull = true;
                    problems.push_back(error(ProblemCode::clubIndexGtMax, {std::to_string(smfigapl::MAX_CLUBS)}, at("clubs", i)));
                }
                if (!colorsFull && colors.size() > static_cast<std::size_t>(smfigapl::MAX_TEAMCOLORS))
                {
                    colorsFull = true;
                    problems.push_back(error(ProblemCode::teamColorIndexGtMax, {std::to_string(smfigapl::MAX_TEAMCOLORS)}, at(at("clubs", i) + ".teams", j)));
                }
            }
        if (clubs.empty())
            problems.push_back(error(ProblemCode::noClubsDefined, {"clubs"}, "clubs"));
    }

    // Whether a field has the type or subtype (member) name
    bool hasField(const smfigapl::TournamentSetup& setup, std::string smfigapl::Field::* member, const std::string& name)
    {
        return std::ranges::find(setup.fields, name, member) != setup.fields.end();
    }

    bool hasTeam(const smfigapl::TournamentSetup& setup, const smfigapl::TeamRef& team)
    {
        return std::ranges::any_of(setup.clubs, [&team](const smfigapl::Club & c)
        {
            return c.name == team.club && std::ranges::find(c.teams, team.team) != c.teams.end();
        });
    }

    void validateConditions(const smfigapl::TournamentSetup& setup, std::vector<smfigapl::Problem>& problems)
    {
        using smfigapl::ProblemCode;
        const smfigapl::Conditions& c = setup.conditions;
        for (std::size_t i = 0; i < c.fieldTypes.size(); i++)
        {
            const smfigapl::FieldTypeLimit& limit = c.fieldTypes[i];
            const std::string path = at("conditions.fieldTypes", i);
            if (limit.min && limit.max && *limit.max < *limit.min)
                problems.push_back(error(ProblemCode::maxLtMin, {std::to_string(*limit.min), std::to_string(*limit.max)}, path));
            if (!hasField(setup, &smfigapl::Field::type, limit.type))
                problems.push_back(error(ProblemCode::invalidValue, {"field_types", limit.type}, path + ".type"));
        }
        for (std::size_t i = 0; i < c.subtypeDeny.size(); i++)
        {
            const smfigapl::SubtypeDeny& deny = c.subtypeDeny[i];
            const std::string path = at("conditions.subtypeDeny", i);
            if (!hasField(setup, &smfigapl::Field::subtype, deny.subtype))
                problems.push_back(error(ProblemCode::invalidValue, {"subtypes", deny.subtype}, path + ".subtype"));
            for (std::size_t j = 0; j < deny.teams.size(); j++)
                if (!hasTeam(setup, deny.teams[j]))
                    problems.push_back(error(ProblemCode::invalidValue, {"teams_deny", deny.teams[j].club + " " + deny.teams[j].team}, at(path + ".teams", j)));
        }
    }

    // The id of a team; throws if there is none
    smfigapl::uint teamId(const smfigapl::Input& in, const smfigapl::TeamRef& team)
    {
        for (smfigapl::uint i = 0; i < in.setup.realCapacity.NTeams; i++)
            if (in.setup.namedTeamByTeamId[i].getTeamClub() == team.club && in.setup.namedTeamByTeamId[i].getTeamColor() == team.team)
                return i;
        throw smfigapl::Exception(smfigapl::ErrorCode::invalidValue, {"teams_deny"}, team.club + " " + team.team);
    }
}

std::vector<smfigapl::Problem> smfigapl::validateSetup(const TournamentSetup& setup)
{
    std::vector<Problem> problems;
    if (setup.rounds > MAX_ROUNDS)
        problems.push_back(error(ProblemCode::nRoundsGtMax, {std::to_string(MAX_ROUNDS)}, "rounds"));
    if (setup.rounds < 1)
        problems.push_back(error(ProblemCode::nRoundsLt1, {}, "rounds"));
    if (setup.fields.size() > MAX_FIELDS)
        problems.push_back(error(ProblemCode::nFieldsGtMax, {std::to_string(MAX_FIELDS)}, "fields"));
    if (setup.fields.empty())
        problems.push_back(error(ProblemCode::nFieldsLt1, {}, "fields"));
    validateClubs(setup, problems);
    validateConditions(setup, problems);
    return problems;
}

void smfigapl::buildInput(const TournamentSetup& setup, Input& in)
{
    in.setNRounds(setup.rounds);
    in.conditions.iterations = setup.iterations;
    in.conditions.repeatPairings = setup.repeatPairings;
    in.conditions.homeAwayBalance = setup.homeAwayBalance;
    in.conditions.optimizeRounds = setup.optimizeRoundOrder;
    for (const Field& f : setup.fields)
        in.addField(f.type, f.subtype, f.name);
    // ids in the order of the clubs and their teams
    for (const Club& c : setup.clubs)
        for (const std::string& team : c.teams)
            in.addTeam(c.name, team);

    // after the teams: some conditions have a value per team
    const Conditions& c = setup.conditions;
    if (c.maxPauses)
        in.conditions.setMaxPauseGames(*c.maxPauses);
    if (c.maxSameClubGames)
        in.conditions.setMaxGamesSameClub(*c.maxSameClubGames);
    in.conditions.setMaxFieldsEmpty(c.maxEmptyFields);
    if (!c.fieldTypes.empty())
        in.conditions.limGamesFieldType.enabled = true;
    for (const FieldTypeLimit& limit : c.fieldTypes)
    {
        // an unset value does not restrict
        in.setMinGamesFieldType(limit.type, limit.min.value_or(0));
        in.setMaxGamesFieldType(limit.type, limit.max.value_or(std::numeric_limits<uint>::max()));
    }
    for (const SubtypeDeny& deny : c.subtypeDeny)
    {
        auto subtype = in.setup.fieldSubTypeIdBySubTypeName.find(deny.subtype);
        if (subtype == in.setup.fieldSubTypeIdBySubTypeName.end())
            throw Exception(ErrorCode::invalidValue, {"subtypes"}, deny.subtype);
        for (const TeamRef& team : deny.teams)
        {
            in.conditions.fieldSubTypeDeny.enabled = true;
            in.conditions.fieldSubTypeDeny.values[teamId(in, team)][subtype->second] = 1U;
        }
    }
}
