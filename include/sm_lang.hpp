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

#ifndef SMLANG
#define SMLANG

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "sm_misc.hpp"
#include "sm_global.hpp"

namespace smfigapl{

class SmLang
{
private:
	std::string language_;
	std::unordered_set<std::string> languages_={"EN","DE","FR"};
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations_ = {
		{"EN",{
		        {"tournament", "Tournament"},
		        {"round", "Round"},
				{"rounds", "Rounds"},
				{"break", "Break"},
				{"iterations", "Iterations"},
		        {"errorId","Error number"},
				{"n_games", "Number of games"},
		        {"home_games", "Number of home games"},
		        {"away_games", "Number of away games"},
		        {"same_club", "Same Club"},
		        {"field_types", "field_types"},
				{"n_same_pairing","Repeated Games"},
		        {"max_games_same_team","Max Games same Team"}

		}},
        {"DE",{
	        {"tournament", "Turnier"},
	        {"round", "Runde"},
			{"rounds", "Runden"},
			{"break", "Pause"},
			{"iterations", "Wiederholungen"},
	        {"errorId","Fehler Nummer"},
	        {"n_games", "Anzahl Spiele"},
	        {"home_games", "Anzahl Heimspiele"},
	        {"away_games", "Anzahl Auswaertsspiele"},
	        {"file_scheme", "Dateischema"},
			{"repeat_pairings", "Mehrfachpaarungen"},
	        {"field_types", "Feldtypen"},
			{"n_same_pairing","Spielwiederholungen"},
	        {"max_games_same_team","Maximale Wiederholung"},
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
	        {"Not Solved", "Spielplan konnte nicht berechnet werden"}
		}},

		// Only as example for further language
	    {"FR", {
	        {"tournament", "Tournament"},
	        {"round", "Round"},
	    }}
	};
	std::unordered_map<std::string, std::unordered_map<uint, std::string>> errorTranslations_ = {
			{"EN",{
					{VAL_ERROR_NROUNDS_GT_MAX, "Number of Rounds too big (Maximum $0)"},
					{VAL_ERROR_NROUNDS_LT_1, "Number of Rounds too small (Minimum 1)"},
					{VAL_ERROR_NFIELDS_GT_MAX, "Number of Fields larger than maximum allowed number of $0 fields"},
					{VAL_ERROR_NFIELDS_LT_1, "Number of Fields too small (Minimum 1)"},
					{TOML_PARSE_ERROR,"Parsing Error"}
				}},
				{"DE",{
			        {VAL_ERROR_NROUNDS_GT_MAX, "Anzahl der Runden zu Groß (Maximal $0)"},
					{VAL_ERROR_NROUNDS_LT_1, "Anzahl der Runden zu klein (Minimum 1)"},
					{VAL_ERROR_NFIELDS_GT_MAX, "Anzahl Felder größer als Maximum ($0)"},
					{VAL_ERROR_NFIELDS_LT_1, "Anzahl Felder zu klein (Minimum 1)"},
					{TOML_PARSE_ERROR,"Parsing Fehler"}
			    }},

				// Only as example for further language
			    {"FR", {
			        {VAL_ERROR_NROUNDS_GT_MAX, "Round"},
			    }}
		};



public:
	SmLang()
	{
		language_=SM_DEFAULT_LANGUAGE;
	}
	std::vector<std::string> getLanguages()
		{
			return{"EN","DE"};
		}
	std::string	get(std::string in);
	std::string	get(std::string in, std::string l);
	std::string	getErrorMessage(uint errorId);
	void setLanguage(std::string l);

	std::vector<std::string> getAll(std::string in);
	std::vector<std::string> getAllWithCapitalization(std::string in);
	std::vector<std::string> getAllWithCapitalization(std::string in, std::string l);
};
}//namespace smfigapl
#endif
