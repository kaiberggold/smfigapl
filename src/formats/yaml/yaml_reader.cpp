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


#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <utility>

#include <ryml_std.hpp>

#include "yaml_reader.hpp"

namespace
{
    using smfigapl::TextPosition;

    // An error of rapidyaml; thrown by the callbacks (they must not return)
    // and caught in Document::parse
    struct Failure
    {
        std::string message;
        std::optional<TextPosition> position;
    };

    std::optional<TextPosition> textPosition(const ryml::Location& loc, std::size_t offset)
    {
        if (loc.line == ryml::npos || loc.col == ryml::npos)
            return std::nullopt;
        return TextPosition{static_cast<std::uint32_t>(loc.line + offset), static_cast<std::uint32_t>(loc.col + offset)};
    }

    std::string text(ryml::csubstr s)
    {
        return {s.str, s.len};
    }

    ryml::csubstr view(std::string_view s)
    {
        return {s.data(), s.size()};
    }

    [[noreturn]] void basicError(ryml::csubstr msg, const ryml::ErrorDataBasic& /*data*/, void* /*user*/)
    {
        throw Failure{text(msg), std::nullopt};
    }

    // the positions of parse errors start at 1
    [[noreturn]] void parseError(ryml::csubstr msg, const ryml::ErrorDataParse& data, void* /*user*/)
    {
        throw Failure{text(msg), textPosition(data.ymlloc, 0)};
    }

    [[noreturn]] void visitError(ryml::csubstr msg, const ryml::ErrorDataVisit& /*data*/, void* /*user*/)
    {
        throw Failure{text(msg), std::nullopt};
    }

    ryml::Callbacks callbacks()
    {
        ryml::Callbacks c;
        c.set_error_basic(basicError).set_error_parse(parseError).set_error_visit(visitError);
        return c;
    }

    smfigapl::Problem syntaxProblem(std::string description, std::optional<TextPosition> position)
    {
        return {smfigapl::ProblemCode::parseError, {std::move(description)}, "", smfigapl::Severity::error, position};
    }

    // The first key that appears twice in a map of node or below
    ryml::ConstNodeRef duplicateKey(ryml::ConstNodeRef node)
    {
        if (node.is_map())
            for (ryml::ConstNodeRef child : node.children())
                for (ryml::ConstNodeRef earlier : node.children())
                {
                    if (earlier.id() == child.id())
                        break;
                    if (earlier.key() == child.key())
                        return child;
                }
        if (node.is_map() || node.is_seq())
            for (ryml::ConstNodeRef child : node.children())
                if (ryml::ConstNodeRef d = duplicateKey(child); d.readable())
                    return d;
        return {};
    }

    bool isScalar(ryml::ConstNodeRef node)
    {
        return node.readable() && node.has_val() && !node.val_is_null();
    }

    // A plain (not quoted) scalar, e.g. a number or true
    bool isPlainScalar(ryml::ConstNodeRef node)
    {
        return isScalar(node) && !node.is_val_quoted();
    }

    // Whether other readers of YAML could take a string for something else:
    // null, a boolean (also those of YAML 1.1), a number, a date or a time
    bool needsQuotes(std::string_view s)
    {
        if (ryml::scalar_is_null(view(s)))
            return true;
        std::string lower(s);
        std::ranges::transform(lower, lower.begin(), [](char c)
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        });
        for (std::string_view word : {"true", "false", "yes", "no", "on", "off", "y", "n", ".inf", "-.inf", "+.inf", ".nan"})
            if (lower == word)
                return true;
        auto isDigit = [](char c)
        {
            return c >= '0' && c <= '9';
        };
        return !s.empty() && (isDigit(s[0]) || (s.size() > 1 && (s[0] == '-' || s[0] == '+' || s[0] == '.') && isDigit(s[1])));
    }

    template<class Number> std::optional<Number> number(ryml::ConstNodeRef node)
    {
        if (!isPlainScalar(node))
            return std::nullopt;
        const ryml::csubstr v = node.val();
        Number n{};
        const auto [end, ec] = std::from_chars(v.begin(), v.end(), n);
        if (ec != std::errc{} || end != v.end())
            return std::nullopt;
        return n;
    }
}

smfigapl::formats::yaml::Document::Document()
    : handler_(callbacks()), parser_(&handler_, ryml::ParserOptions().locations(true)), tree_(callbacks())
{
}

std::optional<smfigapl::Problem> smfigapl::formats::yaml::Document::parse(std::string_view text)
{
    try
    {
        ryml::parse_in_arena(&parser_, view(text), &tree_);
        tree_.resolve();
        const ryml::ConstNodeRef r = tree_.crootref();
        if (r.is_stream())
            return syntaxProblem("more than one document", std::nullopt);
        if (!r.is_map())
        {
            if (r.has_val() || r.is_seq())
                return syntaxProblem("not a map of keys", position(r));
            // nothing but comments
            tree_.rootref().set_map();
        }
        if (const ryml::ConstNodeRef d = duplicateKey(r); d.readable())
            return syntaxProblem("duplicate key " + ::text(d.key()), position(d));
    }
    catch (const Failure& f)
    {
        return syntaxProblem(f.message, f.position);
    }
    return std::nullopt;
}

ryml::ConstNodeRef smfigapl::formats::yaml::Document::root() const
{
    return tree_.crootref();
}

std::optional<smfigapl::TextPosition> smfigapl::formats::yaml::Document::position(ryml::ConstNodeRef node) const
{
    if (!node.readable())
        return std::nullopt;
    try
    {
        // the positions of nodes start at 0
        return textPosition(node.location(parser_), 1);
    }
    catch (const Failure&)
    {
        return std::nullopt;
    }
}

smfigapl::formats::yaml::Reader::Reader(const Document& document)
    : document_(document)
{
}

void smfigapl::formats::yaml::Reader::add(ProblemCode code, std::vector<std::string> parameters, std::string path, ryml::ConstNodeRef where, Severity severity)
{
    problems_.push_back(Problem{code, std::move(parameters), std::move(path), severity, document_.position(where)});
}

ryml::ConstNodeRef smfigapl::formats::yaml::Reader::find(ryml::ConstNodeRef map, std::string_view key)
{
    if (!map.readable() || !map.is_map())
        return {};
    for (ryml::ConstNodeRef child : map.children())
        if (child.key() == view(key))
            return child;
    return {};
}

std::optional<std::uint32_t> smfigapl::formats::yaml::Reader::unsignedNumber(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable())
    {
        if (missing != ProblemCode::none)
            add(missing, {std::string(key)}, path, map);
        return std::nullopt;
    }
    return unsignedEntry(node, key, path);
}

std::optional<int> smfigapl::formats::yaml::Reader::signedNumber(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable())
    {
        if (missing != ProblemCode::none)
            add(missing, {std::string(key)}, path, map);
        return std::nullopt;
    }
    const auto v = number<int>(node);
    if (!v)
        add(ProblemCode::invalidValue, {std::string(key)}, path, node);
    return v;
}

std::optional<std::string> smfigapl::formats::yaml::Reader::string(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable())
    {
        if (missing != ProblemCode::none)
            add(missing, {std::string(key)}, path, map);
        return std::nullopt;
    }
    return stringEntry(node, key, path);
}

std::optional<bool> smfigapl::formats::yaml::Reader::boolean(ryml::ConstNodeRef map, std::string_view key, const std::string& path)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable())
        return std::nullopt;
    if (isPlainScalar(node))
    {
        // the spellings of the YAML core schema
        const ryml::csubstr v = node.val();
        if (v == "true" || v == "True" || v == "TRUE")
            return true;
        if (v == "false" || v == "False" || v == "FALSE")
            return false;
    }
    add(ProblemCode::invalidValue, {std::string(key)}, path, node);
    return std::nullopt;
}

ryml::ConstNodeRef smfigapl::formats::yaml::Reader::map(ryml::ConstNodeRef map, std::string_view key, const std::string& path)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable() || !isMap(node, key, path))
        return {};
    return node;
}

ryml::ConstNodeRef smfigapl::formats::yaml::Reader::list(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing)
{
    const ryml::ConstNodeRef node = find(map, key);
    if (!node.readable())
    {
        if (missing != ProblemCode::none)
            add(missing, {std::string(key)}, path, map);
        return {};
    }
    if (node.is_seq())
        return node;
    add(ProblemCode::invalidValue, {std::string(key)}, path, node);
    return {};
}

bool smfigapl::formats::yaml::Reader::isMap(ryml::ConstNodeRef node, std::string_view section, const std::string& path)
{
    if (node.readable() && node.is_map())
        return true;
    add(ProblemCode::invalidValue, {std::string(section)}, path, node);
    return false;
}

std::optional<std::string> smfigapl::formats::yaml::Reader::stringEntry(ryml::ConstNodeRef node, std::string_view section, const std::string& path)
{
    if (isScalar(node))
        return text(node.val());
    add(ProblemCode::invalidValue, {std::string(section)}, path, node);
    return std::nullopt;
}

std::optional<std::uint32_t> smfigapl::formats::yaml::Reader::unsignedEntry(ryml::ConstNodeRef node, std::string_view section, const std::string& path)
{
    const auto v = number<std::uint64_t>(node);
    if (!v || *v > std::numeric_limits<std::uint32_t>::max())
    {
        add(ProblemCode::invalidValue, {std::string(section)}, path, node);
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(*v);
}

void smfigapl::formats::yaml::Reader::unknownKeys(ryml::ConstNodeRef map, std::initializer_list<std::string_view> known, const std::string& path, std::initializer_list<std::string_view> ignored)
{
    for (ryml::ConstNodeRef child : map.children())
    {
        const std::string key = text(child.key());
        auto isKey = [&key](std::string_view k)
        {
            return key == k;
        };
        if (std::ranges::none_of(known, isKey) && std::ranges::none_of(ignored, isKey))
            add(ProblemCode::unknownKey, {key}, member(path, key), child, Severity::warning);
    }
}

void smfigapl::formats::yaml::Reader::version(ryml::ConstNodeRef map, std::string_view key, std::uint32_t supported)
{
    if (auto v = unsignedNumber(map, key, std::string(key)); v && *v > supported)
        add(ProblemCode::unsupportedFormatVersion, {std::to_string(*v), std::to_string(supported)}, std::string(key), find(map, key), Severity::warning);
}

bool smfigapl::formats::yaml::Reader::hasErrors() const
{
    return std::ranges::any_of(problems_, [](const Problem & p)
    {
        return p.severity == Severity::error;
    });
}

std::vector<smfigapl::Problem> smfigapl::formats::yaml::Reader::takeProblems()
{
    return std::move(problems_);
}

smfigapl::formats::yaml::Writer::Writer()
    : tree_(callbacks())
{
    tree_.rootref().set_map(ryml::BLOCK);
}

ryml::NodeRef smfigapl::formats::yaml::Writer::root()
{
    return tree_.rootref();
}

ryml::NodeRef smfigapl::formats::yaml::Writer::map(ryml::NodeRef parent, std::string_view key, bool flow)
{
    ryml::NodeRef node = parent.append_child();
    node.set_key(tree_.to_arena(view(key)));
    node.set_map(flow ? ryml::FLOW_SL : ryml::BLOCK);
    return node;
}

ryml::NodeRef smfigapl::formats::yaml::Writer::list(ryml::NodeRef parent, std::string_view key, bool flow)
{
    ryml::NodeRef node = parent.append_child();
    node.set_key(tree_.to_arena(view(key)));
    node.set_seq(flow ? ryml::FLOW_SL : ryml::BLOCK);
    return node;
}

ryml::NodeRef smfigapl::formats::yaml::Writer::mapEntry(ryml::NodeRef list, bool flow)
{
    ryml::NodeRef node = list.append_child();
    node.set_map(flow ? ryml::FLOW_SL : ryml::BLOCK);
    return node;
}

void smfigapl::formats::yaml::Writer::scalar(ryml::NodeRef node, std::string_view value, bool text)
{
    node.set_val(tree_.to_arena(view(value)));
    if (text && needsQuotes(value))
        node.set_val_style(ryml::VAL_SQUO);
}

void smfigapl::formats::yaml::Writer::string(ryml::NodeRef map, std::string_view key, std::string_view value)
{
    ryml::NodeRef node = map.append_child();
    node.set_key(tree_.to_arena(view(key)));
    scalar(node, value, true);
}

void smfigapl::formats::yaml::Writer::number(ryml::NodeRef map, std::string_view key, std::uint32_t value)
{
    ryml::NodeRef node = map.append_child();
    node.set_key(tree_.to_arena(view(key)));
    scalar(node, std::to_string(value), false);
}

void smfigapl::formats::yaml::Writer::signedNumber(ryml::NodeRef map, std::string_view key, int value)
{
    ryml::NodeRef node = map.append_child();
    node.set_key(tree_.to_arena(view(key)));
    scalar(node, std::to_string(value), false);
}

void smfigapl::formats::yaml::Writer::boolean(ryml::NodeRef map, std::string_view key, bool value)
{
    ryml::NodeRef node = map.append_child();
    node.set_key(tree_.to_arena(view(key)));
    scalar(node, value ? "true" : "false", false);
}

void smfigapl::formats::yaml::Writer::stringEntry(ryml::NodeRef list, std::string_view value)
{
    scalar(list.append_child(), value, true);
}

void smfigapl::formats::yaml::Writer::numberEntry(ryml::NodeRef list, std::uint32_t value)
{
    scalar(list.append_child(), std::to_string(value), false);
}

std::string smfigapl::formats::yaml::Writer::text() const
{
    return ryml::emitrs_yaml<std::string>(tree_, ryml::EmitOptions().force_flow_spc(true));
}

std::string smfigapl::formats::yaml::at(const std::string& path, std::size_t index)
{
    return path + "[" + std::to_string(index) + "]";
}

std::string smfigapl::formats::yaml::member(const std::string& path, std::string_view key)
{
    return path.empty() ? std::string(key) : path + "." + std::string(key);
}
