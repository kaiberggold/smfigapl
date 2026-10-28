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


// Reading and writing YAML texts for the YAML adapter (rapidyaml): the
// parsed text with the positions of its nodes, typed values with problems
// and paths, and a tree for writing. Private to the adapter.

#pragma once

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <ryml.hpp>

#include "smfigapl/result.hpp"

namespace smfigapl::formats::yaml
{
    // A parsed text: the tree and the parser, which knows the positions of
    // the nodes. The errors of rapidyaml go to callbacks of this object
    // (never to the global ones), so that several texts can be read at once.
    class Document
    {
    private:
        ryml::EventHandlerTree handler_;
        ryml::Parser parser_;
        ryml::Tree tree_;

    public:
        Document();
        Document(const Document&) = delete;
        Document& operator=(const Document&) = delete;
        Document(Document&&) = delete;
        Document& operator=(Document&&) = delete;
        ~Document() = default;

        // Parses text. A syntax error, a duplicate key, several documents
        // or a text that is not a map of keys become the problem parseError.
        // An empty text is an empty map.
        [[nodiscard]] std::optional<Problem> parse(std::string_view text);
        // The map of the text
        [[nodiscard]] ryml::ConstNodeRef root() const;
        // The position of a node (of its key if it has one), if it is known
        [[nodiscard]] std::optional<TextPosition> position(ryml::ConstNodeRef node) const;
    };

    class Reader
    {
    private:
        const Document& document_;
        std::vector<Problem> problems_;

    public:
        explicit Reader(const Document& document);

        void add(ProblemCode code, std::vector<std::string> parameters, std::string path, ryml::ConstNodeRef where, Severity severity = Severity::error);

        // The value of key in a map; an invalid node if there is none
        [[nodiscard]] static ryml::ConstNodeRef find(ryml::ConstNodeRef map, std::string_view key);

        // Typed values of key. Without a value if the key is missing (the
        // problem missing if it is not none) or has the wrong type
        // (invalidValue with the key).
        std::optional<std::uint32_t> unsignedNumber(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing = ProblemCode::none);
        std::optional<int> signedNumber(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing = ProblemCode::none);
        std::optional<std::string> string(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing = ProblemCode::none);
        std::optional<bool> boolean(ryml::ConstNodeRef map, std::string_view key, const std::string& path);
        // A map or a list; an invalid node if it is missing or of another type
        ryml::ConstNodeRef map(ryml::ConstNodeRef map, std::string_view key, const std::string& path);
        ryml::ConstNodeRef list(ryml::ConstNodeRef map, std::string_view key, const std::string& path, ProblemCode missing = ProblemCode::none);

        // Elements of lists (and values of maps); invalidValue with section
        // for another type
        bool isMap(ryml::ConstNodeRef node, std::string_view section, const std::string& path);
        std::optional<std::string> stringEntry(ryml::ConstNodeRef node, std::string_view section, const std::string& path);
        std::optional<std::uint32_t> unsignedEntry(ryml::ConstNodeRef node, std::string_view section, const std::string& path);

        // A warning for every key of map that is none of known; the keys in
        // ignored are accepted without a warning
        void unknownKeys(ryml::ConstNodeRef map, std::initializer_list<std::string_view> known, const std::string& path, std::initializer_list<std::string_view> ignored = {});

        // A warning if the version of the text is newer than supported
        void version(ryml::ConstNodeRef map, std::string_view key, std::uint32_t supported);

        [[nodiscard]] bool hasErrors() const;
        [[nodiscard]] std::vector<Problem> takeProblems();
    };

    // Builds a text: maps keep the order in which their keys are added.
    // Strings are quoted where YAML needs it and where other readers of
    // YAML could take them for a number, a boolean, a date or null.
    class Writer
    {
    private:
        ryml::Tree tree_;

        // text: a string (quoted if needed), else a number or a boolean
        void scalar(ryml::NodeRef node, std::string_view value, bool text);

    public:
        Writer();

        [[nodiscard]] ryml::NodeRef root();
        // A map or a list under key of a map; flow: on one line
        ryml::NodeRef map(ryml::NodeRef parent, std::string_view key, bool flow = false);
        ryml::NodeRef list(ryml::NodeRef parent, std::string_view key, bool flow = false);
        // A map as the next element of a list
        static ryml::NodeRef mapEntry(ryml::NodeRef list, bool flow = false);
        void string(ryml::NodeRef map, std::string_view key, std::string_view value);
        void number(ryml::NodeRef map, std::string_view key, std::uint32_t value);
        void signedNumber(ryml::NodeRef map, std::string_view key, int value);
        void boolean(ryml::NodeRef map, std::string_view key, bool value);
        // Elements of a list
        void stringEntry(ryml::NodeRef list, std::string_view value);
        void numberEntry(ryml::NodeRef list, std::uint32_t value);

        [[nodiscard]] std::string text() const;
    };

    // Path of an element: path[index]
    std::string at(const std::string& path, std::size_t index);
    // Path of a key in a map: path.key (key alone at the top)
    std::string member(const std::string& path, std::string_view key);
}
