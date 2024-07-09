#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class PositionSelection: public Module
{
  
    matrix position_input;
    matrix position_output;
    matrix previous_input;


    int difference_margin;
    bool new_goal;

    


    void Init()
    {
    
        Bind(position_input, "PositionInput");
        Bind(position_output, "PositionOutput");

        previous_input.copy(position_input);
        previous_input.set_name("PreviousOutput");
        
        difference_margin=1;
        new_goal = false;
        


    }


    void Tick()
    {     


        auto [changed, changed_row] = matrix_comparison(position_input, previous_input, difference_margin);
        
        if(changed)
        {   
            std::cout << "new goal" << std::endl;
            position_output.copy(position_input[changed_row]);
        }
        
        // Avoid setting goal positions to zero
        for(int i=0; i<position_output.size(); i++)
            if(position_output[i] == 0)
                position_output[i] = position_input(0,i);
        
        previous_input.copy(position_input);

        
    }

    
    std::pair<bool, int> matrix_comparison( matrix m1, matrix m2, int margin)
    {
        if (m1.rank() != m2.rank())
            return {false, -1};
        for(int i=0; i<m1.rank(); i++)
        { 
        if (abs(m1[i].sum() - m2[i].sum()) > margin && m1[i].sum() >0 && m2[i].sum() >0)
            return {true, i};
        
        }
        
        return {false, -1};
    }
};





INSTALL_CLASS(PositionSelection)

