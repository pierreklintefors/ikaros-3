#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class ForceCheck: public Module
{
    matrix current;
    matrix current_limit;
    matrix current_output;
    matrix position;// assumes degrees
    matrix goal_position;

    int tickCount;
    int current_increment;
    int current_value;
    int position_margin;
    int current_margin;
    
    matrix Limiter(matrix currents, matrix limits, int increment)
    {
        for (int i = 0; i < currents.size(); i++) {
            int current_value = currents[i];
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
    matrix MovingCheck(matrix positions, matrix goal_positions, matrix current_limits)
    {
        for (int i = 0; i < positions.size(); i++) {
                
            float current_position = positions[i];
            float goal = goal_positions[i];
            float current_value = current_output[i];
            float limit_value = current_limit[i];
            if (abs(current_position - goal) > position_margin & abs(current_value - limit_value) < current_margin)
            {
                current_output[i] = 0; 
            }
                
        }
        return current_output;
    }

    void Init()
    {
        Bind(current, "PresentCurrent");
        Bind(current_limit, "CurrentLimit");
        Bind(current_output, "CurrentOutput");
        Bind(position, "PresentPosition");
        Bind(goal_position, "GoalPosition");

        
    
        

        tickCount =0;
        current_output.reset();
        position.print();
        current_increment = 5;
        current_margin = 100;
        position_margin = 5;

    }


    void Tick()
    {    std::cout << "tick" << std::endl;
        if (tickCount > 0){  
            current_output = MovingCheck(position, goal_position, current_limit);
            current_output = Limiter(current, current_limit, current_increment);
        }
        tickCount ++;
            
    }
};


INSTALL_CLASS(ForceCheck)

