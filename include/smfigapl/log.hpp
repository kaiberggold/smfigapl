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


/// @file log.hpp
/// @brief The logging interface: the host decides where the messages go.
///
/// The library writes its log messages to a LogSink that the caller passes
/// in; without a sink nothing is logged and nothing is formatted. The
/// command line tool logs to the console, an app to its platform's log.

#pragma once

#include <cstdint>
#include <string_view>

namespace smfigapl
{
    /// How important a message is.
    enum class LogLevel : std::uint8_t
    {
        error,   ///< something failed
        warning, ///< something is probably not meant this way
        info,    ///< what happens
        debug    ///< details for finding errors
    };

    /// What a message is about (the debug categories of the search).
    enum class LogCategory : std::uint8_t
    {
        general,     ///< everything else
        treeSearch,  ///< the candidate rounds
        search,      ///< the search of the game plan
        conditions,  ///< the checks of the conditions
        file,        ///< reading a configuration
        searchSteps  ///< every step of the search
    };

    /// Where the log messages go. Called from the thread of the search: an
    /// implementation used by several searches at once has to be thread safe.
    class LogSink
    {
    public:
        /// @cond
        LogSink() = default;
        LogSink(const LogSink&) = default;
        LogSink& operator=(const LogSink&) = default;
        LogSink(LogSink&&) = default;
        LogSink& operator=(LogSink&&) = default;
        /// @endcond
        virtual ~LogSink() = default;
        /// True if messages of this level and category are wanted; asked
        /// before a message is formatted.
        [[nodiscard]] virtual bool enabled(LogLevel level, LogCategory category) const = 0;
        /// One message (without a line break at the end).
        virtual void write(LogLevel level, LogCategory category, std::string_view message) = 0;
    };
}
