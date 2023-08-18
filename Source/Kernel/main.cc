// Ikaros 3.0

#include <string>

#include "ikaros.h"

using namespace ikaros;

int
main(int argc, char *argv[])
{

/*
    Timer   t;

    double x = t.WaitUntil(1);
    double a = t.GetTime();

    Sleep(2.5);
    double b = t.GetTime();

    std::cout << "lag: " << x << std::endl;

    std::cout << a << "\n" << b << std::endl;
    std::cout << TimeString(a) << std::endl;
    std::cout << TimeString(b) << std::endl;
    std::cout << GetClockTimeString() << std::endl;
    std::cout << GetTimeStamp() << std::endl;

    for(int i=0; i<5; i++)
    {
        std::cout << GetClockTimeString() << std::endl;
        Sleep(1);
    }


exit(0);

*/















    try
    { 
        Kernel & k = kernel();

        options o;
 
        //o.add_option("l", "loglevel", "what to print to the log");
        //o.add_option("q", "quiet", "do not print log to terminal; equal to loglevel=0");
    
        //o.add_option("c", "lagcutoff", "reset lag and restart timing if it exceed this value", "10s");

        o.add_option("d", "tick_duration", "duration of each tick");
        o.add_option("r", "real_time", "run in real-time mode");
        o.add_option("S", "start", " start-up automatically without waiting for commands (from WebUI)");
        o.add_option("s", "stop", "stop Ikaros after this tick");
        o.add_option("w", "webui_port", "port for ikaros WebUI", "8000");
 
        o.parse_args(argc, argv);

        std::cout << "Ikaros 3.0 Starting\n" << std::endl;


    // Move the rest to ikaros kernel - load and restart
    
        //k.ListClasses();
        k.LoadFiles(o.filenames, o);
        k.ResolveParameters();
        k.CalculateDelays();
        k.CalculateSizes();
        k.InitBuffers();

        k.InitComponents();
        k.ListComponents();
        k.ListConnections();
        k.ListInputs();
        k.ListOutputs();
        k.ListParameters();
        k.PrintLog();
        k.Run();
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


/*

    main-loop
    handle-command / command line parameters
    


*/