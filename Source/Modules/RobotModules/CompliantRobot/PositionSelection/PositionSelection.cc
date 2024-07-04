#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class PositionSelection: public Module
{
  
    matrix position_input1;
    matrix new_position;
    matrix position_input2;
    matrix position_output;

    


    void Init()
    {
    
        Bind(position_input1, "PositionInput1");
        Bind(position_input2, "PositionInput2");
        Bind(position_output, "PositionOutput");

        new_position.set_name("NewPosition");
        position_input1.info();


    }


    void Tick()
    {     
        new_position.print();
        position_input1.print();
        position_input2.print();
        if (!position_input1.empty()) {
            
            if( position_input1.sum() != new_position.sum())
            {
                position_output.copy(position_input1);
                new_position.copy(position_input1);
                return;
                
            }
            if (position_input2.sum() > 0 && position_input1.sum() == new_position.sum() )
            {
                std::cout << "Position2 selected" << std::endl;
                position_output.copy(position_input2);          
                   
            }
            
              
            
        }




            
    }
};


INSTALL_CLASS(PositionSelection)

