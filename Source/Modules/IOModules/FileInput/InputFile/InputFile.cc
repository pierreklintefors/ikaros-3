#include "ikaros.h"

using namespace ikaros;

class InputFile: public Module
{
public:
    parameter filename;

// Initialization must be done in creator

    InputFile()
    {
        Bind(filename, "filename");

        info_["outputs"] = list(); // Temporary fix; check that it does not already exist in AddOutput later

        AddOutput("MYOUTPUT", 10, "my programatically added output");
    }


    void Init()
    {
        std::cout << "InputFile::Init" << std::endl;
    }


    void Tick()
    {
       std::cout << "InputFile::Tick" << std::endl;
    }
};


INSTALL_CLASS(InputFile)

