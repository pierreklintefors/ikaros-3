// utilities.h

#ifndef UTILITIES
#define UTILITIES


#include <string>
#include <vector>
#include <iostream>


namespace ikaros
{
    std::string trim(const std::string &s);
    const std::vector<std::string> split(const std::string & s, const std::string & sep, int maxsplit=-1);
    const std::vector<std::string> rsplit(const std::string & str, const std::string & sep, int maxsplit=-1);
    std::string join(const std::string & separator, const std::vector<std::string> & v, bool reverse);
    bool starts_with(const std::string & s, const std::string & start); // waiting for C++20
    bool ends_with(const std::string & s, const std::string & end); // waiting for C++20
    const std::string head(std::string s, char token); // without token
    const std::string tail(std::string s, char token, bool include_token=false);
    const std::string rhead(std::string s, char token); // like head but split from end
    const std::string rtail(std::string s, char token); // like tail but split from end
    bool contains(std::string & s, std::string n);
    bool is_integer(std::string & s);

    std::ostream& operator<<(std::ostream& os, const std::vector<int> & v);

    // Utility functions

    auto tab = [](int d){ return std::string(3*d, ' ');};

    void print_attribute_value(const std::string & name, int value, int indent=0);
    void print_attribute_value(const std::string & name, const std::string & value, int indent=0);
    void print_attribute_value(const std::string & name, const std::vector<int> & values, int indent=0, int max_items=0);
    void print_attribute_value(const std::string name, std::vector<float> & values, int indent=0, int max_items=0);
    void print_attribute_value(const std::string & name, const std::vector<std::vector<std::string>> &  values, int indent=0, int max_items=0);
};

#endif