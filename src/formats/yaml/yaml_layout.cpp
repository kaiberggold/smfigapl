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


// The output layout (format file) as YAML text

#include <algorithm>
#include <utility>
#include <vector>

#include "smfigapl/formats/yaml.hpp"
#include "yaml_reader.hpp"

namespace
{
    using smfigapl::ProblemCode;
    using smfigapl::formats::yaml::Reader;
    using smfigapl::formats::yaml::Writer;
    using smfigapl::formats::yaml::at;
    using smfigapl::formats::yaml::member;
    namespace layout = smfigapl::layout;

    // ---- reading ----

    // n digits of text from pos, as a number; false if they are not there
    bool digits(std::string_view text, std::size_t pos, std::size_t n, std::uint32_t& value)
    {
        if (pos + n > text.size())
            return false;
        value = 0;
        for (const char c : text.substr(pos, n))
        {
            if (c < '0' || c > '9')
                return false;
            value = value * 10 + static_cast<std::uint32_t>(c - '0');
        }
        return true;
    }

    // HH:MM or HH:MM:SS from pos to the end of text
    bool parseTime(std::string_view text, std::size_t pos, layout::DateTimeValue& v)
    {
        const std::size_t n = text.size() - pos;
        if (n != 5 && n != 8)
            return false;
        if (!digits(text, pos, 2, v.hour) || text[pos + 2] != ':' || !digits(text, pos + 3, 2, v.minute))
            return false;
        if (n == 8 && (text[pos + 5] != ':' || !digits(text, pos + 6, 2, v.second)))
            return false;
        return v.hour < 24 && v.minute < 60 && v.second < 60;
    }

    // A date (YYYY-MM-DD), a time (HH:MM or HH:MM:SS) or both, separated by
    // T or a space
    std::optional<layout::DateTimeValue> parseDateTime(std::string_view text)
    {
        layout::DateTimeValue v;
        if (text.size() < 10 || text[4] != '-')
            return parseTime(text, 0, v) ? std::optional(v) : std::nullopt;
        if (!digits(text, 0, 4, v.year) || !digits(text, 5, 2, v.month) || text[7] != '-' || !digits(text, 8, 2, v.day))
            return std::nullopt;
        if (v.month < 1 || v.month > 12 || v.day < 1 || v.day > 31)
            return std::nullopt;
        if (text.size() == 10)
            return v;
        if ((text[10] != 'T' && text[10] != ' ') || !parseTime(text, 11, v))
            return std::nullopt;
        return v;
    }

    void readPosRel(Reader& r, ryml::ConstNodeRef t, const std::string& path, layout::PosRel& pos)
    {
        pos.xRel = r.signedNumber(t, "pos_rel_x", member(path, "pos_rel_x")).value_or(pos.xRel);
        pos.yRel = r.signedNumber(t, "pos_rel_y", member(path, "pos_rel_y")).value_or(pos.yRel);
    }

    // A map with pos_x and pos_y
    void readPos(Reader& r, ryml::ConstNodeRef parent, std::string_view key, const std::string& parentPath, layout::Pos& pos)
    {
        const std::string path = member(parentPath, key);
        const ryml::ConstNodeRef t = r.map(parent, key, path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"pos_x", "pos_y"}, path);
        pos.x = r.unsignedNumber(t, "pos_x", member(path, "pos_x")).value_or(pos.x);
        pos.y = r.unsignedNumber(t, "pos_y", member(path, "pos_y")).value_or(pos.y);
    }

    // A map with pos_rel_x and pos_rel_y
    void readPosRelTable(Reader& r, ryml::ConstNodeRef parent, std::string_view key, const std::string& parentPath, layout::PosRel& pos)
    {
        const std::string path = member(parentPath, key);
        const ryml::ConstNodeRef t = r.map(parent, key, path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"pos_rel_x", "pos_rel_y"}, path);
        readPosRel(r, t, path, pos);
    }

    // The map title with pos_rel_x, pos_rel_y and text
    void readTitle(Reader& r, ryml::ConstNodeRef parent, const std::string& parentPath, layout::TextField& title)
    {
        const std::string path = member(parentPath, "title");
        const ryml::ConstNodeRef t = r.map(parent, "title", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"pos_rel_x", "pos_rel_y", "text"}, path);
        readPosRel(r, t, path, title);
        title.text = r.string(t, "text", member(path, "text")).value_or(title.text);
    }

    // A list of maps with type, pos_rel_x, pos_rel_y and the text in
    // textKey (if text is set) into entries[type]
    template<class Entry> void readEntries(Reader& r, ryml::ConstNodeRef parent, std::string_view key, const std::string& parentPath,
                                           std::unordered_map<std::string, Entry>& entries, std::string_view textKey, std::string Entry::* text)
    {
        const std::string listPath = member(parentPath, key);
        const ryml::ConstNodeRef list = r.list(parent, key, listPath);
        if (!list.readable())
            return;
        std::size_t i = 0;
        for (ryml::ConstNodeRef t : list.children())
        {
            const std::string path = at(listPath, i++);
            if (!r.isMap(t, "format", path))
                continue;
            if (text != nullptr)
                r.unknownKeys(t, {"type", "pos_rel_x", "pos_rel_y", textKey}, path);
            else
                r.unknownKeys(t, {"type", "pos_rel_x", "pos_rel_y"}, path);
            auto type = r.string(t, "type", member(path, "type"), ProblemCode::missingValue);
            if (!type)
                continue;
            Entry& entry = entries[*type];
            readPosRel(r, t, path, entry);
            if (text != nullptr)
                entry.*text = r.string(t, textKey, member(path, textKey)).value_or(entry.*text);
        }
    }

    void readInformation(Reader& r, ryml::ConstNodeRef root, layout::Information& info)
    {
        const ryml::ConstNodeRef t = r.map(root, "information", "information");
        if (!t.readable())
            return;
        r.unknownKeys(t, {"general", "date_time"}, "information");
        if (const ryml::ConstNodeRef general = r.map(t, "general", "information.general"); general.readable())
            for (ryml::ConstNodeRef value : general.children())
            {
                const std::string key(value.key().str, value.key().len);
                if (auto s = r.stringEntry(value, "information", member("information.general", key)))
                    info.general.values[key] = *s;
            }
        const ryml::ConstNodeRef dateTime = r.map(t, "date_time", "information.date_time");
        if (!dateTime.readable())
            return;
        for (ryml::ConstNodeRef value : dateTime.children())
        {
            const std::string key(value.key().str, value.key().len);
            const auto s = r.stringEntry(value, "information", member("information.date_time", key));
            if (!s)
                continue;
            const auto dt = parseDateTime(*s);
            if (!dt)
            {
                r.add(ProblemCode::invalidValue, {"information", *s}, member("information.date_time", key), value);
                continue;
            }
            info.dateTime.values[key] = *dt;
        }
    }

    void readSettings(Reader& r, ryml::ConstNodeRef format, layout::Format::Settings& settings)
    {
        const std::string path = "format.settings";
        const ryml::ConstNodeRef t = r.map(format, "settings", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"separate_tournaments", "show_teams", "show_teams_text_colors", "teams_use_column", "statistic_use_column", "show_statistic",
                          "show_filter_column", "use_team_color", "fixed_fields", "fixed_rounds", "fixed_tournaments", "filter_column", "filter_column_fixed_rows"
                         }, path);
        auto flag = [&](std::string_view key, bool & value)
        {
            value = r.boolean(t, key, member(path, key)).value_or(value);
        };
        auto number = [&](std::string_view key, std::uint32_t& value)
        {
            value = r.unsignedNumber(t, key, member(path, key)).value_or(value);
        };
        flag("separate_tournaments", settings.separateTournaments);
        flag("show_teams", settings.showTeams);
        flag("show_teams_text_colors", settings.showTeamsTextColors);
        flag("teams_use_column", settings.teamsUseColumn);
        flag("statistic_use_column", settings.statisticUseColumn);
        flag("show_statistic", settings.showStatistic);
        flag("show_filter_column", settings.showFilterColumn);
        flag("use_team_color", settings.useTeamColor);
        number("fixed_fields", settings.fixedFields);
        number("fixed_rounds", settings.fixedRounds);
        number("fixed_tournaments", settings.fixedTournaments);
        number("filter_column", settings.filterColumn);
        const std::string rowsPath = member(path, "filter_column_fixed_rows");
        if (const ryml::ConstNodeRef rows = r.list(t, "filter_column_fixed_rows", rowsPath); rows.readable())
        {
            std::size_t i = 0;
            for (ryml::ConstNodeRef row : rows.children())
                if (auto n = r.unsignedEntry(row, "filter_column_fixed_rows", at(rowsPath, i++)))
                    settings.fixedRows.push_back(*n);
        }
    }

    void readInfoFields(Reader& r, ryml::ConstNodeRef format, layout::Format::InfoFields& info)
    {
        const std::string path = "format.information";
        const ryml::ConstNodeRef t = r.map(format, "information", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"base_pos", "result_message", "text_fields", "date_time", "date_time_header"}, path);
        readPos(r, t, "base_pos", path, info.basePos);
        readPosRelTable(r, t, "result_message", path, info.resultMessage);
        readEntries<layout::PosRel>(r, t, "text_fields", path, info.general, "", nullptr);
        readEntries(r, t, "date_time", path, info.dateTime, "format_string", &layout::DateTimeField::formatString);
        readEntries(r, t, "date_time_header", path, info.dateTimeHeader, "text", &layout::Header::text);
    }

    void readTeams(Reader& r, ryml::ConstNodeRef format, layout::Format::Teams& teams)
    {
        const std::string path = "format.teams";
        const ryml::ConstNodeRef t = r.map(format, "teams", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"base_pos", "title", "text_color"}, path);
        readPos(r, t, "base_pos", path, teams.basePos);
        readTitle(r, t, path, teams.title);
        const std::string colorPath = member(path, "text_color");
        const ryml::ConstNodeRef c = r.map(t, "text_color", colorPath);
        if (!c.readable())
            return;
        r.unknownKeys(c, {"colors", "pos_rel_x", "pos_rel_y", "title"}, colorPath);
        const std::string colorsPath = member(colorPath, "colors");
        if (const ryml::ConstNodeRef colors = r.list(c, "colors", colorsPath); colors.readable())
        {
            std::size_t i = 0;
            for (ryml::ConstNodeRef color : colors.children())
                if (auto s = r.stringEntry(color, "colors", at(colorsPath, i++)))
                    teams.textColors.colors.push_back(*s);
        }
        teams.textColors.posRel.x = r.unsignedNumber(c, "pos_rel_x", member(colorPath, "pos_rel_x")).value_or(teams.textColors.posRel.x);
        teams.textColors.posRel.y = r.unsignedNumber(c, "pos_rel_y", member(colorPath, "pos_rel_y")).value_or(teams.textColors.posRel.y);
        readTitle(r, c, colorPath, teams.textColors.title);
    }

    void readGamePlan(Reader& r, ryml::ConstNodeRef format, layout::Format::GamePlan& gamePlan)
    {
        const std::string path = "format.game_plan";
        const ryml::ConstNodeRef t = r.map(format, "game_plan", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"base_pos", "title"}, path);
        readPos(r, t, "base_pos", path, gamePlan.basePos);
        readTitle(r, t, path, gamePlan.title);
    }

    void readTournament(Reader& r, ryml::ConstNodeRef format, layout::Format::Tournament& tournament)
    {
        const std::string path = "format.tournament";
        const ryml::ConstNodeRef t = r.map(format, "tournament", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"rel_pos", "title", "columns"}, path);
        readPosRelTable(r, t, "rel_pos", path, tournament.relPos);
        readTitle(r, t, path, tournament.title);
        const std::string columnsPath = member(path, "columns");
        const ryml::ConstNodeRef columns = r.map(t, "columns", columnsPath);
        if (!columns.readable())
            return;
        r.unknownKeys(columns, {"rel_pos", "column"}, columnsPath);
        readPosRelTable(r, columns, "rel_pos", columnsPath, tournament.columns.relPos);
        readEntries(r, columns, "column", columnsPath, tournament.columns.col, "text", &layout::Header::text);
    }

    void readStatistic(Reader& r, ryml::ConstNodeRef format, layout::Format::Statistic& statistic)
    {
        const std::string path = "format.statistic";
        const ryml::ConstNodeRef t = r.map(format, "statistic", path);
        if (!t.readable())
            return;
        r.unknownKeys(t, {"base_pos", "title", "rows"}, path);
        readPos(r, t, "base_pos", path, statistic.basePos);
        readTitle(r, t, path, statistic.title);
        const std::string rowsPath = member(path, "rows");
        const ryml::ConstNodeRef rows = r.map(t, "rows", rowsPath);
        if (!rows.readable())
            return;
        r.unknownKeys(rows, {"rel_pos", "row"}, rowsPath);
        readPosRelTable(r, rows, "rel_pos", rowsPath, statistic.rows.relPos);
        readEntries(r, rows, "row", rowsPath, statistic.rows.row, "text", &layout::Header::text);
    }

    // ---- writing ----

    void writePos(Writer& w, ryml::NodeRef parent, std::string_view key, const layout::Pos& p)
    {
        ryml::NodeRef m = w.map(parent, key, true);
        w.number(m, "pos_x", p.x);
        w.number(m, "pos_y", p.y);
    }

    void writePosRel(Writer& w, ryml::NodeRef map, const layout::PosRel& p)
    {
        w.signedNumber(map, "pos_rel_x", p.xRel);
        w.signedNumber(map, "pos_rel_y", p.yRel);
    }

    void writePosRelMap(Writer& w, ryml::NodeRef parent, std::string_view key, const layout::PosRel& p)
    {
        writePosRel(w, w.map(parent, key, true), p);
    }

    void writeTitle(Writer& w, ryml::NodeRef parent, const layout::TextField& t)
    {
        ryml::NodeRef m = w.map(parent, "title", true);
        writePosRel(w, m, t);
        w.string(m, "text", t.text);
    }

    // The keys of a map in order, so that the text does not depend on the hash order
    template<class Map> std::vector<std::string> sortedKeys(const Map& map)
    {
        std::vector<std::string> keys;
        keys.reserve(map.size());
        for (auto&& [key, value] : map)
            keys.push_back(key);
        std::ranges::sort(keys);
        return keys;
    }

    template<class Entry> void writeEntries(Writer& w, ryml::NodeRef parent, std::string_view key, const std::unordered_map<std::string, Entry>& map,
                                            std::string_view textKey, std::string Entry::* text)
    {
        ryml::NodeRef list = w.list(parent, key);
        for (const std::string& type : sortedKeys(map))
        {
            const Entry& entry = map.at(type);
            ryml::NodeRef m = Writer::mapEntry(list, true);
            w.string(m, "type", type);
            writePosRel(w, m, entry);
            if (text != nullptr)
                w.string(m, textKey, entry.*text);
        }
    }

    std::string twoDigits(std::uint32_t n)
    {
        return (n < 10 ? "0" : "") + std::to_string(n);
    }

    // A date, a time or both, depending on which parts are set
    std::string dateTimeText(const layout::DateTimeValue& v)
    {
        std::string date = std::to_string(v.year) + "-" + twoDigits(v.month) + "-" + twoDigits(v.day);
        std::string time = twoDigits(v.hour) + ":" + twoDigits(v.minute) + ":" + twoDigits(v.second);
        const bool hasDate = v.year != 0 || v.month != 0 || v.day != 0;
        const bool hasTime = v.hour != 0 || v.minute != 0 || v.second != 0;
        if (!hasDate)
            return time;
        if (!hasTime)
            return date;
        return date + "T" + time;
    }

    void writeInformation(Writer& w, const layout::Information& info)
    {
        ryml::NodeRef information = w.map(w.root(), "information");
        ryml::NodeRef general = w.map(information, "general");
        for (const std::string& key : sortedKeys(info.general.values))
            w.string(general, key, info.general.values.at(key));
        ryml::NodeRef dateTime = w.map(information, "date_time");
        for (const std::string& key : sortedKeys(info.dateTime.values))
            w.string(dateTime, key, dateTimeText(info.dateTime.values.at(key)));
    }

    void writeSettings(Writer& w, ryml::NodeRef format, const layout::Format::Settings& s)
    {
        ryml::NodeRef m = w.map(format, "settings");
        w.boolean(m, "separate_tournaments", s.separateTournaments);
        w.boolean(m, "show_teams", s.showTeams);
        w.boolean(m, "show_teams_text_colors", s.showTeamsTextColors);
        w.boolean(m, "teams_use_column", s.teamsUseColumn);
        w.boolean(m, "statistic_use_column", s.statisticUseColumn);
        w.boolean(m, "show_statistic", s.showStatistic);
        w.boolean(m, "show_filter_column", s.showFilterColumn);
        w.boolean(m, "use_team_color", s.useTeamColor);
        w.number(m, "fixed_fields", s.fixedFields);
        w.number(m, "fixed_rounds", s.fixedRounds);
        w.number(m, "fixed_tournaments", s.fixedTournaments);
        w.number(m, "filter_column", s.filterColumn);
        ryml::NodeRef rows = w.list(m, "filter_column_fixed_rows", true);
        for (std::uint32_t row : s.fixedRows)
            w.numberEntry(rows, row);
    }

    void writeInfoFields(Writer& w, ryml::NodeRef format, const layout::Format::InfoFields& info)
    {
        ryml::NodeRef m = w.map(format, "information");
        writePos(w, m, "base_pos", info.basePos);
        writePosRelMap(w, m, "result_message", info.resultMessage);
        writeEntries<layout::PosRel>(w, m, "text_fields", info.general, "", nullptr);
        writeEntries(w, m, "date_time", info.dateTime, "format_string", &layout::DateTimeField::formatString);
        writeEntries(w, m, "date_time_header", info.dateTimeHeader, "text", &layout::Header::text);
    }

    void writeTeams(Writer& w, ryml::NodeRef format, const layout::Format::Teams& teams)
    {
        ryml::NodeRef m = w.map(format, "teams");
        writePos(w, m, "base_pos", teams.basePos);
        writeTitle(w, m, teams.title);
        ryml::NodeRef textColor = w.map(m, "text_color");
        ryml::NodeRef colors = w.list(textColor, "colors", true);
        for (const std::string& color : teams.textColors.colors)
            w.stringEntry(colors, color);
        w.number(textColor, "pos_rel_x", teams.textColors.posRel.x);
        w.number(textColor, "pos_rel_y", teams.textColors.posRel.y);
        writeTitle(w, textColor, teams.textColors.title);
    }

    void writeFormat(Writer& w, const layout::Format& f)
    {
        ryml::NodeRef format = w.map(w.root(), "format");
        writeSettings(w, format, f.settings);
        writeInfoFields(w, format, f.infoFields);
        writeTeams(w, format, f.teams);
        ryml::NodeRef gamePlan = w.map(format, "game_plan");
        writePos(w, gamePlan, "base_pos", f.gamePlan.basePos);
        writeTitle(w, gamePlan, f.gamePlan.title);
        ryml::NodeRef tournament = w.map(format, "tournament");
        writePosRelMap(w, tournament, "rel_pos", f.tournament.relPos);
        writeTitle(w, tournament, f.tournament.title);
        ryml::NodeRef columns = w.map(tournament, "columns");
        writePosRelMap(w, columns, "rel_pos", f.tournament.columns.relPos);
        writeEntries(w, columns, "column", f.tournament.columns.col, "text", &layout::Header::text);
        ryml::NodeRef statistic = w.map(format, "statistic");
        writePos(w, statistic, "base_pos", f.statistic.basePos);
        writeTitle(w, statistic, f.statistic.title);
        ryml::NodeRef rows = w.map(statistic, "rows");
        writePosRelMap(w, rows, "rel_pos", f.statistic.rows.relPos);
        writeEntries(w, rows, "row", f.statistic.rows.row, "text", &layout::Header::text);
    }
}

smfigapl::Result<smfigapl::layout::OutputLayout> smfigapl::formats::yaml::readLayout(std::string_view text)
{
    Document document;
    if (auto error = document.parse(text))
        return Result<layout::OutputLayout>::failure(std::move(*error));
    const ryml::ConstNodeRef root = document.root();
    Reader r(document);
    layout::OutputLayout result;
    // the keys of the setup are not checked here
    r.version(root, "layout_version", layoutFormatVersion);
    readInformation(r, root, result.info);
    if (const ryml::ConstNodeRef f = r.map(root, "format", "format"); f.readable())
    {
        r.unknownKeys(f, {"settings", "information", "teams", "game_plan", "tournament", "statistic"}, "format");
        readSettings(r, f, result.format.settings);
        readInfoFields(r, f, result.format.infoFields);
        readTeams(r, f, result.format.teams);
        readGamePlan(r, f, result.format.gamePlan);
        readTournament(r, f, result.format.tournament);
        readStatistic(r, f, result.format.statistic);
    }
    if (r.hasErrors())
        return Result<layout::OutputLayout>::failure(r.takeProblems());
    return {std::move(result), r.takeProblems()};
}

std::string smfigapl::formats::yaml::writeLayout(const layout::OutputLayout& layout)
{
    Writer w;
    w.number(w.root(), "layout_version", layoutFormatVersion);
    writeInformation(w, layout.info);
    writeFormat(w, layout.format);
    return w.text();
}
