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

#ifndef SMSTAT
#define SMSTAT

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

#include "sm_in.hpp"
#include "sm_misc.hpp"
#include "sm_out.hpp"
#include "sm_lang.hpp"

namespace smfigapl{

class GamePlanStatistic
{
	private:
		SmInput *input;
		SmOutput *output;
		std::unordered_map<std::string,std::vector<uint>> statisticArray_;
		std::unordered_map<std::string,std::string> statHeader_;
		std::vector<std::string> statKeys_;
	public:
		GamePlanStatistic(smfigapl::SmInput *in, smfigapl::SmOutput *out):input{in} ,output{out}
		{
			;
		}
		uint calcNGames(uint teamId);
		uint calcNRepeatedGames(uint teamId);
		uint calcMaxGamesSameTeam(uint teamId);
		uint calcNGamesFieldType(uint teamId, uint ft);
		uint calcNGamesSameClub(uint teamId);
		uint calcNHomeAway(uint teamId, bool home);
		void calcStatistic();

		std::string getHeader (std::string s)
		{
			return statHeader_[s];
		}
		uint getEntry (std::string s, uint i)
		{
			return statisticArray_[s][i];
		}
		std::string getKey(uint i)
		{
			return statKeys_[i];
		}
		uint getNKeys()
		{
			return static_cast<uint>(statKeys_.size());
		}

};
}//namespace smfigapl
#endif
