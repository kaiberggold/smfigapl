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

#include <limits>
#include <utility>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_except.hpp"

void smfigapl::Input::setNRounds(uint n)
{
    setup.realCapacity.NRounds = n;
}

smfigapl::ClubId smfigapl::Input::addClubNameIfNew(const std::string& s)
{
    auto p = std::ranges::find(setup.clubNameByClubId, s);
    if (p != setup.clubNameByClubId.end())
        return ClubId{static_cast<uint>(std::distance(setup.clubNameByClubId.begin(), p))};//already contained
    // a further team of the last possible club is no new club
    if (setup.realCapacity.NClubs < smfigapl::MAX_CLUBS)
    {
        setup.clubNameByClubId.push_back(s);
        const ClubId id{setup.realCapacity.NClubs};
        setup.clubIdByClubName[s] = id;
        setup.realCapacity.NClubs++;
        return id;
    }
    throw Exception(ErrorCode::clubIndexGtMax, {std::to_string(MAX_CLUBS)});
}

smfigapl::uint smfigapl::Input::addIfNew(const std::string& s, uint max, ErrorCode valId, std::vector<std::string>& array, uint& counter)
{
    if (counter > max)
    {
        throw Exception(valId, {std::to_string(max)});
    }
    auto p = std::ranges::find(array, s);
    if (p != array.end())
        return static_cast<uint>(std::distance(array.begin(), p));//already contained

    counter++;
    array.push_back(s);
    return counter - 1;
}

smfigapl::ColorId smfigapl::Input::addTeamColorIfNew(const std::string& s)
{
    auto p = std::ranges::find(setup.colorNameByColorId, s);
    if (p != setup.colorNameByColorId.end())
        return ColorId{static_cast<uint>(std::distance(setup.colorNameByColorId.begin(), p))};//already contained
    // the number of colors is limited, not the length of the name
    if (setup.realCapacity.NTeamColors >= static_cast<uint>(MAX_TEAMCOLORS))
        throw Exception(ErrorCode::teamColorIndexGtMax, {std::to_string(MAX_TEAMCOLORS)});
    setup.colorNameByColorId.push_back(s);
    const ColorId id{setup.realCapacity.NTeamColors};
    setup.colorIdByColor[s] = id;
    setup.realCapacity.NTeamColors++;
    return id;
}

void smfigapl::Input::addField(const std::string& type, const std::string& subType, const std::string& name)
{
    {
        uint ft = addIfNew(type, MAX_FIELDS, ErrorCode::nFieldsGtMax, setup.fieldTypeNameById, setup.realCapacity.NFieldTypes);
        uint fst = addIfNew(subType, MAX_FIELDS, ErrorCode::nFieldsGtMax, setup.fieldSubTypeNameById, setup.realCapacity.NFieldSubTypes);
        setup.mapping.indexedFieldTypeByFieldId.push_back(ft);
        setup.mapping.indexedFieldSubTypeByFieldId.push_back(fst);
        setup.fieldSubTypeIdBySubTypeName[subType] = fst;
        setup.fieldNameByFieldId.push_back(name);
        setup.fieldTypeNameByFieldId.push_back(type);
        setup.fieldSubTypeNameByFieldId.push_back(subType);
        setup.realCapacity.NFields++;
        limGamesFieldTypeInit();
    }
}


void smfigapl::Input::addTeam(const std::string& name, const std::string& color)
{
    const ClubId clubId = addClubNameIfNew(name);
    const ColorId colorId = addTeamColorIfNew(color);
    addTeam(clubId, colorId);
    setup.teamIdArray.resize(setup.clubNameByClubId.size());
    for (auto&& i : setup.teamIdArray)
        i.resize(setup.colorNameByColorId.size());
    setup.teamIdArray[clubId.value()][colorId.value()] = setup.realCapacity.NTeams - 1;
    conditions.fieldSubTypeDeny.values.resize(setup.realCapacity.NTeams);
    for (auto&& i : conditions.fieldSubTypeDeny.values)
        i.resize(setup.realCapacity.NFieldSubTypes);
}
void smfigapl::Input::addTeam(ClubId cl, ColorId co)
{
    if (cl.value() >= setup.realCapacity.NClubs || co.value() >= setup.realCapacity.NTeamColors)
        throw Exception(ErrorCode::clubIndexOverflow, {std::to_string(cl.value()), std::to_string(co.value())});
    {
        setup.mapping.indexedClubByTeamId.push_back(cl);
        setup.mapping.indexedColorByTeamId.push_back(co);
        setup.realCapacity.NTeams++;
        smfigapl::NamedTeam  team;
        team.setTeamClub(setup.clubNameByClubId[cl.value()]);
        team.setTeamColor(setup.colorNameByColorId[co.value()]);
        setup.namedTeamByTeamId.push_back(team);
        maxGamesSameClubInit();
        limGamesFieldTypeInit();
    }
}

void smfigapl::GamePlanConditions::setMaxPauseGames(uint i)
{
    maxPauseGames.value = i;
    maxPauseGames.enabled = true;
}

void smfigapl::GamePlanConditions::setMaxFieldsEmpty(uint i)
{
    maxFieldsEmpty = i;
}

void smfigapl::GamePlanConditions::setMaxGamesSameClub(uint i)
{
    fill(maxGamesSameClub.values.begin(), maxGamesSameClub.values.end(), i);
    maxGamesSameClub.enabled = true;
}

void smfigapl::GamePlanConditions::setMinGamesFieldType(uint ft, uint i)
{
    limGamesFieldType.values.minRaw[ft] = i;
    limGamesFieldType.values.min = limGamesFieldType.values.minRaw;
    limGamesFieldType.enabled = true;
}

void smfigapl::Input::setMinGamesFieldType(const std::string& ft, uint i)
{
    // the field type id is the index in the list of field type names
    auto p = std::ranges::find(setup.fieldTypeNameById, ft);
    if (p == setup.fieldTypeNameById.end())
        throw Exception(ErrorCode::invalidValue, {"field_types"}, ft);
    conditions.limGamesFieldType.values.minRaw[std::distance(setup.fieldTypeNameById.begin(), p)] = i;
    conditions.limGamesFieldType.values.min = conditions.limGamesFieldType.values.minRaw;
    conditions.limGamesFieldType.enabled = true;
}

void smfigapl::Input::setMaxGamesFieldType(const std::string& ft, uint i)
{
    // the field type id is the index in the list of field type names
    auto p = std::ranges::find(setup.fieldTypeNameById, ft);
    if (p == setup.fieldTypeNameById.end())
        throw Exception(ErrorCode::invalidValue, {"field_types"}, ft);
    conditions.limGamesFieldType.values.maxRaw[std::distance(setup.fieldTypeNameById.begin(), p)] = i;
    conditions.limGamesFieldType.values.max = conditions.limGamesFieldType.values.maxRaw;
    conditions.limGamesFieldType.enabled = true;
}

void smfigapl::Input::maxGamesSameClubInit()
{
    conditions.maxGamesSameClub.values.resize(setup.realCapacity.NTeams);
}

void smfigapl::Input::limGamesFieldTypeInit()
{
    conditions.limGamesFieldType.values.min.resize(setup.realCapacity.NFieldTypes);
    // field types without a limit are not restricted
    conditions.limGamesFieldType.values.max.resize(setup.realCapacity.NFieldTypes, std::numeric_limits<uint>::max());
    conditions.limGamesFieldType.values.minRaw.resize(setup.realCapacity.NFieldTypes);
    conditions.limGamesFieldType.values.maxRaw.resize(setup.realCapacity.NFieldTypes, std::numeric_limits<uint>::max());
}
