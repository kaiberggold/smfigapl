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


// solve() on several threads at once, as the app runs it on workers:
// the same results as one after another, cancellation of some searches
// while others go on (run under ThreadSanitizer in CI)

#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>
#include "gtest/gtest.h"

#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/planner.hpp"

using namespace smfigapl;

namespace
{
    std::vector<TournamentSetup> setups()
    {
        std::vector<TournamentSetup> all;
        for (std::uint32_t teams = 5; teams <= 8; teams++)
        {
            TournamentSetup s;
            s.rounds = 4;
            s.fields = {{"1", "A", "x"}, {"2", "B", "x"}, {"3", "A", "x"}};
            s.clubs = {{"North", {}}, {"South", {}}};
            for (std::uint32_t t = 0; t < teams; t++)
                s.clubs[t % 2].teams.push_back(std::to_string(t));
            s.conditions.maxEmptyFields = 1;
            all.push_back(s);
        }
        return all;
    }

    TournamentSetup longSearch()
    {
        std::ifstream file(std::filesystem::path(SM_GOLDEN_DIR) / "configs" / "gen_6f_12t_2ft_3-4_games_per_type.yaml", std::ios::binary);
        const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        auto setup = formats::yaml::readSetup(text);
        return setup.has_value() ? *setup : TournamentSetup{};
    }

    // The result without the time it took
    SolveResult withoutDuration(const Result<SolveResult>& r)
    {
        EXPECT_TRUE(r.has_value());
        SolveResult s = r.has_value() ? *r : SolveResult{};
        s.info.duration = {};
        return s;
    }
}

TEST(plannerThreads, parallelSolve)
{
    const std::vector<TournamentSetup> all = setups();
    std::vector<SolveResult> sequential;
    sequential.reserve(all.size());
    for (const TournamentSetup& s : all)
        sequential.push_back(withoutDuration(solve(s)));

    // every setup twice at once
    std::vector<SolveResult> parallel(2 * all.size());
    {
        std::vector<std::jthread> threads;
        threads.reserve(parallel.size());
        for (std::size_t i = 0; i < parallel.size(); i++)
            threads.emplace_back([&all, &parallel, i]
        {
            parallel[i] = withoutDuration(solve(all[i % all.size()]));
        });
    }
    for (std::size_t i = 0; i < parallel.size(); i++)
    {
        EXPECT_EQ(parallel[i].status, SearchStatus::solved);
        EXPECT_EQ(parallel[i], sequential[i % all.size()]) << i;
    }
}

// Long searches cancelled from the main thread while short ones finish
TEST(plannerThreads, cancelSome)
{
    const TournamentSetup hard = longSearch();
    ASSERT_FALSE(hard.fields.empty());
    const std::vector<TournamentSetup> easy = setups();
    constexpr std::size_t nHard = 3;
    std::vector<SolveResult> hardResults(nHard);
    std::vector<SolveResult> easyResults(easy.size());
    std::atomic<std::size_t> started{0};
    {
        std::vector<std::jthread> hardThreads;
        hardThreads.reserve(nHard);
        for (std::size_t i = 0; i < nHard; i++)
            hardThreads.emplace_back([&hard, &hardResults, &started, i](std::stop_token stop)
        {
            SolveOptions options;
            options.stopToken = std::move(stop);
            options.progressInterval = 100;
            bool counted = false;
            options.progress = [&started, &counted](const Progress & p)
            {
                if (!counted && p.steps >= 100)
                {
                    started++;
                    counted = true;
                }
            };
            hardResults[i] = withoutDuration(solve(hard, options));
        });
        {
            std::vector<std::jthread> easyThreads;
            easyThreads.reserve(easy.size());
            for (std::size_t i = 0; i < easy.size(); i++)
                easyThreads.emplace_back([&easy, &easyResults, i]
            {
                easyResults[i] = withoutDuration(solve(easy[i]));
            });
        }
        // the easy searches are done; stop the hard ones once they all search
        while (started < nHard)
            std::this_thread::yield();
        for (auto& t : hardThreads)
            t.request_stop();
    }
    for (const SolveResult& r : hardResults)
    {
        EXPECT_EQ(r.status, SearchStatus::cancelled);
        ASSERT_EQ(r.info.steps.size(), 1u);
        EXPECT_GE(r.info.steps.front(), 100u);
    }
    for (std::size_t i = 0; i < easy.size(); i++)
        EXPECT_EQ(easyResults[i], withoutDuration(solve(easy[i]))) << i;
}
