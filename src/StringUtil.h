#pragma once

#include <iostream>

static inline bool ends_with(std::string const & value, std::string const & ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static inline bool begins_with(std::string const & value, std::string const & beginning) {
    return value.rfind(beginning, 0) == 0;
}

static inline std::string to_string(IPAddress ip_address) {
    std::string ret = "";
    ret += ip_address[0] += '.';
    ret += ip_address[1] += '.';
    ret += ip_address[2] += '.';
    ret += ip_address[3] += '.';
    return ret;
}

static inline std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}