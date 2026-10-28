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


/// @file result.hpp
/// @brief Problems and results: the error model of the API.
///
/// Problems of the input, format errors and the reasons why no game plan
/// exists are returned as data (Problem); no exception crosses the API.
/// The codes are stable numbers and part of the semantic versioning; the
/// text of a message is not part of the API: a client translates the code
/// and its parameters.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace smfigapl
{
    /// The code of a problem. The numbers do not change (they are also the
    /// exit codes of the command line tool).
    enum class ProblemCode : std::uint32_t
    {
        none = 0,                   ///< no problem
        parseError = 1,             ///< syntax error of a text (or a duplicate key); parameters: description
        nRoundsNotSet = 10,         ///< the number of rounds is missing
        noFieldsDefined = 11,       ///< no fields; parameters: key
        noClubsDefined = 12,        ///< no clubs; parameters: key
        missingValue = 13,          ///< a required value is missing; parameters: key
        invalidValue = 14,          ///< a value has the wrong type or is unknown; parameters: section, the value (if known)
        unsupportedFormatVersion = 15, ///< warning: a text of a newer format version; parameters: version, supported version
        unknownKey = 16,            ///< warning: a key the reader does not know (ignored); parameters: key
        clubIndexGtMax = 101,       ///< too many clubs; parameters: maximum
        clubIndexOverflow = 102,    ///< internal: unknown club or color; parameters: club, color
        teamColorIndexGtMax = 103,  ///< too many team colors; parameters: maximum
        nFieldsGtMax = 105,         ///< too many fields or field types; parameters: maximum
        nFieldsLt1 = 106,           ///< no field
        nRoundsGtMax = 107,         ///< too many rounds; parameters: maximum
        nRoundsLt1 = 108,           ///< no round
        searchTooLarge = 109,       ///< too many possible rounds for the search; parameters: rounds, maximum
        searchTooManyPairings = 110, ///< too many pairings of the teams; parameters: pairings, maximum
        maxLtMin = 200,             ///< a minimum above its maximum; parameters: minimum, maximum
        // reasons why no game plan exists (the result of a search, not an
        // input error)
        impossibleFieldTypeMin = 300,     ///< parameters: field type, places, needed
        impossibleFieldTypeMax = 301,     ///< parameters: field type, places to fill, allowed
        impossibleTeamFieldTypeMin = 302, ///< parameters: team, games needed, games possible
        impossibleTeamGames = 303,        ///< parameters: team, games to play, games allowed
        impossiblePauses = 304,           ///< parameters: pauses needed, pauses allowed
        impossibleSameClub = 305,         ///< parameters: team, games to play, games possible
        impossibleOpponents = 306,        ///< parameters: team, games to play, opponents left
        impossibleSearch = 307,           ///< the search tried all possibilities
        impossibleGames = 308,            ///< parameters: places to fill, games possible
        internal = 900                    ///< an error inside the library; parameters: description
    };

    /// How serious a problem is.
    enum class Severity : std::uint8_t
    {
        error,   ///< the input cannot be used
        warning  ///< the input is used, but something is probably not meant this way
    };

    /// A position in a text (1-based), for problems of a stored format.
    struct TextPosition
    {
        std::uint32_t line = 0;   ///< The line, starting at 1.
        std::uint32_t column = 0; ///< The column, starting at 1.
        friend bool operator==(const TextPosition&, const TextPosition&) = default;
    };

    /// A problem: its code, the values for its message, where it is.
    struct Problem
    {
        /// What the problem is.
        ProblemCode code = ProblemCode::none;
        /// The values of the message, in the order of the code's description.
        std::vector<std::string> parameters;
        /// Where in the setup, e.g. "fields[2].type"; empty if it concerns the whole setup.
        std::string path;
        /// An error or a warning.
        Severity severity = Severity::error;
        /// Where in a text, if the problem was found while reading one.
        std::optional<TextPosition> position;
        friend bool operator==(const Problem&, const Problem&) = default;
    };

    /// A value, or the problems why there is none. Similar to C++23
    /// std::expected, with one addition: a result with a value can carry
    /// warnings.
    template<class T>
    class Result
    {
    private:
        std::optional<T> value_;
        std::vector<Problem> problems_;

        [[noreturn]] static void noValue()
        {
            // a precondition of the caller is broken; there is no exception at the API
            std::abort();
        }

    public:
        /// A result with a value (and possibly warnings).
        Result(T value, std::vector<Problem> warnings = {}): value_{std::move(value)}, problems_{std::move(warnings)}
        {
        }
        /// A result without a value; problems has at least one error.
        static Result failure(std::vector<Problem> problems)
        {
            Result r;
            r.problems_ = std::move(problems);
            return r;
        }
        /// A result without a value, from one problem.
        static Result failure(Problem problem)
        {
            return failure(std::vector<Problem> {std::move(problem)});
        }

        /// True if there is a value.
        [[nodiscard]] bool has_value() const noexcept
        {
            return value_.has_value();
        }
        /// True if there is a value.
        explicit operator bool() const noexcept
        {
            return has_value();
        }
        /// The value; only if has_value() (the program ends otherwise).
        [[nodiscard]] const T& value() const&
        {
            if (!value_)
                noValue();
            return *value_;
        }
        /// The value; only if has_value() (the program ends otherwise).
        [[nodiscard]] T& value()&
        {
            if (!value_)
                noValue();
            return *value_;
        }
        /// The value, moved out; only if has_value() (the program ends otherwise).
        [[nodiscard]] T&& value()&&
        {
            if (!value_)
                noValue();
            return std::move(*value_);
        }
        /// The value; only if has_value().
        [[nodiscard]] const T& operator*() const&
        {
            return value();
        }
        /// The members of the value; only if has_value().
        [[nodiscard]] const T* operator->() const
        {
            return &value();
        }
        /// All problems: the errors without a value, the warnings with one.
        [[nodiscard]] const std::vector<Problem>& problems() const noexcept
        {
            return problems_;
        }

    private:
        Result() = default;
    };
}
