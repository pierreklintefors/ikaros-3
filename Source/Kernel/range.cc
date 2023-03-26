// range.cc

#include <iostream>

#include "range.h"

using namespace ikaros;

std::ostream& operator<<(std::ostream& os, const range & x)
{
    std::string sep;
    std::cout << "(";
    for(auto i : x.index_)
    {
        std::cout << sep << i;
        sep = ", ";
    }
    std::cout << ")";
    return os;
};
