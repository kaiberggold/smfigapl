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

// Several searches at once, as the app runs them on worker threads: every
// search has its own state, the library has no global state (run under
// ThreadSanitizer in CI)

#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"

#include "test_setup.hpp"
#include "test_misc.hpp"

using namespace smfigapl;

namespace
{
    // the plan (teams of every field in every round) and the steps of a search
    struct Outcome
    {
        bool solved = false;
        uint steps = 0;
        std::vector<std::tuple<uint, uint, uint, uint>> games;
        bool operator==(const Outcome&) const = default;
    };

    Outcome search(const std::string& config)
    {
        TestSetup t(config);
        Outcome o;
        o.solved = t.solve();
        o.steps = t.steps;
        const Capacity& c = t.in.setup.realCapacity;
        for (uint r = 0; r < c.NRounds; r++)
            for (uint f = 0; f < c.NFields; f++)
            {
                IndexedGame g = t.out.indexedGamePlan.getGame(r, f);
                o.games.emplace_back(r, f, g.getTeamByGameId(0).getTeamId(), g.getTeamByGameId(1).getTeamId());
            }
        return o;
    }

    std::vector<std::string> configs()
    {
        const std::string clubs8 = club("North", {"1", "2", "3", "4"}) + club("South", {"1", "2", "3", "4"});
        const std::string clubs7 = club("North", {"1", "2", "3", "4"}) + club("South", {"1", "2", "3"});
        const std::string twoTypes = field("A", "x", "1") + field("B", "x", "2") + field("A", "x", "3") + field("B", "x", "4");
        const std::string pauses = "conditions:\n  max_pauses: 1\nconditions:\n  max_empty_fields: 1\n";
        const std::string tight = "conditions:\n  field_types:\n    - {type: \"A\", min: 3, max: 4}\nconditions:\n  field_types:\n    - {type: \"B\", min: 3, max: 4}\n";
        return {"rounds: 7\n" + twoTypes + clubs8, "rounds: 7\n" + twoTypes + clubs7 + pauses,
                "rounds: 7\n" + twoTypes + clubs8 + tight, "rounds: 5\n" + twoTypes + clubs7 + pauses + "conditions:\n  max_same_club_games: 1\n"};
    }
}

TEST(threads, parallelSearches)
{
    const std::vector<std::string> all = configs();
    std::vector<Outcome> serial;
    serial.reserve(all.size());
    for (const std::string& config : all)
        serial.push_back(search(config));
    // every configuration twice at the same time
    std::vector<Outcome> parallel(2 * all.size());
    std::vector<std::thread> threads;
    threads.reserve(parallel.size());
    for (size_t i = 0; i < parallel.size(); i++)
        threads.emplace_back([&parallel, &all, i]
    {
        parallel[i] = search(all[i % all.size()]);
    });
    for (std::thread& thread : threads)
        thread.join();
    for (size_t i = 0; i < parallel.size(); i++)
    {
        EXPECT_EQ(parallel[i], serial[i % all.size()]) << "configuration " << i % all.size();
        EXPECT_GT(parallel[i].steps, 0u);
    }
}
