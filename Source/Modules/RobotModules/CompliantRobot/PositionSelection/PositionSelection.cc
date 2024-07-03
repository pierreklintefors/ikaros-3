#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class PositionSelection: public Module
{
  
    matrix position_input1;
    matrix position_input2;
    matrix position_output;

    


    void Init()
    {
    
        Bind(position_input1, "PositionInput1");
        Bind(position_input2, "PositionInput2");
        Bind(position_output, "PositionOutput");

        
        


    }


    void Tick()
    {   
        Notify(msg_debug, "Entering Tick of PositionSelection");
        
        if (position_input1.connected() &position_input2.connected()) {
            Notify(msg_debug, "Input connected in PositionSelection");
            if (position_input2.sum() > 0)
            {
                position_output.copy(position_input2);
            }
            else
            {
                position_output.copy(position_input1);
            }
            
        }
        
        position_output = {180,180,0,0,12,12,0,0,0,0,0,0,0,0,0,0,0,0,0};
        position_output.print();

            
    }
};


INSTALL_CLASS(PositionSelection)

