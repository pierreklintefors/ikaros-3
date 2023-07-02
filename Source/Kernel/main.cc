// Ikaros 3.0

#include <string>

#include "ikaros.h"

using namespace ikaros;

int
main(int argc, char *argv[])
{

/*
    matrix m;
    matrix m0 = {0};
    matrix m1 = {1};
    matrix m2 = {2};
    matrix m3 = {3};

    CircularBuffer circ(m, 5);

    circ.rotate(m1);
    circ.rotate(m2);
    circ.rotate(m3);
    circ.rotate(m0);
    circ.rotate(m0);
    circ.rotate(m0);
    circ.rotate(m0);
    circ.rotate(m0);
    circ.rotate(m0);

    matrix m0 = {0, 0};
    matrix m = {1, 2};

    matrix l = m.last();
    m = m0;

*/

    try
    { 
        Kernel & k = kernel();

        options o;
        o.add_option("r", "realtime", "run in real-time mode");
        o.add_option("s", "stop", "stop Ikaros after this tick");
        o.add_option("w", "webuiport", "port for ikaros WebUI", "8000");
 
        o.parse_args(argc, argv);

        std::cout << "Ikaros 3.0 Starting\n" << std::endl;

        k.stop_after = o.get_long("stop");

        //k.ListClasses();
        k.LoadFiles(o.filenames, o);
        k.ResolveParameters();
        k.CalculateDelays();
        k.CalculateSizes();
        k.InitBuffers();

        k.InitComponents();
        //k.ListComponents();
        k.ListConnections();
        k.ListInputs();
        k.ListOutputs();
        k.ListParameters();
        k.PrintLog();

        while(true && k.IsRunning())
        {   
            k.Tick();
            k.RotateBuffers();
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

