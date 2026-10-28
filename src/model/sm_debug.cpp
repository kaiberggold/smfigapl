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

#include <streambuf>
#include <string>
#include <utility>

#include "smfigapl/model/sm_debug.hpp"

namespace
{
    // Collects the characters of a line, gives the line to the sink at '\n'
    class LineBuffer : public std::streambuf
    {
    private:
        smfigapl::LogSink* sink_;
        std::string line_;
        smfigapl::LogCategory category_ = smfigapl::LogCategory::general;

        void endLine()
        {
            if (sink_ != nullptr)
                sink_->write(smfigapl::LogLevel::debug, category_, line_);
            line_.clear();
        }

    protected:
        int_type overflow(int_type c) override
        {
            if (traits_type::eq_int_type(c, traits_type::eof()))
                return traits_type::not_eof(c);
            if (traits_type::to_char_type(c) == '\n')
                endLine();
            else
                line_ += traits_type::to_char_type(c);
            return c;
        }

    public:
        explicit LineBuffer(smfigapl::LogSink* sink): sink_{sink} {}
        LineBuffer(const LineBuffer&) = delete;
        LineBuffer& operator=(const LineBuffer&) = delete;
        LineBuffer(LineBuffer&&) = delete;
        LineBuffer& operator=(LineBuffer&&) = delete;
        // a last line without '\n'
        ~LineBuffer() override
        {
            if (!line_.empty())
                endLine();
        }
        // the category of a new line
        void category(smfigapl::LogCategory c)
        {
            if (line_.empty())
                category_ = c;
        }
    };
}

class smfigapl::DebugLog::Lines
{
public:
    LineBuffer buffer;
    std::ostream stream;
    explicit Lines(LogSink* sink): buffer{sink}, stream{&buffer} {}
};

smfigapl::LogCategory smfigapl::logCategory(DebugFlag flag)
{
    switch (flag)
    {
        case DebugFlag::ts:
            return LogCategory::treeSearch;
        case DebugFlag::gps:
            return LogCategory::search;
        case DebugFlag::cond:
            return LogCategory::conditions;
        case DebugFlag::file:
            return LogCategory::file;
        case DebugFlag::stepsDetails:
            return LogCategory::searchSteps;
    }
    return LogCategory::general;
}

bool smfigapl::DebugFlags::has(LogCategory category) const
{
    for (DebugFlag f : {DebugFlag::ts, DebugFlag::gps, DebugFlag::cond, DebugFlag::file, DebugFlag::stepsDetails})
        if (logCategory(f) == category)
            return has(f);
    return true;
}

bool smfigapl::StreamLogSink::enabled(LogLevel level, LogCategory category) const
{
    return level != LogLevel::debug || flags_.has(category);
}

void smfigapl::StreamLogSink::write(LogLevel /*level*/, LogCategory /*category*/, std::string_view message)
{
    out_ << message << '\n';
}

smfigapl::DebugLog::DebugLog(LogSink* sink): sink_{sink}, lines_{std::make_shared<Lines>(sink)}
{
}

std::ostream& smfigapl::DebugLog::out(DebugFlag f) const
{
    std::ostream& s = out();
    lines_->buffer.category(logCategory(f));
    return s;
}

std::ostream& smfigapl::DebugLog::out() const
{
    // without a sink: a stream of its own that discards (a log is used by one search only)
    if (!lines_)
        lines_ = std::make_shared<Lines>(nullptr);
    lines_->buffer.category(LogCategory::general);
    return lines_->stream;
}
