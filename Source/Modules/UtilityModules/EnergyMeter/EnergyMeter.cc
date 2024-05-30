#include "ikaros.h"

using namespace ikaros;

class EnergyMeter: public Module
{
    Socket      socket;

    parameter address;
    matrix measured_power;
    matrix energy;

    void Init()
    {
        Bind(address, "address");
        Bind(measured_power, "MEASURED_POWER");
        Bind(energy, "ENERGY");
    }


    void Tick()
    {
        try
        {
            // auto data = socket.HTTPGet("192.168.50.59/status");
            auto data = socket.HTTPGet(address);
            measured_power[0] = std::stof(split(data, "\"power\":", 1).at(1)); // Not exactly JSON parsing but it will do.
        }
        catch(const std::exception& e)
        {
            energy[0] = 0;
        }

        energy[0] += kernel().GetTickDuration()*(measured_power[0]) /(1000.0*1000.0*3600.0); // Convert time interval to hours and integrate to Wh
        //printf("ENERGY: %f, POWER: %f\n", *energy, *measurered_power);
    
    }
};

INSTALL_CLASS(EnergyMeter)

