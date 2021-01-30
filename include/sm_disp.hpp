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

#ifndef SMGPDISPLAY
#define SMGPDISPLAY

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "sm_in.hpp"
#include "sm_misc.hpp"
#include "sm_search.hpp"
#include "sm_stat.hpp"
#include "sm_lang.hpp"


namespace smfigapl{

class SmDisplay
{
	private:
		smfigapl::SmInput *input;
		smfigapl::SmOutput *output;
		smfigapl::GamePlanStatistic *statistic;

		uint spacing_;
		uint spacingLarge_;
		std::vector<std::vector<std::string>> outTable_;
		uint outTableSizex_;
		uint outTableSizey_;
		std::string separator_=",";

	public:
		SmDisplay(smfigapl::SmInput *in, smfigapl::SmOutput *out,smfigapl::GamePlanStatistic *st):input{in}, output{out},statistic{st}
		{
			spacing_=25;
			spacingLarge_=30;
			outTable_.clear();
			outTableSizex_=0;
			outTableSizey_=0;
		}
		~SmDisplay()
		{
			outTable_.clear();
		}
	void displayGamePlanSimple();
	void displayStatisticSimple();
	uint getSpacing(bool large);
	void setSize(uint x , uint y);
	void setOutTable(uint x , uint y, std::string s, bool overWrite);
	void setOutTable(uint x , uint y, std::string s);
	void displayGamePlanFull();
};
}//namespace smfigapl
#endif
