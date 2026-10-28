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

#include <cstdint>
#include <ostream>

namespace smfigapl
{

// Basic types
    using uint = std::uint32_t;

// An index that cannot be mixed up with other indices: it is only created
// explicitly from a number, and value() gives the number for indexing.
    template<class Tag>
    class StrongId
    {
    private:
        uint value_ = 0;
    public:
        constexpr StrongId() = default;
        constexpr explicit StrongId(uint value): value_{value} {}
        [[nodiscard]] constexpr uint value() const
        {
            return value_;
        }
        constexpr bool operator==(const StrongId&) const = default;
        friend constexpr bool operator<(StrongId a, StrongId b)
        {
            return a.value_ < b.value_;
        }
        friend std::ostream& operator<<(std::ostream& os, StrongId id)
        {
            return os << id.value_;
        }
    };
    using ClubId = StrongId<struct ClubTag>;
    using ColorId = StrongId<struct ColorTag>;

// Limits of the configuration
    inline constexpr uint MAX_CLUBS = 16;
    inline constexpr uint MAX_ROUNDS = 10;
    inline constexpr uint MAX_FIELDS = 10;
    inline constexpr int MAX_TEAMCOLORS = 16;
    inline constexpr uint MAX_FIELDTYPES = 4;
    // Possible rounds of one tournament (atomic set) the search accepts: the
    // tables need about 1.4 KB per round (8 fields, 16 teams: 2 million rounds)
    // The candidates of the search are computed from the pairings of the teams
    // and the field mixes: the pairings are stored, the candidates only if
    // every one has to be checked (subtypes, more virtual teams than empty
    // fields allowed). 8 fields: 2 million pairings, 9 fields: 34 million.
    inline constexpr double MAX_SEARCH_PAIRINGS = 2.1e6;
    inline constexpr double MAX_SEARCH_ROUNDS = 2e9;
    inline constexpr double MAX_SEARCH_ROUNDS_CHECKED = 3e7;

}//namespace smfigapl
