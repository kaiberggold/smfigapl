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

#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include "smfigapl/model/sm_named.hpp"


//Class NamedTeam

bool smfigapl::NamedTeam::operator==(const NamedTeam& other) const
{
    return other.getTeamClub() == teamClub_ && other.getTeamColor() == teamColor_;
}

std::string smfigapl::NamedTeam::getTeamName() const
{
    return teamClub_ + " " + teamColor_;
}

std::string smfigapl::NamedTeam::getTeamColor() const
{
    return teamColor_;
}

std::string smfigapl::NamedTeam::getTeamClub() const
{
    return teamClub_;
}

void smfigapl::NamedTeam::setTeamColor(std::string s)
{
    teamColor_ = std::move(s);
}

void smfigapl::NamedTeam::setTeamClub(std::string s)
{
    teamClub_ = std::move(s);
}
