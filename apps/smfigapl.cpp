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

#include "sm_global.hpp"
#include "Stopwatch.hpp"
#include "cxxopts.hpp"
#include "sm_disp.hpp"
#include "sm_file.hpp"
#include "sm_in.hpp"
#include "sm_out.hpp"
#include "sm_search.hpp"
#include "sm_stat.hpp"
#include "sm_val.hpp"
#include "sm_except.hpp"
#include "sm_version.hpp"

int main(int argc, const char* argv[])
{
#ifdef SM_CATCH_ALL
	try
	{
	#endif
		//smfigapl::uint maxSteps=std::numeric_limits<smfigapl::uint>::max();
		smfigapl::uint maxSteps=10000000;
		std::string fileNameIn="";
		std::string cfgFileNameIn="";
		std::string fileNameOut="";
		std::string lang=SM_DEFAULT_LANGUAGE;
		smfigapl::uint stepSize;
		bool steps=false;
		bool debug=false;
		smfigapl::uint dbg=0;
		bool calcStat=false;
		bool enableStopWatch=false;
		bool enableFileWrite=false;
		bool textFile=false;
		bool stdInStream=false;
		bool separateFormatFile=false;

		try
		{
			cxxopts::Options options(argv[0], "Small Field Game Planer.\nGame Plan algorithm for special cases");
			options
			.add_options()
			 #ifdef SM_DBG
			 ("h,help", "Print usage")
			 ("v,version", "Print version")
			 ("d,debug", "Debug", cxxopts::value<bool>())
			 ("e,debugoptions", "Debug Options", cxxopts::value<smfigapl::uint>())
			 #endif
			 ("w,stopwatch", "Enable Stopwatch", cxxopts::value<bool>())
			 ("m,maxsteps", "Maximum search steps", cxxopts::value<smfigapl::uint>(), "N")
			 ("p,progress", "Print Progress", cxxopts::value<bool>())
			 ("s,statistic", "Print Statistic (For text output)", cxxopts::value<bool>())
			 ("t,text", "Simple Text Output", cxxopts::value<bool>())
			 ("z,stepsize", "Steps printing spacing", cxxopts::value<smfigapl::uint>(), "N")
			 ("i,input", "Input File", cxxopts::value<std::string>(), "FILE")
			 ("f,format", "Format File", cxxopts::value<std::string>(), "FILE")
			 ("l,language", "Language (EN,DE)", cxxopts::value<std::string>()->default_value(SM_DEFAULT_LANGUAGE), "LANGUAGE")
			 ("o,output", "Output File", cxxopts::value<std::string>(),"FILE");
			#ifndef SM_STREAM_STDIN
			if (argc==1)
				{
				  std::cout << options.help({"", "Group"}) << std::endl;
				  exit(0);
				}
			#endif
			auto params = options.parse(argc, argv);
			#ifdef SM_DBG
			if (params.count("d"))
			{
				debug=true;
			  std::cout << "Debugging enabled" << std::endl;
				if (params.count("e"))
				{
					dbg=params["e"].as<smfigapl::uint>();
				  std::cout << "Debug state" << dbg <<std::endl;
				}
			}
			#endif
			if (params.count("help"))
				{
				  std::cout << options.help({"", "Group"}) << std::endl;
				  exit(0);
				}
			if (params.count("version"))
				{
				std::string v=VERSION;
				  std::cout << "Smfigapl Version: " <<v<< std::endl;
				  exit(0);
				}

			if (params.count("m"))
			{
				maxSteps=params["m"].as<smfigapl::uint>();
				if (debug)
					std::cout<<"Maxsteps: "<<maxSteps<<std::endl;
			}
			if (params.count("p"))
			{
				steps=true;
				if (debug)
					std::cout << "Display Progress enabled" << std::endl;
			}
			if (params.count("w"))
			{
				enableStopWatch=true;
				if (debug)
					std::cout << "Stopwatch enabled" << std::endl;
			}
			if (params.count("z"))
			{
				stepSize=params["z"].as<smfigapl::uint>();
				if (debug)
					std::cout<<"Step size: "<<stepSize<<std::endl;
			}
			else
				stepSize=smfigapl::NSTEPSDISPLAY;

			// Statistic
			if (params.count("s"))
			{
				calcStat=true;
				if (debug)
					std::cout << "Statistic enabled" << std::endl;
			}
			if (params.count("t"))
			{
				textFile=true;
				if (debug)
					std::cout << "Textfile enabled" << std::endl;
			}
			if (params.count("language"))
			{
				lang =params["l"].as<std::string>();
				if (debug)
					std::cout<<"Language: "<<lang<<std::endl;
			}

			if (params.count("o"))
			{
				enableFileWrite=true;
				fileNameOut=params["o"].as<std::string>();
				if (debug)
					std::cout<<"fileNameOut: "<<fileNameOut<<std::endl;
			}
			if (params.count("i"))
			{
				fileNameIn =params["i"].as<std::string>();
				if (debug)
					std::cout<<"fileNameIn: "<<fileNameIn<<std::endl;
			}
			else
			{
				stdInStream=true;
			}
			if (params.count("f"))
			{
				cfgFileNameIn =params["f"].as<std::string>();
				if (debug)
					std::cout<<"CfgfileNameIn: "<<fileNameIn<<std::endl;
				separateFormatFile=true;
			}


		}
		catch (const cxxopts::OptionException& e)
		{
			std::cout << "error parsing options: " << e.what() << std::endl;
			exit(1);
		}

		using namespace smfigapl;
		SmInput in;
		in.setup.lang.setLanguage(lang);
		#ifdef SM_DBG
		in.dbg=dbg;
		#endif
		SmOutput out(&in);
		GamePlanFile inFile(&in);
		SmInputValidate inval(&in);
		GamePlanSearch gps(&in,&out);
		GamePlanStatistic stat(&in,&out);
		SmDisplay disp(&in,&out, &stat);

		gps.setStepsDisplay(steps,stepSize);
		std::string std_input;
		std::stringstream stream;
		if (stdInStream)
		{
			inFile.readStdIn();
		}
		else
		{
			if (separateFormatFile)
				inFile.readFile(fileNameIn,cfgFileNameIn);
			else
				inFile.readFile(fileNameIn);
		}
		inFile.parseFile();
		gps.setMaxSteps(maxSteps);
		gps.setStepsDisplay(steps, stepSize);
		if (inval.validateInput())
		{

			ResultArray result;
			   //

	   //Create and start a stopwatch
	   stopwatch::Stopwatch my_watch;
	   if (enableStopWatch)
	   {

		   my_watch.start();
	   }
	   result=gps.gpSearch();
	   if (enableStopWatch)
	   {
		   std::cout<<std::endl<<"Time: "<<my_watch.elapsed()<<" ms"<<std::endl;
		   std::cout<<"Steps: "<<gps.getSteps()<<std::endl;
	   }
	   if (result.getResult())
	   {

		   if (enableFileWrite)
		   {
			   std::ofstream out(fileNameOut);
			   std::streambuf *coutBuf = std::cout.rdbuf();
			   std::cout.rdbuf(out.rdbuf());
			   if (textFile)
			   {
				   disp.displayGamePlanSimple();
				   if (calcStat)
				   {
					   stat.calcStatistic();
					   disp.displayStatisticSimple();
				   }
			   }
			   else
			   {
				   if (in.format.settings.showStatistic)
					   stat.calcStatistic();
				   disp.displayGamePlanFull();
			   }
			   std::cout.rdbuf(coutBuf);
		   }
		   else
		   {
			   if (textFile)
			   {
				   disp.displayGamePlanSimple();
				   if (calcStat&textFile)
				   {
					   stat.calcStatistic();
					   disp.displayStatisticSimple();
				   }
			   }
			   else
			   {
				   if (in.format.settings.showStatistic)
					   stat.calcStatistic();
				   disp.displayGamePlanFull();
			   }
			}
	   }
	   else
	   {
			std::cerr <<"Aborted:"<<result.getResult()<<std::endl;
			if (textFile)
			{
				disp.displayGamePlanSimple();
				if (calcStat)
				{
					stat.calcStatistic();
					disp.displayStatisticSimple();
				}
			}
			else
			{
			   disp.displayGamePlanFull();
			}
		}
	}
		else
		{
			std::cout <<inval.getValError();
			exit(2);
		}
		#ifdef SM_DBG
		if (debug)
		{
			std::cout<<std::endl;
			gps.dispNFailArray();
		}
			#endif
			return 0;
	#ifdef SM_CATCH_ALL
	}
	catch(smfigapl::SmException const& e)
	{
	    std::cerr<<e.what();
	}

	catch (...)
	{
		 std::cerr << "Unknown Error\n";
		 exit(-1 );
	}
	#endif

	exit(0);
}

