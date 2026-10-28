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
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include "smfigapl/model/sm_types.hpp"
#include "smfigapl/model/sm_error_codes.hpp"
#include "smfigapl/model/sm_debug.hpp"
#include "smfigapl/model/sm_capacity.hpp"
#include "smfigapl/model/sm_named.hpp"

namespace smfigapl
{

    class GamePlanSetup
    {

    private:
    public:
        smfigapl::Capacity realCapacity;
        smfigapl::Mapping mapping;

        std::vector<std::vector<uint>> teamIdArray;

        std::map<std::string, ColorId> colorIdByColor;
        std::vector<std::string> fieldTypeNameByFieldId;
        std::vector<std::string> clubNameByClubId;
        std::map<std::string, ClubId> clubIdByClubName;
        std::vector<std::string> colorNameByColorId;
        std::vector<std::string> fieldSubTypeNameByFieldId;
        std::vector<std::string> fieldNameByFieldId;
        std::vector<std::string> fieldSubTypeNameById;
        std::vector<std::string> fieldTypeNameById;
        std::vector<smfigapl::NamedTeam> namedTeamByTeamId;
        std::map<std::string, uint> fieldSubTypeIdBySubTypeName;

    };

    class GamePlanConditions
    {
    public:
        uint iterations = 1;
        bool repeatPairings = false;
        bool homeAwayBalance = true;
        bool optimizeRounds = true;
        struct
        {
            bool enabled = false;
            struct
            {
                std::vector<uint> minRaw;
                std::vector<uint> min;
                std::vector<uint> maxRaw;
                std::vector<uint> max;
            } values;
        } limGamesFieldType;
        // off by default like the other conditions (it used to be on with 0,
        // so without [conditions.pause_games] no team could pause)
        struct
        {
            bool enabled = false;
            uint value = 0;
        } maxPauseGames;
        uint extraPauseFields = 0;
        struct
        {
            bool enabled = false;
            std::vector<uint> values ;
        } maxGamesSameClub;
        uint maxFieldsEmpty = 0;
        struct
        {
            bool enabled = false;
            std::vector<std::vector<uint>> values ;
        } fieldSubTypeDeny;

        void setMaxPauseGames(uint i);
        void setMinGamesFieldType(uint ft, uint i);

        void setMaxGamesSameClub(uint i);
        void setMaxFieldsEmpty(uint i);
    };

    class Input
    {
    private:

    public:
        GamePlanSetup setup;
        GamePlanConditions conditions;

        DebugLog dbg;

        void setNRounds(uint n);
        ClubId addClubNameIfNew(const std::string& s);
        ColorId addTeamColorIfNew(const std::string& s);
        static uint addIfNew(const std::string& s, uint max, ErrorCode valId, std::vector<std::string>& array, uint& counter);

        void addTeam(ClubId cl, ColorId co);
        void addTeam(const std::string& name, const std::string& color);
        void addField(const std::string& type, const std::string& subType, const std::string& name);
        void maxGamesSameClubInit();
        void limGamesFieldTypeInit();
        void setMinGamesFieldType(const std::string& ft, uint i);
        void setMaxGamesFieldType(const std::string& ft, uint i);
    };
} //namespace smGamePlan
