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

#include <string>
#include <unordered_map>
#include <algorithm>

#include "../include/sm_lang.hpp"


std::string	smfigapl::SmLang::get(std::string in)
{
	if (translations_[language_].find(in)!=translations_[language_].end())
		return translations_[language_][in];
	else
		return in;
}

std::string	smfigapl::SmLang::get(std::string in, std::string l)
{
	if (translations_[l].find(in)!=translations_[l].end())
		return translations_[l][in];
	else
		return in;
}

std::vector<std::string>	smfigapl::SmLang::getAll(std::string in)
{
	std::vector<std::string> temp;
	temp.clear();
	for (auto && l: languages_)
	{
		if (translations_[l].find(in)!=translations_[l].end())
			temp.push_back(translations_[l][in]);
	}
	if(static_cast<uint>(temp.size())==0)
		return {in};
	else
	{
		temp.push_back(in);
		return temp;
	}
}

std::vector<std::string>	smfigapl::SmLang::getAllWithCapitalization(std::string in)
{
	std::vector<std::string> temp=this->getAll(in);
	uint size=static_cast<uint>(temp.size());
	temp.resize(size*3);
	for (uint i=0;i<size;i++)
		std::transform(temp[i].begin(), temp[i].end(), temp[i].begin(), ::tolower);
	for (uint i=0;i<size;i++)
	{
		temp[size+i]=temp[i];
		std::transform(temp[i].begin(), temp[i].begin()+1, temp[i].begin(), ::toupper);
	}
	for (uint i=0;i<size;i++)
	{
		temp[size*2+i]=temp[i];
		std::transform(temp[i].begin(), temp[i].end(), temp[i].begin(), ::toupper);
	}
	return temp;
}

std::vector<std::string>	smfigapl::SmLang::getAllWithCapitalization(std::string in, std::string l)
{
	std::vector<std::string> temp;
	temp.resize(3);
	temp[0]=this->get(in,l);
	std::transform(temp[0].begin(), temp[0].end(), temp[0].begin(), ::tolower);
	temp[1]=temp[0];
	std::transform(temp[1].begin(), temp[1].begin()+1, temp[1].begin(), ::toupper);
	temp[2]=temp[0];
	std::transform(temp[2].begin(), temp[2].end(), temp[2].begin(), ::toupper);
	return temp;
}

void smfigapl::SmLang::setLanguage (std::string l)
{
	language_=l;
}

std::string	smfigapl::SmLang::getErrorMessage(smfigapl::uint errorId)
{
	auto a=errorTranslations_.find(language_);
	auto b=errorTranslations_.end();

	if (errorTranslations_.find(language_)==errorTranslations_.end())
		return "";
	if (errorTranslations_[language_].find(errorId)!=errorTranslations_[language_].end())
		return errorTranslations_[language_][errorId];
	else
		return "";
}

