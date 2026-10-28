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

#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include "gtest/gtest.h"

#include <fstream>
#include <iterator>

#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_out.hpp"
#include "smfigapl/planner.hpp"
#include "smfigapl/report/text.hpp"
#include "smfigapl/solver/sm_search.hpp"
#include "test_misc.hpp"
#include "test_setup.hpp"

using namespace smfigapl;

// The solver reports progress through the callback and writes debug output
// only to the sink of the DebugLog; the writers write to the stream they
// get. None of them writes directly to the console.
class smTestSearchOutput : public ::testing::Test
{
public:
    std::string text;
    std::unique_ptr<TestSetup> setup;
    std::ostringstream debugOut;
    std::unique_ptr<StreamLogSink> sink;

    void load(DebugFlags flags)
    {
        std::ifstream file(testDataDir + "/generic/0001_3f_7t_2c_2ft.yaml", std::ios::binary);
        text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        setup = std::make_unique<TestSetup>(text);
        ASSERT_TRUE(setup->valid());
        sink = std::make_unique<StreamLogSink>(flags, debugOut);
        setup->in.dbg = DebugLog{sink.get()};
    }
};

TEST_F(smTestSearchOutput, progressCallback)
{
    load(DebugFlags{});
    GamePlanSearch gps{setup->in, setup->out};
    std::vector<SearchProgress> reports;
    const uint stepSize = 10;
    gps.setProgressCallback(stepSize, [&reports](const SearchProgress & p) { reports.push_back(p); });

    testing::internal::CaptureStdout();
    ResultArray result = gps.gpSearch();
    std::string console = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result.getResult());
    EXPECT_EQ(console, "");
    ASSERT_FALSE(reports.empty());
    // every stepSize steps while searching, then once when finished
    for (size_t i = 0; i + 1 < reports.size(); i++)
    {
        EXPECT_FALSE(reports[i].finished);
        EXPECT_EQ(reports[i].step % stepSize, 0u);
    }
    EXPECT_TRUE(reports.back().finished);
    EXPECT_EQ(reports.back().step, gps.getSteps());
}

TEST_F(smTestSearchOutput, debugOutputGoesToTheLog)
{
    // all debug flags
    load(DebugFlags{31});
    GamePlanSearch gps{setup->in, setup->out};

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    ResultArray result = gps.gpSearch();
    std::string console = testing::internal::GetCapturedStdout();
    std::string consoleErr = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(result.getResult());
    EXPECT_EQ(console, "");
    EXPECT_EQ(consoleErr, "");
    EXPECT_NE(debugOut.str().find("Sl: "), std::string::npos);
}

TEST_F(smTestSearchOutput, writersWriteToTheGivenStream)
{
    load(DebugFlags{});
    const auto config = formats::yaml::readSetup(text);
    ASSERT_TRUE(config.has_value());
    testing::internal::CaptureStdout();
    const auto result = solve(*config);
    ASSERT_TRUE(result.has_value());
    std::ostringstream plan;
    report::writeText(plan, result->plan, report::Labels{});
    report::writeStatisticText(plan, computeStatistic(*config, *result), report::Labels{});
    std::string console = testing::internal::GetCapturedStdout();

    EXPECT_EQ(console, "");
    for (uint i = 1; i <= config->rounds; i++)
        EXPECT_NE(plan.str().find("Round: " + std::to_string(i) + "\n"), std::string::npos) << plan.str();
}
