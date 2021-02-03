#pragma once

#include <iostream>

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

std::string to_string(IPAddress ip_address) {
    std::string ret = "";
    ret += ip_address[0] += '.';
    ret += ip_address[1] += '.';
    ret += ip_address[2] += '.';
    ret += ip_address[3] += '.';
    return ret;
}