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


/// @file formats/yaml.hpp
/// @brief The YAML adapter: the setup and the output layout as YAML text.
///
/// The adapter turns text into the public types and back; nothing else in
/// the library knows YAML. Another format would be another adapter with
/// the same functions.
///
/// The keys are the canonical ones (English, lower case, words joined by
/// `_`), as the writers write them and the schemas in *docs/schema*
/// describe them. Reading is tolerant where it can be:
/// - a key the reader does not know is ignored with a warning
///   (ProblemCode::unknownKey);
/// - a text of a newer version is read as far as possible, with a warning
///   (ProblemCode::unsupportedFormatVersion).
///
/// Syntax errors, duplicate keys and values of the wrong type are errors.
/// Every problem has the position in the text if there is one. The
/// problems of the setup have the path in TournamentSetup (e.g.
/// `fields[2].type`), those of the layout the path of the YAML keys (e.g.
/// `format.teams.title`).
///
/// The reader checks the format only. Whether a setup can be planned
/// (numbers of rounds and fields, names used in the conditions) is checked
/// by the validation of the library.
///
/// The setup and the layout can be in one text (keys of the other part are
/// ignored without a warning) or in two.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "smfigapl/layout.hpp"
#include "smfigapl/result.hpp"
#include "smfigapl/setup.hpp"

namespace smfigapl::formats::yaml
{
    /// The version of the setup format that writeSetup() writes (key `format_version`).
    inline constexpr std::uint32_t setupFormatVersion = 1;
    /// The version of the layout format that writeLayout() writes (key `layout_version`).
    inline constexpr std::uint32_t layoutFormatVersion = 1;

    /// Reads a setup. Without a value if there is an error; warnings can
    /// accompany a value.
    [[nodiscard]] Result<TournamentSetup> readSetup(std::string_view text);

    /// Writes a setup with the canonical keys and `format_version`.
    /// readSetup() of the text gives the same setup.
    [[nodiscard]] std::string writeSetup(const TournamentSetup& setup);

    /// Reads an output layout (the maps `information` and `format`).
    /// Missing keys keep the defaults of layout::OutputLayout.
    [[nodiscard]] Result<layout::OutputLayout> readLayout(std::string_view text);

    /// Writes an output layout with the canonical keys and
    /// `layout_version`. readLayout() of the text gives the same layout.
    [[nodiscard]] std::string writeLayout(const layout::OutputLayout& layout);
}
