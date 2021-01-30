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
#include <tuple>
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

std::vector<unsigned int> stepsList(std::string path, std::vector<std::string> configFiles)
{
	std::vector<unsigned int> steps;
	steps.resize(0);
	std::string local_path=LOCAL_PATH_STRING;
	toml::table stepList_T = toml::parse_file(local_path+"/test/cfg_files/generic/steps.toml");
	for(auto& f: configFiles)
	{
		std::string fileName =f.substr(f.find_last_of("/")+1);
		fileName=fileName.substr(0, fileName.find_last_of("."));
		if (stepList_T[fileName].is_number())
			steps.push_back(static_cast<unsigned int>(stepList_T[fileName].as_integer()->get()));
		else
			steps.push_back(0);
	}
	return steps;
}

std::vector<std::tuple<std::string,unsigned int>> initParams()
{
	std::string local_path=LOCAL_PATH_STRING;
	std::vector<std::string> configFiles=folderList(local_path+"/test/cfg_files/generic");
	std::vector<unsigned int> steps=stepsList(local_path+"/test/cfg_files/generic/steps.txt",configFiles);
	std::vector<std::tuple<std::string,unsigned int>> temp;
	temp.resize(0);
	for (unsigned int i=0;i<configFiles.size();i++)
	{
		temp.push_back({configFiles[i],steps[i]});
	}
	return temp;
}

class smTestFixture :public ::testing::TestWithParam<std::tuple<std::string,unsigned int>>{
public:

	SmInput *in;
	SmOutput *out;
	GamePlanFile *inFile;
	SmInputValidate *inval;
	GamePlanSearch *gps;
	GamePlanStatistic *stat;
	SmDisplay *disp;

	smTestFixture()
	{
		in = new SmInput();
		out= new SmOutput(in);
		inFile = new  GamePlanFile(in);
		inval = new SmInputValidate(in);
		gps = new GamePlanSearch(in,out);
		stat = new GamePlanStatistic(in,out);
		disp = new SmDisplay (in,out, stat);
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


	gps->setStepsDisplay(steps,stepSize);
	inFile->readFile(std::get<0>(GetParam()));
	inFile->parseFile();
	gps->setMaxSteps(maxSteps);
	gps->setStepsDisplay(steps, stepSize);
	if (inval->validateInput())
	{

		ResultArray result;
		   //

		result=gps->gpSearch();
		if (result.getResult())
		{
			out->setSolved();
		}
		}
}


	void TearDown() override
	{
		if (gps->getSteps()<std::get<1>(GetParam())||std::get<1>(GetParam())==0)
		{
			std::string local_path=LOCAL_PATH_STRING;
			toml::table stepList_T = toml::parse_file(local_path+"/test/cfg_files/generic/steps.toml");
			std::string f= std::get<0>(GetParam());
			std::string fileName =f.substr(f.find_last_of("/")+1);
			fileName=fileName.substr(0, fileName.find_last_of("."));
			stepList_T.insert_or_assign(fileName, toml::value{gps->getSteps()});
			std::ofstream stepsFile;
			stepsFile.open (local_path+"/test/cfg_files/generic/steps.toml");
			stepsFile << stepList_T;
			stepsFile.close();
		}

		delete in ;
		delete out;
		delete inFile;
		delete inval;
		delete gps;
		delete stat;
		delete disp;

	}

	~smTestFixture( )  {
	   // no exceptions
	}

};


TEST_P (smTestFixture, conditions)
{

	//Number of Rounds
	EXPECT_EQ(in->setup.realCapacity.NRounds,out->namedGamePlan.getNRounds())
    		<< "Number of Output Rounds not equal to input rounds. Cfg-File: "<<std::get<0>(GetParam());

    //Lim Fieldtypes
    if (in->conditions.limGamesFieldType.enabled)
    {
    	for (unsigned int i=0;i<in->setup.realCapacity.NTeams;i++)
    		for (unsigned int j=0;j<in->setup.realCapacity.NFieldTypes;j++)
    			{
    			EXPECT_LE(stat->calcNGamesFieldType(i,j),in->conditions.limGamesFieldType.values.max[j])
						<< "N Games on ft: "<<j<<" for team: "<<i<<"larger than maximum: " <<in->conditions.limGamesFieldType.values.max[j]<<", Cfg-File: "<<std::get<0>(GetParam());
    			EXPECT_GE(stat->calcNGamesFieldType(i,j),in->conditions.limGamesFieldType.values.min[j])
						<< "N Games on ft: "<<j<<" for team: "<<i<<"smaller than minimum: " <<in->conditions.limGamesFieldType.values.min[j]<<", Cfg-File: "<<std::get<0>(GetParam());
    			}
    }

    //Max Games Same Club
    if (in->conditions.maxGamesSameClub.enabled)
    {
		for (unsigned int i=0;i<in->setup.realCapacity.NTeams;i++)
		{
			EXPECT_LE(stat->calcNGamesSameClub(i),in->conditions.maxGamesSameClub.values[i]);
		}
    }
    //Max Pause Games
    if (in->conditions.maxPauseGames.enabled)
	{
		for (unsigned int i=0;i<in->setup.realCapacity.NTeams;i++)
		{
			EXPECT_LE(out->namedGamePlan.getNRounds()-stat->calcNGames(i),in->conditions.maxPauseGames.value)<<"N Pause Games for team :"<<i<< " larger than maximum: "<<out->namedGamePlan.getNRounds()-stat->calcNGames(i)<<", Cfg-File: "<<std::get<0>(GetParam());
		}
    }

    //number of steps
	if (std::get<1>(GetParam())>0)
	{
		ASSERT_LE(gps->getSteps(),std::get<1>(GetParam()))<<"Nr. of steps: "<<gps->getSteps()<< " larger than stored valus: "<<std::get<1>(GetParam())<<", Cfg-File: "<<std::get<0>(GetParam());
	}

    //Home Away balance
    if (in->conditions.homeAwayBalance)
	{

		for (unsigned int i=0;i<in->setup.realCapacity.NTeams;i++)
		{
			EXPECT_LE(std::max(stat->calcNHomeAway(i, true),stat->calcNHomeAway(i, false))-\
				std::min(stat->calcNHomeAway(i, true),stat->calcNHomeAway(i, false)),static_cast<uint>(1))\
				<<"Home Away Balance team: "<<i<< ", Cfg-File: "<<std::get<0>(GetParam());
		}
    }

    //PauseTeams Plausibility no team twice in pause
	for (unsigned int i=0;i<in->setup.realCapacity.NRounds;i++)
	{

		for (unsigned int j=0;j<out->indexedGamePlan.getNPauseTeams(i);j++)
		{

			for (unsigned int k=0;k<out->indexedGamePlan.getNPauseTeams(i);k++)
			{
				if (k!=j)
				{
					bool check=out->indexedGamePlan.getPauseTeam(i, j).getTeamId()!=out->indexedGamePlan.getPauseTeam(i, k).getTeamId();
					EXPECT_TRUE(check)
						<< "Team more than once in Pause Teams in Round: "<<i<<" Index: "<<j<<", Cfg-File: "<<std::get<0>(GetParam());
				}
			}
		}

	}


}

std::string local_path=LOCAL_PATH_STRING;
std::vector<std::string> configFiles=folderList(local_path+"/test/cfg_files/generic/");
std::vector<std::tuple<std::string,unsigned int>> testParams=initParams();

INSTANTIATE_TEST_SUITE_P(generic_cfg_files, smTestFixture,
                         testing::ValuesIn(testParams));
