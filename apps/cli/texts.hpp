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

// The texts of the command line tool: the labels of the output and the
// messages of the problems, in English and German. The library has no
// texts; it gives problem codes and values.

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "smfigapl/report/labels.hpp"
#include "smfigapl/result.hpp"

namespace smfigapl::cli
{

// Language used when none is given (option -l)
    inline constexpr const char* defaultLanguage = "DE";

    class Lang
    {
    private:
        std::string language_;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations_ =
        {
            {
                "EN", {
                    {"tournament", "Tournament"},
                    {"round", "Round"},
                    {"rounds", "Rounds"},
                    {"break", "Break"},
                    {"iterations", "Iterations"},
                    {"errorId", "Error number"},
                    {"warningId", "Warning number"},
                    {"n_games", "Number of games"},
                    {"home_games", "Number of home games"},
                    {"away_games", "Number of away games"},
                    {"same_club", "Same Club"},
                    {"field_types", "field_types"},
                    {"n_same_pairing", "Repeated Games"},
                    {"max_games_same_team", "Max Games same Team"},
                    {"Impossible", "No game plan possible"},
                    {"Aborted", "Search aborted (limit reached), unknown whether a game plan exists"}
                }
            },
            {
                "DE", {
                    {"tournament", "Turnier"},
                    {"round", "Runde"},
                    {"rounds", "Runden"},
                    {"break", "Pause"},
                    {"iterations", "Wiederholungen"},
                    {"errorId", "Fehler Nummer"},
                    {"warningId", "Warnung Nummer"},
                    {"n_games", "Anzahl Spiele"},
                    {"home_games", "Anzahl Heimspiele"},
                    {"away_games", "Anzahl Auswaertsspiele"},
                    {"file_scheme", "Dateischema"},
                    {"repeat_pairings", "Mehrfachpaarungen"},
                    {"field_types", "Feldtypen"},
                    {"n_same_pairing", "Spielwiederholungen"},
                    {"max_games_same_team", "Maximale Wiederholung"},
                    {"fields", "Felder"},
                    {"field_types", "Feldtypen"},
                    {"type", "Typ"},
                    {"subtype", "Zuordnung"},
                    {"subtypes", "Zuordnungen"},
                    {"conditions", "Bedingungen"},
                    {"field_types", "Feldtypen"},
                    {"pause_games", "Pausen"},
                    {"same_club", "Vereinsintern"},
                    {"teams_deny", "Sperrliste"},
                    {"colors", "Farben"},
                    {"clubs", "Vereine"},
                    {"name", "Name"},
                    {"min", "Min"},
                    {"max", "Max"},
                    {"false", "Nein"},
                    {"true", "Ja"},
                    {"Solved", "Spielplan berechnet"},
                    {"Impossible", "Es gibt keinen Spielplan"},
                    {"Aborted", "Suche abgebrochen (Grenze erreicht), es ist offen, ob es einen Spielplan gibt"}
                }
            },

            // Only as example for further language
            {
                "FR", {
                    {"tournament", "Tournament"},
                    {"round", "Round"},
                }
            }
        };
        std::unordered_map<std::string, std::unordered_map<ProblemCode, std::string>> errorTranslations_ =
        {
            {
                "EN", {
                    {ProblemCode::nRoundsGtMax, "Number of Rounds too big (Maximum $0)"},
                    {ProblemCode::nRoundsLt1, "Number of Rounds too small (Minimum 1)"},
                    {ProblemCode::searchTooLarge, "Too many teams and fields in one tournament: $0 possible rounds (maximum $1). Use fewer fields or split the tournament with subtypes"},
                    {ProblemCode::searchTooManyPairings, "Too many teams in one tournament: $0 pairings of the teams (maximum $1). Use fewer fields or split the tournament with subtypes"},
                    {ProblemCode::nFieldsGtMax, "Number of Fields larger than maximum allowed number of $0 fields"},
                    {ProblemCode::nFieldsLt1, "Number of Fields too small (Minimum 1)"},
                    {ProblemCode::parseError, "Parsing Error"},
                    {ProblemCode::nRoundsNotSet, "Number of rounds missing (rounds)"},
                    {ProblemCode::noFieldsDefined, "No fields defined ($0)"},
                    {ProblemCode::noClubsDefined, "No clubs defined ($0)"},
                    {ProblemCode::missingValue, "Value missing: $0"},
                    {ProblemCode::invalidValue, "Invalid value in $0"},
                    {ProblemCode::unsupportedFormatVersion, "Format version $0 is newer than the supported version $1, read as far as possible"},
                    {ProblemCode::unknownKey, "Unknown key $0 ignored"},
                    {ProblemCode::clubIndexGtMax, "Too many clubs (Maximum $0)"},
                    {ProblemCode::clubIndexOverflow, "Internal error: unknown club or color ($0, $1)"},
                    {ProblemCode::teamColorIndexGtMax, "Too many team colors (Maximum $0)"},
                    {ProblemCode::maxLtMin, "Minimum $0 larger than maximum $1 (field_types)"},
                    {ProblemCode::impossibleFieldTypeMin, "Field type $0 has $1 places, the minimums of the teams need $2"},
                    {ProblemCode::impossibleFieldTypeMax, "Field type $0 has $1 places to fill, the maximums of the teams allow only $2"},
                    {ProblemCode::impossibleTeamFieldTypeMin, "Team $0 needs $1 games for the minimums of the field types, but plays at most $2"},
                    {ProblemCode::impossibleTeamGames, "Team $0 has to play at least $1 games, the maximums of the field types allow only $2"},
                    {ProblemCode::impossiblePauses, "At least $0 pauses are needed, the pause limit allows only $1"},
                    {ProblemCode::impossibleSameClub, "Team $0 has to play at least $1 games, the club limit and the opponents allow only $2"},
                    {ProblemCode::impossibleOpponents, "Team $0 has to play at least $1 games, but has only $2 opponents left (pairings may not be repeated more often)"},
                    {ProblemCode::impossibleSearch, "The search tried all possibilities"},
                    {ProblemCode::impossibleGames, "The fields need at least $0 teams in all rounds, the teams can play only $1 games"},
                    {ProblemCode::planRounds, "The plan has $0 rounds, the setup $1"},
                    {ProblemCode::planFields, "Round $0: field $1 of the plan, field $2 of the setup"},
                    {ProblemCode::planUnknownTeam, "Team $0 (round $1) is not in the setup"},
                    {ProblemCode::planTeamTwice, "Team $0 plays more than once in round $1"},
                    {ProblemCode::planPauses, "Team $0 pauses $1 times, allowed are $2"},
                    {ProblemCode::planEmptyFields, "Round $0 has $1 empty fields, allowed are $2"},
                    {ProblemCode::planSameClub, "Team $0 plays its own club $1 times, allowed are $2"},
                    {ProblemCode::planFieldTypeMin, "Team $0 plays $2 games on field type $1, at least $3 are needed"},
                    {ProblemCode::planFieldTypeMax, "Team $0 plays $2 games on field type $1, at most $3 are allowed"},
                    {ProblemCode::planSubtypeDenied, "Team $0 may not play on field $1 (round $2)"},
                    {ProblemCode::planRepeatedPairing, "$0 and $1 play each other $2 times, allowed are $3"},
                    {ProblemCode::planOpenGame, "Team $0 has no opponent in round $1"}
                }
            },
            {
                "DE", {
                    {ProblemCode::nRoundsGtMax, "Anzahl der Runden zu Groß (Maximal $0)"},
                    {ProblemCode::nRoundsLt1, "Anzahl der Runden zu klein (Minimum 1)"},
                    {ProblemCode::searchTooLarge, "Zu viele Teams und Felder in einem Turnier: $0 mögliche Runden (Maximum $1). Weniger Felder verwenden oder das Turnier mit Untertypen teilen"},
                    {ProblemCode::searchTooManyPairings, "Zu viele Teams in einem Turnier: $0 Paarungen der Teams (Maximum $1). Weniger Felder verwenden oder das Turnier mit Untertypen teilen"},
                    {ProblemCode::nFieldsGtMax, "Anzahl Felder größer als Maximum ($0)"},
                    {ProblemCode::nFieldsLt1, "Anzahl Felder zu klein (Minimum 1)"},
                    {ProblemCode::parseError, "Parsing Fehler"},
                    {ProblemCode::nRoundsNotSet, "Anzahl der Runden fehlt (Runden)"},
                    {ProblemCode::noFieldsDefined, "Keine Felder definiert ($0)"},
                    {ProblemCode::noClubsDefined, "Keine Vereine definiert ($0)"},
                    {ProblemCode::missingValue, "Wert fehlt: $0"},
                    {ProblemCode::invalidValue, "Ungültiger Wert in $0"},
                    {ProblemCode::unsupportedFormatVersion, "Formatversion $0 ist neuer als die unterstützte Version $1, so weit wie möglich gelesen"},
                    {ProblemCode::unknownKey, "Unbekannter Schlüssel $0 ignoriert"},
                    {ProblemCode::clubIndexGtMax, "Zu viele Vereine (Maximal $0)"},
                    {ProblemCode::clubIndexOverflow, "Interner Fehler: unbekannter Verein oder Farbe ($0, $1)"},
                    {ProblemCode::teamColorIndexGtMax, "Zu viele Farben (Maximal $0)"},
                    {ProblemCode::maxLtMin, "Minimum $0 größer als Maximum $1 (Feldtypen)"},
                    {ProblemCode::impossibleFieldTypeMin, "Feldtyp $0 hat $1 Plätze, die Minima der Teams brauchen $2"},
                    {ProblemCode::impossibleFieldTypeMax, "Feldtyp $0 hat $1 zu füllende Plätze, die Maxima der Teams erlauben nur $2"},
                    {ProblemCode::impossibleTeamFieldTypeMin, "Team $0 braucht $1 Spiele für die Minima der Feldtypen, spielt aber höchstens $2"},
                    {ProblemCode::impossibleTeamGames, "Team $0 muss mindestens $1 Spiele spielen, die Maxima der Feldtypen erlauben nur $2"},
                    {ProblemCode::impossiblePauses, "Mindestens $0 Pausen sind nötig, die Pausengrenze erlaubt nur $1"},
                    {ProblemCode::impossibleSameClub, "Team $0 muss mindestens $1 Spiele spielen, die Vereinsgrenze und die Gegner erlauben nur $2"},
                    {ProblemCode::impossibleOpponents, "Team $0 muss mindestens $1 Spiele spielen, hat aber nur noch $2 Gegner (Paarungen dürfen nicht öfter wiederholt werden)"},
                    {ProblemCode::impossibleSearch, "Die Suche hat alle Möglichkeiten probiert"},
                    {ProblemCode::impossibleGames, "Die Felder brauchen in allen Runden mindestens $0 Teams, die Teams können nur $1 Spiele spielen"},
                    {ProblemCode::planRounds, "Der Plan hat $0 Runden, das Setup $1"},
                    {ProblemCode::planFields, "Runde $0: Feld $1 des Plans, Feld $2 des Setups"},
                    {ProblemCode::planUnknownTeam, "Team $0 (Runde $1) ist nicht im Setup"},
                    {ProblemCode::planTeamTwice, "Team $0 spielt in Runde $1 mehr als einmal"},
                    {ProblemCode::planPauses, "Team $0 pausiert $1-mal, erlaubt sind $2"},
                    {ProblemCode::planEmptyFields, "Runde $0 hat $1 leere Felder, erlaubt sind $2"},
                    {ProblemCode::planSameClub, "Team $0 spielt $1-mal gegen den eigenen Verein, erlaubt sind $2"},
                    {ProblemCode::planFieldTypeMin, "Team $0 spielt $2 Spiele auf Feldtyp $1, nötig sind mindestens $3"},
                    {ProblemCode::planFieldTypeMax, "Team $0 spielt $2 Spiele auf Feldtyp $1, erlaubt sind höchstens $3"},
                    {ProblemCode::planSubtypeDenied, "Team $0 darf nicht auf Feld $1 spielen (Runde $2)"},
                    {ProblemCode::planRepeatedPairing, "$0 und $1 spielen $2-mal gegeneinander, erlaubt sind $3"},
                    {ProblemCode::planOpenGame, "Team $0 hat in Runde $1 keinen Gegner"}
                }
            },

            // Only as example for further language
            {
                "FR", {
                    {ProblemCode::nRoundsGtMax, "Round"},
                }
            }
        };



        // the translation of in into the language l, nullptr if there is none
        [[nodiscard]] const std::string* find(const std::string& l, const std::string& in) const;

    public:
        Lang(): language_(defaultLanguage)
        {
        }
        static std::vector<std::string> getLanguages()
        {
            return{"EN", "DE"};
        }
        [[nodiscard]] std::string   get(const std::string& in) const;
        [[nodiscard]] std::string   get(const std::string& in, const std::string& l) const;
        [[nodiscard]] std::string   getErrorMessage(ProblemCode errorId) const;
        void setLanguage(std::string l);
    };

    // The text of a problem in the language of lang, with its values, without number and position
    [[nodiscard]] std::string problemText(const Problem& problem, const Lang& lang);

    // "<Error number><code>: <text>", the detail (position of a syntax error,
    // the invalid value) on the next line; source: the file of a syntax error
    [[nodiscard]] std::string problemMessage(const Problem& problem, const Lang& lang, const std::string& source = "");

    // The texts of the output writers in the language of lang
    [[nodiscard]] report::Labels labels(const Lang& lang);
}//namespace smfigapl::cli
