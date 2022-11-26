#pragma once

#include <iostream>
#include <sstream>

namespace StringUtil
{
    static inline bool ends_with(std::string const &value, std::string const &ending)
    {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    static inline bool begins_with(std::string const &value, std::string const &beginning)
    {
        return value.rfind(beginning, 0) == 0;
    }

    static inline std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    static inline std::string bool_to_string(bool value)
    {
        return value ? "true" : "false";
    }

    template <typename T>
    static inline std::string to_string(T const &value)
    {
        std::stringstream sstr;
        sstr << value;
        return sstr.str();
    }
};