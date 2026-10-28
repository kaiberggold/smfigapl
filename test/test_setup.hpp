#pragma once

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "smfigapl/model/sm_setup.hpp"
#include "smfigapl/formats/yaml.hpp"
#include "smfigapl/model/sm_except.hpp"
#include "smfigapl/model/sm_in.hpp"
#include "smfigapl/model/sm_out.hpp"
#include "smfigapl/solver/sm_search.hpp"

// The numbers of rounds and fields: the setup is read, solve() fails
inline bool isCountProblem(smfigapl::ProblemCode code)
{
    using smfigapl::ProblemCode;
    return code == ProblemCode::nRoundsGtMax || code == ProblemCode::nRoundsLt1 || code == ProblemCode::nFieldsGtMax || code == ProblemCode::nFieldsLt1;
}

// Throws the first error of problems as Exception; a syntax error has its
// position as detail ("line L, column C"), an invalid value its value
inline void throwFirstError(const std::vector<smfigapl::Problem>& problems)
{
    for (const auto& p : problems)
    {
        if (p.severity != smfigapl::Severity::error || isCountProblem(p.code))
            continue;
        std::vector<std::string> params = p.parameters;
        std::string detail;
        if (p.code == smfigapl::ProblemCode::invalidValue && params.size() > 1)
        {
            detail = params[1];
            params.resize(1);
        }
        if (p.code == smfigapl::ProblemCode::parseError)
        {
            detail = params.empty() ? "" : params.front();
            if (p.position)
                detail += " (line " + std::to_string(p.position->line) + ", column " + std::to_string(p.position->column) + ")";
            params.clear();
        }
        throw smfigapl::Exception(p.code, params, detail);
    }
}

// Configurations are built from fragments of YAML text: field(), club()
// and texts such as "conditions:\n  max_pauses: 1\n", each starting with the
// keys of its section. joinFragments() drops a key line ("fields:") that
// repeats the last one of its indentation, so that the fragments of a
// section in a row form one map or list.
inline std::string joinFragments(const std::string& text)
{
    std::istringstream in(text);
    std::string result;
    std::vector<std::string> keys; // the last key line of each indentation
    for (std::string line; std::getline(in, line);)
    {
        const std::size_t indent = line.find_first_not_of(' ');
        if (indent == std::string::npos)
            continue;
        const bool keyLine = line.back() == ':' && line[indent] != '-' && line[indent] != '#';
        if (keys.size() <= indent)
            keys.resize(indent + 1);
        if (keyLine && keys[indent] == line)
            continue;
        keys[indent] = keyLine ? line : "";
        keys.resize(indent + 1);
        result += line + "\n";
    }
    return result;
}

// A configuration read from a string (YAML adapter, validation, Input) and
struct TestSetup
{
    smfigapl::Input in;
    smfigapl::layout::OutputLayout presentation;
    smfigapl::Output out{in};
    smfigapl::uint steps = 0;
    smfigapl::ResultArray result;
    // the problems of the numbers of rounds and fields
    std::vector<smfigapl::Problem> countProblems;

    explicit TestSetup(const std::string& config)
    {
        namespace yaml = smfigapl::formats::yaml;
        const std::string text = joinFragments(config);
        auto setup = yaml::readSetup(text);
        throwFirstError(setup.problems());
        const std::vector<smfigapl::Problem> problems = smfigapl::validateSetup(*setup);
        throwFirstError(problems);
        std::ranges::copy_if(problems, std::back_inserter(countProblems), [](const smfigapl::Problem & p)
        {
            return isCountProblem(p.code);
        });
        smfigapl::buildInput(*setup, in);
        auto layout = yaml::readLayout(text);
        throwFirstError(layout.problems());
        presentation = *layout;
    }

    // true if the setup can be searched
    [[nodiscard]] bool valid() const
    {
        return countProblems.empty();
    }

    // Runs the search, true if a game plan was found
    bool solve(smfigapl::uint maxSteps = 1000000)
    {
        if (!valid())
            return false;
        smfigapl::GamePlanSearch search{in, out};
        search.setMaxSteps(maxSteps);
        result = search.gpSearch();
        bool solved = result.getResult();
        if (solved)
            out.setSolved();
        steps = search.getSteps();
        return solved;
    }
};

// The error a configuration causes while reading, ErrorCode::none if there is none
inline smfigapl::ErrorCode configError(const std::string& config)
{
    try
    {
        TestSetup t(config);
    }
    catch (const smfigapl::Exception& e)
    {
        return e.code();
    }
    return smfigapl::ErrorCode::none;
}

// Building blocks for configurations (see joinFragments)
inline std::string field(const std::string& type, const std::string& subtype, const std::string& name)
{
    return "fields:\n  - {type: \"" + type + "\", subtype: \"" + subtype + "\", name: \"" + name + "\"}\n";
}

inline std::string club(const std::string& name, const std::vector<std::string>& colors)
{
    std::string s = "clubs:\n  - name: \"" + name + "\"\n    teams: [";
    for (size_t i = 0; i < colors.size(); i++)
        s += ((i != 0u) ? ", \"" : "\"") + colors[i] + "\"";
    return s + "]\n";
}
