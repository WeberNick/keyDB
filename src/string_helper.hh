#pragma once

#include "types.hh"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace StringUtil
{
    inline std::string join(const string_vt& aData, size_t aStartVal) noexcept
    {
        auto it_begin = std::begin(aData);
        for(size_t i = 0; i < aStartVal; ++i)
        {
            ++it_begin;
        }
        std::ostringstream strm;

        while(it_begin != std::end(aData))
        {
            strm << " " << *it_begin;
            ++it_begin;
        }
        return strm.str();
    }

    // trim from start (in place)
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    inline std::string ltrim_copy(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    inline std::string rtrim_copy(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    inline std::string trim_copy(std::string s) {
        trim(s);
        return s;
    }
  
    inline string_vt splitString(const std::string& aString, char aDelimiter) 
    {
        string_vt output;
        boost::split(output, aString, boost::is_any_of(std::string(1, aDelimiter)));
        return output;
    }
}

