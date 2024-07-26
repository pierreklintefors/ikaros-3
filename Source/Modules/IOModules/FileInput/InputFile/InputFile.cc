#include "ikaros.h"

using namespace ikaros;

class InputFile: public Module
{
<<<<<<< HEAD
    parameter data;
    matrix output;

    void Init()
    {
        Bind(data, "data");
        Bind(output, "OUTPUT");
=======
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
>>>>>>> ac1fbd8dff7b0227f19b1ce154af41efac03d72e
    }


    void Tick()
    {
<<<<<<< HEAD
        output.copy(data);
    }
};

=======
       std::cout << "InputFile::Tick" << std::endl;
    }
};


>>>>>>> ac1fbd8dff7b0227f19b1ce154af41efac03d72e
INSTALL_CLASS(InputFile)

