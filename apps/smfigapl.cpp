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

#define NOMINMAX
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <optional>
#include <span>
#include <iostream>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string_view>

#include "cxxopts.hpp"
#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/log.hpp"
#include "smfigapl/planner.hpp"
#include "smfigapl/report/csv.hpp"
#include "smfigapl/report/text.hpp"
#include "smfigapl/version.hpp"
#include "cli/texts.hpp"

// Without arguments: read the configuration from stdin instead of printing the help
static constexpr bool readStdInWithoutArguments = false;

// Default spacing of the progress output (option -z)
static const std::uint32_t NSTEPSDISPLAY = 1000;

namespace
{
    // Settings from the command line
    struct Options
    {
        std::uint32_t maxSteps = 10000000;
        // seconds, 0: no time limit
        double timeLimit = 0;
        std::string fileNameIn;
        std::string cfgFileNameIn;
        std::string fileNameOut;
        std::string lang = smfigapl::cli::defaultLanguage;
        std::uint32_t stepSize = NSTEPSDISPLAY;
        bool steps = false;
        bool debug = false;
        std::uint32_t dbg = 0;
        bool calcStat = false;
        bool enableStopWatch = false;
        bool enableFileWrite = false;
        bool textFile = false;
        bool stdInStream = false;
        bool separateFormatFile = false;
    };

    // Prints the setting if debugging is enabled
    void debugInfo(const Options& opt, const std::string& text)
    {
        if (opt.debug)
            std::cout << text << std::endl;
    }

    // Reads the command line into opt. Returns the exit code if the program
    // ends here (help, version, wrong options).
    std::optional<int> parseOptions(std::span<const char*> args, Options& opt)
    {
        try
        {
            cxxopts::Options options(args.front(), "Small Field Game Planer.\nGame Plan algorithm for special cases");
            options
            .add_options()
            ("h,help", "Print usage")
            ("v,version", "Print version")
            ("d,debug", "Debug", cxxopts::value<bool>())
            ("e,debugoptions", "Debug Options", cxxopts::value<std::uint32_t>())
            ("w,stopwatch", "Enable Stopwatch", cxxopts::value<bool>())
            ("m,maxsteps", "Maximum search steps", cxxopts::value<std::uint32_t>(), "N")
            ("timelimit", "Maximum search time in seconds", cxxopts::value<double>(), "SECONDS")
            ("p,progress", "Print Progress", cxxopts::value<bool>())
            ("s,statistic", "Print Statistic (For text output)", cxxopts::value<bool>())
            ("t,text", "Simple Text Output", cxxopts::value<bool>())
            ("z,stepsize", "Steps printing spacing", cxxopts::value<std::uint32_t>(), "N")
            ("i,input", "Input File", cxxopts::value<std::string>(), "FILE")
            ("f,format", "Format File", cxxopts::value<std::string>(), "FILE")
            ("l,language", "Language (EN,DE)", cxxopts::value<std::string>()->default_value(smfigapl::cli::defaultLanguage), "LANGUAGE")
            ("o,output", "Output File", cxxopts::value<std::string>(), "FILE");
            if (!readStdInWithoutArguments && args.size() == 1)
            {
                std::cout << options.help({"", "Group"}) << std::endl;
                return 0;
            }
            auto params = options.parse(static_cast<int>(args.size()), args.data());
            if (params.contains("d"))
            {
                opt.debug = true;
                std::cout << "Debugging enabled" << std::endl;
                if (params.contains("e"))
                {
                    opt.dbg = params["e"].as<std::uint32_t>();
                    std::cout << "Debug state" << opt.dbg << std::endl;
                }
            }
            if (params.contains("help"))
            {
                std::cout << options.help({"", "Group"}) << std::endl;
                return 0;
            }
            if (params.contains("version"))
            {
                std::cout << "Smfigapl Version: " << smfigapl::VERSION_STRING << std::endl;
                return 0;
            }

            if (params.contains("m"))
            {
                opt.maxSteps = params["m"].as<std::uint32_t>();
                debugInfo(opt, "Maxsteps: " + std::to_string(opt.maxSteps));
            }
            if (params.contains("timelimit"))
                opt.timeLimit = params["timelimit"].as<double>();
            if (params.contains("p"))
            {
                opt.steps = true;
                debugInfo(opt, "Display Progress enabled");
            }
            if (params.contains("w"))
            {
                opt.enableStopWatch = true;
                debugInfo(opt, "Stopwatch enabled");
            }
            if (params.contains("z"))
            {
                opt.stepSize = params["z"].as<std::uint32_t>();
                debugInfo(opt, "Step size: " + std::to_string(opt.stepSize));
            }
            if (params.contains("s"))
            {
                opt.calcStat = true;
                debugInfo(opt, "Statistic enabled");
            }
            if (params.contains("t"))
            {
                opt.textFile = true;
                debugInfo(opt, "Textfile enabled");
            }
            if (params.contains("language"))
            {
                opt.lang = params["l"].as<std::string>();
                debugInfo(opt, "Language: " + opt.lang);
            }
            if (params.contains("o"))
            {
                opt.enableFileWrite = true;
                opt.fileNameOut = params["o"].as<std::string>();
                debugInfo(opt, "fileNameOut: " + opt.fileNameOut);
            }
            if (params.contains("i"))
            {
                opt.fileNameIn = params["i"].as<std::string>();
                debugInfo(opt, "fileNameIn: " + opt.fileNameIn);
            }
            else
                opt.stdInStream = true;
            if (params.contains("f"))
            {
                opt.cfgFileNameIn = params["f"].as<std::string>();
                debugInfo(opt, "CfgfileNameIn: " + opt.cfgFileNameIn);
                opt.separateFormatFile = true;
            }
        }
        catch (const cxxopts::exceptions::exception& e)
        {
            std::cerr << "error parsing options: " << e.what() << std::endl;
            return 1;
        }
        return std::nullopt;
    }

    // The debug output to the console: the categories of option -e and the
    // general messages, only with option -d
    class ConsoleLog : public smfigapl::LogSink
    {
    private:
        bool debug_;
        std::uint32_t flags_;
    public:
        ConsoleLog(bool debug, std::uint32_t flags): debug_{debug}, flags_{flags} {}
        [[nodiscard]] bool enabled(smfigapl::LogLevel /*level*/, smfigapl::LogCategory category) const override
        {
            using smfigapl::LogCategory;
            if (!debug_)
                return false;
            // the bits of option -e
            switch (category)
            {
                case LogCategory::general:
                    return true;
                case LogCategory::treeSearch:
                    return (flags_ & 1U) != 0;
                case LogCategory::search:
                    return (flags_ & 2U) != 0;
                case LogCategory::conditions:
                    return (flags_ & 4U) != 0;
                case LogCategory::file:
                    return (flags_ & 8U) != 0;
                case LogCategory::searchSteps:
                    return (flags_ & 16U) != 0;
            }
            return false;
        }
        void write(smfigapl::LogLevel /*level*/, smfigapl::LogCategory /*category*/, std::string_view message) override
        {
            std::cout << message << '\n';
        }
    };

    // An error of the input: its message, its code is the exit code
    class InputError
    {
    public:
        std::vector<smfigapl::Problem> problems;
        std::string source;
    };

    // Prints the problems (errors and warnings); throws InputError if there is an error
    void check(const std::vector<smfigapl::Problem>& problems, const std::string& source)
    {
        if (std::ranges::any_of(problems, [](const smfigapl::Problem & p)
    {
        return p.severity == smfigapl::Severity::error;
    }))
        throw InputError{problems, source};
    }

    void printProblems(const std::vector<smfigapl::Problem>& problems, const smfigapl::cli::Lang& language, const std::string& source)
    {
        for (const auto& p : problems)
            std::cerr << smfigapl::cli::problemMessage(p, language, source) << std::endl;
    }

    // The text of a file or of stdin
    std::string inputText(const std::string& fileName)
    {
        if (fileName.empty())
        {
            std::string s;
            for (std::string line; std::getline(std::cin, line);)
                s += line + "\n";
            return s;
        }
        std::ifstream file(fileName, std::ios::binary);
        if (!file)
        {
            smfigapl::Problem p;
            p.code = smfigapl::ProblemCode::parseError;
            p.parameters = {"File could not be opened for reading\n\t(error occurred at '" + fileName + "')"};
            throw InputError{{p}, ""};
        }
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    // The setup and the output layout from the files of the options
    struct Input
    {
        smfigapl::TournamentSetup setup;
        smfigapl::layout::OutputLayout layout;
    };

    Input readInput(const Options& opt, const smfigapl::cli::Lang& language)
    {
        namespace yaml = smfigapl::formats::yaml;
        const std::string text = inputText(opt.stdInStream ? "" : opt.fileNameIn);
        // debug option file: the configuration as read
        if (opt.debug && (opt.dbg & 8U) != 0)
            std::cout << text << "\n";
        auto setup = yaml::readSetup(text);
        check(setup.problems(), opt.fileNameIn);
        printProblems(setup.problems(), language, opt.fileNameIn);
        // information and format: from the format file if there is one
        const std::string layoutSource = opt.separateFormatFile ? opt.cfgFileNameIn : opt.fileNameIn;
        auto layout = yaml::readLayout(opt.separateFormatFile ? inputText(opt.cfgFileNameIn) : text);
        check(layout.problems(), layoutSource);
        printProblems(layout.problems(), language, layoutSource);
        Input in{std::move(setup).value(), std::move(layout).value()};
        // a text color for every team: filled up with the colors from the start, or clipped
        const std::size_t nTeams = std::accumulate(in.setup.clubs.begin(), in.setup.clubs.end(), std::size_t{0}, [](std::size_t n, const smfigapl::Club & c)
        {
            return n + c.teams.size();
        });
        auto& colors = in.layout.format.teams.textColors.colors;
        if (!colors.empty())
        {
            const std::size_t size = colors.size();
            colors.resize(nTeams);
            for (std::size_t i = size; i < colors.size(); i++)
                colors[i] = colors[i % size];
        }
        return in;
    }

    // Writes the game plan (and the statistic) as text or for the spreadsheet
    void writeGamePlan(const Options& opt, const Input& in, const smfigapl::SolveResult& result, bool showStatistic,
                       const smfigapl::report::Labels& labels, std::ostream& os)
    {
        namespace report = smfigapl::report;
        if (opt.textFile)
        {
            // the sub tournaments apart if pairings are repeated
            report::writeText(os, result.plan, labels, in.setup.repeatPairings);
            if (opt.calcStat)
                report::writeStatisticText(os, smfigapl::computeStatistic(in.setup, result), labels);
            return;
        }
        smfigapl::Statistic statistic;
        if (showStatistic)
            statistic = smfigapl::computeStatistic(in.setup, result);
        report::writeCsv(os, in.setup, result, showStatistic ? &statistic : nullptr, in.layout, labels);
    }

    // Reads the configuration, searches the game plan and writes it. Returns the exit code.
    int run(const Options& opt, smfigapl::cli::Lang& language)
    {
        using namespace smfigapl;
        language.setLanguage(opt.lang);
        const report::Labels labels = cli::labels(language);
        ConsoleLog log(opt.debug, opt.dbg);
        const Input in = readInput(opt, language);
        check(validate(in.setup), opt.fileNameIn);

        SolveOptions options;
        options.maxSteps = opt.maxSteps;
        if (opt.timeLimit > 0)
            options.timeLimit = std::chrono::milliseconds(static_cast<long long>(opt.timeLimit * 1000));
        if (opt.steps)
        {
            options.progressInterval = opt.stepSize;
            options.progress = [](const Progress & p)
            {
                std::cout << "step: " << p.steps << std::endl;
            };
        }
        options.log = &log;
        const Result<SolveResult> solved = solve(in.setup, options);
        check(solved.problems(), opt.fileNameIn);
        const SolveResult& result = *solved;
        if (opt.enableStopWatch)
        {
            std::cout << std::endl << "Time: " << result.info.duration.count() << " ms" << std::endl;
            std::cout << "Steps: " << std::accumulate(result.info.steps.begin(), result.info.steps.end(), std::uint64_t{0}) << std::endl;
        }
        if (result.status == SearchStatus::solved)
        {
            // the game plan goes to the output file (-o) or to the console
            std::ofstream file;
            if (opt.enableFileWrite)
                file.open(opt.fileNameOut);
            std::ostream& os = opt.enableFileWrite ? static_cast<std::ostream&>(file) : std::cout;
            writeGamePlan(opt, in, result, in.layout.format.settings.showStatistic, labels, os);
        }
        else
        {
            // no game plan exists (with the reasons), or the search stopped at a limit
            std::cerr << (result.status == SearchStatus::impossible ? labels.impossible : labels.aborted) << std::endl;
            for (const auto& reason : result.reasons)
                std::cerr << cli::problemText(reason, language) << std::endl;
            // an aborted search shows where it stopped (without statistic)
            if (result.status == SearchStatus::aborted)
                writeGamePlan(opt, in, result, false, labels, std::cout);
        }
        return 0;
    }
}

int main(int argc, const char* argv[])
{
    // for the error messages, set to the chosen language
    smfigapl::cli::Lang language;
    try
    {
        Options opt;
        if (auto exitCode = parseOptions(std::span(argv, static_cast<size_t>(argc)), opt))
            return *exitCode;
        return run(opt, language);
    }
    catch (const InputError& e)
    {
        // errors of the input: translated messages, the code of the first error is the exit code
        printProblems(e.problems, language, e.source);
        const auto first = std::ranges::find_if(e.problems, [](const smfigapl::Problem & p)
        {
            return p.severity == smfigapl::Severity::error;
        });
        return static_cast<int>(first->code);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown Error\n";
        return -1;
    }
}
