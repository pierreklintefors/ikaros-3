
#include "ikaros.h"

using namespace ikaros;

class Add: public Module
{
    matrix      input1;
    matrix      input2;
    matrix      output;
    //parameter   x;
/*
    int SetSizes()
    {
        Bind(input1, "INPUT1");
        Bind(input2, "INPUT2");
        Bind(output, "OUTPUT");
    
        if(output.rank())
            return 1; // already set - 1 = complete

        if(input1.rank())
        {
            output.realloc(input1.shape());     // FIXME: should also check that input1 has the same size
            return 1;
        }

        if(input2.rank())
        {
            output.realloc(input2.shape());     // FIXME: should also check that input1 has the same size
            return 1;
        }
        
        return 0; // No progress
    }
*/


    void Init()
    {
        Bind(input1, "INPUT1");
        Bind(input2, "INPUT2");
        Bind(output, "OUTPUT");
        //Bind(x, "X");
    }


    void Tick()
    {
        output.add(input1, input2);
    }
};


INSTALL_CLASS(Add)

