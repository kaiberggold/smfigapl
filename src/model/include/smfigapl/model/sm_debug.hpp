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

// The debug output of the search on a LogSink: the call sites write to a
// stream (as they always did), whole lines go to the sink as messages of
// level debug. Without a sink nothing is enabled, so nothing is formatted.

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "smfigapl/log.hpp"
#include "smfigapl/model/sm_types.hpp"

namespace smfigapl
{
    // The debug categories of the command line option -d (bits)
    enum class DebugFlag : uint
    {
        ts = 1,          // tree search
        gps = 2,         // game plan search
        cond = 4,        // conditions
        file = 8,        // configuration file
        stepsDetails = 16 // search steps in detail
    };

    // The log category of a debug flag
    LogCategory logCategory(DebugFlag flag);

    class DebugFlags
    {
    private:
        uint bits_ = 0;
    public:
        DebugFlags() = default;
        explicit DebugFlags(uint bits): bits_{bits} {}
        [[nodiscard]] bool has(DebugFlag f) const
        {
            return (bits_ & static_cast<uint>(f)) != 0;
        }
        // true for the category of a set flag, and for general
        [[nodiscard]] bool has(LogCategory category) const;
    };

    // A LogSink that writes the debug messages of the flags (and of the
    // general category) to a stream, one line each. Not thread safe.
    class StreamLogSink : public LogSink
    {
    private:
        DebugFlags flags_;
        std::ostream& out_;
    public:
        StreamLogSink(DebugFlags flags, std::ostream& out): flags_{flags}, out_{out} {}
        [[nodiscard]] bool enabled(LogLevel level, LogCategory category) const override;
        void write(LogLevel level, LogCategory category, std::string_view message) override;
    };

    class DebugLog
    {
    private:
        class Lines;
        LogSink* sink_ = nullptr;
        // shared by the copies (the tree search has one), so the lines stay in order
        mutable std::shared_ptr<Lines> lines_;
    public:
        DebugLog() = default;
        // The sink must outlive the log and its copies
        explicit DebugLog(LogSink* sink);
        // true if the sink wants the messages of the flag; ask before formatting
        [[nodiscard]] bool has(DebugFlag f) const
        {
            return sink_ != nullptr && sink_->enabled(LogLevel::debug, logCategory(f));
        }
        // The stream for messages of the flag; a line goes to the sink at its
        // end, with the category of its first part. Discards without a sink.
        [[nodiscard]] std::ostream& out(DebugFlag f) const;
        // The stream for messages of the category general
        [[nodiscard]] std::ostream& out() const;
    };
}//namespace smfigapl
