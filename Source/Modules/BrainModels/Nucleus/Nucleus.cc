#include "ikaros.h"

using namespace ikaros;

class Nucleus: public Module
{
    parameter   alpha;      // parameters
    parameter   beta;
    parameter   phi;
    parameter   chi;
    parameter   psi;
    parameter   scale;
    parameter   epsilon;

    float       x;          // internal state

    matrix      excitation; // io
    matrix      inhibition;
    matrix      shunting_inhibition;
    matrix      output;

    void Init()
    {
        Bind(alpha, "alpha");
        Bind(beta, "beta");

        Bind(phi, "phi");
        Bind(chi, "chi");
        Bind(psi, "psi");

        Bind(scale, "scale");
        Bind(epsilon, "epsilon"); // FIXME: Change to rate parameter

        Bind(excitation, "EXCITATION");
        Bind(inhibition, "INHIBITION");
        Bind(shunting_inhibition, "SHUNTING_INHIBITION");
        Bind(output, "OUTPUT");

        x = 0;
    }

    void Tick()
    {
        float a = 0;
        float s = 1;

        if(scale)
        {
            s = 1/(1+psi * shunting_inhibition.average());
            a += phi * s * excitation.average();
            a -= chi * inhibition.average();
        }
        else
        {
            s = 1/(1+psi*shunting_inhibition.sum());
            a += phi * s * excitation.sum();
            a -= chi * inhibition.sum();
        }

        x += epsilon * (a - x);
        output = std::max(0.0f, alpha + beta * atan(x)/tan(1));
    }
};

INSTALL_CLASS(Nucleus)

