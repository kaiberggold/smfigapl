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
#include <vector>
#include <string>

#include "../include/sm_val.hpp"

bool smfigapl::SmInputValidate::validateInput()
{
	try
	{
		if (input->setup.realCapacity.NRounds>MAX_ROUNDS)

			throw SmValueException(
			#ifdef SM_DBG
					"",
			#endif
			VAL_ERROR_NROUNDS_GT_MAX,{std::to_string(input->setup.realCapacity.NRounds)},input);
		if (input->setup.realCapacity.NRounds<1)
			throw SmValueException(
			#ifdef SM_DBG
					"",
			#endif
			VAL_ERROR_NROUNDS_LT_1,{std::to_string(input->setup.realCapacity.NRounds)},input);
		if (input->setup.realCapacity.NFields>MAX_FIELDS)
			throw SmValueException(
			#ifdef SM_DBG
					"",
			#endif
			VAL_ERROR_NFIELDS_GT_MAX,{std::to_string(input->setup.realCapacity.NFields)},input);

	}
	catch (const SmValueException& e)
	{
		 std::cerr << e.what();
		 exit(e.getErrorId() );
	}
	return true;
}
int smfigapl::SmInputValidate::getValError()
{
	return -123;
}
