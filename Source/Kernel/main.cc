// Ikaros 3.0

#include "ikaros.h"

using namespace ikaros;

int
main(int argc, char *argv[])
{
    try
    { 
        Kernel & k = kernel();

        options o;
        o.set_default("w", "8000");
        o.parse_args(argc, argv);
        //o.filenames.push_back("Add_test.ikg"); // Testing

        std::cout << "Ikaros 3.0 Starting" << std::endl;

        //k.ListClasses();
        k.LoadFiles(o.filenames);

        // ResolveParameters - indirection - sharing etc

        k.CalculateSizes();
        k.InitModules();
        //k.ListModules();
        //k.ListConnections();
        k.ListInputs();
        k.ListOutputs();
        k.ListParameters();

        exit(0);

        for(int i=0; i< 10; i++)
        {
            k.Tick();
            k.Propagate();
        }
        //k.ListInputs();
        //k.ListOutputs();

        std::cout << "Ikaros 3.0 Ended" << std::endl;
    }
    catch(exception e)
    {
        std::cerr << "IKAROS::EXCEPTION: " << e.what() << std::endl;
    }

    catch(const std::exception& e)
    {
        std::cerr << "STD::EXCEPTION: " << e.what() << std::endl;
    }
   catch(...)
    {
        std::cerr << "UNKOWN EXCEPTION " << std::endl;
    }
    return 0;
}

//
// TEMPORARY: INCLUDE MODULES HERE
//

