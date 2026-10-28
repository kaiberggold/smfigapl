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

#include <utility>
#include <string>
#include <vector>
#include "smfigapl/model/sm_types.hpp"

namespace smfigapl
{

// Game plan based on club and color names
    class NamedTeam
    {
    private:
        std::string teamClub_;
        std::string teamColor_;

    public:
        NamedTeam() = default;

        bool operator==(const NamedTeam& other) const;

        [[nodiscard]] std::string getTeamName() const;

        [[nodiscard]] std::string getTeamColor() const;

        [[nodiscard]] std::string getTeamClub() const;

        void setTeamColor(std::string s);

        void setTeamClub(std::string s);
    };
}//namespace smfigapl
