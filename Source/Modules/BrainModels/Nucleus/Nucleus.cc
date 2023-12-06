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
        float a = 0;
        float s = 1;
    
        if(scale)
        {
            s= 1/(1+psi * shunting_inhibition.average());
            a += phi * s * excitation.average();
            a -= chi * inhibition.average();
        }
        else
        {
            s= 1/(1+psi*shunting_inhibition.sum());
            a += phi * s * excitation.sum();
            a -= chi * inhibition.sum();
        }

        x += float(epsilon) * (a - x);

        
        // Activation function with
        // f(x) = 0 if x <= 0
        // f(x) = 1 if x = 1
        // f(x) -> 2 as x -> inf
        // S-shaped from 0+
        // derivative: 4x / (x^2+1)^2
    
        switch(1)
        {
            case 0:
                if(x < 0)
                    output = 0;
                else
                    output = 2*x*x/(1+x*x);
                break;
                
            case 1:
                if(x < 0)
                    output = 0;
                else
                    output = alpha + beta * atan(x)/tan(1);
                break;
        }
        
    }
};

INSTALL_CLASS(Nucleus)

/*
    if(scale)
    {
        if(excitation_size > 0)
            phi_scale = 1.0/float(excitation_size);

        if(inhibition_size > 0)
            chi_scale = 1.0/float(excitation_size);

        if(shunting_inhibition_size > 0)
            psi_scale = 1.0/float(shunting_inhibition_size);
    }
    else
    {
        phi_scale = 1.0;
        chi_scale = 1.0;
        psi_scale = 1.0;
    }

    float a = 0;
    float s = 1;
    
    if(shunting_inhibition)
        s = 1/(1+psi*psi_scale*sum(shunting_inhibition, shunting_inhibition_size));

    if(excitation)
        a += phi * phi_scale * s * sum(excitation, excitation_size);

     if(inhibition)
        a -= chi * chi_scale * sum(inhibition, inhibition_size);

     x += epsilon * (a - x);
    
    // Activation function with
    // f(x) = 0 if x <= 0
    // f(x) = 1 if x = 1
    // f(x) -> 2 as x -> inf
    // S-shaped from 0+
    // derivative: 4x / (x^2+1)^2
    
    switch(1)
    {
        case 0:
            if(x < 0)
                *output = 0;
            else
                *output = 2*x*x/(1+x*x);
            break;
            
        case 1:
            if(x < 0)
                *output = 0;
            else
                *output = alpha + beta * atan(x)/tan(1);
            break;
    }

}
*/

