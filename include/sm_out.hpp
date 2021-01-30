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

#ifndef SMOUTPUT
#define SMOUTPUT
#include <iostream>
#include <vector>
#include <string>

#include "sm_in.hpp"
#include "sm_misc.hpp"


namespace smfigapl{

class SmOutput
{
private:
	smfigapl::SmInput *input;
	bool solved_;
	std::string resultMessage_;
public:
	IndexedGamePlan indexedGamePlan;
	NamedGamePlan namedGamePlan;
	SmOutput(smfigapl::SmInput *in):input{in}
	{
		resultMessage_="";
		solved_=false;
	};
	void init(uint na);
	bool isSolved();
	void setSolved();
	void resetSolved();
	void setResult(ResultArray res);
	std::string getResultMessage();

};

}; //namespace smGamePlan

#endif
