// Ikaros 3.0

#include <string>

#include "ikaros.h"

using namespace ikaros;

int
main(int argc, char *argv[])
{
    try
    { 
        Kernel & k = kernel();

        options o;
        o.add_option("r", "realtime", "run in real-time mode");
        o.add_option("s", "stop", "stop Ikaros after this tick");
        o.add_option("w", "webuiport", "port for ikaros WebUI", "8000");
 
        o.parse_args(argc, argv);

        std::cout << "Ikaros 3.0 Starting\n" << std::endl;

        //k.ListClasses();
        k.LoadFiles(o.filenames, o);
        k.ResolveParameters();
        k.CalculateSizes();
        k.InitComponents();
        //k.ListComponents();
        k.ListConnections();
        k.ListInputs();
        k.ListOutputs();
        k.ListParameters();
        k.PrintLog();


        for(int i=0; i<0; i++)
        {   
            k.Tick();
            k.Propagate();
        }
        //k.ListInputs();
        //k.ListOutputs();

        std::cout << "\nIkaros 3.0 Ended" << std::endl;
    }
    catch(exception & e)
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

