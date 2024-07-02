#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class ForceCheck: public Module
{
    matrix present_current;
    matrix current_limit;
    matrix current_output;
    matrix position;// assumes degrees
    matrix goal_position_in;
    matrix goal_position_out;

    int tickCount;
    int current_increment;
    int current_value;
    int position_margin;
    int current_margin;
    int minimum_current = 5;
    int element;
    
    matrix Limiter(matrix currents, matrix limits, int increment)
    {
        for (int i = 0; i < position.size(); i++) {
            int current_value = abs(currents[i]);
            int limit_value = limits[i];   
            if ( current_value< limit_value) {
                current_output[i] = std::min(current_value + increment, limit_value); // Cap at limit_value              
            } else {
                current_output[i]= current_output[i];
                
            }
            element = current_output[i];
            current_output[i] = std::max(element, minimum_current); 
        }
        
        return current_output;
    }

    //Could also be done by using dynamixel_sdk
    matrix MovingCheck(matrix positions,  matrix current_limits, matrix goal_position_in, matrix goal_position_out )
    {
        for (int i = 0; i < positions.size(); i++) {
            float current_position = positions[i];
            float goal = goal_position_in[i];
            float current_value = current_output[i];
            float limit_value = current_limit[i];

            if (abs(current_position - goal) > position_margin & abs(limit_value -current_value) < current_margin)
            {   
                std::cout << "Obsticale detected, lowering current" << std::endl;
                current_output[i]=0; 
                goal_position_out[i] = current_position;
            }
            else 
            {  
                current_output[i] = current_value;
     
            }
        }
        return current_output;
    }

    void Init()
    {
        Bind(present_current, "PresentCurrent");
        Bind(current_limit, "CurrentLimit");
        Bind(current_output, "CurrentOutput");
        Bind(position, "PresentPosition");
        Bind(goal_position_in, "GoalPositionIn");
        Bind(goal_position_in, "GoalPositionOut");


        
        goal_position_out.set(0);
        

        
      
        current_increment = 10;
    
        current_margin = 10;
        position_margin = 5;


    }


    void Tick()
    {   
        

        if (position.connected() & present_current.connected()){ 
            goal_position_out = goal_position_in;
            current_output = MovingCheck(position, current_limit, goal_position_in, goal_position_out);
            current_output = Limiter(present_current, current_limit, current_increment);
        }
        
       
        
            
    }
};


INSTALL_CLASS(ForceCheck)

