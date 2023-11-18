// Ikaros 3.0

#include <string>


#include "ikaros.h"

using namespace ikaros;

static bool terminate = false;
static ServerSocket * socket = nullptr;


/*

void HTTPReply(std::string msg)
{
    Dictionary header;
    header.Set("Content-Type", "text/text");
    header.Set("Cache-Control", "no-cache");
    header.Set("Cache-Control", "no-store");
    header.Set("Pragma", "no-cache");
    socket->SendHTTPHeader(&header);
    socket->Send(msg);
    socket->Send("\n\n");
    socket->Close();
}



void
HTTPThread()
{
    while(!terminate)
    {
        if (socket != nullptr && socket->GetRequest(true))
        {
            if (equal_strings(socket->header.Get("Method"), "GET"))
            {
              //  while(true) // tick_is_running
              //      {}
                //handling_request = true;

                std::string uri = socket->header.Get("URI");
                std::cout << uri << std::endl;

                if(starts_with(uri, "/ikaros/"))
                {
                                    std::cout << "TOP LEVEL COMMAND: " << uri << std::endl;
                                    HTTPReply("OK");
                }
                else
                {
                    // Send to kernel if any is running
                }
            }

            //handling_request = false;
        }
        socket->Close();
    }
}

*/



int
main(int argc, char *argv[])
{
    matrix m = "1, 2, 3; 4, 5, 6";

    m.set_name("MATRIX");
    m.set_labels(0, "Row1", "Row2");
    m.set_labels(1, "Col1", "Col2, Col3");
    m.print();
    m.info();

    std::cout << m << std::endl;
    std::cout << m.json() << std::endl;


/*
    int port = 8000;
    try
    {
        socket =  new ServerSocket(port);
    }
    catch (const SocketException& e)
    {
        throw fatal_error("Ikaros is unable to start a webserver on port "+std::to_string(port)+". Make sure no other ikaros process is running and try again.");
    }

    std::thread * httpThread = new std::thread(HTTPThread);

    while(true)
        {
                    // std::cout << "*** loop" << std::endl;
        }
*/


    try
    { 
        Kernel & k =kernel();

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

        long port = o.get_long("webui_port");

        std::cout << "Ikaros 3.0 Starting\n" << std::endl;

    // Move the rest to ikaros kernel - load and restart
    
        //k.ListClasses();
        k.InitSocket(port); // FIXME: Port cannot be set in IKG file; should it?

        k.LoadFiles(o.filenames, o);
        //k.ListBuffers();
        //k.ListCircularBuffers();

        k.Run();
        //k.ListInputs();
        //k.ListOutputs();

        std::cout << "\nIkaros 3.0 Ended" << std::endl;
    }
    catch(fatal_error & e)
    {
        std::cerr << "IKAROS::FATAL ERROR: " << e.what() << std::endl;
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

