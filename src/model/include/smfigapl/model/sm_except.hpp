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

#pragma once

#include <exception>
#include <string>
#include <vector>

#include "smfigapl/model/sm_error_codes.hpp"

namespace smfigapl
{

// An error inside the library: its code, the values for the message ($0,
// $1, ...) and an optional detail. The facade turns it into a Problem;
// what() is a short untranslated form.
    class Exception : public std::exception
    {
    private:
        ErrorCode code_;
        std::vector<std::string> params_;
        std::string detail_;
        std::string what_;
    public:
        explicit Exception(ErrorCode code, std::vector<std::string> params = {}, std::string detail = "");
        [[nodiscard]] const char* what() const noexcept override
        {
            return what_.c_str();
        }
        [[nodiscard]] ErrorCode code() const noexcept
        {
            return code_;
        }
        [[nodiscard]] const std::vector<std::string>& params() const noexcept
        {
            return params_;
        }
        [[nodiscard]] const std::string& detail() const noexcept
        {
            return detail_;
        }
    };
}//namespace
