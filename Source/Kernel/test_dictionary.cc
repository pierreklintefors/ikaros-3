// dictionary_test.cc


#include <string>
#include <iostream>

#include "dictionary.h"
#include "xml.h"

// To allow compilation directly from editor
#include "xml.cc"
#include "dictionary.cc"


using namespace ikaros;

int
main()
{

    dictionary d;
    dictionary e = d;

    d["a"] = "AAA";
    d["b"] = "BBB";
    d["c"] = true;
    d["d"] = 42;
    d["e"] = 3.14;

    d["a"] = "XXX";

 std::cout <<  d << std::endl;
 std::cout <<  e.json() << std::endl;

/*
dictionary xmldict = dictionary("../Modules/TestModules/Test1.ikc");
//std::cout << xmldict.json() << std::endl;
std::cout <<  std::endl;
std::cout << xmldict.xml("group") << std::endl;



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


    std::cout << std::endl;

    for(auto & p : dictionary(u))
    {
        std::cout << p.first << ": " << p.second << std::endl;
    }


    dictionary d42("../Modules/TestModules/Test1.ikc");

        std::cout << std::endl;
        std::cout << std::endl;

        dictionary dddd;
        dddd["a"] = "32";
        dddd["l"][0] = "X";
        dddd["l"][1] = "Y";
        dddd["l"][3] = "Z";

        std::cout << dddd.xml("group") << std::endl;

    value xxx;

    int a = 42;
    int * ap = &a;
    int & b(*ap);

        std::cout << b << std::endl;


    dictionary p;
    dictionary q;

    p["a"] = "ap";
    p["b"] = "bp";

    q["b"] = "bq";
    q["c"] = "cq";

    p.merge(q, true);

        std::cout << p << std::endl;
*/
    return 0;
}

