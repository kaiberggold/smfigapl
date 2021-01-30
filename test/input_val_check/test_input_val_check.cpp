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

#define NOMINMAX
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <iostream>
#include <filesystem>
#include "gtest/gtest.h"

#include "sm_global.hpp"
#include "cxxopts.hpp"
#include "sm_disp.hpp"
#include "sm_file.hpp"
#include "sm_in.hpp"
#include "sm_out.hpp"
#include "sm_search.hpp"
#include "sm_stat.hpp"
#include "sm_val.hpp"
#include "sm_except.hpp"
#include "toml.hpp"
#include "path.hpp"
#include "test_misc.hpp"
#undef SM_DBG

using namespace smfigapl;
using ::testing::TestWithParam;
using ::testing::Values;

std::vector<std::string> initParamsVal()
{
	std::string local_path=LOCAL_PATH_STRING;
	return folderList(local_path+"/test/cfg_files/input_validation");
}

class smTestValFixture :public ::testing::TestWithParam<std::string>{
public:

	SmInput *in;
	SmOutput *out;
	GamePlanFile *inFile;
	SmInputValidate *inval;
	smTestValFixture()
	{
		in = new SmInput();
		in->setup.lang.setLanguage("EN");
		out= new SmOutput(in);
		inFile = new  GamePlanFile(in);
		inval = new SmInputValidate(in);
	}


void SetUp ( ) override
{

	unsigned int maxSteps=std::numeric_limits<unsigned int>::max();
	std::string fileNameIn="";
	std::string fileNameOut="";
	unsigned int stepSize=1000;
	bool steps=false;
	bool debug=false;
	unsigned int dbg=0;
	bool calcStat=false;
	bool enableStopWatch=false;
	bool enableFileWrite=false;


	#ifdef SM_DBG
	in->dbg=dbg;
	#endif


	inFile->readFile(GetParam());
	inFile->parseFile();
}


	void TearDown() override
	{
		delete in ;
		delete out;
		delete inFile;
		delete inval;
	}

	~smTestValFixture( )  {
		//no exceptions
	}
};

TEST_P (smTestValFixture, conditions)
{

	//Number of Rounds
	if (in->setup.realCapacity.NRounds>MAX_ROUNDS)
		 EXPECT_EXIT(bool res=inval->validateInput(),testing::ExitedWithCode(VAL_ERROR_NROUNDS_GT_MAX),".*Number of Rounds too big.*")<<GetParam();
	if (in->setup.realCapacity.NRounds<1)
		 EXPECT_EXIT(bool res=inval->validateInput(),testing::ExitedWithCode(VAL_ERROR_NROUNDS_LT_1),".*Number of Rounds too small.*")<<GetParam();
	if (in->setup.realCapacity.NFields<1)
		 EXPECT_EXIT(bool res=inval->validateInput(),testing::ExitedWithCode(VAL_ERROR_NFIELDS_LT_1),".*Number of Fields too small.*")<<GetParam();
	if (in->setup.realCapacity.NFields>MAX_FIELDS)
		 EXPECT_EXIT(bool res=inval->validateInput(),testing::ExitedWithCode(VAL_ERROR_NFIELDS_GT_MAX),".*Number of Fields larger than maximum allowed number of.*")<<GetParam();

}

std::vector<std::string> testParamsVal=initParamsVal();

INSTANTIATE_TEST_SUITE_P(input_validation, smTestValFixture,
                         testing::ValuesIn(testParamsVal));

