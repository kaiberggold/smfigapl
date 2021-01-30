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

#ifndef SMGPFILE
#define SMGPFILE
#include <iostream>
#include <vector>
#include <string>
#include "toml.hpp"
#include "sm_in.hpp"
#include "sm_misc.hpp"
#include "sm_except.hpp"
#include "sm_global.hpp"

namespace smfigapl{

class GamePlanFile
{

	private:
		SmInput *input;
		toml::table inTable_;
		toml::table inTableCfg_;
	public:
		GamePlanFile(smfigapl::SmInput *in):input{in}
		{
		}
		void readFile(std::string fileName, std::string fileNameCfg);
		void readFile(std::string fileName);
		void parseFile();
		void readStdIn();
		void getBool(toml::v2::table *t, std::string key, bool *value,bool required,uint errorId);
		void getBool(toml::v2::table *t, std::string key, bool *value);
		toml::v2::table *getTable(std::vector<std::string>  keys);
		toml::v2::array *getArray(toml::v2::table *table,std::string  key);
		template<class type> void get(toml::v2::table *t, std::string key, type *value, bool required, uint errorId);
		template<class type> void get(toml::v2::table *t, std::string key, type *value);
		void displayErrorMessage(const toml::parse_error& err);
};

} //namespace smGamePlan

#endif
