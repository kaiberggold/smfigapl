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


// The setup as YAML text. The keys are the names of TournamentSetup in
// lower case with "_" (the paths of the problems use the names themselves).

#include <utility>

#include "smfigapl/formats/yaml.hpp"
#include "yaml_reader.hpp"

namespace
{
    using smfigapl::ProblemCode;
    using smfigapl::formats::yaml::Reader;
    using smfigapl::formats::yaml::Writer;
    using smfigapl::formats::yaml::at;

    // Keys of the layout, read by readLayout
    constexpr std::initializer_list<std::string_view> layoutKeys = {"information", "format", "layout_version"};

    void readFields(Reader& r, ryml::ConstNodeRef root, smfigapl::TournamentSetup& setup)
    {
        const ryml::ConstNodeRef fields = r.list(root, "fields", "fields", ProblemCode::noFieldsDefined);
        if (!fields.readable())
            return;
        std::size_t i = 0;
        for (ryml::ConstNodeRef f : fields.children())
        {
            const std::string path = at("fields", i++);
            if (!r.isMap(f, "fields", path))
                continue;
            r.unknownKeys(f, {"name", "type", "subtype"}, path);
            smfigapl::Field& field = setup.fields.emplace_back();
            field.name = r.string(f, "name", path + ".name", ProblemCode::missingValue).value_or("");
            field.type = r.string(f, "type", path + ".type", ProblemCode::missingValue).value_or("");
            field.subtype = r.string(f, "subtype", path + ".subtype", ProblemCode::missingValue).value_or("");
        }
    }

    void readClubs(Reader& r, ryml::ConstNodeRef root, smfigapl::TournamentSetup& setup)
    {
        const ryml::ConstNodeRef clubs = r.list(root, "clubs", "clubs", ProblemCode::noClubsDefined);
        if (!clubs.readable())
            return;
        std::size_t i = 0;
        for (ryml::ConstNodeRef c : clubs.children())
        {
            const std::string path = at("clubs", i++);
            if (!r.isMap(c, "clubs", path))
                continue;
            r.unknownKeys(c, {"name", "teams"}, path);
            smfigapl::Club& club = setup.clubs.emplace_back();
            club.name = r.string(c, "name", path + ".name", ProblemCode::missingValue).value_or("");
            const ryml::ConstNodeRef teams = r.list(c, "teams", path + ".teams", ProblemCode::missingValue);
            if (!teams.readable())
                continue;
            std::size_t j = 0;
            for (ryml::ConstNodeRef t : teams.children())
                if (auto team = r.stringEntry(t, "teams", at(path + ".teams", j++)))
                    club.teams.push_back(*team);
        }
    }

    void readFieldTypes(Reader& r, ryml::ConstNodeRef conditions, smfigapl::Conditions& setup)
    {
        const ryml::ConstNodeRef fieldTypes = r.list(conditions, "field_types", "conditions.fieldTypes");
        if (!fieldTypes.readable())
            return;
        std::size_t i = 0;
        for (ryml::ConstNodeRef f : fieldTypes.children())
        {
            const std::string path = at("conditions.fieldTypes", i++);
            if (!r.isMap(f, "field_types", path))
                continue;
            r.unknownKeys(f, {"type", "min", "max"}, path);
            smfigapl::FieldTypeLimit& limit = setup.fieldTypes.emplace_back();
            limit.type = r.string(f, "type", path + ".type", ProblemCode::missingValue).value_or("");
            limit.min = r.unsignedNumber(f, "min", path + ".min");
            limit.max = r.unsignedNumber(f, "max", path + ".max");
        }
    }

    void readDenyTeams(Reader& r, ryml::ConstNodeRef deny, const std::string& path, smfigapl::SubtypeDeny& setup)
    {
        const ryml::ConstNodeRef teams = r.list(deny, "teams", path + ".teams", ProblemCode::missingValue);
        if (!teams.readable())
            return;
        std::size_t j = 0;
        for (ryml::ConstNodeRef t : teams.children())
        {
            const std::string teamPath = at(path + ".teams", j++);
            if (!r.isMap(t, "subtype_deny", teamPath))
                continue;
            r.unknownKeys(t, {"club", "team"}, teamPath);
            auto club = r.string(t, "club", teamPath + ".club", ProblemCode::missingValue);
            auto team = r.string(t, "team", teamPath + ".team", ProblemCode::missingValue);
            if (club && team)
                setup.teams.push_back({*club, *team});
        }
    }

    void readSubtypeDeny(Reader& r, ryml::ConstNodeRef conditions, smfigapl::Conditions& setup)
    {
        const ryml::ConstNodeRef subtypes = r.list(conditions, "subtype_deny", "conditions.subtypeDeny");
        if (!subtypes.readable())
            return;
        std::size_t i = 0;
        for (ryml::ConstNodeRef s : subtypes.children())
        {
            const std::string path = at("conditions.subtypeDeny", i++);
            if (!r.isMap(s, "subtype_deny", path))
                continue;
            r.unknownKeys(s, {"subtype", "teams"}, path);
            smfigapl::SubtypeDeny& deny = setup.subtypeDeny.emplace_back();
            deny.subtype = r.string(s, "subtype", path + ".subtype", ProblemCode::missingValue).value_or("");
            readDenyTeams(r, s, path, deny);
        }
    }

    void readConditions(Reader& r, ryml::ConstNodeRef root, smfigapl::Conditions& setup)
    {
        const ryml::ConstNodeRef conditions = r.map(root, "conditions", "conditions");
        if (!conditions.readable())
            return;
        r.unknownKeys(conditions, {"max_pauses", "max_same_club_games", "max_empty_fields", "field_types", "subtype_deny"}, "conditions");
        setup.maxPauses = r.unsignedNumber(conditions, "max_pauses", "conditions.maxPauses");
        setup.maxSameClubGames = r.unsignedNumber(conditions, "max_same_club_games", "conditions.maxSameClubGames");
        setup.maxEmptyFields = r.unsignedNumber(conditions, "max_empty_fields", "conditions.maxEmptyFields").value_or(0);
        readFieldTypes(r, conditions, setup);
        readSubtypeDeny(r, conditions, setup);
    }

    void writeConditions(Writer& w, const smfigapl::Conditions& c)
    {
        if (!c.maxPauses && !c.maxSameClubGames && c.maxEmptyFields == 0 && c.fieldTypes.empty() && c.subtypeDeny.empty())
            return;
        ryml::NodeRef conditions = w.map(w.root(), "conditions");
        if (c.maxPauses)
            w.number(conditions, "max_pauses", *c.maxPauses);
        if (c.maxSameClubGames)
            w.number(conditions, "max_same_club_games", *c.maxSameClubGames);
        if (c.maxEmptyFields != 0)
            w.number(conditions, "max_empty_fields", c.maxEmptyFields);
        if (!c.fieldTypes.empty())
        {
            ryml::NodeRef fieldTypes = w.list(conditions, "field_types");
            for (const smfigapl::FieldTypeLimit& l : c.fieldTypes)
            {
                ryml::NodeRef limit = Writer::mapEntry(fieldTypes, true);
                w.string(limit, "type", l.type);
                if (l.min)
                    w.number(limit, "min", *l.min);
                if (l.max)
                    w.number(limit, "max", *l.max);
            }
        }
        if (!c.subtypeDeny.empty())
        {
            ryml::NodeRef subtypes = w.list(conditions, "subtype_deny");
            for (const smfigapl::SubtypeDeny& d : c.subtypeDeny)
            {
                ryml::NodeRef deny = Writer::mapEntry(subtypes);
                w.string(deny, "subtype", d.subtype);
                ryml::NodeRef teams = w.list(deny, "teams");
                for (const smfigapl::TeamRef& t : d.teams)
                {
                    ryml::NodeRef team = Writer::mapEntry(teams, true);
                    w.string(team, "club", t.club);
                    w.string(team, "team", t.team);
                }
            }
        }
    }
}

smfigapl::Result<smfigapl::TournamentSetup> smfigapl::formats::yaml::readSetup(std::string_view text)
{
    Document document;
    if (auto error = document.parse(text))
        return Result<TournamentSetup>::failure(std::move(*error));
    const ryml::ConstNodeRef root = document.root();
    Reader r(document);
    TournamentSetup setup;
    r.unknownKeys(root, {"format_version", "rounds", "iterations", "repeat_pairings", "home_away_balance", "optimize_round_order", "fields", "clubs", "conditions"}, "", layoutKeys);
    r.version(root, "format_version", setupFormatVersion);
    setup.rounds = r.unsignedNumber(root, "rounds", "rounds", ProblemCode::nRoundsNotSet).value_or(0);
    setup.iterations = r.unsignedNumber(root, "iterations", "iterations").value_or(setup.iterations);
    setup.repeatPairings = r.boolean(root, "repeat_pairings", "repeatPairings").value_or(setup.repeatPairings);
    setup.homeAwayBalance = r.boolean(root, "home_away_balance", "homeAwayBalance").value_or(setup.homeAwayBalance);
    setup.optimizeRoundOrder = r.boolean(root, "optimize_round_order", "optimizeRoundOrder").value_or(setup.optimizeRoundOrder);
    readFields(r, root, setup);
    readClubs(r, root, setup);
    readConditions(r, root, setup.conditions);
    if (r.hasErrors())
        return Result<TournamentSetup>::failure(r.takeProblems());
    return {std::move(setup), r.takeProblems()};
}

std::string smfigapl::formats::yaml::writeSetup(const TournamentSetup& setup)
{
    Writer w;
    ryml::NodeRef root = w.root();
    w.number(root, "format_version", setupFormatVersion);
    w.number(root, "rounds", setup.rounds);
    w.number(root, "iterations", setup.iterations);
    w.boolean(root, "repeat_pairings", setup.repeatPairings);
    w.boolean(root, "home_away_balance", setup.homeAwayBalance);
    w.boolean(root, "optimize_round_order", setup.optimizeRoundOrder);
    ryml::NodeRef fields = w.list(root, "fields");
    for (const Field& f : setup.fields)
    {
        ryml::NodeRef field = Writer::mapEntry(fields, true);
        w.string(field, "name", f.name);
        w.string(field, "type", f.type);
        w.string(field, "subtype", f.subtype);
    }
    ryml::NodeRef clubs = w.list(root, "clubs");
    for (const Club& c : setup.clubs)
    {
        ryml::NodeRef club = Writer::mapEntry(clubs);
        w.string(club, "name", c.name);
        ryml::NodeRef teams = w.list(club, "teams", true);
        for (const std::string& team : c.teams)
            w.stringEntry(teams, team);
    }
    writeConditions(w, setup.conditions);
    return w.text();
}
