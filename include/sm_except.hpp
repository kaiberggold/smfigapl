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

#ifndef SMEXCEPT
#define SMEXCEPT

#include <iostream>
#include <string>
#include <exception>
#include <map>
#include <regex>

#include "sm_misc.hpp"
#include "sm_lang.hpp"
#include "sm_in.hpp"

namespace smfigapl{


class SmException : public std::exception
{
protected:
	#ifdef SM_DBG
    std::string dbgText_;
	#endif
    uint errorId_;
    std::vector<std::string> params_;
	smfigapl::SmInput *input;

public:
	#ifdef SM_DBG
	SmException(std::string dt, uint id, std::vector<std::string> p,smfigapl::SmInput *in) : dbgText_(dt),errorId_(id),params_(p),input{in} { }
	#else
	SmException(uint id, std::vector<std::string> p,smfigapl::SmInput *in) : errorId_(id),params_(p),input{in} { }
	#endif
    const char * what () const throw ()
	{
    	return "";
	}
	uint  getErrorId() const
	{
		return errorId_;
	}

};

class SmValueException : public SmException
{
	using SmException::SmException;
	public:
		const char * what () const throw ()
		{
				std::string errorText=input->setup.lang.getErrorMessage(errorId_);
		    	for(uint i=0; i<static_cast<uint>(params_.size());i++)
		    	{
		    		errorText = std::regex_replace(errorText, std::regex("\\$"+std::to_string(i)),params_[i]);
		    	}
		    	std::string strMessage=input->setup.lang.get("errorId") +std::to_string(errorId_)+": "+errorText;
				#ifdef SM_DBG
		    	strMessage=strMessage+"\n"+dbgText_;
				#endif
		    	auto temp=strMessage.c_str();
		        return temp;
		}


};

}//namespace
#endif
