#include "ikaros.h"

using namespace ikaros;

class Nucleus: public Module
{
    parameter   alpha;
    parameter   beta;
    parameter   phi;
    parameter   chi;
    parameter   psi;
    parameter   scale;
    parameter   epsilon;

    float       phi_scale;
    float       chi_scale;
    float       psi_scale;

    float       x;
    
    matrix      excitation;
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
        Bind(epsilon, "epsilon");


        Bind(excitation, "EXCITATION");
        Bind(inhibition, "INHIBITION");
        Bind(shunting_inhibition, "SHUNTING_INHIBITION");
        Bind(output, "OUTPUT");

        x = 0;
    }


    void Tick()
    {
        excitation.print();
        inhibition.print();

        output.subtract(excitation, inhibition); // FIXME: Insert correct function here *****
    }
};


INSTALL_CLASS(Nucleus)

