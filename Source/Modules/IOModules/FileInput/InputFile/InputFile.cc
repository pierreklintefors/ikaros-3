#include "ikaros.h"

using namespace ikaros;

class InputFile: public Module
{
    parameter data;
    matrix output;

    void Init()
    {
        Bind(data, "data");
        Bind(output, "OUTPUT");
    }


    void Tick()
    {
        output.copy(data);
    }
};

INSTALL_CLASS(InputFile)

