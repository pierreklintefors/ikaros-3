#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class ForceCheck: public Module
{
    matrix present_current;
    matrix current_limit;
    matrix current_output;
    matrix present_position;// assumes degrees
    matrix goal_position_in;
    matrix goal_position_out;

    int tickCount;
    int current_increment;
    int current_value;
    int position_margin;
    int current_margin;
    int minimum_current;
    int element;
    
    matrix Limiter(matrix currents, matrix limits, int increment)
    {
        for (int i = 0; i < present_position.size(); i++) {
            int current_value = abs(currents[i]);
            int limit_value = limits[i];   
            if ( current_value< limit_value) {
                current_output[i] = std::min(current_value + increment, limit_value); // Cap at limit_value              
            } else {
                current_output[i]= current_output[i];
                
            }
             
        }
        
        return current_output;
    }

    //Could also be done by using dynamixel_sdk
    matrix MovingCheck(matrix positions,  matrix current_limits, matrix goal_position_in, matrix goal_position_out )
    {   
       
   ;
        for (int i = 0; i < positions.size(); i++) {
            float current_position = positions(i);
            float goal = goal_position_in[i];
            float current_value = current_output[i];
            float limit_value = current_limit[i];
            if (current_position >0){

                if (abs(goal - current_position) > position_margin & abs(limit_value -current_value) < current_margin)
                {   
                    current_output[i]=0; 
                    std::cout << "Obstacle detected, lowering current for servo " << i+1 << " to " << current_output(i) << std::endl;
                    goal_position_out[i] = current_position;
                    std::cout << "Goal position changed to " << current_position << std::endl;
                }
                else 
                {  
                    current_output[i] = current_value;
                    
                }

            }
        }
        return current_output;
    }

    void Init()
    {
        Bind(present_current, "PresentCurrent");
        Bind(current_limit, "CurrentLimit");
        Bind(current_output, "CurrentOutput");
        Bind(present_position, "PresentPosition");
        Bind(goal_position_in, "GoalPositionIn");
        Bind(goal_position_out, "GoalPositionOut");

      
        current_increment = 10;
    
        current_margin = 10;
        position_margin = 10;
        tickCount = 0;
        minimum_current = 5;

    }


    void Tick()
    {   
       

        if (!present_position.empty() & !present_current.empty() & !goal_position_in.empty()) {
            current_output = MovingCheck(present_position, current_limit, goal_position_in, goal_position_out);
            current_output = Limiter(present_current, current_limit, current_increment);
        }
        
        
       
      
            
    }
};


INSTALL_CLASS(ForceCheck)

