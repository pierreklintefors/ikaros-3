#include "ikaros.h"

#include <math.h>

using namespace ikaros;

class Oscillator: public Module
{
    parameter   osc_type;
    parameter   frequency;
    matrix      output;


    void Init()
    {
        Bind(osc_type, "type");
        Bind(frequency, "frequency");
        Bind(output, "OUTPUT");
    }


    float func(float freq, float time)
    {
        switch(int(osc_type))
        {
            case 0: return sin((1/(2*M_PI))*time*freq);
            case 1: return sin((1/(2*M_PI))*time*freq) > 0 ? 1 : 0;
            default: return 0;
        }
    }

    void Tick()
    {
        float time = kernel().GetTickTime();

        output.apply(frequency, [=](float x, float f) {return func(f, kernel().GetTime());});
    }
};


INSTALL_CLASS(Oscillator)

