#include "ikaros.h"

using namespace ikaros;

class Constant: public Module
{
public:

    parameter data;
    matrix output;

    int _SetSizes()
    {
        Bind(output, "OUTPUT");
        Bind(data, "data"); // Matrix parameter (later - string for now)

        if(output.rank() == 0)
        {
             if(std::string(data).empty()) // FIXME: Add to parameter functions parameter. empty() is string and rank for matrix
                return 0;
    
            auto l = split(data, ",");
            output.realloc((int)l.size());

            // Copy data to output

            return 1;
        }

        return 0;
    }

    void Init()
    {

    }


    void Tick()
    {
        // Copy data to output - parameter data may have changed from WebUI

        output[0] = 2;
    }
};

INSTALL_CLASS(Constant)

