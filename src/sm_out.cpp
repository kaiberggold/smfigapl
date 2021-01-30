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

#include <iostream>
#include <vector>
#include <string>

#include "sm_misc.hpp"
#include "../include/sm_out.hpp"


void smfigapl::SmOutput::init(uint na)
{
	indexedGamePlan.init(input->setup.realCapacity.NRounds,input->setup.realCapacity.NFields,na);
	namedGamePlan.init(input->setup.realCapacity.NRounds,input->setup.realCapacity.NFields);
}
bool smfigapl::SmOutput::isSolved()
{
	return solved_;
}

void smfigapl::SmOutput::setSolved()
{
	solved_=true;
}

void smfigapl::SmOutput::resetSolved()
{
	solved_=false;
}

void smfigapl::SmOutput::setResult(smfigapl::ResultArray res)
{
	if (res.getResult())
	{
		solved_=true;
		resultMessage_=input->setup.lang.get("Solved");
	}
	else
	{
		solved_=false;
		resultMessage_=input->setup.lang.get("Not Solved");
	}
}

std::string smfigapl::SmOutput::getResultMessage()
{
	return resultMessage_;
}
