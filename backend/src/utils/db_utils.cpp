#include "db_utils.hpp"
#include <iostream>
#include <random>

std::string join_players(const std::vector<std::string>& v) {
    std::string res;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) res += ",";
        res += v[i];
    }
    return res;
}
std::vector<std::string> split_players(const std::string& s) {
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = s.find(',', start)) != std::string::npos) {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    if (start < s.size()) result.push_back(s.substr(start));
    return result;
}