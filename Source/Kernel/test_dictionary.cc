// dictionary_test.cc


#include <string>
#include <iostream>

#include "dictionary.h"
#include "xml.h"

using namespace ikaros;

int
main()
{
    dictionary d;

    d["a"] = "AAA";
    d["b"] = "BBB";

    d["c"] = dictionary();  // optional
    d["c"]["d"] = "CD";


    d["d"]["e"] = "DE";     // dictionary is created when accessed

    d["e"] = list();        // optional
    d["e"].push_back("X");

    d["f"].push_back("X");  // list at e is cerated when accessed
    d["f"][1] = "Y";        // create space in list when assigned

    std::cout << d << std::endl;


    dictionary d2;

    d2["A"]["B"][3]["C"][2] = "42";     // creates full data structure

    std::cout << d2 << std::endl;

        d2["A"]["B"][3]["D"][2] = "43";     // creates full data structure

    std::cout << d2 << std::endl;

    std::string s = d2["A"]["B"][3]["D"][2];

    std::cout << s << std::endl;

    value v = "xyz";    

    std::cout << v.size() << std::endl; // size is 1 since value contains one string

    value u;
    u["A"] = "1";
    u["B"] = "2";   

    std::cout << u.size() << std::endl; // size is 2 since u contains two entires

    value t;
    t[3] = "22";

     std::cout << t.size() << std::endl; // size is 4

    list z;
    z[3] = "22";

     std::cout << z << " size:"<< z.size() << std::endl; // size is 4

dictionary xmldict = dictionary("Add_test.ikg", true);
std::cout << xmldict << std::endl;

        std::cout << std::endl;

        for(auto & p : dictionary(u))
        {
            std::cout << p.first << ": " << p.second << std::endl;
        }

    return 0;
}



#include "xml.cc"

