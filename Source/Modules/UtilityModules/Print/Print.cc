#include "ikaros.h"
#include <iostream>

using namespace ikaros;

class Print: public Module
{
public:
    matrix      input;

    void Init()
    {
        Bind(input, "INPUT");
    }


    void Tick()
    {
        if(input.size(0) > 0)
            std::cout << "PRINT: " << input[0] << std::endl;
        else
        std::cout << "PRINT: no data " << std::endl;
     }
};


INSTALL_CLASS(Print)
